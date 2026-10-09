// © 2021 NVIDIA Corporation

// Goal: ray tracing
// https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html
// https://microsoft.github.io/DirectX-Specs/d3d/Raytracing2.html

#pragma once

#define NRI_RAY_TRACING_H 1

namespace Core::RHI {
  /*clang-format off*/
  struct AccelerationStructure; // bottom- or top- level acceleration structure (aka BLAS or TLAS respectively)
  struct Micromap;              // a micromap that encodes sub-triangle opacity (aka OMM, can be attached to a triangle BLAS)

  // static const Buffer *NriConstant(HAS_BUFFER) = (Buffer*)1; // only to indicate buffer presence in "AccelerationStructureDesc"

  //============================================================================================================================================================================================
  #pragma region [ Pipeline ]
  //============================================================================================================================================================================================

  ENGINE_BITS(RayTracingPipelineBits, uint8_t,
    None                        = 0,
    SkipTriangles              = ENGINE_BIT(0), // provides knowledge that "triangles" doesn't need to be considered
    SkipAABBs                  = ENGINE_BIT(1), // provides knowledge that "aabbs" doesn't need to be considered
    AllowMicromaps             = ENGINE_BIT(2), // specifies that the ray tracing pipeline can be used with acceleration structures which reference micromaps
    FailOnCacheMiss          = ENGINE_BIT(3)  // "CreateRayTracingPipeline" returns "FAILURE" if the pipeline is not found in the supplied cache (requires "features.pipelineCacheControl")
  );

  struct ShaderLibraryDesc {
    const ShaderInfo *shaders;
    uint32_t shaderNum;
  };

  struct ShaderGroupDesc {
    // Use cases:
    //  - general: RAYGEN_SHADER, MISS_SHADER or CALLABLE_SHADER
    //  - HitGroup: CLOSEST_HIT_SHADER and/or ANY_HIT_SHADER in any order
    //  - HitGroup with an intersection shader: INTERSECTION_SHADER + CLOSEST_HIT_SHADER and/or ANY_HIT_SHADER in any order
    uint32_t shaderIndices[3]; // in ShaderLibrary, starting from 1 (0 - unused)
  };

  struct RayTracingPipelineDesc {
    const PipelineLayout *pipelineLayout;
    const ShaderLibraryDesc *shaderLibrary;
    const ShaderGroupDesc *shaderGroups;
    uint32_t shaderGroupNum;
    uint32_t recursionMaxDepth;
    uint32_t rayPayloadMaxSize;
    uint32_t rayHitAttributeMaxSize;
    RayTracingPipelineBits flags;
    Robustness robustness;
    const PipelineCache *cache; // if non-NULL, pipeline creation can be served from a cached blob and the result will be added to the cache on a miss
  };

  #pragma endregion

  //============================================================================================================================================================================================
  #pragma region [ Opacity Micromap (OMM) ]
  //============================================================================================================================================================================================

  enum class MicromapFormat: uint16_t {
    Opacity2State             = 1,
    Opacity4State             = 2
  };

  enum class MicromapSpecialIndex: int8_t {
    // 2/4 state
    FullyTransparent           = -1,           // specifies that the entire triangle is fully transparent
    FullyOpaque                = -2,           // specifies that the entire triangle is fully opaque

    // 4 state
    FullyUnknownTransparent   = -3,           // specifies that the entire triangle is unknown-transparent
    FullyUnknownOpaque        = -4            // specifies that the entire triangle is unknown-opaque
  };

  ENGINE_BITS(MicromapBits, uint8_t,
    None                        = 0,
    AllowCompaction            = ENGINE_BIT(1),    // allows to compact the micromap by copying using "COMPACT" mode
    PreferFastTrace           = ENGINE_BIT(2),    // prioritize traversal performance over build time
    PreferFastBuild           = ENGINE_BIT(3)     // prioritize build time over traversal performance
  );

