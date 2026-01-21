#include "EarthMeshGenerator.h"
#include "TriangleTable.h"
#include "Core/Scene/Components/MeshRenderer.h"
#include "Core/Scene/Gameobject.h"
#include <core/Rendering/Mesh/Mesh.h>
#include "Core/engine.h"


void EarthMeshGenerator::OnStart()
{
	mr = gameObject->getComponent<MeshRenderer>();
    innitiateVertexValues();
	marchingCubes();
}

void EarthMeshGenerator::OnUpdate()
{
}


void EarthMeshGenerator::innitiateVertexValues()
{
    // (gridCubes+1) samples on each axis
    const float radius = planetSize * 0.5f;               // world-space radius
    const float halfGrid = (float)gridCubes * 0.5f;
    const Vector3 centerGrid(halfGrid, halfGrid, halfGrid);

    // World-space center of the planet
    const Vector3 centerWS = centerGrid * Vector3(scale, scale, scale);

    for (int x = 0; x <= gridCubes; x++)
    {
        for (int y = 0; y <= gridCubes; y++)
        {
            for (int z = 0; z <= gridCubes; z++)
            {
                Vector3 pGrid((float)x, (float)y, (float)z);
                Vector3 pWS = pGrid * Vector3(scale, scale, scale);

                float dist = Math::Length(pWS - centerWS);

                // SDF of a sphere
                vertexValues[x][y][z] = dist - radius;
            }
        }
    }
}

// Interpolates points where SDF crosses the surface (uses glm helpers)
Vector3 InterpolateEdge(const Vector3& p1, const Vector3& p2, float valP1, float valP2)
{
    if (std::abs(valP1 - valP2) < 0.0001f)
        return p1;

    float t = valP1 / (valP1 - valP2);   // interpolation factor
    return Math::Lerp(p1, p2, t);          // GLM linear interpolation
}

Vector3 CalculateNormal(const Vector3& a, const Vector3& b, const Vector3& c)
{
    Vector3 edge1 = b - a;
    Vector3 edge2 = c - a;
    Vector3 normal = Math::Cross(edge1, edge2);
    return Math::Normalize(normal);
}

