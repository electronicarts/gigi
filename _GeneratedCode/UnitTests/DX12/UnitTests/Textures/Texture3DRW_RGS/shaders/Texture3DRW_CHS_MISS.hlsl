// Texture3DRW_RGS technique, shader RWCHS




struct Payload
{
    bool hit;
};

[shader("miss")]
void missmain(inout Payload payload : SV_RayPayload)
{
	payload.hit = false;
}

[shader("closesthit")]
void chsmain(inout Payload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes intersection : SV_IntersectionAttributes)
{
	payload.hit = true;
}
