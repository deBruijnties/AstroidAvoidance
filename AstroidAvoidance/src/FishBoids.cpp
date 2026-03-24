#include <core/Scene/Components/Transform.h>
#include "FishBoids.h"
#include <core/Time.h>
#include <cstdlib>
#include <cmath>

//random float
static float RandF(float lo, float hi)
{
    return lo + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (hi - lo);
}

static Vector3 RandInsideSphere(float radius)
{
    Vector3 p;
    do {
        p = { RandF(-1.f, 1.f), RandF(-1.f, 1.f), RandF(-1.f, 1.f) };
    } while (Math::Dot(p, p) > 1.0f);
    return p * radius;
}

Vector3 FishBoids::ClampMagnitude(const Vector3& v, float maxLen)
{
    float sqLen = Math::Dot(v, v);
    if (sqLen > maxLen * maxLen && sqLen > 1e-12f)
        return v * (maxLen / sqrtf(sqLen));
    return v;
}

Vector3 FishBoids::Steer(const Vector3& velocity, const Vector3& desired,
    float maxSpeed, float maxForce)
{
    float dLen = Math::Length(desired);
    if (dLen < 1e-6f) return { 0, 0, 0 };
    Vector3 desiredScaled = desired * (maxSpeed / dLen);
    return ClampMagnitude(desiredScaled - velocity, maxForce);
}


Vector3 FishBoids::CalcSeparation(const Fish& self) const
{
    Vector3 steer{ 0, 0, 0 };
    int     count = 0;

    for (const auto& other : m_fish)
    {
        if (other.id == self.id) continue;

        Vector3 diff = self.position - other.position;
        float   dist = Math::Length(diff);

        if (dist > 0.001f && dist < separationRadius)
        {
            steer = steer + (diff / (dist * dist));
            ++count;
        }
    }

    if (count > 0)
        return Steer(self.velocity, steer, maxSpeed, maxSteerForce);

    return { 0, 0, 0 };
}

Vector3 FishBoids::CalcAlignment(const Fish& self) const
{
    Vector3 avgVel{ 0, 0, 0 };
    int     count = 0;

    for (const auto& other : m_fish)
    {
        if (other.id == self.id) continue;

        if (Math::Length(other.position - self.position) < alignmentRadius)
        {
            avgVel = avgVel + other.velocity;
            ++count;
        }
    }

    if (count > 0)
        return Steer(self.velocity, avgVel * (1.0f / count), maxSpeed, maxSteerForce);

    return { 0, 0, 0 };
}

Vector3 FishBoids::CalcCohesion(const Fish& self) const
{
    Vector3 center{ 0, 0, 0 };
    int     count = 0;

    for (const auto& other : m_fish)
    {
        if (other.id == self.id) continue;

        if (Math::Length(other.position - self.position) < cohesionRadius)
        {
            center = center + other.position;
            ++count;
        }
    }

    if (count > 0)
    {
        Vector3 desired = (center * (1.0f / count)) - self.position;
        return Steer(self.velocity, desired, maxSpeed, maxSteerForce);
    }

    return { 0, 0, 0 };
}

Vector3 FishBoids::CalcBoundary(const Fish& self,
    const Vector3& sMin, const Vector3& sMax,
    float margin) const
{
    Vector3 steer{ 0, 0, 0 };

    auto AxisForce = [&](float pos, float lo, float hi) -> float
        {
            float distLo = pos - lo;
            float distHi = hi - pos;

            if (distLo < margin)
                return maxSpeed * (1.0f - distLo / margin);
            if (distHi < margin)
                return -maxSpeed * (1.0f - distHi / margin);
            return 0.0f;
        };

    steer.x = AxisForce(self.position.x, sMin.x, sMax.x);
    steer.y = AxisForce(self.position.y, sMin.y, sMax.y);
    steer.z = AxisForce(self.position.z, sMin.z, sMax.z);

    if (Math::Length(steer) < 1e-6f)
        return { 0, 0, 0 };

    return Steer(self.velocity, steer, maxSpeed, maxSteerForce);
}


void FishBoids::OnStart()
{
    m_fish.clear();
    m_fish.reserve(fishCount);

    for (int i = 0; i < fishCount; ++i)
    {
        Fish f{};
        f.id = m_nextId++;
        f.phase = RandF(0.0f, Math::TWO_PI);
        f.wanderAngle = RandF(0.0f, Math::TWO_PI);
        f.position = RandInsideSphere(spawnRadius);

        Vector3 dir = {
            RandF(-1.f,  1.f),
            RandF(-0.3f, 0.3f),
            RandF(-1.f,  1.f)
        };
        float len = Math::Length(dir);
        if (len > 1e-6f) dir = dir / len;
        f.velocity = dir * RandF(minSpeed, maxSpeed);

        m_fish.push_back(f);
    }

    m_instanceBuffer = new StructuredBuffer(INSTANCE_BINDING, sizeof(Matrix4));
    m_instanceBuffer->Allocate(m_fish.size());
}

