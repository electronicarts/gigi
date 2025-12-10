/*$(ShaderResources)*/

#include "HLSLTime.hlsl"		// Gigi need this registered as FileCopy with "Shader" type

struct VSOutput
{
	float4 csPos : SV_POSITION;
};

// pretty gradient from black to red, yellow, white
// @param x 0..1, outside is clamped
// @return linear RGB
float3 gradient(float x)
{
    x = pow(saturate(x),4);
    return float3(sqrt(x), x , x * x);
}

float2 mandelPoint(float2 offset, float2 center, float zoom) {
    float aspect = 1.0;
    offset.x *= aspect;
    return center + offset * zoom;
}

float4 mainPS(VSOutput input) : SV_Target0
{
    // 0..1
    float2 uv = input.csPos.xy / 512.0f;

    float startTime = clock32();

//   int maxIter = /*$(Variable:Iterations)*/; // does not work ?
    const int maxIter = 1000;

    int i=0;
    float2 z = 0;
    float2 c = mandelPoint(uv * 2 - 1, float2(-0.5f,0), 2.0f);
    [loop] for(i=0; i<maxIter; i++)
    {
        float x = (z.x * z.x - z.y * z.y) + c.x;
        float y = (2.0 * z.x * z.y) + c.y;
        z = float2(x, y);
        if(dot(z, z) > 4.0) break;
    }

    float time = clock32() - startTime;
    float t = frac(time * 0.000008f);

    t += frac(i / (float)20) * 0.000001f;    // too faint to see but enough the compiler cannot compile it out

    return float4(gradient(t),1);
}
