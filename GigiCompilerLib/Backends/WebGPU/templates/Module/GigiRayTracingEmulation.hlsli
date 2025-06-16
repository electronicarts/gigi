///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#if GIGI_SCENE_IS_BVH
#include "BVH.hlsli"
#endif

/*
The following must be defined before this file is included:
* GIGI_RTSCENENAME                - The name of the buffer resource holding the scene data
* GIGI_RTSCENETYPENAME            - The view datatype of the buffer
* GIGI_RTSCENEPOSFIELDNAME        - The name of the position field in the buffer, proceeded by a dot. Leave this blank or set it to .xyz for simple POD type buffers that don't have struct fields.
* GIGI_ISRAYGENSHADER             - 1 if this is included for a ray gen shader. 0 if not.
* GIGI_INTERSECTIONINFOOBJECTTYPE - The intersection info object type. BuiltInTriangleIntersectionAttributes or a custom struct type.
* GIGI_INTERSECTIONINFOCUSTOM     - 1 if a custom type, 0 if not.
* GIGI_SCENE_IS_BVH               - Whether to use the BVH code path (1) or not (0)

It assumes the following symbols are also available:
* SCENE_IS_AABBS  - 1 if the scene is AABBs (and use intersection shaders), 0 otherwise. AABBs cannot also be bvh.
These are slang properties that are defined in the wgsl at runtime of webgpu to control which code paths to take.

*/

struct RayDesc
{
    float3 Origin;
    float TMin;
    float3 Direction;
    float TMax;
};

#define COMMITTED_NOTHING 0
#define COMMITTED_TRIANGLE_HIT 1
#define COMMITTED_PROCEDURAL_PRIMITIVE_HIT 2

#define CANDIDATE_NON_OPAQUE_TRIANGLE 0
#define CANDIDATE_PROCEDURAL_PRIMITIVE 1

#if GIGI_SCENE_IS_BVH == 0

// returns hitT. -1.0 for a miss
float RayVsAABB(in float3 rayPos, in float3 rayDir, in float3 axesMin, in float3 axesMax)
{
    float rayMinTime = -3.402823466e+38f;
    float rayMaxTime = 3.402823466e+38f;

    for (int axis = 0; axis < 3; ++axis)
    {
        float axisMin = axesMin[axis];
        float axisMax = axesMax[axis];

        // if the ray is parallel with this axis
        if (abs(rayDir[axis]) < 0.0001f)
        {
            // if the ray isn't in the box, bail out. We know there's no intersection.
            if (rayPos[axis] < axisMin || rayPos[axis] > axisMax)
                return -1.0f;
        }
        else
        {
            // figure out the intersection times of the ray with the 2 values of this axis
            float axisMinTime = (axisMin - rayPos[axis]) / rayDir[axis];
            float axisMaxTime = (axisMax - rayPos[axis]) / rayDir[axis];

            // make sure min < max
            if (axisMinTime > axisMaxTime)
            {
                float temp = axisMinTime;
                axisMinTime = axisMaxTime;
                axisMaxTime = temp;
            }

            // union this time slice with our running total time slice
            rayMinTime = max(rayMinTime, axisMinTime);
            rayMaxTime = min(rayMaxTime, axisMaxTime);

            // if our time slice shrinks to zero, we don't intersect
            if (rayMinTime > rayMaxTime)
                return -1.0f;
        }
    }

    // if we got here, we do intersect, but the ray may start inside the box
    return (rayMinTime >= 0.0f) ? rayMinTime : rayMaxTime;
}

// returns float3(hitT, bary.v, bary.w). hitT is -1.0 for a miss.
float3 RayVsTriangle(in float3 rayPos, in float3 rayDir, in float3 posA, in float3 posB, in float3 posC)
{
    float3 e1 = posB - posA;
    float3 e2 = posC - posA;

    float3 q = cross(rayDir, e2);
    float a = dot(e1, q);

    if (abs(a) == 0.0f)
        return float3(-1.0f, 0.0f, 0.0f);

    float3 s = (rayPos - posA) / a;
    float3 r = cross(s, e1);
    float3 b; // barycentric coordinates
    b.x = dot(s, q);
    b.y = dot(r, rayDir);
    b.z = 1.0f - b.x - b.y;

    if (b.x < 0.0f || b.y < 0.0f || b.z < 0.0f)
        return float3(-1.0f, 0.0f, 0.0f);

    float hitT = dot(e2,r);

    return float3(hitT, b.x, b.y);
}

