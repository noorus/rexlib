#pragma once

#include <windows.h>
#include <string>
#include <list>
#include <exception>
#include "REXNative.h"

namespace Rex {

# define REX_LIBRARY_FILE L"REX Shared Library.dll"

  typedef float Tempo;
  typedef unsigned __int32 uint32_t;

  class Exception: public std::runtime_error {
  public:
    Exception( const std::string& msg );
  };

  class Factory;

  class Loop {
  friend class Factory;
  protected:
    Factory* mFactory;
    HANDLE mFile;
    LPBYTE mBuffer;
    RexNative::Handle mHandle;
    RexNative::Info mInfo;
    RexNative::Creator mCreator;
  protected:
    Loop( Factory* factory, const std::wstring& rexFile );
    ~Loop();
    void loadFromBuffer( LPBYTE buffer, DWORD length );
    inline void closeFile();
  public:
    uint32_t getChannels();
    uint32_t getSampleRate();
    uint32_t getSliceCount();
    float getBPM();
    float getNativeBPM();
    uint32_t getBits();
  };

  typedef std::list<Loop*> LoopList;

  class Factory {
  friend class Loop;
  protected:
    HMODULE mLibrary;
    HANDLE mHeap;
    LoopList mLoops;
    RexNative::fnOpen pfnOpen;
    RexNative::fnClose pfnClose;
    RexNative::fnREXCreate pfnREXCreate;
    RexNative::fnREXDelete pfnREXDelete;
    RexNative::fnREXGetInfo pfnREXGetInfo;
    RexNative::fnREXGetCreatorInfo pfnREXGetCreatorInfo;
    HANDLE getHeap();
  public:
    Factory( const std::wstring& libraryFile = REX_LIBRARY_FILE, HANDLE customHeap = NULL );
    Loop* loadLoop( const std::wstring& rexFile );
    void freeLoop( Loop* loop );
    ~Factory();
  };

}