
// Hardcoding triangle coordinates
/*static const float4 vpos[3] =
{
    { -0.5f, -0.5f, 0.5f, 1.0f },
    {  0.5f, -0.5f, 0.5f, 1.0f },
    {  0.0f,  0.5f, 0.5f, 1.0f }
};*/

struct Vertex
{
    float3 Pos : Position;
    float3 Norm : Normal;
    float3 Tangent : Tangent;
    float2 UV : UV;
};

struct PsIn
{
    float4 ScreenPos : Sv_Position;
    float3 Normal : TEXCOORD0;
};

PsIn main(Vertex input)
{
    PsIn output = (PsIn) 0;

    output.ScreenPos = float4(input.Pos, 1.0);
    output.Normal = input.Norm;

    return output;
}