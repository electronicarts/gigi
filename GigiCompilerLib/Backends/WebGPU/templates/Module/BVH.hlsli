///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

/*
 BVH code adapted from tinybvh
https://github.com/jbikker/tinybvh/
This file specifically:
https://github.com/jbikker/tinybvh/blob/main/traverse_bvh2.cl

----- Parameter Meanings -----

* altNode     - the list of BVH nodes
* idx         - the lists of triangle IDs referenced by the BVH nodes.
* verts       - the actual vertex data of the triangles.
* combinedBVH - as an alternately to altNode, idx, verts, this buffer contains all 3 buffers in the order: verts, altNode, idx. There are two additional uint32's at the beginning of the file that specify the size in bytes of verts and altNode.
* O           - the ray origin
* D           - the ray direction
* rD          - 1/D.  Calculated as rcp(D).
* tmax        - the maximum length of the ray allowed. Shorter rays can cull nodes from the BVH tree without recursing.

----- Functions -----

BVHCost               - Debug only, gives a score for how costly a ray was (computation and memory reads)
RayVsMesh_BVH         - Uses the BVH to see if a ray hits the mesh.  Returns the closest ray hit t, the barycentrics of where it hit, and the triangle index that was hit. The triangle index can be used in the verts buffer, and the original vertex buffer that made the BVH. It's the same ordering.
IsOccluded_BVH        - Uses the BVH to see if a ray hits the mesh.  Returns true or false. Cheaper than RayVsMesh because it returns true on the first hit found, even if it's not the closest hit.
RayVsMesh_BruteForce  - Same as RayVsMesh_BVH, but tests all triangles, instead of using the BVH acceleration structure.
IsOccluded_BruteForce - same asIsOccluded_BVH, but does not use the BVH.

There are 2 versions of BVHCost, RayVsMesh_BVH and IsOccluded_BVH.

One version takes three separate buffers for the BVH: altNode, idx, verts.  The other version takes a combined buffer that contains all 3 chunks of data (verts then altNode, then idx), and a uint2 header with the sizes of the first two buffers in bytes.

----- Making BVH Files -----

To make BVH data in this format, drag/drop the mesh onto the Gigi viewer, view the vertex buffer, and click "Save as .bvh".
It will save 4 binary files that correspond to the buffers:
* altNode:  .nodes.bvh
* idx:      .triindices.bvh
* verts:    .vertices.bvh
* combined: .combined.bvh

*/

#define STACK_SIZE 32

float rcp(in float v)
{
    if (v > 1e-12f || v < -1e-12f)
        return 1.0f / v;
    else
        return v >= 0.0f ? 1e30f : -1e30f;
}

float3 rcp(in float3 v)
{
    float3 ret;
    ret.x = rcp(v.x);
    ret.y = rcp(v.y);
    ret.z = rcp(v.z);
    return ret;
}

struct Struct_BVHNode
{
    float3 lmin;
    uint left;
    float3 lmax;
    uint right;
    float3 rmin;
    uint triCount;
    float3 rmax;
    uint firstTri;
};

