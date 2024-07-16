// Texture2DRW_RGS technique, shader RWCHS
/*$(ShaderResources)*/

struct Payload
{
	bool hit;
};

/*$(_closesthit:chsmain)*/
{
	payload.hit = true;
}

/*$(_miss:missmain)*/
{
	payload.hit = false;
}
