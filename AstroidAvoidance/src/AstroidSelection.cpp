#include <core/engine.h>
#include <core/Scene/Gameobject.h>
#include <core/Scene/Components/Transform.h>
#include "core/Rendering/Buffers/StructuredBuffer.h"
#include <core/Scene/Components/Camera.h>
#include <core/Scene/Scene.h>
#include "AstroidSelection.h"
#include <Core/input/Mouse.h>
#include <iostream>

// Projects mouse position onto CRT quad and returns UV in its local space
bool GetCRTSurfaceUV(
	float mouseX,
	float mouseY,
	float screenW,
	float screenH,
	const Matrix4& model,
	const Matrix4& view,
	const Matrix4& projection,
	Vector2& outUV)
{
	// CRT quad vertices in model space
	Vector3 TL_local = Vector3(-0.3125f, 0.89985125f, -0.29094875f);
	Vector3 BL_local = Vector3(-0.3125f, 0.277229375f, -0.34542125f);
	Vector3 TR_local = Vector3(0.3125f, 0.89985125f, -0.29094875f);
	Vector3 BR_local = Vector3(0.3125f, 0.277229375f, -0.34542125f);

	// Transform a vertex to screen space
	auto ToScreen = [&](const Vector3& local) -> Vector2
		{
			Vector4 world = model * Vector4(local.x, local.y, local.z, 1.0);
			Vector4 clip = projection * view * world;

			Vector3 ndc = Vector3(clip.x, clip.y, clip.z) / clip.w;

			Vector2 screen;
			screen.x = (ndc.x * 0.5f + 0.5f) * screenW;
			screen.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * screenH;
			return screen;
		};

	Vector2 TL = ToScreen(TL_local);
	Vector2 TR = ToScreen(TR_local);
	Vector2 BL = ToScreen(BL_local);
	Vector2 BR = ToScreen(BR_local);

	// Compute UV via projection onto quad axes
	Vector2 A = TL;
	Vector2 AB = TR - TL;
	Vector2 AC = BL - TL;

	Vector2 AM(mouseX - A.x, mouseY - A.y);

	float u = Math::Dot(AM, AB) / Math::Dot(AB, AB);
	float v = Math::Dot(AM, AC) / Math::Dot(AC, AC);

	// Outside quad
	if (u < 0.f || u > 1.f || v < 0.f || v > 1.f)
		return false;

	// Map to CRT UV region
	outUV.x = (1 - u - .5f) * 2;
	outUV.y = (1 - v - .5f) * 2;
	return true;
}

void AstroidSelection::OnStart()
{
	m_instanceBuffer = new StructuredBuffer(12, sizeof(Matrix4));
	m_instanceBuffer->Allocate(18);
}

// Converts NDC mouse position to a world-space ray
void AstroidSelection::screenPosToWorldRay(
	const Vector2& ndcMouse,
	const Matrix4& view,
	const Matrix4& projection,
	Vector3& outOrigin,
	Vector3& outDir)
{
	Matrix4 invVP = Math::Inverse(projection * view);

	Vector4 pNear = Vector4(ndcMouse.x, ndcMouse.y, -1.0f, 1.0f);
	Vector4 pFar = Vector4(ndcMouse.x, ndcMouse.y, 1.0f, 1.0f);

	Vector4 worldNear = invVP * pNear;
	Vector4 worldFar = invVP * pFar;

	worldNear /= worldNear.w;
	worldFar /= worldFar.w;

	outOrigin = Vector3(worldNear.x, worldNear.y, worldNear.z);
	Vector4 out = worldFar - worldNear;
	outDir = Math::Normalize(Vector3(out.x, out.y, out.z));
}

