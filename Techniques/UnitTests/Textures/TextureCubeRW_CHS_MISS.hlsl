// Texture2DArrayRW_CS technique, shader RWCHS
/*$(ShaderResources)*/

struct Payload
{
	bool hit;
};

/*$(_miss:missmain)*/
{
	payload.hit = false;
}

/*$(_closesthit:chsmain)*/
{
	payload.hit = true;
}
