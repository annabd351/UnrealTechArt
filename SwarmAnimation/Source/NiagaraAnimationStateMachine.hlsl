// 
// This is a chunk of HLSL from the Niagara system which animates the vermin in the swarm.  
// It implements a simple state machine, and controls the blending between pre-baked animation cycles
// (very similar to how a one-dimensional blend space works).
// 
// Variables which aren't defined are the inputs/outputs to/from the custom HLSL node containing this code.
//

#define Idle 0
#define Run 1
#define Search 2
#define Walk 3
#define Undefined 4

#define SMALL_NUMBER (float) 1e-8
#define UNDEFINED_VELOCITY float3(0.0, 0.0, 0.0)

// Process state changes, and blend animation cycles.

// Note that this code is intended to be non-branching (best for GPUs), so there are computations which may appear unnecessary
// or out of order vs. typical sequential (CPU) code.

AnimCycleChangeCooldown = max(AnimCycleChangeCooldown, SMALL_NUMBER);
BlendTime = max(BlendTime, SMALL_NUMBER);

// Default to same state.  Then, find potential new states.  Later, decide to transition (or not).
int newState = AnimCycleInt;
float3 newVelocity = Velocity;

AnimCycleIntOut = AnimCycleInt;
VelocityOut = Velocity;
PrevAnimCycleIntOut = PrevAnimCycleInt;
BlendingInProgressOut = BlendingInProgress;
BlendTimerStartOut = BlendTimerStart;
CooldownTimerStartOut = CooldownTimerStart;


// Velocity-induced state change
const float speed = length(Velocity);
const bool isStationary = speed <= StationarySpeedThreshold;
const bool shouldWalk = speed <= RunSpeedThreshold && isStationary == false;
const bool shouldRun = speed > RunSpeedThreshold;

// Random state changes
const int randomStationaryState = RandA <= saturate(IdleVsSearchChance) ? Idle : Search;
const bool shouldStartMoving = RandB <= saturate(StationaryToMovingChance);
const bool shouldStopMoving = RandC <= saturate(MovingToStationaryChance);
const int newMovingState = length(LastMovingVelocity) < RunSpeedThreshold ? Walk : Run;

// "Stationary" velocity -- zero velocity would break orientation calculations
const float3 verySmallVelocity = LastValidDirection * SMALL_NUMBER;

[flatten]
switch(AnimCycleInt)
{
    case Idle:
    case Search:
        newState = shouldStartMoving ? newMovingState : AnimCycleInt;
        newVelocity = shouldStartMoving ? LastMovingVelocity : Velocity;
        break;

    case Run:
    case Walk:
        newState = shouldStopMoving ? randomStationaryState : AnimCycleInt;
        newVelocity = shouldStopMoving ? verySmallVelocity : Velocity;
        break;

    default:
        newState = AnimCycleInt;
        newVelocity = Velocity;
        break;
}

[flatten]
if (newState == AnimCycleInt)
{
    switch (AnimCycleInt) 
    {
        case Idle:
        case Search:
            newState = shouldWalk ? Walk : (shouldRun ? Run : AnimCycleInt);
            break;

        case Run:
        case Walk:
            newState = isStationary ? randomStationaryState : (shouldWalk ? Walk : Run);
            break;

        default:
            newState = AnimCycleInt;
            break;
    }
}


// Calculate blend weights based on current state
const float elapsedBlendFraction = saturate((Time - CooldownTimerStart)/BlendTime);
const float remainingBlendFraction = 1.0 - elapsedBlendFraction;

const float smoothElapsedBlendFraction = smoothstep(0.0, 1.0, elapsedBlendFraction);
const float smoothRemainingBlendFraction = smoothstep(0.0, 1.0, remainingBlendFraction);

[flatten]
if (AnimCycleInt != PrevAnimCycleInt && remainingBlendFraction > 0.0) 
{
    BlendIdle = PrevAnimCycleInt == Idle ? smoothRemainingBlendFraction : (AnimCycleInt == Idle ? smoothElapsedBlendFraction : 0.0);
    BlendRun = PrevAnimCycleInt == Run ? smoothRemainingBlendFraction : (AnimCycleInt == Run ? smoothElapsedBlendFraction : 0.0);
    BlendSearch = PrevAnimCycleInt == Search ? smoothRemainingBlendFraction : (AnimCycleInt == Search ? smoothElapsedBlendFraction : 0.0);
    BlendWalk = PrevAnimCycleInt == Walk ? smoothRemainingBlendFraction : (AnimCycleInt == Walk ? smoothElapsedBlendFraction : 0.0);
}
else
{
    BlendIdle = AnimCycleInt == Idle ? 1.0 : 0.0;
    BlendRun = AnimCycleInt == Run ? 1.0 : 0.0;
    BlendSearch = AnimCycleInt == Search ? 1.0 : 0.0;
    BlendWalk = AnimCycleInt == Walk ? 1.0 : 0.0;
}


// Now, change states if possible
const bool coolingDown = Time - CooldownTimerStart < AnimCycleChangeCooldown;

[flatten]
if (coolingDown == false && AnimCycleInt != newState)
{ 
    AnimCycleIntOut = newState;
    VelocityOut = newVelocity;

    PrevAnimCycleIntOut = AnimCycleInt;
    CooldownTimerStartOut = Time;
}


// Smooth velocity
VelocityOut = lerp(Velocity, VelocityOut, saturate(VelocityConvergenceRate * DeltaTime));

// Min velocity required -- zero velocities break the sim
VelocityOut = length(VelocityOut) > 0.0 ? VelocityOut : verySmallVelocity;

// Save velocity and direction for later
const float speedOut = length(VelocityOut);
LastMovingVelocityOut = speedOut > StationarySpeedThreshold ? VelocityOut : LastMovingVelocityOut;
LastValidDirectionOut = speedOut > 0.0 ? normalize(VelocityOut) : LastValidDirection;
