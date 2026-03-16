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

    bool Started = false;

    ParticleSystem* ps;
    int hits = 0;
    Canvas* gameovercanvas;

    EarthMeshGenerator* earthMeshGenerator = nullptr;
    MeshRendererInstanced* meshRendererInstanced = nullptr;

    struct AstroidData
    {
        uint64_t id = 0;

        Vector3 position{};
        Vector3 rotation{};
        Vector3 angularVelocity{};

        Vector3 orbitAxis{};
        float orbitRadius = 0.0f;
        float orbitAngle = 0.0f;
        float orbitSpeed = 0.0f;
        float radialSpeed = 0.0f;
        float eccentricity = 0.0f;
        float size = 1.0f;
    };
    void DestroyAstroid(AstroidData& astroid);

    std::vector<Vector3> PredictFuturePosition(AstroidData& astroid, int secondsAhead);

    const std::vector<AstroidData>& GetAstroidData() const
    {
        return m_astroids;
    }

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

    // Difficulty curve
    float m_minAsteroids = 2.0f;
    float m_maxAsteroids = 200.0f;
    float m_rampUpTime = 300.0f; // seconds to reach max (5 min)

    float m_spawnAccumulator = 0.0f;
    float m_spawnPerSecondMin = 0.2f;  // very slow at start
    float m_spawnPerSecondMax = 5.0f;  // late game chaos


    AstroidData createRandomAstroid();

    bool checkCollision(AstroidData& astroid);
    

private:
    std::vector<AstroidData> m_astroids;

    StructuredBuffer* m_instanceBuffer = nullptr;

    uint64_t m_nextAstroidId = 1;
};
