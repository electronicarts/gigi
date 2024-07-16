// DemofoxVerletCar technique, shader CSDisplay
/*$(ShaderResources)*/

/*
Credits:

Chanel Wolfe - Art Direction
Paul Im - Technical Advisor (Thanks for convincing me to try Verlet!)
Alan Wolfe - Everything Else

Eiffie - Gave a code change to fix a bug with the score display on some machines.
Nrx - Found an uninitialized variable being used, causing graphical glitches on some machines.

Some great resources on game physics:

http://gafferongames.com/game-physics/
http://lonesock.net/article/verlet.html
http://www.gamedev.net/page/resources/_/technical/math-and-physics/a-verlet-based-approach-for-2d-game-physics-r2714
*/

#define AA_AMOUNT 7.0 / iResolution.x

#define SCORE_SIZE 25.0  // in pixels

#define DEBUG_WHEELSTOUCHING 0  // wheels tinted green when they are touching the ground
#define DEBUG_FUELCOLLIDE 0 // Shows the fuel orb colliding area around the car

static const float c_grassDistance = 0.25; // how far apart on the x axis
static const float c_grassMaxDepth = 1.0;  // how far below ground level it can go

static const float c_treeDistance = 3.0; // how far apart on the x axis
static const float c_treeMaxDepth = 0.5; // how far below ground level it can go

static const float c_cloudDistance = 2.0; // how far apart on the x axis
static const float c_cloudMaxDepth = 2.0; // vertical offset. +/- this amount max

#include "DemofoxVerletCar_common.hlsl"

//============================================================
// save/load code from IQ's shader: https://www.shadertoy.com/view/MddGzf

//float4 loadValue( in float2 re )
//{
    //return texture( iChannel0, (0.5+re) / iChannelResolution[0].xy, -100.0 );
//}

//============================================================
// Signed Distance Functions taken/adapted/inspired by from:
// https://iquilezles.org/articles/distfunctions

float UDCircle( in float2 coords, in float2 circle, float radius)
{    
    return max(length(coords - circle.xy) - radius, 0.0);
}

//============================================================
float UDFatLineSegment (in float2 coords, in float2 A, in float2 B, in float height)
{    
    // calculate x and y axis of box
    float2 xAxis = normalize(B-A);
    float2 yAxis = float2(xAxis.y, -xAxis.x);
    float width = length(B-A);
    
	// make coords relative to A
    coords -= A;
    
    float2 relCoords;
    relCoords.x = dot(coords, xAxis);
    relCoords.y = dot(coords, yAxis);
    
    // calculate closest point
    float2 closestPoint;
    closestPoint.x = clamp(relCoords.x, 0.0, width);
    closestPoint.y = clamp(relCoords.y, -height * 0.5, height * 0.5);
    
    return length(relCoords - closestPoint);
}

//============================================================
float rand(float2 co)
{
    return frac(sin(dot(co.xy ,float2(12.9898,78.233))) * 43758.5453);
}

//============================================================
float RandomFloat (float2 seed) // returns 0..1
{
    return rand(float2(seed.x*0.645, 0.453+seed.y*0.329));
}

//============================================================
//number rendering from https://www.shadertoy.com/view/XdjSWz
bool number(int x, int y, int n)
{
    return ((y==1 && x>1 && x<5 && n!=1 && n!=4) ||
            (y==5 && x>1 && x<5 && n!=0 && n!=1 && n!=7) ||
            (y==9 && x>1 && x<5 && n!=1 && n!=4 && n!=7) ||
            (x==1 && y>1 && y<5 && n!=1 && n!=2 && n!=3) ||
            (x==5 && y>1 && y<5 && n!=5 && n!=6) ||
            (x==1 && y>5 && y<9 && (n==0 || n==2 || n==6 || n==8)) ||
            (x==5 && y>5 && y<9 && n!=2) );
}