  struct MicromapUsageDesc {
    uint32_t triangleNum;                       // represents "MicromapTriangle" number for "{format, subdivisionLevel}" pair contained in the micromap
    uint16_t subdivisionLevel;                  // micro triangles count = 4 ^ subdivisionLevel
    MicromapFormat format;
  };

  struct MicromapDesc {
    uint64_t optimizedSize;         // can be retrieved by "CmdWriteMicromapsSizes" and used for compaction via "CmdCopyMicromap"
    const MicromapUsageDesc *usages;
    uint32_t usageNum;
    MicromapBits flags;
  };

  struct BindMicromapMemoryDesc {
    Micromap *micromap;
    Memory *memory;
    uint64_t offset;
  };

  struct BuildMicromapDesc {
    Micromap *dst;
    const Buffer *dataBuffer;
    uint64_t dataOffset;
    const Buffer *triangleBuffer;        // contains "MicromapTriangle" entries
    uint64_t triangleOffset;
    Buffer *scratchBuffer;
    uint64_t scratchOffset;
  };

  struct BottomLevelMicromapDesc {
  // For each triangle in the geometry, the acceleration structure build fetches an index from "indexBuffer".
    // If an index is the unsigned cast of one of the values from "MicromapSpecialIndex" then that triangle behaves as described for that special value.
    // Otherwise that triangle uses the micromap information from "micromap" at that index plus "baseTriangle".
    // If an index buffer is not provided, "1:1" mapping between geometry triangles and micromap triangles is assumed.

    Micromap *micromap;
    const Buffer *indexBuffer;
    uint64_t indexOffset;
    uint32_t baseTriangle;
    IndexType indexType;
  };

  // Data layout
  struct MicromapTriangle {
    uint32_t dataOffset;
    uint16_t subdivisionLevel;
    MicromapFormat format;
  };

  #pragma endregion

  //============================================================================================================================================================================================
  #pragma region [ Acceleration Structure: Bottom Level (BLAS) ]
  //============================================================================================================================================================================================

  enum class BottomLevelGeometryType : uint8_t {
    Triangles,
    AABBs
  };

  ENGINE_BITS(BottomLevelGeometryBits, uint8_t,
    None                                = 0,
    OpaqueGeometry                     = ENGINE_BIT(0),    // the geometry acts as if no any hit shader is present (can be overriden by "TopLevelInstanceBits" or ray flags)
    NoDuplicateAnyHitInvocation     = ENGINE_BIT(1)     // the any-hit shader must be called once for each primitive in this geometry
  );

  struct BottomLevelTrianglesDesc {
    // Vertices
    const Buffer *vertexBuffer;
    uint64_t vertexOffset;
    uint32_t vertexNum;
    uint16_t vertexStride;
    DataFormat vertexFormat;

    // Indices
    const Buffer *indexBuffer;
    uint64_t indexOffset;
    uint32_t indexNum;
    IndexType indexType;

    // Transform
    const Buffer *transformBuffer;   // contains "TransformMatrix" entries
    uint64_t transformOffset;

    // Micromap
    const BottomLevelMicromapDesc *micromap;
  };

  struct BottomLevelAabbsDesc {
    const Buffer *buffer;                        // contains "BottomLevelAabb" entries
    uint64_t offset;
    uint32_t num;
    uint32_t stride;
  };

  struct BottomLevelGeometryDesc {
    BottomLevelGeometryBits flags;
    BottomLevelGeometryType type;
    union {
      BottomLevelTrianglesDesc triangles;
      BottomLevelAabbsDesc aabbs;
    };
  };

  // Data layout
  struct TransformMatrix {
    float transform[3][4]; // 3x4 row-major affine transformation matrix, the first three columns of matrix must define an invertible 3x3 matrix
  };

  struct BottomLevelAabb {
    float minX;
    float minY;
    float minZ;
    float maxX;
    float maxY;
    float maxZ;
  };

  #pragma endregion

  //============================================================================================================================================================================================
  #pragma region [ Acceleration Structure: Top Level (TLAS) ]
  //============================================================================================================================================================================================

