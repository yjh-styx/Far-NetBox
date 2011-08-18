//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <Common.h>
#include <RemoteFiles.h>
#include <Terminal.h>
#include <DiscMon.hpp>
#include <Exceptions.h>
#include "GUIConfiguration.h"
#include "CoreMain.h"
#include "TextsCore.h"
#include "SynchronizeController.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TSynchronizeController::TSynchronizeController(
  TSynchronizeEvent AOnSynchronize, TSynchronizeInvalidEvent AOnSynchronizeInvalid,
  TSynchronizeTooManyDirectories AOnTooManyDirectories)
{
  FOnSynchronize = AOnSynchronize;
  FOnSynchronizeInvalid = AOnSynchronizeInvalid;
  FOnTooManyDirectories = AOnTooManyDirectories;
  FSynchronizeMonitor = NULL;
  FSynchronizeAbort = NULL;
  FSynchronizeLog = NULL;
  FOptions = NULL;
}
//---------------------------------------------------------------------------
TSynchronizeController::~TSynchronizeController()
{
  assert(FSynchronizeMonitor == NULL);
}
//---------------------------------------------------------------------------
void TSynchronizeController::StartStop(TObject * Sender,
  bool Start, const TSynchronizeParamType & Params, const TCopyParamType & CopyParam,
  TSynchronizeOptions * Options,
  TSynchronizeAbortEvent OnAbort, TSynchronizeThreadsEvent OnSynchronizeThreads,
  TSynchronizeLog OnSynchronizeLog)
{
  if (Start)
  {
    try
    {
      assert(OnSynchronizeLog != NULL);
      FSynchronizeLog = OnSynchronizeLog;

      FOptions = Options;
      if (FLAGSET(Params.Options, soSynchronize) &&
          (FOnSynchronize != NULL))
      {
        FOnSynchronize(this, Params.LocalDirectory,
          Params.RemoteDirectory, CopyParam,
          Params, NULL, FOptions, true);
      }

      FCopyParam = CopyParam;
      FSynchronizeParams = Params;

      assert(OnAbort);
      FSynchronizeAbort = OnAbort;

      if (FLAGSET(FSynchronizeParams.Options, soRecurse))
      {
        SynchronizeLog(slScan,
          FMTLOAD(SYNCHRONIZE_SCAN, (FSynchronizeParams.LocalDirectory)));
      }

      FSynchronizeMonitor = new TDiscMonitor(dynamic_cast<TComponent*>(Sender));
      FSynchronizeMonitor->SubTree = false;
      TMonitorFilters Filters;
      Filters << moFilename << moLastWrite;
      if (FLAGSET(FSynchronizeParams.Options, soRecurse))
      {
        Filters << moDirName;
      }
      FSynchronizeMonitor->Filters = Filters;
      FSynchronizeMonitor->MaxDirectories = 0;
      FSynchronizeMonitor->ChangeDelay = GUIConfiguration->KeepUpToDateChangeDelay;
      FSynchronizeMonitor->OnTooManyDirectories = SynchronizeTooManyDirectories;
      FSynchronizeMonitor->OnDirectoriesChange = SynchronizeDirectoriesChange;
      FSynchronizeMonitor->OnFilter = SynchronizeFilter;
      FSynchronizeMonitor->AddDirectory(FSynchronizeParams.LocalDirectory,
        FLAGSET(FSynchronizeParams.Options, soRecurse));
      FSynchronizeMonitor->OnChange = SynchronizeChange;
      FSynchronizeMonitor->OnInvalid = SynchronizeInvalid;
      FSynchronizeMonitor->OnSynchronize = OnSynchronizeThreads;
      // get count before open to avoid thread issues
      int Directories = FSynchronizeMonitor->Directories->Count;
      FSynchronizeMonitor->Open();

      SynchronizeLog(slStart, FMTLOAD(SYNCHRONIZE_START, (Directories)));
    }
    catch(...)
    {
      SAFE_DESTROY(FSynchronizeMonitor);
      throw;
    }
  }
  else
  {
    FOptions = NULL;
    SAFE_DESTROY(FSynchronizeMonitor);
  }
}
//---------------------------------------------------------------------------
void TSynchronizeController::SynchronizeChange(
  TObject * /*Sender*/, const wstring Directory, bool & SubdirsChanged)
{
  try
  {
    wstring RemoteDirectory;
    wstring RootLocalDirectory;
    RootLocalDirectory = IncludeTrailingBackslash(FSynchronizeParams.LocalDirectory);
    RemoteDirectory = UnixIncludeTrailingBackslash(FSynchronizeParams.RemoteDirectory);

    wstring LocalDirectory = IncludeTrailingBackslash(Directory);

    assert(LocalDirectory.SubString(1, RootLocalDirectory.Length()) ==
      RootLocalDirectory);
    RemoteDirectory = RemoteDirectory +
      ToUnixPath(LocalDirectory.SubString(RootLocalDirectory.Length() + 1,
        LocalDirectory.Length() - RootLocalDirectory.Length()));

    SynchronizeLog(slChange, FMTLOAD(SYNCHRONIZE_CHANGE,
      (ExcludeTrailingBackslash(LocalDirectory))));

    if (FOnSynchronize != NULL)
    {
      // this is completelly wrong as the options structure
      // can contain non-root specific options in future
      TSynchronizeOptions * Options =
        ((LocalDirectory == RootLocalDirectory) ? FOptions : NULL);
      TSynchronizeChecklist * Checklist = NULL;
      FOnSynchronize(this, LocalDirectory, RemoteDirectory, FCopyParam,
        FSynchronizeParams, &Checklist, Options, false);
      if (Checklist != NULL)
      {
        try
        {
          if (FLAGSET(FSynchronizeParams.Options, soRecurse))
          {
            SubdirsChanged = false;
            assert(Checklist != NULL);
            for (int Index = 0; Index < Checklist->Count; Index++)
            {
              const TSynchronizeChecklist::TItem * Item = Checklist->Item[Index];
              // note that there may be action saDeleteRemote even if nothing has changed
              // so this is sub-optimal
              if (Item->IsDirectory)
              {
                if ((Item->Action == TSynchronizeChecklist::saUploadNew) ||
                    (Item->Action == TSynchronizeChecklist::saDeleteRemote))
                {
                  SubdirsChanged = true;
                  break;
                }
                else
                {
                  assert(false);
                }
              }
            }
          }
          else
          {
            SubdirsChanged = false;
          }
        }
        __finally
        {
          delete Checklist;
        }
      }
    }
  }
  catch(Exception & E)
  {
    SynchronizeAbort(dynamic_cast<EFatal*>(&E) != NULL);
  }
}
//---------------------------------------------------------------------------
void TSynchronizeController::SynchronizeAbort(bool Close)
{
  if (FSynchronizeMonitor != NULL)
  {
    FSynchronizeMonitor->Close();
  }
  assert(FSynchronizeAbort);
  FSynchronizeAbort(NULL, Close);
}
//---------------------------------------------------------------------------
void TSynchronizeController::LogOperation(TSynchronizeOperation Operation,
  const wstring FileName)
{
  TSynchronizeLogEntry Entry;
  wstring Message;
  switch (Operation)
  {
    case soDelete:
      Entry = slDelete;
      Message = FMTLOAD(SYNCHRONIZE_DELETED, (FileName));
      break;

    default:
      assert(false);
      // fallthru

    case soUpload:
      Entry = slUpload;
      Message = FMTLOAD(SYNCHRONIZE_UPLOADED, (FileName));
      break;
  }
  SynchronizeLog(Entry, Message);
}
//---------------------------------------------------------------------------
void TSynchronizeController::SynchronizeLog(TSynchronizeLogEntry Entry,
  const wstring Message)
{
  if (FSynchronizeLog != NULL)
  {
    FSynchronizeLog(this, Entry, Message);
  }
}
//---------------------------------------------------------------------------
void TSynchronizeController::SynchronizeFilter(TObject * /*Sender*/,
  const wstring DirectoryName, bool & Add)
{
  if ((FOptions != NULL) && (FOptions->Filter != NULL))
  {
    if (IncludeTrailingBackslash(ExtractFilePath(DirectoryName)) ==
          IncludeTrailingBackslash(FSynchronizeParams.LocalDirectory))
    {
      int FoundIndex;
      Add = FOptions->Filter->Find(ExtractFileName(DirectoryName), FoundIndex);
    }
  }
  TFileMasks::TParams MaskParams; // size does not matter for directories
  Add = Add && FCopyParam.AllowTransfer(DirectoryName, osLocal, true, MaskParams);
}
//---------------------------------------------------------------------------
void TSynchronizeController::SynchronizeInvalid(
  TObject * /*Sender*/, const wstring Directory, const wstring ErrorStr)
{
  if (FOnSynchronizeInvalid != NULL)
  {
    FOnSynchronizeInvalid(this, Directory, ErrorStr);
  }

  SynchronizeAbort(false);
}
//---------------------------------------------------------------------------
void TSynchronizeController::SynchronizeTooManyDirectories(
  TObject * /*Sender*/, int & MaxDirectories)
{
  if (FOnTooManyDirectories != NULL)
  {
    FOnTooManyDirectories(this, MaxDirectories);
  }
}
//---------------------------------------------------------------------------
void TSynchronizeController::SynchronizeDirectoriesChange(
  TObject * /*Sender*/, int Directories)
{
  SynchronizeLog(slDirChange, FMTLOAD(SYNCHRONIZE_START, (Directories)));
}
