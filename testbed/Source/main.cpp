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
      loop->getChannels(), loop->getSampleRate(), loop->getBits(),
      loop->getSliceCount(), loop->getBPM(), loop->getNativeBPM() );
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