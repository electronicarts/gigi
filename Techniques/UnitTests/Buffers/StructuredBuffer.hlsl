// BufferFormats technique, shader csmain
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	buff[0].TheInt = 1;
	buff[0].TheFloat = 1.3f;
	buff[0].TheEnum = Trinary::False;
	buff[0].TheBool = 0;
	buff[0].TheUINT = 31337;
	buff[0].TheFloat2 = float2(0.3f, 3.0f);

	buff[1].TheInt = /*$(Variable:frameIndex)*/;
	buff[1].TheFloat = /*$(Variable:frameDeltaTime)*/;
	buff[1].TheEnum = Trinary::Maybe;
	buff[1].TheBool = 2;
	buff[1].TheUINT = 255;
	buff[1].TheFloat2 = float2(99.0f, 1.0f / 99.0f);	
}

/*
Shader Resources:
	Buffer buff (as UAV)
*/
