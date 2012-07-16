#include <windows.h>
#include "REXNative.h"
#include "REXLib.h"

namespace Rex {

  using std::string;
  using std::wstring;

# define LOAD_REX_FUNC(x,y) pfn##x##=(RexNative::fn##x##)GetProcAddress(y,#x)

  // Utility functions --------------------------------------------------------

  wstring stringToWide( const string& str, UINT codepage )
  {
    DWORD length = MultiByteToWideChar( codepage, NULL,
      str.c_str(), -1, NULL, NULL );
    if ( length == 0 )
      return wstring();
    wstring result( length, NULL );
    MultiByteToWideChar( codepage, NULL, str.c_str(), -1,
      &result[0], length );
    return result;
  }

  // Exception class ----------------------------------------------------------

  Exception::Exception( const string& msg ): std::runtime_error( msg )
  {
    //
  }

  // Loop class ---------------------------------------------------------------

  Loop::Loop( Factory* factory, const wstring& rexFile ): mFactory( factory ),
  mFile( INVALID_HANDLE_VALUE ), mBuffer( NULL ), mHandle( 0 )
  {
    mFile = CreateFileW( rexFile.c_str(), GENERIC_READ, FILE_SHARE_READ,
      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( mFile == INVALID_HANDLE_VALUE )
      throw Exception( "Couldn't open loop file" );

    DWORD fileSize = GetFileSize( mFile, NULL );
    if ( !fileSize || fileSize == INVALID_FILE_SIZE )
    {
      _closeFile();
      throw Exception( "Invalid loop file" );
    }

    mBuffer = (LPBYTE)HeapAlloc( mFactory->getHeap(), NULL, fileSize );
    if ( !mBuffer )
      throw Exception( "Heap allocation failed" );

    DWORD bytesRead = 0;
    if ( !ReadFile( mFile, mBuffer, fileSize, &bytesRead, NULL ) || bytesRead < fileSize )
    {
      HeapFree( mFactory->getHeap(), NULL, (LPVOID)mBuffer );
      _closeFile();
      throw Exception( "Reading loop file failed" );
    }

    _loadFromBuffer( mBuffer, fileSize );
  }

  Loop::Loop( Factory* factory, void* memory, uint32_t size ):
  mFactory( factory ), mFile( INVALID_HANDLE_VALUE ), mBuffer( NULL ),
  mHandle( 0 )
  {
    _loadFromBuffer( (LPBYTE)memory, size );
  }

  void Loop::_loadFromBuffer( LPBYTE buffer, DWORD length )
  {
    if ( mFactory->pfnREXCreate( &mHandle, buffer, length, NULL, NULL ) != RexNative::Return_Ok )
      throw Exception( "REXCreate failed" );
    _readInfo();
    _readCreator();
  }

  void Loop::_readInfo()
  {
    RexNative::Info info;
    if ( mFactory->pfnREXGetInfo( mHandle, sizeof( RexNative::Info ), &info ) != RexNative::Return_Ok )
      throw Exception( "REXGetInfo failed" );
    mInfo.channels = info.channels;
    mInfo.sampleRate = info.sampleRate;
    mInfo.bits = info.bits;
    mInfo.slices = info.slices;
    mInfo.length = info.length;
    mInfo.tempo = (float)info.tempo / 1000.0f;
    mInfo.nativeTempo = (float)info.nativeTempo / 1000.0f;
  }

  void Loop::_readCreator()
  {
    RexNative::Creator creator;
    if ( mFactory->pfnREXGetCreatorInfo( mHandle, sizeof( RexNative::Creator ), &creator ) != RexNative::Return_Ok )
      throw Exception( "REXGetCreatorInfo failed" );
    mCreator.name = stringToWide( creator.name, CP_UTF8 );
    mCreator.copyright = stringToWide( creator.copyright, CP_UTF8 );
    mCreator.url = stringToWide( creator.url, CP_UTF8 );
    mCreator.email = stringToWide( creator.email, CP_UTF8 );
    mCreator.description = stringToWide( creator.description, CP_UTF8 );
  }

  const Info& Loop::getInfo()
  {
    return mInfo;
  }

  const CreatorInfo& Loop::getCreator()
  {
    return mCreator;
  }

  void Loop::_closeFile()
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
    _closeFile();
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

    LOAD_REX_FUNC( Open, mLibrary );
    LOAD_REX_FUNC( Close, mLibrary );
    LOAD_REX_FUNC( REXCreate, mLibrary );
    LOAD_REX_FUNC( REXDelete, mLibrary );
    LOAD_REX_FUNC( REXGetInfo, mLibrary );
    LOAD_REX_FUNC( REXGetCreatorInfo, mLibrary );

    if ( !pfnOpen || !pfnClose || !pfnREXCreate || !pfnREXDelete
      || !pfnREXGetInfo || !pfnREXGetCreatorInfo )
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

  Loop* Factory::loadLoop( void* memory, uint32_t size )
  {
    Loop* loop = new Loop( this, memory, size );
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
    throw Exception( "Unknown loop object" );
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