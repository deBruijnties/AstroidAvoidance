#include "DuckBounce.h"
#include <Core/engine.h>
#include <Core/Scene/Gameobject.h>
#include <Core/Scene/Components/Transform.h>

// Creates a looping 0–1 ping-pong value
float PingPong(float t)
{
    return 1.0f - std::abs(std::fmod(t, 2.0f) - 1.0f);
}

// Generates a repeating 2D movement pattern (DVD-style bounce)
Vector2 GetDvdPos(float time)
{
    float sx = 1.0f;
    float sz = 1.37f;

    float x = PingPong(time * sx) * 0.7f;
    float z = PingPong(time * sz) * 0.3f;

    return { x, z };
}

// Simple wave function for water surface height
float SampleWaterHeight(float worldX, float worldZ, float t)
{
    return sin(worldX * 5.0f + t) * 0.01f
        + cos(worldZ * 5.0f + t) * 0.01f;
}

float val;

void DuckBounce::OnUpdate()
{
    // Moving position over water surface
    Vector2 dvdPos = GetDvdPos(Time::timeSinceStartup * 0.03f) + Vector2(0.7f, 1.1f);

    float t = Time::timeSinceStartup;

    // Sample wave height at duck position
    float waterHeight = SampleWaterHeight(dvdPos.x, dvdPos.y, t);

    float waterBaseY = 1.440f;
    float duckY = waterBaseY + waterHeight;

    // Apply position and rotation
    transform->localPosition = Vector3(dvdPos.x, duckY, dvdPos.y);

    transform->localRotation =
        Quaternion::FromEuler(Math::Radians(Vector3(0, Time::timeSinceStartup * 18, 0)));

    transform->MarkDirty();
}