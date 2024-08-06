// One Pass Blur Shader

RWTexture2D<float4> Input : register(u0);

#line 1

[numthreads(8, 8, 1)]
#line 2
void Main(uint3 DTid : SV_DispatchThreadID)
{
    float4 pixel = Input[DTid.xy];
    float shade = dot(pixel.rbg, float3(0.3f, 0.59f, 0.11f));
    Input[DTid.xy] = float4(shade, shade, shade, pixel.a);
}
