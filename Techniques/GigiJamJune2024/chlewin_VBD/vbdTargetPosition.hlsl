// Unnamed technique, shader vbdIntegrate
/*$(ShaderResources)*/

#include "simpleRenderCommon.hlsl"

/*$(_compute:csMain)*/(uint idx : SV_DispatchThreadID)
{
	float dt = 1.0 / 60.0;

	float4 startPosition = vertexFrameStartPosition[idx];

    // if(idx == 752)
    // {
	// 	vertexTargetPosition[idx] = startPosition;
	// 	vertexCandidatePositions[idx] = startPosition;
    //     return;
    // }

	float4 prevVelocity = vertexPreviousVelocities[idx];
	float4 velocity = vertexVelocity[idx];
	vertexPreviousVelocities[idx] = velocity;
	float4 aExt = float4(0,-1,0,0);

	float4 n = float4(
		vertexNormals[idx * 3],
		vertexNormals[idx * 3 + 1],
		vertexNormals[idx * 3 + 2],
		0
	) / FixedPointMultiplier;

	if(length(n) != 0)
	{
	aExt -= /*$(Variable:pressureStrength)*/*normalize(n);
	}


	float aExtLen = length(aExt);
	
	float4 at = (velocity - prevVelocity)/dt;

	float atExt = dot(at, normalize(aExt));


	float4 aTilde = (float4)0;
	if(atExt > aExtLen)
	{
		aTilde = aExt;
	}
	else if(atExt < 0)
	{
		aTilde = 0;
	}
	else
	{
		aTilde = aExt * (atExt / aExtLen);
	}

	float4 targetPosition = startPosition + velocity*dt + aExt*dt*dt;

	vertexTargetPosition[idx] = targetPosition;

	float4 candidatePosition = startPosition + velocity*dt + aTilde*dt*dt;

	vertexCandidatePositions[idx] = candidatePosition;
}
