#pragma once
#include <Core/Scene/Component.h>
#include "AstroidSpawner.h"
#include "core/Rendering/Buffers/StructuredBuffer.h"

class AstroidSelection : public Component
{
public:
	MeshRendererInstanced* meshRendererInstanced = nullptr;
    AstroidSpawner* spawner = nullptr;
    Camera* spaceCam = nullptr;
    EarthMeshGenerator* earthGen = nullptr;
    Transform* crt = nullptr;
    Transform* roomCam = nullptr;
    Transform* AstroidCam = nullptr;
    Transform* SunObj = nullptr;

    ParticleSystem* ps = nullptr;
    
    int m_selectedIndex = -1;

    void OnStart() override;
    void screenPosToWorldRay(const Vector2& ndcMouse, const Matrix4& view, const Matrix4& projection, Vector3& outOrigin, Vector3& outDir);
    void OnUpdate() override;

    bool raySphere(const Vector3& rayOrigin, const Vector3& rayDir, const Vector3& sphereCenter, float sphereRadius, float& outT);
    const AstroidSpawner::AstroidData* raycast(const Vector3& rayOrigin, const Vector3& rayDir, float planetRadius);
private:
    StructuredBuffer* m_instanceBuffer = nullptr;

};


