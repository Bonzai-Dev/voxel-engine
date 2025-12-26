#pragma once

namespace Renderer {
  class Shader {
    public:
      Shader(const char *vertexShaderPath, const char *fragmentShaderPath);

      ~Shader() = default;

      void Use() const;

    protected:
      unsigned int m_program;
  };
}