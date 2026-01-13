#include <glad/gl.h>
#include <stb_image.h>
#include "../logger.hpp"
#include "renderer.hpp"
#include "util/io.hpp"

using namespace Renderer;

namespace Renderer {
  void initialize() {
    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Enables debugging for OpenGL
    glDebugMessageCallback(debugMessageCallback, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  }

  void clearBuffer(glm::vec4 color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  unsigned int createVertexBufferObject(const void *data, size_t size) {
    unsigned int object;
    glGenBuffers(1, &object);
    glBindBuffer(GL_ARRAY_BUFFER, object);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    return object;
  }

  unsigned int createVertexArrayObject() {
    unsigned int object;
    glGenVertexArrays(1, &object);
    glBindVertexArray(object);
    return object;
  }

  unsigned int createElementBufferObject(const void *data, size_t size) {
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    return buffer;
  }

  void useElementBufferObject(unsigned int buffer) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
  }

  void useVertexArrayObject(unsigned int object) {
    glBindVertexArray(object);
  }

  void useVertexBufferObject(unsigned int buffer) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
  }

  int getUniform(const char *name, unsigned int shaderProgram) {
    const std::string uniformKey = name + std::to_string(shaderProgram);
    const GLint uniformLocation = glGetUniformLocation(shaderProgram, name); // INSTEAD OF GETTING THE NAME, I GOT THE KEY RAHHHHH
    if (uniformLocation == -1) {
      Logger::logWarning(Logger::Context::Renderer, "Unable to get the uniform location of %s", name);
      return -1;
    }

    if (m_uniforms.contains(uniformKey))
      return m_uniforms[uniformKey.data()];

    m_uniforms[uniformKey.c_str()] = uniformLocation;
    return uniformLocation;
  }

  void setFillMode(MeshFillMode fillMode) {
    switch (fillMode) {
      case MeshFillMode::Wireframe:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
      case MeshFillMode::Solid:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    }
  }

  unsigned int compileShader(const char *filepath, ShaderType type) {
    const unsigned int shader = glCreateShader(type == ShaderType::Vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
    const auto shaderSource = Util::File::readTextFile(filepath);
    const char *shaderContent = shaderSource.c_str();
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
        Logger::Context::Renderer, "Failed to compile %s shader (%s): %s",
        type == ShaderType::Vertex ? "vertex" : "fragment",
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
    const unsigned int vertexShader = compileShader(vertexShaderPath, ShaderType::Vertex);
    const unsigned int fragmentShader = compileShader(fragmentShaderPath, ShaderType::Fragment);

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

    Logger::Level logLevel = Logger::Level::Warning;

    switch (type) {
      case GL_DEBUG_TYPE_ERROR: logLevel = Logger::Level::Error;
        break;
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: logLevel = Logger::Level::Warning;
        break;
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: logLevel = Logger::Level::Warning;
        break;
      case GL_DEBUG_TYPE_PORTABILITY: logLevel = Logger::Level::Warning;
        break;
      case GL_DEBUG_TYPE_PERFORMANCE: logLevel = Logger::Level::Warning;
        break;
      case GL_DEBUG_TYPE_MARKER: logLevel = Logger::Level::Info;
        break;
      case GL_DEBUG_TYPE_OTHER: logLevel = Logger::Level::Info;
        break;
    }

    Logger::log(logLevel, Logger::Context::Renderer, "OpenGL ID %u: %s", id, message);
  }

  void useTexture(unsigned int texture) {
    glBindTexture(GL_TEXTURE_2D, texture);
  }

  unsigned int loadPng(const char *filepath) {
    int width, height, channelsCount;
    unsigned char *data = stbi_load(filepath, &width, &height, &channelsCount, 0);
    if (!data) {
      Logger::logError(Logger::Context::Core, "Failed to load image at \"%s\".", filepath);
      return 0;
    }

    unsigned int texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    return texture;
  }
}
