//  technique, shader PathTrace
/*$(ShaderResources)*/

#include "DemofoxNeonDesert_common.hlsl"

// =======================================================================
// Scene Descriptions - both ray tracing and ray marched objects
// =======================================================================

void RayTraceMountainCylinders(in float3 rayPos, in float3 rayDir, inout SRayHitInfo hitInfo, in float seed, in float radius, in float maxMountainHeight)
{
    SMaterial material;
    material.diffuse = float3(1.0f, 1.0f, 1.0f);
    material.specular = float3(0.0f, 0.0f, 0.0f);
    material.roughness = 0.025f;
    material.emissive = float3(0.0f, 0.0f, 0.0f);

    SRayHitInfo oldHitInfo = hitInfo;

    if(TestCylinderTrace(rayPos, rayDir, hitInfo, float3(0.0f, -15.0f, 0.0f), radius, maxMountainHeight, material))
    {
        float3 hitPos = rayPos + rayDir * hitInfo.dist;
        float azimuthPercent = clamp((atan2(hitPos.z, hitPos.x) + c_pi) / c_twopi, 0.0f, 1.0f);
        float heightPercent = (hitPos.y + 15.0f) / maxMountainHeight;

        float thisHeightIndex = floor(azimuthPercent * 20.0f);
        float nextHeightIndex = mod(thisHeightIndex + 1.0f, 20.0f);
        float thisHeightIndexFraction = frac(azimuthPercent * 20.0f);

        float thisHeight = hash12(float2(thisHeightIndex, seed)) * 0.75f + 0.25f;
        float nextHeight = hash12(float2(nextHeightIndex, seed)) * 0.75f + 0.25f;

        float mountainHeight = lerp(thisHeight, nextHeight, thisHeightIndexFraction);

        if (heightPercent > mountainHeight)
        {
            hitInfo = oldHitInfo;
        }
        else
        {
            float3 brightPink = pow(float3(0.99f, 0.04f, 0.98f), float3(2.2f, 2.2f, 2.2f)) * 5.0f;
            
            float borderSize = 0.25f / maxMountainHeight;
            
            hitInfo.material.emissive = brightPink * smoothstep(mountainHeight - borderSize, mountainHeight, heightPercent);
        }
    }    
}

