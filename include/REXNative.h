#pragma once

namespace RexNative {

  typedef unsigned __int32 uint32_t;

  typedef unsigned long Handle;

  enum Return: uint32_t {
    Return_Failed = 0,
    Return_Ok,
    Return_Aborted,
    Return_Unavailable,
    Return_DLL_OutOfMemory = 100,
    Return_DLL_LoadFailed,
    Return_DLL_Outdated,
    Return_DLL_NotFound,
    Return_APIOutdated,
    Return_OutOfMemory,
    Return_FileCorrupted,
    Return_FileUnsupported,
    Return_FileEmpty,
    Return_BadOS,
    Return_DLL_NotLoaded = 200,
    Return_DLL_Loaded,
    Return_BadHandle,
    Return_BadSize,
    Return_BadArgument,
    Return_BadSlice,
    Return_BadSampleRate,
    Return_BufferTooSmall,
    Return_InPreviewMode,
    Return_NotInPreviewMode,
    Return_BadTempo,
    Return_Unknown = 666
  };

  enum CallbackReturn: uint32_t {
    CallbackReturn_Fail = 1,
    CallbackReturn_Ok
  };

# pragma pack(push,4)

  struct Info {
    uint32_t channels;
    uint32_t sampleRate;
    uint32_t slices;
    uint32_t tempo;
    uint32_t nativeTempo;
    uint32_t length;
    uint32_t unk1;
    uint32_t unk2;
    uint32_t bits;
  };

  struct Slice {
    uint32_t position;
    uint32_t length;
  };

  struct Creator {
    char name[256];
    char copyright[256];
    char url[256];
    char email[256];
    char description[256];
  };

# pragma pack(pop)

  extern "C" {

    typedef CallbackReturn (*fnCreateCallback)( long percent, void* data );

    typedef uint32_t (*fnOpen)();
    typedef void     (*fnClose)();
    typedef Return   (*fnREXCreate)( Handle* handle, void* buffer, long size, fnCreateCallback callback, void* data );
    typedef void     (*fnREXDelete)( Handle* handle );
    typedef Return   (*fnREXGetInfo)( Handle handle, long infoSize, Info* info );
    typedef Return   (*fnREXGetInfoFromBuffer)( long size, void* buffer, long infoSize, Info* info );
    typedef Return   (*fnREXGetCreatorInfo)( Handle handle, long infoSize, Creator* info );
    typedef Return   (*fnREXGetSliceInfo)( Handle handle, long index, long infoSize, Slice* info );
    typedef Return   (*fnREXSetOutputSampleRate)( Handle handle, long sampleRate );
    typedef Return   (*fnREXRenderSlice)( Handle handle, long index, long length, float* buffers[2] );
    typedef Return   (*fnREXStartPreview)( Handle handle );
    typedef Return   (*fnREXStopPreview)( Handle handle );
    typedef Return   (*fnREXRenderPreviewBatch)( Handle handle, long length, float* buffers[2] );
    typedef Return   (*fnREXSetPreviewTempo)( Handle handle, long tempo );

  };

}