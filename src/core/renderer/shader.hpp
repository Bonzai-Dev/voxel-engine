#pragma once

namespace Renderer {
  class Shader {
    public:
      Shader(const char *vertexShaderPath, const char *fragmentShaderPath);

      ~Shader() = default;

      void use() const;

    protected:
      unsigned int program;
  };
}