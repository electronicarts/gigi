// DemofoxMinesweeper technique, shader Gameplay CS
/*$(ShaderResources)*/

// the size in X and Y of our gameplay grid
//const float c_gridSize = 16.0;
//const float c_maxGridCell = c_gridSize - 1.0;
static const float c_numBombs = 32.0;  // on average, this many bombs will show up. could be more or less though.
static const int c_gridChecksPerFrame = 16;

// The grid representing the board
// x = revealed (1.0) or not (0.0)
// y = how many bombs are near it 0 to 8, normalized to 0..1
// z = is a bomb (1.0) or not a bomb (0.0)
// w = is a flag (1.0) or not a flag (0.0)
//static const float4 txCells = float4(0.0, 0.0, /*$(Variable:c_gridSize1D)*/ - 1.0, /*$(Variable:c_gridSize1D)*/ - 1.0);

// other variables
//const float2 txState = float2(2.0, c_gridSize);
// x = game state.
// y = unused.  Was: mouse button down last frame. 
// z,w = last cell checked for victory

// keys
//const float KEY_SPACE = 32.5/256.0;
//const float KEY_F = 70.5/256.0;

//============================================================

// save/load code from IQ's shader: https://www.shadertoy.com/view/MddGzf

//float isInside( float2 p, float2 c ) { float2 d = abs(p-0.5-c) - 0.5; return -max(d.x,d.y); }
//float isInside( float2 p, float4 c ) { float2 d = abs(p-0.5-c.xy-c.zw*0.5) - 0.5*c.zw - 0.5; return -max(d.x,d.y); }

//float4 loadValue( in float2 re )
//{
//    return texture( iChannel0, (0.5+re) / iChannelResolution[0].xy, -100.0 );
//}

//void storeValue( in float2 re, in float4 va, inout float4 fragColor, in float2 fragCoord )
//{
    //fragColor = ( isInside(fragCoord,re) > 0.0 ) ? va : fragColor;
//}

//void storeValue( in float4 re, in float4 va, inout float4 fragColor, in float2 fragCoord )
//{
    //fragColor = ( isInside(fragCoord,re) > 0.0 ) ? va : fragColor;
//}

