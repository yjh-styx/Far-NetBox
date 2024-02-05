/* inffast.c -- fast decoding
 * Copyright (C) 1995-2017 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "zbuild.h"
#include "zutil.h"
#include "inftrees.h"
#include "inflate.h"
#include "inffast.h"
#include "memcopy.h"

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    (hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    do { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } while (0)

#ifdef INFFAST_CHUNKSIZE
/*
   Ask the compiler to perform a wide, unaligned load with an machine
   instruction appropriate for the inffast_chunk_t type.
 */
static inline inffast_chunk_t loadchunk(unsigned char const* s) {
    inffast_chunk_t c;
    __builtin_memcpy(&c, s, sizeof(c));
    return c;
}

/*
   Ask the compiler to perform a wide, unaligned store with an machine
   instruction appropriate for the inffast_chunk_t type.
 */
static inline void storechunk(unsigned char* d, inffast_chunk_t c) {
    __builtin_memcpy(d, &c, sizeof(c));
}

/*
   Behave like memcpy, but assume that it's OK to overwrite at least
   INFFAST_CHUNKSIZE bytes of output even if the length is shorter than this,
   that the length is non-zero, and that `from` lags `out` by at least
   INFFAST_CHUNKSIZE bytes (or that they don't overlap at all or simply that
   the distance is less than the length of the copy).

   Aside from better memory bus utilisation, this means that short copies
   (INFFAST_CHUNKSIZE bytes or fewer) will fall straight through the loop
   without iteration, which will hopefully make the branch prediction more
   reliable.
 */
static inline unsigned char* chunkcopy(unsigned char *out, unsigned char const *from, unsigned len) {
    --len;
    storechunk(out, loadchunk(from));
    out += (len % INFFAST_CHUNKSIZE) + 1;
    from += (len % INFFAST_CHUNKSIZE) + 1;
    len /= INFFAST_CHUNKSIZE;
    while (len-- > 0) {
        storechunk(out, loadchunk(from));
        out += INFFAST_CHUNKSIZE;
        from += INFFAST_CHUNKSIZE;
    }
    return out;
}

/*
   Behave like chunkcopy, but avoid writing beyond of legal output.
 */
static inline unsigned char* chunkcopysafe(unsigned char *out, unsigned char const *from, unsigned len,
                                           unsigned char *safe) {
    if (out > safe) {
        while (len-- > 0) {
          *out++ = *from++;
        }
        return out;
    }
    return chunkcopy(out, from, len);
}

/*
   Perform short copies until distance can be rewritten as being at least
   INFFAST_CHUNKSIZE.

   This assumes that it's OK to overwrite at least the first
   2*INFFAST_CHUNKSIZE bytes of output even if the copy is shorter than this.
   This assumption holds because inflate_fast() starts every iteration with at
   least 258 bytes of output space available (258 being the maximum length
   output from a single token; see inflate_fast()'s assumptions below).
 */
static inline unsigned char* chunkunroll(unsigned char *out, unsigned *dist, unsigned *len) {
    unsigned char const *from = out - *dist;
    while (*dist < *len && *dist < INFFAST_CHUNKSIZE) {
        storechunk(out, loadchunk(from));
        out += *dist;
        *len -= *dist;
        *dist += *dist;
    }
    return out;
}
#endif

#ifdef INFFAST_CHUNKSIZE
/*
   Ask the compiler to perform a wide, unaligned load with an machine
   instruction appropriate for the inffast_chunk_t type.
 */
static inline inffast_chunk_t loadchunk(uint8_t const* s)
{
    inffast_chunk_t c;
    __builtin_memcpy(&c, s, sizeof(c));
    return c;
}

/*
   Ask the compiler to perform a wide, unaligned store with an machine
   instruction appropriate for the inffast_chunk_t type.
 */
static inline void storechunk(uint8_t* d, inffast_chunk_t c)
{
    __builtin_memcpy(d, &c, sizeof(c));
}

/*
   Behave like memcpy, but assume that it's OK to overwrite at least
   INFFAST_CHUNKSIZE bytes of output even if the length is shorter than this,
   that the length is non-zero, and that `from` lags `out` by at least
   INFFAST_CHUNKSIZE bytes (or that they don't overlap at all or simply that
   the distance is less than the length of the copy).

   Aside from better memory bus utilisation, this means that short copies
   (INFFAST_CHUNKSIZE bytes or fewer) will fall straight through the loop
   without iteration, which will hopefully make the branch prediction more
   reliable.
 */
