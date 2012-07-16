#include <stdio.h>
#include <windows.h>
#include <REXLib.h>

int wmain( int argc, wchar_t* argv[], wchar_t* envp[] )
{
  try {
    wprintf_s( L"Creating factory\r\n" );
    Rex::Factory* factory = new Rex::Factory();
    wprintf_s( L"Loading loop\r\n" );
    Rex::Loop* loop = factory->loadLoop( L"kissbang.rx2" );
    wprintf_s( L"Loop info:\r\n  Channels: %u\r\n  Sample rate: %u\r\n  Bits: %u\r\n  Slices: %u\r\n  BPM: %.3f\r\n  Native BPM: %.3f\r\n",
      loop->getInfo().channels, loop->getInfo().sampleRate, loop->getInfo().bits,
      loop->getInfo().slices, loop->getInfo().tempo, loop->getInfo().nativeTempo );
    wprintf_s( L"Creator info:\r\n  Name: %s\r\n  Copyright: %s\r\n  URL: %s\r\n  Email: %s\r\n  Description: %s\r\n",
      loop->getCreator().name.c_str(), loop->getCreator().copyright.c_str(),
      loop->getCreator().url.c_str(), loop->getCreator().email.c_str(),
      loop->getCreator().description.c_str() );
    wprintf_s( L"Freeing loop\r\n" );
    factory->freeLoop( loop );
    wprintf_s( L"Freeing factory\r\n" );
    delete factory;
  } catch ( Rex::Exception& e ) {
    wprintf_s( L"Exception: %S\r\n", e.what() );
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}