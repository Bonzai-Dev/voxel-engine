#pragma once

namespace Core::RHI {
  constexpr VkColorComponentFlags getVulkanColorComponent(ColorWriteBits colorWriteMask) {
    return VkColorComponentFlags(colorWriteMask & ColorWriteBits::RGBA);
  }

  constexpr std::array<VkLogicOp, (size_t)LogicOp::Count> vulkanLogicOps = {
    VK_LOGIC_OP_MAX_ENUM, // NONE
    VK_LOGIC_OP_CLEAR, // CLEAR
    VK_LOGIC_OP_AND, // AND
    VK_LOGIC_OP_AND_REVERSE, // AND_REVERSE
    VK_LOGIC_OP_COPY, // COPY
    VK_LOGIC_OP_AND_INVERTED, // AND_INVERTED
    VK_LOGIC_OP_XOR, // XOR
    VK_LOGIC_OP_OR, // OR
    VK_LOGIC_OP_NOR, // NOR
    VK_LOGIC_OP_EQUIVALENT, // EQUIVALENT
    VK_LOGIC_OP_INVERT, // INVERT
    VK_LOGIC_OP_OR_REVERSE, // OR_REVERSE
    VK_LOGIC_OP_COPY_INVERTED, // COPY_INVERTED
    VK_LOGIC_OP_OR_INVERTED, // OR_INVERTED
    VK_LOGIC_OP_NAND, // NAND
    VK_LOGIC_OP_SET // SET
  };

  constexpr VkLogicOp getVulkanLogicOp(LogicOp logicOp) {
    return vulkanLogicOps[(size_t)logicOp];
  }

  constexpr std::array<VkBlendFactor, (size_t)BlendFactor::Count> vulkanBlendFactors = {
    VK_BLEND_FACTOR_ZERO, // ZERO
    VK_BLEND_FACTOR_ONE, // ONE
    VK_BLEND_FACTOR_SRC_COLOR, // SRC_COLOR
    VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, // ONE_MINUS_SRC_COLOR
    VK_BLEND_FACTOR_DST_COLOR, // DST_COLOR
    VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR, // ONE_MINUS_DST_COLOR
    VK_BLEND_FACTOR_SRC_ALPHA, // SRC_ALPHA
    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // ONE_MINUS_SRC_ALPHA
    VK_BLEND_FACTOR_DST_ALPHA, // DST_ALPHA
    VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, // ONE_MINUS_DST_ALPHA
    VK_BLEND_FACTOR_CONSTANT_COLOR, // CONSTANT_COLOR
    VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR, // ONE_MINUS_CONSTANT_COLOR
    VK_BLEND_FACTOR_CONSTANT_ALPHA, // CONSTANT_ALPHA
    VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA, // ONE_MINUS_CONSTANT_ALPHA
    VK_BLEND_FACTOR_SRC_ALPHA_SATURATE, // SRC_ALPHA_SATURATE
    VK_BLEND_FACTOR_SRC1_COLOR, // SRC1_COLOR
    VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR, // ONE_MINUS_SRC1_COLOR
    VK_BLEND_FACTOR_SRC1_ALPHA, // SRC1_ALPHA
    VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA, // ONE_MINUS_SRC1_ALPHA
  };

  constexpr VkBlendFactor getVulkanBlendFactor(BlendFactor blendFactor) {
    return vulkanBlendFactors[(size_t)blendFactor];
  }

  constexpr std::array<VkBlendOp, (size_t)BlendOp::Count> vulkanBlendOps = {
    VK_BLEND_OP_ADD, // ADD
    VK_BLEND_OP_SUBTRACT, // SUBTRACT
    VK_BLEND_OP_REVERSE_SUBTRACT, // REVERSE_SUBTRACT
    VK_BLEND_OP_MIN, // MIN
    VK_BLEND_OP_MAX // MAX
  };

  constexpr VkBlendOp getVulkanBlendOp(BlendOp blendFunc) {
    return vulkanBlendOps[(size_t)blendFunc];
  }

