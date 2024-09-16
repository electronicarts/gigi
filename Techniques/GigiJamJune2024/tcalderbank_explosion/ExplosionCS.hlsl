// Explosion technique, shader DoBlur
/*$(ShaderResources)*/

// Credit
// Based on these 2 tutorials
// https://github.com/ssloy/tinykaboom/wiki/KABOOM!-in-180-lines-of-code
// https://www.youtube.com/watch?v=welK2U7UkzE

// constants
static const float c_time = /*$(Variable:iTime)*/;
static const float3 c_resolution = /*$(Variable:iResolution)*/;
static const float c_pi = 3.14159;
static const float c_fov = c_pi / 3.0f; // 60 degrees
static const float3 c_cameraOrigin = float3(0.0f, 0.0f, 3.0f);

static const int c_rayMarchIterations = 128;
static const float c_rayMarchStepPercent = 0.1f;
static const float c_rayMarchMinStepSize = 0.01f;
static const float4 c_backgroundColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
static const float4 c_hitColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
static const float3 c_lightPosition = float3(10.0f, 10.0f, 10.0f);

static const float c_sphereRadius = /*$(Variable:SphereRadius)*/;

static const float c_freq1 = /*$(Variable:NoiseFreq1)*/;
static const float c_freq2 = /*$(Variable:NoiseFreq2)*/;
static const float c_freq3 = /*$(Variable:NoiseFreq3)*/;
static const float c_freq4 = /*$(Variable:NoiseFreq4)*/;

static const float4 c_noiseWeights = /*$(Variable:NoiseWeights)*/;
static const float c_noiseAmp = /*$(Variable:NoiseAmplitude)*/;
static const float c_noiseBias = /*$(Variable:NoiseBias)*/;

static const float3 c_color1 = /*$(Variable:Color1)*/;
static const float3 c_color2 = /*$(Variable:Color2)*/;
static const float3 c_color3 = /*$(Variable:Color3)*/;
static const float3 c_color4 = /*$(Variable:Color4)*/;
static const float3 c_color5 = /*$(Variable:Color5)*/;

static const float3 c_colorThreshholds = /*$(Variable:ColorThresholds)*/;

static const bool c_animateRadius = /*$(Variable:AnimateRadius)*/;
static const bool c_rotate = /*$(Variable:Rotate)*/;
static const float c_rotationSpeed = /*$(Variable:RotationSpeed)*/;

// Perlin Noise
// Source: https://github.com/gegamongy/3DPerlinNoiseGodot/blob/main/3DPerlinNoise.gdshader
float3 random3D(float3 uvw){
	
    uvw = float3( dot(uvw, float3(127.1,311.7, 513.7) ),
               dot(uvw, float3(269.5,183.3, 396.5) ),
			   dot(uvw, float3(421.3,314.1, 119.7) ) );
			
    return -1.0 + 2.0 * frac(sin(uvw) * 43758.5453123);
}

float noise3D(float3 uvw){
	//uvw *= noise_scale;
	//uvw += noise_transform;
	
	float3 gridIndex = floor(uvw); 
	float3 gridFract = frac(uvw);
	
	float3 blur = smoothstep(0.0, 1.0, gridFract);
	//blur = gridFract;
	
	float3 blb = gridIndex + float3(0.0, 0.0, 0.0);
	float3 brb = gridIndex + float3(1.0, 0.0, 0.0);
	float3 tlb = gridIndex + float3(0.0, 1.0, 0.0);
	float3 trb = gridIndex + float3(1.0, 1.0, 0.0);
	float3 blf = gridIndex + float3(0.0, 0.0, 1.0);
	float3 brf = gridIndex + float3(1.0, 0.0, 1.0);
	float3 tlf = gridIndex + float3(0.0, 1.0, 1.0);
	float3 trf = gridIndex + float3(1.0, 1.0, 1.0);
	
	float3 gradBLB = random3D(blb); 
	float3 gradBRB = random3D(brb);
	float3 gradTLB = random3D(tlb);
	float3 gradTRB = random3D(trb);
	float3 gradBLF = random3D(blf);
	float3 gradBRF = random3D(brf);
	float3 gradTLF = random3D(tlf);
	float3 gradTRF = random3D(trf);
	
	float3 distToPixelFromBLB = gridFract - float3(0.0, 0.0, 0.0);
	float3 distToPixelFromBRB = gridFract - float3(1.0, 0.0, 0.0);
	float3 distToPixelFromTLB = gridFract - float3(0.0, 1.0, 0.0);
	float3 distToPixelFromTRB = gridFract - float3(1.0, 1.0, 0.0);
	float3 distToPixelFromBLF = gridFract - float3(0.0, 0.0, 1.0);
	float3 distToPixelFromBRF = gridFract - float3(1.0, 0.0, 1.0);
	float3 distToPixelFromTLF = gridFract - float3(0.0, 1.0, 1.0);
	float3 distToPixelFromTRF = gridFract - float3(1.0, 1.0, 1.0);
	
	float dotBLB = dot(gradBLB, distToPixelFromBLB);
	float dotBRB = dot(gradBRB, distToPixelFromBRB);
	float dotTLB = dot(gradTLB, distToPixelFromTLB);
	float dotTRB = dot(gradTRB, distToPixelFromTRB);
	float dotBLF = dot(gradBLF, distToPixelFromBLF);
	float dotBRF = dot(gradBRF, distToPixelFromBRF);
	float dotTLF = dot(gradTLF, distToPixelFromTLF);
	float dotTRF = dot(gradTRF, distToPixelFromTRF);
	
	return lerp(
		lerp(
			lerp(dotBLB, dotBRB, blur.x),
			lerp(dotTLB, dotTRB, blur.x), blur.y
		),
		lerp(
			lerp(dotBLF, dotBRF, blur.x),
			lerp(dotTLF, dotTRF, blur.x), blur.y
		), blur.z
	) + 0.5;
}

