// Unnamed technique, shader WG1
/*$(ShaderResources)*/

struct PixelWorkRecord
{
    uint2 dispatchSize : SV_DispatchGrid;
};

// Entry point: Get size of texture and do a dispatch to cover each pixel
[Shader("node")]
[NodeIsProgramEntry]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void main(
    [MaxRecords(1)]
    [NodeID("PixelWriter")]
    NodeOutput<PixelWorkRecord> childNode)
{
    GroupNodeOutputRecords<PixelWorkRecord> outRec = childNode.GetGroupNodeOutputRecords(1);

    uint2 dimensions;
    Output.GetDimensions(dimensions.x, dimensions.y);
    outRec.Get().dispatchSize = (dimensions + uint2(7, 7)) / uint2(8, 8);

    outRec.OutputComplete();
}

// Per pixel "Compute Shader"
[Shader("node")]
[NodeID("PixelWriter")]
[NodeLaunch("broadcasting")]
[NumThreads(8, 8, 1)]
[NodeMaxDispatchGrid(512, 512, 1)]
void PixelWriter(
    DispatchNodeInputRecord<PixelWorkRecord> input,
    uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint2 px = dispatchThreadID.xy;

    uint2 dimensions;
    Output.GetDimensions(dimensions.x, dimensions.y);

    Output[px] = float4((float2(px) + 0.5f) / float2(dimensions), 0.0f, 1.0f);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
