// © 2024 NVIDIA Corporation

// Goal: data streaming

#pragma once

#define NRI_STREAMER_H 1

namespace Core::RHI {
    struct Streamer;

    struct DataSize {
        const void* data;
        uint64_t size;
    };

    struct BufferOffset {
        Buffer *buffer;
        uint64_t offset;
    };

    struct StreamerDesc {
        // Statically allocated ring-buffer for dynamic constants
        MemoryLocation constantBufferMemoryLocation; // UPLOAD or DEVICE_UPLOAD
        uint64_t constantBufferSize;            // should be large enough to avoid overwriting data for enqueued frames

        // Dynamically (re)allocated ring-buffer for copying and rendering
        MemoryLocation dynamicBufferMemoryLocation;    // UPLOAD or DEVICE_UPLOAD
        BufferDesc dynamicBufferDesc;                  // "size" is ignored
        uint32_t queuedFrameNum;                            // number of frames "in-flight" (usually 1-3), adds 1 under the hood for the current "not-yet-committed" frame
    };

    struct StreamBufferDataDesc {
        // Data to upload
        const DataSize *dataChunks;                  // will be concatenated in dynamic buffer memory
        uint32_t dataChunkNum;
        uint32_t placementAlignment;                        // desired alignment for "BufferOffset::offset"

        // Destination
        Buffer *dstBuffer;
        uint64_t dstOffset;
    };

    struct StreamTextureDataDesc {
        // Data to upload
        const void* data;
        uint32_t dataRowPitch;
        uint32_t dataSlicePitch;

        // Destination
        Texture *dstTexture;
        TextureRegionDesc dstRegion;
    };

    // Threadsafe: yes by default (see NRI_STREAMER_THREAD_SAFE CMake option)
    struct StreamerInterface {
        Result         (*CreateStreamer)              (Device& device, const StreamerDesc& streamerDesc, Streamer*& streamer);
        void                (*DestroyStreamer)             (Streamer* streamer);

        // Statically allocated (never changes)
        Buffer*      (*GetStreamerConstantBuffer)   (Streamer& streamer);

        // (HOST) Stream data to a dynamic buffer. Return "buffer & offset" for direct usage in the current frame
        BufferOffset   (*StreamBufferData)            (Streamer& streamer, const StreamBufferDataDesc& streamBufferDataDesc);
        BufferOffset   (*StreamTextureData)           (Streamer& streamer, const StreamTextureDataDesc& streamTextureDataDesc);

        // (HOST) Stream data to a constant buffer. Return "offset" in "GetStreamerConstantBuffer" for direct usage in the current frame
        uint32_t            (*StreamConstantData)          (Streamer& streamer, const void* data, uint32_t dataSize);

        // Command buffer
        // {
        // (DEVICE) Copy data to destinations (if any), which must be in "COPY_DESTINATION" state
        void            (*CmdCopyStreamedData)         (CommandBuffer& commandBuffer, Streamer& streamer);
        // }

        // (HOST) Must be called once at the very end of the frame
        void                (*EndStreamerFrame)            (Streamer& streamer);
    };
}