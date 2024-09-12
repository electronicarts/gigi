// https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
float3 HSVToRGB(float h, float s, float v)
{
	float h_i = floor(h * 6.0f);
	float f = h * 6.0f - h_i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - f * s);
	float t = v * (1.0f - (1.0f - f) * s);
	switch (int(h_i))
	{
		case 0: return float3( v, t, p );
		case 1: return float3( q, v, p );
		case 2: return float3( p, v, t );
		case 3: return float3( p, q, v );
		case 4: return float3( t, p, v );
		case 5: return float3( v, p, q );
		default: return float3( 1.0f, 0.0f, 1.0f ); // doesn't happen
	}
}

// This uses the golden ratio to make N hues that are maximally distant
// from each other for any N colors desired. Need to use indices [0,N)
// for this to work though. Just does a 1D low discrepancy sequence
// for hue, and has constant s and v values.
float3 IndexToColor(int index, float s = 0.5f, float v = 0.95f)
{
	static const float c_goldenRatioConjugate = 0.61803398875f;
	float h = frac(float(index) * c_goldenRatioConjugate);
	return HSVToRGB(h, s, v);
}