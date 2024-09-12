// Unnamed technique, shader vbd
/*$(ShaderResources)*/

float3x3 outerProduct(float3 x, float3 y)
{
    return float3x3(x[0]*y, x[1]*y, x[2]*y);
}

float det(float3x3 m)
{
	float n11 = m[0][0], n12 = m[0][1], n13 = m[0][2];
	float n21 = m[1][0], n22 = m[1][1], n23 = m[1][2];
	float n31 = m[2][0], n32 = m[2][1], n33 = m[2][2];

	float t11 = n22 * n33 - n23 * n32;
	float t12 = n23 * n31 - n21 * n33;
	float t13 = n21 * n32 - n22 * n31;

	float det = n11 * t11 + n12 * t12 + n13 * t13;
	return det;
}

// From Frostbite
float3x3 inverse(float3x3 m)
{
	float n11 = m[0][0], n12 = m[0][1], n13 = m[0][2];
	float n21 = m[1][0], n22 = m[1][1], n23 = m[1][2];
	float n31 = m[2][0], n32 = m[2][1], n33 = m[2][2];

	float t11 = n22 * n33 - n23 * n32;
	float t12 = n23 * n31 - n21 * n33;
	float t13 = n21 * n32 - n22 * n31;

	float det = n11 * t11 + n12 * t12 + n13 * t13;
	float idet = 1.0f / det;

	float3x3 ret;

	ret[0][0] = t11 * idet;
	ret[0][1] = (n13 * n32 - n12 * n33) * idet;
	ret[0][2] = (n12 * n23 - n13 * n22) * idet;
	ret[1][0] = t12 * idet;
	ret[1][1] = (n11 * n33 - n13 * n31) * idet;
	ret[1][2] = (n13 * n21 - n11 * n23) * idet;
	ret[2][0] = t13 * idet;
	ret[2][1] = (n12 * n31 - n11 * n32) * idet;
	ret[2][2] = (n11 * n22 - n12 * n21) * idet;

	return ret;
}


static const float3x3 Identity = float3x3(1,0,0,0,1,0,0,0,1);

struct ElasticityContext
{
    uint vertexEdgesBegin;
    uint vertexEdgesEnd;
    uint vertexCount;
    uint vertexIndex;
    float edgeLengthMultiplier;
    float edgeStiffness;
};

void calculateForceJacobian(float3 vertexPosition, ElasticityContext context, out float3 force, out float3x3 jacobian)
{
    force = (float3)0;
    jacobian = (float3x3)0;

    // Accumulate forces and jacobian of edge springs
    for (uint vertexEdgeIdx = context.vertexEdgesBegin; vertexEdgeIdx < context.vertexEdgesEnd; ++vertexEdgeIdx)
    {
        uint edge = vertexEdgeData[2 + context.vertexCount + vertexEdgeIdx];

        uint i0 = edgeIndices[edge].x;
        uint i1 = edgeIndices[edge].y;

        float l0 = context.edgeLengthMultiplier*edgeLengths[edge];

        uint iOther = (i0 == context.vertexIndex ? i1 : i0);

        float3 pOther = vertexPositions[iOther].xyz;

        float3 o = vertexPosition - pOther;

        float l = length(o);

        force += context.edgeStiffness*o*(l0/l - 1.0);

        // == d(force) / d(vertexPosition)
        jacobian += context.edgeStiffness*(Identity*(l0/l - 1) - (l0 / (l*l))*outerProduct(o,o));
    }

    // Include dynamic energy
    float h = 1.0 / 60.0;

    float3 targetPosition = vertexTargetPositions[context.vertexIndex].xyz;
    float mass = 1.0;

    float weighting = mass/(h*h);

    force += (targetPosition-vertexPosition)*weighting;
    jacobian -= Identity*weighting;
}

/*$(_compute:csMain)*/(uint idx : SV_DispatchThreadID)
{
	uint colorCount = vertexColorData[0];
	uint color = VERTEX_COLOR;
	uint colorVertexBegin = vertexColorData[1 + color];
	uint colorVertexEnd = vertexColorData[2 + color];
	uint colorVertexCount = colorVertexEnd - colorVertexBegin;

	if (idx >= colorVertexCount)
	{
		return;
	}

	uint vertexIndex = vertexColorData[2 + colorCount + colorVertexBegin + idx];

	uint vertexCount = vertexEdgeData[0];
	uint vertexEdgesBegin = vertexEdgeData[1 + vertexIndex];
	uint vertexEdgesEnd = vertexEdgeData[2 + vertexIndex];

    ElasticityContext context = {vertexEdgesBegin, vertexEdgesEnd, vertexCount, vertexIndex, 1, 2000};

	float3 vertexPosition = vertexPositions[vertexIndex].xyz;

    float3 originalVertexPosition = vertexPosition;

    [loop]
    for(uint newtonIteration = 0; newtonIteration < 5; ++newtonIteration)
    {
        float3 force = (float3)0;
        float3x3 jacobian = (float3x3)0;
        calculateForceJacobian(vertexPosition, context, force, jacobian);

        // If force is near zero then we are done for this iteration
        // This means we have solved this vertex to near machine precision
        // and forces are balanced
        if(length(force) <= 1e-6)
            break;

        // Rank deficient so we should skip it
        if(abs(det(jacobian)) <= 1e-4)
            break;

        float kappa = 1;
        float3 update = -mul(inverse(jacobian), force);
        
        #if 1 // Backtracking Line search
        float originalForceLength = dot(force, force);
        float kappaLimit = 0.01;
        while(kappa > kappaLimit)
        {
            float3 lineSearchPosition = vertexPosition + update*kappa; 
            float3 lineSearchForce = float3(0,0,0);
            float3x3 lineSearchJacobian;
            calculateForceJacobian(lineSearchPosition, context, lineSearchForce, lineSearchJacobian);

            float objective = dot(lineSearchForce, lineSearchForce);

            if(objective <= originalForceLength)
                break;
            kappa *= 0.5;
        }
        // If we decreased kappa all the way to the given limit without finding an adequate decrease of the objective
        // then we assume no motion of this vertex is possible. Progress can likely still be made by other vertices.
        if(kappa <= kappaLimit)
        {
            kappa = 0;
        }
        #endif

        vertexPosition += kappa*update;
        //vertexPosition += force* 0.00001;

    }

    if(vertexIndex == 371 || vertexIndex == 1491)
    {
        vertexPosition = originalVertexPosition;
    }
    //vertexPosition = clamp(vertexPosition, -2, 2);

	vertexPositions[vertexIndex] = float4(vertexPosition, 0);
}