static inline uint8_t* chunkcopy(uint8_t *out, uint8_t const *from, uint32_t len)
{
    --len;
    storechunk(out, loadchunk(from));
    out += (len % INFFAST_CHUNKSIZE) + 1;
    from += (len % INFFAST_CHUNKSIZE) + 1;
    len /= INFFAST_CHUNKSIZE;
    while (len-- > 0) {
        storechunk(out, loadchunk(from));
        out += INFFAST_CHUNKSIZE;
        from += INFFAST_CHUNKSIZE;
    }
    return out;
}

/*
   Behave like chunkcopy, but avoid writing beyond of legal output.
 */
static inline uint8_t* chunkcopysafe(uint8_t *out, uint8_t const *from, uint32_t len,
                                           uint8_t *safe)
{
    if (out > safe) {
        while (len-- > 0) {
          *out++ = *from++;
        }
        return out;
    }
    return chunkcopy(out, from, len);
}

/*
   Perform short copies until distance can be rewritten as being at least
   INFFAST_CHUNKSIZE.

   This assumes that it's OK to overwrite at least the first
   2*INFFAST_CHUNKSIZE bytes of output even if the copy is shorter than this.
   This assumption holds because inflate_fast() starts every iteration with at
   least 258 bytes of output space available (258 being the maximum length
   output from a single token; see inflate_fast()'s assumptions below).
 */
static inline uint8_t* chunkunroll(uint8_t *out, uint32_t *dist, uint32_t *len)
{
    uint8_t const *from = out - *dist;
    while (*dist < *len && *dist < INFFAST_CHUNKSIZE) {
        storechunk(out, loadchunk(from));
        out += *dist;
        *len -= *dist;
        *dist += *dist;
    }
    return out;
}
#endif

/*
   Decode literal, length, and distance codes and write out the resulting
   literal and match bytes until either not enough input or output is
   available, an end-of-block is encountered, or a data error is encountered.
   When large enough input and output buffers are supplied to inflate(), for
   example, a 16K input buffer and a 64K output buffer, more than 95% of the
   inflate execution time is spent in this routine.

   Entry assumptions:

        state->mode == LEN
        strm->avail_in >= INFLATE_FAST_MIN_HAVE
        strm->avail_out >= INFLATE_FAST_MIN_LEFT
        start >= strm->avail_out
        state->bits < 8

   On return, state->mode is one of:

        LEN -- ran out of enough output space or enough available input
        TYPE -- reached end of block code, inflate() to interpret next block
        BAD -- error in block data

   Notes:

    - The maximum input bits used by a length/distance pair is 15 bits for the
      length code, 5 bits for the length extra, 15 bits for the distance code,
      and 13 bits for the distance extra.  This totals 48 bits, or six bytes.
      Therefore if strm->avail_in >= 6, then there is enough input to avoid
      checking for available input while decoding.

    - On some architectures, it can be significantly faster (e.g. up to 1.2x
      faster on x86_64) to load from strm->next_in 64 bits, or 8 bytes, at a
      time, so INFLATE_FAST_MIN_HAVE == 8.

    - The maximum bytes that a single length/distance pair can output is 258
      bytes, which is the maximum length that can be coded.  inflate_fast()
      requires strm->avail_out >= 258 for each loop to avoid checking for
      output space.
 */