//============================================================
void DrawGrass (in float2 uv, inout float3 pixelColor, in float3 tint, in float scale)
{
	uint w, h;
	output.GetDimensions(w, h);
	float2 iResolution = float2(w, h);

    // draws periodic grass tufts
    float2 grassOrigin;
    grassOrigin.x = floor(uv.x / c_grassDistance) * c_grassDistance + c_grassDistance * 0.5;
    grassOrigin.y = GroundHeightAtX(grassOrigin.x, scale);    
    
    float forceTop = RandomFloat(grassOrigin + float2(0.342, 0.856)) > 0.25 ? 1.0 : 0.0;
    grassOrigin.y -= forceTop * (RandomFloat(grassOrigin + float2(0.756, 0.564)) * c_grassMaxDepth);
    
    float2 grassYAxis = -GroundFunctionGradiant(grassOrigin, scale);
    float2 grassXAxis = float2(grassYAxis.y, -grassYAxis.x);
    
    float2 uvRelative = uv - grassOrigin;
    float2 uvLocal;
    uvLocal.x = dot(uvRelative, grassXAxis);
    uvLocal.y = dot(uvRelative, grassYAxis);
    uvLocal /= scale;
    
    float snowLine = sin(uv.x*2.35) * 0.1 + sin(uv.x*3.14) * 0.01;
    float grassStoneMix = smoothstep(snowLine-0.3, snowLine+0.3, uv.y);        
    float3 grassColor = lerp(float3(0.3,0.4,0.1),float3(0.7,0.8,0.5),grassStoneMix * 0.5);
    
    // draw a few random tufts
    for (int i = 0; i < 5; ++i)
    {
    	float2 endPoint;
        endPoint.x = (RandomFloat(grassOrigin + float2(0.254, 0.873) * float(i)) * 2.0 - 1.0) * 0.1;
        endPoint.y = RandomFloat(grassOrigin + float2(0.254, 0.873) * float(i)) * 0.03 + 0.02;
        
        float2 startingOffset;
        startingOffset.x = endPoint.x  * 0.6;
        startingOffset.y = 0.0;
        
    	float tuftDistance = UDFatLineSegment(uvLocal, startingOffset, endPoint, 0.01);
    	tuftDistance = 1.0 - smoothstep(0.0, AA_AMOUNT, tuftDistance);
		pixelColor = lerp(pixelColor, grassColor * tint, tuftDistance);
    }
}

//============================================================
void DrawTrees (in float2 uv, inout float3 pixelColor, in float3 tint, in float scale)
{
	uint w, h;
	output.GetDimensions(w, h);
	float2 iResolution = float2(w, h);

    // draw periodic trees
    float2 treeOrigin;
    treeOrigin.x = floor(uv.x / c_treeDistance) * c_treeDistance + c_treeDistance * 0.5;
    treeOrigin.y = GroundHeightAtX(treeOrigin.x, scale);    
    
    float forceTop = 1.0;//RandomFloat(treeOrigin + float2(0.342, 0.856)) > 0.75 ? 1.0 : 0.0;
    treeOrigin.y -= forceTop * (RandomFloat(treeOrigin + float2(0.756, 0.564)) * c_treeMaxDepth);
    
    float2 treeYAxis = -GroundFunctionGradiant(treeOrigin, scale);
    float2 treeXAxis = float2(treeYAxis.y, -treeYAxis.x);
    
    float2 uvRelative = uv - treeOrigin;
    float2 uvLocal;
    uvLocal.x = dot(uvRelative, treeXAxis);
    uvLocal.y = dot(uvRelative, treeYAxis);
    uvLocal /= scale;
    
    // draw a brown trunk
   	float dist = UDFatLineSegment(uvLocal, float2(0.0, 0.0), float2(0.0,0.15), 0.035);
   	dist = 1.0 - smoothstep(0.0, AA_AMOUNT, dist);
	pixelColor = lerp(pixelColor, float3(0.6, 0.3, 0.1) * tint, dist);
    
    // draw some green circles
    dist = 1.0;
    for (int i = 0; i < 5; ++i)
    {
       	float3 circle;
        circle.x = 0.05 * (RandomFloat(treeOrigin + float2(0.453, 0.923) * float(i)) * 2.0 - 1.0);
        circle.y = 0.08 + 0.2 * RandomFloat(treeOrigin + float2(0.543, 0.132) * float(i));
        circle.z = 0.05 + 0.02 * RandomFloat(treeOrigin + float2(0.132, 0.645) * float(i));
    	dist = min(dist, UDCircle(uvLocal, circle.xy, circle.z));  
    }    
    dist = 1.0 - smoothstep(0.0, AA_AMOUNT * 3.0, dist);
    pixelColor = lerp(pixelColor, float3(0.0,0.4,0.0) * tint, dist);       
}