void RayTraceScene(in float3 rayPos, in float3 rayDir, inout SRayHitInfo hitInfo)
{
    // floor
    {
    	SMaterial material;
        material.diffuse = float3(0.1f, 0.1f, 0.1f);
        material.specular = float3(0.0f, 0.0f, 0.0f);
        material.roughness = 0.02f;
        material.emissive = float3(0.0f, 0.0f, 0.0f);
    	if (TestPlaneTrace(rayPos, rayDir, hitInfo, float4( normalize(float3(0.0f, 1.0f, 0.0f)), -5.0f), material))
        {
            float3 intersectPos = rayPos + rayDir * hitInfo.dist;
            float2 uv = intersectPos.xz;
                        
            uv = frac(uv / 75.0f);
            
            float dist = min(abs(uv.x - 0.5), abs(uv.y - 0.5));
            dist = step(dist, 0.005f);
                       
            float len2d = (cos(intersectPos.x * 0.01530f + intersectPos.y * 0.06320f) + 1.0f) / 2.5f;
            
            float3 brightPurple = pow(float3(0.73f, 0.06f, 0.99f), float3(2.2f, 2.2f, 2.2f));
            brightPurple = lerp(brightPurple, float3(0.0625f, 0.0f, 1.0f), len2d);
            
            float3 darkPurple = pow(float3(0.12f, 0.02f, 0.09f), float3(2.2f, 2.2f, 2.2f));
            
            hitInfo.material.emissive = lerp(darkPurple, brightPurple, dist);
            hitInfo.material.specular = lerp(float3(0.33f, 0.33f, 0.33f), float3(0.0f, 0.0f, 0.0f), dist);
        }
    }
    
    // reflective balls
    {
    	SMaterial material;
        material.diffuse = float3(0.5f, 0.5f, 0.5f);
        material.specular = float3(1.0f, 1.0f, 1.0f);
        material.roughness = 0.035f;
        material.emissive = float3(0.0f, 0.0f, 0.0f);
    	TestSphereTrace(rayPos, rayDir, hitInfo, float4(20.0f, 5.0f, -20.0f, 10.0f), material);
        
        TestSphereTrace(rayPos, rayDir, hitInfo, float4(-60.0f, 0.0f, 20.0f, 10.0f), material);
        
        
        TestSphereTrace(rayPos, rayDir, hitInfo, float4(0.0f, 5.0f,  -65.0f, 10.0f), material);
        
        
        TestSphereTrace(rayPos, rayDir, hitInfo, float4(15.0f, -3.0f, 65.0f, 2.0f), material);
    }
    
    // Cylinders with mountain cutouts - draw front to back
    {
        RayTraceMountainCylinders(rayPos, rayDir, hitInfo,  284.0f, 700.0f, 50.0f);
        RayTraceMountainCylinders(rayPos, rayDir, hitInfo, 1337.0f, 800.0f, 150.0f);
        RayTraceMountainCylinders(rayPos, rayDir, hitInfo, 1932.0f, 900.0f, 250.0f);
	}
    
    // dark sun with gradient
    {
    	SMaterial material;
        material.diffuse = float3(0.0f, 0.0f, 0.0f);
        material.specular = float3(0.0f, 0.0f, 0.0f);
        material.roughness = 0.0f;
        material.emissive = float3(0.0f, 0.0f, 0.0f);
        
        SRayHitInfo oldHitInfo = hitInfo;
        
    	if (TestPlaneTrace(rayPos, rayDir, hitInfo, float4( normalize(float3(0.0f, 0.0f, -1.0f)), -1000.0f), material))
        {
            float3 intersectPos = rayPos + rayDir * hitInfo.dist;
            float2 uv = intersectPos.xy;
            
            int stripe = int((-uv.y + 500.0f) / 50.0f);
            if (stripe > 1)
                stripe = (stripe-1) % 2;
            else
                stripe = 0;
            
            if (length(uv) > 500.0f)
            {
                hitInfo = oldHitInfo;
            }
            else if (stripe == 1)
            {
                hitInfo.material.emissive = pow(float3(0.29f, 0.03f, 0.42f) * 0.55f, float3(2.2f, 2.2f, 2.2f));
            }
            else
            {
                float3 topColor = pow(float3(0.99f, 0.04f, 0.98f), float3(2.2f, 2.2f, 2.2f));
                float3 bottomColor = pow(float3(0.12f, 0.02f, 0.09f), float3(2.2f, 2.2f, 2.2f));

                float lerpAmount = clamp((uv.y - 100.0f) / 400.0f, 0.0f, 1.0f);
                lerpAmount = Bias(lerpAmount, 0.05);
                hitInfo.material.emissive = lerp(bottomColor, topColor, lerpAmount);
            }
        }
    }
    
    // ringed planet
    {
        // planet
        {
            SMaterial material;
            material.diffuse = float3(0.0f, 0.0f, 0.0f);
            material.specular = float3(0.0f, 0.0f, 0.0f);
            material.roughness = 0.0f;
            material.emissive = float3(0.0f, 0.0f, 0.0f);
            if(TestSphereTrace(rayPos, rayDir, hitInfo, float4(0.0f, 500.0f, -1200.0f, 200.0f), material))
            {
                // TODO: gradient: yellow to red, then red to purple?
                float3 intersectPos = rayPos + rayDir * hitInfo.dist;

                float lerpTop = clamp((intersectPos.y - 500.0f) / 200.0f, 0.0f, 1.0f);
                float lerpMiddle = 1.0f - clamp((intersectPos.y - 500.0f) / 200.0f, 0.0f, 1.0f);
                float lerpBottom = clamp((intersectPos.y - 500.0f) / -200.0f, 0.0f, 1.0f);
                
                lerpTop = Bias(lerpTop, 0.1f);
                lerpMiddle = Bias(lerpMiddle, 0.9f);
                lerpBottom = Bias(lerpBottom, 0.1f);

                float3 color = float3(1.0f, 1.0f, 0.0f) * lerpTop;
                color += float3(1.0f, 0.0f, 0.0f) * lerpMiddle;
                color += float3(1.0f, 0.0f, 1.0f) * lerpBottom;

                hitInfo.material.emissive = color;
            }
        }
        
        // ring
        {
            SMaterial material;
            material.diffuse = float3(0.0f, 0.0f, 0.0f);
            material.specular = float3(0.0f, 0.0f, 0.0f);
            material.roughness = 0.0f;
            material.emissive = float3(0.8f, 0.0f, 1.0f);// * 2.0f;// * 0.1f;

            TestCylinderTrace(rayPos, rayDir, hitInfo, float3(0.0f, 460.0f, -1200.0f), 300.0f, 10.0f, material);
            TestCylinderTrace(rayPos, rayDir, hitInfo, float3(0.0f, 460.0f, -1200.0f), 350.0f, 10.0f, material);
            TestCylinderTrace(rayPos, rayDir, hitInfo, float3(0.0f, 460.0f, -1200.0f), 375.0f, 10.0f, material);
        }
    }
    
    // sky dome
    {
    	SMaterial material;
        material.diffuse = float3(0.0f, 0.0f, 0.0f);
        material.specular = float3(0.0f, 0.0f, 0.0f);
        material.roughness = 0.0f;
        material.emissive = pow(float3(0.29f, 0.03f, 0.42f) * 0.55f, float3(2.2f, 2.2f, 2.2f));
    	if(TestSphereTrace(rayPos, rayDir, hitInfo, float4(0.0f, 0.0f, 0.0f, 2000.0f), material))
        {
            const float c_skyStarCells = 45.0f;
            
            // convert the hit position to polar, but percentages from 0 to 1.
            // make the pole at the horizon to hide the distortion that would be in the sky otherwise.
            float3 hitPos = (rayPos + rayDir * hitInfo.dist).yzx;
            float theta = (atan2(hitPos.y, hitPos.x) + c_pi) / c_twopi;
            float phi = (atan2(sqrt(hitPos.x*hitPos.x + hitPos.y*hitPos.y), hitPos.z) + c_pi) / c_twopi;

            // figure out what cell we are in and the offset in that cell
            float2 cellIndex = float2(floor(theta * c_skyStarCells), floor(phi * c_skyStarCells));
            float2 cellUV = float2(frac(theta * c_skyStarCells), frac(phi * c_skyStarCells));

            // make a star at a random place in the cell
            float starRadius = hash12(cellIndex) * 0.1f + 0.07f;
			float2 starPos = hash22(cellIndex) * (1.0f - starRadius * 2.0f) + starRadius;
            float dist = sdStar(starPos - cellUV, starRadius, 4, 3.75f);
            
            dist *= pow(length(starPos - cellUV) / starRadius, 10.0f);
                       
            float value = step(dist, 0.0f);
			value *= pow(1.0f - clamp(length(starPos - cellUV) / starRadius, 0.0f, 1.0f), 3.0f);
                        
            hitInfo.material.emissive = lerp(material.emissive, float3(1.0f, 1.0f, 1.0f), value);            
        }
    }
}