float BVHCost(in StructuredBuffer<Struct_BVHNode> altNode, in StructuredBuffer<uint> idx, in StructuredBuffer<float4> verts, const float3 O, const float3 D, const float3 rD, const float tmax )
{
    // traverse BVH
    float4 hit;
    hit.x = tmax;
    uint node = 0, stack[STACK_SIZE], stackPtr = 0;
    float cost = 0;
    while (true)
    {
        // fetch the node
        cost += 1.2f; // TODO: obtain somehow via tiny_bvh.h?
        const float3 lmin = altNode[node].lmin, lmax = altNode[node].lmax;
        const float3 rmin = altNode[node].rmin, rmax = altNode[node].rmax;
        const uint triCount = altNode[node].triCount;
        if (triCount > 0)
        {
            // process leaf node
            const uint firstTri = altNode[node].firstTri;
            for (uint i = 0; i < triCount; i++)
            {
                cost += 1.0f; // TODO: obtain somehow via tiny_bvh.h?
                const uint triIdx = idx[firstTri + i];
#if 1 //#ifdef ISAPPLE
                // FIX error: initializing 'const __private float4 *__private' with an expression of type '__global float4 *' changes address space of pointer
                const float4 tri[3] = 
                {
                    verts[3 * triIdx],
                    verts[3 * triIdx + 1],
                    verts[3 * triIdx + 2],
                };
#else
                const float4* tri = verts + 3 * triIdx;
#endif
                // triangle intersection - M�ller-Trumbore
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
                if (d > 0.0f && d < hit.x) hit = float4(d, u, v, asfloat( triIdx ));
            }
            if (stackPtr == 0) break;
            node = stack[--stackPtr];
            continue;
        }
        uint left = altNode[node].left, right = altNode[node].right;
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
    // write back intersection result
    return cost;
}

float BVHCost(in StructuredBuffer<uint> combinedBVH, const float3 O, const float3 D, const float3 rD, const float tmax )
{
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
    uint node = 0, stack[STACK_SIZE], stackPtr = 0;
    float cost = 0;
    while (true)
    {
        // fetch the node
        cost += 1.2f; // TODO: obtain somehow via tiny_bvh.h?
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
                cost += 1.0f; // TODO: obtain somehow via tiny_bvh.h?
                const uint triIdx = combinedBVH[idxOffset + firstTri + i];
#if 1 //#ifdef ISAPPLE
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
                // triangle intersection - M�ller-Trumbore
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
                if (d > 0.0f && d < hit.x) hit = float4(d, u, v, asfloat( triIdx ));
            }
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
    // write back intersection result
    return cost;
}

struct RayVsBVHResult
{
    float distance;
    float2 barycentrics;
    uint triangleIndex;
};

RayVsBVHResult RayVsMesh_BVH(in StructuredBuffer<Struct_BVHNode> altNode, in StructuredBuffer<uint> idx, in StructuredBuffer<float4> verts, const float3 O, const float3 D, const float3 rD, const float tmax )
{
    // traverse BVH
    float4 hit;
    hit.x = tmax;
    uint node = 0;
    uint stack[STACK_SIZE];
    uint stackPtr = 0;
    while (true)
    {
        // fetch the node
        const float3 lmin = altNode[node].lmin, lmax = altNode[node].lmax;
        const float3 rmin = altNode[node].rmin, rmax = altNode[node].rmax;
        const uint triCount = altNode[node].triCount;
        if (triCount > 0)
        {
            // process leaf node
            const uint firstTri = altNode[node].firstTri;
            for (uint i = 0; i < triCount; i++)
            {
                const uint triIdx = idx[firstTri + i];
#if 1//#ifdef ISAPPLE
                // FIX error: initializing 'const __private float4 *__private' with an expression of type '__global float4 *' changes address space of pointer
                const float4 tri[3] = 
                {
                    verts[3 * triIdx],
                    verts[3 * triIdx + 1],
                    verts[3 * triIdx + 2],
                };
#else
                const float4* tri = verts + 3 * triIdx;
#endif
                // triangle intersection - M�ller-Trumbore
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
                if (d > 0.0f && d < hit.x) hit = float4(d, u, v, asfloat( triIdx ));
            }
            if (stackPtr == 0) break;
            node = stack[--stackPtr];
            continue;
        }
        uint left = altNode[node].left, right = altNode[node].right;
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
    // write back intersection result
    RayVsBVHResult result;
    result.distance = (hit.x < tmax) ? hit.x : -1.0f;
    result.barycentrics = hit.yz;
    result.triangleIndex = asuint(hit.w);
    return result;
}

RayVsBVHResult RayVsMesh_BVH(in StructuredBuffer<uint> combinedBVH, const float3 O, const float3 D, const float3 rD, const float tmax )
{
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
    while (true)
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
                // triangle intersection - M�ller-Trumbore
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
                if (d > 0.0f && d < hit.x) hit = float4(d, u, v, asfloat( triIdx ));
            }
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
    // write back intersection result
    RayVsBVHResult result;
    result.distance = (hit.x < tmax) ? hit.x : -1.0f;
    result.barycentrics = hit.yz;
    result.triangleIndex = asuint(hit.w);
    return result;
}

bool IsOccluded_BVH(in StructuredBuffer<Struct_BVHNode> altNode, in StructuredBuffer<uint> idx, in StructuredBuffer<float4> verts, const float3 O, const float3 D, const float3 rD, const float tmax )
{
    // traverse BVH
    uint node = 0;
    uint stack[STACK_SIZE];
    uint stackPtr = 0;

    while (true)
    {
        // fetch the node
        const float3 lmin = altNode[node].lmin, lmax = altNode[node].lmax;
        const float3 rmin = altNode[node].rmin, rmax = altNode[node].rmax;
        const uint triCount = altNode[node].triCount;
        if (triCount > 0)
        {
            // process leaf node
            const uint firstTri = altNode[node].firstTri;
            for (uint i = 0; i < triCount; i++)
            {
                const uint triIdx = idx[firstTri + i];
#if 1//#ifdef ISAPPLE
                // FIX error: initializing 'const __private float4 *__private' with an expression of type '__global float4 *' changes address space of pointer
                const float4 tri[3] = 
                {
                    verts[3 * triIdx],
                    verts[3 * triIdx + 1],
                    verts[3 * triIdx + 2],
                };
#else
                const float4* tri = verts + 3 * triIdx;
#endif
                // triangle intersection - M�ller-Trumbore
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
                if (d > 0.0f && d < tmax) return true;
            }
            if (stackPtr == 0) break;
            node = stack[--stackPtr];
            continue;
        }
        uint left = altNode[node].left, right = altNode[node].right;
        // child AABB intersection tests
        const float3 t1a = (lmin.xyz - O) * rD, t2a = (lmax.xyz - O) * rD;
        const float3 t1b = (rmin.xyz - O) * rD, t2b = (rmax.xyz - O) * rD;
        const float3 minta = min( t1a, t2a ), maxta = max( t1a, t2a );
        const float3 mintb = min( t1b, t2b ), maxtb = max( t1b, t2b );
        const float tmina = max( max( max( minta.x, minta.y ), minta.z ), 0 );
        const float tminb = max( max( max( mintb.x, mintb.y ), mintb.z ), 0 );
        const float tmaxa = min( min( min( maxta.x, maxta.y ), maxta.z ), tmax );
        const float tmaxb = min( min( min( maxtb.x, maxtb.y ), maxtb.z ), tmax );
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
    // no hit found
    return false;
}

bool IsOccluded_BVH(in StructuredBuffer<uint> combinedBVH, const float3 O, const float3 D, const float3 rD, const float tmax )
{
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
    uint node = 0;
    uint stack[STACK_SIZE];
    uint stackPtr = 0;

    while (true)
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
                // triangle intersection - M�ller-Trumbore
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
                if (d > 0.0f && d < tmax) return true;
            }
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
        const float tmaxa = min( min( min( maxta.x, maxta.y ), maxta.z ), tmax );
        const float tmaxb = min( min( min( maxtb.x, maxtb.y ), maxtb.z ), tmax );
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
    // no hit found
    return false;
}

RayVsBVHResult RayVsMesh_BruteForce(in StructuredBuffer<float4> verts, const float3 O, const float3 D, in float tmax)
{
    RayVsBVHResult result;
    result.distance = -1.0f;
    result.barycentrics = float2(0.0f, 0.0f);
    result.triangleIndex = 0;

    uint count, stride;
    verts.GetDimensions(count, stride);

    for (uint vertIndex = 0; vertIndex + 3 < count; vertIndex += 3)
    {
        const float4 tri[3] = 
        {
            verts[vertIndex + 0],
            verts[vertIndex + 1],
            verts[vertIndex + 2],
        };

        // triangle intersection - M�ller-Trumbore
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
        if (d > 0.0f && d < tmax)
        {
            tmax = d;
            result.distance = d;
            result.barycentrics = float2(u,v);
            result.triangleIndex = vertIndex/3;
        }
    }

    return result;
}

bool IsOccluded_BruteForce(in StructuredBuffer<float4> verts, const float3 O, const float3 D, const float tmax)
{
    uint count, stride;
    verts.GetDimensions(count, stride);

    for (uint vertIndex = 0; vertIndex + 3 < count; vertIndex += 3)
    {
        const float4 tri[3] = 
        {
            verts[vertIndex + 0],
            verts[vertIndex + 1],
            verts[vertIndex + 2],
        };

        // triangle intersection - M�ller-Trumbore
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
        if (d > 0.0f && d < tmax) return true;
    }

    return false;
}
