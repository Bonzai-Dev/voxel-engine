#pragma once
#include <glad/gl.h>
#include <string>
#include <vector>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

namespace Renderer {
  enum class ShaderType {
    Vertex,
    Fragment,
  };

  enum class MeshFillMode {
    Wireframe,
    Solid
  };

  struct MeshVertex {
    glm::vec3 position;
  };

  using Indices = std::vector<unsigned int>;
  using MeshVertexData = std::vector<MeshVertex>;

  struct MeshData {
    MeshVertexData vertexData;
    Indices indices;
  };

  void initialize();

  void clearBuffer(glm::vec4 color);

  int getUniform(const char *name, unsigned int shaderProgram);

  void setFillMode(MeshFillMode fillMode);

  inline std::unordered_map<std::string, unsigned int> m_uniforms;

  unsigned int compileShader(const char *filepath, ShaderType type);

  unsigned int createShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath);

  unsigned int createVertexBufferObject(const void *data, size_t size);

  unsigned int createVertexArrayObject();

  unsigned int createElementBufferObject(const void *data, size_t size);

  void useElementBufferObject(unsigned int buffer);

  void useVertexArrayObject(unsigned int object);

  void useVertexBufferObject(unsigned int buffer);

  template <typename TypeT>
  void setVertexData(size_t index, size_t size, size_t stride, bool normalized, size_t offset) {
    const size_t typeSize = sizeof(TypeT);
    glVertexAttribPointer(index, size, GL_FLOAT, normalized, stride * typeSize, reinterpret_cast<void *>(offset * typeSize));
    glEnableVertexAttribArray(index);
  }

  void debugMessageCallback(
    unsigned int source,
    unsigned int type,
    unsigned int id,
    unsigned int severity,
    int length,
    const char *message,
    const void *userParam
  );

  void useTexture(unsigned int texture);

  unsigned int loadPng(const char *filepath);
}
