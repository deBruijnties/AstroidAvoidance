#include "AstroidSpawner.h"
#include <core/Time.h>
#include <core/Rendering/Buffers/StructuredBuffer.h>
#include <core/engine.h>
#include <core/Input/Input.h>
#include <core/Input/KeyCodes.h>
#include <core/Rendering/Renderer.h>
#include <cstdlib>

// Returns a random float in [min, max]
static float RandRange(float min, float max)
{
    float t = float(rand()) / float(RAND_MAX);
    return min + t * (max - min);
}

// Generates a uniformly distributed random direction on a sphere
static Vector3 RandomDirectionSphere()
{
    float u = float(rand()) / float(RAND_MAX);
    float v = float(rand()) / float(RAND_MAX);

    float theta = Math::TWO_PI * 2 * u;
    float phi = acos(2.0f * v - 1.0f);

    return {
        sin(phi) * cos(theta),
        sin(phi) * sin(theta),
        cos(phi)
    };
}

// Creates a randomized asteroid with orbital + rotation properties
AstroidSpawner::AstroidData AstroidSpawner::createRandomAstroid()
{
    AstroidData a{};
    a.id = m_nextAstroidId++;

    a.orbitAxis = RandomDirectionSphere();
    a.orbitRadius = RandRange(550.0f, 660.0f);
    a.orbitAngle = RandRange(0.0f, Math::TWO_PI);
    a.orbitSpeed = RandRange(0.1f, 0.3f);
    a.radialSpeed = RandRange(15.0f, 25.0f);
    a.eccentricity = RandRange(0.2f, 0.6f);
    a.size = RandRange(0.5f, 1.5f);

    a.rotation = {
        RandRange(0.0f, 360.0f),
        RandRange(0.0f, 360.0f),
        RandRange(0.0f, 360.0f)
    };

    a.angularVelocity = {
        RandRange(-90.0f, 90.0f),
        RandRange(-90.0f, 90.0f),
        RandRange(-90.0f, 90.0f)
    };

    // Initial orbit position in local ellipse space
    Vector3 orbitPos(
        a.orbitRadius * (1.0f - a.eccentricity),
        0.0f,
        a.orbitRadius * a.eccentricity
    );

    // Rotate orbit into world space
    Matrix4 rot = Math::Rotate(a.orbitAngle, a.orbitAxis);
    Vector4 rotated = (rot * Vector4(orbitPos.x, orbitPos.y, orbitPos.z, 1.0f));

    a.position = Vector3(rotated.x, rotated.y, rotated.z);
    return a;
}

// Simple sphere collision check between asteroid and planet
bool AstroidSpawner::checkCollision(AstroidData& astroid)
{
    float astroidSize = 30 * astroid.size;
    float planetSize = earthMeshGenerator->planetSize;

    float distanceBetween = Math::Length(astroid.position) - astroidSize / 2 - planetSize / 2;

    return distanceBetween <= 0;
}

// Initialize asteroid system and GPU instance buffer
void AstroidSpawner::OnStart()
{
    m_astroids.clear();
    m_astroids.reserve(256);

    // Start with very few asteroids
    for (int i = 0; i < 2; ++i)
        m_astroids.push_back(createRandomAstroid());

    constexpr unsigned int INSTANCE_BINDING = 12;
    constexpr size_t INSTANCE_STRIDE = sizeof(Matrix4);

    m_instanceBuffer = new StructuredBuffer(INSTANCE_BINDING, INSTANCE_STRIDE);
    m_instanceBuffer->Allocate(m_astroids.size());
}

