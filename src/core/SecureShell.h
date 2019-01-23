
#pragma once

#include <rdestl/vector.h>
#include "PuttyIntf.h"
#include "Configuration.h"
#include "SessionData.h"
#include "SessionInfo.h"
//---------------------------------------------------------------------------
#ifndef PuttyIntfH
__removed struct Backend;
__removed struct Conf;
#endif
//---------------------------------------------------------------------------
struct _WSANETWORKEVENTS;
typedef struct _WSANETWORKEVENTS WSANETWORKEVENTS;
using SOCKET = UINT_PTR;
using TSockets = rde::vector<SOCKET>;
struct TPuttyTranslation;

enum TSshImplementation
{
  sshiUnknown,
  sshiOpenSSH,
  sshiProFTPD,
  sshiBitvise,
  sshiTitan,
  sshiOpenVMS,
  sshiCerberus,
};
//---------------------------------------------------------------------------
NB_DEFINE_CLASS_ID(TSecureShell);
class TSecureShell : public TObject
{
  friend class TPoolForDataEvent;
  NB_DISABLE_COPY(TSecureShell)
public:
  static bool classof(const TObject *Obj) { return Obj->is(OBJECT_CLASS_TSecureShell); }
  bool is(TObjectClassId Kind) const override { return (Kind == OBJECT_CLASS_TSecureShell) || TObject::is(Kind); }
private:
  SOCKET FSocket{INVALID_SOCKET};
  HANDLE FSocketEvent{};
  TSockets FPortFwdSockets;
  TSessionUI *FUI{nullptr};
  TSessionData *FSessionData{nullptr};
  bool FActive{false};
  mutable TSessionInfo FSessionInfo{};
  mutable bool FSessionInfoValid{false};
  TDateTime FLastDataSent{};
  Backend *FBackend{nullptr};
  void *FBackendHandle{nullptr};
  mutable const uint32_t *FMinPacketSize{nullptr};
  mutable const uint32_t *FMaxPacketSize{nullptr};
  TNotifyEvent FOnReceive{nullptr};
  bool FFrozen{false};
  bool FDataWhileFrozen{false};
  bool FStoredPasswordTried{false};
  bool FStoredPasswordTriedForKI{false};
  bool FStoredPassphraseTried{false};
  mutable int FSshVersion{0};
  bool FOpened{false};
  intptr_t FWaiting{0};
  bool FSimple{false};
  bool FNoConnectionResponse{false};
  bool FCollectPrivateKeyUsage{false};
  intptr_t FWaitingForData{0};
  TSshImplementation FSshImplementation{sshiUnknown};

  intptr_t PendLen{0};
  intptr_t PendSize{0};
  intptr_t OutLen{0};
  uint8_t *OutPtr{nullptr};
  uint8_t *Pending{nullptr};
  TSessionLog *FLog{nullptr};
  TConfiguration *FConfiguration{nullptr};
  bool FAuthenticating{false};
  bool FAuthenticated{false};
  UnicodeString FStdErrorTemp;
  UnicodeString FStdError;
  UnicodeString FCWriteTemp;
  UnicodeString FAuthenticationLog;
  UnicodeString FLastTunnelError;
  UnicodeString FUserName;
  bool FUtfStrings{false};
  DWORD FLastSendBufferUpdate{0};
  intptr_t FSendBuf{0};

public:
  static TCipher FuncToSsh1Cipher(const void *Cipher);
  static TCipher FuncToSsh2Cipher(const void *Cipher);
  UnicodeString FuncToCompression(int SshVersion, const void *Compress) const;
  void Init();
  void SetActive(bool Value);
  void inline CheckConnection(int Message = -1);
  void WaitForData();
  void Discard();
  void FreeBackend();
  void PoolForData(WSANETWORKEVENTS &Events, uint32_t &Result);
  void CaptureOutput(TLogLineType Type, UnicodeString Line);
  void ResetConnection();
  void ResetSessionInfo();
  void SocketEventSelect(SOCKET Socket, HANDLE Event, bool Startup);
  bool EnumNetworkEvents(SOCKET Socket, WSANETWORKEVENTS &Events);
  void HandleNetworkEvents(SOCKET Socket, WSANETWORKEVENTS &Events);
  bool ProcessNetworkEvents(SOCKET Socket);
  bool EventSelectLoop(uintptr_t MSec, bool ReadEventRequired,
    WSANETWORKEVENTS *Events);
  void UpdateSessionInfo() const;
  bool GetReady() const;
  void DispatchSendBuffer(intptr_t BufSize);
  void SendBuffer(uint32_t &Result);
  uintptr_t TimeoutPrompt(TQueryParamsTimerEvent PoolEvent);
  bool TryFtp();
  UnicodeString ConvertInput(RawByteString Input, uintptr_t CodePage = CP_ACP) const;
  void GetRealHost(UnicodeString &Host, intptr_t &Port) const;
  UnicodeString RetrieveHostKey(UnicodeString Host, intptr_t Port, UnicodeString KeyType) const;

protected:
  TCaptureOutputEvent FOnCaptureOutput;

