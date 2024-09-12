/*$(ShaderResources)*/

#include "BattleConstants.hlsli"

float random(float2 st)
{
    return frac(sin(dot(st.xy, float2(12.9898,78.233)))*43758.5453123);
}

float2 wrapCoordinates(float2 c)
{
    return frac(c * 0.5 + 0.5) * 2 - 1;
}

float2 getForwardVector(float rotation)
{
    return float2(cos(rotation), sin(rotation));
}

float getRotation(float2 v)
{
    return atan2(v.y, v.x);
}

bool isLaserReady(Struct_EntityState state)
{
    const float iTime = /*$(Variable:iTime)*/;
    const float LaserRateOfFire = /*$(Variable:LaserRateOfFire)*/;

    return (iTime - state.LastLaserFireTime) > (1.0f / LaserRateOfFire);
}

#define MAX_CLOSEST 16
Struct_EntityState GetClosestEntities(const Struct_EntityState state, out int foundCount, float radius, bool onlyEnemies = false)[MAX_CLOSEST]
{
    const uint TotalEntities = /*$(Variable:EntityCount)*/;

    foundCount = 0;
    float distances[MAX_CLOSEST];

    for (int i = 0; i < MAX_CLOSEST; i++)
        distances[i] = -1.0f;

    Struct_EntityState otherEntities[MAX_CLOSEST];

    for (int e = 0; e < TotalEntities; e++)
    {
        Struct_EntityState otherEntity = EntityStates[e];

        if (otherEntity.IsAlive && otherEntity.Id != state.Id)
        {
            if (otherEntity.Team == state.Team && onlyEnemies)
                continue;

            float2 direction = state.Position - otherEntity.Position;
            float distance2 = dot(direction, direction);

            for(int i = 0; i < MAX_CLOSEST; i++)
            {
                if (distances[i] < 0.0f)
                {
                    otherEntities[i] = otherEntity;
                    distances[i] = distance2;
                }
                else if (distance2 < distances[i])
                {
                    for (int o = (foundCount - 1); o > i; o--)
                    {
                        // Shift others down
                        otherEntities[o] = otherEntities[o - 1];
                        distances[o] = distances[o - 1];
                    }
                    // Insert new entity
                    otherEntities[i] = otherEntity;
                    distances[i] = distance2;
                    foundCount = min(foundCount + 1, MAX_CLOSEST);
                    break; // Exit the loop after insertion
                }
            }
        }
    }

    return otherEntities;
}

/*$(_compute:simSpawn)*/(uint3 DTid : SV_DispatchThreadID)
{
    const uint iFrame = /*$(Variable:iFrame)*/;
    const uint iTime = /*$(Variable:iTime)*/;
    const uint TeamCount = /*$(Variable:TeamCount)*/;
    const float LaserRateOfFire = /*$(Variable:LaserRateOfFire)*/;

    const uint entityIdx = DTid.x;

    Struct_EntityState state = EntityStates[entityIdx];

    if (iFrame == 0)
    {
        // Initialize entity
        state.Id = entityIdx;
        state.IsAlive = false;
        state.Team = entityIdx % TeamCount;
        state.Lives = 100;

        // Set death time to respawn time so we spawn immediately
        state.DeathTime = -RespawnTime;
    }
    
    if (!state.IsAlive && state.Lives > 0 && (iTime - state.DeathTime) > RespawnTime)
    {
        // Spawn entity
        state.Position = float2(random(float2(state.Id, iFrame)), random(float2(state.Id, iFrame + 5))) * 2.0f - 1.0f;
        state.Rotation = random(float2(state.Id, iFrame + 2)) * 3.1415 * 2;
        state.Size = 0.015f;
        state.IsAlive = true;
        state.SpawnTime = iTime;

        // Randomize the laser recharge time a bit so lasers don't all fire at once
        float FireDelay = 1 / LaserRateOfFire;
        state.LastLaserFireTime = iTime - random(float2(state.Id, iFrame + 3)) * FireDelay + RespawnInvincibilityTime;
        state.LaserStart = sqrt(-1); // Nan initial target
        state.LaserEnd = sqrt(-1); // Nan initial target

        state.Lives--;
    }

    EntityStates[entityIdx] = state;
}

