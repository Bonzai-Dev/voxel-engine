#pragma once

namespace Core::RHI {
  /* clang-format off */
  ENGINE_BITS(RayTracingPipelineBits, uint8_t,
    None                        = 0,
    SkipTriangles               = ENGINE_BIT(0), // provides knowledge that "triangles" doesn't need to be considered
    SkipAABBS                   = ENGINE_BIT(1), // provides knowledge that "aabbs" doesn't need to be considered
    AllowMicromaps              = ENGINE_BIT(2), // specifies that the ray tracing pipeline can be used with acceleration structures which reference micromaps
    FailOnCacheMiss             = ENGINE_BIT(3)  // "CreateRayTracingPipeline" returns "FAILURE" if the pipeline is not found in the supplied cache (requires "features.pipelineCacheControl")
  );
/* clang-format on */

  struct RayTracingPipelineInfo {
    // const PipelineLayout *pipelineLayout;
    // const ShaderLibraryDesc *shaderLibrary;
    // const NriPtr(ShaderGroupDesc) shaderGroups;
    // uint32_t shaderGroupNum;
    // uint32_t recursionMaxDepth;
    // uint32_t rayPayloadMaxSize;
    // uint32_t rayHitAttributeMaxSize;
    // Nri(RayTracingPipelineBits) flags;
    // Nri(Robustness) robustness;
    // NriOptional const NriPtr(PipelineCache) cache; // if non-NULL, pipeline creation can be served from a cached blob and the result will be added to the cache on a miss
  };
}