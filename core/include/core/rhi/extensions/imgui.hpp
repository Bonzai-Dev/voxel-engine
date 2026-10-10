// © 2025 NVIDIA Corporation

// Goal: ImGui rendering

#pragma once

#define NRI_IMGUI_H 1

/*
Requirements:
- ImGui 1.92+ with "ImGuiBackendFlags_RendererHasTextures" flag ("IMGUI_DISABLE_OBSOLETE_FUNCTIONS" is recommended)
- unmodified "ImDrawVert" (20 bytes) and "ImDrawIdx" (2 bytes)
- "ImTextureID_Invalid" = 0

Expected usage:
- the goal of this extension is to support latest ImGui only
- designed only for rendering
- "drawList->AddCallback" functionality is not supported! But there is a special callback, allowing to override "hdrScale":
     drawList->AddCallback(NRI_IMGUI_OVERRIDE_HDR_SCALE(1000.0f)); // to override "DrawImguiDesc::hdrScale"
     drawList->AddCallback(NRI_IMGUI_OVERRIDE_HDR_SCALE(0.0f));    // to revert back to "DrawImguiDesc::hdrScale"
- "ImGui::Image*" functions are supported. "ImTextureID" must be a "SHADER_RESOURCE" descriptor:
     ImGui::Image((ImTextureID)descriptor, ...)
*/

struct ImDrawList;
struct ImTextureData;

namespace Core::RHI {
  struct Imgui;
  struct Streamer;

  struct ImguiDesc {
    RHI_OPTIONAL uint32_t descriptorPoolSize;    // upper bound of textures used by Imgui for drawing: {number of queued frames} * {number of "CmdDrawImgui" calls} * (1 + {"drawList->AddImage*" calls})
  };

  struct CopyImguiDataDesc {
    const ImDrawList* const* drawLists;         // ImDrawData::CmdLists.Data
    uint32_t drawListNum;                       // ImDrawData::CmdLists.Size
    ImTextureData* const* textures;             // ImDrawData::Textures->Data (same as "ImGui::GetPlatformIO().Textures.Data")
    uint32_t textureNum;                        // ImDrawData::Textures->Size (same as "ImGui::GetPlatformIO().Textures.Size")
  };

  struct DrawImguiDesc {
    const ImDrawList* const* drawLists;         // ImDrawData::CmdLists.Data (same as for "CopyImguiDataDesc")
    uint32_t drawListNum;                       // ImDrawData::CmdLists.Size (same as for "CopyImguiDataDesc")
    Dim2_t displaySize;                    // ImDrawData::DisplaySize
    float hdrScale;                             // SDR intensity in HDR mode (1 by default)
    Format attachmentFormat;               // destination attachment (render target) format
    bool linearColor;                           // apply de-gamma to vertex colors (needed for sRGB attachments and HDR)
  };

  // Threadsafe: yes
  struct ImguiInterface {
    Result (*CreateImgui)         (Device &device, const ImguiDesc &imguiDesc, RHI_OUT Imgui *&imgui);
    void        (*DestroyImgui)        (Imgui *imgui);

    // Command buffer
    // {
    // Copy
    void    (*CmdCopyImguiData)    (CommandBuffer &commandBuffer, Streamer &streamer, Imgui &imgui, const CopyImguiDataDesc &streamImguiDesc);

    // Draw (changes descriptor pool, pipeline layout and pipeline, barriers are externally controlled)
    void    (*CmdDrawImgui)        (CommandBuffer &commandBuffer, Imgui &imgui, const DrawImguiDesc &drawImguiDesc);
    // }
  };
}

#define NRI_IMGUI_OVERRIDE_HDR_SCALE(hdrScale) (ImDrawCallback)1, _NriCastFloatToVoidPtr(hdrScale)

inline void* _NriCastFloatToVoidPtr(float f) {
  // A strange cast is there to get a fast path in Imgui
  return *(void**)&f;
}