#endif

// Emulating inline ray query functionality
// https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#rayquery
struct RayQuery<uint FLAGS>
{
    bool m_finished = true;
    int m_candidatePrimitiveIndex = -1;
    RayDesc m_ray;
    float3 m_minTAndBary = float3(-1.0f, 0.0f, 0.0f);
    uint m_rayFlags = 0;

    [mutating]
    void TraceRayInline(
        StructuredBuffer<GIGI_RTSCENETYPENAME> scene,
        uint rayFlags,
        uint InstanceInclusionMask,
        RayDesc ray
    )
    {
        m_finished = false;
        m_candidatePrimitiveIndex = -1;
        m_ray = ray;
        m_minTAndBary = float3(-1.0f, 0.0f, 0.0f);
        m_rayFlags = rayFlags | FLAGS;
    }

    [mutating]
    bool Proceed()
    {
        if (m_finished)
            return false;

        // Find the closest triangle that the ray hits, if any
        m_minTAndBary = float3(m_ray.TMax, 0.0f, 0.0f);
        m_candidatePrimitiveIndex = -1;

        #if GIGI_SCENE_IS_BVH == 1
            // Intersect the ray against the BVH
            RayVsBVHResult result = RayVsMesh_BVH(GIGI_RTSCENENAME, m_ray.Origin, m_ray.Direction, rcp(m_ray.Direction), m_ray.TMax);

            // Store the results if we hit anything
            if (result.distance >= 0.0f)
            {
                m_minTAndBary = float3(result.distance, result.barycentrics);
                m_candidatePrimitiveIndex = result.triangleIndex;
            }
        #else
            // Calculate how many primitives there are
            uint2 numVertsAndStride;
            GIGI_RTSCENENAME.GetDimensions(numVertsAndStride.x, numVertsAndStride.y);
            uint numTris = numVertsAndStride.x / 3;

            // Brute force search all triangles
            for (uint i = 0; i < numTris; ++i)
            {
                float3 posA = GIGI_RTSCENENAME[i * 3 + 0]GIGI_RTSCENEPOSFIELDNAME;
                float3 posB = GIGI_RTSCENENAME[i * 3 + 1]GIGI_RTSCENEPOSFIELDNAME;
                float3 posC = GIGI_RTSCENENAME[i * 3 + 2]GIGI_RTSCENEPOSFIELDNAME;

                float3 hitTAndBary = RayVsTriangle(m_ray.Origin, m_ray.Direction, posA, posB, posC);

                // If we have a new candidate for closest hit
                if (hitTAndBary.x >= m_ray.TMin && hitTAndBary.x <= m_minTAndBary.x)
                {
                    m_minTAndBary = hitTAndBary;
                    m_candidatePrimitiveIndex = i;
                }
            }
        #endif // GIGI_SCENE_IS_BVH

        // Return true if we hit something
        m_finished = true;
        return m_candidatePrimitiveIndex != -1;
    }

    [mutating]
    void Abort()
    {
        // make it so proceed always returns false.
        m_finished = true;
        m_candidatePrimitiveIndex = -1;
        m_minTAndBary = float3(-1.0f, 0.0f, 0.0f);
    }

    uint CommittedStatus()
    {
        return (m_candidatePrimitiveIndex != -1) ? COMMITTED_TRIANGLE_HIT : COMMITTED_NOTHING;
    }

    uint RayFlags()
    {
        return m_rayFlags;
    }

    float3 WorldRayOrigin()
    {
        return m_ray.Origin;
    }

    float3 WorldRayDirection()
    {
        return m_ray.Direction;
    }

    float RayTMin()
    {
        return m_ray.TMin;
    }

    float CandidateTriangleRayT()
    {
        return m_minTAndBary.x;
    }

    float CommittedRayT()
    {
        return m_minTAndBary.x;
    }

    uint CandidateInstanceIndex()
    {
        return 0;
    }

