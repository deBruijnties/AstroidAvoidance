#pragma once

#include <vector>
#include <cstdint>
#include <core/Scene/Component.h>
#include <Core/Rendering/Buffers/StructuredBuffer.h>
#include <Core/Scene/Components/MeshRendererInstanced.h>
#include "EarthMeshGenerator.h"
#include <Core/Scene/Components/ParticleSystem.h>
#include <Core/UI/Canvas.h>

class AstroidSpawner : public Component
{
public:
    void OnStart() override;
    void OnUpdate() override;

    bool Started = false; // game start state

    ParticleSystem* ps = nullptr;
    int hits = 0; // number of asteroid impacts
    Canvas* gameovercanvas = nullptr;

    EarthMeshGenerator* earthMeshGenerator = nullptr;
    MeshRendererInstanced* meshRendererInstanced = nullptr;

    float TimeSinceStartGame = 0;

    // Runtime data for each asteroid
    struct AstroidData
    {
        int id = 0;

        Vector3 position{};
        Vector3 rotation{};
        Vector3 angularVelocity{};

        // Orbit parameters
        Vector3 orbitAxis{};
        float orbitRadius = 0.0f;
        float orbitAngle = 0.0f;
        float orbitSpeed = 0.0f;
        float radialSpeed = 0.0f;
        float eccentricity = 0.0f;

        float size = 1.0f;
    };

    void DestroyAstroid(AstroidData& astroid);

    // Predicts future positions (1 step per second)
    std::vector<Vector3> PredictFuturePosition(AstroidData& astroid, int secondsAhead);

    // Read-only access to all asteroids
    const std::vector<AstroidData>& GetAstroidData() const
    {
        return m_astroids;
    }

    // Returns asteroid by id or nullptr
    AstroidSpawner::AstroidData* GetAstroidById(int id)
    {
        for (auto& a : m_astroids)
        {
            if (a.id == id)
                return &a;
        }
        return nullptr;
    }

private:
    float m_elapsed = 0.0f;

    // Controls difficulty scaling over time
    float m_minAsteroids = 2.0f;
    float m_maxAsteroids = 200.0f;
    float m_rampUpTime = 300.0f;

    float m_spawnAccumulator = 0.0f;
    float m_spawnPerSecondMin = 0.2f;
    float m_spawnPerSecondMax = 5.0f;

    // Creates a randomized asteroid
    AstroidData createRandomAstroid();

    // Checks collision with planet
    bool checkCollision(AstroidData& astroid);

private:
    std::vector<AstroidData> m_astroids;

    // GPU buffer for instanced rendering
    StructuredBuffer* m_instanceBuffer = nullptr;

    int m_nextAstroidId = 1;
};