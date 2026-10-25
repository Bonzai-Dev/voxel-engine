[shader("vertex")]
float4 main(uint vertexId : SV_VertexID) : SV_Position {
  float2 position;
  if (vertexId == 0) {
    position = float2(0.0, -0.65);
  } else if (vertexId == 1) {
    position = float2(0.65, 0.65);
  } else {
    position = float2(-0.65, 0.65);
  }

  return float4(position, 0.0, 1.0);
}
