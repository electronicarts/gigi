
#define PI 3.14159265359f

struct ViewStruct
{
	float DepthOfFieldFocalLength;
	float DepthOfFieldFocalDistance;
	float DepthOfFieldFocalRegion;
	float DepthOfFieldNearTransitionRegion;	
	float DepthOfFieldFarTransitionRegion;
	float DepthOfFieldScale;
};

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

// Maps a unit square in [0, 1] to a unit disk in [-1, 1]
// Returns polar coordinates (radius, angle)
// Shirley 97 "A Low Distortion Map Between Disk and Square"
float2 UnitSquareToUnitDiskPolar(float2 uv)
{
	float radius;
	float angle;
	const float PI_BY_2 = 1.5707963f; // PI / 2
	const float PI_BY_4 = 0.785398f;  // PI / 4
	const float EPSILON = 0.000001f;
	
	// Remap [0, 1] to [-1, 1] centered
	float a = (2.0f * uv.x) - 1.0f;
	float b = (2.0f * uv.y) - 1.0f;
	
	// Morph to unit disk
	if (abs(a) > abs(b)) 
	{
		// First region (left and right quadrants of the disk)
		radius = a;
		angle = b / (a + EPSILON) * PI_BY_4;
	} 
	else 
	{
		// Second region (top and bottom quadrants of the disk)
		radius = b;
		angle = PI_BY_2 - (a / (b + EPSILON) * PI_BY_4);
	}
	if (radius < 0)
	{
		radius *= -1.0f;
		angle += PI;
	}
	return float2(radius, angle);
}
// Maps a unit square in [0, 1] to a unit disk in [-1, 1]
// Returns new cartesian coordinates (u,v) 
float2 SquareToDiskMapping(float2 uv) {
	float2 PolarCoord = UnitSquareToUnitDiskPolar(uv);
	return float2(PolarCoord.x * cos(PolarCoord.y), PolarCoord.x * sin(PolarCoord.y));
}

// Remap a unit square in [0, 1] to a unit polygon in [-1, 1]
// Returns new cartesian coordinates (u,v) 
float2 SquareToPolygonMapping(float2 uv, in float4 KernelSize) {
	const float N = KernelSize.z; // Edge count of the polygon. 
	float2 PolarCoord = UnitSquareToUnitDiskPolar(uv); // (radius, angle)
	
	if (N >= 3.0f) 
	{
		// Re-scale radius to match a polygon shape
		// http://www.crytek.com/download/Sousa_Graphics_Gems_CryENGINE3.pdf
		PolarCoord.x *= ( cos(PI / N) / ( cos(PolarCoord.y - (2.0f * PI / N) * floor((N*PolarCoord.y + PI) / 2.0f / PI ) )));
	
		// Apply a rotation to the polygon shape. 
		PolarCoord.y += KernelSize.y; 
	}
	
	return float2(PolarCoord.x * cos(PolarCoord.y), PolarCoord.x * sin(PolarCoord.y));
}