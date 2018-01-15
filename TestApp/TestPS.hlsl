struct PsIn
{
    float4 ScreenPos : Sv_Position;
    float3 Normal : TEXCOORD0;
};

float4 main(PsIn input) : SV_TARGET
{
    return float4(input.Normal*0.5+0.5, 1.0f);
}