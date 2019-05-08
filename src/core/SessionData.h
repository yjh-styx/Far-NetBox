//---------------------------------------------------------------------------
#pragma once

#include <Common.h>
#include <FileBuffer.h>

#include "Option.h"
#include "NamedObjs.h"
#include "HierarchicalStorage.h"
#include "Configuration.h"
#include <Xml.XMLIntf.hpp>
//---------------------------------------------------------------------------
#define SET_SESSION_PROPERTY(Property) \
  if (F##Property != Value) { F##Property = Value; Modify(); }
//---------------------------------------------------------------------------
enum TCipher { cipWarn, cip3DES, cipBlowfish, cipAES, cipDES, cipArcfour, cipChaCha20, };
#define CIPHER_COUNT (cipChaCha20 + 1)
// explicit values to skip obsoleted fsExternalSSH, fsExternalSFTP
enum TFSProtocol { fsSCPonly = 0, fsSFTP = 1, fsSFTPonly = 2, fsFTP = 5, fsWebDAV = 6, fsS3 = 7, };
#define FSPROTOCOL_COUNT (fsS3+1)
extern const wchar_t *ProxyMethodNames;
enum TProxyMethod { pmNone, pmSocks4, pmSocks5, pmHTTP, pmTelnet, pmCmd, pmSystemOld, pmSystem = 100, };
enum TSshProt { ssh1only, ssh1deprecated, ssh2deprecated, ssh2only, };
enum TKex { kexWarn, kexDHGroup1, kexDHGroup14, kexDHGEx, kexRSA, kexECDH, };
#define KEX_COUNT (kexECDH + 1)
enum THostKey { hkWarn, hkRSA, hkDSA, hkECDSA, hkED25519, hkMax, };
#define HOSTKEY_COUNT (hkMax)
enum TGssLib { gssGssApi32, gssSspi, gssCustom, };
#define GSSLIB_COUNT (gssCustom + 1)
// names have to match PuTTY registry entries (see settings.c)
enum TSshBug { sbIgnore1, sbPlainPW1, sbRSA1, sbHMAC2, sbDeriveKey2, sbRSAPad2,
  sbPKSessID2, sbRekey2, sbMaxPkt2, sbIgnore2, sbOldGex2, sbWinAdj, sbChanReq, };
#define BUG_COUNT (sbChanReq + 1)
enum TSftpBug { sbSymlink, sbSignedTS, };
#define SFTP_BUG_COUNT (sbSignedTS + 1)
extern const wchar_t *PingTypeNames;
enum TPingType { ptOff, ptNullPacket, ptDummyCommand, };
enum TAddressFamily { afAuto, afIPv4, afIPv6, };
enum TFtps { ftpsNone, ftpsImplicit, ftpsExplicitSsl, ftpsExplicitTls, };

// has to match SSL_VERSION_XXX constants in AsyncSslSocketLayer.h
enum TTlsVersion { ssl2 = 2, ssl3 = 3, tls10 = 10, tls11 = 11, tls12 = 12 };
enum TSessionSource { ssNone, ssStored, ssStoredModified };
enum TSessionUrlFlags
{
  sufSpecific = 0x01,
  sufUserName = 0x02,
  sufPassword = 0x04,
  sufHostKey = 0x08,
  sufRawSettings = 0x10,
  sufHttpForWebDAV = 0x20,
  sufSession = sufUserName | sufPassword | sufHostKey,
  sufComplete = sufSession | sufRawSettings,
  sufOpen = sufUserName | sufPassword,
};
enum TParseUrlFlags
{
  pufAllowStoredSiteWithProtocol = 0x01,
};

enum TFSProtocol_219
{
  fsFTPS_219 = 6,
  fsHTTP_219 = 7,
  fsHTTPS_219 = 8,
};


enum TLoginType
{
  ltAnonymous = 0,
  ltNormal = 1,
};
//---------------------------------------------------------------------------
NB_CORE_EXPORT extern const UnicodeString CipherNames[CIPHER_COUNT];
NB_CORE_EXPORT extern const UnicodeString KexNames[KEX_COUNT];
NB_CORE_EXPORT extern const UnicodeString HostKeyNames[HOSTKEY_COUNT];
NB_CORE_EXPORT extern const UnicodeString GssLibNames[GSSLIB_COUNT];
NB_CORE_EXPORT extern const wchar_t SshProtList[][10];
NB_CORE_EXPORT extern const TCipher DefaultCipherList[CIPHER_COUNT];
NB_CORE_EXPORT extern const TKex DefaultKexList[KEX_COUNT];
NB_CORE_EXPORT extern const THostKey DefaultHostKeyList[HOSTKEY_COUNT];
NB_CORE_EXPORT extern const TGssLib DefaultGssLibList[GSSLIB_COUNT];
NB_CORE_EXPORT extern const wchar_t FSProtocolNames[FSPROTOCOL_COUNT][16];
NB_CORE_EXPORT extern const intptr_t DefaultSendBuf;
NB_CORE_EXPORT extern const UnicodeString AnonymousUserName;
NB_CORE_EXPORT extern const UnicodeString AnonymousPassword;
NB_CORE_EXPORT extern const intptr_t SshPortNumber;
NB_CORE_EXPORT extern const intptr_t FtpPortNumber;
NB_CORE_EXPORT extern const intptr_t FtpsImplicitPortNumber;
NB_CORE_EXPORT extern const intptr_t HTTPPortNumber;
NB_CORE_EXPORT extern const intptr_t HTTPSPortNumber;
NB_CORE_EXPORT extern const intptr_t TelnetPortNumber;
NB_CORE_EXPORT extern const intptr_t ProxyPortNumber;
NB_CORE_EXPORT extern const UnicodeString PuttySshProtocol;
NB_CORE_EXPORT extern const UnicodeString PuttyTelnetProtocol;
NB_CORE_EXPORT extern const UnicodeString SftpProtocol;
NB_CORE_EXPORT extern const UnicodeString ScpProtocol;
NB_CORE_EXPORT extern const UnicodeString FtpProtocol;
NB_CORE_EXPORT extern const UnicodeString FtpsProtocol;
NB_CORE_EXPORT extern const UnicodeString FtpesProtocol;
NB_CORE_EXPORT extern const UnicodeString WebDAVProtocol;
NB_CORE_EXPORT extern const UnicodeString WebDAVSProtocol;
NB_CORE_EXPORT extern const UnicodeString S3Protocol;
NB_CORE_EXPORT extern const UnicodeString SshProtocol;
NB_CORE_EXPORT extern const UnicodeString WinSCPProtocolPrefix;
NB_CORE_EXPORT extern const wchar_t UrlParamSeparator;
NB_CORE_EXPORT extern const wchar_t UrlParamValueSeparator;
NB_CORE_EXPORT extern const UnicodeString UrlHostKeyParamName;
NB_CORE_EXPORT extern const UnicodeString UrlSaveParamName;
NB_CORE_EXPORT extern const UnicodeString PassphraseOption;
NB_CORE_EXPORT extern const UnicodeString S3HostName;

constexpr intptr_t SFTPMinVersion = 0;
constexpr intptr_t SFTPMaxVersion = 6;
//---------------------------------------------------------------------------
struct NB_CORE_EXPORT TIEProxyConfig : public TObject
{
  TIEProxyConfig() = default;
  bool AutoDetect{false}; // not used
  UnicodeString AutoConfigUrl; // not used
  UnicodeString Proxy; //< string in format "http=host:80;https=host:443;ftp=ftpproxy:20;socks=socksproxy:1080"
  UnicodeString ProxyBypass; //< string in format "*.local, foo.com, google.com"
  UnicodeString ProxyHost;
  intptr_t ProxyPort{0};
  TProxyMethod ProxyMethod{pmNone};
};
//---------------------------------------------------------------------------
class TStoredSessionList;
//---------------------------------------------------------------------------
NB_DEFINE_CLASS_ID(TSessionData);
class NB_CORE_EXPORT TSessionData : public TNamedObject
{
  friend class TStoredSessionList;
  NB_DISABLE_COPY(TSessionData)
public:
  static bool classof(const TObject *Obj) { return Obj->is(OBJECT_CLASS_TSessionData); }
  bool is(TObjectClassId Kind) const override { return (Kind == OBJECT_CLASS_TSessionData) || TNamedObject::is(Kind); }
private:
  UnicodeString FHostName;
  intptr_t FPortNumber{0};
  UnicodeString FUserName;
  RawByteString FPassword;
  RawByteString FNewPassword;
  bool FChangePassword{false};
  intptr_t FPingInterval{0};
  TPingType FPingType{ptOff};
  bool FTryAgent{false};
  bool FAgentFwd{false};
  UnicodeString FListingCommand;
  bool FAuthTIS{false};
  bool FAuthKI{false};
  bool FAuthKIPassword{false};
  bool FAuthGSSAPI{false};
  bool FGSSAPIFwdTGT{false};
  bool FChangeUsername{false};
  bool FCompression{false};
  TSshProt FSshProt{};
  bool FSsh2DES{false};
  bool FSshNoUserAuth{false};
  TCipher FCiphers[CIPHER_COUNT]{};
  TKex FKex[KEX_COUNT]{};
  THostKey FHostKeys[HOSTKEY_COUNT]{};
  TGssLib FGssLib[GSSLIB_COUNT]{};
  UnicodeString FGssLibCustom;
  bool FClearAliases{false};
  TEOLType FEOLType{};
  bool FTrimVMSVersions{false};
  UnicodeString FPublicKeyFile;
  UnicodeString FPassphrase;
  UnicodeString FPuttyProtocol;
  TFSProtocol FFSProtocol;
  bool FModified{false};
  UnicodeString FLocalDirectory;
  UnicodeString FRemoteDirectory;
  bool FLockInHome{false};
  bool FSpecial{false};
  bool FSynchronizeBrowsing{false};
  bool FUpdateDirectories{false};
  bool FCacheDirectories{false};
  bool FCacheDirectoryChanges{false};
  bool FPreserveDirectoryChanges{false};
  bool FSelected{false};
  TAutoSwitch FLookupUserGroups{asOn};
  UnicodeString FReturnVar;
  bool FExitCode1IsError{false};
  bool FScp1Compatibility{false};
  UnicodeString FShell;
  UnicodeString FSftpServer;
  intptr_t FTimeout{0};
  bool FUnsetNationalVars{false};
  bool FIgnoreLsWarnings{false};
  bool FTcpNoDelay{false};
  intptr_t FSendBuf{0};
  bool FSshSimple{false};
  TProxyMethod FProxyMethod{pmNone};
  UnicodeString FProxyHost;
  intptr_t FProxyPort{0};
  UnicodeString FProxyUsername;
  RawByteString FProxyPassword;
  UnicodeString FProxyTelnetCommand;
  UnicodeString FProxyLocalCommand;
  TAutoSwitch FProxyDNS{};
  bool FProxyLocalhost{false};
  intptr_t FFtpProxyLogonType{0};
  TAutoSwitch FBugs[BUG_COUNT]{};
  UnicodeString FCustomParam1;
  UnicodeString FCustomParam2;
  bool FResolveSymlinks{false};
  bool FFollowDirectorySymlinks{false};
  TDateTime FTimeDifference;
  bool FTimeDifferenceAuto{false};
  intptr_t FSFTPDownloadQueue{0};
  intptr_t FSFTPUploadQueue{0};
  intptr_t FSFTPListingQueue{0};
  intptr_t FSFTPMaxVersion{0};
  intptr_t FSFTPMaxPacketSize{0};
  TDSTMode FDSTMode{dstmKeep};
  TAutoSwitch FSFTPBugs[SFTP_BUG_COUNT]{};
  bool FDeleteToRecycleBin{false};
  bool FOverwrittenToRecycleBin{false};
  UnicodeString FRecycleBinPath;
  UnicodeString FPostLoginCommands;
  TAutoSwitch FSCPLsFullTime{};
  TAutoSwitch FFtpListAll{};
  TAutoSwitch FFtpHost{};
  TAutoSwitch FFtpDeleteFromCwd{};
  bool FSslSessionReuse{false};
  UnicodeString FTlsCertificateFile;
  TAddressFamily FAddressFamily{afAuto};
  UnicodeString FRekeyData;
  uintptr_t FRekeyTime{0};
  intptr_t FColor{0};
  bool FTunnel{false};
  UnicodeString FTunnelHostName;
  intptr_t FTunnelPortNumber{0};
  UnicodeString FTunnelUserName;
  RawByteString FTunnelPassword;
  UnicodeString FTunnelPublicKeyFile;
  intptr_t FTunnelLocalPortNumber;
  UnicodeString FTunnelPortFwd;
  UnicodeString FTunnelHostKey;
  bool FFtpPasvMode{false};
  TAutoSwitch FFtpForcePasvIp{};
  TAutoSwitch FFtpUseMlsd{};
  UnicodeString FFtpAccount;
  intptr_t FFtpPingInterval{0};
  TPingType FFtpPingType{};
  TAutoSwitch FFtpTransferActiveImmediately{};
  TFtps FFtps{};
  TTlsVersion FMinTlsVersion{};
  TTlsVersion FMaxTlsVersion{};
  TAutoSwitch FNotUtf{};
  intptr_t FInternalEditorEncoding{0};
  UnicodeString FS3DefaultRegion;
  bool FIsWorkspace{false};
  UnicodeString FLink;
  UnicodeString FNameOverride;
  UnicodeString FHostKey;
  bool FFingerprintScan{false};
  bool FOverrideCachedHostKey{false};
  UnicodeString FNote;
  UnicodeString FWinTitle;
  RawByteString FEncryptKey;

  UnicodeString FOrigHostName;
  intptr_t FOrigPortNumber{0};
  TProxyMethod FOrigProxyMethod{pmNone};
  TSessionSource FSource{ssNone};
  bool FSaveOnly{false};
  UnicodeString FLogicalHostName;

public:
  void SetHostName(UnicodeString Value);
  UnicodeString GetHostNameExpanded() const;
  UnicodeString GetHostNameExpanded() const;
  void SetPortNumber(intptr_t value);
  void SessionSetUserName(UnicodeString value);
  UnicodeString GetUserNameExpanded() const;
  UnicodeString __fastcall GetPassword() const;
  void __fastcall SetNewPassword(UnicodeString value);
  UnicodeString __fastcall GetNewPassword() const;
  void __fastcall SetChangePassword(bool value);
  void __fastcall SetPingInterval(int value);
  void SetPingInterval(intptr_t value);
  void __fastcall SetTryAgent(bool value);
  void __fastcall SetAgentFwd(bool value);
  void __fastcall SetAuthTIS(bool value);
  void __fastcall SetAuthKI(bool value);
  void __fastcall SetAuthKIPassword(bool value);
  void __fastcall SetAuthGSSAPI(bool value);
  void __fastcall SetGSSAPIFwdTGT(bool value);
  void __fastcall SetChangeUsername(bool value);
  void __fastcall SetCompression(bool value);
  void __fastcall SetSshProt(TSshProt value);
  void __fastcall SetSsh2DES(bool value);
  void __fastcall SetSshNoUserAuth(bool value);
  void __fastcall SetCipher(int Index, TCipher value);
  void SetCipher(intptr_t Index, TCipher value);
  TCipher GetCipher(intptr_t Index) const;
  void SetKex(intptr_t Index, TKex value);
  TKex GetKex(intptr_t Index) const;
  void SetHostKeys(intptr_t Index, THostKey value);
  THostKey GetHostKeys(intptr_t Index) const;
  void SetGssLib(intptr_t Index, TGssLib value);
  TGssLib GetGssLib(intptr_t Index) const;


  void __fastcall SetPuttyProtocol(UnicodeString value);
  bool GetCanLogin() const;
  void __fastcall SetPingIntervalDT(TDateTime value);
  TDateTime GetPingIntervalDT() const;
  TDateTime GetFtpPingIntervalDT() const;
  void __fastcall SetTimeDifference(TDateTime value);
  void __fastcall SetTimeDifferenceAuto(bool value);
  void __fastcall SetPingType(TPingType value);
  UnicodeString GetSessionName() const;
  UnicodeString GetDefaultSessionName() const;
  UnicodeString GetProtocolUrl(bool HttpForWebDAV) const;
  void __fastcall SetFSProtocol(TFSProtocol value);
  UnicodeString GetFSProtocolStr() const;
  void __fastcall SetLocalDirectory(UnicodeString value);
  void __fastcall SetRemoteDirectory(UnicodeString value);
  void __fastcall SetSynchronizeBrowsing(bool value);
  void __fastcall SetUpdateDirectories(bool value);
  void __fastcall SetCacheDirectories(bool value);
  void __fastcall SetCacheDirectoryChanges(bool value);
  void __fastcall SetPreserveDirectoryChanges(bool value);
  void __fastcall SetLockInHome(bool value);
  void __fastcall SetSpecial(bool value);
  UnicodeString GetInfoTip() const;
  bool GetDefaultShell() const;
  void __fastcall SetDetectReturnVar(bool value);
  bool GetDetectReturnVar() const;
  void __fastcall SetListingCommand(UnicodeString value);
  void __fastcall SetClearAliases(bool value);
  void __fastcall SetDefaultShell(bool value);
  void __fastcall SetEOLType(TEOLType value);
  void __fastcall SetTrimVMSVersions(bool value);
  void __fastcall SetLookupUserGroups(TAutoSwitch value);
  void __fastcall SetReturnVar(UnicodeString value);
  void __fastcall SetExitCode1IsError(bool value);
  void __fastcall SetScp1Compatibility(bool value);
  void __fastcall SetShell(UnicodeString value);
  void __fastcall SetSftpServer(UnicodeString value);
  void SetTimeout(intptr_t value);
  void __fastcall SetUnsetNationalVars(bool value);
  void __fastcall SetIgnoreLsWarnings(bool value);
  void __fastcall SetTcpNoDelay(bool value);
  void SetSendBuf(intptr_t value);
  void __fastcall SetSshSimple(bool value);
  UnicodeString GetSshProtStr() const;
  bool GetUsesSsh() const;
  void __fastcall SetCipherList(UnicodeString value);
  UnicodeString __fastcall GetCipherList() const;
  void __fastcall SetKexList(UnicodeString value);
  UnicodeString __fastcall GetKexList() const;
  void __fastcall SetHostKeyList(UnicodeString value);
  UnicodeString __fastcall GetHostKeyList() const;
  void __fastcall SetGssLibList(UnicodeString value);
  UnicodeString __fastcall GetGssLibList() const;
  void __fastcall SetProxyMethod(TProxyMethod value);
  void __fastcall SetProxyHost(UnicodeString value);
  void SetProxyPort(intptr_t value);
  void __fastcall SetProxyUsername(UnicodeString value);
  void __fastcall SetProxyPassword(UnicodeString value);
  void __fastcall SetProxyTelnetCommand(UnicodeString value);
  void __fastcall SetProxyLocalCommand(UnicodeString value);
  void __fastcall SetProxyDNS(TAutoSwitch value);
  void __fastcall SetProxyLocalhost(bool value);
  UnicodeString __fastcall GetProxyPassword() const;
  void SetFtpProxyLogonType(intptr_t value);
  void __fastcall SetBug(TSshBug Bug, TAutoSwitch value);
  TAutoSwitch __fastcall GetBug(TSshBug Bug) const;
  UnicodeString GetSessionKey() const;
  void __fastcall SetCustomParam1(UnicodeString value);
  void __fastcall SetCustomParam2(UnicodeString value);
  void __fastcall SetResolveSymlinks(bool value);
  void __fastcall SetFollowDirectorySymlinks(bool value);
  void SetSFTPDownloadQueue(intptr_t value);
  void SetSFTPUploadQueue(intptr_t value);
  void SetSFTPListingQueue(intptr_t value);
  void SetSFTPMaxVersion(intptr_t value);
  void SetSFTPMaxPacketSize(intptr_t value);
  void __fastcall SetSFTPBug(TSftpBug Bug, TAutoSwitch value);
  TAutoSwitch __fastcall GetSFTPBug(TSftpBug Bug) const;
  void __fastcall SetSCPLsFullTime(TAutoSwitch value);
  void __fastcall SetFtpListAll(TAutoSwitch value);
  void __fastcall SetFtpHost(TAutoSwitch value);
  void __fastcall SetFtpDeleteFromCwd(TAutoSwitch value);
  void __fastcall SetSslSessionReuse(bool value);
  void __fastcall SetTlsCertificateFile(UnicodeString value);
  UnicodeString GetStorageKey() const;
  UnicodeString GetInternalStorageKey() const;
  UnicodeString GetSiteKey() const;
  void __fastcall SetDSTMode(TDSTMode value);
  void __fastcall SetDeleteToRecycleBin(bool value);
  void __fastcall SetOverwrittenToRecycleBin(bool value);
  void __fastcall SetRecycleBinPath(UnicodeString value);
  void __fastcall SetPostLoginCommands(UnicodeString value);
  void __fastcall SetAddressFamily(TAddressFamily value);
  void __fastcall SetRekeyData(UnicodeString value);
  void SetRekeyTime(uintptr_t value);
  void SetColor(intptr_t value);
  void __fastcall SetTunnel(bool value);
  void __fastcall SetTunnelHostName(UnicodeString value);
  void SetTunnelPortNumber(intptr_t value);
  void __fastcall SetTunnelUserName(UnicodeString value);
  void __fastcall SetTunnelPassword(UnicodeString value);
  UnicodeString __fastcall GetTunnelPassword() const;
  void __fastcall SetTunnelPublicKeyFile(UnicodeString value);
  void __fastcall SetTunnelPortFwd(UnicodeString value);
  void SetTunnelLocalPortNumber(intptr_t value);
  bool GetTunnelAutoassignLocalPortNumber() const;
  void __fastcall SetTunnelHostKey(UnicodeString value);
  void __fastcall SetFtpPasvMode(bool value);
  void __fastcall SetFtpForcePasvIp(TAutoSwitch value);
  void __fastcall SetFtpUseMlsd(TAutoSwitch value);
  void __fastcall SetFtpAccount(UnicodeString value);
  void SetFtpPingInterval(intptr_t value);
  void __fastcall SetFtpPingType(TPingType value);
  void __fastcall SetFtpTransferActiveImmediately(TAutoSwitch value);
  void __fastcall SetFtps(TFtps value);
  void __fastcall SetMinTlsVersion(TTlsVersion value);
  void __fastcall SetMaxTlsVersion(TTlsVersion value);
  void __fastcall SetNotUtf(TAutoSwitch value);
  void SetInternalEditorEncoding(intptr_t value);
  void __fastcall SetS3DefaultRegion(UnicodeString value);
  void __fastcall SetLogicalHostName(UnicodeString value);
  void __fastcall SetIsWorkspace(bool value);
  void __fastcall SetLink(UnicodeString value);
  void __fastcall SetNameOverride(UnicodeString value);
  void __fastcall SetHostKey(UnicodeString value);
  void __fastcall SetNote(UnicodeString value);
  void __fastcall SetWinTitle(UnicodeString value);
  UnicodeString __fastcall GetEncryptKey() const;
  void __fastcall SetEncryptKey(UnicodeString value);

  TDateTime GetTimeoutDT() const;
  void __fastcall SavePasswords(THierarchicalStorage * Storage, bool PuttyExport, bool DoNotEncryptPasswords);
  UnicodeString GetLocalName() const;
  UnicodeString GetFolderName() const;
  void __fastcall Modify();
  UnicodeString GetSource() const;
  void __fastcall DoLoad(THierarchicalStorage * Storage, bool PuttyImport, bool & RewritePassword);
  void __fastcall DoSave(THierarchicalStorage * Storage,
    bool PuttyExport, const TSessionData *Default, bool DoNotEncryptPasswords);
#if 0
  UnicodeString ReadXmlNode(_di_IXMLNode Node, UnicodeString Name, UnicodeString Default);
  int ReadXmlNode(_di_IXMLNode Node, UnicodeString Name, int Default);
  _di_IXMLNode FindSettingsNode(_di_IXMLNode Node, UnicodeString Name);
  UnicodeString ReadSettingsNode(_di_IXMLNode Node, UnicodeString Name, UnicodeString Default);
  int ReadSettingsNode(_di_IXMLNode Node, UnicodeString Name, int Default);
#endif // #if 0
  bool IsSame(const TSessionData * Default, bool AdvancedOnly, TStrings * DifferentProperties) const;
  UnicodeString GetNameWithoutHiddenPrefix() const;
  bool HasStateData() const;
  UnicodeString __fastcall GetNormalizedPuttyProtocol() const;
  static RawByteString __fastcall EncryptPassword(const UnicodeString & Password, UnicodeString Key);
  static UnicodeString __fastcall DecryptPassword(const RawByteString & Password, UnicodeString Key);
  static RawByteString EncryptPassword(UnicodeString Password, UnicodeString Key);
  static UnicodeString DecryptPassword(RawByteString Password, UnicodeString Key);
  static RawByteString StronglyRecryptPassword(RawByteString Password, UnicodeString Key);
  static bool DoIsProtocolUrl(UnicodeString AUrl, UnicodeString AProtocol, intptr_t& ProtocolLen);
  static bool IsProtocolUrl(UnicodeString AUrl, UnicodeString Protocol, intptr_t& ProtocolLen);
  static void AddSwitch(UnicodeString &Result, UnicodeString Name, UnicodeString Value);
  static void AddSwitch(UnicodeString &Result, UnicodeString Switch);
  static void AddSwitch(UnicodeString &Result, UnicodeString Name, intptr_t Value, bool Rtf);
  static void AddSwitch(UnicodeString &Result, UnicodeString AName, intptr_t Value);
#if 0
    UnicodeString &Result, TAssemblyLanguage Language,
    UnicodeString &Name, UnicodeString &Value);
  static void __fastcall AddAssemblyProperty(
    UnicodeString &Result, TAssemblyLanguage Language,
    UnicodeString &Name, UnicodeString &Type,
    UnicodeString &Member);
  static void __fastcall AddAssemblyProperty(
    UnicodeString &Result, TAssemblyLanguage Language,
    UnicodeString &Name, int Value);
  void __fastcall AddAssemblyProperty(
    UnicodeString &Result, TAssemblyLanguage Language,
    UnicodeString &Name, bool Value);
#endif // #if 0
  void __fastcall ApplyRawSettings(TStrings * RawSettings);
  TStrings * __fastcall GetRawSettingsForUrl();
  template<class AlgoT>
  void __fastcall SetAlgoList(AlgoT * List, const AlgoT * DefaultList, const UnicodeString * Names,
    intptr_t Count, AlgoT WarnAlgo, UnicodeString AValue);
  static void Remove(THierarchicalStorage * Storage, UnicodeString Name);

  __property UnicodeString InternalStorageKey = { read = GetInternalStorageKey };

public:
  TSessionData() = delete;
  explicit TSessionData(UnicodeString aName) noexcept;
  virtual ~TSessionData() noexcept;
  TSessionData * Clone() const;
  void __fastcall NonPersistant();
  void NonPersistent();
  void __fastcall ApplyRawSettings(THierarchicalStorage * Storage);
  void __fastcall ImportFromFilezilla(_di_IXMLNode Node, const UnicodeString & Path, _di_IXMLNode SettingsNode);
  __removed void ImportFromFilezilla(_di_IXMLNode Node, UnicodeString Path, _di_IXMLNode SettingsNode);
    const TSessionData *Default = nullptr);
  void __fastcall SaveRecryptedPasswords(THierarchicalStorage * Storage);
  void __fastcall RecryptPasswords();
  bool HasPassword() const;
  bool HasAnySessionPassword() const;
  bool HasAnyPassword() const;
  void __fastcall ClearSessionPasswords();
  void __fastcall MaskPasswords();
  void __fastcall Remove();
  void __fastcall CacheHostKeyIfNotCached();
  virtual void Assign(const TPersistent * Source);
  intptr_t Compare(const TNamedObject * Other) const override;
  void CopyData(const TSessionData * Source);
  void __fastcall CopyDirectoriesStateData(TSessionData * SourceData);
  bool __fastcall ParseUrl(UnicodeString Url, TOptions * Options,
    TStoredSessionList *AStoredSessions, bool &DefaultsOnly,
    UnicodeString *AFileName, bool *AProtocolDefined, UnicodeString *MaskedUrl, intptr_t Flags);
  bool __fastcall ParseOptions(TOptions * Options);
  void ConfigureTunnel(intptr_t PortNumber);
  void __fastcall RollbackTunnel();
  void __fastcall ExpandEnvironmentVariables();
  void DisableAuthenticationsExceptPassword();
  bool IsSame(const TSessionData * Default, bool AdvancedOnly) const;
  bool IsSameSite(const TSessionData * Default) const;
  bool IsInFolderOrWorkspace(UnicodeString Name) const;
  UnicodeString GenerateSessionUrl(uintptr_t Flags) const;
  bool __fastcall HasRawSettingsForUrl();
  bool HasSessionName() const;

  UnicodeString GenerateOpenCommandArgs(bool Rtf) const;
  __removed void GenerateAssemblyCode(TAssemblyLanguage Language, UnicodeString & Head, UnicodeString & Tail, int & Indent);
  void __fastcall LookupLastFingerprint();
  bool GetIsSecure() const;
  static void ValidatePath(UnicodeString Path);
  static void ValidateName(UnicodeString Name);
  static UnicodeString MakeValidName(UnicodeString Name);
  static UnicodeString ExtractLocalName(UnicodeString Name);
  static UnicodeString ExtractFolderName(UnicodeString Name);
  static UnicodeString ComposePath(UnicodeString Path, UnicodeString Name);
  static bool IsSensitiveOption(UnicodeString Option);
  static bool IsOptionWithParameters(UnicodeString Option);
  static bool MaskPasswordInOptionParameter(UnicodeString Option, UnicodeString & Param);
  static UnicodeString FormatSiteKey(UnicodeString HostName, intptr_t PortNumber);

  __property UnicodeString HostName  = { read = FHostName, write = SetHostName };
  RWProperty<UnicodeString> HostName{nb::bind(&TSessionData::GetHostName, this), nb::bind(&TSessionData::SetHostName, this)};
  __property UnicodeString HostNameExpanded  = { read = GetHostNameExpanded };
  ROProperty<UnicodeString> HostNameExpanded{nb::bind(&TSessionData::GetHostNameExpanded, this)};
  __property int PortNumber = { read = FPortNumber, write = SetPortNumber };
  RWProperty<intptr_t> PortNumber{nb::bind(&TSessionData::GetPortNumber, this), nb::bind(&TSessionData::SetPortNumber, this)};
  __property UnicodeString UserName = { read = FUserName, write = SessionSetUserName };
  RWProperty<UnicodeString> UserName{nb::bind(&TSessionData::SessionGetUserName, this), nb::bind(&TSessionData::SessionSetUserName, this)};
  __property UnicodeString UserNameExpanded  = { read = GetUserNameExpanded };
  __property UnicodeString Password  = { read = GetPassword, write = SetPassword };
  RWProperty<UnicodeString> Password{nb::bind(&TSessionData::GetPassword, this), nb::bind(&TSessionData::SetPassword, this)};
  __property UnicodeString NewPassword  = { read = GetNewPassword, write = SetNewPassword };
  __property bool ChangePassword  = { read = FChangePassword, write = SetChangePassword };
  __property int PingInterval  = { read = FPingInterval, write = SetPingInterval };
  __property bool TryAgent  = { read = FTryAgent, write = SetTryAgent };
  RWProperty<bool> TryAgent{nb::bind(&TSessionData::GetTryAgent, this), nb::bind(&TSessionData::SetTryAgent, this)};
  __property bool AgentFwd  = { read = FAgentFwd, write = SetAgentFwd };
  RWProperty<bool> AgentFwd{nb::bind(&TSessionData::GetAgentFwd, this), nb::bind(&TSessionData::SetAgentFwd, this)};
  __property UnicodeString ListingCommand = { read = FListingCommand, write = SetListingCommand };
  __property bool AuthTIS  = { read = FAuthTIS, write = SetAuthTIS };
  __property bool AuthKI  = { read = FAuthKI, write = SetAuthKI };
  __property bool AuthKIPassword  = { read = FAuthKIPassword, write = SetAuthKIPassword };
  __property bool AuthGSSAPI  = { read = FAuthGSSAPI, write = SetAuthGSSAPI };
  __property bool GSSAPIFwdTGT = { read = FGSSAPIFwdTGT, write = SetGSSAPIFwdTGT };
  __property bool ChangeUsername  = { read = FChangeUsername, write = SetChangeUsername };
  __property bool Compression  = { read = FCompression, write = SetCompression };
  RWProperty<bool> Compression{nb::bind(&TSessionData::GetCompression, this), nb::bind(&TSessionData::SetCompression, this)};
  __property TSshProt SshProt  = { read = FSshProt, write = SetSshProt };
  RWProperty<TSshProt> SshProt{nb::bind(&TSessionData::GetSshProt, this), nb::bind(&TSessionData::SetSshProt, this)};
  __property bool UsesSsh = { read = GetUsesSsh };
  __property bool Ssh2DES  = { read = FSsh2DES, write = SetSsh2DES };
  __property bool SshNoUserAuth  = { read = FSshNoUserAuth, write = SetSshNoUserAuth };
  __property TCipher Cipher[int Index] = { read = GetCipher, write = SetCipher };
  __property TKex Kex[int Index] = { read = GetKex, write = SetKex };
  __property THostKey HostKeys[int Index] = { read = GetHostKeys, write = SetHostKeys };
  __property TGssLib GssLib[int Index] = { read = GetGssLib, write = SetGssLib };
  __property UnicodeString GssLibCustom = { read = FGssLibCustom, write = SetGssLibCustom };
  __property UnicodeString PublicKeyFile  = { read = FPublicKeyFile, write = SetPublicKeyFile };
  RWProperty<UnicodeString> PublicKeyFile{nb::bind(&TSessionData::GetPublicKeyFile, this), nb::bind(&TSessionData::SetPublicKeyFile, this)};
  __property UnicodeString Passphrase  = { read = GetPassphrase, write = SetPassphrase };
  RWProperty<UnicodeString> Passphrase{nb::bind(&TSessionData::GetPassphrase, this), nb::bind(&TSessionData::SetPassphrase, this)};
  __property UnicodeString PuttyProtocol  = { read = FPuttyProtocol, write = SetPuttyProtocol };
  RWProperty<UnicodeString> PuttyProtoco{nb::bind(&TSessionData::GetPuttyProtocol, this), nb::bind(&TSessionData::SetPuttyProtocol, this)};
  __property TFSProtocol FSProtocol = { read = FFSProtocol, write = SetFSProtocol  };
  RWProperty<TFSProtocol> FSProtocol{nb::bind(&TSessionData::GetFSProtocol, this), nb::bind(&TSessionData::SetFSProtocol, this)};
  __property UnicodeString FSProtocolStr  = { read = GetFSProtocolStr };
  ROProperty<UnicodeString> FSProtocolStr{nb::bind(&TSessionData::GetFSProtocolStr, this)};
  __property bool Modified  = { read = FModified, write = FModified };
  __property bool CanLogin  = { read = GetCanLogin };
  __property bool ClearAliases = { read = FClearAliases, write = SetClearAliases };
  __property TDateTime PingIntervalDT = { read = GetPingIntervalDT, write = SetPingIntervalDT };
  __property TDateTime TimeDifference = { read = FTimeDifference, write = SetTimeDifference };
  __property bool TimeDifferenceAuto = { read = FTimeDifferenceAuto, write = SetTimeDifferenceAuto };
  __property TPingType PingType = { read = FPingType, write = SetPingType };
  __property UnicodeString SessionName  = { read = GetSessionName };
  ROProperty<UnicodeString> SessionName{nb::bind(&TSessionData::GetSessionName, this)};
  __property UnicodeString DefaultSessionName  = { read = GetDefaultSessionName };
  ROProperty<UnicodeString> DefaultSessionName{nb::bind(&TSessionData::GetDefaultSessionName, this)};
  __property UnicodeString LocalDirectory  = { read = FLocalDirectory, write = SetLocalDirectory };
  __property UnicodeString RemoteDirectory  = { read = FRemoteDirectory, write = SetRemoteDirectory };
  RWProperty<UnicodeString> RemoteDirectory{nb::bind(&TSessionData::GetRemoteDirectory, this), nb::bind(&TSessionData::SetRemoteDirectory, this)};
  __property bool SynchronizeBrowsing = { read = FSynchronizeBrowsing, write = SetSynchronizeBrowsing };
  __property bool UpdateDirectories = { read = FUpdateDirectories, write = SetUpdateDirectories };
  RWProperty<bool> UpdateDirectories{nb::bind(&TSessionData::GetUpdateDirectories, this), nb::bind(&TSessionData::SetUpdateDirectories, this)};
  __property bool CacheDirectories = { read = FCacheDirectories, write = SetCacheDirectories };
  __property bool CacheDirectoryChanges = { read = FCacheDirectoryChanges, write = SetCacheDirectoryChanges };
  __property bool PreserveDirectoryChanges = { read = FPreserveDirectoryChanges, write = SetPreserveDirectoryChanges };
  __property bool LockInHome = { read = FLockInHome, write = SetLockInHome };
  __property bool Special = { read = FSpecial, write = SetSpecial };
  __property bool Selected  = { read = FSelected, write = FSelected };
  bool& Selected{FSelected};
  __property UnicodeString InfoTip  = { read = GetInfoTip };
  __property bool DefaultShell = { read = GetDefaultShell, write = SetDefaultShell };
  __property bool DetectReturnVar = { read = GetDetectReturnVar, write = SetDetectReturnVar };
  __property TEOLType EOLType = { read = FEOLType, write = SetEOLType };
  __property bool TrimVMSVersions = { read = FTrimVMSVersions, write = SetTrimVMSVersions };
  __property TAutoSwitch LookupUserGroups = { read = FLookupUserGroups, write = SetLookupUserGroups };
  __property UnicodeString ReturnVar = { read = FReturnVar, write = SetReturnVar };
  __property bool ExitCode1IsError = { read = FExitCode1IsError, write = SetExitCode1IsError };
  __property bool Scp1Compatibility = { read = FScp1Compatibility, write = SetScp1Compatibility };
  __property UnicodeString Shell = { read = FShell, write = SetShell };
  __property UnicodeString SftpServer = { read = FSftpServer, write = SetSftpServer };
  __property int Timeout = { read = FTimeout, write = SetTimeout };
  __property TDateTime TimeoutDT = { read = GetTimeoutDT };
  __property bool UnsetNationalVars = { read = FUnsetNationalVars, write = SetUnsetNationalVars };
  __property bool IgnoreLsWarnings  = { read = FIgnoreLsWarnings, write = SetIgnoreLsWarnings };
  __property bool TcpNoDelay  = { read = FTcpNoDelay, write = SetTcpNoDelay };
  __property int SendBuf  = { read = FSendBuf, write = SetSendBuf };
  __property bool SshSimple  = { read = FSshSimple, write = SetSshSimple };
  __property UnicodeString SshProtStr  = { read = GetSshProtStr };
  __property UnicodeString CipherList  = { read = GetCipherList, write = SetCipherList };
  __property UnicodeString KexList  = { read = GetKexList, write = SetKexList };
  __property UnicodeString HostKeyList  = { read = GetHostKeyList, write = SetHostKeyList };
  __property UnicodeString GssLibList  = { read = GetGssLibList, write = SetGssLibList };
  __property TProxyMethod ProxyMethod  = { read = FProxyMethod, write = SetProxyMethod };
  __property UnicodeString ProxyHost  = { read = FProxyHost, write = SetProxyHost };
  __property int ProxyPort  = { read = FProxyPort, write = SetProxyPort };
  __property UnicodeString ProxyUsername  = { read = FProxyUsername, write = SetProxyUsername };
  __property UnicodeString ProxyPassword  = { read = GetProxyPassword, write = SetProxyPassword };
  __property UnicodeString ProxyTelnetCommand  = { read = FProxyTelnetCommand, write = SetProxyTelnetCommand };
  __property UnicodeString ProxyLocalCommand  = { read = FProxyLocalCommand, write = SetProxyLocalCommand };
  __property TAutoSwitch ProxyDNS  = { read = FProxyDNS, write = SetProxyDNS };
  __property bool ProxyLocalhost  = { read = FProxyLocalhost, write = SetProxyLocalhost };
  __property int FtpProxyLogonType  = { read = FFtpProxyLogonType, write = SetFtpProxyLogonType };
  __property TAutoSwitch Bug[TSshBug Bug]  = { read = GetBug, write = SetBug };
  __property UnicodeString CustomParam1 = { read = FCustomParam1, write = SetCustomParam1 };
  __property UnicodeString CustomParam2 = { read = FCustomParam2, write = SetCustomParam2 };
  __property UnicodeString SessionKey = { read = GetSessionKey };
  __property bool ResolveSymlinks = { read = FResolveSymlinks, write = SetResolveSymlinks };
  __property bool FollowDirectorySymlinks = { read = FFollowDirectorySymlinks, write = SetFollowDirectorySymlinks };
  __property int SFTPDownloadQueue = { read = FSFTPDownloadQueue, write = SetSFTPDownloadQueue };
  __property int SFTPUploadQueue = { read = FSFTPUploadQueue, write = SetSFTPUploadQueue };
  __property int SFTPListingQueue = { read = FSFTPListingQueue, write = SetSFTPListingQueue };
  __property int SFTPMaxVersion = { read = FSFTPMaxVersion, write = SetSFTPMaxVersion };
  __property uintptr_t SFTPMaxPacketSize = { read = FSFTPMaxPacketSize, write = SetSFTPMaxPacketSize };
  __property TAutoSwitch SFTPBug[TSftpBug Bug]  = { read = GetSFTPBug, write = SetSFTPBug };
  __property TAutoSwitch SCPLsFullTime = { read = FSCPLsFullTime, write = SetSCPLsFullTime };
  __property TAutoSwitch FtpListAll = { read = FFtpListAll, write = SetFtpListAll };
  __property TAutoSwitch FtpHost = { read = FFtpHost, write = SetFtpHost };
  __property TAutoSwitch FtpDeleteFromCwd = { read = FFtpDeleteFromCwd, write = SetFtpDeleteFromCwd };
  __property bool SslSessionReuse = { read = FSslSessionReuse, write = SetSslSessionReuse };
  __property UnicodeString TlsCertificateFile = { read = FTlsCertificateFile, write = SetTlsCertificateFile };
  RWProperty<UnicodeString> TlsCertificateFile{nb::bind(&TSessionData::GetTlsCertificateFile, this), nb::bind(&TSessionData::SetTlsCertificateFile, this)};
  __property TDSTMode DSTMode = { read = FDSTMode, write = SetDSTMode };
  __property bool DeleteToRecycleBin = { read = FDeleteToRecycleBin, write = SetDeleteToRecycleBin };
  __property bool OverwrittenToRecycleBin = { read = FOverwrittenToRecycleBin, write = SetOverwrittenToRecycleBin };
  __property UnicodeString RecycleBinPath = { read = FRecycleBinPath, write = SetRecycleBinPath };
  __property UnicodeString PostLoginCommands = { read = FPostLoginCommands, write = SetPostLoginCommands };
  __property TAddressFamily AddressFamily = { read = FAddressFamily, write = SetAddressFamily };
  RWProperty<TAddressFamily> AddressFamily{nb::bind(&TSessionData::GetAddressFamily, this), nb::bind(&TSessionData::SetAddressFamily, this)};
  __property UnicodeString RekeyData = { read = FRekeyData, write = SetRekeyData };
  __property uintptr_t RekeyTime = { read = FRekeyTime, write = SetRekeyTime };
  __property int Color = { read = FColor, write = SetColor };
  __property bool Tunnel = { read = FTunnel, write = SetTunnel };
  __property UnicodeString TunnelHostName = { read = FTunnelHostName, write = SetTunnelHostName };
  __property int TunnelPortNumber = { read = FTunnelPortNumber, write = SetTunnelPortNumber };
  __property UnicodeString TunnelUserName = { read = FTunnelUserName, write = SetTunnelUserName };
  __property UnicodeString TunnelPassword = { read = GetTunnelPassword, write = SetTunnelPassword };
  __property UnicodeString TunnelPublicKeyFile = { read = FTunnelPublicKeyFile, write = SetTunnelPublicKeyFile };
  __property bool TunnelAutoassignLocalPortNumber = { read = GetTunnelAutoassignLocalPortNumber };
  __property int TunnelLocalPortNumber = { read = FTunnelLocalPortNumber, write = SetTunnelLocalPortNumber };
  __property UnicodeString TunnelPortFwd = { read = FTunnelPortFwd, write = SetTunnelPortFwd };
  __property UnicodeString TunnelHostKey = { read = FTunnelHostKey, write = SetTunnelHostKey };
  __property bool FtpPasvMode = { read = FFtpPasvMode, write = SetFtpPasvMode };
  __property TAutoSwitch FtpForcePasvIp = { read = FFtpForcePasvIp, write = SetFtpForcePasvIp };
  __property TAutoSwitch FtpUseMlsd = { read = FFtpUseMlsd, write = SetFtpUseMlsd };
  __property UnicodeString FtpAccount = { read = FFtpAccount, write = SetFtpAccount };
  __property int FtpPingInterval  = { read = FFtpPingInterval, write = SetFtpPingInterval };
  __property TDateTime FtpPingIntervalDT  = { read = GetFtpPingIntervalDT };
  __property TPingType FtpPingType = { read = FFtpPingType, write = SetFtpPingType };
  __property TAutoSwitch FtpTransferActiveImmediately = { read = FFtpTransferActiveImmediately, write = SetFtpTransferActiveImmediately };
  __property TFtps Ftps = { read = FFtps, write = SetFtps };
  RWProperty<TFtps> Ftps{nb::bind(&TSessionData::GetFtps, this), nb::bind(&TSessionData::SetFtps, this)};
  __property TTlsVersion MinTlsVersion = { read = FMinTlsVersion, write = SetMinTlsVersion };
  __property TTlsVersion MaxTlsVersion = { read = FMaxTlsVersion, write = SetMaxTlsVersion };
  __property UnicodeString LogicalHostName = { read = FLogicalHostName, write = SetLogicalHostName };
  RWProperty<UnicodeString> LogicalHostName{nb::bind(&TSessionData::GetLogicalHostName, this), nb::bind(&TSessionData::SetLogicalHostName, this)};
  __property TAutoSwitch NotUtf = { read = FNotUtf, write = SetNotUtf };
  __property int InternalEditorEncoding = { read = FInternalEditorEncoding, write = SetInternalEditorEncoding };
  __property UnicodeString S3DefaultRegion = { read = FS3DefaultRegion, write = SetS3DefaultRegion };
  RWProperty<UnicodeString> S3DefaultRegion{nb::bind(&TSessionData::GetS3DefaultRegion, this), nb::bind(&TSessionData::SetS3DefaultRegion, this)};
  __property bool IsWorkspace = { read = FIsWorkspace, write = SetIsWorkspace };
  __property UnicodeString Link = { read = FLink, write = SetLink };
  __property UnicodeString NameOverride = { read = FNameOverride, write = SetNameOverride };
  RWProperty<UnicodeString> NameOverride{nb::bind(&TSessionData::GetNameOverride, this), nb::bind(&TSessionData::SetNameOverride, this)};
  __property UnicodeString HostKey = { read = FHostKey, write = SetHostKey };
  RWProperty<UnicodeString> HostKey{nb::bind(&TSessionData::GetHostKey, this), nb::bind(&TSessionData::SetHostKey, this)};
  __property bool FingerprintScan = { read = FFingerprintScan, write = FFingerprintScan };
  __property bool OverrideCachedHostKey = { read = FOverrideCachedHostKey };
  __property UnicodeString Note = { read = FNote, write = SetNote };
  __property UnicodeString WinTitle = { read = FWinTitle, write = SetWinTitle };
  __property UnicodeString EncryptKey = { read = GetEncryptKey, write = SetEncryptKey };
  RWProperty<UnicodeString> EncryptKey{nb::bind(&TSessionData::GetEncryptKey, this), nb::bind(&TSessionData::SetEncryptKey, this)};

  __property UnicodeString StorageKey = { read = GetStorageKey };
  ROProperty<UnicodeString> StorageKey{nb::bind(&TSessionData::GetStorageKey, this)};
  __property UnicodeString SiteKey = { read = GetSiteKey };
  ROProperty<UnicodeString> SiteKey{nb::bind(&TSessionData::GetSiteKey, this)};
  __property UnicodeString OrigHostName = { read = FOrigHostName };
  const UnicodeString& OrigHostName{FOrigHostName};
  __property int OrigPortNumber = { read = FOrigPortNumber };
  const intptr_t& OrigPortNumber{FOrigPortNumber};
  __property UnicodeString LocalName = { read = GetLocalName };
  ROProperty<UnicodeString> LocalName{nb::bind(&TSessionData::GetLocalName, this)};
  __property UnicodeString FolderName = { read = GetFolderName };
  ROProperty<UnicodeString> FolderName{nb::bind(&TSessionData::GetFolderName, this)};
  __property UnicodeString Source = { read = GetSource };
  ROProperty<UnicodeString> Source{nb::bind(&TSessionData::GetSource, this)};
  __property bool SaveOnly = { read = FSaveOnly };
  const bool& SaveOnly{FSaveOnly};

