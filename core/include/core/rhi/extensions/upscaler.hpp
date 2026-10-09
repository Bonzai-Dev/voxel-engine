// © 2025 NVIDIA Corporation

// Goal: providing easy-to-use access to modern upscalers: DLSS, FSR, XESS, NIS

#pragma once

#define NRI_UPSCALER_H 1

namespace Core::RHI {

  struct Upscaler;

  enum class UpscalerType : uint8_t {
    // Name                                     // Notes
    NIS,                        // NVIDIA Image Scaling                     sharpener-upscaler, cross vendor
    FSR,                        // AMD FidelityFX Super Resolution          upscaler, cross vendor
    XESS,                       // INTEL XeSS Super Resolution              upscaler, cross vendor
    DLSR,                       // NVIDIA Deep Learning Super Resolution    upscaler, NVIDIA only
    DLRR                        // NVIDIA Deep Learning Ray Reconstruction  upscaler-denoiser, NVIDIA only
  };

  enum class UpscalerMode : uint8_t {
    // Scaling factor       // Min jitter phases (or just use unclamped Halton2D)
    Native,                     // 1.0x                 8
    UltraQuality,              // 1.3x                 14
    Quality,                    // 1.5x                 18
    Balanced,                   // 1.7x                 23
    Performance,                // 2.0x                 32
    UltraPerformance           // 3.0x                 72
  };

  ENGINE_BITS(UpscalerBits, uint16_t,
    None                        = 0,
    HDR                         = ENGINE_BIT(0),            // "input" uses colors in High-Dynamic Range (HDR)
    SRGB                        = ENGINE_BIT(1),            // "input" uses Low-Dynamic Range (LDR) colors in sRGB space
    UseExposure                = ENGINE_BIT(2),            // "exposure" texture is provided (automatic exposure otherwise)
    UseReactive                = ENGINE_BIT(3),            // "reactive" texture is provided
    DepthInverted              = ENGINE_BIT(4),            // "depth" is inverted, i.e. the near plane is mapped to 1
    DepthInfinite              = ENGINE_BIT(5),            // "depth" uses INF far plane
    DepthLinear                = ENGINE_BIT(6),            // "depth" is linear viewZ (HW otherwise)
    MvUpscaled                 = ENGINE_BIT(7),            // "mv" are rendered at upscale resolution
    MvJittered                 = ENGINE_BIT(8)             // "mv" include jitter
  );

  ENGINE_BITS(DispatchUpscaleBits, uint8_t,
    None                        = 0,
    ResetHistory               = ENGINE_BIT(0),            // restart accumulation
    UseSpecularMotion         = ENGINE_BIT(1)             // ("DLRR" only) if set, "specularMvOrHitT" represents "specular motion" not "hit distance"
  );

  struct UpscalerDesc {
    Dim2_t upscaleResolution;                      // output resolution
    UpscalerType type;
    UpscalerMode mode;                             // not needed for NIS
    UpscalerBits flags;
    uint8_t preset;                         // preset for DLSR or XESS (0 default, >1 presets A, B, C...)
    CommandBuffer* commandBuffer;    // a non-copy-only command buffer in opened state, submission must be done manually ("wait for idle" executed, if not provided)
  };

  struct UpscalerProps {
    float scalingFactor;                                // per dimension scaling factor
    float mipBias;                                      // mip bias for materials textures, computed as "-log2(scalingFactor) - 1" (keep an eye on normal maps)
    Dim2_t upscaleResolution;                      // output resolution
    Dim2_t renderResolution;                       // optimal render resolution
    Dim2_t renderResolutionMin;                    // minimal render resolution (for Dynamic Resolution Scaling)
    uint8_t jitterPhaseNum;                             // minimal number of phases in the jitter sequence, computed as "ceil(8 * scalingFactor ^ 2)" ("Halton(2, 3)" recommended)
  };

  struct UpscalerResource {
    Texture* texture;
    Descriptor* descriptor;                      // "SHADER_RESOURCE" or "SHADER_RESOURCE_STORAGE", see comments below
  };

