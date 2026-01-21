#include "game.h"
#include <iostream>
#include "Core/Time.h"
#include "Core/engine.h"
#include <Core/Scene/Scene.h>
#include <Core/Scene/Gameobject.h>
#include <Core/Scene/Components/Transform.h>
#include <Core/Scene/Components/Camera.h>
#include <Core/Rendering/Mesh/Mesh.h>
#include <Core/Rendering/Resources/Material.h>
#include <Core/Rendering/Resources/Shader.h>
#include <Core/Scene/Components/MeshRenderer.h>
#include <Core/Rendering/Renderer.h>
#include <Core/Scene/Components/Lighting/PointLight.h>


Mesh DuckMesh = Mesh::LoadMeshFromFile("assets/models/ducksmooth.obj"); // flat shaded assets/models/duck.obj

Material duckMaterial;

GameObject* DuckObj;


inline std::ostream& operator<<(std::ostream& os, const GalacticEngine::Quaternion& q)
{
	os << "Quaternion(x=" << q.x
		<< ", y=" << q.y
		<< ", z=" << q.z
		<< ", w=" << q.w << ")";
	return os;
}

void Game::OnStart()
{
	currentScene = new Scene();

	DuckMesh.GenerateBuffers();

	Shader* duckShader = new Shader("assets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "assets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.frag", true);
	duckMaterial = Material(duckShader);
	duckMaterial.SetTexture("uAlbedoMap", "assets/textures/duck_albedo.png", true);
	duckMaterial.SetBool("uUseAlbedoMap", true);

	DuckObj = currentScene->createObject("");
	DuckObj->transform->localPosition = Vector3(0, 1, -2);
	DuckObj->transform->MarkDirty();
	{ // components
		MeshRenderer* meshRenderer = DuckObj->addComponent<MeshRenderer>();
		meshRenderer->mesh = &DuckMesh;
		meshRenderer->material = &duckMaterial;
	}

	GameObject* roomCamObj = currentScene->createObject("roomCamera");
	roomCamObj->transform->localPosition = Vector3(0,0,0);
	roomCamObj->transform->MarkDirty();
	{ // components
		Camera* roomCam = roomCamObj->addComponent<Camera>();
		roomCam->clearColor = Vector3(0.1f, 0.1f, 0.1f);
		Renderer::SetCamera(roomCam);

	}

	GameObject* RoomLightObj = currentScene->createObject("testLight");
	RoomLightObj->transform->localPosition = Vector3(0, 2, 0);
	RoomLightObj->transform->MarkDirty();
	{ // components
		PointLight* pointLight = RoomLightObj->addComponent<PointLight>();
		pointLight->color = Vector3(0.9647058823529412f, 0.8941176470588236f, 0.7372549019607844f);
		pointLight->intensity = 0.95f;
		pointLight->radius = 20.0f;
	}




}





void Game::OnUpdate()
{

	DuckObj->transform->localRotation = Quaternion::FromEuler(Vector3(0, Time::timeSinceStartup, 0));
	DuckObj->transform->MarkDirty();
}

void Game::OnProcessInput()
{
}
