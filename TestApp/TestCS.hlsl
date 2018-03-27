Texture2D<unorm float4> InTex : register(t0);
RWTexture2D<unorm float4> OutTex : register(u0);

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float2 c = DTid.xy / float2(1024, 1024);
    c = 2.0f*(c * 2 - 1);

    //c *= 0.005;
   // c += float2(-0.6, -0.44 ); // panning    

    float iters = -1;
    static const float N = 10000;
    float2 z = 0;
    for (float i = 0; i < N; i++)
    {
        // iterate
        z = float2(z.x * z.x - z.y * z.y, 2 * z.x * z.y) + c;

        if (dot(z, z) > 4)
        {
            iters = i;
            break;
        }
           
    }

    if(iters == -1)
        OutTex[DTid.xy] = float4(0, 0, 0, 1);
    else
    {
        static float NC = 32.0f;

        float ia = fmod(iters, NC) / NC;

        OutTex[DTid.xy] = float4(saturate(abs(ia * 6 - 3) - 1),saturate(2 - abs(ia * 6 - 2)), saturate(2 - abs(ia * 6 - 4)), 1);

        /*
        // Smooth coloring

        float ia = fmod(floor(iters / NC), NC) / NC;
        float ib = fmod(floor(iters / NC) + 1, NC) / NC;

        float3 a = { saturate(abs(ia * 6 - 3) - 1),
                     saturate(2 - abs(ia * 6 - 2)),
                     saturate(2 - abs(ia * 6 - 4))
                   };
        float3 b = { saturate(abs(ib * 6 - 3) - 1),
                     saturate(2 - abs(ib * 6 - 2)),
                     saturate(2 - abs(ib * 6 - 4))
                   };

        OutTex[DTid.xy] = float4(lerp(fmod(iters, NC)/NC, a, b), 1);*/
    }
}