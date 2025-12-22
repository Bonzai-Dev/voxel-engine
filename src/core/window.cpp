#include <glad/gl.h>
#include <iostream>
#include <string_view>
#include <SDL3/SDL.h>
#include <util/io.hpp>
#include "logger.hpp"
#include "renderer/renderer.hpp"
#include "window.hpp"

using namespace logger;

Window::Window(const std::string_view windowName, bool mouseLocked) {
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  // Get display information
  int displayCount;
  m_displays = SDL_GetDisplays(&displayCount);
  if (displayCount > 0) {
    logInfo(Context::CORE, "Found %d display(s).", displayCount);
  } else {
    logWarning(Context::CORE, "No display found.");
  }
  m_currentDisplay = SDL_GetCurrentDisplayMode(m_displays[0]);

  // Creating window
  m_sdlWindow = SDL_CreateWindow(
    windowName.data(),
    m_currentDisplay->w, m_currentDisplay->h,
    SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN
  );

  if (!m_sdlWindow) {
    logError(Context::CORE, "Failed to create SDL window: %s", SDL_GetError());
    return;
  }

  // Creating OpenGL context
  m_glContext = SDL_GL_CreateContext(m_sdlWindow);
  const int version = gladLoadGL(SDL_GL_GetProcAddress);
  if (!version) {
    logError(Context::RENDERER, "Failed to initialize OpenGL context.");
    return;
  }

  SDL_SetWindowRelativeMouseMode(m_sdlWindow, mouseLocked);
  running = true;
  logInfo(
    Context::RENDERER,
    "Program has successfully initialized OpenGL %d.%d core profile.",
    GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version)
  );

  Render();
}

void Window::Render() {
  float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
  };

  const char *fragmentShaderSource = getFileContents("./res/default.frag");
  const unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
  glCompileShader(fragmentShader);

  const char *vertexShaderSource = getFileContents("./res/default.vert");
  const unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  glCompileShader(vertexShader);

  const unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  unsigned int vertexArrayObject;
  glGenVertexArrays(1, &vertexArrayObject);
  glBindVertexArray(vertexArrayObject);

  unsigned int vertexBufferObject;
  glGenBuffers(1, &vertexBufferObject);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT:
          running = false;
          break;
      }
    }

    renderer::clearBuffer(glm::vec4(76, 199, 255, 1) / 255.0f);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    UpdateBuffer();

    const double currentTime = static_cast<double>(SDL_GetTicks()) / 1000;
    deltaTime = currentTime - m_lastFrameTime;
    m_lastFrameTime = static_cast<double>(currentTime);
  }

  Destroy();
}

void Window::Destroy() const {
  SDL_GL_DestroyContext(m_glContext);
  SDL_DestroyWindow(m_sdlWindow);
  SDL_free(m_displays);
}

void Window::UpdateBuffer() const {
  SDL_GL_SwapWindow(m_sdlWindow);
}

void Window::UnlockMouse() {
  SDL_SetWindowRelativeMouseMode(m_sdlWindow, false);
}

void Window::LockMouse() {
  SDL_SetWindowRelativeMouseMode(m_sdlWindow, true);
}