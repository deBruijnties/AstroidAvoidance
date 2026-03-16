#include <core/engine.h>
#include <core/Scene/Gameobject.h>
#include <core/Scene/Components/Transform.h>
#include "core/Rendering/Buffers/StructuredBuffer.h"
#include <core/Scene/Components/Camera.h>
#include <core/Scene/Scene.h>
#include "AstroidSelection.h"
#include <Core/input/Mouse.h>
#include <iostream>

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
	// --- 1. CRT quad vertices from your .obj file (local model space) ---
	Vector3 TL_local = Vector3(-0.3125f, 0.89985125f, -0.29094875f);
	Vector3 BL_local = Vector3(-0.3125f, 0.277229375f, -0.34542125f);
	Vector3 TR_local = Vector3(0.3125f, 0.89985125f, -0.29094875f);
	Vector3 BR_local = Vector3(0.3125f, 0.277229375f, -0.34542125f);

	// --- 2. Transform all quad vertices to clip space ---
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

	// --- 3. Compute UV using rectangle projection ---
	Vector2 A = TL;  // origin
	Vector2 AB = TR - TL; // horizontal axis
	Vector2 AC = BL - TL; // vertical axis

	Vector2 AM(mouseX - A.x, mouseY - A.y);

	float u = Math::Dot(AM, AB) / Math::Dot(AB, AB);
	float v = Math::Dot(AM, AC) / Math::Dot(AC, AC);

	// If mouse is outside quad
	if (u < 0.f || u > 1.f || v < 0.f || v > 1.f)
		return false;

	// --- 4. Convert quad UV → CRT UV region ---
	// Your obj UVs:
	// U: 0.5 → 1.0
	// V: 0.0 → 0.5

	outUV.x = (1 - u - .5f) * 2;
	outUV.y = (1 - v - .5f) * 2;
	return true;
}

void AstroidSelection::OnStart()
{
	m_instanceBuffer = new StructuredBuffer(12, sizeof(Matrix4));
	m_instanceBuffer->Allocate(18);

}

void AstroidSelection::screenPosToWorldRay(
	const Vector2& ndcMouse,        // (-1..1, -1..1)
	const Matrix4& view,
	const Matrix4& projection,
	Vector3& outOrigin,
	Vector3& outDir)
{
	// 1. Inverse view-projection
	Matrix4 invVP = Math::Inverse(projection * view);

	// 2. Two points in clip space
	Vector4 pNear = Vector4(ndcMouse.x, ndcMouse.y, -1.0f, 1.0f);
	Vector4 pFar = Vector4(ndcMouse.x, ndcMouse.y, 1.0f, 1.0f);

	// 3. Unproject
	Vector4 worldNear = invVP * pNear;
	Vector4 worldFar = invVP * pFar;

	// Perspective divide
	worldNear /= worldNear.w;
	worldFar /= worldFar.w;

	// 4. Ray definition
	outOrigin = Vector3(worldNear.x, worldNear.y, worldNear.z);
	Vector4 out = worldFar - worldNear;
	outDir = Math::Normalize(Vector3(out.x, out.y, out.z));
}


