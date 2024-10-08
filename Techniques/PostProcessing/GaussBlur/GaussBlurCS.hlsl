// Unnamed technique, shader GaussBlurCS
/*$(ShaderResources)*/

// Calculations adapted from http://demofox.org/gauss.html

#define c_sigma /*$(Variable:Sigma)*/
#define c_support /*$(Variable:Support)*/

float erf(float x)
{
  // save the sign of x
  float sign = (x >= 0) ? 1 : -1;
  x = abs(x);

  // constants
  static const float a1 =  0.254829592;
  static const float a2 = -0.284496736;
  static const float a3 =  1.421413741;
  static const float a4 = -1.453152027;
  static const float a5 =  1.061405429;
  static const float p  =  0.3275911;

  // A&S formula 7.1.26
  float t = 1.0/(1.0 + p*x);
  float y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-x * x);
  return sign * y; // erf(-x) = -erf(x);
}

float IntegrateGaussian(float x, float sigma)
{
  float p1 = erf((x-0.5)/sigma*sqrt(0.5));
  float p2 = erf((x+0.5)/sigma*sqrt(0.5));
  return (p2-p1)/2.0;
}

float3 LinearToSRGB(float3 linearCol)
{
	float3 sRGBLo = linearCol * 12.92;
	float3 sRGBHi = (pow(abs(linearCol), float3(1.0 / 2.4, 1.0 / 2.4, 1.0 / 2.4)) * 1.055) - 0.055;
	float3 sRGB;
	sRGB.r = linearCol.r <= 0.0031308 ? sRGBLo.r : sRGBHi.r;
	sRGB.g = linearCol.g <= 0.0031308 ? sRGBLo.g : sRGBHi.g;
	sRGB.b = linearCol.b <= 0.0031308 ? sRGBLo.b : sRGBHi.b;
	return sRGB;
}

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	int2 px = DTid.xy;
	int2 maxPx;
	Input.GetDimensions(maxPx.x, maxPx.y);
	maxPx -= int2(1,1);

	// calculate radius of our blur based on sigma and support percentage
	const int radius = int(ceil(sqrt(-2.0 * c_sigma * c_sigma * log(1.0 - c_support))));

	// initialize values
	float weight = 0.0f;
	float3 color = float3(0.0f, 0.0f, 0.0f);

	// loop horizontally or vertically, as appropriate
	for (int index = -radius; index <= radius; ++index)
	{
		float kernel = IntegrateGaussian(index, c_sigma);

		int2 offset = (BLURH) ? int2(index, 0) : int2(0, index);

		int2 readPx = clamp(px + offset, int2(0, 0), maxPx);

		color += Input[readPx].rgb * kernel;
		weight += kernel;
	}

	// normalize blur
	color /= weight;

	if (/*$(Variable:sRGB)*/)
		color = LinearToSRGB(color);

	Output[px] = float4(color, 1.0f);
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture Output (as UAV)
*/