float3 rotateAroundYAxis(float3 p, float angle)
{
	float3x3 mat = {
		cos(angle), 0, sin(angle),
		0, 1, 0,
		-sin(angle), 0, cos(angle)
	};

	return mul(mat, p);
}

// calculate noise at multiple frequencies and do a weighted sum of the results
float noise3DMultipleFreq(float3 p)
{
	float noiseVal1 = noise3D(p * c_freq1);
	float noiseVal2 = noise3D(p * c_freq2);
	float noiseVal3 = noise3D(p * c_freq3);
	float noiseVal4 = noise3D(p * c_freq4);

	float4 noise = float4(noiseVal1, noiseVal2, noiseVal3, noiseVal4);

	return dot(noise, c_noiseWeights) * c_noiseAmp + c_noiseBias;
}

// negative if inside sphere, positive otherwise
float calculateSignedDistance(float3 pos, float sphereRadius)
{
	float displacement = -noise3DMultipleFreq(pos);

	float radius = sphereRadius + displacement;

	return length(pos) - radius;
};

float3 calculateHitColor(float displacement)
{
    float x = clamp(displacement, 0.0f, 1.0f);

    if (x < c_colorThreshholds[0])
        return lerp(c_color5, c_color4, x * 4.0f);
    else if (x < c_colorThreshholds[1])
        return lerp(c_color4, c_color3, x * 4.0f - 1.0f);
    else if (x < c_colorThreshholds[2])
        return lerp(c_color3, c_color2, x * 4.0f - 2.0f);

    return lerp(c_color2, c_color1, x * 4.0f - 3.0f);
}

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	float sphereRadius = c_sphereRadius;

	if (c_animateRadius)
	{
		sphereRadius = abs(sin(c_time / 4.0f)) * 3.0f;
	}

	// add +0.5 to get middle of pixel
	// subtract half width/height to get distance from center of image
	float directionX = (float(DTid.x) + 0.5f) - (float(c_resolution.x) / 2.0f);

	float directionY = (float(DTid.y) + 0.5f) - (float(c_resolution.y) / 2.0f);
	directionY *= -1.0f; // flip it so that positive is up
	
	float directionZ = -(float(c_resolution.y) / (2.0f * tan(c_fov / 2.0f)));

	float3 direction = normalize(float3(directionX, directionY, directionZ));

	float widthPercent = float(DTid.x) / float(c_resolution.x);
	float heightPercent = float(DTid.y) / float(c_resolution.y);

	float brightness = (widthPercent + heightPercent) / 2.0f;

	float3 position = c_cameraOrigin;

	float4 outColor = c_backgroundColor;

	for (int i = 0; i < c_rayMarchIterations; i++) 
	{
		float3 posRotated = position;

		if (c_rotate)
		{
			posRotated = rotateAroundYAxis(position, c_time * c_rotationSpeed);
		}

		float distanceFromOrigin = length(posRotated);

		float signedDistance = calculateSignedDistance(posRotated, sphereRadius);

		float displacement = distanceFromOrigin - signedDistance - sphereRadius;

		bool hitBool = (signedDistance < 0.0f);

		if (hitBool)
		{
			// calculate normal vector
			float epsilon = 0.1f;

			float dx = calculateSignedDistance(posRotated + float3(epsilon, 0.0f, 0.0f), sphereRadius) - signedDistance;
			float dy = calculateSignedDistance(posRotated + float3(0.0f, epsilon, 0.0f), sphereRadius) - signedDistance;
			float dz = calculateSignedDistance(posRotated + float3(0.0f, 0.0f, epsilon), sphereRadius) - signedDistance;

			float3 n = normalize(float3(dx, dy, dz));

			// calculate diffuse lighting
			float3 l = normalize(c_lightPosition - posRotated);

			float intensity = max(dot(l, n), 0.4f); // clamp to avoid pure black

			float3 hitColor = calculateHitColor((-displacement - 0.08f) * 2.0f) * intensity;

			outColor = float4(hitColor, 1.0f);

			break;
		}

		position += direction * max(signedDistance * c_rayMarchStepPercent, c_rayMarchMinStepSize);
	}

	direction.z = 0.0f;

	// These are just for debug purposes
	perlin1[DTid.xy] = float4(float3(1.0f, 1.0f, 1.0f) * noise3D(direction * c_freq1), 1.0f);
	perlin2[DTid.xy] = float4(float3(1.0f, 1.0f, 1.0f) * noise3D(direction * c_freq2), 1.0f);
	perlin3[DTid.xy] = float4(float3(1.0f, 1.0f, 1.0f) * noise3D(direction * c_freq3), 1.0f);
	perlin4[DTid.xy] = float4(float3(1.0f, 1.0f, 1.0f) * noise3D(direction * c_freq4), 1.0f);

	float sphereMax = (c_sphereRadius * 3.0f) - sphereRadius;
	float sphereS = (sphereRadius - c_sphereRadius) / sphereMax;

	float fadeToWhite = lerp(0.0f, 1.0f, sphereS);

	outColor.x += fadeToWhite;
	outColor.y += fadeToWhite;
	outColor.z += fadeToWhite;

	output[DTid.xy] = outColor;
}

/*
Shader Resources:
	Texture intput (as SRV)
	Texture output (as UAV)
*/
