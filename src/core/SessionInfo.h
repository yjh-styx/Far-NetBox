
#pragma once

#include <tinylog/TinyLog.h>
#include "SessionData.h"
#include "Interface.h"
//---------------------------------------------------------------------------
enum TSessionStatus
{
  ssClosed,
  ssOpening,
  ssOpened,
  ssClosing,
};
//---------------------------------------------------------------------------
struct NB_CORE_EXPORT TSessionInfo
{
  CUSTOM_MEM_ALLOCATION_IMPL
  TSessionInfo() noexcept;

  TDateTime LoginTime;
  UnicodeString ProtocolBaseName;
  UnicodeString ProtocolName;
  UnicodeString SecurityProtocolName;

  UnicodeString CSCipher;
  UnicodeString CSCompression;
  UnicodeString SCCipher;
  UnicodeString SCCompression;

  UnicodeString SshVersionString;
  UnicodeString SshImplementation;
  UnicodeString HostKeyFingerprintSHA256;
  UnicodeString HostKeyFingerprintMD5;

  UnicodeString CertificateFingerprint;
  UnicodeString Certificate;
  bool CertificateVerifiedManually{false};
};
//---------------------------------------------------------------------------
enum TFSCapability
{
  fcUserGroupListing = 0, fcModeChanging, fcGroupChanging,
  fcOwnerChanging, fcGroupOwnerChangingByID, fcAnyCommand, fcHardLink,
  fcSymbolicLink,
  // With WebDAV this is always true, to avoid double-click on
  // file try to open the file as directory. It does no harm atm as
  // WebDAV never produce a symlink in listing.
  fcResolveSymlink,
  fcTextMode, fcRename, fcNativeTextMode, fcNewerOnlyUpload, fcRemoteCopy,
  fcTimestampChanging, fcRemoteMove, fcLoadingAdditionalProperties,
  fcCheckingSpaceAvailable, fcIgnorePermErrors, fcCalculatingChecksum,
  fcModeChangingUpload, fcPreservingTimestampUpload, fcShellAnyCommand,
  fcSecondaryShell, fcRemoveCtrlZUpload, fcRemoveBOMUpload, fcMoveToQueue,
  fcLocking, fcPreservingTimestampDirs, fcResumeSupport,
  fcChangePassword, fsSkipTransfer, fsParallelTransfers, fsBackgroundTransfers,
  fcCount,
};
//---------------------------------------------------------------------------
struct NB_CORE_EXPORT TFileSystemInfo
{
  CUSTOM_MEM_ALLOCATION_IMPL
  TFileSystemInfo() noexcept;

  UnicodeString ProtocolBaseName;
  UnicodeString ProtocolName;
  UnicodeString RemoteSystem;
  UnicodeString AdditionalInfo;
  bool IsCapable[fcCount]{};
};
//---------------------------------------------------------------------------
NB_DEFINE_CLASS_ID(TSessionUI);
class NB_CORE_EXPORT TSessionUI : public TObject
{
public:
  static bool classof(const TObject *Obj) { return Obj->is(OBJECT_CLASS_TSessionUI); }
  bool is(TObjectClassId Kind) const override { return (Kind == OBJECT_CLASS_TSessionUI) || TObject::is(Kind); }
public:
  explicit TSessionUI(TObjectClassId Kind) noexcept : TObject(Kind) {}
  virtual ~TSessionUI() noexcept = default;
  virtual void Information(const UnicodeString AStr, bool Status) = 0;
  virtual uint32_t QueryUser(const UnicodeString AQuery,
    TStrings *MoreMessages, uint32_t Answers, const TQueryParams *Params,
    TQueryType QueryType = qtConfirmation) = 0;
  virtual uint32_t QueryUserException(const UnicodeString AQuery,
    Exception *E, uint32_t Answers, const TQueryParams *Params,
    TQueryType QueryType = qtConfirmation) = 0;
  virtual bool PromptUser(TSessionData *Data, TPromptKind Kind,
    const UnicodeString AName, const UnicodeString AInstructions, TStrings *Prompts,
    TStrings *Results) = 0;
  virtual void DisplayBanner(const UnicodeString ABanner) = 0;
  virtual void FatalError(Exception *E, const UnicodeString AMsg, const UnicodeString AHelpKeyword = L"") = 0;
  virtual void HandleExtendedException(Exception *E) = 0;
  virtual void Closed() = 0;
  virtual void ProcessGUI() = 0;
};
//---------------------------------------------------------------------------
enum TLogLineType
{
  llOutput,
  llInput,
  llStdError,
  llMessage,
  llException,
};

enum TLogAction
{
  laUpload, laDownload, laTouch, laChmod, laMkdir, laRm, laMv, laCp, laCall, laLs,
  laStat, laChecksum, laCwd, laDifference
};
//---------------------------------------------------------------------------
enum TCaptureOutputType { cotOutput, cotError, cotExitCode };
#if 0
typedef void (__closure *TCaptureOutputEvent)(
  const UnicodeString Str, TCaptureOutputType OutputType);
#endif // #if 0
using TCaptureOutputEvent = nb::FastDelegate2<void,
  UnicodeString /*Str*/, TCaptureOutputType /*OutputType*/>;