public:
  void SetSFTPMinPacketSize(intptr_t Value);
  void SetFingerprintScan(bool Value) { FFingerprintScan = Value; }
  bool GetSaveOnly() const { return FSaveOnly; }
  bool GetChangePassword() const { return FChangePassword; }
  UnicodeString GetGssLibCustom() const { return FGssLibCustom; }
  void SetFtpDupFF(bool Value);
  void SetFtpUndupFF(bool Value);
  UnicodeString GetWinTitle() const { return FWinTitle; }

  bool GetTimeDifferenceAuto() const { return FTimeDifferenceAuto; }
  UnicodeString GetNote() const { return FNote; }
  UnicodeString GetProtocolStr() const;
  void SetProtocolStr(UnicodeString Value);

  UnicodeString GetHostName() const { return FHostName; }
  intptr_t GetPortNumber() const { return FPortNumber; }
  TLoginType GetLoginType() const;
  void SetLoginType(TLoginType Value);
  UnicodeString SessionGetUserName() const { return FUserName; }
  intptr_t GetPingInterval() const { return FPingInterval; }
  bool GetTryAgent() const { return FTryAgent; }
  bool GetAgentFwd() const { return FAgentFwd; }
  UnicodeString GetListingCommand() const { return FListingCommand; }
  bool GetAuthTIS() const { return FAuthTIS; }
  bool GetAuthKI() const { return FAuthKI; }
  bool GetAuthKIPassword() const { return FAuthKIPassword; }
  bool GetAuthGSSAPI() const { return FAuthGSSAPI; }
  bool GetGSSAPIFwdTGT() const { return FGSSAPIFwdTGT; }
  bool GetChangeUsername() const { return FChangeUsername; }
  bool GetCompression() const { return FCompression; }
  TSshProt GetSshProt() const { return FSshProt; }
  bool GetSsh2DES() const { return FSsh2DES; }
  bool GetSshNoUserAuth() const { return FSshNoUserAuth; }
  UnicodeString GetPublicKeyFile() const { return FPublicKeyFile; }
  UnicodeString GetPuttyProtocol() const { return FPuttyProtocol; }
  TFSProtocol GetFSProtocol() const { return FFSProtocol; }
  bool GetModified() const { return FModified; }
  void SetModified(bool Value) { FModified = Value; }
  bool GetClearAliases() const { return FClearAliases; }
  TDateTime GetTimeDifference() const { return FTimeDifference; }
  TPingType GetPingType() const { return FPingType; }
  UnicodeString GetLocalDirectory() const { return FLocalDirectory; }
  UnicodeString GetRemoteDirectory() const { return FRemoteDirectory; }
  bool GetSynchronizeBrowsing() const { return FSynchronizeBrowsing; }
  bool GetUpdateDirectories() const { return FUpdateDirectories; }
  bool GetCacheDirectories() const { return FCacheDirectories; }
  bool GetCacheDirectoryChanges() const { return FCacheDirectoryChanges; }
  bool GetPreserveDirectoryChanges() const { return FPreserveDirectoryChanges; }
  bool GetLockInHome() const { return FLockInHome; }
  bool GetSpecial() const { return FSpecial; }
  bool GetSelected() const { return FSelected; }
  void SetSelected(bool Value) { FSelected = Value; }
  TEOLType GetEOLType() const { return FEOLType; }
  bool GetTrimVMSVersions() const { return FTrimVMSVersions; }
  TAutoSwitch GetLookupUserGroups() const { return FLookupUserGroups; }
  UnicodeString GetReturnVar() const { return FReturnVar; }
  bool GetExitCode1IsError() const { return FExitCode1IsError; }
  bool GetScp1Compatibility() const { return FScp1Compatibility; }
  UnicodeString GetShell() const { return FShell; }
  UnicodeString GetSftpServer() const { return FSftpServer; }
  intptr_t GetTimeout() const { return FTimeout; }
  bool GetUnsetNationalVars() const { return FUnsetNationalVars; }
  bool GetIgnoreLsWarnings() const { return FIgnoreLsWarnings; }
  bool GetTcpNoDelay() const { return FTcpNoDelay; }
  intptr_t GetSendBuf() const { return FSendBuf; }
  bool GetSshSimple() const { return FSshSimple; }
  TProxyMethod GetProxyMethod() const { return FProxyMethod; }
  TProxyMethod GetActualProxyMethod() const;
  UnicodeString GetProxyHost() const;
  intptr_t GetProxyPort() const;
  UnicodeString GetProxyUsername() const;
  UnicodeString GetProxyTelnetCommand() const { return FProxyTelnetCommand; }
  UnicodeString GetProxyLocalCommand() const { return FProxyLocalCommand; }
  TAutoSwitch GetProxyDNS() const { return FProxyDNS; }
  bool GetProxyLocalhost() const { return FProxyLocalhost; }
  intptr_t GetFtpProxyLogonType() const { return FFtpProxyLogonType; }
  UnicodeString GetCustomParam1() const { return FCustomParam1; }
  UnicodeString GetCustomParam2() const { return FCustomParam2; }
  bool GetResolveSymlinks() const { return FResolveSymlinks; }
  bool GetFollowDirectorySymlinks() const { return FFollowDirectorySymlinks; }
  intptr_t GetSFTPDownloadQueue() const { return FSFTPDownloadQueue; }
  intptr_t GetSFTPUploadQueue() const { return FSFTPUploadQueue; }
  intptr_t GetSFTPListingQueue() const { return FSFTPListingQueue; }
  intptr_t GetSFTPMaxVersion() const { return FSFTPMaxVersion; }
  intptr_t GetSFTPMinPacketSize() const { return FSFTPMinPacketSize; }
  intptr_t GetSFTPMaxPacketSize() const { return FSFTPMaxPacketSize; }
  TAutoSwitch GetSCPLsFullTime() const { return FSCPLsFullTime; }
  TAutoSwitch GetFtpListAll() const { return FFtpListAll; }
  TAutoSwitch GetFtpHost() const { return FFtpHost; }
  bool GetFtpDupFF() const { return FFtpDupFF; }
  bool GetFtpUndupFF() const { return FFtpUndupFF; }
  TAutoSwitch GetFtpDeleteFromCwd() const { return FFtpDeleteFromCwd; }
  bool GetSslSessionReuse() const { return FSslSessionReuse; }
  UnicodeString GetTlsCertificateFile() const { return FTlsCertificateFile; }
  TDSTMode GetDSTMode() const { return FDSTMode; }
  bool GetDeleteToRecycleBin() const { return FDeleteToRecycleBin; }
  bool GetOverwrittenToRecycleBin() const { return FOverwrittenToRecycleBin; }
  UnicodeString GetRecycleBinPath() const { return FRecycleBinPath; }
  UnicodeString GetPostLoginCommands() const { return FPostLoginCommands; }
  TAddressFamily GetAddressFamily() const { return FAddressFamily; }
  UnicodeString GetCodePage() const { return FCodePage; }
  void SetCodePage(UnicodeString Value);
  uintptr_t GetCodePageAsNumber() const;
  UnicodeString GetRekeyData() const { return FRekeyData; }
  uintptr_t GetRekeyTime() const { return FRekeyTime; }
  intptr_t GetColor() const { return FColor; }
  bool GetTunnel() const { return FTunnel; }
  UnicodeString GetTunnelHostName() const { return FTunnelHostName; }
  intptr_t GetTunnelPortNumber() const { return FTunnelPortNumber; }
  UnicodeString GetTunnelUserName() const { return FTunnelUserName; }
  UnicodeString GetTunnelPublicKeyFile() const { return FTunnelPublicKeyFile; }
  intptr_t GetTunnelLocalPortNumber() const { return FTunnelLocalPortNumber; }
  UnicodeString GetTunnelPortFwd() const { return FTunnelPortFwd; }
  UnicodeString GetTunnelHostKey() const { return FTunnelHostKey; }
  bool GetFtpPasvMode() const { return FFtpPasvMode; }
  bool GetFtpAllowEmptyPassword() const { return FFtpAllowEmptyPassword; }
  void SetFtpAllowEmptyPassword(bool Value);
  TAutoSwitch GetFtpForcePasvIp() const { return FFtpForcePasvIp; }
  TAutoSwitch GetFtpUseMlsd() const { return FFtpUseMlsd; }
  UnicodeString GetFtpAccount() const { return FFtpAccount; }
  intptr_t GetFtpPingInterval() const { return FFtpPingInterval; }
  TPingType GetFtpPingType() const { return FFtpPingType; }
  TAutoSwitch GetFtpTransferActiveImmediately() const { return FFtpTransferActiveImmediately; }
  TFtps GetFtps() const { return FFtps; }
  TTlsVersion GetMinTlsVersion() const { return FMinTlsVersion; }
  TTlsVersion GetMaxTlsVersion() const { return FMaxTlsVersion; }
  TAutoSwitch GetNotUtf() const { return FNotUtf; }
  intptr_t GetInternalEditorEncoding() const { return FInternalEditorEncoding; }
  UnicodeString GetS3DefaultRegion() const { return FS3DefaultRegion; }
  bool GetIsWorkspace() const { return FIsWorkspace; }
  UnicodeString GetLink() const { return FLink; }
  UnicodeString GetNameOverride() const { return FNameOverride; }
  UnicodeString GetHostKey() const { return FHostKey; }
  bool GetFingerprintScan() const { return FFingerprintScan; }
  bool GetOverrideCachedHostKey() const { return FOverrideCachedHostKey; }
  UnicodeString GetOrigHostName() const { return FOrigHostName; }
  UnicodeString GetLogicalHostName() const { return FLogicalHostName; }
  intptr_t GetOrigPortNumber() const { return FOrigPortNumber; }
  void SetPasswordless(bool Value);

  intptr_t GetNumberOfRetries() const { return FNumberOfRetries; }
  void SetNumberOfRetries(intptr_t Value) { FNumberOfRetries = Value; }
  uintptr_t GetSessionVersion() const { return FSessionVersion; }
  void SetSessionVersion(uintptr_t Value) { FSessionVersion = Value; }
  void RemoveProtocolPrefix(UnicodeString &HostName) const;
  static void AddSwitchValue(UnicodeString &Result, UnicodeString Name, UnicodeString Value);

