#include <core/shared_pointer.hpp>
#include <renderer/renderer_context.hpp>
#include <scene/scene_renderer.hpp>


namespace Engine::Scene {
  SceneRenderer::SceneRenderer(const SharedPtr<Renderer::RendererContext> &rendererContext): context(rendererContext) {
  }

  SceneRenderer::~SceneRenderer() {

  }
}