    uint CandidateInstanceID()
    {
        return 0;
    }

    uint CandidateInstanceContributionToHitGroupIndex()
    {
        return 0;
    }

    uint CandidateGeometryIndex()
    {
        return 0;
    }

    uint CandidatePrimitiveIndex()
    {
        return (uint)m_candidatePrimitiveIndex;
    }

    float3 CandidateObjectRayOrigin()
    {
        return m_ray.Origin;
    }

    float3 CandidateObjectRayDirection()
    {
        return m_ray.Direction;
    }

    float3x4 CandidateObjectToWorld3x4()
    {
        return float3x4(1, 0, 0,   0, 1, 0,   0, 0, 1,   0, 0, 0);
    }

    float4x3 CandidateObjectToWorld4x3()
    {
        return float4x3(1, 0, 0, 0,   0, 1, 0, 0,   0, 0, 1, 0);
    }

    float3x4 CandidateWorldToObject3x4()
    {
        return float3x4(1, 0, 0,   0, 1, 0,   0, 0, 1,   0, 0, 0);
    }

    float4x3 CandidateWorldToObject4x3()
    {
        return float4x3(1, 0, 0, 0,   0, 1, 0, 0,   0, 0, 1, 0);
    }

    uint CommittedInstanceIndex()
    {
        return 0;
    }

    uint CommittedInstanceID()
    {
        return 0;
    }

    uint CommittedInstanceContributionToHitGroupIndex()
    {
        return 0;
    }

    uint CommittedGeometryIndex()
    {
        return 0;
    }

    uint CommittedPrimitiveIndex()
    {
        return (uint)m_candidatePrimitiveIndex;
    }

    float3 CommittedObjectRayOrigin()
    {
        return m_ray.Origin;
    }

    float3 CommittedObjectRayDirection()
    {
        return m_ray.Direction;
    }

    float3x4 CommittedObjectToWorld3x4()
    {
        return float3x4(1, 0, 0,   0, 1, 0,   0, 0, 1,   0, 0, 0);
    }

    float4x3 CommittedObjectToWorld4x3()
    {
        return float4x3(1, 0, 0, 0,   0, 1, 0, 0,   0, 0, 1, 0);
    }

    float3x4 CommittedWorldToObject3x4()
    {
        return float3x4(1, 0, 0,   0, 1, 0,   0, 0, 1,   0, 0, 0);
    }

    float4x3 CommittedWorldToObject4x3()
    {
        return float4x3(1, 0, 0, 0,   0, 1, 0, 0,   0, 0, 1, 0);
    }

    float2 CandidateTriangleBarycentrics()
    {
        return m_minTAndBary.yz;
    }

    bool CandidateTriangleFrontFace()
    {
        return true;
    }

    float2 CommittedTriangleBarycentrics()
    {
        return m_minTAndBary.yz;
    }

    bool CommittedTriangleFrontFace()
    {
        return true;
    }

    uint CandidateType()
    {
        return CANDIDATE_NON_OPAQUE_TRIANGLE;
    }

    bool CandidateProceduralPrimitiveNonOpaque()
    {
        return false;
    }

    void CommitNonOpaqueTriangleHit()
    {
    }

    void CommitProceduralPrimitiveHit(float tHit)
    {
    }
};

#if GIGI_ISRAYGENSHADER

bool IntersectShaderReportHit(in RayDesc _ray, inout float _RayTCurrent, in uint _hitGroupIndex, inout Payload _payload, in uint _PrimitiveIndex, inout uint _HitPrimitiveIndex, inout GIGI_INTERSECTIONINFOOBJECTTYPE _attributes, in float tHit, in uint HitKind, in GIGI_INTERSECTIONINFOOBJECTTYPE Attributes)
{
    if (tHit < _ray.TMin || tHit > _RayTCurrent)
        return false;

    // Handle AnyHit shaders
    uint anyHitResult = AnyHitResult_Continue;
    RunAnyHitShader(_payload, Attributes, anyHitResult, _ray, _RayTCurrent, _PrimitiveIndex, _hitGroupIndex);
    // Ignore the hit if the any hit shader said we should
    if (anyHitResult == AnyHitResult_IgnoreHit)
        return false;

    _RayTCurrent = tHit;
    _HitPrimitiveIndex = _PrimitiveIndex;
    _attributes = Attributes;

    // Need to find out what happens if this occurs.
    // We could make the calling function exit out, with some cleverness, but not sure what the appropriate action is here
    // Accept this hit as the final result if the any hit shader said we should
    //if (anyHitResult == AnyHitResult_AcceptHitAndEndSearch)
    //    break;

    return true;
}

