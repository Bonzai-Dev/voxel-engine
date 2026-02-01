#version 460 core

out vec4 VertexColor;
in vec2 TextureCoordinates;
in vec3 VertexNormal;

uniform float sunBrightness;
uniform vec3 sunDirection;
uniform vec3 sunColor;
uniform vec3 ambientLight;
uniform sampler2D imageTexture;

vec3 diffuse(vec3 normal, vec3 lightDirection, vec3 color) {
  return max(0.0, dot(lightDirection, normal)) * color;
}

void main() {
  vec4 textureColor = texture(imageTexture, TextureCoordinates);
  if (textureColor.a < 0.1)
    discard;

  vec4 brightness = vec4(sunBrightness, sunBrightness, sunBrightness, 1.0f);
  VertexColor = textureColor * vec4((ambientLight + diffuse(VertexNormal, -sunDirection, sunColor)), 1.0f) * brightness;
}