//============================================================
void DrawHills (in float2 uv, inout float3 pixelColor, in float3 tint, in float scale)
{
	uint w, h;
	output.GetDimensions(w, h);
	float2 iResolution = float2(w, h);

    float snowLine = sin(uv.x*2.35) * 0.1 + sin(uv.x*3.14) * 0.01;
    float grassStoneMix = smoothstep(snowLine-0.3, snowLine+0.3, uv.y);    
    
    float dist = EstimatedDistanceFromPointToGround(uv, scale) * -1.0;
    float green = clamp(dist * -3.0, 0.0, 1.0);
    green = smoothstep(0.0, 1.0, green) * 0.25;
    float3 grassPixel = lerp(pixelColor, float3(0.35, (0.85 - green), 0.15) * tint, 1.0 - smoothstep(0.0, AA_AMOUNT, dist)); 
    
    float3 stonePixel = lerp(pixelColor, float3((0.85 - green), (0.85 - green), (0.85 - green)) * tint, 1.0 - smoothstep(0.0, AA_AMOUNT, dist)); 
    
    pixelColor = lerp(grassPixel,stonePixel,grassStoneMix);  
    
    DrawGrass(uv, pixelColor, tint, scale);
    DrawTrees(uv, pixelColor, tint, scale);
}

//============================================================
void DrawClouds (in float2 uv, inout float3 pixelColor, in float3 tint, in float scale, in float alpha)
{
	uint w, h;
	output.GetDimensions(w, h);
	float2 iResolution = float2(w, h);

    // draw clusters of tinted white circles?
    float2 cloudOrigin = float2(0.0, 0.0f);
    cloudOrigin.x = floor(uv.x / c_cloudDistance) * c_cloudDistance + c_cloudDistance * 0.5;
    cloudOrigin.y = (RandomFloat(cloudOrigin + float2(0.453, 0.748) * 2.0 - 1.0) * c_cloudMaxDepth);
    
    float2 uvRelative = uv - cloudOrigin;
    uvRelative /= scale;    
    
    float dist = 1.0;
    for (int i = 0; i < 10; ++i)
    {
       	float3 circle;
        circle.x = 0.5 * (RandomFloat(cloudOrigin + float2(0.453, 0.923) * float(i)) * 2.0 - 1.0);
        circle.y = 0.08 + 0.2 * RandomFloat(cloudOrigin + float2(0.543, 0.132) * float(i));
        circle.z = 0.1 + 0.1 * RandomFloat(cloudOrigin + float2(0.132, 0.645) * float(i));
    	dist = min(dist, UDCircle(uvRelative, circle.xy, circle.z));  
    }    
    dist = 1.0 - smoothstep(0.0, AA_AMOUNT*40.0, dist);
    pixelColor = lerp(pixelColor, tint, dist * alpha);       
}

//============================================================
void DrawWheel (in float2 uv, in float2 wheelPos, inout float3 pixelColor, bool touchingGround)
{
	uint w, h;
	output.GetDimensions(w, h);
	float2 iResolution = float2(w, h);

    float3 wheelColor = float3(0.0, 0.0, 0.0);
    
    #if DEBUG_WHEELSTOUCHING
    if (touchingGround)
    	wheelColor = float3(0.5,1.0,0.5);    
    #endif
        
	float zoomCircleDist = UDCircle(uv, wheelPos, c_wheelRadius);
    zoomCircleDist = 1.0 - smoothstep(0.0, AA_AMOUNT, zoomCircleDist);
    pixelColor = lerp(pixelColor, wheelColor, zoomCircleDist);    
    
	zoomCircleDist = UDCircle(uv, wheelPos, c_wheelRadius*0.5);
    zoomCircleDist = 1.0 - smoothstep(0.0, AA_AMOUNT, zoomCircleDist);
    pixelColor = lerp(pixelColor, 0.75, zoomCircleDist);      
}