  constexpr std::array<VkStencilOp, (size_t)StencilOp::Count> vulkanStencilOps = {
    VK_STENCIL_OP_KEEP, // KEEP
    VK_STENCIL_OP_ZERO, // ZERO
    VK_STENCIL_OP_REPLACE, // REPLACE
    VK_STENCIL_OP_INCREMENT_AND_CLAMP, // INCREMENT_AND_CLAMP
    VK_STENCIL_OP_DECREMENT_AND_CLAMP, // DECREMENT_AND_CLAMP
    VK_STENCIL_OP_INVERT, // INVERT
    VK_STENCIL_OP_INCREMENT_AND_WRAP, // INCREMENT_AND_WRAP
    VK_STENCIL_OP_DECREMENT_AND_WRAP // DECREMENT_AND_WRAP
  };

  constexpr VkStencilOp getVulkanStencilOps(StencilOp stencilFunc) {
    return vulkanStencilOps[(size_t)stencilFunc];
  }

  constexpr std::array<VkCompareOp, (size_t)CompareOp::Count> vulkanCompareOps = {
    VK_COMPARE_OP_NEVER, // NONE
    VK_COMPARE_OP_ALWAYS, // ALWAYS
    VK_COMPARE_OP_NEVER, // NEVER
    VK_COMPARE_OP_EQUAL, // EQUAL
    VK_COMPARE_OP_NOT_EQUAL, // NOT_EQUAL
    VK_COMPARE_OP_LESS, // LESS
    VK_COMPARE_OP_LESS_OR_EQUAL, // LESS_EQUAL
    VK_COMPARE_OP_GREATER, // GREATER
    VK_COMPARE_OP_GREATER_OR_EQUAL, // GREATER_EQUAL
  };

  constexpr VkCompareOp getVulkanCompareOps(CompareOp compareOp) {
    return vulkanCompareOps[(size_t)compareOp];
  }

  constexpr VkPipelineStageFlags2 getVulkanPipelineStageFlags(StageBits stageBits) {
    // Check non-mask values first
    if (stageBits == StageBits::All)
      return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    if (stageBits == StageBits::None)
      return VK_PIPELINE_STAGE_2_NONE;

    // Gather bits
    VkPipelineStageFlags2 flags = 0;

    if (stageBits & StageBits::IndexInput)
      flags |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;

    if (stageBits & StageBits::VertexShader)
      flags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;

    if (stageBits & StageBits::TessellationControlShader)
      flags |= VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT;

    if (stageBits & StageBits::TessellationEvaluationShader)
      flags |= VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT;

    if (stageBits & StageBits::GeometryShader)
      flags |= VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT;

    if (stageBits & StageBits::TaskShader)
      flags |= VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT;

    if (stageBits & StageBits::MeshShader)
      flags |= VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;

    if (stageBits & StageBits::FragmentShader)
      flags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    if (stageBits & StageBits::DepthStencilAttachment)
      flags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    // TODO: separate?

    if (stageBits & StageBits::ColorAttachment)
      flags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    if (stageBits & StageBits::ShadingRateAttachment)
      flags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;

    if (stageBits & StageBits::ComputeShader)
      flags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

    if (stageBits & StageBits::RayTracingShaders)
      flags |= VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;

    if (stageBits & StageBits::Indirect)
      flags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;

    if (stageBits & StageBits::Copy)
      flags |= VK_PIPELINE_STAGE_2_COPY_BIT;

    if (stageBits & StageBits::Resolve)
      flags |= VK_PIPELINE_STAGE_2_RESOLVE_BIT;

    if (stageBits & StageBits::ClearStorage)
      flags |= VK_PIPELINE_STAGE_2_CLEAR_BIT;

    if (stageBits & StageBits::AccelerationStructure)
      flags |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    // already includes "VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR" (more strict according to the spec)

    if (stageBits & StageBits::Micromap)
      flags |= VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT;

    return flags;
  }

