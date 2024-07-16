// Texture2DRW_RGS technique, shader RWCHS




struct Payload
{
	bool hit;
};

[shader("closesthit")]
void chsmain(inout Payload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes intersection : SV_IntersectionAttributes)
{
	payload.hit = true;
}

[shader("miss")]
void missmain(inout Payload payload : SV_RayPayload)
{
	payload.hit = false;
}
