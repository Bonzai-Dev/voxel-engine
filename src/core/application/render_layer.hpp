#pragma once

namespace Core {
  class Application;

  class RenderLayer {
    public:
      explicit RenderLayer(const Application &application) : application(application) {}

      virtual ~RenderLayer() = default;

      virtual void render() {}

    protected:
      const Application &application;
   };
}