void AstroidSelection::OnUpdate()
{
	AstroidSpawner::AstroidData* selected = spawner->GetAstroidById(m_selectedIndex);

	if (selected && Input::Input::IsKeyReleased(Input::Key::Space))
	{
		ps->baseSize = selected->size;
		ps->Burst(15, selected->position);
		spawner->DestroyAstroid(*selected);
	}

	if (selected != nullptr)
	{
		if (spawner->hits > 3) // on gameover dont allow selection of astroids
			return;

		//transform.right from AstroidCam
		Vector3 right = Math::Normalize(AstroidCam->worldMatrix.GetColumn(0));
	
		Vector3 EarthDir = Math::Normalize(Vector3() - selected->position);
		Vector3 SunDir = Math::Normalize(SunObj->worldPosition - selected->position);

		//final offset dir
		Vector3 dir = Math::Normalize(EarthDir - SunDir);

		AstroidCam->localPosition = selected->position + (-dir * Vector3(50,50,50));
		//recalculate dir so lookat the astroid
		dir = Math::Normalize(selected->position - AstroidCam->localPosition);

		Quaternion rotation = Quaternion::quatLookAtRH(dir, Vector3(0, 1, 0));

		AstroidCam->localRotation = rotation;
		AstroidCam->MarkDirty();

	}

	if (!crt || !roomCam)
		return;

	Vector2 screenMouse = Vector2();

	bool hit = GetCRTSurfaceUV(
		Input::Mouse::position.x,
		Input::Mouse::position.y,
		Engine::width,
		Engine::height,

		crt->worldMatrix, // MODEL

		Math::Inverse(roomCam->worldMatrix), // VIEW

		roomCam->gameObject->getComponent<Camera>()->GetProjectionMatrix(), // PROJECTION

		screenMouse
	);

	if (hit)
	{
		if (Input::Mouse::buttons[0])
		{
			Vector3 rayOrigin;
			Vector3 rayDir; // normalized

			screenPosToWorldRay(screenMouse, Math::Inverse(spaceCam->transform->worldMatrix), spaceCam->gameObject->getComponent<Camera>()->GetProjectionMatrix(), rayOrigin, rayDir);

			const AstroidSpawner::AstroidData* hit =
				raycast(rayOrigin, rayDir, earthGen->planetSize * 0.5f);

			if (hit == nullptr)
			{
				// planet hit OR nothing hit
			}
			else
			{
				// asteroid hit
				std::cout << "Hit asteroid id = " << hit->id << "\n";

				m_selectedIndex = hit->id;
			}
		}

	}

	AstroidSpawner::AstroidData* selectedAsteroid = spawner->GetAstroidById(m_selectedIndex);
	if (selectedAsteroid == nullptr)
	{
		for (size_t i = 0; i < 17; i++)
		{
			m_instanceBuffer->Set(i, Matrix4());
		}
		m_instanceBuffer->Upload();
		return;
	}
	std::vector<Vector3> pos = spawner->PredictFuturePosition(*selectedAsteroid, 17);

	for (size_t i = 0; i < 17; i++)
	{

		Matrix4 m = Matrix4::Identity();
		m = Math::Translate(m, pos[i]);
		
		m = Math::Scale(m, Vector3(.1, .1, .1));
		m_instanceBuffer->Set(i, m);

	}
	m_instanceBuffer->Upload();
	meshRendererInstanced->instanceBuffer = m_instanceBuffer;

}

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
		return false; // no hit

	float thc = sqrt(r2 - d2);
	float t0 = tca - thc;
	float t1 = tca + thc;

	if (t0 > 0.0f)
		outT = t0;
	else
		outT = t1;

	return t1 >= 0.0f; // must be in front of ray
}


const AstroidSpawner::AstroidData* AstroidSelection::raycast(
	const Vector3& rayOrigin,
	const Vector3& rayDir,
	float planetRadius)
{
	const AstroidSpawner::AstroidData* closestHit = nullptr;

	float closestDist = FLT_MAX;

	// Planet is centered at (0,0,0)
	{
		float tPlanet;
		if (raySphere(rayOrigin, rayDir, Vector3(), planetRadius, tPlanet))
		{
			if (tPlanet >= 0.0f)
			{
				// Planet blocks the ray → return nullptr immediately
				return nullptr;
			}
		}
	}

	for (auto& a : spawner->GetAstroidData())
	{
		float asteroidRadius = a.size *20;   // matches collision code
		float tHit = 0.0f;

		if (!raySphere(rayOrigin, rayDir, a.position, asteroidRadius, tHit))
			continue;

		if (tHit < 0.0f)
			continue; // behind camera

		if (tHit < closestDist)
		{
			closestDist = tHit;
			closestHit = &a;
		}
	}

	return closestHit; // nullptr if none hit
}



