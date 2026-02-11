#include <vector>
#include <iostream>
#include <Core/Renderer/Renderer.h>
#include <Core/Renderer/Frontend/Mesh.h>
#include <Core/Renderer/Frontend/Shader.h>
#include <Core/Renderer/APIs/OpenGL/OpenGLMaterial.h>
#include <Core/Renderer/Frontend/Material.h>
#include <Core/Window/Window.h>
#include <Core/Math/Math.h>

using GalacticEngine::Mesh;
using GalacticEngine::Shader;
using GalacticEngine::Renderer;
using GalacticEngine::Material;
using GalacticEngine::OpenGLMaterial;
using GalacticEngine::GraphicsApi;
using GalacticEngine::Window;

namespace Math = GalacticEngine::Math;
using Vector2 = GalacticEngine::Vector2;
using Vector3 = GalacticEngine::Vector3;
using Vector4 = GalacticEngine::Vector4;
using Vector2Int = GalacticEngine::Vector2Int;
using Vector3Int = GalacticEngine::Vector3Int;
using Vector4Int = GalacticEngine::Vector4Int;
using Matrix4 = GalacticEngine::Matrix4;
using Quaternion = GalacticEngine::Quaternion;

int main()
{
    std::cout << "START GAME\n";

    Window window = Window(400, 400, "TEST MULTI ENGINE");

    Renderer::Innit(GraphicsApi::OpenGL);

    Shader* testShader = new Shader("assets/shaders/test.vert", "assets/shaders/test.frag");

    Material testMaterial = Material(testShader);

    std::vector<Vector3> verts =
    {
        Vector3(-0.5,-0.5,0),
        Vector3(-0.5,0.5,0),
        Vector3(0.5,-0.5,0),
        Vector3(0.5,0.5,0)
    };

    std::vector<unsigned int> tries =
    {
        0,1,2, // clockwise
        1,3,2, // clockwise
    };


    Mesh testMesh = Mesh("assets/models/CRT.obj");
    //Mesh testMesh = Mesh(verts, tries, {}, {}, {}, {}, false);

    Vector2Int size = window.GetSize();
    Matrix4 view = Matrix4::Identity();
    Matrix4 model = Matrix4::Identity().Translate(Vector3(0, 0, -10));
    Matrix4 projection = Math::perspective(Math::Radians(80),
        static_cast<float>(size.x) / static_cast<float>(size.y),
        0.01,
        200);


    testMaterial.SetMat4("u_view", view);
    testMaterial.SetMat4("u_model", model);
    testMaterial.SetMat4("u_projection", projection);


    Vector3 color(0, 1, 0);
    testMaterial.SetVec3("color", color);

    while (!window.ShouldClose())
    {
        GalacticEngine::RenderCommand cmd;
        cmd.mesh = &testMesh;
        cmd.material = &testMaterial;
        cmd.CastsShadow = true;

        Renderer::Submit(cmd);

        Renderer::Render();

        window.PollEvents();
    }
}