  ENGINE_BITS(TopLevelInstanceBits, uint32_t,
    NONE                        = 0,
    TRIANGLE_CULL_DISABLE       = ENGINE_BIT(0), // disables face culling for this instance
    TRIANGLE_FLIP_FACING        = ENGINE_BIT(1), // inverts the facing determination for geometry in this instance (since the facing is determined in object space, an instance transform does not change the winding, but a geometry transform does)
    FORCE_OPAQUE                = ENGINE_BIT(2), // force enable "OPAQUE_GEOMETRY" bit on all geometries referenced by this instance
    FORCE_NON_OPAQUE            = ENGINE_BIT(3), // force disable "OPAQUE_GEOMETRY" bit on all geometries referenced by this instance
    FORCE_OPACITY_2_STATE       = ENGINE_BIT(4), // ignore the "unknown" state and only consider the "transparent" or "opaque" bit for all 4-state micromaps encountered during traversal
    DISABLE_MICROMAPS           = ENGINE_BIT(5)  // disable micromap test for all triangles and revert to using geometry opaque/non-opaque state instead
  );

  struct TopLevelInstance {
    float transform[3][4];
    uint32_t instanceId                     : 24;
    uint32_t mask                           : 8;
    uint32_t shaderBindingTableLocalOffset  : 24;
    TopLevelInstanceBits flags         : 8;
    uint64_t accelerationStructureHandle;
  };

  #pragma endregion

  //============================================================================================================================================================================================
  #pragma region [ Acceleration structure (AS) ]
  //============================================================================================================================================================================================

  enum class AccelerationStructureType : uint8_t {
    TopLevel,
    BottomLevel
  };

  ENGINE_BITS(AccelerationStructureBits, uint8_t,
    None                        = 0,
    AllowUpdate                = ENGINE_BIT(0),                // allows to do "updates", which are faster than "builds" (may increase memory usage, build time and decrease traversal performance)
    AllowCompaction            = ENGINE_BIT(1),                // allows to compact the acceleration structure by copying using "COMPACT" mode
    AllowDataAccess           = ENGINE_BIT(2),                // allows to access vertex data from shaders (requires "features.rayTracingPositionFetch")
    AllowMicromapUpdate       = ENGINE_BIT(3),                // allows to update micromaps via acceleration structure update (may increase size and decrease traversal performance)
    AllowDisableMicromaps     = ENGINE_BIT(4),                // allows to have "DISABLE_MICROMAPS" flag for instances referencing this BLAS
    PreferFastTrace           = ENGINE_BIT(5),                // prioritize traversal performance over build time
    PreferFastBuild           = ENGINE_BIT(6),                // prioritize build time over traversal performance
    MinimizeMemory             = ENGINE_BIT(7)                 // minimize the amount of memory used during the build (may increase build time and decrease traversal performance)
  );

  struct AccelerationStructureDesc {
    uint64_t optimizedSize;                     // can be retrieved by "CmdWriteAccelerationStructuresSizes" and used for compaction via "CmdCopyAccelerationStructure"
    const BottomLevelGeometryDesc *geometries;       // needed only for "BOTTOM_LEVEL", "HAS_BUFFER" can be used to indicate a buffer presence (no real entities needed at initialization time)
    uint32_t geometryOrInstanceNum;
    AccelerationStructureBits flags;
    AccelerationStructureType type;
  };

  struct BindAccelerationStructureMemoryDesc {
    AccelerationStructure *accelerationStructure;
    Memory *memory;
    uint64_t offset;
  };

  struct BuildTopLevelAccelerationStructureDesc {
    AccelerationStructure *dst;
    const AccelerationStructure *src;    // implies "update" instead of "build" if provided (requires "ALLOW_UPDATE")
    uint32_t instanceNum;
    const Buffer *instanceBuffer;                    // contains "TopLevelInstance" entries
    uint64_t instanceOffset;
    Buffer *scratchBuffer;                           // use "GetAccelerationStructureBuildScratchBufferSize" or "GetAccelerationStructureUpdateScratchBufferSize" to determine the required size
    uint64_t scratchOffset;
  };