  // Guide buffers
  struct UpscalerGuides {                             // For FSR, XESS, DLSR
    UpscalerResource mv;                           // .xy - surface motion
    UpscalerResource depth;                        // .x - HW depth
    UpscalerResource exposure;         // .x - 1x1 exposure
    UpscalerResource reactive;         // .x - bias towards "input"
  };

  struct DenoiserGuides {                             // For DLRR
    UpscalerResource mv;                           // .xy - surface motion
    UpscalerResource depth;                        // .x - HW or linear depth
    UpscalerResource normalRoughness;              // .xyz - world-space normal (not encoded), .w - linear roughness
    UpscalerResource diffuseAlbedo;                // .xyz - diffuse albedo (LDR sky color for sky)
    UpscalerResource specularAlbedo;               // .xyz - specular albedo (environment BRDF)
    UpscalerResource specularMvOrHitT;             // .xy - specular virtual motion of the reflected world, or .x - specular hit distance otherwise
    UpscalerResource exposure;         // .x - 1x1 exposure
    UpscalerResource reactive;         // .x - bias towards "input"
    UpscalerResource sss;              // .x - subsurface scattering, computed as "Luminance(colorAfterSSS - colorBeforeSSS)"
  };

  // Settings
  struct NISSettings {
    float sharpness;                                    // [0; 1]
  };

  struct FSRSettings {
    float zNear;                                        // distance to the near plane (units)
    float zFar;                                         // distance to the far plane, unused if "DEPTH_INFINITE" is set (units)
    float verticalFov;                                  // vertical field of view angle (radians)
    float frameTime;                                    // the time elapsed since the last frame (ms)
    float viewSpaceToMetersFactor;                      // for converting view space units to meters (m/unit)
    float sharpness;                                    // [0; 1]
  };

  struct DLRRSettings {
    float worldToViewMatrix[16];                        // {Xx, Yx, Zx, 0, Xy, Yy, Zy, 0, Xz, Yz, Zz, 0, Tx, Ty, Tz, 1}, where {X, Y, Z} - axises, T - translation
    float viewToClipMatrix[16];                         // {-, -, -, 0, -, -, -, 0, -, -, -, A, -, -, -, B}, where {A; B} = {0; 1} for ortho or {-1/+1; 0} for perspective projections
  };

  struct DispatchUpscaleDesc {
    // Output (required "SHADER_RESOURCE_STORAGE" for resource state & descriptor)
    UpscalerResource output;                       // .xyz - upscaled RGB color

    // Input (required "SHADER_RESOURCE" for resource state & descriptor)
    UpscalerResource input;                        // .xyz - input RGB color

    // Guides (required "SHADER_RESOURCE" for resource states & descriptors)
    union {                                             // Choosen based on "UpscalerType" passed during creation
      UpscalerGuides upscaler;                   //      FSR, XESS, DLSR
      DenoiserGuides denoiser;                   //      DLRR (sRGB not supported)
    } guides;

    // Settings
    union {                                             // Choosen based on "UpscalerType" passed during creation
      NISSettings nis;                           //      NIS settings
      FSRSettings fsr;                           //      FSR settings
      DLRRSettings dlrr;                         //      DLRR settings
    } settings;

    Dim2_t currentResolution;                      // current render resolution for inputs and guides, renderResolutionMin <= currentResolution <= renderResolution
    Float2_t cameraJitter;                         // pointing towards the pixel center, in [-0.5; 0.5] range
    Float2_t mvScale;                              // used to convert motion vectors to pixel space
    DispatchUpscaleBits flags;
  };

  // Threadsafe: yes
  struct UpscalerInterface {
    Result     (*createUpscaler)          (Device& device, const UpscalerDesc& upscalerDesc, Upscaler*& upscaler);
    void            (*destroyUpscaler)         (Upscaler* upscaler);

    bool            (*isUpscalerSupported)     (const Device& device, UpscalerType type);
    void            (*getUpscalerProps)        (const Upscaler& upscaler, UpscalerProps& upscalerProps);

    // Command buffer
    // {
    // Dispatch (changes descriptor pool, pipeline layout and pipeline, barriers are externally controlled)
    void        (*cmdDispatchUpscale)      (CommandBuffer& commandBuffer, Upscaler& upscaler, const DispatchUpscaleDesc& dispatchUpscaleDesc);
    // }
  };
}