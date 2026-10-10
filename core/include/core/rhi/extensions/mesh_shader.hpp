// © 2021 NVIDIA Corporation

// Goal: mesh shaders
// https://www.khronos.org/blog/mesh-shading-for-vulkan
// https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html

#pragma once

#define NRI_MESH_SHADER_H 1

namespace Core::RHI {
  struct DrawMeshTasksDesc {
    uint32_t x, y, z;
  };

  // Threadsafe: no
  struct MeshShaderInterface {
    // Command buffer
    // {
    // Draw
    void    (*CmdDrawMeshTasks)            (CommandBuffer &commandBuffer, const DrawMeshTasksDesc &drawMeshTasksDesc);
    void    (*CmdDrawMeshTasksIndirect)    (CommandBuffer &commandBuffer, const Buffer &buffer, uint64_t offset, uint32_t drawNum, uint32_t stride, RHI_OPTIONAL  const Buffer *countBuffer, uint64_t countBufferOffset); // buffer contains "DrawMeshTasksDesc" commands
    // }
  };
}