/*$(_compute:simMove)*/(uint3 DTid : SV_DispatchThreadID)
{
    const float iTime = /*$(Variable:iTime)*/;
    const float PI = 3.14159265f;

    const uint entityIdx = DTid.x;

    Struct_EntityState state = EntityStates[entityIdx];

    if (state.IsAlive)
    {
        const float iTimeDelta = /*$(Variable:iTimeDelta)*/;
        const float2 iResolution = /*$(Variable:iResolution)*/.xy;
        const float4 MouseState = /*$(Variable:MouseState)*/;

        uint numOtherEntities;
        Struct_EntityState closestEntities[MAX_CLOSEST] = GetClosestEntities(state, numOtherEntities, ViewRange, false);

        // Get desired direction
        float2 ourForward = getForwardVector(state.Rotation);

        // Find the best target
        int targetIdx = -1;
        float targetScore = -1000;
        if (isLaserReady(state))
        {
            for (int i = 0; i < numOtherEntities; i++)
            {
                Struct_EntityState otherState = closestEntities[i];

                if (otherState.Team == state.Team)
                    continue; // Ignore same team

                float2 otherVector = otherState.Position - state.Position;
                float otherDistance = length(otherVector);
                float2 otherDirection = otherVector / otherDistance;

                float targetRange = LaserRange;

                float otherAngle = dot(otherDirection, ourForward) * 0.5 + 0.5;
                float otherTargetScore = otherAngle * 2 - saturate(otherDistance / targetRange);

                if (otherTargetScore > targetScore)
                {
                    targetIdx = i;
                    targetScore = otherTargetScore;
                }
            }
        }

        float2 desiredDirection = ourForward * 0.1f;

        // Turn to fire at the target
        if (targetIdx >= 0)
        {
            Struct_EntityState otherState = closestEntities[targetIdx];
            float2 otherVector = otherState.Position - state.Position;
            float otherDistance = length(otherVector);
            float2 otherDirection = otherVector / otherDistance;

            desiredDirection += otherDirection * 4;
        }

        for (int i = 0; i < numOtherEntities; i++)
        {
            Struct_EntityState otherState = closestEntities[i];
            float2 otherVector = otherState.Position - state.Position;
            float otherDistance = length(otherVector);
            float2 otherDirection = otherVector / otherDistance;

            float viewAngle = 0.25;
            float otherAngle = dot(otherDirection, ourForward);
            float normalizedOtherAngle = otherAngle * 0.5f + 0.5f;
            
            if (normalizedOtherAngle > viewAngle) 
            {
                // Avoid hitting other ships
                float avoidRange = ViewRange / 3;
                float weight = smoothstep(1, 0, otherDistance / avoidRange);
                desiredDirection -= otherDirection * weight;

                // Alignment and cohesion with allies
                if (otherState.Team == state.Team)
                {
                    float alignmentRange = ViewRange / 2;
                    float alignmentWeight = smoothstep(1, 0, otherDistance / alignmentRange) * 0.5;
                    desiredDirection += alignmentWeight * getForwardVector(otherState.Rotation);

                    float cohesionRange = ViewRange;
                    float cohesionWeight = smoothstep(1, 0, otherDistance / cohesionRange) * 0.2;
                    desiredDirection += cohesionWeight * otherDirection;
                }  
            }
        }

        // Avoid hitting walls
        {
            float avoidanceGradientWidth = 0.1f; // How soft is the correction from the wall
            float wallPosition = 1.0; // How far from origin are the walls

            float2 wallDistances = wallPosition - abs(state.Position);
            float2 wallCorrection = -sign(state.Position) * saturate(1 - (wallDistances / avoidanceGradientWidth));
            desiredDirection += wallCorrection;
        }

        // Turn towards desired direction
        if (any(desiredDirection != float2(0,0)))
        {
            float rotationDiff = getRotation(desiredDirection) - state.Rotation;
            if (rotationDiff > PI)
                rotationDiff -= 2 * PI;
            else if (rotationDiff <= -PI)
                rotationDiff += 2 * PI;

            state.Rotation += rotationDiff * ShipMaxTurnRate * iTimeDelta;
        }

        // Move forward at speed
        state.Position += getForwardVector(state.Rotation) * iTimeDelta * ShipSpeed;
        EntityStates[entityIdx] = state;
    }
}

/*$(_compute:simShoot)*/(uint3 DTid : SV_DispatchThreadID)
{
    const float iTime = /*$(Variable:iTime)*/;

    const uint entityIdx = DTid.x;

    Struct_EntityState state = EntityStates[entityIdx];

    if (state.IsAlive)
    {
        uint numOtherEntities;
        Struct_EntityState closestEntities[MAX_CLOSEST] = GetClosestEntities(state, numOtherEntities, LaserRange, true);

        // Fire Laser at enemies
        bool spawning = (iTime - state.SpawnTime) < RespawnInvincibilityTime;

        if (isLaserReady(state) && !spawning)
        {
            for (int i = 0; i < numOtherEntities; i++)
            {
                if (closestEntities[i].Team != state.Team)
                {
                    float2 enemyDirection = closestEntities[i].Position - state.Position;
                    
                    float angle = dot(getForwardVector(state.Rotation), normalize(enemyDirection));
                    bool isInvincible = (iTime - closestEntities[i].SpawnTime) < RespawnInvincibilityTime;

                    // Fire Laser
                    if (angle > LaserAngle && length(enemyDirection) < LaserRange && !isInvincible)
                    {
                        state.LaserStart = state.Position;
                        state.LaserEnd = closestEntities[i].Position;
                        state.LastLaserFireTime = iTime;

                        EntityStates[entityIdx] = state;
                        return;
                    }
                }
            }
        }
        
    }
}

/*$(_compute:simDie)*/(uint3 DTid : SV_DispatchThreadID)
{
    const float iTime = /*$(Variable:iTime)*/;
    const uint entityIdx = DTid.x;

    Struct_EntityState state = EntityStates[entityIdx];

    uint numOtherEntities;
    Struct_EntityState closestEntities[MAX_CLOSEST] = GetClosestEntities(state, numOtherEntities, 1, true);

    for (int i = 0; i < numOtherEntities; i++)
    {
        float LaserDistance = length(closestEntities[i].LaserEnd - state.Position);
        bool LaserValid = iTime - closestEntities[i].LastLaserFireTime < LaserFadeTime;

        // Die from enemy laser
        if (LaserDistance < state.Size && LaserValid)
        {
            state.IsAlive = false;
            state.DeathTime = iTime;

            EntityStates[entityIdx] = state;
            return;
        }
    }

}