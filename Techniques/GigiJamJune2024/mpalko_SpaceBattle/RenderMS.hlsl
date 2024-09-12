/*$(ShaderResources)*/

#include "RenderCommon.hlsli"
#include "BattleConstants.hlsli"

float2 LocalToWorld(float2 position, float2 translation, float rotation)
{
    position = mul(position, float2x2(cos(rotation), sin(rotation), -sin(rotation), cos(rotation)));
    position = position + translation;

    return position;
}

#define LASER_VERTS 4
#define LASER_TRIS 2
/*$(_mesh:msLasers)*/(uint dtid : SV_DispatchThreadID, uint gtid : SV_GroupIndex, uint3 gid : SV_GroupID, out vertices VertexData verts[LASER_VERTS], out indices uint3 tris[LASER_TRIS])
{
	SetMeshOutputCounts(LASER_VERTS, LASER_TRIS);

    const Struct_EntityState state = EntityStates[gid.x];

    float beamWidth = state.Size * 0.1;
    float2 beamDirection = normalize(state.LaserEnd - state.LaserStart);
    float2 beam90 = float2(-beamDirection.y, beamDirection.x);

    const float2 mesh_positions[4] = { 
        state.LaserStart + beam90 * beamWidth,
        state.LaserStart - beam90 * beamWidth, 
        state.LaserEnd + beam90 * beamWidth,
        state.LaserEnd - beam90 * beamWidth
    };

    const float3 mesh_barycentric[4] = { float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1), float3(1, 0, 0) };
    const float2 mesh_uv[4] = { float2(0, 0), float2(0, 1), float2(1, 0), float2(1, 1) };
    
    if (gtid < LASER_VERTS)
    {
        verts[gtid].position = float4(mesh_positions[gtid], 0, 1);
        verts[gtid].uv = mesh_uv[gtid];
        verts[gtid].barycentric = mesh_barycentric[gtid];
        verts[gtid].entityidx = gid.x;
        verts[gtid].meshPart = 1;
    }

    const uint3 mesh_triangles[2] = { uint3(0, 1, 2), uint3(1, 2, 3) };
    if (gtid < LASER_TRIS) 
    {
        tris[gtid] = mesh_triangles[gtid];
    }
}

#define SHIP_VERTS 4
#define SHIP_TRIS 2
/*$(_mesh:msShips)*/(uint dtid : SV_DispatchThreadID, uint gtid : SV_GroupIndex, uint3 gid : SV_GroupID, out vertices VertexData verts[SHIP_VERTS], out indices uint3 tris[SHIP_TRIS])
{
	SetMeshOutputCounts(SHIP_VERTS, SHIP_TRIS);

    const Struct_EntityState state = EntityStates[gid.x];

    const float2 mesh_positions[4] = { float2(-0.75, 0), float2(0.5, 0), float2(-1, 0.5), float2(-1, -0.5) };
    const float3 mesh_barycentric[4] = { float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1), float3(0, 0, 1) };
    const float2 mesh_uv[4] = { float2(0, 0), float2(0, 1), float2(1, 0), float2(1, 1) };

    if (!state.IsAlive)
    {
        tris[gtid] = 0;
        return;
    }

    float2 vert_position = LocalToWorld(mesh_positions[gtid] * state.Size, state.Position, state.Rotation);

    if (gtid < SHIP_VERTS)
    {
        verts[gtid].position = float4(vert_position.xy, 0, 1);
        verts[gtid].barycentric = mesh_barycentric[gtid];
        verts[gtid].uv = mesh_uv[gtid];
        verts[gtid].entityidx = gid.x;
        verts[gtid].meshPart = 0;
    }

    const uint3 mesh_triangles[2] = { uint3(0, 1, 2), uint3(0, 1, 3) };
    if (gtid < SHIP_TRIS) 
    {
        tris[gtid] = mesh_triangles[gtid];
    }
}

#define EXPLOSION_VERTS 6
#define EXPLOSION_TRIS 2
/*$(_mesh:msExplosions)*/(uint dtid : SV_DispatchThreadID, uint gtid : SV_GroupIndex, uint3 gid : SV_GroupID, out vertices VertexData verts[EXPLOSION_VERTS], out indices uint3 tris[EXPLOSION_TRIS])
{
    const float iTime = /*$(Variable:iTime)*/;

	SetMeshOutputCounts(EXPLOSION_VERTS, EXPLOSION_TRIS);

    const Struct_EntityState state = EntityStates[gid.x];

    if (state.IsAlive)
    {
        tris[gtid] = 0;
        return;
    }

    const float2 mesh_positions[3] = { float2(0, 1.5), float2(-1.5, -1), float2(1.5, -1), };
    const float3 mesh_barycentric[3] = { float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1) };
    const float2 mesh_uv[3] = { float2(0, 0), float2(0, 1), float2(1, 0) };
    float2 vert_position = mesh_positions[gtid % 3];

    float scale = state.Size * ExplosionInitialScale + (iTime - state.DeathTime) * ExplosionGrowthRate;
    float rotation = iTime * 10 + state.Id;
    if (gtid < 3)
    {
        vert_position = LocalToWorld(vert_position * scale, state.Position, rotation);
    }
    else
    {
        vert_position = LocalToWorld(vert_position * scale * 0.9, state.Position, -rotation);
    }

    if (gtid < EXPLOSION_VERTS)
    {
        verts[gtid].position = float4(vert_position.xy, 0, 1);
        verts[gtid].uv = mesh_positions[gtid];
        verts[gtid].barycentric = mesh_barycentric[gtid % 3];
        verts[gtid].uv = mesh_uv[gtid % 3];
        verts[gtid].entityidx = gid.x;
        verts[gtid].meshPart = 2;
    }

    const uint3 mesh_triangles[EXPLOSION_TRIS] = { uint3(0, 1, 2), uint3(3, 4, 5) };
    if (gtid < EXPLOSION_TRIS) 
    {
        tris[gtid] = mesh_triangles[gtid];
    }
}