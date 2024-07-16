// DemofoxMinesweeper technique, shader Display CS
/*$(ShaderResources)*/

// the size in X and Y of our gameplay grid
//const float c_gridSize = 16.0;
//const float c_maxGridCell = c_gridSize - 1.0;

// graphics values
#define CELL_SHADE_MARGIN 0.1
#define CELL_SHADE_DARK   0.4
#define CELL_SHADE_MED    0.8
#define CELL_SHADE_LIGHT  1.0
#define CELL_SHADE_REVEAL 0.6
#define CELL_SHADE_MARGIN_REVEALED 0.025

// variables
//const float2 txState = float2(2.0, c_gridSize);  // x = state. y = mouse button down last frame. zw unused

//============================================================
// save/load code from IQ's shader: https://www.shadertoy.com/view/MddGzf

//float4 loadValue( in float2 re )
//{
    //return texture( iChannel0, (0.5+re) / iChannelResolution[0].xy, -100.0 );
//}

float mod(float x, float y)
{
	return x - y * floor(x/y);
}

//============================================================
float SDFCircle( in float2 coords, in float3 circle )
{
    coords -= circle.xy;
    float v = coords.x * coords.x + coords.y * coords.y;
    float2  g = float2(circle.z * coords.x, circle.z * coords.y);
    return abs(v)/length(g); 
}

//============================================================
void PixelToCell (in float2 fragCoord, out float2 uv, out float2 cell, out float2 cellFract)
{
	uint w, h;
	output.GetDimensions(w, h);
	float2 iResolution = float2(w, h);

    float aspectRatio = iResolution.x / iResolution.y;
    uv = (fragCoord.xy / iResolution.xy) * 1.2 - float2(0.1f, 0.1f);
    uv.x *= aspectRatio;
    cell = floor(uv * float2/*$(Variable:c_gridSize2D)*/);
    cellFract = frac(uv * float2/*$(Variable:c_gridSize2D)*/);
}

//============================================================
float3 BackgroundPixel (in float2 uv)
{
	uint w, h;
	/*$(Image:flagstone-rubble-7669-in-architextures.png:RGBA8_Unorm_sRGB:float4:false)*/.GetDimensions(w, h);

	float iTime = /*$(Variable:iTime)*/;

    float distortX = sin(iTime * 0.6 + uv.x*5.124) * 0.03 + iTime*0.06;
    float distortY = sin(iTime * 0.7 + uv.y*3.165) * 0.05 + iTime*0.04;
    
    float2 offsetG = float2(sin(iTime*1.534), cos(iTime*1.453)) * 10.0 / float2(w,h);
    float2 offsetB = float2(sin(iTime*1.756), cos(iTime*1.381)) * 10.0 / float2(w,h);

   	float3 ret;    
    ret.r = /*$(Image:flagstone-rubble-7669-in-architextures.png:RGBA8_Unorm_sRGB:float4:false)*/.SampleLevel(texSampler, uv + float2(distortX, distortY), 0).r;
    ret.g = /*$(Image:flagstone-rubble-7669-in-architextures.png:RGBA8_Unorm_sRGB:float4:false)*/.SampleLevel(texSampler, uv + float2(distortX, distortY) + offsetG, 0).r;
    ret.b = /*$(Image:flagstone-rubble-7669-in-architextures.png:RGBA8_Unorm_sRGB:float4:false)*/.SampleLevel(texSampler, uv + float2(distortX, distortY) + offsetB, 0).b;
    return ret;
}

