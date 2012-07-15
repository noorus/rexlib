#include <windows.h>
#include "REXNative.h"
#include "REXLib.h"

namespace Rex {

  using std::string;
  using std::wstring;

# define LOAD_REX_FUNC(x,y,z) p##x##=(RexNative::##x##)GetProcAddress(y,z)

  // Exception class ----------------------------------------------------------

  Exception::Exception( const string& msg ): std::runtime_error( msg )
  {
    //
  }

  // Loop class ---------------------------------------------------------------

  Loop::Loop( Factory* factory, const wstring& rexFile ): mFactory( factory ),
  mFile( NULL ), mBuffer( NULL ), mHandle( 0 )
  {
    mFile = CreateFileW( rexFile.c_str(), GENERIC_READ, FILE_SHARE_READ,
      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( mFile == INVALID_HANDLE_VALUE )
      throw Exception( "Couldn't open loop file" );

    DWORD fileSize = GetFileSize( mFile, NULL );
    if ( !fileSize || fileSize == INVALID_FILE_SIZE )
    {
      closeFile();
      throw Exception( "Invalid loop file" );
    }

    mBuffer = (LPBYTE)HeapAlloc( mFactory->getHeap(), NULL, fileSize );
    if ( !mBuffer )
      throw Exception( "Heap allocation failed" );

    DWORD bytesRead = 0;
    if ( !ReadFile( mFile, mBuffer, fileSize, &bytesRead, NULL ) || bytesRead < fileSize )
    {
      HeapFree( mFactory->getHeap(), NULL, (LPVOID)mBuffer );
      closeFile();
      throw Exception( "Reading loop file failed" );
    }

    loadFromBuffer( mBuffer, fileSize );
  }

  uint32_t Loop::getChannels()
  {
    return mInfo.channels;
  }

  uint32_t Loop::getSampleRate()
  {
    return mInfo.sampleRate;
  }

  uint32_t Loop::getSliceCount()
  {
    return mInfo.slices;
  }

  float Loop::getBPM()
  {
    float bpm = (float)mInfo.tempo / 1000.0f;
    return bpm;
  }

  float Loop::getNativeBPM()
  {
    float bpm = (float)mInfo.nativeTempo / 1000.0f;
    return bpm;
  }

  uint32_t Loop::getBits()
  {
    return mInfo.bits;
  }

  void Loop::loadFromBuffer( LPBYTE buffer, DWORD length )
  {
    if ( mFactory->pfnREXCreate( &mHandle, buffer, length, NULL, NULL ) != RexNative::Return_Ok )
      throw Exception( "REXCreate failed" );
    if ( mFactory->pfnREXGetInfo( mHandle, sizeof( RexNative::Info ), &mInfo ) != RexNative::Return_Ok )
      throw Exception( "REXGetInfo failed" );
    if ( mFactory->pfnREXGetCreatorInfo( mHandle, sizeof( RexNative::Creator ), &mCreator ) != RexNative::Return_Ok )
      throw Exception( "REXGetCreatorInfo failed" );
  }

  void Loop::closeFile()
  {
    if ( mFile != INVALID_HANDLE_VALUE )
      CloseHandle( mFile );
    mFile = INVALID_HANDLE_VALUE;
  }

  Loop::~Loop()
  {
    if ( mHandle )
      mFactory->pfnREXDelete( &mHandle );
    if ( mBuffer )
      HeapFree( mFactory->getHeap(), NULL, (LPVOID)mBuffer );
    closeFile();
  }

  // Factory class ------------------------------------------------------------

  Factory::Factory( const wstring &libraryFile, HANDLE customHeap ):
  mLibrary( NULL ), pfnOpen( NULL ), pfnClose( NULL ), mHeap( NULL )
  {
    if ( !mHeap || mHeap == INVALID_HANDLE_VALUE )
      mHeap = GetProcessHeap();

    mLibrary = LoadLibraryW( libraryFile.c_str() );
    if ( !mLibrary )
      throw Exception( "Couldn't load REX shared library" );

    LOAD_REX_FUNC( fnOpen, mLibrary, "Open" );
    LOAD_REX_FUNC( fnClose, mLibrary, "Close" );
    LOAD_REX_FUNC( fnREXCreate, mLibrary, "REXCreate" );
    LOAD_REX_FUNC( fnREXDelete, mLibrary, "REXDelete" );
    LOAD_REX_FUNC( fnREXGetInfo, mLibrary, "REXGetInfo" );
    LOAD_REX_FUNC( fnREXGetCreatorInfo, mLibrary, "REXGetCreatorInfo" );

    if ( !pfnOpen || !pfnClose || !pfnREXCreate || !pfnREXDelete )
      throw Exception( "Couldn't resolve REX shared library exports" );

    if ( !pfnOpen() )
      throw Exception( "REX shared library initialization failed" );
  }

  HANDLE Factory::getHeap()
  {
    return mHeap;
  }

  Loop* Factory::loadLoop( const wstring& rexFile )
  {
    Loop* loop = new Loop( this, rexFile );
    mLoops.push_back( loop );
    return loop;
  }

  void Factory::freeLoop( Loop* loop )
  {
    for ( LoopList::iterator it = mLoops.begin(); it != mLoops.end(); ++it )
      if ( (*it) == loop ) {
        mLoops.erase( it );
        delete loop;
        return;
      }
    throw Exception( "Bad/unknown loop object in freeLoop()" );
  }

  Factory::~Factory()
  {
    for ( LoopList::iterator it = mLoops.begin(); it != mLoops.end(); ++it )
      delete (*it);
    if ( pfnClose )
      pfnClose();
    if ( mLibrary )
      FreeLibrary( mLibrary );
  }

}