  struct BuildBottomLevelAccelerationStructureDesc {
    AccelerationStructure *dst;
    const AccelerationStructure *src;    // implies "update" instead of "build" if provided (requires "ALLOW_UPDATE")
    const BottomLevelGeometryDesc *geometries;
    uint32_t geometryNum;
    Buffer *scratchBuffer;
    uint64_t scratchOffset;
  };

  #pragma endregion

  //============================================================================================================================================================================================
  #pragma region [ Other ]
  //============================================================================================================================================================================================

  enum class CopyMode : uint8_t {
    Clone,
    Compact,
  };

  struct StridedBufferRegion {
    const Buffer *buffer;
    uint64_t offset;
    uint64_t size;
    uint64_t stride;
  };

  struct DispatchRaysDesc {
    StridedBufferRegion raygenShader;
    StridedBufferRegion missShaders;
    StridedBufferRegion hitShaderGroups;
    StridedBufferRegion callableShaders;

    uint32_t x, y, z;
  };

  struct DispatchRaysIndirectDesc {
    uint64_t raygenShaderRecordAddress;
    uint64_t raygenShaderRecordSize;

    uint64_t missShaderBindingTableAddress;
    uint64_t missShaderBindingTableSize;
    uint64_t missShaderBindingTableStride;

    uint64_t hitShaderBindingTableAddress;
    uint64_t hitShaderBindingTableSize;
    uint64_t hitShaderBindingTableStride;

    uint64_t callableShaderBindingTableAddress;
    uint64_t callableShaderBindingTableSize;
    uint64_t callableShaderBindingTableStride;

    uint32_t x, y, z;
  };

  #pragma endregion

  // Threadsafe: yes
  struct RayTracingInterface {
    // Create
    Result     (*createRayTracingPipeline)                        (Device &device, const RayTracingPipelineDesc &rayTracingPipelineDesc, Pipeline *&pipeline);
    Result     (*createAccelerationStructureDescriptor)           (const AccelerationStructure &accelerationStructure, Descriptor *&descriptor);

    // Get
    uint64_t        (*getAccelerationStructureHandle)                  (const AccelerationStructure &accelerationStructure);
    uint64_t        (*getAccelerationStructureUpdateScratchBufferSize) (const AccelerationStructure &accelerationStructure);
    uint64_t        (*getAccelerationStructureBuildScratchBufferSize)  (const AccelerationStructure &accelerationStructure);
    uint64_t        (*getMicromapBuildScratchBufferSize)               (const Micromap &micromap);

    // For barriers
    Buffer*  (*getAccelerationStructureBuffer)                  (const AccelerationStructure &accelerationStructure);
    Buffer*  (*getMicromapBuffer)                               (const Micromap &micromap);

    // Destroy
    void            (*destroyAccelerationStructure)                    (AccelerationStructure *accelerationStructure);
    void            (*destroyMicromap)                                 (Micromap *micromap);

    // Resources and memory (VK style)
    Result     (*createAccelerationStructure)                     (Device &device, const AccelerationStructureDesc &accelerationStructureDesc, AccelerationStructure *&accelerationStructure);
    Result     (*createMicromap)                                  (Device &device, const MicromapDesc &micromapDesc, Micromap *&micromap);
    void            (*getAccelerationStructureMemoryDesc)              (const AccelerationStructure &accelerationStructure, MemoryLocation memoryLocation, MemoryDesc &memoryDesc);
    void            (*getMicromapMemoryDesc)                           (const Micromap &micromap, MemoryLocation memoryLocation, MemoryDesc &memoryDesc);
    Result     (*bindAccelerationStructureMemory)                 (const BindAccelerationStructureMemoryDesc *bindAccelerationStructureMemoryDescs, uint32_t bindAccelerationStructureMemoryDescNum);
    Result     (*bindMicromapMemory)                              (const BindMicromapMemoryDesc *bindMicromapMemoryDescs, uint32_t bindMicromapMemoryDescNum);

