// DemofoxVerletCar technique, shader CSGameplay
/*$(ShaderResources)*/

// game speed in ticks per second
static const float c_tickRate = 16.0;

// what factor to slow the game down by when it's game over
static const float c_tickRateGameOver = 0.0;

// how long the game over state slows down the simulation for
static const float c_gameOverSlowdownDuration = 3.0;

// how many simulation steps to do per tick.  More = more costly, but better simulations.
static const int c_numSimulationSteps = 1;

// simulation constants
static const float2 c_gravityAcceleration = float2(0.0, -5.0);  // -9.8 is real life values, but 1 unit != 1 meter in our sim.
static const float c_rotationMultiplier = 0.1; // how much rotation done
static const float c_throttleAcceleration = 4.0;
static const float c_fuelBurnRate = 10.0;
static const float c_checkPointDistance = 5.0; // in world units, distance between checkpoints.

// derived values
static const float c_tickDeltaTime = 1.0 / c_tickRate;
static const float c_tickDeltaTimeSq = c_tickDeltaTime*c_tickDeltaTime;

// keys
static const float KEY_SPACE = 32;
static const float KEY_LEFT  = 37;
static const float KEY_UP    = 38;
static const float KEY_RIGHT = 39;
static const float KEY_DOWN  = 40;

#include "DemofoxVerletCar_common.hlsl"

//============================================================
// save/load code from IQ's shader: https://www.shadertoy.com/view/MddGzf

float isInside( float2 p, float2 c ) { float2 d = abs(p-0.5-c) - 0.5; return -max(d.x,d.y); }
float isInside( float2 p, float4 c ) { float2 d = abs(p-0.5-c.xy-c.zw*0.5) - 0.5*c.zw - 0.5; return -max(d.x,d.y); }

#if 0
float4 loadValue( in float2 re )
{
    return texture( iChannel0, (0.5+re) / iChannelResolution[0].xy, -100.0 );
}

void storeValue( in float2 re, in float4 va, inout float4 fragColor, in float2 fragCoord )
{
    fragColor = ( isInside(fragCoord,re) > 0.0 ) ? va : fragColor;
}

void storeValue( in float4 re, in float4 va, inout float4 fragColor, in float2 fragCoord )
{
    fragColor = ( isInside(fragCoord,re) > 0.0 ) ? va : fragColor;
}
#endif

//============================================================
float2 RotatePoint (float2 p, float theta)
{
    return float2(p.x * cos(theta) - p.y * sin(theta), p.y * cos(theta) + p.x * sin(theta));
}

//============================================================
float2 RotatePointAroundPoint (float2 p, float2 origin, float theta)
{
    return RotatePoint(p-origin, theta) + origin;
}

//============================================================
void VerletIntegrate (inout float4 p, in float2 acceleration)
{
	float2 currentPos = p.xy;
    float2 lastPos = p.zw;

    float2 newPos = currentPos + currentPos - lastPos + acceleration * c_tickDeltaTimeSq;
    
    p.xy = newPos;
    p.zw = currentPos;
}

//============================================================
void ResolveGroundCollision (inout float2 p, inout bool pointTouchingGround)
{
    float2 gradient;
    float dist = EstimatedDistanceFromPointToGround (p, 1.0, 1.0, gradient) * -1.0;
    if (dist < c_wheelRadius)
    {
        float distanceAdjust = c_wheelRadius - dist;
        p -= normalize(gradient) * distanceAdjust;
        pointTouchingGround = true;
    }
}

//============================================================
void ResolveDistanceConstraint (inout float2 pointA, inout float2 pointB, float distance)
{
    // calculate how much we need to adjust the distance between the points
    // and cut it in half since we adjust each point half of the way
    float halfDistanceAdjust = (distance - length(pointB-pointA)) * 0.5;
    
    // calculate the vector we need to adjust along
    float2 adjustVector = normalize(pointB-pointA);
    
    // adjust each point half of the adjust distance, along the adjust vector
    pointA -= adjustVector * halfDistanceAdjust;
    pointB += adjustVector * halfDistanceAdjust;
}

bool KeyPressed(float key)
{
	return keyStates[key] != 0.0f;
}

