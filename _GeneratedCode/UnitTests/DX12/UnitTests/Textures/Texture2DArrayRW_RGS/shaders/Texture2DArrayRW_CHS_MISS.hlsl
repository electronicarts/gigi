// Texture2DArrayRW_CS technique, shader RWCHS



#line 2


struct Payload
{
	bool hit;
};

[shader("miss")]
#line 9
void missmain(inout Payload payload : SV_RayPayload)
{
	payload.hit = false;
}

[shader("closesthit")]
#line 14
void chsmain(inout Payload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes intersection : SV_IntersectionAttributes)
{
	payload.hit = true;
}
