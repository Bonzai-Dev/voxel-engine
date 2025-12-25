#pragma once

#include <string>
#include <glm/vec4.hpp>

namespace Renderer {
  enum class ShaderType {
    VERTEX,
    FRAGMENT,
  };

  enum class MeshFillMode {
    WIREFRAME,
    SOLID
  };

  void initialize();

  void clearBuffer(glm::vec4 color);

  unsigned int getUniform(const char *name);

  void setFillMode(MeshFillMode fillMode);

  inline std::unordered_map<std::string, unsigned int> m_uniforms;

  unsigned int compileShader(const char *filepath, ShaderType type);

  unsigned int createShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath);

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
