/*$(ShaderResources)*/

/*$(_compute:Presentation)*/(uint3 DTid : SV_DispatchThreadID)
{
    const int c_borderSize = 3;

    const int2 c_drawPanelPos = int2(30, 30);
    const int2 c_drawPanelSize = int2/*$(Variable:c_drawingCanvasSize)*/;

    const int2 c_inputPanelPos = int2(c_drawPanelPos.x + c_drawPanelSize.x + c_borderSize * 2 + 10, 30);
    const int2 c_inputPanelSize = int2/*$(Variable:c_NNInputImageSize)*/;

    const int2 c_hiddenPanelPos = int2(c_inputPanelPos.x + c_inputPanelSize.x + c_borderSize * 2 + 10, 30);
    const int2 c_hiddenPanelSize = int2(28, 840 + c_borderSize * 29);

    const int2 c_outputPanelPos = int2(c_hiddenPanelPos.x + c_hiddenPanelSize.x + c_borderSize * 2 + 10, 30);
    const int2 c_outputPanelSize = int2(28, 280 + c_borderSize * 9);

    const int2 c_outputLabelsPos = int2(c_outputPanelPos.x + c_outputPanelSize.x + c_borderSize * 2, 30);
    const int2 c_outputLabelsSize = int2(28, 280 + c_borderSize * 9);

    const int2 c_instructionsPos = int2(30, c_drawPanelPos.y + c_drawPanelSize.y + c_borderSize * 2 + 30);
    const int2 c_instructionsSize = int2(290, 85);

    const float4 c_borderColor = float4(0.8f, 0.8f, 0.0f, 1.0f);
    const float4 c_backgroundColor = float4(0.2f, 0.2f, 0.2f, 1.0f);

    const float4 c_mouseCursorColor = float4(1.0f, 1.0f, 1.0f, 0.15f);

    // Draw the draw panel
    {
        int2 relPos = int2(DTid.xy) - c_drawPanelPos;
        if (relPos.x >= 0 && relPos.y >= 0 && relPos.x < c_drawPanelSize.x && relPos.y < c_drawPanelSize.y)
        {
            float4 mouse = /*$(Variable:MouseState)*/;
            float3 color;

            if (!/*$(Variable:UseImportedImage)*/)
            {
                color = float3(0.0f, DrawCanvas[relPos], 0.0f);
            }
            else
            {
                const uint2 c_drawingCanvasSize = uint2/*$(Variable:c_drawingCanvasSize)*/;
                const uint2 c_NNInputImageSize = uint2/*$(Variable:c_NNInputImageSize)*/;
                int2 srcPos = float2(relPos) * float2(c_NNInputImageSize) / float2(c_drawingCanvasSize);
                float value = NNInput[srcPos];

                color = float3(value, value, 0.0f);
            }

            if (length(mouse.xy - float2(DTid.xy)) < /*$(Variable:PenSize)*/)
                color = lerp(color, c_mouseCursorColor.rgb, c_mouseCursorColor.aaa);

            PresentationCanvas[DTid.xy] = float4(color, 1.0f);
            return;
        }

        if (relPos.x >= -c_borderSize && relPos.y >= -c_borderSize && relPos.x < c_drawPanelSize.x + c_borderSize && relPos.y < c_drawPanelSize.y + c_borderSize)
        {
            PresentationCanvas[DTid.xy] = c_borderColor;
            return;
        }
    }

    // Draw the input layer activations (the NN input)
    {
        int2 relPos = int2(DTid.xy) - c_inputPanelPos;
        if (relPos.x >= 0 && relPos.y >= 0 && relPos.x < c_inputPanelSize.x && relPos.y < c_inputPanelSize.y)
        {
            float value = NNInput[relPos];
            PresentationCanvas[DTid.xy] = float4(value.xxx, 1.0f);
            return;
        }

        if (relPos.x >= -c_borderSize && relPos.y >= -c_borderSize && relPos.x < c_inputPanelSize.x + c_borderSize && relPos.y < c_inputPanelSize.y + c_borderSize)
        {
            PresentationCanvas[DTid.xy] = c_borderColor;
            return;
        }
    }

    // Draw the hidden layer activations
    {
        int2 relPos = int2(DTid.xy) - c_hiddenPanelPos;

        if (relPos.x >= 0 && relPos.y >= 0 && relPos.x < c_hiddenPanelSize.x && relPos.y < c_hiddenPanelSize.y)
        {
            if ((relPos.y % 31) >= 28)
            {
                PresentationCanvas[DTid.xy] = c_borderColor;
                return;
            }

            relPos.y /= 31;
            float value = HiddenLayerActivations[relPos.y];
            PresentationCanvas[DTid.xy] = float4(value.xxx, 1.0f);
            return;
        }

        if (relPos.x >= -c_borderSize && relPos.y >= -c_borderSize && relPos.x < c_hiddenPanelSize.x + c_borderSize && relPos.y < c_hiddenPanelSize.y + c_borderSize)
        {
            PresentationCanvas[DTid.xy] = c_borderColor;
            return;
        }
    }

    // Draw the output layer activations
    {
        int2 relPos = int2(DTid.xy) - c_outputPanelPos;

        if (relPos.x >= 0 && relPos.y >= 0 && relPos.x < c_outputPanelSize.x && relPos.y < c_outputPanelSize.y)
        {
            if ((relPos.y % 31) >= 28)
            {
                PresentationCanvas[DTid.xy] = c_borderColor;
                return;
            }

            relPos.y /= 31;
            float value = OutputLayerActivations[relPos.y];
            PresentationCanvas[DTid.xy] = float4(value.xxx, 1.0f);
            return;
        }

        if (relPos.x >= -c_borderSize && relPos.y >= -c_borderSize && relPos.x < c_outputPanelSize.x + c_borderSize && relPos.y < c_outputPanelSize.y + c_borderSize)
        {
            PresentationCanvas[DTid.xy] = c_borderColor;
            return;
        }
    }

    // Draw the output layer labels
    {
        int2 relPos = int2(DTid.xy) - c_outputLabelsPos;

        if (relPos.x >= 0 && relPos.y >= 0 && relPos.x < c_outputLabelsSize.x && relPos.y < c_outputLabelsSize.y)
        {
            if ((relPos.y % 31) < 28)
            {
                int index = relPos.y / 31;
                relPos.y = relPos.y % 31;

                float alpha = 0.0f;
                switch(index)
                {
                    case 0: alpha = /*$(Image:0.png:RGBA8_Unorm_sRGB:float:true)*/[relPos].r; break;
                    case 1: alpha = /*$(Image:1.png:RGBA8_Unorm_sRGB:float:true)*/[relPos].r; break;
                    case 2: alpha = /*$(Image:2.png:RGBA8_Unorm_sRGB:float:true)*/[relPos].r; break;
                    case 3: alpha = /*$(Image:3.png:RGBA8_Unorm_sRGB:float:true)*/[relPos].r; break;
                    case 4: alpha = /*$(Image:4.png:RGBA8_Unorm_sRGB:float:true)*/[relPos].r; break;
                    case 5: alpha = /*$(Image:5.png:RGBA8_Unorm_sRGB:float:true)*/[relPos].r; break;
                    case 6: alpha = /*$(Image:6.png:RGBA8_Unorm_sRGB:float:true)*/[relPos].r; break;
                    case 7: alpha = /*$(Image:7.png:RGBA8_Unorm_sRGB:float:true)*/[relPos].r; break;
                    case 8: alpha = /*$(Image:8.png:RGBA8_Unorm_sRGB:float:true)*/[relPos].r; break;
                    case 9: alpha = /*$(Image:9.png:RGBA8_Unorm_sRGB:float:true)*/[relPos].r; break;
                }

                if (alpha > 0.0f)
                {
                    float3 pixelColor = lerp(float3(0.4f, 0.0f, 0.0f), float3(1.0f, 1.0f, 0.0f), OutputLayerActivations[index]);
                    pixelColor = lerp(c_backgroundColor.rgb, pixelColor, alpha);
                    PresentationCanvas[DTid.xy] = float4(pixelColor, 1.0f);
                    return;
                }
            }
        }
    }

    // Draw the instructions
    {
        int2 relPos = int2(DTid.xy) - c_instructionsPos;

        if (relPos.x >= 0 && relPos.y >= 0 && relPos.x < c_instructionsSize.x && relPos.y < c_instructionsSize.y)
        {
            PresentationCanvas[DTid.xy] = float4(/*$(Image:instructions.png:RGBA8_Unorm_sRGB:float:true)*/[relPos.xy].rrr, 1.0f);
            return;
        }

        if (relPos.x >= -c_borderSize && relPos.y >= -c_borderSize && relPos.x < c_instructionsSize.x + c_borderSize && relPos.y < c_instructionsSize.y + c_borderSize)
        {
            PresentationCanvas[DTid.xy] = c_borderColor;
            return;
        }
    }

    // background color
    PresentationCanvas[DTid.xy] = c_backgroundColor;
}
