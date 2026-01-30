#version 460 core

out vec4 VertexColor;

uniform samplerCube cubeMap;

void main() {
  VertexColor = vec4(1, 1, 1, 1);
}