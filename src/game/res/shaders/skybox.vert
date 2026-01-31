#version 460 core

layout (location = 0) in vec3 vertexPosition;

out vec3 TextureCoordinates;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
  TextureCoordinates = vertexPosition - vec3(0.5f, 0.5f, 0.5f);
  vec4 position = projectionMatrix * viewMatrix * vec4(vertexPosition - vec3(0.5f, 0.5f, 0.5f), 1.0);
  gl_Position = position.xyww;
}