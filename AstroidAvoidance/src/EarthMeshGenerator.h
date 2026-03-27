#pragma once
#include "Core/Scene/Component.h"
#include "Core/engine.h"

class MeshRenderer;
class Mesh;

// Generates a planet mesh using a 3D signed distance field (sphere)
// and the marching cubes algorithm.
// Also supports carving holes into the planet and regenerating the mesh.
class EarthMeshGenerator : public Component
{
public:
    // Grid resolution (number of cubes per axis)
    static const int gridCubes = 16;

    // Diameter of the planet in world units
    static const int planetSize = 150;

    // Scale factor applied to the generated mesh
    static const int scale = 10;

    // Generates mesh from current SDF values
    void marchingCubes();

    // Carves a spherical hole into the SDF and updates the mesh
    void carveSphereHole(const Vector3& sphereCenter, float radius);

private:
    void OnStart() override;

    MeshRenderer* mr = nullptr;

    // Generated mesh instance
    Mesh* planetMesh = nullptr;

    // 3D grid storing signed distance values (SDF)
    float vertexValues[gridCubes + 1][gridCubes + 1][gridCubes + 1] = {};

    // Initializes SDF values for a sphere
    void innitiateVertexValues();
};