#include <glad/gl.h>
#include "renderer.hpp"
#include "shader.hpp"

namespace Renderer {
  Shader::Shader(const char *vertexShaderPath, const char *fragmentShaderPath) :
  program(createShaderProgram(vertexShaderPath, fragmentShaderPath)) {
  }

  void Shader::use() const {
    glUseProgram(program);
  }
}
