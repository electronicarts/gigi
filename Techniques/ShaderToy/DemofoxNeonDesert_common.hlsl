// =======================================================================
// Constants
// =======================================================================

static const float c_pi = 3.14159265359f;
static const float c_twopi = 2.0f * c_pi;
static const float KEY_SPACE = 32.5/256.0;

// =======================================================================
// Parameters
// =======================================================================

// camera parameters
//static const float3 c_cameraAt = float3(0.0f, 0.0f, 0.0f);
//static const float c_cameraDistance = 80.0f;
//static const float c_apertureRadius = 0.4f;  // in world units. 0 for pinhole camera.
//static const float c_focalPlaneDistance = 80.0f; // in world units. How far from the camera things are in focus at.
//static const float c_minCameraAngle = c_pi / 2.0f - 0.125f;
//static const float c_maxCameraAngle = (c_pi - 0.01f);
//static const float c_exposure = 2.0f;  // +1 fstops of exposure
//static const float c_FOV = 90.0f; // in degrees

// the place to put the mouse if it hasn't moved yet (aka the default view)
//static const float2 c_defaultMousePos = float2(140.0f / 800.0f, 40.0f / 450.0f);  // viewing the dark sun
//const float2 c_defaultMousePos = float2(300.0f / 800.0f, 30.0f / 450.0f);  // viewing saturn

// bloom parameters
static const float3 c_bloomMin = float3(1.0f, 1.0f, 1.0f);
// The array below is a 1d gaussian which starts at index 0.
// A nice gaussian calculator is at http://dev.theomader.com/gaussian-kernel-calculator/
// Just calculate a kernel that is (c_radius * 2 + 1) large and use the second half of the result (including the center).
// That should give you an array that is (c_radius+1) items large.
//const int c_bloomRadius = 3;
//const float c_bloomKernel[c_bloomRadius+1] = float[c_bloomRadius+1](0.383103, 0.241843, 0.060626, 0.00598); // sigma 1.0
static const int c_bloomRadius = 5;
//const float c_bloomKernel[c_bloomRadius+1] = float[c_bloomRadius+1](0.382925, 0.24173, 0.060598, 0.005977, 0.000229, 0.000003); // sigma 1.0
static const float c_bloomKernel[c_bloomRadius+1] = {0.163053, 0.150677, 0.118904, 0.080127, 0.046108, 0.022657}; // sigma 2.5
//const float c_bloomKernel[c_bloomRadius+1] = float[c_bloomRadius+1](0.109317, 0.107159, 0.100939, 0.091364, 0.079465, 0.066414); // sigma 5.0    

// scene parameters
//static const float c_skyboxMultiplier = 0.0f; // for the cube map, if visible.

// ray tracing parameters
//static const int c_numBounces = 4;
static const float c_minimumRayHitTime = 0.1f;
static const float c_superFar = 10000.0f;

// ray marching parameters
//static const int c_numSteps = 256;
//static const float c_minStepDistance = 0.1f;

// other parameters
static const float c_rayPosNormalNudge = 0.01f; // after a hit, it moves the ray this far along the normal away from a surface

// =======================================================================
// Display Adaptation Functions
// =======================================================================

float3 LessThan(float3 f, float value)
{
    return float3(
        (f.x < value) ? 1.0f : 0.0f,
        (f.y < value) ? 1.0f : 0.0f,
        (f.z < value) ? 1.0f : 0.0f);
}

float3 LinearToSRGB(float3 rgb)
{
    rgb = clamp(rgb, 0.0f, 1.0f);
    
    return lerp(
        pow(rgb * 1.055f, 1.f / 2.4f) - 0.055f,
        rgb * 12.92f,
        LessThan(rgb, 0.0031308f)
    );
}

// ACES tone mapping curve fit to go from HDR to LDR
//https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x*(a*x + b)) / (x*(c*x + d) + e), 0.0f, 1.0f);
}

// =======================================================================
// Bloom Functions
// =======================================================================

float3 BloomPass1(in float2 pixelPos, in Texture2D<float4> linearImage)
{
    // horizontal & conditional
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    for (int ix = -c_bloomRadius; ix <= c_bloomRadius; ++ix)
    {
        float3 pixelColor = linearImage[pixelPos + float2(ix, 0)].rgb;
        float3 bloomColor = clamp(pixelColor - c_bloomMin, 0.0f, 1.0f);
        
        bloomColor = (dot(bloomColor, c_bloomMin) > 0.0f ? bloomColor : float3(0.0f, 0.0f, 0.0f));

        float weight = c_bloomKernel[abs(ix)];
        sum += bloomColor * weight;
    }
    
    return sum;
}

