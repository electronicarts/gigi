
// from https://learnopengl.com/PBR/IBL/Diffuse-irradiance
float2 SampleEquirectangularMap(float3 v)
{
    const float2 invAtan = float2(0.1591f, 0.3183f);
    float2 uv = float2(atan2(v.z, v.x), asin(-v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float3 CubeUVFaceToDir(float2 uv, uint face)
{
    // Switch on the face
    float3 dir = float3(1.0f, 0.0f, 0.0f);
    switch (face)
    {
        // Right (X+)
        case 0:
        {
            dir = float3(1.0f, uv.y, -uv.x);
            break;
        }
        // Left (X-)
        case 1:
        {
            dir = float3(-1.0f, uv.y, uv.x);
            break;
        }
        // Top (Y+)
        case 2:
        {
            dir = float3(uv.x, 1.0f, -uv.y);
            break;
        }
        // Bottom (Y-)
        case 3:
        {
            dir = float3(uv.x, -1.0f, uv.y);
            break;
        }
        // Front (Z+)
        case 4:
        {
            dir = float3(uv.x, uv.y, 1.0f);
            break;
        }
        // Back (Z-)
        case 5:
        {
            dir = float3(-uv.x, uv.y, -1.0f);
            break;
        }
    }

    return normalize(dir);
}
