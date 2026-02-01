#version 460 core

out vec4 VertexColor;
in vec2 TextureCoordinates;
in vec3 VertexNormal;
in vec4 ViewTransform;

struct Fog {
  vec3 color;
  float start; // Linear fog only
  float end; // Linear fog only
  float density; // exp and exp2 fog only
  int equation; // 0 = linear, 1 = exp, 2 = exp2
};

struct Sun {
  float brightness;
  vec3 direction;
  vec3 color;
};

uniform Fog fog;
uniform Sun sun;
uniform vec3 ambientLight;
uniform sampler2D imageTexture;

vec3 diffuse(vec3 normal, vec3 lightDirection, vec3 color) {
  return max(0.0, dot(lightDirection, normal)) * color;
}

float fogFactor(Fog parameters, float fogCoordinate) {
  float result = 0.0f;
  if(parameters.equation == 0)
    result = (parameters.end - fogCoordinate) / (parameters.end - parameters.start);
  else if(parameters.equation == 1)
    result = exp(-parameters.density * fogCoordinate);
  else if(parameters.equation == 2)
    result = exp(-pow(parameters.density * fogCoordinate, 2.0f));

  result = 1.0f - clamp(result, 0.0f, 1.0f);
  return result;
}

void main() {
  vec4 textureColor = texture(imageTexture, TextureCoordinates);
  if (textureColor.a < 0.1)
    discard;

  vec4 brightness = vec4(sun.brightness, sun.brightness, sun.brightness, 1.0f);
  vec4 lighting = textureColor * vec4((ambientLight + diffuse(VertexNormal, -sun.direction, sun.color)), 1.0f) * brightness;
  VertexColor = lighting;

  VertexColor = mix(lighting, vec4(fog.color, 1.0f), fogFactor(fog, abs(ViewTransform.z / ViewTransform.w)));
}