float3 BloomPass2(in float2 pixelPos, in Texture2D<float4> bloomImage)
{
    // vertical
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    for (int iy = -c_bloomRadius; iy <= c_bloomRadius; ++iy)
    {
        float3 bloomColor = bloomImage[pixelPos + float2(0, iy)].rgb;
        float weight = c_bloomKernel[abs(iy)];
        sum += bloomColor * weight;
    }
    
    return sum;
}

// =======================================================================
// RNG Functions
// =======================================================================

// from "Hash without Sine" https://www.shadertoy.com/view/4djSRW
float hash11(float p)
{
    p = frac(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return frac(p);
}
float hash12(float2 p)
{
	float3 p3  = frac(float3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return frac((p3.x + p3.y) * p3.z);
}
float2 hash22(float2 p)
{
	float3 p3 = frac(float3(p.xyx) * float3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx+33.33);
    return frac((p3.xx+p3.yz)*p3.zy);

}

uint wang_hash(inout uint seed)
{
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}

float RandomFloat01(inout uint state)
{
    return float(wang_hash(state)) / 4294967296.0;
}

float3 RandomUnitVector(inout uint state)
{
    float z = RandomFloat01(state) * 2.0f - 1.0f;
    float a = RandomFloat01(state) * c_twopi;
    float r = sqrt(1.0f - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return float3(x, y, z);
}

// =======================================================================
// Misc Functions
// =======================================================================

float MaxComp(float3 v)
{
    return max(v.x, max(v.y, v.z));
}

int MaxCompIndex(float3 v)
{
    if (v.x > v.y)
    {
        if (v.x > v.z)
            return 0;
        else
            return 2;
    }
    else
    {
        if (v.y > v.z)
            return 1;
        else
            return 2;
    }
}

float Bias(float x, float bias)
{
  return (x / ((((1.0f/bias) - 2.0f)*(1.0f - x))+1.0f));
}

float dot2(in float3 v ) { return dot(v,v); }

// =======================================================================
// Distance Functions 2D
// https://iquilezles.org/articles/distfunctions2d
// =======================================================================

float sdEquilateralTriangle( in float2 p )
{
    const float k = sqrt(3.0);
    p.x = abs(p.x) - 1.0;
    p.y = p.y + 1.0/k;
    if( p.x+k*p.y>0.0 ) p = float2(p.x-k*p.y,-k*p.x-p.y)/2.0;
    p.x -= clamp( p.x, -2.0, 0.0 );
    return -length(p)*sign(p.y);
}

float sdRoundedX( in float2 p, in float w, in float r )
{
    p = abs(p);
    return length(p-min(p.x+p.y,w)*0.5) - r;
}

float mod(float x, float y)
{
	return x - y * floor(x/y);
}

float sdStar(in float2 p, in float r, in int n, in float m)
{
    // next 4 lines can be precomputed for a given shape
    float an = 3.141593/float(n);
    float en = 3.141593/m;  // m is between 2 and n
    float2  acs = float2(cos(an),sin(an));
    float2  ecs = float2(cos(en),sin(en)); // ecs=float2(0,1) for regular polygon,

    float bn = mod(atan2(p.x,p.y),2.0*an) - an;
    p = length(p)*float2(cos(bn),abs(sin(bn)));
    p -= r*acs;
    p += ecs*clamp( -dot(p,ecs), 0.0, r*acs.y/ecs.y);
    return length(p)*sign(p.x);
}

float sdCircle( float2 p, float r )
{
  return length(p) - r;
}

// =======================================================================
// Distance Functions 3D
// https://iquilezles.org/articles/distfunctions
// =======================================================================

float SphereDistance(in float4 sphere, in float3 p)
{
    return length(p - sphere.xyz) - sphere.w;
}

float PlaneDistance(in float4 plane, in float3 p)
{
    return (dot(plane.xyz, p) - plane.w) / dot(plane.xyz, plane.xyz);
}

float BoxDistance(in float3 boxPos, in float3 boxRadius, in float width, in float3 p)
{
    p -= boxPos;
    width = max(width, 0.01f);  // fix for a problem i didn't investigate :P
    
    // from https://www.youtube.com/watch?reload=9&v=62-pRVZuS5c
    // rounding the box from https://www.youtube.com/watch?v=s5NGeUV2EyU
    float3 q = abs(p) - boxRadius;
    return length(max(q,float3(0.0f,0.0f,0.0f))) + min(MaxComp(q), 0.0f) - width;
}

float LineDistance(in float3 A, in float3 B, float width, in float3 p, out float3 normal)
{
	float3 AP = p - A;
    float3 AB = B - A;
    
    // from https://www.youtube.com/watch?v=PMltMdi1Wzg
    // added normal calculation though
    float h = min(1.0f, max(0.0f, dot(AP, AB) / dot(AB, AB)));
    float3 closestPoint = A + h * AB;
    normal = normalize(p - closestPoint);
    return length(p - closestPoint) - width;
}

float BezierDistance( in float3 pos, in float3 A, in float3 B, in float3 C, in float width)
{    
    float3 a = B - A;
    float3 b = A - 2.0*B + C;
    float3 c = a * 2.0;
    float3 d = A - pos;
    float kk = 1.0/dot(b,b);
    float kx = kk * dot(a,b);
    float ky = kk * (2.0*dot(a,a)+dot(d,b)) / 3.0;
    float kz = kk * dot(d,a);      
    float res = 0.0;
    float p = ky - kx*kx;
    float p3 = p*p*p;
    float q = kx*(2.0*kx*kx-3.0*ky) + kz;
    float h = q*q + 4.0*p3;
    if( h >= 0.0) 
    { 
        h = sqrt(h);
        float2 x = (float2(h,-h)-q)/2.0;
        float2 uv = sign(x)*pow(abs(x), 1.0/3.0);
        float t = clamp( uv.x+uv.y-kx, 0.0, 1.0 );
        res = dot2(d + (c + b*t)*t);
    }
    else
    {
        float z = sqrt(-p);
        float v = acos( q/(p*z*2.0) ) / 3.0;
        float m = cos(v);
        float n = sin(v)*1.732050808;
        float3  t = clamp(float3(m+m,-n-m,n-m)*z-kx,0.0,1.0);
        res = min( dot2(d+(c+b*t.x)*t.x),
                   dot2(d+(c+b*t.y)*t.y) );
        // the third root cannot be the closest
        // res = min(res,dot2(d+(c+b*t.z)*t.z));
    }
    return sqrt( res ) - width;
}

// =======================================================================
// Object Intersection Helpers for Ray Marching
// =======================================================================

struct SMaterial
{
    float3 diffuse;
    float3 specular;
	float roughness;
    float3 emissive;    
};

struct SRayHitInfo
{
    bool hitAnObject;
    bool rayMarchedObject;
    float dist;
    float3 normal;
    SMaterial material;
};
    
void TestSphereMarch(in float3 rayPos, inout SRayHitInfo info, in float4 sphere, in SMaterial material)
{
    float dist = SphereDistance(sphere, rayPos);
    if (dist < info.dist)
    {
        info.rayMarchedObject = true;
        info.dist = dist;        
        info.normal = normalize(rayPos - sphere.xyz);
        info.material = material;
    }    
}

void TestPlaneMarch(in float3 rayPos, inout SRayHitInfo info, in float4 plane, in SMaterial material)
{
    float dist = PlaneDistance(plane, rayPos);
    if (dist < info.dist)
    {
        info.rayMarchedObject = true;
        info.dist = dist;        
        info.normal = plane.xyz;
        info.material = material;
    }    
}

void TestBoxMarch(in float3 rayPos, inout SRayHitInfo info, in float3 boxPos, in float3 boxRadius, in float width, in SMaterial material)
{
    float dist = BoxDistance(boxPos, boxRadius, width, rayPos);
    if (dist < info.dist)
    {
        info.rayMarchedObject = true;
        info.dist = dist;
        
        float3 relPos = max(abs(rayPos - boxPos) - boxRadius, 0.0f);
        int maxIndex = MaxCompIndex(relPos);
        if (maxIndex == 0)
        {
            info.normal = (rayPos.x < boxPos.x) ? float3(-1.0f, 0.0f, 0.0f) : float3(1.0f, 0.0f, 0.0f);
        }
        else if(maxIndex == 1)
        {
            info.normal = (rayPos.y < boxPos.y) ? float3(0.0f, -1.0f, 0.0f) : float3(0.0f, 1.0f, 0.0f);
        }
        else
        {
            info.normal = (rayPos.z < boxPos.z) ? float3(0.0f, 0.0f, -1.0f) : float3(0.0f, 0.0f, 1.0f);
        }
        
        info.material = material;
    }    
}

void TestLineMarch(in float3 rayPos, inout SRayHitInfo info, in float3 A, in float3 B, in float width, in SMaterial material)
{   
    float3 normal;
    float dist = LineDistance(A, B, width, rayPos, normal);
    if (dist < info.dist)
    {
        info.rayMarchedObject = true;
        info.dist = dist;        
        info.normal = normal;
        info.material = material;
    }    
}

void TestBezierMarch(in float3 rayPos, inout SRayHitInfo info, in float3 A, in float3 B, in float3 C, in float width, in SMaterial material)
{
    float dist = BezierDistance(rayPos, A, B, C, width);
    if (dist < info.dist)
    {
        info.rayMarchedObject = true;
        info.dist = dist;    
        info.normal = float3(1.0f, 0.0f, 0.0f); // TODO: normal, if you ever need it!
        info.material = material;
    }    
}

// =======================================================================
// Object Intersection Helpers for Ray Tracing
// =======================================================================

bool TestPlaneTrace(in float3 rayPos, in float3 rayDir, inout SRayHitInfo info, in float4 plane, in SMaterial material)
{
    float dist = -1.0f;
    float denom = dot(plane.xyz, rayDir);
    if (abs(denom) > 0.001f)
    {
        dist = (plane.w - dot(plane.xyz, rayPos)) / denom;
    
        if (dist > c_minimumRayHitTime && dist < info.dist)
        {
            info.hitAnObject = true;
            info.rayMarchedObject = false;
            info.dist = dist;        
            info.normal = plane.xyz;
            info.material = material;
            return true;
        }
    }
    return false;
}

bool TestSphereTrace(in float3 rayPos, in float3 rayDir, inout SRayHitInfo info, in float4 sphere, in SMaterial material)
{
	//get the vector from the center of this sphere to where the ray begins.
	float3 m = rayPos - sphere.xyz;

    //get the dot product of the above vector and the ray's vector
	float b = dot(m, rayDir);

	float c = dot(m, m) - sphere.w * sphere.w;

	//exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
	if(c > 0.0 && b > 0.0)
		return false;

	//calculate discriminant
	float discr = b * b - c;

	//a negative discriminant corresponds to ray missing sphere
	if(discr < 0.0)
		return false;
    
	//ray now found to intersect sphere, compute smallest t value of intersection
    bool fromInside = false;
	float dist = -b - sqrt(discr);
    if (dist < 0.0f)
    {
        fromInside = true;
        dist = -b + sqrt(discr);
    }
    
	if (dist > c_minimumRayHitTime && dist < info.dist)
    {
        info.hitAnObject = true;
        info.rayMarchedObject = false;
        info.dist = dist;        
        info.normal = normalize((rayPos+rayDir*dist) - sphere.xyz) * (fromInside ? -1.0f : 1.0f);
		info.material = material;        
        return true;
    }
    
    return false;
}

// Tests against a vertical cylinder without caps
// Derived from the sphere function and not super optimal but whatever.
bool TestCylinderTrace(in float3 rayPos_, in float3 rayDir_, inout SRayHitInfo info, in float3 pos, in float radius, in float height, in SMaterial material)
{
    float3 relativeRayPos = rayPos_ - pos;
    float3 projectedRayPos = float3(relativeRayPos.x, 0.0f, relativeRayPos.z);
    float3 projectedRayDir = float3(rayDir_.x, 0.0f, rayDir_.z);
    
    float lengthCorrection = length(projectedRayDir);
    projectedRayDir /= lengthCorrection;
    
	//get the vector from the center of this circle to where the ray begins.
	float3 m = projectedRayPos;

    //get the dot product of the above vector and the ray's vector
	float b = dot(m, projectedRayDir);

	float c = dot(m, m) - radius * radius;

	//exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
	if(c > 0.0 && b > 0.0)
		return false;

	//calculate discriminant
	float discr = b * b - c;

	//a negative discriminant corresponds to ray missing sphere
	if(discr < 0.0)
		return false;
    
	//ray now found to intersect sphere, compute smallest t value of intersection
    bool fromInside = false;
	float dist = -b - sqrt(discr);
    if (dist < 0.0f)
    {
        fromInside = true;
        dist = -b + sqrt(discr);
    }

    // adjust distance
    dist /= lengthCorrection;

    // enforce height - note we may be invalid height for the first distance but not the second.
    float3 relativeHitPos = relativeRayPos + rayDir_ * dist;
    if (relativeHitPos.y < 0.0f || relativeHitPos.y > height)
    {
        dist = -b + sqrt(discr);
        dist /= lengthCorrection;
        relativeHitPos = relativeRayPos + rayDir_ * dist;
        if (relativeHitPos.y < 0.0f || relativeHitPos.y > height)
      		return false;
    }
        
	if (dist > c_minimumRayHitTime && dist < info.dist)
    {
        info.hitAnObject = true;
        info.rayMarchedObject = false;
        info.dist = dist;        
        info.normal = normalize(float3(relativeHitPos.x, 0.0f, relativeHitPos.z)) * (fromInside ? -1.0f : 1.0f);
		info.material = material;        
        return true;
    }
    
    return false;
}
