#pragma once
#include <Core/Scene/Component.h>
#include "AstroidSpawner.h"
#include "core/Rendering/Buffers/StructuredBuffer.h"

class AstroidSelection : public Component
{
public:
    // Renders predicted positions of selected asteroid
    MeshRendererInstanced* meshRendererInstanced = nullptr;

    // References to systems and scene objects
    AstroidSpawner* spawner = nullptr;
    Camera* spaceCam = nullptr;
    EarthMeshGenerator* earthGen = nullptr;

    Transform* crt = nullptr;        // CRT screen in world
    Transform* roomCam = nullptr;    // camera viewing CRT
    Transform* AstroidCam = nullptr; // follow camera
    Transform* SunObj = nullptr;

    ParticleSystem* ps = nullptr;

    int m_selectedIndex = -1; // currently selected asteroid id

    void OnStart() override;

    // Converts screen-space mouse to world-space ray
    void screenPosToWorldRay(
        const Vector2& ndcMouse,
        const Matrix4& view,
        const Matrix4& projection,
        Vector3& outOrigin,
        Vector3& outDir
    );

    void OnUpdate() override;

    // Ray vs sphere intersection
    bool raySphere(
        const Vector3& rayOrigin,
        const Vector3& rayDir,
        const Vector3& sphereCenter,
        float sphereRadius,
        float& outT
    );

    // Returns closest asteroid hit or nullptr
    const AstroidSpawner::AstroidData* raycast(
        const Vector3& rayOrigin,
        const Vector3& rayDir,
        float planetRadius
    );

private:
    // Instance buffer for rendering prediction markers
    StructuredBuffer* m_instanceBuffer = nullptr;
};