  constexpr VkShaderStageFlags getVulkanShaderStageFlags(StageBits stage) {
    // Check non-mask values first
    if (stage == StageBits::All)
      return VK_SHADER_STAGE_ALL;

    if (stage == StageBits::None)
      return 0;

    // Gather bits
    VkShaderStageFlags stageFlags = 0;

    if (stage & StageBits::VertexShader)
      stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;

    if (stage & StageBits::TessellationControlShader)
      stageFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

    if (stage & StageBits::TessellationEvaluationShader)
      stageFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

    if (stage & StageBits::GeometryShader)
      stageFlags |= VK_SHADER_STAGE_GEOMETRY_BIT;

    if (stage & StageBits::FragmentShader)
      stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;

    if (stage & StageBits::ComputeShader)
      stageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;

    if (stage & StageBits::RayGenShader)
      stageFlags |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    if (stage & StageBits::MissShader)
      stageFlags |= VK_SHADER_STAGE_MISS_BIT_KHR;

    if (stage & StageBits::IntersectionShader)
      stageFlags |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;

    if (stage & StageBits::ClosestHitShader)
      stageFlags |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

    if (stage & StageBits::AnyHitShader)
      stageFlags |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;

    if (stage & StageBits::CallableShader)
      stageFlags |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;

    if (stage & StageBits::TaskShader)
      stageFlags |= VK_SHADER_STAGE_TASK_BIT_EXT;

    if (stage & StageBits::MeshShader)
      stageFlags |= VK_SHADER_STAGE_MESH_BIT_EXT;

    return stageFlags;
  }

  constexpr std::array<VkPrimitiveTopology, (size_t)Topology::Count> vulkanTopologies = {
    VK_PRIMITIVE_TOPOLOGY_POINT_LIST, // POINT_LIST
    VK_PRIMITIVE_TOPOLOGY_LINE_LIST, // LINE_LIST
    VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, // LINE_STRIP
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, // TRIANGLE_LIST
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, // TRIANGLE_STRIP
    VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY, // LINE_LIST_WITH_ADJACENCY
    VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY, // LINE_STRIP_WITH_ADJACENCY
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY, // TRIANGLE_LIST_WITH_ADJACENCY
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY, // TRIANGLE_STRIP_WITH_ADJACENCY
    VK_PRIMITIVE_TOPOLOGY_PATCH_LIST // PATCH_LIST
  };

  constexpr VkPrimitiveTopology getVulkanTopology(Topology topology) {
    return vulkanTopologies[(size_t)topology];
  }

  constexpr std::array<VkCullModeFlags, (size_t)CullMode::Count> vulkanCullModes = {
    VK_CULL_MODE_NONE, // NONE
    VK_CULL_MODE_FRONT_BIT, // FRONT
    VK_CULL_MODE_BACK_BIT // BACK
  };

  constexpr VkCullModeFlags getVulkanCullMode(CullMode cullMode) {
    return vulkanCullModes[(size_t)cullMode];
  }

  constexpr std::array<VkPolygonMode, (size_t)FillMode::Count> vulkanFillModes = {
    VK_POLYGON_MODE_FILL, // SOLID
    VK_POLYGON_MODE_LINE, // WIREFRAME
  };

  constexpr VkPolygonMode getVulkanPolygonMode(FillMode fillMode) {
    return vulkanFillModes[(size_t)fillMode];
  }

