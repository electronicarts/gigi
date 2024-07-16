#define PI 3.14159265359
#define PIOVERTWO (PI * 0.5)
#define TWOPI (PI * 2.0)

static const float c_wheelRadius = 0.04;
static const float c_wheelDistance = 0.125;

static const float c_fuelCanDistance = 20.0;
static const float c_fuelCanRadius = 0.075;

// variables
static const float2 txState = float2(0.0,0.0);
// x = timer to handle fixed rate gameplay
// y = queued input.  0.0 = left, 1.0 = right, 0.5 = none
// zw = camera center
#define VAR_FRAME_PERCENT state.x
#define VAR_QUEUED_INPUT state.y
#define VAR_CAMERA_CENTER state.zw
static const float2 txState2 = float2(1.0,0.0);
// x = camera scale
// y = back wheel is on the ground (1.0 or 0.0)
// z = front wheel is on the ground (1.0 or 0.0)
// w = game is over (1.0) or not (0.0)
#define VAR_CAMERA_SCALE state2.x
#define VAR_BACKWHEEL_ONGROUND state2.y
#define VAR_FRONTWHEEL_ONGROUND state2.z
#define VAR_GAMEOVER state2.w
static const float2 txState3 = float2(2.0,0.0);
// x = used to slowdown simulation only right when you hit game over state
// y = last collected fuel orb distance
// z = spedometer
// w = fuel remaining
#define VAR_SIMSLOWDOWN state3.x
#define VAR_LASTFUELORB state3.y
#define VAR_SPEDOMETER state3.z
#define VAR_FUELREMAINING state3.w
// these are used by check points.  We always restore to the older check point so
// the player doesn't get stuck in a shitty check point.
static const float2 txFrontWheelCP1 = float2(3.0,0.0);
static const float2 txFrontWheelCP2 = float2(4.0,0.0);
static const float2 txBackWheelCP1 = float2(5.0,0.0);
static const float2 txBackWheelCP2 = float2(6.0,0.0);
static const float2 txState4 = float2(7.0,0.0);
// x = fuel at CP1
// y = fuel at CP2
// z = last CP hit
// w = unused
#define VAR_FUELREMAININGCP1 state4.x
#define VAR_FUELREMAININGCP2 state4.y
#define VAR_LASTCPHIT state4.w

// simulated points
// format: xy = location this frame. zw = location last frame
static const float2 txBackWheel = float2(8.0, 0.0);
static const float2 txFrontWheel = float2(9.0, 0.0);

static const float2 txVariableArea = float2(10.0, 1.0);

float GroundHeightAtX (float x, float scale)
{
    
    //return 0.0;
    
    /*
    float frequency = 2.0 * frequencyScale;
    float amplitude = 0.1 * scale;
    return sin(x*frequency) * amplitude +
           sin(x*frequency*2.0) * amplitude / 2.0
           + sin(x*frequency*3.0) * amplitude / 3.0
           + sin(x*1.0) * amplitude * 5.0;
    */
    
    #define ADDWAVE(frequency, start, easein, amplitude, scalarFrequency) ret += sin(x * frequency) * clamp((x-start)/easein, 0.0, 1.0) * amplitude * (sin(x*scalarFrequency) * 0.5 + 0.5);
    
    x *= scale;
    
    // add several sine waves together to make the terrain
    // frequency and amplitudes increase over distance    
    float ret = 0.0;
    
    // have a low frequency, low amplitude sine wave
    ADDWAVE(0.634, 0.0, 0.001, 0.55, 0.1);
    
    // a slightly higher frequency adds in amplitude over time
    ADDWAVE(1.0, 0.0, 50.0, 0.5, 0.37);
    
    // at 75 units in, start adding in a higher frequency, lower amplitude wave
    ADDWAVE(3.17, 75.0, 50.0, 0.1, 0.054); 
    
    // at 150 units, add in higher frequency waves
    ADDWAVE(9.17, 150.0, 50.0, 0.05, 0.005);
    
    // at 225 units, add another low frequency, medium amplitude sine wave
    ADDWAVE(0.3, 225.0, 10.0, 0.9, 0.01);    
    
    // add an explicit envelope to the starting area
    ret *= smoothstep(x / 2.0, 0.0, 1.0);
    
    return ret * scale;  
}

float GroundFunction (float2 p, float scale)
{
    return GroundHeightAtX(p.x, scale) - p.y;
}

float2 AsyncPointPos (in float4 p, in float frameFraction)
{
    return lerp(p.zw, p.xy, frameFraction);
}

float2 AsyncBikePos (in float4 backWheel, in float4 frontWheel, in float frameFraction)
{
    return (AsyncPointPos(backWheel, frameFraction)+AsyncPointPos(frontWheel, frameFraction)) * 0.5;
}

float2 GroundFunctionGradiant (in float2 coords, float scale)
{
    float2 h = float2( 0.01, 0.0 );
    return float2( GroundFunction(coords+h.xy, scale) - GroundFunction(coords-h.xy, scale),
                 GroundFunction(coords+h.yx, scale) - GroundFunction(coords-h.yx, scale) ) / (2.0*h.x);
}

float EstimatedDistanceFromPointToGround (in float2 p, float scale)
{
    float v = GroundFunction(p, scale);
    float2  g = GroundFunctionGradiant(p, scale);
    return v/length(g);
}

float EstimatedDistanceFromPointToGround (in float2 p, float scale, float frequencyScale, out float2 gradient)
{
    float v = GroundFunction(p, scale);
    gradient = GroundFunctionGradiant(p, scale);
    return v/length(gradient);
}

float mod(float x, float y)
{
	return x - y * floor(x/y);
}
