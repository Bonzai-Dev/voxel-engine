#version 460 core

out vec4 VertexColor;
in vec3 TextureCoordinates;

uniform samplerCube cubeMap;

void main() {
  vec4 textureColor = texture(cubeMap, TextureCoordinates);
  VertexColor = textureColor;
}