void EarthMeshGenerator::marchingCubes()
{
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<unsigned> indices;

    struct Vec3Key {
        float x, y, z;
        bool operator==(const Vec3Key& o) const { return x == o.x && y == o.y && z == o.z; }
    };
    struct Vec3KeyHash {
        size_t operator()(const Vec3Key& v) const {
            size_t h1 = std::hash<float>()(v.x);
            size_t h2 = std::hash<float>()(v.y);
            size_t h3 = std::hash<float>()(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    std::unordered_map<Vec3Key, unsigned, Vec3KeyHash> vertexMap;

    auto addVertex = [&](const Vector3& v) -> unsigned {
        Vec3Key key{ v.x, v.y, v.z };
        auto it = vertexMap.find(key);
        if (it != vertexMap.end())
            return it->second;

        unsigned index = (unsigned)vertices.size();
        vertices.push_back(v);
        normals.push_back(Vector3(0.0f, 0.0f, 0.0f));
        vertexMap[key] = index;
        return index;
        };

    const float halfGrid = gridCubes * 0.5f;
    const Vector3 centerGrid(halfGrid, halfGrid, halfGrid);

    for (int x = 0; x < gridCubes; x++)
    {
        for (int y = 0; y < gridCubes; y++)
        {
            for (int z = 0; z < gridCubes; z++)
            {
                float v[8];
                v[0] = vertexValues[x][y][z];
                v[1] = vertexValues[x + 1][y][z];
                v[2] = vertexValues[x + 1][y][z + 1];
                v[3] = vertexValues[x][y][z + 1];
                v[4] = vertexValues[x][y + 1][z];
                v[5] = vertexValues[x + 1][y + 1][z];
                v[6] = vertexValues[x + 1][y + 1][z + 1];
                v[7] = vertexValues[x][y + 1][z + 1];

                int CubeIndex = 0;
                if (v[0] < 0) CubeIndex |= 1;
                if (v[1] < 0) CubeIndex |= 2;
                if (v[2] < 0) CubeIndex |= 4;
                if (v[3] < 0) CubeIndex |= 8;
                if (v[4] < 0) CubeIndex |= 16;
                if (v[5] < 0) CubeIndex |= 32;
                if (v[6] < 0) CubeIndex |= 64;
                if (v[7] < 0) CubeIndex |= 128;

                if (CubeIndex == 0 || CubeIndex == 255)
                    continue;

                Vector3 P[8] = {
                    Vector3(x,     y,     z),
                    Vector3(x + 1, y,     z),
                    Vector3(x + 1, y,     z + 1),
                    Vector3(x,     y,     z + 1),
                    Vector3(x,     y + 1, z),
                    Vector3(x + 1, y + 1, z),
                    Vector3(x + 1, y + 1, z + 1),
                    Vector3(x,     y + 1, z + 1)
                };

                Vector3 E[12] = {
                    InterpolateEdge(P[0], P[1], v[0], v[1]),
                    InterpolateEdge(P[1], P[2], v[1], v[2]),
                    InterpolateEdge(P[2], P[3], v[2], v[3]),
                    InterpolateEdge(P[3], P[0], v[3], v[0]),
                    InterpolateEdge(P[4], P[5], v[4], v[5]),
                    InterpolateEdge(P[5], P[6], v[5], v[6]),
                    InterpolateEdge(P[6], P[7], v[6], v[7]),
                    InterpolateEdge(P[7], P[4], v[7], v[4]),
                    InterpolateEdge(P[0], P[4], v[0], v[4]),
                    InterpolateEdge(P[1], P[5], v[1], v[5]),
                    InterpolateEdge(P[2], P[6], v[2], v[6]),
                    InterpolateEdge(P[3], P[7], v[3], v[7])
                };

                for (int i = 0; i < 16; i += 3)
                {
                    int aIdx = triangle_table[CubeIndex][i];
                    if (aIdx == -1) break;

                    int bIdx = triangle_table[CubeIndex][i + 1];
                    int cIdx = triangle_table[CubeIndex][i + 2];

                    Vector3 a = (E[aIdx] - centerGrid) * Vector3(scale, scale, scale);
                    Vector3 b = (E[bIdx] - centerGrid) * Vector3(scale, scale, scale);
                    Vector3 c = (E[cIdx] - centerGrid) * Vector3(scale, scale, scale);

                    unsigned ia = addVertex(a);
                    unsigned ib = addVertex(b);
                    unsigned ic = addVertex(c);

                    indices.push_back(ia);
                    indices.push_back(ib);
                    indices.push_back(ic);

                    Vector3 faceN = CalculateNormal(a, b, c);
                    normals[ia] += faceN;
                    normals[ib] += faceN;
                    normals[ic] += faceN;
                }
            }
        }
    }

    for (int i = 0; i < normals.size(); i++)
    {
        normals[i] = Math::Normalize(vertices[i]);
        normals[i] = Math::Normalize(normals[i]);

    }

    if (planetMesh)
        delete planetMesh;

    planetMesh = new Mesh(vertices, indices, {}, normals, {});
    planetMesh->GenerateBuffers();
    mr->mesh = planetMesh;
}

void EarthMeshGenerator::carveSphereHole(const Vector3& sphereCenter, float radius)
{
    const int size = gridCubes + 1;

    for (int x = 0; x < size; x++)
    {
        for (int y = 0; y < size; y++)
        {
            for (int z = 0; z < size; z++)
            {
                // Convert voxel index to world space
                Vector3 pWS = Vector3(x, y, z) * Vector3(scale, scale, scale);

                // Sphere signed distance
                float sphereSDF = Math::Length(pWS - sphereCenter) - radius;

                // Subtract (carve out): push SDF toward "empty"
                vertexValues[x][y][z] = std::max(vertexValues[x][y][z], -sphereSDF);
            }
        }
    }
}
