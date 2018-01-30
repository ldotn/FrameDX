Texture2D<unorm float4> InTex : register(t0);
RWTexture2D<unorm float4> OutTex : register(u0);

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float2 v0 = InTex[DTid.xy].xy;
   
    v0 *= cos(0.25f * DTid.x) * sin(0.25f * DTid.y);

    OutTex[DTid.xy] = float4(v0, 0, 1);
}