SRayHitInfo TestSceneMarch(in float3 rayPos)
{
    SRayHitInfo hitInfo;
    hitInfo.hitAnObject = false;
    hitInfo.dist = c_superFar;
    
    // glowing triangles
    {

        float3 A = float3(0.0f, 0.0f, 0.0f);
        float3 B = float3(1.5f, 3.0f, 0.0f);
        float3 C = float3(3.0f, 0.0f, 0.0f);
        float lineWidth = 0.1f;
        
        float3 center = (A + B + C) / 3.0f;
        A -= center;
        B -= center;
        C -= center;
        
        A *= 3.0f;
        B *= 3.0f;
        C *= 3.0f;
        
        // foreground purple one
        SMaterial material;
        material.diffuse = float3(0.0f, 0.0f, 0.0f);
        material.specular = float3(0.0f, 0.0f, 0.0f);
        material.roughness = 0.0f;
        material.emissive = pow(float3(0.73f, 0.06f, 0.99f), float3(2.2f, 2.2f, 2.2f)) * 10.0f;            

        TestLineMarch(rayPos, hitInfo, A, B, lineWidth, material);
        TestLineMarch(rayPos, hitInfo, B, C, lineWidth, material);
        TestLineMarch(rayPos, hitInfo, C, A, lineWidth, material);
        
        // blue one slightly behind
        material.emissive = pow(float3(0.3f, 0.15f, 1.0f), float3(2.2f, 2.2f, 2.2f)) * 10.0f;
        A += float3(0.0f, 0.0f, 5.0f);
        B += float3(0.0f, 0.0f, 5.0f);
        C += float3(0.0f, 0.0f, 5.0f);
        TestLineMarch(rayPos, hitInfo, A, B, lineWidth, material);
        TestLineMarch(rayPos, hitInfo, B, C, lineWidth, material);
        TestLineMarch(rayPos, hitInfo, C, A, lineWidth, material);        
        
        // red one behind more
        material.emissive = pow(float3(1.0f, 0.15f, 0.3f), float3(2.2f, 2.2f, 2.2f)) * 10.0f;
        A += float3(0.0f, 0.0f, 5.0f);
        B += float3(0.0f, 0.0f, 5.0f);
        C += float3(0.0f, 0.0f, 5.0f);
        TestLineMarch(rayPos, hitInfo, A, B, lineWidth, material);
        TestLineMarch(rayPos, hitInfo, B, C, lineWidth, material);
        TestLineMarch(rayPos, hitInfo, C, A, lineWidth, material);              
	}    

    // a neon cactus
    {
    	SMaterial material;
        material.diffuse = float3(0.0f, 0.0f, 0.0f);
        material.specular = float3(0.0f, 0.0f, 0.0f);
        material.roughness = 0.0f;
        material.emissive = pow(float3(0.73f, 0.06f, 0.99f), float3(2.2f, 2.2f, 2.2f)) * 10.0f;
        
        float3 cactusOffset = float3(0.0f, 0.0f, 50.0f);
        
        // main body section
        {
            float3 A = float3(-40.0f, -10.0f, 0.0f) + cactusOffset;
            float3 B = float3(-40.0f, 5.0f, 1.0f) + cactusOffset;
            float3 C = float3(-40.0f, 20.0f, 0.0f) + cactusOffset;
            TestBezierMarch(rayPos, hitInfo, A, B, C, 2.0f, material);
        }
        
        // Arm going to left
        {
            float3 A = float3(-40.0f, 5.0f, 1.0f) + cactusOffset;
            float3 B = float3(-32.5f, 10.0f, 0.0f) + cactusOffset;
            float3 C = float3(-32.5f, 15.0f, -1.0f) + cactusOffset;
            TestBezierMarch(rayPos, hitInfo, A, B, C, 1.0f, material);
        }
        
        // Arm going to right
        {
            float3 A = float3(-40.0f, 2.0f, 1.0f) + cactusOffset;
            float3 B = float3(-47.5f, 7.0f, 2.0f) + cactusOffset;
            float3 C = float3(-47.5f, 13.0f, 4.0f) + cactusOffset;
            TestBezierMarch(rayPos, hitInfo, A, B, C, 1.0f, material);
        }        
        
    }

    return hitInfo;
}