private:
  uintptr_t GetDefaultVersion() const { return ::GetCurrentVersionNumber(); }
  TFSProtocol TranslateFSProtocolNumber(intptr_t FSProtocol);
  TFSProtocol TranslateFSProtocol(UnicodeString ProtocolID) const;
  TFtps TranslateFtpEncryptionNumber(intptr_t FtpEncryption) const;

  TProxyMethod GetSystemProxyMethod() const;
  void PrepareProxyData() const;
  void ParseIEProxyConfig() const;
  void FromURI(UnicodeString ProxyURI,
    UnicodeString &ProxyUrl, intptr_t &ProxyPort, TProxyMethod &ProxyMethod) const;
  void AdjustHostName(UnicodeString &HostName, UnicodeString Prefix) const;

private:
  intptr_t FSFTPMinPacketSize{0};
  bool FFtpDupFF{false};
  bool FFtpUndupFF{false};
  bool FTunnelConfigured{false};
  UnicodeString FCodePage;
  mutable uintptr_t FCodePageAsNumber{0};
  bool FFtpAllowEmptyPassword{false};
  TLoginType FLoginType{};
  intptr_t FNumberOfRetries{0};
  uintptr_t FSessionVersion{0};

  mutable TIEProxyConfig *FIEProxyConfig{nullptr};
};
//---------------------------------------------------------------------------
NB_DEFINE_CLASS_ID(TStoredSessionList);
class NB_CORE_EXPORT TStoredSessionList : public TNamedObjectList
{
  NB_DISABLE_COPY(TStoredSessionList)
public:
  TStoredSessionList() noexcept;
  explicit TStoredSessionList(bool aReadOnly) noexcept;
  void __fastcall Save(bool All, bool Explicit);
  void __fastcall Saved();
  void __fastcall ImportFromFilezilla(const UnicodeString FileName, const UnicodeString ConfigurationFileName);
  void ImportFromFilezilla(UnicodeString FileName, UnicodeString ConfigurationFileName);
  void __fastcall Export(const UnicodeString FileName);
  void Export(UnicodeString FileName);
  void Load(UnicodeString AKey, bool UseDefaults);
    bool UseDefaults = false, bool PuttyImport = false);
  void __fastcall Save(THierarchicalStorage * Storage, bool All = false);
  void __fastcall SelectAll(bool Select);
  void __fastcall Import(TStoredSessionList * From, bool OnlySelected, TList * Imported);
  void __fastcall RecryptPasswords(TStrings * RecryptPasswordErrors);
  TSessionData * __fastcall AtSession(int Index)
  { return static_cast<TSessionData *>(AtObject(Index)); }
  void __fastcall SelectSessionsToImport(TStoredSessionList * Dest, bool SSHOnly);
  void __fastcall Cleanup();
  void __fastcall UpdateStaticUsage();
  intptr_t IndexOf(TSessionData * Data) const;
  const TSessionData * FindSame(TSessionData * Data);
  TSessionData * __fastcall NewSession(UnicodeString SessionName, TSessionData * Session);
  void __fastcall NewWorkspace(UnicodeString Name, TList * DataList);
  bool GetIsFolder(UnicodeString Name) const;
  bool GetIsWorkspace(UnicodeString Name) const;
  TSessionData * __fastcall ParseUrl(UnicodeString Url, TOptions * Options, bool & DefaultsOnly,
    UnicodeString *AFileName = nullptr, bool *AProtocolDefined = nullptr, UnicodeString *MaskedUrl = nullptr, intptr_t Flags = 0);
  bool __fastcall IsUrl(UnicodeString Url);
  bool __fastcall CanLogin(TSessionData * Data);
  void GetFolderOrWorkspace(UnicodeString Name, TList * List);
  TStrings * GetFolderOrWorkspaceList(UnicodeString Name);
  TStrings * GetWorkspaces() const;
  bool HasAnyWorkspace() const;
  TSessionData * __fastcall SaveWorkspaceData(TSessionData * Data, int Index);
  virtual ~TStoredSessionList() noexcept;
  __property TSessionData *Sessions[int Index]  = { read = AtSession };
  __property TSessionData *DefaultSettings  = { read = FDefaultSettings, write = SetDefaultSettings };
  RWProperty<const TSessionData *> DefaultSettings{nb::bind(&TStoredSessionList::GetDefaultSettingsConst, this), nb::bind(&TStoredSessionList::SetDefaultSettings, this)};

  static void __fastcall ImportHostKeys(
    THierarchicalStorage * SourceStorage, THierarchicalStorage * TargetStorage, TStoredSessionList * Sessions, bool OnlySelected);
  static void __fastcall ImportHostKeys(
    UnicodeString & SourceKey, TStoredSessionList * Sessions, bool OnlySelected);
  static void __fastcall ImportSelectedKnownHosts(TStoredSessionList * Sessions);
  static bool __fastcall OpenHostKeysSubKey(THierarchicalStorage * Storage, bool CanCreate);

  const TSessionData *GetSession(intptr_t Index) const { return dyn_cast<TSessionData>(AtObject(Index)); }
  TSessionData *GetSession(intptr_t Index) { return dyn_cast<TSessionData>(AtObject(Index)); }
  const TSessionData *GetDefaultSettingsConst() const { return FDefaultSettings.get(); }
  TSessionData *GetDefaultSettings() { return FDefaultSettings.get(); }
  void SetDefaultSettings(const TSessionData *Value);
  const TSessionData *GetSessionByName(UnicodeString SessionName) const;
