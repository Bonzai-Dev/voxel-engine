#include <glad/gl.h>
#include "renderer.hpp"
#include "shader.hpp"

namespace Renderer {
  Shader::Shader(const char *vertexShaderPath, const char *fragmentShaderPath) :
  m_program(createShaderProgram(vertexShaderPath, fragmentShaderPath)) {
  }

  void Shader::Use() const {
    glUseProgram(m_program);
  }
}