// =======================================================================
// Ray Marching
// =======================================================================

void RayMarchScene(in float3 startingRayPos, in float3 rayDir, inout SRayHitInfo oldHitInfo)
{
    SMaterial dummyMaterial = (SMaterial)0;
    
    float rayDistance = c_minimumRayHitTime;
    float lastRayDistance = c_minimumRayHitTime;
    
    float lastHitInfoDist = 0.0f;
    
    SRayHitInfo newHitInfo = oldHitInfo;
    newHitInfo.hitAnObject = false;
    
    for (int stepIndex = 0; stepIndex < /*$(Variable:rayMarchSteps)*/; ++stepIndex)
    {
        float3 rayPos = startingRayPos + rayDistance * rayDir;
        
        newHitInfo = TestSceneMarch(rayPos);
        
        // these two lines are so that the material code goes away when the test functions are inlines
        newHitInfo.normal = float3(0.0f, 0.0f, 0.0f);
        newHitInfo.material = dummyMaterial;
        
        newHitInfo.hitAnObject = newHitInfo.dist < 0.0f;
        if (newHitInfo.hitAnObject)
            break;
        
        lastRayDistance = rayDistance;
        rayDistance += max(newHitInfo.dist, /*$(Variable:minStepDistance)*/);

        lastHitInfoDist = newHitInfo.dist;
        
        if (rayDistance > oldHitInfo.dist)
            break;
    }
    
    if (newHitInfo.hitAnObject)
    {
		float refinedHitPercent = lastHitInfoDist / (lastHitInfoDist - newHitInfo.dist);
        newHitInfo.dist = lerp(lastRayDistance, rayDistance, refinedHitPercent);
        
        if (newHitInfo.dist < oldHitInfo.dist)
            oldHitInfo = newHitInfo;
    }
}

// =======================================================================
// Iterative bounce ray tracing
// =======================================================================
    
