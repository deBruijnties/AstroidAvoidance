#pragma once
#include "Core/Scene/Component.h"
#include "Core/engine.h"
class MeshRenderer;
class Mesh;

// this script generates the planet based on a 3d point sphere function
// using the marchingcubes algorithm i generate a mesh based on the vertexvalues
// this class also contains the function to carve holes into the mesh vertexdata and then updates mesh.

class EarthMeshGenerator : public Component
{
public:
    static const int gridCubes = 16;
    static const int planetSize = 150;
    static const int scale = 10;

    void marchingCubes();
    void carveSphereHole(const Vector3& sphereCenter, float radius);
private:
  


    void OnStart() override;

    MeshRenderer* mr = nullptr;

    // Allocate storage for the static members
    Mesh* planetMesh = nullptr;
    //MarchingCubes Planet::marchingCubes;
    float vertexValues[gridCubes + 1][gridCubes + 1][gridCubes + 1] = {};

    void innitiateVertexValues();
   

};

