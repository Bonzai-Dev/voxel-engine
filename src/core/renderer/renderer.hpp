#pragma once
#include <cmath>
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

  void deleteVertexArrayObject(unsigned int object);

  void deleteVertexBufferObject(unsigned int buffer);

  void deleteElementBufferObject(unsigned int buffer);

  void unbindVertexArrayObject();

  void unbindVertexBufferObject();

  void unbindElementBufferObject();

  void drawTriangles(unsigned int vertexArrayObject, unsigned int vertexBuffer, unsigned int indexBuffer, size_t indicesSize);

  template <typename TypeT>
  constexpr GLenum getGlType() {
    if (std::is_same_v<TypeT, std::float_t>)
      return GL_FLOAT;
    if (std::is_same_v<TypeT, std::double_t>)
      return GL_DOUBLE;
    if (std::is_same_v<TypeT, std::uint32_t>)
      return GL_UNSIGNED_INT;
    if (std::is_same_v<TypeT, std::int32_t>)
      return GL_INT;
    return GL_INVALID_ENUM;
  }

  template <typename TypeT>
  void setVertexData(size_t index, size_t size, size_t stride, bool normalized, size_t offset, unsigned int vertexBuffer) {
    const size_t typeSize = sizeof(TypeT);
    useVertexBufferObject(vertexBuffer);
    glVertexAttribPointer(index, size, getGlType<TypeT>(), normalized, stride * typeSize, reinterpret_cast<void *>(offset * typeSize));
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

  unsigned int loadCubemap(std::vector<std::string_view> faces);

  unsigned int loadPng(const char *filepath);
}
