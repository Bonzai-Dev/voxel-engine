#version 460 core

layout (location = 0) in vec3 vertexPosition;

out vec3 TextureCoordinates;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
  TextureCoordinates = vertexPosition;
  gl_Position = projectionMatrix * viewMatrix * vec4(vertexPosition, 1.0);
}