  void GotHostKey();
  int TranslatePuttyMessage(const TPuttyTranslation *Translation,
    intptr_t Count, UnicodeString &Message, UnicodeString *HelpKeyword = nullptr) const;
  int TranslateAuthenticationMessage(UnicodeString &Message, UnicodeString *HelpKeyword = nullptr);
  int TranslateErrorMessage(UnicodeString &Message, UnicodeString *HelpKeyword = nullptr);
  void AddStdError(UnicodeString AStr);
  void AddStdErrorLine(UnicodeString AStr);
  void LogEvent(UnicodeString AStr);
  void FatalError(UnicodeString Error, UnicodeString HelpKeyword = "");
  UnicodeString FormatKeyStr(UnicodeString AKeyStr) const;
  static Conf *StoreToConfig(TSessionData *Data, bool Simple);

public:
  explicit TSecureShell(TSessionUI *UI, TSessionData *SessionData,
    TSessionLog *Log, TConfiguration *Configuration) noexcept;
  virtual ~TSecureShell() noexcept;
  void Open();
  void Close();
  void KeepAlive();
  intptr_t Receive(uint8_t *Buf, intptr_t Length);
  bool Peek(uint8_t *& Buf, intptr_t Length) const;
  UnicodeString ReceiveLine();
  void Send(const uint8_t *Buf, intptr_t Length);
  void SendSpecial(intptr_t Code);
  void Idle(uintptr_t MSec = 0);
  void SendEOF();
  void SendLine(UnicodeString Line);
  void SendNull();

  const TSessionInfo &GetSessionInfo() const;
  void GetHostKeyFingerprint(UnicodeString &SHA256, UnicodeString &MD5) const;
  bool SshFallbackCmd() const;
  uint32_t MinPacketSize() const;
  uint32_t MaxPacketSize() const;
  void ClearStdError();
  bool GetStoredCredentialsTried() const;
  void CollectUsage();
  bool CanChangePassword() const;

  void RegisterReceiveHandler(TNotifyEvent Handler);
  void UnregisterReceiveHandler(TNotifyEvent Handler);

  // interface to PuTTY core
  void UpdateSocket(SOCKET Value, bool Startup);
  void UpdatePortFwdSocket(SOCKET Value, bool Startup);
  void PuttyFatalError(UnicodeString AError);
  TPromptKind IdentifyPromptKind(UnicodeString &AName) const;
  bool PromptUser(bool ToServer,
    UnicodeString AName, bool NameRequired,
    UnicodeString AInstructions, bool InstructionsRequired,
    TStrings *Prompts, TStrings *Results);
  void FromBackend(bool IsStdErr, const uint8_t *Data, intptr_t Length);
  void CWrite(const char *Data, intptr_t Length);
  UnicodeString GetStdError() const;
  void VerifyHostKey(
    UnicodeString AHost, intptr_t Port, UnicodeString AKeyType, UnicodeString AKeyStr,
    UnicodeString AFingerprint);
  bool HaveHostKey(UnicodeString AHost, intptr_t Port, UnicodeString KeyType);
  void AskAlg(UnicodeString AAlgType, UnicodeString AlgName);
  void DisplayBanner(UnicodeString Banner);
  void OldKeyfileWarning();
  void PuttyLogEvent(const char *AStr);
  UnicodeString ConvertFromPutty(const char *Str, intptr_t Length) const;

  __property bool Active = { read = FActive, write = SetActive };
  __property bool Ready = { read = GetReady };
  __property TCaptureOutputEvent OnCaptureOutput = { read = FOnCaptureOutput, write = FOnCaptureOutput };
  __property TDateTime LastDataSent = { read = FLastDataSent };
  __property UnicodeString LastTunnelError = { read = FLastTunnelError };
  __property UnicodeString UserName = { read = FUserName };
  __property bool Simple = { read = FSimple, write = FSimple };
  __property TSshImplementation SshImplementation = { read = FSshImplementation };
  __property bool UtfStrings = { read = FUtfStrings, write = FUtfStrings };

  bool GetActive() const { return FActive; }
  const TCaptureOutputEvent GetOnCaptureOutput() const { return FOnCaptureOutput; }
  void SetOnCaptureOutput(TCaptureOutputEvent Value) { FOnCaptureOutput = Value; }
  TDateTime GetLastDataSent() const { return FLastDataSent; }
  UnicodeString GetLastTunnelError() const { return FLastTunnelError; }
  UnicodeString ShellGetUserName() const { return FUserName; }
  bool GetSimple() const { return FSimple; }
  void SetSimple(bool Value) { FSimple = Value; }
  TSshImplementation GetSshImplementation() const { return FSshImplementation; }
  bool GetUtfStrings() const { return FUtfStrings; }
  void SetUtfStrings(bool Value) { FUtfStrings = Value; }
};
//---------------------------------------------------------------------------