// Main update loop: spawning, simulation, collision, rendering
void AstroidSpawner::OnUpdate()
{
    if (hits < 3)
        TimeSinceStartGame += Time::deltaTime;

    // Wait for player input before starting
    if (!Started)
    {
        if (Started != Input::Input::IsAnyKeyPressed())
        {
            TimeSinceStartGame = 0;
            Started = true;
            Renderer::SetCanvas(nullptr);
        }
        return;
    }

    m_elapsed += Time::deltaTime;

    // Difficulty ramp (quadratic ease-in)
    float t = Math::Clamp(m_elapsed / m_rampUpTime, 0.0f, 1.0f);
    float curve = t * t;

    float targetAsteroids = Math::Lerp(m_minAsteroids, m_maxAsteroids, curve);
    float spawnRate = Math::Lerp(m_spawnPerSecondMin, m_spawnPerSecondMax, curve);

    m_spawnAccumulator += spawnRate * Time::deltaTime;

    // Spawn new asteroids based on accumulated rate
    while (m_spawnAccumulator >= 1.0f && m_astroids.size() < (size_t)targetAsteroids)
    {
        m_astroids.push_back(createRandomAstroid());
        m_spawnAccumulator -= 1.0f;
    }

    // Simulate asteroid movement
    for (auto& a : m_astroids)
    {
        a.orbitAngle += a.orbitSpeed * Time::deltaTime;
        a.orbitRadius -= a.radialSpeed * Time::deltaTime;

        if (a.orbitRadius < 0.0f)
            a.orbitRadius = 0.0f;

        Vector3 orbitPos(
            a.orbitRadius * (1.0f - a.eccentricity),
            0.0f,
            a.orbitRadius * a.eccentricity
        );

        Matrix4 rot = Math::Rotate(a.orbitAngle, a.orbitAxis);
        a.rotation += a.angularVelocity * Time::deltaTime;

        Vector4 rotated = rot * Vector4(orbitPos.x, orbitPos.y, orbitPos.z, 1.0f);
        a.position = Vector3(rotated.x, rotated.y, rotated.z);

        // Keep rotation within [0, 360]
        a.rotation = Math::Mod(a.rotation, Vector3(360.0f, 360.0f, 360.0f));
    }

    // Resize instance buffer if needed
    if (m_instanceBuffer->GetCapacity() != m_astroids.size())
        m_instanceBuffer->Allocate(m_astroids.size());

    // Upload transform matrices for instanced rendering
    for (size_t i = 0; i < m_astroids.size(); ++i)
    {
        const auto& a = m_astroids[i];

        Matrix4 m = Matrix4::Identity();
        m = Math::Translate(m, a.position);
        m = Math::Rotate(m, Quaternion::FromEuler(Math::Radians(a.rotation)));
        m = Math::Scale(m, Vector3(a.size, a.size, a.size));

        m_instanceBuffer->Set(i, m);
    }

    // Collision + destruction (reverse loop for safe removal)
    for (int i = (int)m_astroids.size() - 1; i >= 0; --i)
    {
        AstroidData& a = m_astroids[i];

        if (checkCollision(a))
        {
            // Convert grid center to world space
            const float halfGrid = (float)earthMeshGenerator->gridCubes * 0.5f;
            const Vector3 centerGrid(halfGrid, halfGrid, halfGrid);
            const Vector3 centerWS = centerGrid * Vector3(
                earthMeshGenerator->scale,
                earthMeshGenerator->scale,
                earthMeshGenerator->scale
            );

            // Carve impact + update mesh
            earthMeshGenerator->carveSphereHole(centerWS + a.position, a.size * 25);
            earthMeshGenerator->marchingCubes();

            // Spawn particles
            ps->baseSize = a.size;
            ps->Burst(15, a.position);

            // Remove asteroid (swap-remove)
            m_astroids[i] = m_astroids.back();
            m_astroids.pop_back();

            hits++;
            std::cout << "hits: " << hits << "\n";

            // Game over condition
            if (hits >= 3)
            {
                Renderer::SetCanvas(gameovercanvas);

                std::stringstream stream;
                stream << std::fixed << std::setprecision(2) << TimeSinceStartGame;
                std::string s = "YOU SURVIVED " + stream.str() + " SECONDS!!!!";
                Engine::SetTitle(s.c_str());
            }
        }
    }

    m_instanceBuffer->Upload();

    if (meshRendererInstanced != nullptr)
        meshRendererInstanced->instanceBuffer = m_instanceBuffer;
}

// Removes an asteroid by ID
void AstroidSpawner::DestroyAstroid(AstroidData& astroid)
{
    m_astroids.erase(
        std::remove_if(
            m_astroids.begin(),
            m_astroids.end(),
            [&](const AstroidData& a) { return a.id == astroid.id; }
        ),
        m_astroids.end()
    );
}

// Predicts future positions (1 step per second)
std::vector<Vector3> AstroidSpawner::PredictFuturePosition(
    AstroidData& astroid,
    int secondsAhead)
{
    std::vector<Vector3> positions;
    positions.reserve(secondsAhead);

    AstroidData a = astroid;
    float dt = 1;

    // simulate with a deltatime of 1  
    for (size_t i = 0; i < secondsAhead; i++)
    {
        a.orbitAngle += a.orbitSpeed * dt;
        a.orbitRadius -= a.radialSpeed * dt;

        if (a.orbitRadius < 0.0f)
            a.orbitRadius = 0.0f;

        Vector3 orbitPos(
            a.orbitRadius * (1.0f - a.eccentricity),
            0.0f,
            a.orbitRadius * a.eccentricity
        );

        Matrix4 rot = Math::Rotate(a.orbitAngle, a.orbitAxis);
        a.rotation += a.angularVelocity * dt;

        Vector4 Rotated = rot * Vector4(orbitPos.x, orbitPos.y, orbitPos.z, 1.0f);
        a.position = Vector3(Rotated.x, Rotated.y, Rotated.z);

        a.rotation = Math::Mod(a.rotation, Vector3(360.0f, 360.0f, 360.0f));

        positions.push_back(a.position);
    }
    return positions;
}