private:
  std::unique_ptr<TSessionData> FDefaultSettings;
  bool FReadOnly{false};
  std::unique_ptr<TStrings> FPendingRemovals;
  __removed void SetDefaultSettings(TSessionData * value);
  void __fastcall DoSave(THierarchicalStorage * Storage, bool All,
    bool RecryptPasswordOnly, TStrings *RecryptPasswordErrors);
  void __fastcall DoSave(bool All, bool Explicit, bool RecryptPasswordOnly,
    TStrings *RecryptPasswordErrors);
  void __fastcall DoSave(THierarchicalStorage * Storage,
    TSessionData *Data, bool All, bool RecryptPasswordOnly,
    TSessionData *FactoryDefaults);
  TSessionData * __fastcall ResolveWorkspaceData(TSessionData * Data);
  bool IsFolderOrWorkspace(UnicodeString Name, bool Workspace) const;
  TSessionData * __fastcall CheckIsInFolderOrWorkspaceAndResolve(
    TSessionData *Data, UnicodeString Name);
  __removed void ImportLevelFromFilezilla(_di_IXMLNode Node, UnicodeString Path, _di_IXMLNode SettingsNode);
  static THierarchicalStorage * CreateHostKeysStorageForWriting();
};
//---------------------------------------------------------------------------
NB_CORE_EXPORT UnicodeString GetExpandedLogFileName(UnicodeString LogFileName, TDateTime Started, TSessionData *SessionData);
NB_CORE_EXPORT bool GetIsSshProtocol(TFSProtocol FSProtocol);
NB_CORE_EXPORT intptr_t GetDefaultPort(TFSProtocol FSProtocol, TFtps Ftps);
NB_CORE_EXPORT bool IsIPv6Literal(UnicodeString HostName);
NB_CORE_EXPORT UnicodeString EscapeIPv6Literal(UnicodeString IP);
NB_CORE_EXPORT TFSProtocol NormalizeFSProtocol(TFSProtocol FSProtocol);
//---------------------------------------------------------------------------
NB_CORE_EXPORT bool GetCodePageInfo(UINT CodePage, CPINFOEX &CodePageInfoEx);
NB_CORE_EXPORT uintptr_t GetCodePageAsNumber(UnicodeString CodePage);
NB_CORE_EXPORT UnicodeString GetCodePageAsString(uintptr_t CodePage);
//---------------------------------------------------------------------------