//============================================================
float rand(float2 co)
{
    return frac(sin(dot(co.xy ,float2(12.9898,78.233))) * 43758.5453);
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
float4 GetCellData (in float2 cell)
{
    if (cell.x >= 0.0 && cell.y >= 0.0 && cell.x <= (/*$(Variable:c_gridSize1D)*/-1) && cell.y <= (/*$(Variable:c_gridSize1D)*/-1))
        return gameBoard[cell];
    else
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

/*$(_compute:gameplaycs)*/(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= /*$(Variable:c_gridSize1D)*/ || DTid.y >= /*$(Variable:c_gridSize1D)*/)
        return;
    
    //----- Load State -----
    float2 cell     = float2(DTid.xy);

    uint gameStateReadIndex = uint(/*$(Variable:iFrame)*/) % 2;
    uint gameStateWriteIndex = (gameStateReadIndex + 1) % 2;
    
    float4 state    = gameState[gameStateReadIndex];
    float4 cellData = gameBoard[cell];//GetCellData(cell);//loadValue(fragCoord.xy-0.5);
    
    // calculate the cell to check, from state.zw
    float2 cellCheck = state.zw;
    
    // get the neighboring cell data
    float4 cellDataUL = GetCellData(cell + float2(-1.0,  1.0));
    float4 cellDataU  = GetCellData(cell + float2( 0.0,  1.0));
    float4 cellDataUR = GetCellData(cell + float2( 1.0,  1.0));
    float4 cellDataR  = GetCellData(cell + float2( 1.0,  0.0));
    float4 cellDataDR = GetCellData(cell + float2( 1.0, -1.0));
    float4 cellDataD  = GetCellData(cell + float2( 0.0, -1.0));
    float4 cellDataDL = GetCellData(cell + float2(-1.0, -1.0));
    float4 cellDataL  = GetCellData(cell + float2(-1.0,  0.0));     
    
    // calculate where the mouse is
	float2 mousePos = /*$(Variable:MouseState)*/.xy;
    float2 mouseUv, mouseCell, mouseCellFract;
    PixelToCell(mousePos, mouseUv, mouseCell, mouseCellFract);   

    // reset game state on first frame
    if (/*$(Variable:iFrame)*/ == 0 || /*$(Variable:Reset)*/)
        state = float4(0.0, 0.0f, 0.0f, 0.0f);
    
    // state .0 -> reset game state
    if (state.x < 0.1)
    {
        state.x = 0.1;
        state.y = 0.0;
        state.zw = float2(0.0, 0.0f);
        
        // initialize all grid cells to unrevealed, zero neighbor bombs, not a bomb
        cellData = float4(0.0, 0.0f, 0.0f, 0.0f);
        
        // random up some bombs!
        if (rand(cell+/*$(Variable:iTime)*/) < c_numBombs / (/*$(Variable:c_gridSize1D)*/*/*$(Variable:c_gridSize1D)*/))
            cellData.z = 1.0;
    }
   	// state .1 -> set neighbor counts on grid
    else if (state.x < 0.2)
    {
        state.x = 0.2;
        float neighborBombs = 
            (cellDataUL.z > 0.0 ? 1.0 : 0.0) +
            (cellDataU.z  > 0.0 ? 1.0 : 0.0) +
            (cellDataUR.z > 0.0 ? 1.0 : 0.0) +
            (cellDataR.z  > 0.0 ? 1.0 : 0.0) +
            (cellDataDR.z > 0.0 ? 1.0 : 0.0) +
            (cellDataD.z  > 0.0 ? 1.0 : 0.0) +
            (cellDataDL.z > 0.0 ? 1.0 : 0.0) +
            (cellDataL.z  > 0.0 ? 1.0 : 0.0);
        
        cellData.y = neighborBombs / 8.0;
        
        // DEBUG: visualize the numbers by uncommenting this
		//cellData.y = clamp(cell.x / 8.0, 0.0, 1.0);
    }
   	// state .2 -> we are playing!
    else if (state.x < 0.3)
    {
		// If the right mouse was released, they may want to flag something
		if (/*$(Variable:MouseStateLastFrame)*/.w == 1.0f && /*$(Variable:MouseState)*/.w == 0.0f)
		{
			// toggle the cell as flagged
			if (cell.x == mouseCell.x && cell.y == mouseCell.y)
				cellData.w = 1.0 - cellData.w;
		}

		// If left mouse was released
		if (/*$(Variable:MouseStateLastFrame)*/.z == 1.0f && /*$(Variable:MouseState)*/.z == 0.0f)
        {
			// get the data for the cell under the mouse
			float4 mouseCellData = GetCellData(mouseCell); 
								
			if (mouseCellData.w < 1.0)
			{
				// reveal the cell
				if (cell.x == mouseCell.x && cell.y == mouseCell.y)
					cellData.x = 1.0;

				// if they clicked on a bomb, die
				if (mouseCellData.z == 1.0)
					state.x = 0.3;
			}
        }

        // if the current cell we are processing is unrevealed
        // and there are neighbors which have zero bomb neighbors but are revealed, reveal this
        // cell too.
        if (cellData.x == 0.0)
        {
            // if any of the neighbors are revealed zeros, then reveal this cell
            if ((cellDataUL.x == 1.0 && cellDataUL.y == 0.0) ||
                (cellDataU.x  == 1.0 && cellDataU.y  == 0.0) ||
                (cellDataUR.x == 1.0 && cellDataUR.y == 0.0) ||
                (cellDataR.x  == 1.0 && cellDataR.y  == 0.0) ||
                (cellDataDR.x == 1.0 && cellDataDR.y == 0.0) ||
                (cellDataD.x  == 1.0 && cellDataD.y  == 0.0) ||
                (cellDataDL.x == 1.0 && cellDataDL.y == 0.0) ||
                (cellDataL.x  == 1.0 && cellDataL.y  == 0.0))
            {
                cellData.x = 1.0;
            }
        }
        
        // check for victory by scanning cells to find any unrevealed cells that aren't bombs.
        for (int i = 0; i < c_gridChecksPerFrame; ++i)
        {
            if (cellCheck.x == (/*$(Variable:c_gridSize1D)*/-1))
            {
                cellCheck.x = 0.0;
                if (cellCheck.y == (/*$(Variable:c_gridSize1D)*/-1))
                {
                    // Victory!
                    cellCheck.y = 0.0;
                    state.x = 0.4;
                    break;
                }
                else
                {
                    cellCheck.y = cellCheck.y + 1.0;
                }
            }
            else
            {
                cellCheck.x = cellCheck.x + 1.0;
            }

            // if the cell we are checking is unrevealed, and not a bomb, start our scan over, they haven't won yet.
            float4 cellCheckData = GetCellData(cellCheck);
            if (cellCheckData.x == 0.0 && cellCheckData.z == 0.0)
            {
                cellCheck = float2(0.0f, 0.0f);
                break;
            }
        }
        
    }
   	// state .3 -> we lost
	// state .4 -> we won
    else
    {
        // reset when user presses space
		if (/*$(Variable:Reset)*/)
			state.x = 0.0; 
        //if (texture( iChannel1, float2(KEY_SPACE,0.25) ).x == 1.0)
        	//state.x = 0.0; 
    }
            
	// convert cellCheck back into state.zw
    state.zw = cellCheck;
    
    //----- Save State -----
    //float4 fragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    //storeValue(txState, state   , fragColor, fragCoord);
    //storeValue(txCells, cellData, fragColor, fragCoord);	
	
	gameBoard[cell] = cellData;

	if (DTid.x == 0 && DTid.y == 0 && DTid.z == 0)
		gameState[gameStateWriteIndex] = state;
}