void FishBoids::OnUpdate()
{
    const float dt = Time::deltaTime;

    Vector3    worldPos;
    Quaternion worldRot;
    Vector3    worldScale;
    Math::DecomposeMatrix(transform->worldMatrix, worldPos, worldRot, worldScale);

    const Vector3 scaledMin = boundsMin * worldScale;
    const Vector3 scaledMax = boundsMax * worldScale;
    const float   scaledMargin = boundaryMargin * Math::Length(worldScale) / 1.732f;


    struct SteeringResult { Vector3 accel; };
    static std::vector<SteeringResult> s_steering;
    s_steering.resize(m_fish.size());

    for (size_t i = 0; i < m_fish.size(); ++i)
    {
        Fish& f = m_fish[i];

        f.wanderAngle += RandF(-1.5f, 1.5f) * dt;
        Vector3 forward = Math::Length(f.velocity) > 1e-6f
            ? f.velocity * (1.0f / Math::Length(f.velocity))
            : Vector3(0, 0, 1);
        Vector3 right = Math::Normalize(Math::Cross(forward, Vector3(0, 1, 0)));
        if (Math::Length(right) < 1e-6f)
            right = { 1, 0, 0 };
        Vector3 up = Math::Cross(right, forward);
        Vector3 wanderTarget = forward
            + right * cosf(f.wanderAngle)
            + up * sinf(f.wanderAngle);

        Vector3 sep = CalcSeparation(f) * separationWeight;
        Vector3 ali = CalcAlignment(f) * alignmentWeight;
        Vector3 coh = CalcCohesion(f) * cohesionWeight;
        Vector3 bnd = CalcBoundary(f, scaledMin, scaledMax, scaledMargin) * boundaryWeight;
        Vector3 wander = Steer(f.velocity, wanderTarget, maxSpeed, maxSteerForce) * wanderWeight;

        s_steering[i].accel = ClampMagnitude(sep + ali + coh + bnd + wander, maxSteerForce);
    }


    for (size_t i = 0; i < m_fish.size(); ++i)
    {
        Fish& f = m_fish[i];

        f.velocity = f.velocity + s_steering[i].accel * dt;

        float speed = Math::Length(f.velocity);
        if (speed < 1e-6f)
        {
            f.velocity = { 0, 0, minSpeed };
        }
        else
        {
            float clamped = Math::Clamp(speed, minSpeed, maxSpeed);
            f.velocity = f.velocity * (clamped / speed);
        }

        f.phase += bobFrequency * dt;

        float bob = sinf(f.phase) * bobAmplitude;
        f.position = f.position + f.velocity * dt + Vector3(0.0f, bob * dt, 0.0f);

        f.position.x = Math::Clamp(f.position.x, scaledMin.x, scaledMax.x);
        f.position.y = Math::Clamp(f.position.y, scaledMin.y, scaledMax.y);
        f.position.z = Math::Clamp(f.position.z, scaledMin.z, scaledMax.z);
    }


    if (m_instanceBuffer->GetCapacity() != m_fish.size())
        m_instanceBuffer->Allocate(m_fish.size());

    for (size_t i = 0; i < m_fish.size(); ++i)
    {
        Fish& f = m_fish[i];

        float speed2D = sqrtf(f.velocity.x * f.velocity.x +
            f.velocity.z * f.velocity.z);

        float yaw = atan2f(f.velocity.x, f.velocity.z);
        float pitch = atan2f(-f.velocity.y, speed2D);

        Vector3 eulerDeg(
            Math::Degrees(pitch),
            Math::Degrees(yaw),
            0.0f
        );

        Matrix4 m = Matrix4::Identity();
        m = Math::Translate(m, worldPos + f.position);
        m = Math::Rotate(m, Quaternion::FromEuler(Math::Radians(eulerDeg)));
        m = Math::Scale(m, Vector3(fishScale, fishScale, fishScale));

        m_instanceBuffer->Set(i, m);
    }

    m_instanceBuffer->Upload();

    if (Renderer != nullptr)
        Renderer->instanceBuffer = m_instanceBuffer;
}