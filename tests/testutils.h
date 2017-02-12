#pragma once

#include <time.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include <vcl.h>
#include <Sysutils.hpp>

#include "FarPlugin.h"
#include "Cryptography.h"
#include "WinSCPSecurity.h"

#define CATCH_CONFIG_MAIN
//#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_CPP11_NO_SHUFFLE
#include <catch/catch.hpp>

//------------------------------------------------------------------------------

#define TEST_CASE_TODO(exp) \
    std::cerr << "TODO: " << #exp << std::endl

#define REQUIRE_EQUAL(exp1, exp2) \
  REQUIRE(exp1 == exp2)

std::ostringstream& operator << (std::ostringstream& os, const AnsiString& value)
{
  os << std::string(value.c_str());
  return os;
}

std::ostringstream& operator << (std::ostringstream& os, const UnicodeString& value)
{
  os << std::string(W2MB(value.c_str()).c_str());
  return os;
}

//------------------------------------------------------------------------------
class TStubFarPlugin : public TCustomFarPlugin
{
public:
    explicit TStubFarPlugin() :
        TCustomFarPlugin(OBJECT_CLASS_TCustomFarPlugin, 0) // GetModuleHandle(0))
    {
      INFO("TStubFarPlugin()");
//      CryptographyInitialize();
    }
    ~TStubFarPlugin()
    {
      INFO("~TStubFarPlugin()");
//      CryptographyFinalize();
    }
protected:
    virtual void GetPluginInfoEx(DWORD &Flags,
        TStrings *DiskMenuStrings, TStrings *PluginMenuStrings,
        TStrings *PluginConfigStrings, TStrings *CommandPrefixes)
    {
        DEBUG_PRINTF(L"call");
    }
    virtual TCustomFarFileSystem * OpenPluginEx(intptr_t OpenFrom, intptr_t Item)
    {
        DEBUG_PRINTF(L"call");
        return NULL;
    }
    virtual bool ConfigureEx(intptr_t Item)
    {
        DEBUG_PRINTF(L"call");
        return false;
    }
    virtual intptr_t ProcessEditorEventEx(intptr_t Event, void *Param)
    {
        DEBUG_PRINTF(L"call");
        return -1;
    }
    virtual intptr_t ProcessEditorInputEx(const INPUT_RECORD *Rec)
    {
        DEBUG_PRINTF(L"call");
        return -1;
    }
};

//------------------------------------------------------------------------------

static TCustomFarPlugin * CreateStub()
{
  return new TStubFarPlugin();
}

//------------------------------------------------------------------------------

