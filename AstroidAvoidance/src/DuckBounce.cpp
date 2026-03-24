#include "DuckBounce.h"
#include <Core/engine.h>
#include <Core/Scene/Gameobject.h>
#include <Core/Scene/Components/Transform.h>

float PingPong(float t)
{
    return 1.0f - std::abs(std::fmod(t, 2.0f) - 1.0f);
}

Vector2 GetDvdPos(float time)
{
    float sx = 1.0f;
    float sz = 1.37f;

    float x = PingPong(time * sx) * 0.7f; // 0 - 0.7
    float z = PingPong(time * sz) * 0.3f; // 0 - 0.3

    return { x, z };
}

float SampleWaterHeight(float worldX, float worldZ, float t)
{
    return sin(worldX * 5.0f + t) * 0.01f
        + cos(worldZ * 5.0f + t) * 0.01f;
}
float val;

void DuckBounce::OnUpdate()
{
    Vector2 dvdPos = GetDvdPos(Time::timeSinceStartup * 0.03f) + Vector2(0.7f, 1.1f);

    // Sample wave at duck position (WORLD SPACE!)
    float t = Time::timeSinceStartup;
    float waterHeight = SampleWaterHeight(dvdPos.x, dvdPos.y, t);

    // Base water plane height in world space
    float waterBaseY = 1.440f;

    // Final duck height
    float duckY = waterBaseY + waterHeight;

    // Apply position
    transform->localPosition =
        Vector3(dvdPos.x, duckY, dvdPos.y);

    transform->localRotation =
        Quaternion::FromEuler(Math::Radians((Vector3(0, Time::timeSinceStartup * 18, 0))));

    transform->MarkDirty();
}
