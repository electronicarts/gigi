/*$(ShaderResources)*/

static const uint2 c_drawingCanvasSize = uint2/*$(Variable:c_drawingCanvasSize)*/;
static const uint2 c_NNInputImageSize = uint2/*$(Variable:c_NNInputImageSize)*/;

void Shrink(uint2 pixelPos)
{
    uint2 sourcePixelMin = float2(pixelPos) * float2(c_drawingCanvasSize) / float2(c_NNInputImageSize);
    uint2 sourcePixelMax = float2(pixelPos + uint2(1, 1)) * float2(c_drawingCanvasSize) / float2(c_NNInputImageSize);

    float count = (sourcePixelMax.x - sourcePixelMin.x) * (sourcePixelMax.y - sourcePixelMin.y);
    float ooCount = 1.0f / float(count);
    float output = 0.0f;
    for (uint iy = sourcePixelMin.y; iy < sourcePixelMax.y; ++iy)
        for (uint ix = sourcePixelMin.x; ix < sourcePixelMax.x; ++ix)
            output += Canvas[uint2(ix, iy)] / count;

    NNInput[pixelPos] = output;
}

void ShrinkNormalize(uint2 pixelPos)
{
    // Get the drawn extents
    uint2 drawMin = uint2(DrawExtents[0].MinX, DrawExtents[0].MinY);
    uint2 drawMax = uint2(DrawExtents[0].MaxX, DrawExtents[0].MaxY);

    // This happens when the canvas is empty
    if (drawMax.x < drawMin.x || drawMax.y < drawMin.y)
    {
        NNInput[pixelPos] = 0.0f;
        return;
    }

    // Preserve aspect ratio by shrinking either x or y in the (20,20) target, and offsetting to center
    uint2 drawSize = drawMax - drawMin;
    uint2 normalizedImageSize = uint2(20, 20);
    int2 offset = int2(4, 4);
    float drawnAspectRatio = float(drawSize.x) / float(drawSize.y);
    if (drawSize.x > drawSize.y)
    {
        normalizedImageSize.y = uint(float(normalizedImageSize.y) / drawnAspectRatio);
        offset.y += (20 - normalizedImageSize.y) / 2;
    }
    else
    {
        normalizedImageSize.x = uint(float(normalizedImageSize.x) * drawnAspectRatio);
        offset.x += (20 - normalizedImageSize.x) / 2;
    }

    // Offset by center of mass
    //int2 drawCenterOfMassOffset = int2(DrawExtents[0].PixelLocationSum) / int(DrawExtents[0].PixelCount) - int2(c_drawingCanvasSize) / 2;
    //drawCenterOfMassOffset = drawCenterOfMassOffset / 2;
    int2 drawCenterOfMassOffset = int2(0, 0);

    uint2 sourcePixelMin = float2(int2(drawMin)-drawCenterOfMassOffset)+float2(int2(pixelPos)-offset) * float2(drawSize) / float2(normalizedImageSize);
    uint2 sourcePixelMax = float2(int2(drawMin)-drawCenterOfMassOffset)+float2(int2(pixelPos)-offset + int2(1, 1)) * float2(drawSize) / float2(normalizedImageSize);

    float count = (sourcePixelMax.x - sourcePixelMin.x) * (sourcePixelMax.y - sourcePixelMin.y);
    float ooCount = 1.0f / float(count);
    float output = 0.0f;
    for (uint iy = sourcePixelMin.y; iy < sourcePixelMax.y; ++iy)
        for (uint ix = sourcePixelMin.x; ix < sourcePixelMax.x; ++ix)
            output += Canvas[uint2(ix, iy)] / count;

    NNInput[pixelPos] = output;
}

/*$(_compute:Shrink)*/(uint3 DTid : SV_DispatchThreadID)
{
    if (/*$(Variable:UseImportedImage)*/)
        NNInput[DTid.xy] = ImportedImage[DTid.xy];
    else if (/*$(Variable:NormalizeDrawing)*/)
        ShrinkNormalize(DTid.xy);
    else
        Shrink(DTid.xy);
}
