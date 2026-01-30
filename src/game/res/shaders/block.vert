#version 460 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 textureCoordinates;

out vec2 TextureCoordinates;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
  gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
  TextureCoordinates = textureCoordinates;
}