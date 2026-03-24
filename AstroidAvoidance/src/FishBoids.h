#pragma once
#include <core/Scene/Component.h>
#include <core/Rendering/Buffers/StructuredBuffer.h>
#include <core/Scene/Components/MeshRendererInstanced.h>
#include <vector>


class FishBoids : public Component
{
public:
    MeshRendererInstanced* Renderer = nullptr;

    // Spawn 
    int   fishCount = 20;
    float spawnRadius = 0.6f;

    // Boid weights 
    float separationWeight = 2.0f;
    float alignmentWeight = 0.8f;
    float cohesionWeight = 0.7f;
    float boundaryWeight = 3.0f;
    float wanderWeight = 0.6f;

    // Perception radii 
    float separationRadius = 0.18f;
    float alignmentRadius = 0.25f;
    float cohesionRadius = 0.20f;

    // Movement 
    float minSpeed = 0.15f;
    float maxSpeed = 0.25f;
    float maxSteerForce = 0.2f;

    // Aesthetics 
    float bobAmplitude = 0.003f;
    float bobFrequency = 2.5f;

    // Soft aquarium bounds (local space, centred on entity) 
    Vector3 boundsMin = { -1.0f, -0.5f, -1.0f };
    Vector3 boundsMax = { 1.0f,  0.5f,  1.0f };
    float   boundaryMargin = 0.15f;

    // Visual 
    float fishScale = 0.04f;

private:
    struct Fish
    {
        uint32_t id;
        Vector3  position;
        Vector3  velocity;
        float    phase;        // per-fish swim-bob phase offset
        float    wanderAngle;  // slowly drifting random heading offset
    };

    std::vector<Fish> m_fish;
    StructuredBuffer* m_instanceBuffer = nullptr;
    uint32_t          m_nextId = 0;

    static constexpr unsigned int INSTANCE_BINDING = 12;

    // Steering rules 
    Vector3 CalcSeparation(const Fish& self) const;
    Vector3 CalcAlignment(const Fish& self) const;
    Vector3 CalcCohesion(const Fish& self) const;
    Vector3 CalcBoundary(const Fish& self, const Vector3& sMin,
        const Vector3& sMax, float margin) const;

    // Math helpers 
    static Vector3 ClampMagnitude(const Vector3& v, float maxLen);
    static Vector3 Steer(const Vector3& velocity, const Vector3& desired,
        float maxSpeed, float maxForce);

    void OnStart()  override;
    void OnUpdate() override;
};