struct BuiltInTriangleIntersectionAttributes
{
    float2 barycentrics;
};

static const uint AnyHitResult_Continue = 0;
static const uint AnyHitResult_IgnoreHit = 1;
static const uint AnyHitResult_AcceptHitAndEndSearch = 2;

#if GIGI_SCENE_IS_BVH == 1

void RayVsScene_BVH(
    in StructuredBuffer<GIGI_RTSCENETYPENAME> combinedBVH,
    in uint RayFlags,
    in uint InstanceInclusionMask,
    in uint RayContributionToHitGroupIndex,
    in uint MultiplierForGeometryContributionToHitGroupIndex,
    in uint MissShaderIndex,
    in RayDesc Ray,
    inout Payload payload
)
{
    // adapted from https://github.com/jbikker/tinybvh/blob/main/traverse_bvh2.cl to include any hit, closest hit, and miss shader invocations.

    uint hitGroupIndex = RayContributionToHitGroupIndex;
    GIGI_INTERSECTIONINFOOBJECTTYPE intersectionInfo = (GIGI_INTERSECTIONINFOOBJECTTYPE)0;

    bool forceExitSearch = false;

    float3 O = Ray.Origin;
    float3 D = Ray.Direction;
    float3 rD = rcp(D);
    float tmax = Ray.TMax;

    // Calculate the offset of where each buffer starts. These are in uints, not bytes.
    uint vertsOffset = 2;
    uint altNodeOffset = vertsOffset + combinedBVH[0] / 4;
    uint idxOffset = altNodeOffset + combinedBVH[1] / 4;

    // Offsets into the Struct_BVHNode structure. These are in uints, not bytes.
    static const uint lminOffset = 0;
    static const uint leftOffset = 3;
    static const uint lmaxOffset = 4;
    static const uint rightOffset = 7;
    static const uint rminOffset = 8;
    static const uint triCountOffset = 11;
    static const uint rmaxOffset = 12;
    static const uint firstTriOffset = 15;
    static const uint nodeSize = 16;

    // traverse BVH
    float4 hit;
    hit.x = tmax;
    uint node = 0;
    uint stack[STACK_SIZE];
    uint stackPtr = 0;
    while (!forceExitSearch)
    {
        // fetch the node
        const float3 lmin = float3(
            asfloat(combinedBVH[altNodeOffset + node * nodeSize + lminOffset + 0]),
            asfloat(combinedBVH[altNodeOffset + node * nodeSize + lminOffset + 1]),
            asfloat(combinedBVH[altNodeOffset + node * nodeSize + lminOffset + 2])
        );
        const float3 lmax = float3(
            asfloat(combinedBVH[altNodeOffset + node * nodeSize + lmaxOffset + 0]),
            asfloat(combinedBVH[altNodeOffset + node * nodeSize + lmaxOffset + 1]),
            asfloat(combinedBVH[altNodeOffset + node * nodeSize + lmaxOffset + 2])
        );
        const float3 rmin = float3(
            asfloat(combinedBVH[altNodeOffset + node * nodeSize + rminOffset + 0]),
            asfloat(combinedBVH[altNodeOffset + node * nodeSize + rminOffset + 1]),
            asfloat(combinedBVH[altNodeOffset + node * nodeSize + rminOffset + 2])
        );
        const float3 rmax = float3(
            asfloat(combinedBVH[altNodeOffset + node * nodeSize + rmaxOffset + 0]),
            asfloat(combinedBVH[altNodeOffset + node * nodeSize + rmaxOffset + 1]),
            asfloat(combinedBVH[altNodeOffset + node * nodeSize + rmaxOffset + 2])
        );
        const uint triCount = combinedBVH[altNodeOffset + node * nodeSize + triCountOffset];
        if (triCount > 0)
        {
            // process leaf node
            const uint firstTri = combinedBVH[altNodeOffset + node * nodeSize + firstTriOffset];
            for (uint i = 0; i < triCount; i++)
            {
                const uint triIdx = combinedBVH[idxOffset + firstTri + i];
#if 1//#ifdef ISAPPLE
                // FIX error: initializing 'const __private float4 *__private' with an expression of type '__global float4 *' changes address space of pointer
                const float4 tri[3] = 
                {
                    float4(
                        asfloat(combinedBVH[vertsOffset + (3 * triIdx + 0) * 4 + 0]),
                        asfloat(combinedBVH[vertsOffset + (3 * triIdx + 0) * 4 + 1]),
                        asfloat(combinedBVH[vertsOffset + (3 * triIdx + 0) * 4 + 2]),
                        asfloat(combinedBVH[vertsOffset + (3 * triIdx + 0) * 4 + 3])
                    ),
                    float4(
                        asfloat(combinedBVH[vertsOffset + (3 * triIdx + 1) * 4 + 0]),
                        asfloat(combinedBVH[vertsOffset + (3 * triIdx + 1) * 4 + 1]),
                        asfloat(combinedBVH[vertsOffset + (3 * triIdx + 1) * 4 + 2]),
                        asfloat(combinedBVH[vertsOffset + (3 * triIdx + 1) * 4 + 3])
                    ),
                    float4(
                        asfloat(combinedBVH[vertsOffset + (3 * triIdx + 2) * 4 + 0]),
                        asfloat(combinedBVH[vertsOffset + (3 * triIdx + 2) * 4 + 1]),
                        asfloat(combinedBVH[vertsOffset + (3 * triIdx + 2) * 4 + 2]),
                        asfloat(combinedBVH[vertsOffset + (3 * triIdx + 2) * 4 + 3])
                    ),
                };
#else
                const float4* tri = verts + 3 * triIdx;
#endif
                // triangle intersection - M?ller-Trumbore
                const float4 edge1 = tri[1] - tri[0], edge2 = tri[2] - tri[0];
                const float3 h = cross( D, edge2.xyz );
                const float a = dot( edge1.xyz, h );
                if (abs( a ) < 0.0000001f) continue;
                const float f = 1 / a;
                const float3 s = O - tri[0].xyz;
                const float u = f * dot( s, h );
                const float3 q = cross( s, edge1.xyz );
                const float v = f * dot( D, q );
                if (u < 0 || v < 0 || u + v > 1) continue;
                const float d = f * dot( edge2.xyz, q );
                //if (d > 0.0f && d < hit.x) hit = float4(d, u, v, asfloat( triIdx ));
                if (d > 0.0f && d < hit.x)
                {
                    uint anyHitResult = AnyHitResult_Continue;
                    RunAnyHitShader(payload, intersectionInfo, anyHitResult, Ray, d, triIdx, hitGroupIndex);

                    // Ignore the hit if the any hit shader said we should
                    if (anyHitResult == AnyHitResult_IgnoreHit)
                        continue;

                    hit = float4(d, u, v, asfloat(triIdx));

                    // Accept this hit as the final result if the any hit shader said we should
                    if (anyHitResult == AnyHitResult_AcceptHitAndEndSearch)
                        forceExitSearch = true;
                }
            }
            if (forceExitSearch)
                break;
            if (stackPtr == 0) break;
            node = stack[--stackPtr];
            continue;
        }
        uint left = combinedBVH[altNodeOffset + node * nodeSize + leftOffset];
        uint right = combinedBVH[altNodeOffset + node * nodeSize + rightOffset];
        // child AABB intersection tests
        const float3 t1a = (lmin.xyz - O) * rD, t2a = (lmax.xyz - O) * rD;
        const float3 t1b = (rmin.xyz - O) * rD, t2b = (rmax.xyz - O) * rD;
        const float3 minta = min( t1a, t2a ), maxta = max( t1a, t2a );
        const float3 mintb = min( t1b, t2b ), maxtb = max( t1b, t2b );
        const float tmina = max( max( max( minta.x, minta.y ), minta.z ), 0 );
        const float tminb = max( max( max( mintb.x, mintb.y ), mintb.z ), 0 );
        const float tmaxa = min( min( min( maxta.x, maxta.y ), maxta.z ), hit.x );
        const float tmaxb = min( min( min( maxtb.x, maxtb.y ), maxtb.z ), hit.x );
        float dist1 = tmina > tmaxa ? 1e30f : tmina;
        float dist2 = tminb > tmaxb ? 1e30f : tminb;
        // traverse nearest child first
        if (dist1 > dist2)
        {
            float h = dist1; dist1 = dist2; dist2 = h;
            uint t = left; left = right; right = t;
        }
        if (dist1 == 1e30f)
        {
            if (stackPtr == 0) break; else node = stack[--stackPtr];
        }
        else
        {
            node = left;
            if (dist2 != 1e30f) stack[stackPtr++] = right;
        }
    }

    // Call the miss shader if we didn't hit anything
    if (hit.x == tmax)
    {
        RunMissShader(payload, MissShaderIndex);
    }
    // Else call the closest hit shader
    else
    {
        #if GIGI_INTERSECTIONINFOCUSTOM == 0
        intersectionInfo.barycentrics = hit.yz;
        #endif
        RunClosestHitShader(payload, intersectionInfo, hitGroupIndex, asuint(hit.w));
    }

    // write back intersection result
    //RayVsBVHResult result;
    //result.distance = (hit.x < tmax) ? hit.x : -1.0f;
    //result.barycentrics = hit.yz;
    //result.triangleIndex = asuint(hit.w);
}

