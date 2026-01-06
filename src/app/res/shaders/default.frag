#version 460 core

out vec4 VertexColor;
in vec2 TextureCoordinates;

uniform sampler2D imageTexture;

void main() {
  VertexColor = texture(imageTexture, TextureCoordinates);
}