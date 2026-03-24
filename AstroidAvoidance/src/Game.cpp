#include "game.h"
#include <iostream>
#include <memory>
#include <core/Scene/Scene.h>
#include <core/Scene/GameObject.h>
#include <core/Rendering/Mesh/Mesh.h>
#include <core/Rendering/Mesh/FullScreenQuad.h>
#include <core/Scene/Components/Transform.h>
#include <core/Scene/Components/Camera.h>
#include <Core/Scene/Components/MeshRenderer.h>
#include <core/Rendering/Resources/Shader.h>
#include <core/Rendering/Resources/Material.h>
#include <core/Rendering/Renderer.h>
#include <Core/Scene/Components/Lighting/PointLight.h>
#include <Core/Time.h>
#include "EarthMeshGenerator.h"
#include "OrbitCamLook.h"
#include "RotateOverTime.h"
#include <core/Rendering/Buffers/framebuffer.h>
#include <Core/Scene/Components/MeshRendererInstanced.h>
#include "AstroidSpawner.h"
#include "AstroidSelection.h"
#include <Core/Scene/Components/ParticleSystem.h>
#include <Core/UI/Canvas.h>
#include <Core/UI/TextureElement.h>
#include <FishBoids.h>
#include <DuckBounce.h>



Mesh DuckMesh = Mesh::LoadMeshFromFile("assets/models/ducksmooth.obj"); // flat shaded assets/models/duck.obj
Mesh crtMesh = Mesh::LoadMeshFromFile("Assets/models/CRT.obj");
Mesh roomMesh = Mesh::LoadMeshFromFile("assets/models/Room.obj");
Mesh roomDeskMesh = Mesh::LoadMeshFromFile("assets/models/Desk.obj");
Mesh aquariumMesh = Mesh::LoadMeshFromFile("assets/models/Aquarium2.obj");
Mesh sunMesh = Mesh::LoadMeshFromFile("assets/models/Sun.obj");
Mesh spaceSkySphereMesh = Mesh::LoadMeshFromFile("assets/models/skysphere.obj");
Mesh astroidMesh = Mesh::LoadMeshFromFile("assets/models/AstroidSmooth.obj");
Mesh astroidSelectionMesh = Mesh::LoadMeshFromFile("assets/models/AstroidPrediction.obj");
Mesh fishIncLogoMesh = Mesh::LoadMeshFromFile("assets/models/FishIncLogo.obj");
Mesh FishMesh = Mesh::LoadMeshFromFile("assets/models/fish.obj");


Material spaceSkySphereMaterial;
Material duckMaterial;
Material crt1Material;
Material crt2Material;
Material roomMaterial;
Material TestUIMaterial;
Material roomDeskMaterial;
Material aquariumMaterial;
Material earthMaterial;
Material fishIncLogoMaterial;
Material sunMaterial;
Material astroidMaterial;
Material astroidSelectionMaterial;

FrameBuffer crtFrameBuffer;
FrameBuffer crt2FrameBuffer;

GameObject* roomAquariumDuckObj;
GameObject* roomParticleTestObj;