#else

void RayVsScene_AABBs(
    in StructuredBuffer<GIGI_RTSCENETYPENAME> scene,
    in uint RayFlags,
    in uint InstanceInclusionMask,
    in uint RayContributionToHitGroupIndex,
    in uint MultiplierForGeometryContributionToHitGroupIndex,
    in uint MissShaderIndex,
    in RayDesc Ray,
    inout Payload payload
)
{
    // Calculate hit group index
    uint hitGroupIndex = RayContributionToHitGroupIndex;

    // Calculate how many primitives there are
    uint2 numVertsStride;
    scene.GetDimensions(numVertsStride.x, numVertsStride.y);
    uint numPrimitives = numVertsStride.x / 6;

    // Storage for the closest hit
    float3 minTAndBary = float3(Ray.TMax, 0.0f, 0.0f);
    uint hitIndex = (uint)-1;
    GIGI_INTERSECTIONINFOOBJECTTYPE intersectionInfo = (GIGI_INTERSECTIONINFOOBJECTTYPE)0;

    // Run the intersection shader for all boxes the ray intersects
    for (uint i = 0; i < numPrimitives; ++i)
    {
        float ax = scene[i*6+0]GIGI_RTSCENEPOSFIELDNAME.x;
        float ay = scene[i*6+1]GIGI_RTSCENEPOSFIELDNAME.x;
        float az = scene[i*6+2]GIGI_RTSCENEPOSFIELDNAME.x;
        float bx = scene[i*6+3]GIGI_RTSCENEPOSFIELDNAME.x;
        float by = scene[i*6+4]GIGI_RTSCENEPOSFIELDNAME.x;
        float bz = scene[i*6+5]GIGI_RTSCENEPOSFIELDNAME.x;
        float3 aabbMin = float3(ax, ay, az);
        float3 aabbMax = float3(bx, by, bz);

        if (RayVsAABB(Ray.Origin, Ray.Direction, aabbMin, aabbMax) >= 0.0f)
            RunIntersectionShader(Ray, minTAndBary.x, i, hitIndex, hitGroupIndex, payload, intersectionInfo);
    }

    // Call the miss shader if we didn't hit anything
    if (hitIndex == (uint)-1)
    {
        RunMissShader(payload, MissShaderIndex);
    }
    // Else call the closest hit shader
    else
    {
        #if GIGI_INTERSECTIONINFOCUSTOM == 0
        intersectionInfo.barycentrics = minTAndBary.yz;
        #endif
        RunClosestHitShader(payload, intersectionInfo, hitGroupIndex, hitIndex);
    }
}

