/*$(ShaderResources)*/

//#include "HLSLTime.hlsl"

#if TEST == 0 
	// low precision, fast
	#define TYPE min16float
	#define TYPE2 min16float2
#elif TEST == 1
	#define TYPE float
	#define TYPE2 float2
	// same as float
	//#define TYPE half
	//#define TYPE2 half2
#elif TEST == 2
	// high precision, slow

	#if ENV_DOUBLE_SUPPORT == 1
		// high precision, slow
		#define TYPE double
		#define TYPE2 double2
	#else
		#define TYPE float
		#define TYPE2 float2
	#endif

#endif


TYPE2 mandelPoint(TYPE2 offset, TYPE2 center, TYPE zoom) {
    // Scale and center UV to Mandelbrot view
    float aspect = 1.0; // Set by viewport, commonly (set as needed)
//    float2 offset = uv * 2.0 - 1.0; // [-1,1] range
    offset.x *= aspect;
    return center + offset * zoom;
}

float3 colorRamp(float t)
{
    // t: 0.0 (in set) => 1.0 (escaped quickly)
    float3 c1 = float3(0.0,0.0,0.0); // black
    float3 c2 = float3(0.0,0.5,1.0); // blue/cyan
    float3 c3 = float3(1.0,1.0,1.0); // white

    if(t < 0.5)
        return lerp(c1, c2, t*2.0);
    else
        return lerp(c2, c3, (t-0.5)*2.0);
}


/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	int2 px = DTid.xy;

//	px -= /*$(Variable:MouseState)*/.xy;
//	px.x -= /*$(Variable:MouseState)*/.x;
	px.xy -= 256;

	uint2 dims;
	Output.GetDimensions(dims.x, dims.y);

//	float timeInSeconds = / * $(Variable:iTime) * /;
//	float timeInSeconds = clock32() / 1000.0f;
	float2 uv = float2(px) / float2(dims);

	TYPE2 center = (TYPE2)/*$(Variable:Center)*/ + (TYPE2)/*$(Variable:CenterFine)*/ * 0.00001; //float2(-0.75, 0.01);
    float zoom =  (float)0.000000001 * exp2(32 * /*$(Variable:Zoom)*/); // Smaller = zoom in

    TYPE2 c = mandelPoint(uv, center, zoom);
    TYPE2 z = float2(0.0, 0.0);

    int maxIter = /*$(Variable:Iterations)*/;
    int i=0;
    [loop] for(i=0; i<maxIter; i++)
    {
        TYPE x = (z.x * z.x - z.y * z.y) + c.x;
        TYPE y = (2.0 * z.x * z.y) + c.y;
        z = TYPE2(x, y);
//        if(dot(z, z) > 4.0) break;
        if(z.x * z.x + z.y * z.y > 4.0) break;
    }

//    float t = i / (float)maxIter;
    float t = i / (float)100;

	t = frac(t);

	float3 color = 0;
	
	if(TEST != 0)
		color = Output[DTid.xy];

	color[TEST] = t;

//    float3 color = lerp(float3(0,0,0), float3(1, 1, 1), t);
//	float3 color = colorRamp(t);

	Output[DTid.xy] = float4(color, 1);
}
