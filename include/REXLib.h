#pragma once

#include <windows.h>
#include <string>
#include <list>
#include <exception>
#include <stdint.h>
#include "REXNative.h"

namespace Rex {

# define REX_LIBRARY_FILE L"REX Shared Library.dll"

  class Exception;
  class Loop;
  class Factory;

  class Exception: public std::runtime_error {
  public:
    Exception( const std::string& msg );
  };

  struct Info {
    uint32_t channels;
    uint32_t sampleRate;
    uint32_t bits;
    uint32_t slices;
    uint32_t length;
    float tempo;
    float nativeTempo;
  };

  struct CreatorInfo {
    std::wstring name;
    std::wstring copyright;
    std::wstring url;
    std::wstring email;
    std::wstring description;
  };

  class Loop {
  friend class Factory;
  protected:
    Factory* mFactory;
    HANDLE mFile;
    LPBYTE mBuffer;
    RexNative::Handle mHandle;
    Info mInfo;
    CreatorInfo mCreator;
  protected:
    Loop( Factory* factory, const std::wstring& rexFile );
    Loop( Factory* factory, void* memory, uint32_t size );
    ~Loop();
    void _loadFromBuffer( LPBYTE buffer, DWORD length );
    void _readInfo();
    void _readCreator();
    inline void _closeFile();
  public:
    const Info& getInfo() throw();
    const CreatorInfo& getCreator() throw();
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
    Factory( const std::wstring& libraryFile = REX_LIBRARY_FILE,
      HANDLE customHeap = NULL );
    Loop* loadLoop( const std::wstring& rexFile );
    Loop* loadLoop( void* memory, uint32_t size );
    void freeLoop( Loop* loop );
    ~Factory();
  };

}