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


// Mesh loading
// these all load at startup, keep an eye on memory if more get added
Mesh DuckMesh = Mesh::LoadMeshFromFile("assets/models/duck.obj");
Mesh crtMesh = Mesh::LoadMeshFromFile("Assets/models/CRT.obj");
Mesh roomMesh = Mesh::LoadMeshFromFile("assets/models/Room.obj");
Mesh roomDeskMesh = Mesh::LoadMeshFromFile("assets/models/Desk.obj");
Mesh aquariumMesh = Mesh::LoadMeshFromFile("assets/models/Aquarium.obj");
Mesh sunMesh = Mesh::LoadMeshFromFile("assets/models/Sun.obj");
Mesh spaceSkySphereMesh = Mesh::LoadMeshFromFile("assets/models/skysphere.obj");
Mesh astroidMesh = Mesh::LoadMeshFromFile("assets/models/Asteroid.obj");
Mesh astroidSelectionMesh = Mesh::LoadMeshFromFile("assets/models/AsteroidPrediction.obj"); // the ring/preview thing that shows where an asteroid will hit
Mesh fishIncLogoMesh = Mesh::LoadMeshFromFile("assets/models/FishIncLogo.obj");
Mesh FishMesh = Mesh::LoadMeshFromFile("assets/models/fish.obj");

// Materials
// declared globally so they can be shared across objects without copying
Material spaceSkySphereMaterial;
Material duckMaterial;
Material crt1Material;
Material crt2Material;      // second CRT screen, shows the asteroid prediction camera
Material roomMaterial;
Material TestUIMaterial;    // leftover from testing, keeping it just in case
Material roomDeskMaterial;
Material aquariumMaterial;
Material earthMaterial;
Material fishIncLogoMaterial;
Material sunMaterial;
Material astroidMaterial;
Material astroidSelectionMaterial;

// Framebuffers
// each CRT screen in the room renders into one of these, then we slap it on the screen mesh as a texture
FrameBuffer crtFrameBuffer;
FrameBuffer crt2FrameBuffer;

// UI Canvases
Canvas* gameOverCanvas;
Canvas* StartCanvas;

