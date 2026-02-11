#pragma once
#include <Core/Scene/Component.h>
#include "AstroidSpawner.h"
#include "core/Rendering/Buffers/StructuredBuffer.h"

class AstroidSelection : public Component
{
public:
	MeshRendererInstanced* meshRendererInstanced;
    AstroidSpawner* spawner;
    Camera* spaceCam;
    EarthMeshGenerator* earthGen;
    Transform* crt;
    Transform* roomCam;
    Transform* AstroidCam;
    Transform* SunObj;

    ParticleSystem* ps;
    
    int m_selectedIndex;

    void OnStart() override;
    void screenPosToWorldRay(const Vector2& ndcMouse, const Matrix4& view, const Matrix4& projection, Vector3& outOrigin, Vector3& outDir);
    void OnUpdate() override;

    bool raySphere(const Vector3& rayOrigin, const Vector3& rayDir, const Vector3& sphereCenter, float sphereRadius, float& outT);
    const AstroidSpawner::AstroidData* raycast(const Vector3& rayOrigin, const Vector3& rayDir, float planetRadius);
private:
    StructuredBuffer* m_instanceBuffer = nullptr;

};


