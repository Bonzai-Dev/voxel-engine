#include <glad/gl.h>
#include "../logger.hpp"
#include "renderer.hpp"
#include "util/io.hpp"

using namespace Renderer;

namespace Renderer {
  void initialize() {
    // Enables debugging for OpenGL
    glDebugMessageCallback(debugMessageCallback, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  }

  void clearBuffer(glm::vec4 color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  unsigned int getUniform(const char *name) {
    if (m_uniforms.contains(name))
      return m_uniforms[name];

    // const GLint uniformLocation = glGetUniformLocation(m_shaderProgram, name);
    // m_uniforms[name] = uniformLocation;
    // return uniformLocation;
    return 0;
  }

  void setFillMode(MeshFillMode fillMode) {
    switch (fillMode) {
      case MeshFillMode::WIREFRAME:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
      case MeshFillMode::SOLID:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    }
  }

  unsigned int compileShader(const char *filepath, ShaderType type) {
    const unsigned int shader = glCreateShader(type == ShaderType::VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
    const char *shaderContent = Util::getFileContents(filepath);
    glShaderSource(shader, 1, &shaderContent, nullptr);
    glCompileShader(shader);

    int shaderCompiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);
    if (!shaderCompiled) {
      int logLength;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
      auto message = static_cast<char*>(alloca(logLength * sizeof(char)));
      glGetShaderInfoLog(shader, logLength, &logLength, message);

      Logger::logError(
        Logger::Context::RENDERER, "Failed to compile %s shader (%s): %s",
        type == ShaderType::VERTEX ? "vertex" : "fragment",
        filepath,
        message
      );

      glDeleteShader(shader);
      return 0;
    }
    return shader;
  }

  unsigned int createShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath) {
    const unsigned int shaderProgram = glCreateProgram();
    const unsigned int vertexShader = compileShader(vertexShaderPath, ShaderType::VERTEX);
    const unsigned int fragmentShader = compileShader(fragmentShaderPath, ShaderType::FRAGMENT);

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);
    glValidateProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
  }

  void debugMessageCallback(
    unsigned int source,
    unsigned int type,
    unsigned int id,
    unsigned int severity,
    int length,
    const char *message,
    const void *userParam
  ) {
    // Stolen code lmaoo
    // Ignore certain verbose info messages (particularly ones on Nvidia).
    if (id == 131169 ||
        id == 131185 || // NV: Buffer will use video memory
        id == 131218 ||
        id == 131204 || // Texture cannot be used for texture mapping
        id == 131222 ||
        id == 131154 || // NV: pixel transfer is synchronized with 3D rendering
        id == 0 // gl{Push, Pop}DebugGroup
    )
      return;

    Logger::Level logLevel = Logger::Level::WARNING;

    switch (type) {
      case GL_DEBUG_TYPE_ERROR: logLevel = Logger::Level::ERROR;
        break;
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: logLevel = Logger::Level::WARNING;
        break;
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: logLevel = Logger::Level::WARNING;
        break;
      case GL_DEBUG_TYPE_PORTABILITY: logLevel = Logger::Level::WARNING;
        break;
      case GL_DEBUG_TYPE_PERFORMANCE: logLevel = Logger::Level::WARNING;
        break;
      case GL_DEBUG_TYPE_MARKER: logLevel = Logger::Level::INFO;
        break;
      case GL_DEBUG_TYPE_OTHER: logLevel = Logger::Level::INFO;
        break;
    }

    Logger::log(logLevel, Logger::Context::RENDERER, "OpenGL ID %u: %s", id, message);
  }
}