void Game::OnStart()
{
    // CRT screens are 275x275 - small enough to look pixelated which fits the vibe
    crtFrameBuffer.init(275, 275, FrameBufferFormat::RGBA8, false);
    crt2FrameBuffer.init(275, 275, FrameBufferFormat::RGBA8, false);

    // push all mesh data up to the GPU
    // (has to happen after the GL context is ready, can't do it at load time)
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

    // one-time setup for the UI texture quad system
    TextureElement::Init();


    // Material setup
    // fair bit of boilerplate here, each material needs its own shader + textures wired up

    // shiny logo on the wall - slight metallic look so it pops
    fishIncLogoMaterial = Material(new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.frag", true));
    fishIncLogoMaterial.SetTexture("uAlbedoMap", "assets/textures/logo/FishLogo.png", true);
    fishIncLogoMaterial.SetBool("uUseAlbedoMap", true);
    fishIncLogoMaterial.SetFloat("uRoughnessValue", 0.0f);
    fishIncLogoMaterial.SetFloat("uMetallicValue", 0.25f);

    // earth has a custom frag shader to handle the cloud layer + day/night blend
    earthMaterial = Material(new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "assets/shaders/Earth/Earth.frag", true));
    earthMaterial.SetTexture("earthInside", "assets/textures/Earth/2k_venus_surface.jpg", true); // visible when earth gets hit / cracks open
    earthMaterial.SetTexture("earthDay", "assets/textures/Earth/2k_earth_daymap.jpg", false);
    earthMaterial.SetTexture("earthSpec", "assets/textures/Earth/2k_earth_specular_map.png", false);
    earthMaterial.SetTexture("earthCloud", "assets/textures/Earth/Earth-clouds.png", false);

    // sun glows using an emission map - boosted intensity so it actually looks like a star
    sunMaterial = Material(new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.frag", true));
    sunMaterial.SetTexture("uEmissionMap", "assets/textures/Earth/2k_venus_surface.jpg", true); // reusing venus texture, works surprisingly well
    sunMaterial.SetBool("uUseEmissionMap", true);
    sunMaterial.SetFloat("uEmissionIntensity", 3);

    duckMaterial = Material(new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.frag", true));
    duckMaterial.SetTexture("uAlbedoMap", "assets/textures/Duck/duck_albedo.png", true);
    duckMaterial.SetBool("uUseAlbedoMap", true);

    // skysphere is unlit - it's just a backdrop, lighting calculations on it would be wrong
    spaceSkySphereMaterial.shader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "engineassets/shaders/StandardGeometryStageShader/StandardUnlitShader.frag", true);
    spaceSkySphereMaterial.SetTexture("uAlbedoMap", "assets/textures/skybox/skybox.png", true);
    spaceSkySphereMaterial.SetBool("uUseAlbedoMap", true);
    spaceSkySphereMaterial.SetTexture("uEmissionMap", "assets/textures/skybox/skyboxEmission.png", true);
    spaceSkySphereMaterial.SetBool("uUseEmissionMap", true);
    spaceSkySphereMaterial.isLit = false;

    // asteroids use the instanced vert shader since there can be a LOT of them on screen at once
    astroidMaterial.shader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShaderInstanced.vert", "engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.frag", true);
    astroidMaterial.SetTexture("uAlbedoMap", "assets/textures/Astroid/material_baseColor_Blurred.png", true); // blurred because the asteroids spin fast
    astroidMaterial.SetBool("uUseAlbedoMap", true);
    astroidMaterial.isLit = true;
    astroidMaterial.isTransparent = false;

    // the selection indicator - custom shader handles the ring/preview visuals
    astroidSelectionMaterial.shader = new Shader("assets/shaders/AstroidShaders/AstroidPredictionShader.vert", "assets/shaders/AstroidShaders/AstroidPredictionShader.frag", true);
    astroidSelectionMaterial.isLit = false;
    astroidSelectionMaterial.isTransparent = false;

    // plain plaster walls - subtle normal map so it's not completely flat
    roomMaterial.shader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.frag", true);
    roomMaterial.SetTexture("uAlbedoMap", "assets/textures/Plaster001_2K-PNG/Plaster001_2K-PNG_Color.png", true);
    roomMaterial.SetBool("uUseAlbedoMap", true);
    roomMaterial.SetBool("uUseNormalMap", true);
    roomMaterial.SetFloat("uNormalIntensity", 0.25f); // very subtle, just enough to break up the flatness

    // wooden desk - same idea, low normal intensity so it doesn't look overdone
    roomDeskMaterial.shader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.frag", true);
    roomDeskMaterial.SetTexture("uAlbedoMap", "assets/textures/Wood051_2K-PNG/Wood051_2K-PNG_Color.png", true);
    roomDeskMaterial.SetBool("uUseAlbedoMap", true);
    roomDeskMaterial.SetTexture("uNormalMap", "assets/textures/Wood051_2K-PNG/Wood051_2K-PNG_NormalGL.png", true);
    roomDeskMaterial.SetBool("uUseNormalMap", true);
    roomDeskMaterial.SetFloat("uNormalIntensity", 0.25f);

    // aquarium glass - transparent, has its own custom shader for the refraction/tint
    aquariumMaterial.shader = new Shader("Assets\\shaders\\Aquarium\\Aquarium.vert", "Assets\\shaders\\Aquarium\\Aquarium.frag", true);
    aquariumMaterial.isTransparent = true;

    // CRT screens sample from the framebuffers above - this is what makes the screens show the game cameras
    crt1Material.shader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "Assets\\shaders\\CrtShader\\Crt.frag", true);
    crt1Material.SetTexture("Texture", "Assets/textures/crt/CRT.png", true); // the plastic casing / bezel texture
    crt1Material.SetFrameBufferTexture("CrtScreenTexture", &crtFrameBuffer);  // live feed from the space camera

    crt2Material.shader = new Shader("engineassets/shaders/StandardGeometryStageShader/StandardGeometryStageShader.vert", "Assets\\shaders\\CrtShader\\Crt.frag", true);
    crt2Material.SetTexture("Texture", "Assets/textures/crt/CRT.png", true);
    crt2Material.SetFrameBufferTexture("CrtScreenTexture", &crt2FrameBuffer); // live feed from the asteroid prediction camera


    // UI setup 
    // game over screen - shown when an asteroid gets through
    gameOverCanvas = new Canvas();
    {
        Texture* buttonTexture = new Texture("assets/textures/UI/GameOver.png");
        TextureElement* button = new TextureElement(buttonTexture);
        button->Position = Vector2(100, 100);
        button->Size = Vector2(200, 50);
        gameOverCanvas->AddElement(button);
    }

    // shown at startup before the player does anything
    StartCanvas = new Canvas();
    {
        Texture* buttonTexture = new Texture("assets/textures/UI/PressToStart.png");
        TextureElement* button = new TextureElement(buttonTexture);
        button->Position = Vector2(100, 100);
        button->Size = Vector2(200, 50);
        StartCanvas->AddElement(button);
    }

    // start with the title screen up
    Renderer::SetCanvas(StartCanvas);


    // SCENE SETUP
    currentScene = new Scene();


    // Room
    // the room is offset way off in world space so it doesn't clip with the space scene
    // both scenes share the same world - they're just very far apart
    GameObject* roomObj = currentScene->createObject("room");
    roomObj->transform->localPosition = Vector3(-10000, 0, 0);
    roomObj->transform->MarkDirty();

    // player camera - sits at roughly eye height, facing into the room
    GameObject* roomCamObj = currentScene->createObject("roomCamera");
    roomCamObj->transform->SetParent(roomObj->transform);
    roomCamObj->transform->localPosition = Vector3(0, 1.75f, -.5f);
    roomCamObj->transform->MarkDirty();
    { // components
        Camera* roomCam = roomCamObj->addComponent<Camera>();
        roomCam->clearColor = Vector3(0.1f, 0.1f, 0.1f);
        roomCam->transform->localRotation = Quaternion::FromEuler(Math::Radians(Vector3(0, 180, 0)));
        Renderer::SetCamera(roomCam); // this is the main camera the player sees through
    }

    // left CRT screen (shows the space overview camera)
    GameObject* roomCrt1Obj = currentScene->createObject("roomCrt1Obj");
    roomCrt1Obj->transform->SetParent(roomObj->transform);
    roomCrt1Obj->transform->localPosition = Vector3(0, 1, 1.35f);
    roomCrt1Obj->transform->MarkDirty();
    { // components
        MeshRenderer* meshRenderer = roomCrt1Obj->addComponent<MeshRenderer>();
        meshRenderer->mesh = &crtMesh;
        meshRenderer->material = &crt1Material;
        meshRenderer->mesh->GenerateBuffers();
    }

    // right CRT screen, angled slightly - shows asteroid prediction cam
    GameObject* roomCrt2Obj = currentScene->createObject("roomCrt2Obj");
    roomCrt2Obj->transform->SetParent(roomObj->transform);
    roomCrt2Obj->transform->localPosition = Vector3(-1, 1, 1.3f);
    roomCrt2Obj->transform->localRotation = Quaternion::FromEuler(Math::Radians(Vector3(0, -20, 0)));
    roomCrt2Obj->transform->MarkDirty();
    { // components
        MeshRenderer* meshRenderer = roomCrt2Obj->addComponent<MeshRenderer>();
        meshRenderer->mesh = &crtMesh;
        meshRenderer->material = &crt2Material;
        meshRenderer->mesh->GenerateBuffers();
    }

    // the fish tank sitting on the desk
    GameObject* roomAquariumObj = currentScene->createObject("roomAquariumObj");
    roomAquariumObj->transform->SetParent(roomObj->transform);
    roomAquariumObj->transform->localPosition = Vector3(1.f, 1.0f, 1.2f);
    roomAquariumObj->transform->MarkDirty();
    { // components
        MeshRenderer* meshRenderer = roomAquariumObj->addComponent<MeshRenderer>();
        meshRenderer->mesh = &aquariumMesh;
        meshRenderer->material = &aquariumMaterial;
        meshRenderer->mesh->GenerateBuffers();
    }

    // tiny duck bobbing in the aquarium - scaled down to 30% so it actually fits
    GameObject* roomAquariumDuckObj = currentScene->createObject("testLight"); // TODO: rename this object, leftover from testing
    roomAquariumDuckObj->transform->SetParent(roomObj->transform);
    roomAquariumDuckObj->transform->localPosition = Vector3(1.4f, 1.5f, 1.4f);
    roomAquariumDuckObj->transform->localScale = Vector3(.3f, .3f, .3f);
    roomAquariumDuckObj->transform->MarkDirty();
    { // components
        MeshRenderer* meshRenderer = roomAquariumDuckObj->addComponent<MeshRenderer>();
        meshRenderer->mesh = &DuckMesh;
        meshRenderer->material = &duckMaterial;
        DuckBounce* duck = roomAquariumDuckObj->addComponent<DuckBounce>(); // bobs up and down in the water
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

    // warm overhead light to make the room feel cozy - slightly yellow/orange tint
    GameObject* RoomLightObj = currentScene->createObject("testLight");
    RoomLightObj->transform->SetParent(roomObj->transform);
    RoomLightObj->transform->localPosition = Vector3(0, 2, 0);
    RoomLightObj->transform->MarkDirty();
    { // components
        PointLight* pointLight = RoomLightObj->addComponent<PointLight>();
        pointLight->color = Vector3(0.9647058823529412f, 0.8941176470588236f, 0.7372549019607844f); // warm white
        pointLight->intensity = 1.95f;
        pointLight->radius = 20.0f; // covers the whole room with a bit of falloff
    }

    // company logo mounted high on the wall behind the player
    GameObject* roomFishIncLogoObj = currentScene->createObject("roomFishIncLogoObj");
    roomFishIncLogoObj->transform->SetParent(roomObj->transform);
    roomFishIncLogoObj->transform->localPosition = Vector3(0, 2.55f, 1.4f);
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

    // particle system that fires off when an asteroid hits - positioned at the first CRT screen
    // loop is off so it only plays once per impact, gets triggered by AstroidSpawner
    GameObject* roomParticleTestObj = currentScene->createObject("roomParticleTestObj");
    roomParticleTestObj->transform->localPosition = Vector3(0.0f, 1.0f, 1.35f);
    roomParticleTestObj->transform->SetParent(roomObj->transform);
    roomParticleTestObj->transform->MarkDirty();
    { // components
        ParticleSystem* particleSystem = roomParticleTestObj->addComponent<ParticleSystem>();
        particleSystem->mesh = &astroidMesh;        // debris looks like little asteroid chunks
        particleSystem->material = &astroidMaterial;
        particleSystem->baseAmount = 0;             // starts with nothing, burst is triggered externally
        particleSystem->baseLifeTime = 1;
        particleSystem->baseSize = 2;
        particleSystem->baseSpeed = 50;
        particleSystem->randomAngularVelocity = true; // chunks tumble randomly
        particleSystem->randomVelocity = true;
        particleSystem->worldSpace = true;            // particles don't follow the parent transform
        particleSystem->loop = false;
    }

    // boid fish swimming around inside the aquarium
    // parented to the aquarium so they stay inside it if it ever moves
    GameObject* roomBoidFishes = currentScene->createObject("FISHH");
    roomBoidFishes->transform->localPosition = Vector3(0, .25f, 0);
    roomBoidFishes->transform->localScale = Vector3(1.0f, .35f, .1f); // flatten the bounding volume since it's a shallow tank
    roomBoidFishes->transform->SetParent(roomAquariumObj->transform);
    roomBoidFishes->transform->MarkDirty();
    { // components
        MeshRendererInstanced* BoidRenderer = roomBoidFishes->addComponent<MeshRendererInstanced>();
        BoidRenderer->mesh = &FishMesh;
        BoidRenderer->material = &astroidMaterial; // reusing asteroid material for now - fish texture TBD

        FishBoids* Boids = roomBoidFishes->addComponent<FishBoids>();
        Boids->Renderer = BoidRenderer;
        Boids->fishCount = 30;
        Boids->boundsMin = { -0.3f, -0.5f, -0.5f }; // keep them inside the tank walls
        Boids->boundsMax = { 0.5f,  0.45f, 0.5f };
    }


    // Space scene

    // empty pivot object that the space camera orbits around (centered on earth)
    GameObject* CameraOrbit = currentScene->createObject("CameraOrbit");

    // main space overview camera - pulls back 200 units so earth fills the screen nicely
    // renders into crtFrameBuffer which feeds CRT screen 1
    GameObject* spacecamObj = currentScene->createObject("SpaceCam");
    spacecamObj->transform->SetParent(CameraOrbit->transform);
    spacecamObj->transform->localPosition = Vector3(0, 0, -200);
    spacecamObj->transform->MarkDirty();
    {
        spacecamObj->addComponent<OrbitCamLook>(); // lets the player drag to rotate the view
        Camera* cam = spacecamObj->addComponent<Camera>();
        cam->SetOutput(&crtFrameBuffer);
        //Renderer::SetCamera(cam); // disabled - room cam is the active one
    }

    // second space camera, fixed position - used for the asteroid trajectory preview on CRT 2
    GameObject* spaceAstroidcamObj = currentScene->createObject("SpaceCam");
    spaceAstroidcamObj->transform->localPosition = Vector3(0, 0, -200);
    spaceAstroidcamObj->transform->MarkDirty();
    {
        Camera* cam = spaceAstroidcamObj->addComponent<Camera>();
        cam->SetOutput(&crt2FrameBuffer);
    }

    // the earth - mesh is generated procedurally at runtime by EarthMeshGenerator
    GameObject* spaceEarth = currentScene->createObject("spaceEarth");
    EarthMeshGenerator* emg; // keeping a pointer so the asteroid spawner can sample the surface
    spaceEarth->transform->MarkDirty();
    { // components
        MeshRenderer* meshRenderer = spaceEarth->addComponent<MeshRenderer>();
        meshRenderer->material = &earthMaterial;
        emg = spaceEarth->addComponent<EarthMeshGenerator>();
    }

    // handles spawning waves of asteroids and tracking game state
    GameObject* spaceAstroidSpawner = currentScene->createObject("spaceAstroidSpawner");
    spaceAstroidSpawner->transform->MarkDirty();
    { // components
        MeshRendererInstanced* meshRendererInstanced = spaceAstroidSpawner->addComponent<MeshRendererInstanced>();
        meshRendererInstanced->material = &astroidMaterial;
        meshRendererInstanced->mesh = &astroidMesh;
        AstroidSpawner* spawner = spaceAstroidSpawner->addComponent<AstroidSpawner>();
        spawner->meshRendererInstanced = meshRendererInstanced;
        spawner->earthMeshGenerator = emg;           // needs this to figure out what counts as a "hit"
        spawner->gameovercanvas = gameOverCanvas;     // shows this when the player loses
        spawner->ps = roomParticleTestObj->getComponent<ParticleSystem>(); // triggers the impact particles
    }

    // pivot for the sun - rotates slowly so the lighting on earth changes over time
    GameObject* SunOrbit = currentScene->createObject("CameraOrbit");
    {
        RotateOverTime* rotate = SunOrbit->addComponent<RotateOverTime>();
        rotate->rotationSpeed = Vector3(0, 10.0f, 0); // degrees per second, nice and slow
    }

    // the sun itself - massive radius light so it illuminates the whole scene
    GameObject* spaceSun = currentScene->createObject("spaceSun");
    spaceSun->transform->SetParent(SunOrbit->transform);
    spaceSun->transform->localPosition = Vector3(750, 0, 0); // orbits far out
    spaceSun->transform->localScale = Vector3(5, 5, 5);
    spaceSun->transform->MarkDirty();
    { // components
        PointLight* pointLight = spaceSun->addComponent<PointLight>();
        pointLight->color = Vector3(0.9647058823529412f, 0.8941176470588236f, 0.7372549019607844f); // warm sunlight color
        pointLight->intensity = 4;
        pointLight->radius = 5000.0f; // has to reach earth from 750 units away

        MeshRenderer* meshRenderer = spaceSun->addComponent<MeshRenderer>();
        meshRenderer->mesh = &sunMesh;
        meshRenderer->material = &sunMaterial;
    }

    // asteroid selection / click handling - lets the player target and destroy incoming asteroids
    GameObject* spaceAstroidSelect = currentScene->createObject("spaceAstroidSelect");
    {
        // note: this renderer is added to spaceAstroidSpawner, not spaceAstroidSelect
        // keeps the instanced draw calls on the same object as the spawner data
        MeshRendererInstanced* meshRendererInstanced = spaceAstroidSpawner->addComponent<MeshRendererInstanced>();
        meshRendererInstanced->mesh = &astroidSelectionMesh;
        meshRendererInstanced->material = &astroidSelectionMaterial;

        AstroidSelection* astroidSelection = spaceAstroidSelect->addComponent<AstroidSelection>();
        astroidSelection->ps = roomParticleTestObj->getComponent<ParticleSystem>(); // same impact particles
        astroidSelection->meshRendererInstanced = meshRendererInstanced;
        astroidSelection->earthGen = emg;
        astroidSelection->spaceCam = spacecamObj->getComponent<Camera>(); // raycasts through this camera
        astroidSelection->spawner = spaceAstroidSpawner->getComponent<AstroidSpawner>();
        astroidSelection->crt = roomCrt1Obj->transform;      // used for screen-space click mapping
        astroidSelection->roomCam = roomCamObj->transform;
        astroidSelection->AstroidCam = spaceAstroidcamObj->transform;
        astroidSelection->SunObj = spaceSun->transform;      // needed to calculate impact side / shadow
    }

    // giant sphere around the whole space scene - gives the illusion of a starfield
    // scaled up so nothing ever clips through it
    GameObject* spaceSkySphere = currentScene->createObject("spaceSkySphere");
    spaceSkySphere->transform->localScale = Vector3(2, 2, 2);
    spaceSkySphere->transform->MarkDirty();
    {
        MeshRenderer* meshRenderer = spaceSkySphere->addComponent<MeshRenderer>();
        meshRenderer->mesh = &spaceSkySphereMesh;
        meshRenderer->material = &spaceSkySphereMaterial;
    }
}


void Game::OnUpdate()
{
    // only quit hook for now - might want a pause menu eventually
    if (Input::Input::IsKeyDown(Input::Key::Escape))
    {
        Engine::Running = false;
    }
}