// Handles selection, raycasting and prediction rendering
void AstroidSelection::OnUpdate()
{
	// Disable selection after game over
	if (spawner->hits >= 3)
	{
		m_selectedIndex = -1;
		return;
	}

	AstroidSpawner::AstroidData* selected = spawner->GetAstroidById(m_selectedIndex);

	// Destroy selected asteroid on input
	if (selected && Input::Input::IsKeyReleased(Input::Key::Space))
	{
		ps->baseSize = selected->size;
		ps->Burst(15, selected->position);
		spawner->DestroyAstroid(*selected);
	}

	// Move camera to follow selected asteroid
	if (selected != nullptr)
	{


		Vector3 right = Math::Normalize(AstroidCam->worldMatrix.GetColumn(0));

		Vector3 EarthDir = Math::Normalize(Vector3() - selected->position);
		Vector3 SunDir = Math::Normalize(SunObj->worldPosition - selected->position);

		Vector3 dir = Math::Normalize(EarthDir - SunDir);

		AstroidCam->localPosition = selected->position + (-dir * Vector3(50, 50, 50));

		dir = Math::Normalize(selected->position - AstroidCam->localPosition);
		Quaternion rotation = Quaternion::quatLookAtRH(dir, Vector3(0, 1, 0));

		AstroidCam->localRotation = rotation;
		AstroidCam->MarkDirty();
	}

	if (!crt || !roomCam)
		return;

	Vector2 screenMouse = Vector2();

	// Project mouse onto CRT surface
	bool hit = GetCRTSurfaceUV(
		Input::Mouse::position.x,
		Input::Mouse::position.y,
		(float)Engine::width,
		(float)Engine::height,
		crt->worldMatrix,
		Math::Inverse(roomCam->worldMatrix),
		roomCam->gameObject->getComponent<Camera>()->GetProjectionMatrix(),
		screenMouse
	);

	if (hit)
	{
		if (Input::Mouse::buttons[0])
		{
			Vector3 rayOrigin;
			Vector3 rayDir;

			// Raycast from CRT into space scene
			screenPosToWorldRay(
				screenMouse,
				Math::Inverse(spaceCam->transform->worldMatrix),
				spaceCam->gameObject->getComponent<Camera>()->GetProjectionMatrix(),
				rayOrigin,
				rayDir
			);

			const AstroidSpawner::AstroidData* hit =
				raycast(rayOrigin, rayDir, earthGen->planetSize * 0.5f);

			if (hit != nullptr)
			{
				std::cout << "Hit asteroid id = " << hit->id << "\n";
				m_selectedIndex = hit->id;
			}
		}
	}

	AstroidSpawner::AstroidData* selectedAsteroid = spawner->GetAstroidById(m_selectedIndex);

	// Clear prediction if nothing selected
	if (selectedAsteroid == nullptr)
	{
		for (size_t i = 0; i < 17; i++)
			m_instanceBuffer->Set(i, Matrix4());

		m_instanceBuffer->Upload();
		return;
	}

	// Draw predicted future positions
	std::vector<Vector3> pos = spawner->PredictFuturePosition(*selectedAsteroid, 17);

	for (size_t i = 0; i < 17; i++)
	{
		Matrix4 m = Matrix4::Identity();
		m = Math::Translate(m, pos[i]);
		m = Math::Scale(m, Vector3(.1f, .1f, .1f));

		m_instanceBuffer->Set(i, m);
	}

	m_instanceBuffer->Upload();
	meshRendererInstanced->instanceBuffer = m_instanceBuffer;
}

// Ray vs sphere intersection
bool AstroidSelection::raySphere(
	const Vector3& rayOrigin,
	const Vector3& rayDir,
	const Vector3& sphereCenter,
	float sphereRadius,
	float& outT)
{
	Vector3 L = sphereCenter - rayOrigin;
	float tca = Math::Dot(L, rayDir);
	float d2 = Math::Dot(L, L) - tca * tca;
	float r2 = sphereRadius * sphereRadius;

	if (d2 > r2)
		return false;

	float thc = sqrt(r2 - d2);
	float t0 = tca - thc;
	float t1 = tca + thc;

	if (t0 > 0.0f)
		outT = t0;
	else
		outT = t1;

	return t1 >= 0.0f;
}

// Raycast against planet and asteroids, returns closest asteroid hit
const AstroidSpawner::AstroidData* AstroidSelection::raycast(
	const Vector3& rayOrigin,
	const Vector3& rayDir,
	float planetRadius)
{
	const AstroidSpawner::AstroidData* closestHit = nullptr;
	float closestDist = FLT_MAX;

	// Planet blocks selection
	{
		float tPlanet;
		if (raySphere(rayOrigin, rayDir, Vector3(), planetRadius, tPlanet))
		{
			if (tPlanet >= 0.0f)
				return nullptr;
		}
	}

	for (auto& a : spawner->GetAstroidData())
	{
		float asteroidRadius = a.size * 20;
		float tHit = 0.0f;

		if (!raySphere(rayOrigin, rayDir, a.position, asteroidRadius, tHit))
			continue;

		if (tHit < 0.0f)
			continue;

		if (tHit < closestDist)
		{
			closestDist = tHit;
			closestHit = &a;
		}
	}

	return closestHit;
}