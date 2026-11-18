#include <engine/memory/intrusive_ptr.hpp>
#include <engine/renderer/renderer_context.hpp>
#include <engine/scene/scene_renderer.hpp>

namespace Engine::Renderer {
  SceneRenderer::SceneRenderer(const SharedPtr<RendererContext> &rendererContext): rendererContext(rendererContext) {

  }

  SceneRenderer::~SceneRenderer() {
    
  }
}