  // Each depth/stencil format is only compatible with itself in VK
  constexpr std::array<VkFormat, static_cast<size_t>(DataFormat::Count)> vulkanFormats = {
    VK_FORMAT_UNDEFINED, // UNKNOWN
    VK_FORMAT_R8_UNORM, // R8_UNORM
    VK_FORMAT_R8_SNORM, // R8_SNORM
    VK_FORMAT_R8_UINT, // R8_UINT
    VK_FORMAT_R8_SINT, // R8_SINT
    VK_FORMAT_R8G8_UNORM, // RG8_UNORM
    VK_FORMAT_R8G8_SNORM, // RG8_SNORM
    VK_FORMAT_R8G8_UINT, // RG8_UINT
    VK_FORMAT_R8G8_SINT, // RG8_SINT
    VK_FORMAT_B8G8R8A8_UNORM, // BGRA8_UNORM
    VK_FORMAT_B8G8R8A8_SRGB, // BGRA8_SRGB
    VK_FORMAT_R8G8B8A8_UNORM, // RGBA8_UNORM
    VK_FORMAT_R8G8B8A8_SRGB, // RGBA8_SRGB
    VK_FORMAT_R8G8B8A8_SNORM, // RGBA8_SNORM
    VK_FORMAT_R8G8B8A8_UINT, // RGBA8_UINT
    VK_FORMAT_R8G8B8A8_SINT, // RGBA8_SINT
    VK_FORMAT_R16_UNORM, // R16_UNORM
    VK_FORMAT_R16_SNORM, // R16_SNORM
    VK_FORMAT_R16_UINT, // R16_UINT
    VK_FORMAT_R16_SINT, // R16_SINT
    VK_FORMAT_R16_SFLOAT, // R16_SFLOAT
    VK_FORMAT_R16G16_UNORM, // RG16_UNORM
    VK_FORMAT_R16G16_SNORM, // RG16_SNORM
    VK_FORMAT_R16G16_UINT, // RG16_UINT
    VK_FORMAT_R16G16_SINT, // RG16_SINT
    VK_FORMAT_R16G16_SFLOAT, // RG16_SFLOAT
    VK_FORMAT_R16G16B16A16_UNORM, // RGBA16_UNORM
    VK_FORMAT_R16G16B16A16_SNORM, // RGBA16_SNORM
    VK_FORMAT_R16G16B16A16_UINT, // RGBA16_UINT
    VK_FORMAT_R16G16B16A16_SINT, // RGBA16_SINT
    VK_FORMAT_R16G16B16A16_SFLOAT, // RGBA16_SFLOAT
    VK_FORMAT_R32_UINT, // R32_UINT
    VK_FORMAT_R32_SINT, // R32_SINT
    VK_FORMAT_R32_SFLOAT, // R32_SFLOAT
    VK_FORMAT_R32G32_UINT, // RG32_UINT
    VK_FORMAT_R32G32_SINT, // RG32_SINT
    VK_FORMAT_R32G32_SFLOAT, // RG32_SFLOAT
    VK_FORMAT_R32G32B32_UINT, // RGB32_UINT
    VK_FORMAT_R32G32B32_SINT, // RGB32_SINT
    VK_FORMAT_R32G32B32_SFLOAT, // RGB32_SFLOAT
    VK_FORMAT_R32G32B32A32_UINT, // RGB32_UINT
    VK_FORMAT_R32G32B32A32_SINT, // RGB32_SINT
    VK_FORMAT_R32G32B32A32_SFLOAT, // RGB32_SFLOAT
    VK_FORMAT_R5G6B5_UNORM_PACK16, // B5_G6_R5_UNORM
    VK_FORMAT_A1R5G5B5_UNORM_PACK16, // B5_G5_R5_A1_UNORM
    VK_FORMAT_A4R4G4B4_UNORM_PACK16, // B4_G4_R4_A4_UNORM
    VK_FORMAT_A2B10G10R10_UNORM_PACK32, // R10_G10_B10_A2_UNORM
    VK_FORMAT_A2B10G10R10_UINT_PACK32, // R10_G10_B10_A2_UINT
    VK_FORMAT_B10G11R11_UFLOAT_PACK32, // R11_G11_B10_UFLOAT
    VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, // R9_G9_B9_E5_UFLOAT
    VK_FORMAT_BC1_RGBA_UNORM_BLOCK, // BC1_RGBA_UNORM
    VK_FORMAT_BC1_RGBA_SRGB_BLOCK, // BC1_RGBA_SRGB
    VK_FORMAT_BC2_UNORM_BLOCK, // BC2_RGBA_UNORM
    VK_FORMAT_BC2_SRGB_BLOCK, // BC2_RGBA_SRGB
    VK_FORMAT_BC3_UNORM_BLOCK, // BC3_RGBA_UNORM
    VK_FORMAT_BC3_SRGB_BLOCK, // BC3_RGBA_SRGB
    VK_FORMAT_BC4_UNORM_BLOCK, // BC4_R_UNORM
    VK_FORMAT_BC4_SNORM_BLOCK, // BC4_R_SNORM
    VK_FORMAT_BC5_UNORM_BLOCK, // BC5_RG_UNORM
    VK_FORMAT_BC5_SNORM_BLOCK, // BC5_RG_SNORM
    VK_FORMAT_BC6H_UFLOAT_BLOCK, // BC6H_RGB_UFLOAT
    VK_FORMAT_BC6H_SFLOAT_BLOCK, // BC6H_RGB_SFLOAT
    VK_FORMAT_BC7_UNORM_BLOCK, // BC7_RGBA_UNORM
    VK_FORMAT_BC7_SRGB_BLOCK, // BC7_RGBA_SRGB
    VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, // ETC2_RGB8_UNORM
    VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK, // ETC2_RGB8_SRGB
    VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, // ETC2_RGB8_A1_UNORM
    VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK, // ETC2_RGB8_A1_SRGB
    VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, // ETC2_RGB8_A8_UNORM
    VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK, // ETC2_RGB8_A8_SRGB
    VK_FORMAT_EAC_R11_UNORM_BLOCK, // ETC2_R11_UNORM
    VK_FORMAT_EAC_R11_SNORM_BLOCK, // ETC2_R11_SNORM
    VK_FORMAT_EAC_R11G11_UNORM_BLOCK, // ETC2_R11_G11_UNORM
    VK_FORMAT_EAC_R11G11_SNORM_BLOCK, // ETC2_R11_G11_SNORM
    VK_FORMAT_ASTC_4x4_UNORM_BLOCK, // ASTC_4X4_UNORM
    VK_FORMAT_ASTC_4x4_SRGB_BLOCK, // ASTC_4X4_SRGB
    VK_FORMAT_ASTC_5x4_UNORM_BLOCK, // ASTC_5X4_UNORM
    VK_FORMAT_ASTC_5x4_SRGB_BLOCK, // ASTC_5X4_SRGB
    VK_FORMAT_ASTC_5x5_UNORM_BLOCK, // ASTC_5X5_UNORM
    VK_FORMAT_ASTC_5x5_SRGB_BLOCK, // ASTC_5X5_SRGB
    VK_FORMAT_ASTC_6x5_UNORM_BLOCK, // ASTC_6X5_UNORM
    VK_FORMAT_ASTC_6x5_SRGB_BLOCK, // ASTC_6X5_SRGB
    VK_FORMAT_ASTC_6x6_UNORM_BLOCK, // ASTC_6X6_UNORM
    VK_FORMAT_ASTC_6x6_SRGB_BLOCK, // ASTC_6X6_SRGB
    VK_FORMAT_ASTC_8x5_UNORM_BLOCK, // ASTC_8X5_UNORM
    VK_FORMAT_ASTC_8x5_SRGB_BLOCK, // ASTC_8X5_SRGB
    VK_FORMAT_ASTC_8x6_UNORM_BLOCK, // ASTC_8X6_UNORM
    VK_FORMAT_ASTC_8x6_SRGB_BLOCK, // ASTC_8X6_SRGB
    VK_FORMAT_ASTC_8x8_UNORM_BLOCK, // ASTC_8X8_UNORM
    VK_FORMAT_ASTC_8x8_SRGB_BLOCK, // ASTC_8X8_SRGB
    VK_FORMAT_ASTC_10x5_UNORM_BLOCK, // ASTC_10X5_UNORM
    VK_FORMAT_ASTC_10x5_SRGB_BLOCK, // ASTC_10X5_SRGB
    VK_FORMAT_ASTC_10x6_UNORM_BLOCK, // ASTC_10X6_UNORM
    VK_FORMAT_ASTC_10x6_SRGB_BLOCK, // ASTC_10X6_SRGB
    VK_FORMAT_ASTC_10x8_UNORM_BLOCK, // ASTC_10X8_UNORM
    VK_FORMAT_ASTC_10x8_SRGB_BLOCK, // ASTC_10X8_SRGB
    VK_FORMAT_ASTC_10x10_UNORM_BLOCK, // ASTC_10X10_UNORM
    VK_FORMAT_ASTC_10x10_SRGB_BLOCK, // ASTC_10X10_SRGB
    VK_FORMAT_ASTC_12x10_UNORM_BLOCK, // ASTC_12X10_UNORM
    VK_FORMAT_ASTC_12x10_SRGB_BLOCK, // ASTC_12X10_SRGB
    VK_FORMAT_ASTC_12x12_UNORM_BLOCK, // ASTC_12X12_UNORM
    VK_FORMAT_ASTC_12x12_SRGB_BLOCK, // ASTC_12X12_SRGB
    VK_FORMAT_D16_UNORM, // D16_UNORM
    VK_FORMAT_D32_SFLOAT, // D32_SFLOAT
    VK_FORMAT_D24_UNORM_S8_UINT, // D24_UNORM_S8_UINT
    VK_FORMAT_D32_SFLOAT_S8_UINT, // D32_SFLOAT_S8_UINT
  };

