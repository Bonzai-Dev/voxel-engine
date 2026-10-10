// © 2024 NVIDIA Corporation

// Goal: data streaming

#pragma once

#define NRI_STREAMER_H 1

namespace Core::RHI {
  struct Streamer;

  struct DataSize {
    const void *data;
    uint64_t size;
  };

  struct BufferOffset {
    Buffer *buffer;
    uint64_t offset;
  };

  struct StreamerDesc {
    // Statically allocated ring-buffer for dynamic constants
    RHI_OPTIONAL MemoryLocation constantBufferMemoryLocation; // UPLOAD or DEVICE_UPLOAD
    RHI_OPTIONAL uint64_t constantBufferSize; // should be large enough to avoid overwriting data for enqueued frames

    // Dynamically (re)allocated ring-buffer for copying and rendering
    MemoryLocation dynamicBufferMemoryLocation; // UPLOAD or DEVICE_UPLOAD
    BufferDesc dynamicBufferDesc; // "size" is ignored
    uint32_t queuedFrameNum;
    // number of frames "in-flight" (usually 1-3), adds 1 under the hood for the current "not-yet-committed" frame
  };

  struct StreamBufferDataDesc {
    // Data to upload
    const DataSize *dataChunks; // will be concatenated in dynamic buffer memory
    uint32_t dataChunkNum;
    uint32_t placementAlignment; // desired alignment for "BufferOffset::offset"

    // Destination
    RHI_OPTIONAL Buffer *dstBuffer;
    RHI_OPTIONAL uint64_t dstOffset;
  };

  struct StreamTextureDataDesc {
    // Data to upload
    const void *data;
    uint32_t dataRowPitch;
    uint32_t dataSlicePitch;

    // Destination
    RHI_OPTIONAL Texture *dstTexture;
    RHI_OPTIONAL TextureRegionDesc dstRegion;
  };

    /*clang-format off*/
    // Threadsafe: yes by default (see NRI_STREAMER_THREAD_SAFE CMake option)
    struct StreamerInterface {
        Result         (*CreateStreamer)              (Device &Device, const StreamerDesc &StreamerDesc, RHI_OUT Streamer *&streamer);
        void           (*DestroyStreamer)             (Streamer *Streamer);

        // Statically allocated (never changes)
        Buffer*        (*GetStreamerConstantBuffer)   (Streamer &Streamer);

        // (HOST) Stream data to a dynamic buffer. Return "buffer & offset" for direct usage in the current frame
        BufferOffset   (*StreamBufferData)            (Streamer &Streamer, const StreamBufferDataDesc &StreamBufferDataDesc);
        BufferOffset   (*StreamTextureData)           (Streamer &Streamer, const StreamTextureDataDesc &StreamTextureDataDesc);

        // (HOST) Stream data to a constant buffer. Return "offset" in "GetStreamerConstantBuffer" for direct usage in the current frame
        uint32_t       (*StreamConstantData)          (Streamer &Streamer, const void* data, uint32_t dataSize);

        // Command buffer
        // {
        // (DEVICE) Copy data to destinations (if any), which must be in "COPY_DESTINATION" state
        void           (*CmdCopyStreamedData)         (CommandBuffer &CommandBuffer, Streamer &Streamer);
        // }

        // (HOST) Must be called once at the very end of the frame
        void           (*EndStreamerFrame)            (Streamer &Streamer);
    };
  /*clang-format on*/
}