//============================================================
void DrawCar (in float2 uv, inout float3 pixelColor, float4 backWheel, float4 frontWheel, float4 state, float4 state2)
{
	uint w, h;
	output.GetDimensions(w, h);
	float2 iResolution = float2(w, h);

    // Draw the bike.  Note that we interpolate between last and
    // current simulation state, which makes the simulation look
    // smoother than it actually is!
   
    float2 backWheelPos = AsyncPointPos(backWheel, VAR_FRAME_PERCENT);
    float2 frontWheelPos = AsyncPointPos(frontWheel, VAR_FRAME_PERCENT);    
    
    // draw the wheels.
    DrawWheel(uv, backWheelPos, pixelColor, VAR_BACKWHEEL_ONGROUND == 1.0);
    DrawWheel(uv, frontWheelPos, pixelColor, VAR_FRONTWHEEL_ONGROUND == 1.0);       
    
    // draw the frame
    float2 carOrigin = backWheelPos;
    float2 xAxis = normalize(frontWheelPos - backWheelPos);
    float2 yAxis = float2(-xAxis.y, xAxis.x);
        
    float2 uvRelative = uv - carOrigin;
    float2 uvLocal;
    uvLocal.x = dot(uvRelative, xAxis);
    uvLocal.y = dot(uvRelative, yAxis);
    
#if 1
    float carDistance = UDFatLineSegment(uvLocal, float2(-c_wheelDistance*0.5, 0.04), float2(c_wheelDistance*1.6, 0.03), 0.035);
    carDistance = min(carDistance, UDFatLineSegment(uvLocal, float2(-0.06,0.04), float2(0.04,0.09), 0.01));
    carDistance = min(carDistance, UDFatLineSegment(uvLocal, float2(0.04,0.09), float2(0.08,0.09), 0.01));
    carDistance = min(carDistance, UDFatLineSegment(uvLocal, float2(0.08,0.09), float2(0.12,0.04), 0.01));
    carDistance -= 0.0025;    
	carDistance = 1.0 - smoothstep(0.0, AA_AMOUNT, carDistance);
    pixelColor = lerp(pixelColor, float3(0.1, 0.0, 0.0), carDistance);

#else
    float carDistance = UDFatLineSegment(uvLocal, float2(-c_wheelDistance*0.5, 0.04), float2(c_wheelDistance*2.0, 0.04), 0.05);
    carDistance = min (carDistance, UDFatLineSegment(uvLocal, float2(-c_wheelDistance*0.5, 0.09), float2(c_wheelDistance, 0.09), 0.05));
    carDistance = 1.0 - smoothstep(0.0, AA_AMOUNT, carDistance);
    pixelColor = lerp(pixelColor, float3(0.3,0.3,0.3), carDistance); 
#endif
    
}

//============================================================
void DrawGround (in float2 uv, in float2 cameraOffset, inout float3 pixelColor, float4 backWheel, float4 frontWheel, float4 state, float4 state2, float4 state3)
{
	uint w, h;
	output.GetDimensions(w, h);
	float2 iResolution = float2(w, h);

	float iTime = /*$(Variable:iTime)*/;

    // draw background layers
    DrawHills (uv + float2(1000.0, -0.3) + cameraOffset *-0.9 , pixelColor, 0.25, 0.7 );
    DrawClouds(uv + float2(1000.0, -0.3) + iTime * float2(0.05,0.0) + cameraOffset *-0.85, pixelColor, 0.3 , 0.75, 0.75);
    DrawHills (uv + float2(300.0 , -0.1) + cameraOffset  *-0.8 , pixelColor, 0.5 , 0.8 );
    DrawClouds(uv + float2(300.0 , -0.1) + iTime * float2(0.15,0.0) + cameraOffset  *-0.7 , pixelColor, 0.6 , 0.75, 0.75);
    
    // draw the car before the ridable layer so that trees and grass appear in front
    DrawCar(uv, pixelColor, backWheel, frontWheel, state, state2);

    // draw the ridable layer
 	DrawHills(uv, pixelColor, 1.0, 1.0);
    
    // draw the periodic fuel orbs
    if (uv.x > VAR_LASTFUELORB)
    {
    	float uvFuelX = mod(uv.x, c_fuelCanDistance) - c_fuelCanDistance * 0.5;
    	float uvFuelY = GroundHeightAtX(floor(uv.x / c_fuelCanDistance) * c_fuelCanDistance + c_fuelCanDistance * 0.5, 1.0);
    	uvFuelY += c_fuelCanRadius*1.1;
		float fuelDist = UDCircle(uv, float2(uvFuelX+uv.x, uvFuelY), c_fuelCanRadius*0.5);          
		fuelDist = 1.0 - smoothstep(0.0, AA_AMOUNT*10.0, fuelDist);
    	pixelColor = lerp(pixelColor, float3(1.0, 0.0, 0.0), fuelDist);
    }
    
    // draw some small foreground clouds
    DrawClouds(uv + float2(700.0 , -1.25) + iTime * float2(0.25,0.0) + cameraOffset * 0.5 , pixelColor, 1.0 , 1.0, 0.5);
}

//============================================================
void DrawSky (in float2 uv, in float2 cameraOffset, inout float3 pixelColor)
{
    float alpha = clamp(0.0,1.0,uv.y + cameraOffset.y * -0.9);
    alpha = smoothstep(0.0, 1.0, alpha);
    pixelColor = lerp(float3(0.25,0.6,1.0), float3(0.25,0.1,0.3), alpha);
}

