/*$(ShaderResources)*/

/*$(_compute:Draw)*/(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x == 0 && DTid.y == 0)
    {
        uint2 canvasSize = uint2/*$(Variable:c_drawingCanvasSize)*/;
        DrawExtents[0].MinX = canvasSize.x;
        DrawExtents[0].MinY = canvasSize.y;
        DrawExtents[0].MaxX = 0;
        DrawExtents[0].MaxY = 0;
        DrawExtents[0].PixelCount = 0;
        DrawExtents[0].PixelLocationSum = uint2(0, 0);
    }

    if (/*$(Variable:Clear)*/ != 0 || /*$(Variable:iFrame)*/ == 0)
    {
        Canvas[DTid.xy] = 0.0f;
        return;
    }

    // Don't allow drawing when the user isn't looking at the drawing canvas
    if (/*$(Variable:UseImportedImage)*/)
        return;

    float4 mouseLastFrame = /*$(Variable:MouseStateLastFrame)*/;
    float4 mouse = /*$(Variable:MouseState)*/;

    // This is to compensate for us drawing it at 30,30 in the presentation pass
    mouseLastFrame.xy -= 30.0f;
    mouse.xy -= 30.0f;

    // If the mouse moved, and was pressed last frame and this frame, draw a line
    // between the mouse positions
    if (((mouseLastFrame.z != 0.0f && mouse.z != 0.0f) || (mouseLastFrame.w != 0.0f && mouse.w != 0.0f)) && (mouseLastFrame.x != mouse.x || mouseLastFrame.y != mouse.y))
    {
        // Get distance from pixel to line
        // Color the pixel in, if it's close enough

        // Get a normalized vector from A to B
        float2 A = mouseLastFrame.xy;
        float2 B = mouse.xy;
        float2 AB = normalize(B - A);

        // Project the pixel onto it
        float t = dot(AB, float2(DTid.xy) - A);

        // Get the closest point on the line to the pixel
        t = clamp(t, 0.0f, length(B-A));
        float2 closestPoint = A + AB * t;

        // color the pixel if it's close enough.
        // Draw a 1 if the left mouse is down. Draw a 0 if the right mouse is down.
        float2 closestPointToPixel = float2(DTid.xy) - closestPoint;
        if (dot(closestPointToPixel, closestPointToPixel) < (/*$(Variable:PenSize)*/ * /*$(Variable:PenSize)*/))
            Canvas[DTid.xy] = mouse.z;
    }
}
