#version 460 core

out vec4 VertexColor;
in vec2 TextureCoordinates;

uniform vec3 ambientLight;
uniform sampler2D imageTexture;

void main() {
  vec4 textureColor = texture(imageTexture, TextureCoordinates);

  if (textureColor.a < 0.1)
    discard;

  VertexColor = textureColor * vec4(ambientLight, 1.0f);
}