void RayVsScene_Triangles(
    in StructuredBuffer<GIGI_RTSCENETYPENAME> scene,
    in uint RayFlags,
    in uint InstanceInclusionMask,
    in uint RayContributionToHitGroupIndex,
    in uint MultiplierForGeometryContributionToHitGroupIndex,
    in uint MissShaderIndex,
    in RayDesc Ray,
    inout Payload payload
)
{
    // Calculate hit group index
    uint hitGroupIndex = RayContributionToHitGroupIndex;

    // Calculate how many primitives there are
    uint2 numVertsStride;
    scene.GetDimensions(numVertsStride.x, numVertsStride.y);
    uint numPrimitives = numVertsStride.x / 3;

    // Storage for the closest hit
    float3 minTAndBary = float3(Ray.TMax, 0.0f, 0.0f);
    int hitIndex = -1;
    GIGI_INTERSECTIONINFOOBJECTTYPE intersectionInfo = (GIGI_INTERSECTIONINFOOBJECTTYPE)0;

    // Test the ray against every triangle, giving a call to any hit shaders if they exist
    for (uint i = 0; i < numPrimitives; ++i)
    {
        float3 posA = scene[i*3+0]GIGI_RTSCENEPOSFIELDNAME;
        float3 posB = scene[i*3+1]GIGI_RTSCENEPOSFIELDNAME;
        float3 posC = scene[i*3+2]GIGI_RTSCENEPOSFIELDNAME;

        float3 hitTAndBary = RayVsTriangle(Ray.Origin, Ray.Direction, posA, posB, posC);

        // If we have a new candidate for closest hit
        if (hitTAndBary.x >= Ray.TMin && hitTAndBary.x <= minTAndBary.x)
        {
            // Handle AnyHit shaders
            #if GIGI_INTERSECTIONINFOCUSTOM == 0
            intersectionInfo.barycentrics = hitTAndBary.yz;
            #endif
            uint anyHitResult = AnyHitResult_Continue;
            RunAnyHitShader(payload, intersectionInfo, anyHitResult, Ray, hitTAndBary.x, i, hitGroupIndex);

            // Ignore the hit if the any hit shader said we should
            if (anyHitResult == AnyHitResult_IgnoreHit)
                continue;

            minTAndBary = hitTAndBary;
            hitIndex = i;

            // Accept this hit as the final result if the any hit shader said we should
            if (anyHitResult == AnyHitResult_AcceptHitAndEndSearch)
                break;
        }
    }

    // Call the miss shader if we didn't hit anything
    if (hitIndex == -1)
    {
        RunMissShader(payload, MissShaderIndex);
    }
    // Else call the closest hit shader
    else
    {
        #if GIGI_INTERSECTIONINFOCUSTOM == 0
        intersectionInfo.barycentrics = minTAndBary.yz;
        #endif
        RunClosestHitShader(payload, intersectionInfo, hitGroupIndex, hitIndex);
    }
}

#endif

void RayVsScene(
    in StructuredBuffer<GIGI_RTSCENETYPENAME> scene,
    in uint RayFlags,
    in uint InstanceInclusionMask,
    in uint RayContributionToHitGroupIndex,
    in uint MultiplierForGeometryContributionToHitGroupIndex,
    in uint MissShaderIndex,
    in RayDesc Ray,
    inout Payload payload
)
{
    #if GIGI_SCENE_IS_BVH == 1
        RayVsScene_BVH(scene, RayFlags, InstanceInclusionMask, RayContributionToHitGroupIndex, MultiplierForGeometryContributionToHitGroupIndex, MissShaderIndex, Ray, payload);
    #else
        if (SCENE_IS_AABBS)
            RayVsScene_AABBs(scene, RayFlags, InstanceInclusionMask, RayContributionToHitGroupIndex, MultiplierForGeometryContributionToHitGroupIndex, MissShaderIndex, Ray, payload);
        else
            RayVsScene_Triangles(scene, RayFlags, InstanceInclusionMask, RayContributionToHitGroupIndex, MultiplierForGeometryContributionToHitGroupIndex, MissShaderIndex, Ray, payload);
    #endif
}

#endif // #if GIGI_ISRAYGENSHADER