    // Resources and memory (D3D12 style)
    void            (*getAccelerationStructureMemoryDesc2)             (const Device &device, const AccelerationStructureDesc &accelerationStructureDesc, MemoryLocation memoryLocation, MemoryDesc &memoryDesc); // requires "features.getMemoryDesc2"
    void            (*getMicromapMemoryDesc2)                          (const Device &device, const MicromapDesc &micromapDesc, MemoryLocation memoryLocation, MemoryDesc &memoryDesc); // requires "features.getMemoryDesc2"
    Result     (*createCommittedAccelerationStructure)            (Device &device, MemoryLocation memoryLocation, float priority, const AccelerationStructureDesc &accelerationStructureDesc, AccelerationStructure *&accelerationStructure);
    Result     (*createCommittedMicromap)                         (Device &device, MemoryLocation memoryLocation, float priority, const MicromapDesc &micromapDesc, Micromap *&micromap);
    Result     (*createPlacedAccelerationStructure)               (Device &device, Memory *memory, uint64_t offset, const AccelerationStructureDesc &accelerationStructureDesc, AccelerationStructure *&accelerationStructure);
    Result     (*createPlacedMicromap)                            (Device &device, Memory *memory, uint64_t offset, const MicromapDesc &micromapDesc, Micromap *&micromap);

    // Shader table
    // "dst" size must be >= "shaderGroupNum * rayTracingShaderGroupIdentifierSize" bytes
    // VK doesn't have a "local root signature" analog, thus stride = "rayTracingShaderGroupIdentifierSize", i.e. tight packing
    Result     (*writeShaderGroupIdentifiers)                     (const Pipeline &pipeline, uint32_t baseShaderGroupIndex, uint32_t shaderGroupNum, void* dst);

    // Command buffer
    // {
    // Micromap
    void        (*cmdBuildMicromaps)                               (CommandBuffer &commandBuffer, const BuildMicromapDesc *buildMicromapDescs, uint32_t buildMicromapDescNum);
    void        (*cmdWriteMicromapsSizes)                          (CommandBuffer &commandBuffer, const Micromap *const* micromaps, uint32_t micromapNum, QueryPool &queryPool, uint32_t queryPoolOffset);
    void        (*cmdCopyMicromap)                                 (CommandBuffer &commandBuffer, Micromap &dst, const Micromap &src, CopyMode copyMode);

    // Acceleration structure
    void        (*cmdBuildTopLevelAccelerationStructures)          (CommandBuffer &commandBuffer, const BuildTopLevelAccelerationStructureDesc *buildTopLevelAccelerationStructureDescs, uint32_t buildTopLevelAccelerationStructureDescNum);
    void        (*cmdBuildBottomLevelAccelerationStructures)       (CommandBuffer &commandBuffer, const BuildBottomLevelAccelerationStructureDesc *buildBottomLevelAccelerationStructureDescs, uint32_t buildBottomLevelAccelerationStructureDescNum);
    void        (*cmdWriteAccelerationStructuresSizes)             (CommandBuffer &commandBuffer, const AccelerationStructure *const* accelerationStructures, uint32_t accelerationStructureNum, QueryPool &queryPool, uint32_t queryPoolOffset);
    void        (*cmdCopyAccelerationStructure)                    (CommandBuffer &commandBuffer, AccelerationStructure &dst, const AccelerationStructure &src, CopyMode copyMode);

    // Ray tracing
    void        (*cmdDispatchRays)                                 (CommandBuffer &commandBuffer, const DispatchRaysDesc &dispatchRaysDesc);
    void        (*cmdDispatchRaysIndirect)                         (CommandBuffer &commandBuffer, const Buffer &buffer, uint64_t offset); // buffer contains "DispatchRaysIndirectDesc" commands
    // }

    // Native object
    uint64_t        (*getAccelerationStructureNativeObject)            (const AccelerationStructure &accelerationStructure); // ID3D12Resource* or VkAccelerationStructureKHR
    uint64_t        (*getMicromapNativeObject)                         (const Micromap &micromap);                           // ID3D12Resource* or VkMicromapEXT
  };
  /*clang-format on*/
}