float3 GetColorForRay(in float3 startRayPos, in float3 startRayDir, inout uint rngState)
{
    float3 ret = float3(0.0f, 0.0f, 0.0f);
    float3 colorMultiplier = float3(1.0f, 1.0f, 1.0f);
    
    float3 rayPos = startRayPos;
    float3 rayDir = startRayDir;
       
    for (int i = 0; i <= /*$(Variable:numBounces)*/; ++i)
    {
        SRayHitInfo hitInfo;
		hitInfo.hitAnObject = false;
        hitInfo.dist = c_superFar;
        
        // ray trace first, which also gives a maximum distance for ray marching
        RayTraceScene(rayPos, rayDir, hitInfo);
        RayMarchScene(rayPos, rayDir, hitInfo);
        
        if (!hitInfo.hitAnObject)
        {
            // handle ray misses
            break;
        }
                      
        // update the ray position
        rayPos += rayDir * hitInfo.dist;
               
        // get the material info if it was a ray marched object
        if (hitInfo.rayMarchedObject)
			hitInfo = TestSceneMarch(rayPos);       
                
		// add in emissive lighting
        ret += hitInfo.material.emissive * colorMultiplier;
        
        // figure out whether we are going to shoot out a specular or diffuse ray.
        // If neither, exit
        float diffuseLength = length(hitInfo.material.diffuse);        
        float specularLength = length(hitInfo.material.specular);
        if (diffuseLength + specularLength == 0.0f)
            break;
        float specularWeight = specularLength / (diffuseLength + specularLength);       
        float doSpecular = float(RandomFloat01(rngState) < specularWeight);
       
        // set up the next ray direction
        float roughness = lerp(1.0f, hitInfo.material.roughness, doSpecular);
        float3 reflectDir = reflect(rayDir, hitInfo.normal);
        float3 randomDir = RandomUnitVector(rngState);
        rayDir = normalize(lerp(reflectDir, randomDir, roughness));
        
        if (dot(rayDir, hitInfo.normal) < 0.0f)
            rayDir *= -1.0f;        
        
        // move the ray away from the surface it hit a little bit
        rayPos += hitInfo.normal * c_rayPosNormalNudge;
        
        // Make all future light affected be modulated by either the diffuse or specular reflection color
        // depending on which we are doing.
        // Attenuate diffuse by the dot product of the outgoing ray and the normal (aka multiply diffuse by cosine theta or N dot L)
        float NdotL = dot(hitInfo.normal, rayDir);
        colorMultiplier *= lerp(hitInfo.material.diffuse * NdotL, hitInfo.material.specular, doSpecular);        
    }
    
    return ret;
}

// =======================================================================
// Main function
// =======================================================================

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
    // initialize a random number state based on frag coord and frame
    uint rngState = uint(uint(DTid.x) * uint(1973) + uint(DTid.y) * uint(9277) + uint(/*$(Variable:iFrame)*/) * uint(26699)) | uint(1);
    
    // add sub pixel jitter for anti aliasing
    float2 fragCoordJittered = float2(DTid.xy) + (float2(RandomFloat01(rngState), RandomFloat01(rngState)) - 0.5f);
    
    // get the camera vectors
	float3 rayOrigin;
	float3 rayDirection;
	{
		uint w, h;
		accumulation.GetDimensions(w, h);
		float2 dimensions = float2(w, h);

		float2 screenPos = fragCoordJittered / dimensions * 2.0 - 1.0;
		screenPos.y = -screenPos.y;

		float4 world = mul(float4(screenPos, 0.0f, 1), /*$(Variable:InvViewProjMtx)*/);
		world.xyz /= world.w;

		rayOrigin = /*$(Variable:CameraPos)*/;
		rayDirection = normalize(world.xyz - rayOrigin);
	}

    // simulate a thin lens (depth of field and circular bokeh)
    if (/*$(Variable:apertureRadius)*/ > 0.0f)
    {
		float3 cameraRight = mul(float4(1.0f, 0.0f, 0.0f, 0.0f), /*$(Variable:InvViewMtx)*/).xyz;
		float3 cameraUp = mul(float4(0.0f, 1.0f, 0.0f, 0.0f), /*$(Variable:InvViewMtx)*/).xyz;

        // calculate point on the focal plane
        float3 focalPlanePoint = rayOrigin + rayDirection * /*$(Variable:focalPlaneDistance)*/;
        
        // calculate a random point on the aperture
        float angle = RandomFloat01(rngState) * 2.0f * c_pi;
		float radius = sqrt(RandomFloat01(rngState)) * /*$(Variable:apertureRadius)*/;
		float2 offset = float2(cos(angle), sin(angle)) * radius;
        
        // update the camera pos
        rayOrigin += offset.x * cameraRight + offset.y * cameraUp;
        
        // update the ray direction
        rayDirection = normalize(focalPlanePoint - rayOrigin);  
    }
    
    // get the linear color of the ray
    float3 color = GetColorForRay(rayOrigin, rayDirection, rngState);
    
    // average the frames together
    float4 lastFrameColor = accumulation[DTid.xy];
    float blend = (/*$(Variable:iFrame)*/ < 2 || /*$(Variable:Reset)*/ || lastFrameColor.a == 0.0f || /*$(Variable:CameraChanged)*/) ? 1.0f : 1.0f / (1.0f + (1.0f / lastFrameColor.a));
    color = lerp(lastFrameColor.rgb, color, blend);	

    // accumulate the result
    accumulation[DTid.xy] = float4(color, blend);	
}
