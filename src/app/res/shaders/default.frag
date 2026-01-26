#version 460 core

out vec4 VertexColor;
in vec2 TextureCoordinates;

uniform sampler2D imageTexture;

void main() {
  vec4 textureColor = texture(imageTexture, TextureCoordinates);

  if (textureColor.a < 0.1)
    discard;

  VertexColor = textureColor;
}