//============================================================
float3 HiddenTileColor (in float2 cell, in float2 cellFract, in float2 mouseCell)
{
    float addMedium = clamp((1.0 - step(cellFract.x, CELL_SHADE_MARGIN)) * (1.0 - step(cellFract.y, CELL_SHADE_MARGIN)), 0.0, 1.0);
    float addLight = clamp(step(1.0 - cellFract.x, CELL_SHADE_MARGIN) + step(1.0 - cellFract.y, CELL_SHADE_MARGIN), 0.0, 1.0);
    addLight *= addMedium;
   
   	float unClickedColor =
        CELL_SHADE_DARK +
        (CELL_SHADE_MED - CELL_SHADE_DARK) * addMedium +
        (CELL_SHADE_LIGHT - CELL_SHADE_MED) * addLight;
    
    float3 ret = float3(unClickedColor.xxx);   
    
    if (cell.x == mouseCell.x && cell.y == mouseCell.y)
        ret.z = 0.0;
    
    return ret;
}

//============================================================
float OutsideCircle (in float2 p, in float3 circle)
{
    return length(p-circle.xy) > circle.z ? 1.0 : 0.0;
}

//============================================================
float3 CountTileColor (in float2 cellFract, float count)
{
    float color = CELL_SHADE_REVEAL;
    
    // if this is an odd number, put a dot in the center
    if (mod(count,2.0) == 1.0)
        color *= smoothstep(0.7,1.5,SDFCircle(cellFract, float3(0.5,0.5,0.1)));
        //color *= OutsideCircle(cellFract, float3(0.5,0.5,0.1));
    
    // if greater than or equal to two, put a dot in the lower left and upper right corner
    if (count >= 2.0)
    {
        color *= smoothstep(0.7,1.5,SDFCircle(cellFract, float3(0.25,0.25,0.1)));
        color *= smoothstep(0.7,1.5,SDFCircle(cellFract, float3(0.75,0.75,0.1)));
    }
    
    // if greater than or equal to four, put a dot in the upper left and lower right corner
    if (count >= 4.0)
    {
        color *= smoothstep(0.7,1.5,SDFCircle(cellFract, float3(0.25,0.75,0.1)));
        color *= smoothstep(0.7,1.5,SDFCircle(cellFract, float3(0.75,0.25,0.1)));   
    }
    
    // if greater than or equal to 6, put a dot on the left and right
    if (count >= 6.0)
    {
        color *= smoothstep(0.7,1.5,SDFCircle(cellFract, float3(0.25,0.5,0.1)));
        color *= smoothstep(0.7,1.5,SDFCircle(cellFract, float3(0.75,0.5,0.1)));          
    }
    
    // if greater than or equal to 8, put a dot on the top and bottom
   	if (count >= 8.0)
    {
        color *= smoothstep(0.7,1.5,SDFCircle(cellFract, float3(0.5,0.25,0.1)));
        color *= smoothstep(0.7,1.5,SDFCircle(cellFract, float3(0.5,0.75,0.1)));           
    }
    
    if ((cellFract.x < CELL_SHADE_MARGIN_REVEALED) || (cellFract.y < CELL_SHADE_MARGIN_REVEALED) ||
        ((1.0 - cellFract.x) < CELL_SHADE_MARGIN_REVEALED) || ((1.0 - cellFract.y) < CELL_SHADE_MARGIN_REVEALED))
        color = CELL_SHADE_DARK;
    
    return float3(color.xxx);
}

//============================================================
float3 FlagColor (in float2 cell, in float2 cellFract, in float2 mouseCell)
{
    float3 pixel = HiddenTileColor(cell, cellFract, mouseCell);
    
    pixel.xz *= smoothstep(1.0,1.5,SDFCircle(cellFract, float3(0.5,0.5,0.2)));
    
    return pixel;
}

//============================================================
float3 BombColor (in float2 cellFract)
{
    float shade = 0.0;
    
    shade += (1.0 - smoothstep(1.0,1.5,SDFCircle(cellFract, float3(0.5,0.5,0.15))));
    shade += (1.0 - smoothstep(1.0,1.5,SDFCircle(cellFract, float3(0.3,0.3,0.1))));
    shade += (1.0 - smoothstep(1.0,1.5,SDFCircle(cellFract, float3(0.3,0.7,0.1))));
    shade += (1.0 - smoothstep(1.0,1.5,SDFCircle(cellFract, float3(0.7,0.3,0.1))));
    shade += (1.0 - smoothstep(1.0,1.5,SDFCircle(cellFract, float3(0.7,0.7,0.1))));
    
    return float3(clamp(shade,0.0,1.0).xxx);
}