/*$(_compute:CSGameplay)*/(uint3 DTid : SV_DispatchThreadID)
{
    //----- Load State -----    
	Struct_GameState s = gameState[0];
    float4 state         = s.state;//loadValue(txState);
    float4 state2        = s.state2;//loadValue(txState2);
    float4 state3        = s.state3;//loadValue(txState3);
    float4 state4        = s.state4;//loadValue(txState4);
    float4 backWheel     = s.backWheel;//loadValue(txBackWheel);   
    float4 frontWheel    = s.frontWheel;//loadValue(txFrontWheel);  
    float4 frontWheelCP1 = s.frontWheelCP1;//loadValue(txFrontWheelCP1);
    float4 frontWheelCP2 = s.frontWheelCP2;//loadValue(txFrontWheelCP2);
    float4 backWheelCP1  = s.backWheelCP1;//loadValue(txBackWheelCP1);
    float4 backWheelCP2  = s.backWheelCP2;//loadValue(txBackWheelCP2);
    
    //----- Initialize -----
    // init on frame 0 or if we are in the game over state and the space bar is pressed
    if (/*$(Variable:iFrame)*/ < 10)
    {
        state = float4(0.0, 0.0f, 0.0f, 0.0f);
        state2 = float4(0.0, 0.0f, 0.0f, 0.0f);
        state3 = float4(0.0, 0.0f, 0.0f, 0.0f);
        state4 = float4(0.0, 0.0f, 0.0f, 0.0f);
        backWheel = float4(0.0, 0.0f, 0.0f, 0.0f);
        frontWheel = float4(0.0, 0.0f, 0.0f, 0.0f);
        backWheelCP1 = float4(0.0, 0.0f, 0.0f, 0.0f);
        frontWheelCP1 = float4(0.0, 0.0f, 0.0f, 0.0f);
        backWheelCP2 = float4(0.0, 0.0f, 0.0f, 0.0f);
        frontWheelCP2 = float4(0.0, 0.0f, 0.0f, 0.0f);
        
        VAR_FRAME_PERCENT = 0.0;
        VAR_QUEUED_INPUT = 0.5;
        VAR_CAMERA_CENTER = float2(0.0, 0.0f);
        
        VAR_CAMERA_SCALE = 2.0;
        VAR_BACKWHEEL_ONGROUND = 0.0;
        VAR_FRONTWHEEL_ONGROUND = 0.0;
        VAR_GAMEOVER = 0.0;
            
        VAR_SIMSLOWDOWN = 1.0;
        VAR_SPEDOMETER = 0.0;
        VAR_LASTFUELORB = 0.0;
        VAR_FUELREMAINING = 1.0;
        
        backWheel = float4(-c_wheelDistance*0.5, c_wheelRadius, -c_wheelDistance*0.5, c_wheelRadius);
        frontWheel = float4( c_wheelDistance*0.5, c_wheelRadius, c_wheelDistance*0.5, c_wheelRadius);   

        // initialize checkpoint data to the starting line state
        backWheelCP1 = backWheel;
        backWheelCP2 = backWheel;
        
        frontWheelCP1 = frontWheel;
        frontWheelCP2 = frontWheel;
        
        VAR_FUELREMAININGCP1 = VAR_FUELREMAINING;
        VAR_FUELREMAININGCP2 = VAR_FUELREMAINING;
        VAR_LASTCPHIT  = 0.0;
    }
    
	// if it's game over and the user presses space bar, restore from the oldest checkpoint
    if (VAR_GAMEOVER == 1.0 && KeyPressed(KEY_SPACE))
    {
        frontWheel = frontWheelCP2;
        backWheel = backWheelCP2;
        
        // make sure you have at least half a tank of gas at a checkpoint. 
        // Your welcome (;
        VAR_FUELREMAINING = max(VAR_FUELREMAININGCP2, 0.5);
        
		VAR_GAMEOVER = 0.0;
        
        VAR_BACKWHEEL_ONGROUND = 0.0;
        VAR_FRONTWHEEL_ONGROUND = 0.0;
        
        VAR_LASTFUELORB = 0.0;
        
        // make sure that we reset CP1 to CP2, so that when we hit CP1 again
        // it doesn't fill in CP1, making it so restarting restarts us at CP1
        // which might be an unsafe checkpoint.  This shows itself as the problem
        // where sometimes some people reset to a checkpoint and they are flipped over
        // with no possible way to survive
        VAR_LASTCPHIT = floor(backWheel.x / c_checkPointDistance) * c_checkPointDistance;
        VAR_FUELREMAININGCP1 = VAR_FUELREMAININGCP2;
        frontWheelCP1 = frontWheelCP2;
        backWheelCP1 = backWheelCP2;        
    }
    
    // make  camera be centered on the bike
	VAR_CAMERA_CENTER = AsyncBikePos(backWheel, frontWheel, VAR_FRAME_PERCENT);    
    
    //----- Input -----
    // input seems backwards in code, but makes sense when playing.
    // Left = accelerate
    // Right = break;
    if (KeyPressed(KEY_RIGHT) || KeyPressed(KEY_UP))
    {
        VAR_QUEUED_INPUT = 1.0;
    }
    if (KeyPressed(KEY_LEFT) || KeyPressed(KEY_DOWN))
    {
        VAR_QUEUED_INPUT = 0.0;
    }
    
    //----- Simulate -----
	float iTimeDelta = /*$(Variable:iTimeDelta)*/;
    if (VAR_GAMEOVER == 1.0)
    {
        VAR_SIMSLOWDOWN += iTimeDelta / c_gameOverSlowdownDuration;
        VAR_SIMSLOWDOWN = min(VAR_SIMSLOWDOWN, 1.0);
    }
    else
    {
        VAR_SIMSLOWDOWN = 1.0;
    }
    
    // slow down the simulation if it's game over
    VAR_FRAME_PERCENT += iTimeDelta * lerp(c_tickRateGameOver, c_tickRate, pow(VAR_SIMSLOWDOWN, 2.0));
    if (VAR_FRAME_PERCENT > 1.0)
    {
        // reset our tick timer
        VAR_FRAME_PERCENT = frac(VAR_FRAME_PERCENT);
        
        float2 frontWheelRelativeToBackWheel = frontWheel.xy - backWheel.xy;
        
        // if both wheels are on the ground, and the front wheel is behind the back wheel,
        // that means we are upside down and it's game over.
        if (VAR_GAMEOVER != 1.0 && VAR_BACKWHEEL_ONGROUND == 1.0 && VAR_FRONTWHEEL_ONGROUND == 1.0 && dot(frontWheelRelativeToBackWheel, float2(-1.0,0.0)) > 0.0)
        {
            VAR_GAMEOVER = 1.0;
            VAR_SIMSLOWDOWN = 0.0;
        }
        
        // if we are in the game over state, stop accepting input
        if (VAR_GAMEOVER == 1.0)
            VAR_QUEUED_INPUT = 0.5;
        
        // burn fuel.  Game over when out of fuel!
        if (VAR_QUEUED_INPUT != 0.5)
        {
            VAR_FUELREMAINING -= 1.0 / (c_tickRate * c_fuelBurnRate);
            
            if (VAR_FUELREMAINING < 0.0)
            {
                VAR_GAMEOVER = 1.0;
                VAR_SIMSLOWDOWN = 0.0;
                VAR_FUELREMAINING = 0.0;
                VAR_QUEUED_INPUT = 0.5;
            }
        }        
        
        // if not game over, and we've passed a new checkpoint, store the info
        if (VAR_GAMEOVER == 0.0 && (backWheel.x - VAR_LASTCPHIT) > c_checkPointDistance)
        {
            VAR_LASTCPHIT = floor(backWheel.x / c_checkPointDistance) * c_checkPointDistance;            
            VAR_FUELREMAININGCP2 = VAR_FUELREMAININGCP1;
            VAR_FUELREMAININGCP1 = VAR_FUELREMAINING;
            frontWheelCP2 = frontWheelCP1;
            frontWheelCP1 = frontWheel;
            backWheelCP2 = backWheelCP1;
            backWheelCP1 = backWheel;
        }
        
        // calculate our acceleration - only accelerate if the back wheel is on the ground
        float2 acceleration = c_gravityAcceleration +
               ((VAR_BACKWHEEL_ONGROUND == 1.0)
                  ? float2(VAR_QUEUED_INPUT * 2.0 - 1.0, 0.0) * c_throttleAcceleration
                  : float2(0.0, 0.0));
        
        // calculate spin amount
        float spin = (VAR_QUEUED_INPUT * 2.0 - 1.0) * c_rotationMultiplier;
        
        // clear queued input
        VAR_QUEUED_INPUT = 0.5;
        
        // move the simulated points
        VerletIntegrate(backWheel, acceleration);
        VerletIntegrate(frontWheel, acceleration);
        
        // apply spin as rotation of the front wheel around the back wheel
        frontWheel.xy = RotatePointAroundPoint(frontWheel.xy, backWheel.xy, spin);
        
        // resolve physical constraints
        bool backWheelOnGround = false;
        bool frontWheelOnGround = false;
        for (int i = 0; i < c_numSimulationSteps; ++i)
        {
        	ResolveGroundCollision(backWheel.xy, backWheelOnGround);
        	ResolveGroundCollision(frontWheel.xy, frontWheelOnGround);
            
            ResolveDistanceConstraint(backWheel.xy, frontWheel.xy, c_wheelDistance);
        }
        
        // remember whether our wheels are on the ground or not
        VAR_BACKWHEEL_ONGROUND = backWheelOnGround ? 1.0 : 0.0;
        VAR_FRONTWHEEL_ONGROUND = frontWheelOnGround ? 1.0 : 0.0;
        
        // cheat code teleportation
        if (backWheel.x < -50.0)
        {
        	backWheel = float4(3000.0 + -c_wheelDistance*0.5, 0.5, 3000.0 + -c_wheelDistance*0.5, 0.5);
        	frontWheel = float4(3000.0 + c_wheelDistance*0.5, 0.5, 3000.0 + c_wheelDistance*0.5, 0.5);   
        	VAR_CAMERA_CENTER = AsyncBikePos(backWheel, frontWheel, VAR_FRAME_PERCENT);
            VAR_FUELREMAINING = 9999.0;
        }
    }
    
    // if the bike is close to a fuel orb, replenish fuel
    // Do it ouside of the tick since we interpolate position so could otherwise miss it
    // We could also do a swept shape test but this is quicker
    float2 asyncBikePos = AsyncBikePos(backWheel, frontWheel, VAR_FRAME_PERCENT);
    float2 uvFuel;
    uvFuel.x = mod(asyncBikePos.x, c_fuelCanDistance) - c_fuelCanDistance * 0.5;
    uvFuel.y = GroundHeightAtX(floor(asyncBikePos.x / c_fuelCanDistance) * c_fuelCanDistance + c_fuelCanDistance * 0.5, 1.0);
    uvFuel.y += c_fuelCanRadius*1.1;
    if ( VAR_LASTFUELORB < asyncBikePos.x && length(asyncBikePos - float2(uvFuel.x+asyncBikePos.x, uvFuel.y)) < c_fuelCanRadius * 2.0)
    {
        VAR_FUELREMAINING = max(VAR_FUELREMAINING, 1.0);
        VAR_LASTFUELORB = floor(asyncBikePos.x / c_fuelCanDistance) * c_fuelCanDistance + c_fuelCanDistance * 0.5 + c_fuelCanRadius*2.0;
    }    
    
    //----- Update Spedometer -----
    float spedTarget = 1.2 * (length(backWheel.xy - backWheel.zw) + length(frontWheel.xy - frontWheel.zw)) * 0.5;
	VAR_SPEDOMETER += (spedTarget - VAR_SPEDOMETER) * 3.0 * iTimeDelta;
    
    //----- Update Camera -----
    // The camera is always centered on the bike
    VAR_CAMERA_CENTER = asyncBikePos;
    // The camera zooms out as you go faster
    float distAboveGround = asyncBikePos.y - GroundHeightAtX(asyncBikePos.x, 1.0);
    float targetZoom = 0.0;//clamp(distAboveGround * 4.0, 2.0, 6.0);
    targetZoom = max(targetZoom, VAR_SPEDOMETER * 4.0 + 2.0);
    
    VAR_CAMERA_SCALE = lerp(VAR_CAMERA_SCALE, targetZoom, iTimeDelta * lerp(c_tickRateGameOver, c_tickRate, pow(VAR_SIMSLOWDOWN, 2.0)) / 25.0); 
    VAR_CAMERA_SCALE = clamp(VAR_CAMERA_SCALE, 2.0, 6.0);
    
    
    //----- Save State -----
    //fragColor = float4(0.0);
    //storeValue(txState, state, fragColor, fragCoord);
    //storeValue(txState2, state2, fragColor, fragCoord);
    //storeValue(txState3, state3, fragColor, fragCoord);
    //storeValue(txState4, state4, fragColor, fragCoord);
    //storeValue(txBackWheel , backWheel , fragColor, fragCoord);
    //storeValue(txFrontWheel, frontWheel, fragColor, fragCoord);
    //storeValue(txBackWheelCP1, backWheelCP1, fragColor, fragCoord);
    //storeValue(txBackWheelCP2, backWheelCP2, fragColor, fragCoord);
    //storeValue(txFrontWheelCP1, frontWheelCP1, fragColor, fragCoord);
    //storeValue(txFrontWheelCP2, frontWheelCP2, fragColor, fragCoord);	

    s.state = state;
    s.state2 = state2;
    s.state3 = state3;
    s.state4 = state4;
    s.backWheel = backWheel;
    s.frontWheel = frontWheel;
    s.frontWheelCP1 = frontWheelCP1;
    s.frontWheelCP2 = frontWheelCP2;
    s.backWheelCP1 = backWheelCP1;
    s.backWheelCP2 = backWheelCP2;
	gameState[0] = s;
}
