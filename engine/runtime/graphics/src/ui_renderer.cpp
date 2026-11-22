#include <engine/runtime/scene/scene_renderer.hpp>
#include <engine/runtime/graphics/ui/ui_renderer.hpp>

namespace Engine::Application {
  UIRenderer::UIRenderer(const SharedPtr<Renderer::RendererContext> &rendererContext):
  sceneRenderer(std::make_unique<Scene::SceneRenderer>(rendererContext)) {
  }

  UIRenderer::~UIRenderer() = default;

  void UIRenderer::render(
    const SharedPtr<Application::GraphicsWindow> &window,
    std::uint32_t width,
    std::uint32_t height
  ) {
    sceneRenderer->render(window, width, height);
  }

  void UIRenderer::createViewport(const SharedPtr<Application::GraphicsWindow> &window) {
    sceneRenderer->createViewport(window);
  }
}
