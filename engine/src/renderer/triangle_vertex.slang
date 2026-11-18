[shader("vertex")]
float4 main(uint vertexId : SV_VertexID) : SV_Position {
  const float2 positions[3] = {
    float2(0.0, -0.65),
    float2(0.65, 0.65),
    float2(-0.65, 0.65)
  };
  return float4(positions[vertexId], 0.0, 1.0);
}