//============================================================
void DrawDigit (float2 fragCoord, int digitValue, int digitIndex, inout float3 pixelColor)
{
    if (digitValue < 0)
        digitValue = 0;
    
	uint w, h;
	output.GetDimensions(w, h);
	float2 iResolution = float2(w, h);

    int indexX = int(fragCoord.x / SCORE_SIZE);
    int indexY = int((iResolution.y - fragCoord.y) / SCORE_SIZE);
    
    if (indexY > 0 || indexX != digitIndex)
        return;
    
    float2 percent = frac(float2(fragCoord.x,iResolution.y-fragCoord.y) / SCORE_SIZE);
    
    int x = int(percent.x * SCORE_SIZE / 2.0);
    int y = int(percent.y * SCORE_SIZE / 2.0);
    
    if (number(x,y,digitValue))
        pixelColor = 1.0f;
}

//============================================================
void DrawScore (float2 fragCoord, float score, inout float3 pixelColor)
{
    // keep score between 0000 and 9999
    score = clamp(score, 0.0, 9999.0);
    
    // digits numbered from right to left
    int digit0 = int(mod(score, 10.0));
    int digit1 = int(mod(score / 10.0, 10.0));
    int digit2 = int(mod(score / 100.0, 10.0));
    int digit3 = int(mod(score / 1000.0, 10.0));
    
    // digit index is from left to right though
    DrawDigit(fragCoord, digit0, 3, pixelColor);
    DrawDigit(fragCoord, digit1, 2, pixelColor);
    DrawDigit(fragCoord, digit2, 1, pixelColor);
    DrawDigit(fragCoord, digit3, 0, pixelColor);
}

//============================================================
void DrawSpeedometer (float2 fragCoord, float speedPercent, inout float3 pixelColor)
{
	uint w, h;
	output.GetDimensions(w, h);
	float2 iResolution = float2(w, h);

    float aspectRatio = iResolution.x / iResolution.y;
    float2 uv = (fragCoord / iResolution.xy) - 0.5;
    uv.x *= aspectRatio;
    
    const float size = 0.15;
    float2 center = float2(0.5 * aspectRatio - (size+AA_AMOUNT), -0.5 + (size+AA_AMOUNT));
    
    // early out if outside spedometer
    if (UDCircle(uv, center, size) > AA_AMOUNT)
        return;
    
    // yellow outer ring
	float zoomCircleDist = UDCircle(uv, center, size);  
	zoomCircleDist = 1.0 - smoothstep(0.0, AA_AMOUNT, zoomCircleDist);
    pixelColor = lerp(pixelColor, float3(0.8,0.6,0.0), zoomCircleDist);  
    
    // grey interior
	zoomCircleDist = UDCircle(uv, center, size - AA_AMOUNT);          
	zoomCircleDist = 1.0 - smoothstep(0.0, AA_AMOUNT, zoomCircleDist);
    pixelColor = lerp(pixelColor, 0.1, zoomCircleDist);
    
    // Tick marks
    float2 relativePoint = uv - center;
    float relativePointAngle = atan2(relativePoint.y, relativePoint.x);
    relativePointAngle += PI * 0.25;
    relativePointAngle = mod(relativePointAngle, TWOPI);
    if (relativePointAngle < PI * 1.5)
    {
    	float2 fakePoint = float2(length(relativePoint) / size, relativePointAngle);
    	fakePoint.y = mod(fakePoint.y, 0.4) - 0.2;
        float tickDistance = UDFatLineSegment(fakePoint, float2(0.85, 0.0), float2(0.95, 0.0), 0.05);
        tickDistance = 1.0 - smoothstep(0.0, AA_AMOUNT*5.0, tickDistance);
        pixelColor = lerp(pixelColor, float3(1.0,1.0,0.0), tickDistance);
    }
    
    // speed bar
    float targetAngle = (1.0 - clamp(speedPercent, 0.0, 1.0)) * PI * 1.5 - PI * 0.25;
    float2 targetPoint = center + size * 0.9 * float2(cos(targetAngle), sin(targetAngle));
        
    float boxDistance = UDFatLineSegment(uv, center, targetPoint , 0.003);
    boxDistance = 1.0 - smoothstep(0.0, AA_AMOUNT, boxDistance);
    pixelColor = lerp(pixelColor, float3(1.0,0.0,0.0), boxDistance);
    
    // red ring in the middle, attached to the bar
	zoomCircleDist = UDCircle(uv, center, AA_AMOUNT);          
	zoomCircleDist = 1.0 - smoothstep(0.0, AA_AMOUNT, zoomCircleDist);
    pixelColor = lerp(pixelColor, float3(1.0,0.0,0.0), zoomCircleDist);        
}