Canvas* gameOverCanvas;
Canvas* StartCanvas;
void Game::OnStart()
{
    crtFrameBuffer.init(275, 275, FrameBufferFormat::RGBA8, false);
    crt2FrameBuffer.init(275, 275, FrameBufferFormat::RGBA8, false);
    crtMesh.GenerateBuffers();
    roomMesh.GenerateBuffers();
    DuckMesh.GenerateBuffers();
    aquariumMesh.GenerateBuffers();
    sunMesh.GenerateBuffers();
    spaceSkySphereMesh.GenerateBuffers();
    astroidMesh.GenerateBuffers();
    fishIncLogoMesh.GenerateBuffers();
    astroidSelectionMesh.GenerateBuffers();
	roomDeskMesh.GenerateBuffers();
    FishMesh.GenerateBuffers();
    TextureElement::Init();



    Shader* fishIncLogoShader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.frag", true);
    fishIncLogoMaterial = Material(fishIncLogoShader);
    fishIncLogoMaterial.SetTexture("uAlbedoMap", "assets/textures/FishLogo.png", true);
    fishIncLogoMaterial.SetBool("uUseAlbedoMap", true);
    fishIncLogoMaterial.SetFloat("uRoughnessValue", 0.0f);
    fishIncLogoMaterial.SetFloat("uMetallicValue", 0.25f);


    

    Shader* earthShader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "assets/shaders/Earth/Earth.frag", true);
    earthMaterial = Material(earthShader);
    earthMaterial.SetTexture("earthInside", "assets/textures/Earth/2k_venus_surface.jpg", true);
    earthMaterial.SetTexture("earthDay", "assets/textures/Earth/2k_earth_daymap.jpg", false);
    earthMaterial.SetTexture("earthSpec", "assets/textures/Earth/2k_earth_specular_map.png", false);
    earthMaterial.SetTexture("earthCloud", "assets/textures/Earth/Earth-clouds.png", false);


    Shader* sunShader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.frag", true);
    sunMaterial = Material(sunShader);
    sunMaterial.SetTexture("uEmissionMap", "assets/textures/Earth/2k_venus_surface.jpg", true);
    sunMaterial.SetBool("uUseEmissionMap", true);
    sunMaterial.SetFloat("uEmissionIntensity", 3);

    Shader* duckShader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.frag", true);
    duckMaterial = Material(duckShader);
    duckMaterial.SetTexture("uAlbedoMap", "assets/textures/duck_albedo.png", true);
    duckMaterial.SetBool("uUseAlbedoMap", true);


    spaceSkySphereMaterial.shader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "engineassets/shaders/StandardGeometryStageShader/StandardUnlitShader.frag", true);
    spaceSkySphereMaterial.SetTexture("uAlbedoMap", "assets/textures/skybox.png", true);
    spaceSkySphereMaterial.SetBool("uUseAlbedoMap", true);
    spaceSkySphereMaterial.SetTexture("uEmissionMap", "assets/textures/skyboxEmission.png", true);
    spaceSkySphereMaterial.SetBool("uUseEmissionMap", true);
    spaceSkySphereMaterial.isLit = false;

    // Somewhere in your main loop / initialization:

    gameOverCanvas = new Canvas();
    {
        Texture* buttonTexture = new Texture("assets/textures/UI/GameOver.png");
        TextureElement* button = new TextureElement(buttonTexture);
        button->Position = Vector2(100, 100);
        button->Size = Vector2(200, 50);
        gameOverCanvas->AddElement(button);
    }

    StartCanvas = new Canvas();
    {
        Texture* buttonTexture = new Texture("assets/textures/UI/PressToStart.png");
        TextureElement* button = new TextureElement(buttonTexture);
        button->Position = Vector2(100, 100);
        button->Size = Vector2(200, 50);
        StartCanvas->AddElement(button);
    }
    


    Renderer::SetCanvas(StartCanvas);



    astroidMaterial.shader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShaderInstanced.vert", "engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.frag", true);
    astroidMaterial.SetTexture("uAlbedoMap", "assets/textures/Astroid/material_baseColor_Blurred.png", true);
    astroidMaterial.SetBool("uUseAlbedoMap", true);
    astroidMaterial.isLit = true;
    astroidMaterial.isTransparent = false;

    astroidSelectionMaterial.shader = new Shader("assets/shaders/AstroidShaders/AstroidPredictionShader.vert", "assets/shaders/AstroidShaders/AstroidPredictionShader.frag", true);
    astroidSelectionMaterial.isLit = false;
    astroidSelectionMaterial.isTransparent = false;

    
    roomMaterial.shader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.frag", true);
    roomMaterial.SetTexture("uAlbedoMap", "assets/textures/Plaster001_2K-PNG/Plaster001_2K-PNG_Color.png", true);
    roomMaterial.SetBool("uUseAlbedoMap", true);
    roomMaterial.SetBool("uUseNormalMap", true);
    roomMaterial.SetFloat("uNormalIntensity", 0.25f);

    roomDeskMaterial.shader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.frag", true);
    roomDeskMaterial.SetTexture("uAlbedoMap", "assets/textures/Wood051_2K-PNG/Wood051_2K-PNG_Color.png", true);
    roomDeskMaterial.SetBool("uUseAlbedoMap", true);
    roomDeskMaterial.SetTexture("uNormalMap", "assets/textures/Wood051_2K-PNG/Wood051_2K-PNG_NormalGL.png", true);
    roomDeskMaterial.SetBool("uUseNormalMap", true);
    roomDeskMaterial.SetFloat("uNormalIntensity", 0.25f);


    aquariumMaterial.shader = new Shader("Assets\\shaders\\Aquarium\\Aquarium.vert", "Assets\\shaders\\Aquarium\\Aquarium.frag", true);
    aquariumMaterial.isTransparent = true;

    crt1Material.shader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "Assets\\shaders\\CrtShader\\Crt.frag", true);
    crt1Material.SetTexture("Texture", "Assets/textures/CRT.png", true);
    crt1Material.SetFrameBufferTexture("CrtScreenTexture", &crtFrameBuffer);

    crt2Material.shader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "Assets\\shaders\\CrtShader\\Crt.frag", true);
    crt2Material.SetTexture("Texture", "Assets/textures/CRT.png", true);
    crt2Material.SetFrameBufferTexture("CrtScreenTexture", &crt2FrameBuffer);

    currentScene = new Scene();


    // room scene
    GameObject* roomObj = currentScene->createObject("room");
    roomObj->transform->localPosition = Vector3(-10000, 0, 0);
    roomObj->transform->MarkDirty();

    GameObject* roomCamObj = currentScene->createObject("roomCamera");
    roomCamObj->transform->SetParent(roomObj->transform);
    roomCamObj->transform->localPosition = Vector3(0, 1.75f, -.5f);
    roomCamObj->transform->MarkDirty();
    { // components
        Camera* roomCam = roomCamObj->addComponent<Camera>();
        roomCam->clearColor = Vector3(0.1f, 0.1f, 0.1f);
        roomCam->transform->localRotation = Quaternion::FromEuler(Math::Radians(Vector3(0, 180, 0)));
        Renderer::SetCamera(roomCam);

    }

    GameObject* roomCrt1Obj = currentScene->createObject("roomCrt1Obj");
    roomCrt1Obj->transform->SetParent(roomObj->transform);
    roomCrt1Obj->transform->localPosition = Vector3(0, 1, 1.35);
    roomCrt1Obj->transform->MarkDirty();
    { // components
        MeshRenderer* meshRenderer = roomCrt1Obj->addComponent<MeshRenderer>();
        meshRenderer->mesh = &crtMesh;
        meshRenderer->material = &crt1Material;
        meshRenderer->mesh->GenerateBuffers();
    }

    GameObject* roomCrt2Obj = currentScene->createObject("roomCrt2Obj");
    roomCrt2Obj->transform->SetParent(roomObj->transform);
    roomCrt2Obj->transform->localPosition = Vector3(-1, 1, 1.3);
    roomCrt2Obj->transform->localRotation = Quaternion::FromEuler(Math::Radians(Vector3(0, -20, 0)));
    roomCrt2Obj->transform->MarkDirty();
    { // components
        MeshRenderer* meshRenderer = roomCrt2Obj->addComponent<MeshRenderer>();
        meshRenderer->mesh = &crtMesh;
        meshRenderer->material = &crt2Material;
        meshRenderer->mesh->GenerateBuffers();
    }

    GameObject* roomAquariumObj = currentScene->createObject("roomAquariumObj");
    roomAquariumObj->transform->SetParent(roomObj->transform);
    roomAquariumObj->transform->localPosition = Vector3(1, 1, 1.2);
    roomAquariumObj->transform->MarkDirty();
    { // components
        MeshRenderer* meshRenderer = roomAquariumObj->addComponent<MeshRenderer>();
        meshRenderer->mesh = &aquariumMesh;
        meshRenderer->material = &aquariumMaterial;
        meshRenderer->mesh->GenerateBuffers();
    }

    roomAquariumDuckObj = currentScene->createObject("testLight");
    roomAquariumDuckObj->transform->SetParent(roomObj->transform);
    roomAquariumDuckObj->transform->localPosition = Vector3(1.4f, 1.5f, 1.4);
    roomAquariumDuckObj->transform->localScale = Vector3(.3f, .3f, .3f);
    roomAquariumDuckObj->transform->MarkDirty();
    { // components
        MeshRenderer* meshRenderer = roomAquariumDuckObj->addComponent<MeshRenderer>();
        meshRenderer->mesh = &DuckMesh;
        meshRenderer->material = &duckMaterial;
        DuckBounce* duck = roomAquariumDuckObj->addComponent<DuckBounce>();
    }

    GameObject* roomModelObj = currentScene->createObject("roomObj");
    roomModelObj->transform->SetParent(roomObj->transform);
    roomModelObj->transform->localPosition = Vector3(0, 0, 0);
    roomModelObj->transform->MarkDirty();
    { // components
        MeshRenderer* meshRenderer = roomModelObj->addComponent<MeshRenderer>();
        meshRenderer->mesh = &roomMesh;
        meshRenderer->material = &roomMaterial;
        meshRenderer->mesh->GenerateBuffers();
    }

    GameObject* RoomLightObj = currentScene->createObject("testLight");
    RoomLightObj->transform->SetParent(roomObj->transform);
    RoomLightObj->transform->localPosition = Vector3(0, 2, 0);
    RoomLightObj->transform->MarkDirty();
    { // components
        PointLight* pointLight = RoomLightObj->addComponent<PointLight>();
        pointLight->color = Vector3(0.9647058823529412f, 0.8941176470588236f, 0.7372549019607844f);
        pointLight->intensity = 1.95f;
        pointLight->radius = 20.0f;
    }

    GameObject* roomFishIncLogoObj = currentScene->createObject("roomFishIncLogoObj");
    roomFishIncLogoObj->transform->SetParent(roomObj->transform);
    roomFishIncLogoObj->transform->localPosition = Vector3(0, 2.55f, 1.4);
    roomFishIncLogoObj->transform->localScale = Vector3(.75f, .75f, .75f);
    roomFishIncLogoObj->transform->MarkDirty();
    { // components
        MeshRenderer* meshRenderer = roomFishIncLogoObj->addComponent<MeshRenderer>();
        meshRenderer->mesh = &fishIncLogoMesh;
        meshRenderer->material = &fishIncLogoMaterial;
    }

    GameObject* roomDeskObj = currentScene->createObject("roomDeskObj");
    roomDeskObj->transform->SetParent(roomObj->transform);
    roomDeskObj->transform->MarkDirty();
    { // components
        MeshRenderer* meshRenderer = roomDeskObj->addComponent<MeshRenderer>();
        meshRenderer->mesh = &roomDeskMesh;
        meshRenderer->material = &roomDeskMaterial;
    }

    roomParticleTestObj = currentScene->createObject("roomParticleTestObj");
    roomParticleTestObj->transform->localPosition = Vector3(0, 1, 1.35);
    roomParticleTestObj->transform->SetParent(roomObj->transform);
    roomParticleTestObj->transform->MarkDirty();
    { // components
        ParticleSystem* particleSystem = roomParticleTestObj->addComponent<ParticleSystem>();
        particleSystem->mesh = &astroidMesh;
        particleSystem->material = &astroidMaterial;
        particleSystem->baseAmount = 0;
        particleSystem->baseLifeTime = 1;
        particleSystem->baseSize = 2;
        particleSystem->baseSpeed = 50;
        particleSystem->randomAngularVelocity = true;
        particleSystem->randomVelocity = true;
        particleSystem->worldSpace = true;
        particleSystem->loop = false;

    }

    GameObject* roomBoidFishes = currentScene->createObject("FISHH");
    roomBoidFishes->transform->localPosition = Vector3(0,.25f,0);
    roomBoidFishes->transform->localScale = Vector3(1.0f,.35f,.1f);
    roomBoidFishes->transform->SetParent(roomAquariumObj->transform);
    roomBoidFishes->transform->MarkDirty();
    { // components
        MeshRendererInstanced* BoidRenderer = roomBoidFishes->addComponent<MeshRendererInstanced>();
        BoidRenderer->mesh = &FishMesh;
        BoidRenderer->material = &astroidMaterial;


        FishBoids* Boids = roomBoidFishes->addComponent<FishBoids>();
        Boids->Renderer = BoidRenderer;
        Boids->fishCount = 30;
        Boids->boundsMin = {-0.3f,-0.5f,-0.5f };
        Boids->boundsMax = { 0.5f,0.45f,0.5f };

    }

    // space scene
    GameObject* CameraOrbit = currentScene->createObject("CameraOrbit");

    GameObject* spacecamObj = currentScene->createObject("SpaceCam");
    spacecamObj->transform->SetParent(CameraOrbit->transform);
    spacecamObj->transform->localPosition = Vector3(0, 0, -200);
    spacecamObj->transform->MarkDirty();
    {
        spacecamObj->addComponent<OrbitCamLook>();
        Camera* cam = spacecamObj->addComponent<Camera>();
        cam->SetOutput(&crtFrameBuffer);
        //Renderer::SetCamera(cam);
    }
    GameObject* spaceAstroidcamObj = currentScene->createObject("SpaceCam");
    spaceAstroidcamObj->transform->localPosition = Vector3(0, 0, -200);
    spaceAstroidcamObj->transform->MarkDirty();
    {
        Camera* cam = spaceAstroidcamObj->addComponent<Camera>();
        cam->SetOutput(&crt2FrameBuffer);
    }

    GameObject* spaceEarth = currentScene->createObject("spaceEarth");
    EarthMeshGenerator* emg;
    spaceEarth->transform->MarkDirty();
    { // components
        MeshRenderer* meshRenderer = spaceEarth->addComponent<MeshRenderer>();
        meshRenderer->material = &earthMaterial;
        emg = spaceEarth->addComponent<EarthMeshGenerator>();
    }

    GameObject* spaceAstroidSpawner = currentScene->createObject("spaceAstroidSpawner");
    spaceAstroidSpawner->transform->MarkDirty();
    { // components
        MeshRendererInstanced* meshRendererInstanced = spaceAstroidSpawner->addComponent<MeshRendererInstanced>();
        meshRendererInstanced->material = &astroidMaterial;
        meshRendererInstanced->mesh = &astroidMesh;
        AstroidSpawner* spawner = spaceAstroidSpawner->addComponent<AstroidSpawner>();
        spawner->meshRendererInstanced = meshRendererInstanced;
        spawner->earthMeshGenerator = emg;
        spawner->gameovercanvas = gameOverCanvas;
        spawner->ps = roomParticleTestObj->getComponent<ParticleSystem>();
    }

    GameObject* SunOrbit = currentScene->createObject("CameraOrbit");
    {
        RotateOverTime* rotate = SunOrbit->addComponent<RotateOverTime>();
        rotate->rotationSpeed = Vector3(0, 10.0f, 0);
    }

    GameObject* spaceSun = currentScene->createObject("spaceSun");
    spaceSun->transform->SetParent(SunOrbit->transform);
    spaceSun->transform->localPosition = Vector3(750, 0, 0);
    spaceSun->transform->localScale = Vector3(5,5,5);
    spaceSun->transform->MarkDirty();
    { // components
        PointLight* pointLight = spaceSun->addComponent<PointLight>();
        pointLight->color = Vector3(0.9647058823529412f, 0.8941176470588236f, 0.7372549019607844f);
        pointLight->intensity = 4;
        pointLight->radius = 5000.0f;

        MeshRenderer* meshRenderer = spaceSun->addComponent<MeshRenderer>();
        meshRenderer->mesh = &sunMesh;
        meshRenderer->material = &sunMaterial;

    }
    GameObject* spaceAstroidSelect = currentScene->createObject("spaceAstroidSelect");
    {
        MeshRendererInstanced* meshRendererInstanced = spaceAstroidSpawner->addComponent<MeshRendererInstanced>();
        meshRendererInstanced->mesh = &astroidSelectionMesh;
        meshRendererInstanced->material = &astroidSelectionMaterial;

        AstroidSelection* astroidSelection = spaceAstroidSelect->addComponent<AstroidSelection>();
		astroidSelection->ps = roomParticleTestObj->getComponent<ParticleSystem>();
        astroidSelection->meshRendererInstanced = meshRendererInstanced;
        astroidSelection->earthGen = emg;
        astroidSelection->spaceCam = spacecamObj->getComponent<Camera>();
        astroidSelection->spawner = spaceAstroidSpawner->getComponent<AstroidSpawner>();
        astroidSelection->crt = roomCrt1Obj->transform;
        astroidSelection->roomCam = roomCamObj->transform;
        astroidSelection->AstroidCam = spaceAstroidcamObj->transform;
        astroidSelection->SunObj = spaceSun->transform;

    }

    GameObject* spaceSkySphere = currentScene->createObject("spaceSkySphere");
    spaceSkySphere->transform->localScale = Vector3(2,2,2);
    spaceSkySphere->transform->MarkDirty();
    { // components
        MeshRenderer* meshRenderer = spaceSkySphere->addComponent<MeshRenderer>();
        meshRenderer->mesh = &spaceSkySphereMesh;
        meshRenderer->material = &spaceSkySphereMaterial;
    }
}



void Game::OnUpdate()
{
    if (Input::Input::IsKeyDown(Input::Key::Escape))
    {
        Engine::Running = false;
    }
}
