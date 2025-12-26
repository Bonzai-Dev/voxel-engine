#pragma once

namespace Core {
  class Application;

  class RenderLayer {
    public:
      explicit RenderLayer(const Application &application) : m_application(application) {}

      virtual ~RenderLayer() = default;

      virtual void Render() {}

    protected:
      const Application &m_application;
   };
}
