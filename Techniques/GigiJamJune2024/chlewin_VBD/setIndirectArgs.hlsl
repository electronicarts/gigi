// Unnamed technique, shader setIndirectArgs
/*$(ShaderResources)*/

uint divUp(uint threadCount, uint divisor)
{
    return (threadCount + divisor - 1) / divisor;
}



/*$(_compute:csMain)*/(uint3 DTid : SV_DispatchThreadID)
{

uint i = DTid.x;
uint colorCount = vertexColorData[0];

if(i >= colorCount)
{
	return;
}

uint colorVertexCount = vertexColorData[2 + i] - vertexColorData[1 + i];

uint dispatchCount = divUp(colorVertexCount, 64);

#define FillIndirectArgs(argsBuf) argsBuf[0] = dispatchCount; argsBuf[1] = 1; argsBuf[2] = 1

if(i == 0)
{
	FillIndirectArgs(indirectArgs0);
}
else if(i == 1)
{
	FillIndirectArgs(indirectArgs1);
}
else if(i == 2)
{
	FillIndirectArgs(indirectArgs2);
}
else if(i == 3)
{
	FillIndirectArgs(indirectArgs3);
}
else if(i == 4)
{
	FillIndirectArgs(indirectArgs4);
}
else if(i == 5)
{
	FillIndirectArgs(indirectArgs5);
}
else if(i == 6)
{
	FillIndirectArgs(indirectArgs6);
}
else if(i == 7)
{
	FillIndirectArgs(indirectArgs7);
}
else if(i == 8)
{
	FillIndirectArgs(indirectArgs8);
}
else if(i == 9)
{
	FillIndirectArgs(indirectArgs9);
}


}

/*
Shader Resources:
	Count  (as Count)
*/