#if 0
typedef void (__closure *TCalculatedChecksumEvent)(
  const UnicodeString FileName, const UnicodeString Alg, const UnicodeString Hash);
#endif // #if 0
using TCalculatedChecksumEvent = nb::FastDelegate3<void,
  UnicodeString /*FileName*/, UnicodeString /*Alg*/, UnicodeString /*Hash*/>;
//---------------------------------------------------------------------------
class TSessionActionRecord;
class TActionLog;
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TSessionAction
{
  CUSTOM_MEM_ALLOCATION_IMPL
  NB_DISABLE_COPY(TSessionAction)
public:
  TSessionAction() = delete;
  explicit TSessionAction(TActionLog *Log, TLogAction Action) noexcept;
  virtual ~TSessionAction() noexcept;

  void Restart();

  void Commit();
  void Rollback(Exception *E = nullptr);
  void Cancel();

protected:
  TSessionActionRecord *FRecord{nullptr};
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TFileSessionAction : public TSessionAction
{
public:
  explicit TFileSessionAction(TActionLog *Log, TLogAction Action) noexcept;
  explicit TFileSessionAction(TActionLog *Log, TLogAction Action, const UnicodeString AFileName) noexcept;

  void SetFileName(const UnicodeString AFileName);
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TFileLocationSessionAction : public TFileSessionAction
{
public:
  TFileLocationSessionAction() = delete;
  explicit TFileLocationSessionAction(TActionLog *Log, TLogAction Action) noexcept;
  explicit TFileLocationSessionAction(TActionLog *Log, TLogAction Action, const UnicodeString AFileName) noexcept;

  void Destination(const UnicodeString Destination);
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TUploadSessionAction : public TFileLocationSessionAction
{
public:
  TUploadSessionAction() = delete;
  explicit TUploadSessionAction(TActionLog *Log) noexcept;
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TDownloadSessionAction : public TFileLocationSessionAction
{
public:
  TDownloadSessionAction() = delete;
  explicit TDownloadSessionAction(TActionLog *Log) noexcept;
};
//---------------------------------------------------------------------------
class TRights;
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TChmodSessionAction : public TFileSessionAction
{
public:
  explicit TChmodSessionAction(TActionLog *Log, const UnicodeString AFileName) noexcept;
  explicit TChmodSessionAction(TActionLog *Log, const UnicodeString AFileName,
    const TRights &ARights) noexcept;

  void Rights(const TRights &Rights);
  void Recursive();
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TTouchSessionAction : public TFileSessionAction
{
public:
  explicit TTouchSessionAction(TActionLog *Log, const UnicodeString AFileName,
    const TDateTime &Modification) noexcept;
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TMkdirSessionAction : public TFileSessionAction
{
public:
  explicit TMkdirSessionAction(TActionLog *Log, const UnicodeString AFileName) noexcept;
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TRmSessionAction : public TFileSessionAction
{
public:
  explicit TRmSessionAction(TActionLog *Log, const UnicodeString AFileName) noexcept;

  void Recursive();
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TMvSessionAction : public TFileLocationSessionAction
{
public:
  explicit TMvSessionAction(TActionLog *Log, const UnicodeString AFileName,
    const UnicodeString ADestination) noexcept;
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TCpSessionAction : public TFileLocationSessionAction
{
public:
  explicit TCpSessionAction(TActionLog * Log, const UnicodeString AFileName,
    const UnicodeString ADestination) noexcept;
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TCallSessionAction : public TSessionAction
{
public:
  explicit TCallSessionAction(TActionLog *Log, const UnicodeString Command,
    const UnicodeString ADestination) noexcept;

  void AddOutput(const UnicodeString Output, bool StdError);
  void ExitCode(int ExitCode);
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TLsSessionAction : public TSessionAction
{
public:
  explicit TLsSessionAction(TActionLog *Log, const UnicodeString Destination) noexcept;

  void FileList(TRemoteFileList *FileList);
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TStatSessionAction : public TFileSessionAction
{
public:
  explicit TStatSessionAction(TActionLog *Log, const UnicodeString AFileName) noexcept;

  void File(TRemoteFile *AFile);
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TChecksumSessionAction : public TFileSessionAction
{
public:
  explicit TChecksumSessionAction(TActionLog *Log) noexcept;

  void Checksum(const UnicodeString Alg, const UnicodeString Checksum);
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TCwdSessionAction : public TSessionAction
{
public:
  TCwdSessionAction(TActionLog *Log, const UnicodeString Path) noexcept;
};
//---------------------------------------------------------------------------
class TDifferenceSessionAction : public TSessionAction
{
public:
  explicit TDifferenceSessionAction(TActionLog * Log, const TChecklistItem* Item) noexcept;
};

using TDoAddLogEvent = nb::FastDelegate2<void,
  TLogLineType /*Type*/, UnicodeString /*Line*/>;
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TSessionLog
{
  CUSTOM_MEM_ALLOCATION_IMPL
  friend class TSessionAction;
  friend class TSessionActionRecord;
  NB_DISABLE_COPY(TSessionLog)
public:
  explicit TSessionLog(TSessionUI *UI, TDateTime Started, TSessionData *SessionData,
    TConfiguration *Configuration) noexcept;
  virtual ~TSessionLog() noexcept;

  void SetParent(TSessionLog *AParent, const UnicodeString AName);

  void Add(TLogLineType Type, const UnicodeString ALine);
  void AddSystemInfo();
  void AddStartupInfo();
  void AddException(Exception *E);
  void AddSeparator();

  void ReflectSettings();

  __property bool Logging = { read = FLogging };
  const bool& Logging{FLogging};
  __property UnicodeString Name = { read = FName };
  const UnicodeString& Name{FName};

  bool GetLogging() const { return FLogging; }
  UnicodeString GetName() const { return FName; }
  UnicodeString GetLogFileName() const { return FCurrentLogFileName; }
  bool LogToFile() const { return LogToFileProtected(); }

protected:
  void CloseLogFile();
  bool LogToFileProtected() const;

private:
  TConfiguration *FConfiguration{nullptr};
  TSessionLog *FParent{nullptr};
  TCriticalSection FCriticalSection;
  bool FLogging{false};
  std::unique_ptr<tinylog::TinyLog> FLogger;
  UnicodeString FCurrentLogFileName;
  UnicodeString FCurrentFileName;
  int64_t FCurrentFileSize{0};
  TSessionUI *FUI{nullptr};
  TSessionData *FSessionData{nullptr};
  TDateTime FStarted;
  UnicodeString FName;
  bool FClosed{false};

  void OpenLogFile();
  UnicodeString GetLogFileName();
  void DoAdd(TLogLineType AType, const UnicodeString ALine,
    TDoAddLogEvent Event);
  __removed void (__closure *f)(TLogLineType Type, const UnicodeString &Line);
  void DoAddToParent(TLogLineType AType, const UnicodeString ALine);
  void DoAddToSelf(TLogLineType AType, const UnicodeString ALine);
  void AddStartupInfo(bool System);
  void DoAddStartupInfo(TSessionData *Data);
  UnicodeString GetTlsVersionName(TTlsVersion TlsVersion) const;
  UnicodeString LogSensitive(const UnicodeString Str);
  void AddOption(const UnicodeString LogStr);
  void AddOptions(TOptions *Options);
  UnicodeString GetCmdLineLog() const;
  void CheckSize(int64_t Addition);
  UnicodeString LogPartFileName(const UnicodeString BaseName, intptr_t Index);

public:
  UnicodeString GetLine(intptr_t Index) const;
  TLogLineType GetType(intptr_t Index) const;
  void DeleteUnnecessary();
  void StateChange();
};
//---------------------------------------------------------------------------
class NB_CORE_EXPORT TActionLog : public TObject
{
friend class TSessionAction;
friend class TSessionActionRecord;
  NB_DISABLE_COPY(TActionLog)
public:
  explicit TActionLog(TSessionUI *UI, TDateTime Started, TSessionData *SessionData,
    TConfiguration *Configuration) noexcept;
  // For fatal failures for .NET assembly
  explicit TActionLog(TDateTime Started, TConfiguration *Configuration) noexcept;
  virtual ~TActionLog() noexcept;

  void ReflectSettings();
  void AddFailure(Exception *E);
  void AddFailure(TStrings *Messages);
  void BeginGroup(const UnicodeString Name);
  void EndGroup();

  __property UnicodeString CurrentFileName = { read = FCurrentFileName };
  const UnicodeString& CurrentFileName{FCurrentFileName};
  __property bool Enabled = { read = FEnabled, write = SetEnabled };
  RWProperty<bool> Enabled{nb::bind(&TActionLog::GetEnabled, this), nb::bind(&TActionLog::SetEnabled, this)};

  UnicodeString GetCurrentFileName() const { return FCurrentFileName; }
  bool GetEnabled() const { return FEnabled; }

protected:
  void CloseLogFile();
  inline void AddPendingAction(TSessionActionRecord *Action);
  void RecordPendingActions();
  void Add(const UnicodeString Line);
  void AddIndented(const UnicodeString ALine);
  void AddMessages(const UnicodeString Indent, TStrings *Messages);
  void Init(TSessionUI *UI, TDateTime Started, TSessionData *SessionData,
    TConfiguration *Configuration);

private:
  TConfiguration *FConfiguration{nullptr};
  TCriticalSection FCriticalSection;
  bool FLogging{false};
  std::unique_ptr<tinylog::TinyLog> FLogger;
  UnicodeString FCurrentLogFileName;
  UnicodeString FCurrentFileName;
  TSessionUI *FUI{nullptr};
  TSessionData *FSessionData{nullptr};
  TDateTime FStarted;
  std::unique_ptr<TList> FPendingActions{nullptr};
  bool FFailed{false};
  bool FClosed{false};
  bool FInGroup{false};
  UnicodeString FIndent;
  bool FEnabled{false};

  void OpenLogFile();
public:
  UnicodeString GetLogFileName() const { return FCurrentLogFileName; }
  void SetEnabled(bool Value);
};
//---------------------------------------------------------------------------
