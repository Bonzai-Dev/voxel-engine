#pragma once

#include <string>
#include <glm/vec4.hpp>

namespace Renderer {
  enum class ShaderType {
    Vertex,
    Fragment,
  };

  enum class MeshFillMode {
    Wireframe,
    Solid
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

  void debugMessageCallback(
    unsigned int source,
    unsigned int type,
    unsigned int id,
    unsigned int severity,
    int length,
    const char *message,
    const void *userParam
  );
}