//============================================================
float3 TileColor (in float2 cell, in float2 cellFract, float4 cellData, float2 mouseCell, bool gameOver)
{
    // on game over, we show all bombs
    if (gameOver)
    {
        // if it's a bomb, show the bomb always
        if (cellData.z == 1.0)
            return BombColor(cellFract);
        // else if it's unrevealed, show the unrevealed tile
        else if (cellData.x == 0.0)
			return HiddenTileColor(cell, cellFract, mouseCell);
        // else show the number of bomb neighbors there are
        else
            return CountTileColor(cellFract, floor(cellData.y * 8.0));
    }
    // else we are playing normal so show everything
    else
    {
        // if it's unrevealed
        if (cellData.x == 0.0)
        {
            // if it's flagged, draw a flag
            if (cellData.w == 1.0)
                return FlagColor(cell, cellFract, mouseCell);
            // else show a regular unrevealed tile
            else
                return HiddenTileColor(cell, cellFract, mouseCell);        
        }
        // else if it's revealed
        else
        {
            // if it's a bomb, draw a bomb
            if (cellData.z == 1.0)
                return BombColor(cellFract);
            // else draw how many neighbors are bombs
            else
                return CountTileColor(cellFract, floor(cellData.y * 8.0));
        }
    }
}

/*$(_compute:displaycs)*/(uint3 DTid : SV_DispatchThreadID)
{
    // load the game state variable
    uint gameStateReadIndex = uint(/*$(Variable:iFrame)*/ + 1) % 2;
    float4 state    = gameState[gameStateReadIndex];

    // calculate the cell data for this specific pixel
    // draw the background if we are outside of the grid
    float2 uv, cell, cellFract;
    PixelToCell(DTid.xy, uv, cell, cellFract);
    if (cell.x < 0.0 || cell.y < 0.0 || cell.x > (/*$(Variable:c_gridSize1D)*/-1) || cell.y > (/*$(Variable:c_gridSize1D)*/-1))
    {
        output[DTid.xy] = float4(BackgroundPixel(uv), 1.0);
        return;
    }
    
    // calculate where the mouse is
	float4 mouseState = /*$(Variable:MouseState)*/;
    float2 mouseUv, mouseCell, mouseCellFract;
    PixelToCell(mouseState.xy, mouseUv, mouseCell, mouseCellFract);
    mouseCell *= mouseState.z > 0.0 ? 1.0 : -1.0;

    // get the data for the current cell
    float4 cellData = gameBoard[cell];
    
    // draw grid of cells
    bool gameOver = state.x > 0.2;
    float3 pixelColor = TileColor(cell, cellFract, cellData, mouseCell, gameOver);
    
    // if we won, make everything green
    if (state.x > 0.3)
         pixelColor.xz = float2(0.0, 0.0f);
    // else if we lost, make everything red
    else if (state.x > 0.2)
        pixelColor.yz = float2(0.0f, 0.0f);

    
    // DEBUG: Visualize all game state
    //pixelColor = gameBoard.SampleLevel(texSampler, uv, 0).rba;

    // DEBUG: Visualize cell fractional offsets
    //pixelColor = float3(cellFract, 0.0);
    
    // DEBUG: Visualize grid cells
    //pixelColor = float3(cell / /*$(Variable:c_gridSize1D)*/, 0.0);
    
    // DEBUG: visualize grid cell data    
    //pixelColor = cellData.rgb;
    
    output[DTid.xy] = float4(pixelColor, 1.0);	
}