void ZLIB_INTERNAL inflate_fast(PREFIX3(stream) *strm, unsigned long start) {
    /* start: inflate()'s starting value for strm->avail_out */
    struct inflate_state *state;
    const uint8_t *in;    /* local strm->next_in */
    const uint8_t *last;  /* have enough input while in < last */
    uint8_t *out;         /* local strm->next_out */
    uint8_t *beg;         /* inflate()'s initial strm->next_out */
    uint8_t *end;         /* while out < end, enough space available */
#ifdef INFFAST_CHUNKSIZE
    unsigned char *safe;        /* can use chunkcopy provided out < safe */
#endif
#ifdef INFLATE_STRICT
    uint32_t dmax;              /* maximum distance from zlib header */
#endif
    uint32_t wsize;             /* window size or zero if not using window */
    uint32_t whave;             /* valid bytes in the window */
    uint32_t wnext;             /* window write index */
    uint8_t *window;      /* allocated sliding window, if wsize != 0 */

    uint32_t bits;              /* local strm->bits */
    /* hold is a local copy of strm->hold. By default, hold satisfies the same
       invariants that strm->hold does, namely that (hold >> bits) == 0. This
       invariant is kept by loading bits into hold one byte at a time, like:

       hold |= next_byte_of_input << bits; in++; bits += 8;

       If we need to ensure that bits >= 15 then this code snippet is simply
       repeated. Over one iteration of the outermost do/while loop, this
       happens up to six times (48 bits of input), as described in the NOTES
       above.

       However, on some little endian architectures, it can be significantly
       faster to load 64 bits once instead of 8 bits six times:

       if (bits <= 16) {
         hold |= next_8_bytes_of_input << bits; in += 6; bits += 48;
       }

       Unlike the simpler one byte load, shifting the next_8_bytes_of_input
       by bits will overflow and lose those high bits, up to 2 bytes' worth.
       The conservative estimate is therefore that we have read only 6 bytes
       (48 bits). Again, as per the NOTES above, 48 bits is sufficient for the
       rest of the iteration, and we will not need to load another 8 bytes.

       Inside this function, we no longer satisfy (hold >> bits) == 0, but
       this is not problematic, even if that overflow does not land on an 8 bit
       byte boundary. Those excess bits will eventually shift down lower as the
       Huffman decoder consumes input, and when new input bits need to be loaded
       into the bits variable, the same input bits will be or'ed over those
       existing bits. A bitwise or is idempotent: (a | b | b) equals (a | b).
       Note that we therefore write that load operation as "hold |= etc" and not
       "hold += etc".

       Outside that loop, at the end of the function, hold is bitwise and'ed
       with (1<<bits)-1 to drop those excess bits so that, on function exit, we
       keep the invariant that (state->hold >> state->bits) == 0.
    */
    uint64_t hold;              /* local strm->hold */
    code const *lcode;          /* local strm->lencode */
    code const *dcode;          /* local strm->distcode */
    uint32_t lmask;             /* mask for first level of length codes */
    uint32_t dmask;             /* mask for first level of distance codes */
    code here;                  /* retrieved table entry */
    uint32_t op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    uint32_t len;               /* match length, unused bytes */
    uint32_t dist;              /* match distance */
    uint8_t *from;        /* where to copy match from */

    /* copy state to local variables */
    state = (struct inflate_state *)strm->state;
    in = strm->next_in;
    last = in + (strm->avail_in - (INFLATE_FAST_MIN_HAVE - 1));
    out = strm->next_out;
    beg = out - (start - strm->avail_out);
    end = out + (strm->avail_out - (INFLATE_FAST_MIN_LEFT - 1));

#ifdef INFFAST_CHUNKSIZE
    safe = out + (strm->avail_out - INFFAST_CHUNKSIZE);
#endif
#ifdef INFLATE_STRICT
    dmax = state->dmax;
#endif
    wsize = state->wsize;
    whave = state->whave;
    wnext = state->wnext;
    window = state->window;
    hold = state->hold;
    bits = state->bits;
    lcode = state->lencode;
    dcode = state->distcode;
    lmask = (1U << state->lenbits) - 1;
    dmask = (1U << state->distbits) - 1;

    /* decode literals and length/distances until end-of-block or not enough
       input data or output space */
    do {
        if (bits < 15) {
            hold |= load_64_bits(in, bits);
            in += 6;
            bits += 48;
        }
        here = lcode[hold & lmask];
      dolen:
        DROPBITS(here.bits);
        op = here.op;
        if (op == 0) {                          /* literal */
            Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", here.val));
            *out++ = (uint8_t)(here.val);
        } else if (op & 16) {                     /* length base */
            len = here.val;
            op &= 15;                           /* number of extra bits */
            if (op) {
                if (bits < op) {
                    hold |= load_64_bits(in, bits);
                    in += 6;
                    bits += 48;
                }
                len += BITS(op);
                DROPBITS(op);
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            if (bits < 15) {
                hold |= load_64_bits(in, bits);
                in += 6;
                bits += 48;
            }
            here = dcode[hold & dmask];
          dodist:
            DROPBITS(here.bits);
            op = here.op;
            if (op & 16) {                      /* distance base */
                dist = here.val;
                op &= 15;                       /* number of extra bits */
                if (bits < op) {
                    hold |= load_64_bits(in, bits);
                    in += 6;
                    bits += 48;
                }
                dist += BITS(op);
#ifdef INFLATE_STRICT
                if (dist > dmax) {
                    strm->msg = (char *)"invalid distance too far back";
                    state->mode = BAD;
                    break;
                }
#endif
                DROPBITS(op);
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (uint32_t)(out - beg);     /* max distance in output */
                if (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    if (op > whave) {
                        if (state->sane) {
                            strm->msg = (char *)"invalid distance too far back";
                            state->mode = BAD;
                            break;
                        }
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                        if (len <= op - whave) {
                            do {
                                *out++ = 0;
                            } while (--len);
                            continue;
                        }
                        len -= op - whave;
                        do {
                            *out++ = 0;
                        } while (--op > whave);
                        if (op == 0) {
                            from = out - dist;
                            do {
                                *out++ = *from++;
                            } while (--len);
                            continue;
                        }
#endif
                    }
#ifdef INFFAST_CHUNKSIZE
                    from = window;
                    if (wnext == 0) {           /* very common case */
                        from += wsize - op;
                    } else if (wnext >= op) {   /* contiguous in window */
                        from += wnext - op;
                    } else {                    /* wrap around window */
                        op -= wnext;
                        from += wsize - op;
                        if (op < len) {         /* some from end of window */
                            len -= op;
                            out = chunkcopysafe(out, from, op, safe);
                            from = window;      /* more from start of window */
                            op = wnext;
                            /* This (rare) case can create a situation where
                               the first chunkcopy below must be checked.
                             */
                        }
                    }
                    if (op < len) {             /* still need some from output */
                        len -= op;
                        out = chunkcopysafe(out, from, op, safe);
                        if (dist == 1) {
                            out = byte_memset(out, len);
                        } else {
                            out = chunkunroll(out, &dist, &len);
                            out = chunkcopysafe(out, out - dist, len, safe);
                        }
                    } else {
                        if (from - out == 1) {
                            out = byte_memset(out, len);
                        } else {
                            out = chunkcopysafe(out, from, len, safe);
                        }
                    }
#else
                    from = window;
                    if (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        if (op < len) {         /* some from window */
                            len -= op;
                            do {
                                *out++ = *from++;
                            } while (--op);
                            from = out - dist;  /* rest from output */
                        }
                    } else if (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        if (op < len) {         /* some from end of window */
                            len -= op;
                            do {
                                *out++ = *from++;
                            } while (--op);
                            from = window;
                            if (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                do {
                                    *out++ = *from++;
                                } while (--op);
                                from = out - dist;      /* rest from output */
                            }
                        }
                    } else {                      /* contiguous in window */
                        from += wnext - op;
                        if (op < len) {         /* some from window */
                            len -= op;
                            do {
                                *out++ = *from++;
                            } while (--op);
                            from = out - dist;  /* rest from output */
                        }
                    }

                    out = chunk_copy(out, from, (int) (out - from), len);
#endif
                } else {
#ifdef INFFAST_CHUNKSIZE
                    if (dist == 1 && len >= sizeof(uint64_t)) {
                        out = byte_memset(out, len);
                    } else {
                        /* Whole reference is in range of current output.  No
                           range checks are necessary because we start with room
                           for at least 258 bytes of output, so unroll and roundoff
                           operations can write beyond `out+len` so long as they
                           stay within 258 bytes of `out`.
                         */
                        out = chunkunroll(out, &dist, &len);
                        out = chunkcopy(out, out - dist, len);
                    }
#else
                    if (len < sizeof(uint64_t))
                      out = set_bytes(out, out - dist, dist, len);
                    else if (dist == 1)
                      out = byte_memset(out, len);
                    else
                      out = chunk_memset(out, out - dist, dist, len);
#endif
                }
            } else if ((op & 64) == 0) {          /* 2nd level distance code */
                here = dcode[here.val + BITS(op)];
                goto dodist;
            } else {
                strm->msg = (char *)"invalid distance code";
                state->mode = BAD;
                break;
            }
        } else if ((op & 64) == 0) {              /* 2nd level length code */
            here = lcode[here.val + BITS(op)];
            goto dolen;
        } else if (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->mode = TYPE;
            break;
        } else {
            strm->msg = (char *)"invalid literal/length code";
            state->mode = BAD;
            break;
        }
    } while (in < last && out < end);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = bits >> 3;
    in -= len;
    bits -= len << 3;
    hold &= (1U << bits) - 1;

    /* update state and return */
    strm->next_in = (z_const unsigned char *)in;
    strm->next_out = out;
    strm->avail_in =
        (uint32_t)(in < last ? (INFLATE_FAST_MIN_HAVE - 1) + (last - in)
                             : (INFLATE_FAST_MIN_HAVE - 1) - (in - last));
    strm->avail_out =
        (unsigned)(out < end ? (INFLATE_FAST_MIN_LEFT - 1) + (end - out)
                             : (INFLATE_FAST_MIN_LEFT - 1) - (out - end));
    state->hold = hold;
    state->bits = bits;
    return;
}

/*
   inflate_fast() speedups that turned out slower (on a PowerPC G3 750CXe):
   - Using bit fields for code structure
   - Different op definition to avoid & for extra bits (do & for table bits)
   - Three separate decoding do-loops for direct, window, and wnext == 0
   - Special case for distance > 1 copies to do overlapped load and store copy
   - Explicit branch predictions (based on measured branch probabilities)
   - Deferring match copy and interspersed it with decoding subsequent codes
   - Swapping literal/length else
   - Swapping window/direct else
   - Larger unrolled copy loops (three is about right)
   - Moving len -= 3 statement into middle of loop
 */