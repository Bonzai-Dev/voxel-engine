#include <glad/gl.h>
#include <stb_image.h>
#include "../logger.hpp"
#include "renderer.hpp"
#include "util/io.hpp"

using namespace Logger;

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
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
    if (buffer == 0) {
      logWarning(Context::Renderer, "Unable to bind element buffer object of id %d.", buffer);
      return;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
  }

  void useVertexArrayObject(unsigned int object) {
    if (object == 0) {
      logWarning(Context::Renderer, "Unable to bind vertex array object of id %d.", object);
      return;
    }

    glBindVertexArray(object);
  }

  void useVertexBufferObject(unsigned int buffer) {
    if (buffer == 0) {
      logWarning(Context::Renderer, "Unable to bind vertex buffer object of id %d.", buffer);
      return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
  }

  void drawTriangles(unsigned int vertexArrayObject, unsigned int vertexBuffer, unsigned int indexBuffer, size_t indicesSize) {
    if (indicesSize == 0) {
      logWarning(Context::Renderer, "Unable to draw triangles, size of index buffer %d is 0.", indexBuffer);
      return;
    }

    useVertexArrayObject(vertexArrayObject);
    useVertexBufferObject(vertexBuffer);
    useElementBufferObject(indexBuffer);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indicesSize), GL_UNSIGNED_INT, nullptr);
  }

  void unbindVertexArrayObject() {
    glBindVertexArray(0);
  }

  void unbindVertexBufferObject() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void unbindElementBufferObject() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void deleteVertexArrayObject(unsigned int object) {
    if (object == 0) {
      logWarning(Context::Renderer, "Attempted to delete an invalid vertex array object of id %d", object);
      return;
    }

    glDeleteVertexArrays(1, &object);
    object = 0;
  }

  void deleteVertexBufferObject(unsigned int buffer) {
    if (buffer == 0) {
      logWarning(Context::Renderer, "Attempted to delete an invalid vertex buffer object of id %d", buffer);
      return;
    }

    glDeleteBuffers(1, &buffer);
    buffer = 0;
  }

  void deleteElementBufferObject(unsigned int buffer) {
    if (buffer == 0) {
      logWarning(Context::Renderer, "Attempted to delete an invalid element buffer object of id %d", buffer);
      return;
    }

    glDeleteBuffers(1, &buffer);
    buffer = 0;
  }

  int getUniform(const char *name, unsigned int shaderProgram) {
    const std::string uniformKey = name + std::to_string(shaderProgram);
    const GLint uniformLocation = glGetUniformLocation(shaderProgram, name);
    if (uniformLocation == -1) {
      logWarning(Context::Renderer, "Unable to get the uniform location of %s", name);
      return -1;
    }

    if (uniforms.contains(uniformKey))
      return uniforms[uniformKey.data()];

    uniforms[uniformKey.c_str()] = uniformLocation;
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

      logError(
        Context::Renderer, "Failed to compile %s shader (%s): %s",
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

    Level logLevel = Level::Warning;

    switch (type) {
      case GL_DEBUG_TYPE_ERROR: logLevel = Level::Error;
        break;
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: logLevel = Level::Warning;
        break;
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: logLevel = Level::Warning;
        break;
      case GL_DEBUG_TYPE_PORTABILITY: logLevel = Level::Warning;
        break;
      case GL_DEBUG_TYPE_PERFORMANCE: logLevel = Level::Warning;
        break;
      case GL_DEBUG_TYPE_MARKER: logLevel = Level::Info;
        break;
      case GL_DEBUG_TYPE_OTHER: logLevel = Level::Info;
        break;
    }

    Logger::log(logLevel, Context::Renderer, "OpenGL ID %u: %s", id, message);
  }

  void useTexture(unsigned int texture) {
    glBindTexture(GL_TEXTURE_2D, texture);
  }

  unsigned int loadCubemap(std::vector<std::string_view> faces) {
    // int width, height, channelsCount;
    // for (size_t index = 0; index < faces.size(); index++) {
    //       unsigned char *data = stbi_load(filepath.data(), &width, &height, &channelsCount, 0);
    //   if (data)
    //   {
    //     glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
    //                  0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
    //     );
    //     stbi_image_free(data);
    //   }
    //   else
    //   {
    //     std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
    //     stbi_image_free(data);
    //   }
    // }
  }

  unsigned int loadPng(const char *filepath) {
    int width, height, channelsCount;
    unsigned char *data = stbi_load(filepath, &width, &height, &channelsCount, 0);
    if (!data) {
      logError(Context::Core, "Failed to load image at \"%s\".", filepath);
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
