cbuffer MeshState : register(b0)
{ 
    float4x4 WVP;
    float4x4 World;
    float3 Color;
};

cbuffer GlobalState : register(b1)
{
    float4x4 View;
    float4x4 Proj;

    float3 CameraPos;
};

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

    output.ScreenPos = mul(float4(input.Pos, 1.0), WVP);
    output.Normal = mul(input.Norm, (float3x3) World);

    return output;
}