  ENGINE_FORCE_INLINE std::string vulkanResultToString(VkResult result) {
    switch (result) {
      case VK_SUCCESS: return "VK_SUCCESS";
      case VK_NOT_READY: return "VK_NOT_READY";
      case VK_TIMEOUT: return "VK_TIMEOUT";
      case VK_EVENT_SET: return "VK_EVENT_SET";
      case VK_EVENT_RESET: return "VK_EVENT_RESET";
      case VK_INCOMPLETE: return "VK_INCOMPLETE";
      case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
      case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
      case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
      case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
      case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
      case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
      case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
      case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
      case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
      case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
      case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
      case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
      case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
      case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
      case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
      case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
      case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
      case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
      case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
      case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
      case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
      case VK_ERROR_NOT_PERMITTED_EXT: return "VK_ERROR_NOT_PERMITTED_EXT";
      default:
        return "Unknown VkResult: " + std::to_string(result);
    }
  }

  constexpr Result vulkanResultToResult(VkResult vkResult) {
    if (vkResult >= 0)
      return Result::Success;

    switch (vkResult) {
      case VK_ERROR_DEVICE_LOST:
        return Result::DeviceLost;

      case VK_ERROR_SURFACE_LOST_KHR:
      case VK_ERROR_OUT_OF_DATE_KHR:
        return Result::OutOfDate;

      case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
      case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
      case VK_ERROR_FORMAT_NOT_SUPPORTED:
      case VK_ERROR_INCOMPATIBLE_DRIVER:
      case VK_ERROR_FEATURE_NOT_PRESENT:
      case VK_ERROR_EXTENSION_NOT_PRESENT:
      case VK_ERROR_LAYER_NOT_PRESENT:
        return Result::Unsupported;

      case VK_ERROR_OUT_OF_HOST_MEMORY:
      case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      case VK_ERROR_OUT_OF_POOL_MEMORY:
      case VK_ERROR_FRAGMENTATION:
      case VK_ERROR_FRAGMENTED_POOL:
        return Result::OutOfMemory;

      default:
        return Result::Failure;
    }
  }

  constexpr VkFormat getVulkanFormat(DataFormat format, bool demoteSrgb = false) {
    if (demoteSrgb) {
      const FormatProperties &formatProps = getFormatProperties(format);
      if (formatProps.isSrgb)
        format = static_cast<DataFormat>(static_cast<uint32_t>(format) - 1);
    }
    return static_cast<VkFormat>(static_cast<uint32_t>(vulkanFormats[static_cast<uint32_t>(format)]));
  }

  constexpr std::array vulkanImageDimensions = {
    VK_IMAGE_TYPE_1D, // TEXTURE_1D
    VK_IMAGE_TYPE_2D, // TEXTURE_2D
    VK_IMAGE_TYPE_3D, // TEXTURE_3D
  };

  constexpr VkImageType getVulkanImageType(TextureDimension dimension) {
    return vulkanImageDimensions[static_cast<size_t>(dimension)];
  }
}
