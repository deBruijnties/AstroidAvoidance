#include "AstroidSpawner.h"
#include <core/Time.h>
#include <core/Rendering/Buffers/StructuredBuffer.h>
#include <core/engine.h>
#include <cstdlib>


static float RandRange(float min, float max)
{
    float t = float(rand()) / float(RAND_MAX);
    return min + t * (max - min);
}

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

    Vector3 orbitPos(
        a.orbitRadius * (1.0f - a.eccentricity),
        0.0f,
        a.orbitRadius * a.eccentricity
    );

    Matrix4 rot =
        Math::Rotate(a.orbitAngle, a.orbitAxis);

    Vector4 rotated = (rot * Vector4(orbitPos.x, orbitPos.y, orbitPos.z, 1.0f));

    a.position = Vector3(rotated.x, rotated.y, rotated.z);
    return a;
}

bool AstroidSpawner::checkCollision(AstroidData& astroid)
{
    float astroidSize = 30 * astroid.size;
    float planetSize = earthMeshGenerator->planetSize;

    float distanceBetween = Math::Length(astroid.position) - astroidSize / 2 - planetSize / 2;


    if (distanceBetween <= 0)
    {
        return true;

    }

    return false;
}

void AstroidSpawner::OnStart()
{
    m_astroids.clear();
    m_astroids.reserve(256);

    // Start with very few
    for (int i = 0; i < 2; ++i)
        m_astroids.push_back(createRandomAstroid());

    constexpr unsigned int INSTANCE_BINDING = 12;
    constexpr size_t INSTANCE_STRIDE = sizeof(Matrix4);

    m_instanceBuffer = new StructuredBuffer(INSTANCE_BINDING, INSTANCE_STRIDE);
    m_instanceBuffer->Allocate(m_astroids.size());
}


void AstroidSpawner::OnUpdate()
{
    m_elapsed += Time::deltaTime;

    // 0 - 1 over ramp time
    float t = Math::Clamp(m_elapsed / m_rampUpTime, 0.0f, 1.0f);

    // Smooth curve (ease-in)
    float curve = t * t;

    // asteroids ExspectedFount
    float targetAsteroids = Math::Lerp(m_minAsteroids, m_maxAsteroids, curve);

    // Spawn speed also ramps
    float spawnRate = Math::Lerp(m_spawnPerSecondMin, m_spawnPerSecondMax, curve);

    m_spawnAccumulator += spawnRate * Time::deltaTime;

    while (m_spawnAccumulator >= 1.0f && m_astroids.size() < (size_t)targetAsteroids)
    {
        m_astroids.push_back(createRandomAstroid());
        m_spawnAccumulator -= 1.0f;
    }


    // Simulate
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

        Matrix4 rot =
            Math::Rotate(a.orbitAngle, a.orbitAxis);
        a.rotation += a.angularVelocity * Time::deltaTime;

        Vector4 rotated = rot * Vector4(orbitPos.x, orbitPos.y, orbitPos.z, 1.0f);
        a.position = Vector3(rotated.x, rotated.y, rotated.z);

        a.rotation = Math::Mod(a.rotation, Vector3(360.0f, 360.0f, 360.0f));
    }

    if (m_instanceBuffer->GetInstanceCount() != m_astroids.size())
        m_instanceBuffer->Allocate(m_astroids.size());

    for (size_t i = 0; i < m_astroids.size(); ++i)
    {
        const auto& a = m_astroids[i];

        Matrix4 m = Matrix4::Identity();
        m = Math::Translate(m, a.position);
        //m = Math::Rotate(m, Quaternion::FromAxisAngle({ 1,0,0 }, Math::Radians(a.rotation.x)));
        //m = Math::Rotate(m, Quaternion::FromAxisAngle({ 0,1,0 }, Math::Radians(a.rotation.y)));
        m = Math::Rotate(m, Quaternion::FromEuler(Math::Radians(a.rotation)));
        m = Math::Scale(m, Vector3(a.size, a.size, a.size));


        m_instanceBuffer->Set(i, m);
    }
    for (size_t i = 0; i < m_astroids.size(); ++i)
    {
        AstroidData& a = m_astroids[i];
        // Collision check
        if (checkCollision(a))
        {
            const float halfGrid = (float)earthMeshGenerator->gridCubes * 0.5f;
            const Vector3 centerGrid(halfGrid, halfGrid, halfGrid); 
            const Vector3 centerWS = centerGrid * Vector3(earthMeshGenerator->scale, earthMeshGenerator->scale, earthMeshGenerator->scale);

            earthMeshGenerator->carveSphereHole(centerWS + a.position, a.size * 25);
            earthMeshGenerator->marchingCubes();

            // Remove asteroid safely
            m_astroids.erase(m_astroids.begin() + i);

            continue;
        }
    }

    m_instanceBuffer->Upload();

    if(meshRendererInstanced != nullptr)
	    meshRendererInstanced->instanceBuffer = m_instanceBuffer;
}

std::vector<Vector3> AstroidSpawner::PredictFuturePosition(
    AstroidData& astroid,
    int secondsAhead)
{
	std::vector<Vector3> positions;
    positions.reserve(secondsAhead);

	AstroidData a = astroid;

    float dt = 1;


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

        Matrix4 rot =
            Math::Rotate( a.orbitAngle, a.orbitAxis);
        a.rotation += a.angularVelocity * dt;

        Vector4 Rotated = rot * Vector4(orbitPos.x, orbitPos.y, orbitPos.z, 1.0f);
        a.position = Vector3(Rotated.x, Rotated.y, Rotated.z);

        a.rotation = Math::Mod(a.rotation, Vector3(360.0f, 360.0f, 360.0f));

		positions.push_back(a.position);
    }
	return positions;

}