//============================================================
void DrawFuelBar(float2 fragCoord, float fuelPercent, inout float3 pixelColor)
{
	uint w, h;
	output.GetDimensions(w, h);
	float2 iResolution = float2(w, h);

    fuelPercent = min(fuelPercent, 1.0);
    float aspectRatio = iResolution.x / iResolution.y;
    float2 uv = (fragCoord / iResolution.xy) - 0.5;
    uv.x *= aspectRatio;    
    
    const float c_width = 0.2;
    const float c_height = 0.05;
    
    float2 boxPosLeft = float2(-0.5 * aspectRatio + 0.01, 0.5 - (c_height + SCORE_SIZE / iResolution.y));
    float2 boxPosRight = float2(-0.5 * aspectRatio + 0.01 + c_width, 0.5 - (c_height + SCORE_SIZE / iResolution.y));
    
    // black outer box
    float boxDistance = UDFatLineSegment(uv, boxPosLeft, boxPosRight, c_height);
    boxDistance = 1.0 - smoothstep(0.0, AA_AMOUNT, boxDistance);
    pixelColor = lerp(pixelColor, float3(0.0,0.0,0.0), boxDistance);
    
    // red fuel amount
    if (fuelPercent > 0.0)
    {
        boxPosRight.x = boxPosLeft.x + (boxPosRight.x - boxPosLeft.x) * fuelPercent;
        boxDistance = UDFatLineSegment(uv, boxPosLeft, boxPosRight, c_height);
        boxDistance = 1.0 - smoothstep(0.0, AA_AMOUNT, boxDistance);
        pixelColor = lerp(pixelColor, float3(1.0,0.0,0.0), boxDistance);   
    }
}

/*$(_compute:CSDisplay)*/(uint3 DTid : SV_DispatchThreadID)
{
    //----- Load State -----    
	Struct_GameState s = gameState[0];
    float4 state         = s.state;//loadValue(txState);
    float4 state2        = s.state2;//loadValue(txState2);
    float4 state3        = s.state3;//loadValue(txState3);
    float4 backWheel     = s.backWheel;//loadValue(txBackWheel);   
    float4 frontWheel    = s.frontWheel;//loadValue(txFrontWheel);  

	uint w, h;
	output.GetDimensions(w, h);
	float2 iResolution = float2(w, h);

	float2 fragCoord = float2(DTid.xy);
	fragCoord.y = iResolution.y - fragCoord.y;

    // calculate coordinates based on camera settings
    float aspectRatio = iResolution.x / iResolution.y;
    float2 uv = (fragCoord / iResolution.xy) - 0.5;
    uv.x *= aspectRatio;
    uv *= VAR_CAMERA_SCALE;
    uv += VAR_CAMERA_CENTER;
    
    // draw the sky
    float3 pixelColor = float3(0.0, 0.0, 0.0);
    DrawSky(uv, VAR_CAMERA_CENTER, pixelColor);
    
    // draw the ground
    DrawGround(uv, VAR_CAMERA_CENTER, pixelColor, backWheel, frontWheel, state, state2, state3);
    
    // Draw UI
    DrawScore(fragCoord, VAR_CAMERA_CENTER.x, pixelColor);
    DrawSpeedometer(fragCoord, VAR_SPEDOMETER, pixelColor);
    DrawFuelBar(fragCoord, VAR_FUELREMAINING, pixelColor);
    
    // if game over, mix it towards red a bit
    if (VAR_GAMEOVER == 1.0)
    {
        float3 greyPixel = float3(dot(pixelColor, float3(0.3, 0.59, 0.11)).xxx);
        pixelColor = lerp(float3(1.0,0.0,0.0), greyPixel, VAR_SIMSLOWDOWN * 0.75 + 0.25);    
    }
    
    #if DEBUG_FUELCOLLIDE
    float2 bikePos = AsyncBikePos(backWheel, frontWheel, VAR_FRAME_PERCENT);
    if (length(bikePos - uv) < c_fuelCanRadius * 2.0)
        pixelColor = lerp(pixelColor, float3(1.0, 1.0, 0.0), 0.25);
    #endif
    
    // output the final color
	output[DTid.xy] = float4(pixelColor,1.0);	
}

/*
Shader Resources:
	Buffer gameState (as SRV)
	Texture output (as UAV)
*/
