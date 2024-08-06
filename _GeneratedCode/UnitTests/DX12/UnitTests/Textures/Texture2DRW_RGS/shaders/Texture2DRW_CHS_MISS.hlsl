// Texture2DRW_RGS technique, shader RWCHS



#line 2


struct Payload
{
	bool hit;
};

[shader("closesthit")]
#line 9
void chsmain(inout Payload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes intersection : SV_IntersectionAttributes)
{
	payload.hit = true;
}

[shader("miss")]
#line 14
void missmain(inout Payload payload : SV_RayPayload)
{
	payload.hit = false;
}
