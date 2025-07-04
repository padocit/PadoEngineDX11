#include "GeometryGenerator.h"

#include "ModelLoader.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

MeshData GeometryGenerator::MakeSquare(const float scale,
                                       const Vector2 texScale)
{
    vector<Vector3> positions;
    vector<Vector3> colors;
    vector<Vector3> normals;
    vector<Vector2> texcoords; // 텍스춰 좌표

    // 앞면
    positions.push_back(Vector3(-1.0f, 1.0f, 0.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 0.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 0.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, 0.0f) * scale);
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

    // Texture Coordinates (Direct3D 9)
    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/texture-coordinates
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    MeshData meshData;

    for (size_t i = 0; i < positions.size(); i++)
    {
        Vertex v;
        v.position = positions[i];
        v.normalModel = normals[i];
        v.texcoord = texcoords[i] * texScale;
        v.tangentModel = Vector3(1.0f, 0.0f, 0.0f);

        // v.color = colors[i];

        meshData.vertices.push_back(v);
    }
    meshData.indices = {
        0, 1, 2, 0, 2, 3, // 앞면
    };

    return meshData;
}

MeshData GeometryGenerator::MakeSquareTerrain(const float scale,
                                              const Vector2 texScale)
{
    vector<Vector3> positions;
    vector<Vector3> colors;
    vector<Vector3> normals;
    vector<Vector2> texcoords; // 텍스춰 좌표

    // 앞면
    positions.push_back(Vector3(-1.0f, 1.0f, 0.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 0.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, 0.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 0.0f) * scale);
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

    // Texture Coordinates (Direct3D 9)
    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/texture-coordinates
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));

    MeshData meshData;

    for (size_t i = 0; i < positions.size(); i++)
    {
        Vertex v;
        v.position = positions[i];
        v.normalModel = normals[i];
        v.texcoord = texcoords[i] * texScale;
        v.tangentModel = Vector3(1.0f, 0.0f, 0.0f);

        // v.color = colors[i];

        meshData.vertices.push_back(v);
    }
    meshData.indices = {
        0, 1, 2, 3, 
    };

    return meshData;
}

MeshData GeometryGenerator::MakeSquareGrid(const int numSlices,
                                           const int numStacks,
                                           const float scale,
                                           const Vector2 texScale)
{
    MeshData meshData;

    float dx = 2.0f / numSlices;
    float dy = 2.0f / numStacks;

    float y = 1.0f;
    for (int j = 0; j < numStacks + 1; j++)
    {
        float x = -1.0f;
        for (int i = 0; i < numSlices + 1; i++)
        {
            Vertex v;
            v.position = Vector3(x, y, 0.0f) * scale;
            v.normalModel = Vector3(0.0f, 0.0f, -1.0f);
            v.texcoord = Vector2(x + 1.0f, y + 1.0f) * 0.5f * texScale;
            v.tangentModel = Vector3(1.0f, 0.0f, 0.0f);

            meshData.vertices.push_back(v);

            x += dx;
        }
        y -= dy;
    }

    for (int j = 0; j < numStacks; j++)
    {
        for (int i = 0; i < numSlices; i++)
        {
            meshData.indices.push_back((numSlices + 1) * j + i);
            meshData.indices.push_back((numSlices + 1) * j + i + 1);
            meshData.indices.push_back((numSlices + 1) * (j + 1) + i);
            meshData.indices.push_back((numSlices + 1) * (j + 1) + i);
            meshData.indices.push_back((numSlices + 1) * j + i + 1);
            meshData.indices.push_back((numSlices + 1) * (j + 1) + i + 1);
        }
    }

    return meshData;
}

MeshData GeometryGenerator::MakeSquareGridTerrain(const int numSlices,
                                                  const int numStacks,
                                                  const float scale,
                                                  const Vector2 texScale)
{
    MeshData meshData;
    meshData.vertices.reserve(numSlices * numStacks * 4);

    const float dx = 2.0f / numSlices;
    const float dy = 2.0f / numStacks;
    const Vector3 normal = {0, 0, -1};
    const Vector3 tangent = {1, 0, 0};

    for (int j = 0; j < numStacks; ++j)
    {
        float y0 = 1.0f - j * dy; // top
        float y1 = y0 - dy;       // bottom

        for (int i = 0; i < numSlices; ++i)
        {
            float x0 = -1.0f + i * dx; // left
            float x1 = x0 + dx;        // right

            // UV 계산 (0,0)=top-left, (1,1)=bottom-right
            Vector2 uvTL = Vector2((x0 + 1) * 0.5f, (1 - y0) * 0.5f) * texScale;
            Vector2 uvTR = Vector2((x1 + 1) * 0.5f, (1 - y0) * 0.5f) * texScale;
            Vector2 uvBL = Vector2((x0 + 1) * 0.5f, (1 - y1) * 0.5f) * texScale;
            Vector2 uvBR = Vector2((x1 + 1) * 0.5f, (1 - y1) * 0.5f) * texScale;

            // TL
            meshData.vertices.push_back(
                {Vector3(x0, y0, 0) * scale, normal, uvTL, tangent});
            // TR
            meshData.vertices.push_back(
                {Vector3(x1, y0, 0) * scale, normal, uvTR, tangent});
            // BL
            meshData.vertices.push_back(
                {Vector3(x0, y1, 0) * scale, normal, uvBL, tangent});
            // BR
            meshData.vertices.push_back(
                {Vector3(x1, y1, 0) * scale, normal, uvBR, tangent});
        }
    }

    meshData.indices.resize(4 * numSlices * numStacks); // TODO: Draw를 사용하기 때문에 임시조치

    return meshData;
}



MeshData GeometryGenerator::MakeGrass()
{

    MeshData grid = MakeSquareGrid(1, 4);

    for (auto &v : grid.vertices)
    {

        // 적당히 가늘게 조절
        v.position.x *= 0.02f;

        // Y범위를 0~1로 조절
        v.position.y = v.position.y * 0.5f + 0.5f;
    }

    // 맨 위를 뾰족하게 만들기 위해 삼각형 하나와 정점 하나 삭제
    grid.indices.erase(grid.indices.begin(), grid.indices.begin() + 3);
    for (auto &i : grid.indices)
    {
        i -= 1;
    }
    grid.vertices.erase(grid.vertices.begin());
    grid.vertices[0].position.x = 0.0f;
    grid.vertices[0].texcoord.x = 0.5f;

    return grid;
}

MeshData GeometryGenerator::MakeBox(const float scale)
{

    vector<Vector3> positions;
    vector<Vector3> colors;
    vector<Vector3> normals;
    vector<Vector2> texcoords; // 텍스춰 좌표

    // 윗면
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // 아랫면
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // 앞면
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // 뒷면
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // 왼쪽
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // 오른쪽
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    MeshData meshData;
    for (size_t i = 0; i < positions.size(); i++)
    {
        Vertex v;
        v.position = positions[i];
        v.normalModel = normals[i];
        v.texcoord = texcoords[i];
        meshData.vertices.push_back(v);
    }

    meshData.indices = {
        0,  1,  2,  0,  2,  3,  // 윗면
        4,  5,  6,  4,  6,  7,  // 아랫면
        8,  9,  10, 8,  10, 11, // 앞면
        12, 13, 14, 12, 14, 15, // 뒷면
        16, 17, 18, 16, 18, 19, // 왼쪽
        20, 21, 22, 20, 22, 23  // 오른쪽
    };

    return meshData;
}

MeshData GeometryGenerator::MakeWireBox(const Vector3 center,
                                        const Vector3 extents)
{

    // 상자를 와이어 프레임으로 그리는 용도

    vector<Vector3> positions;

    // 앞면
    positions.push_back(center + Vector3(-1.0f, -1.0f, -1.0f) * extents);
    positions.push_back(center + Vector3(-1.0f, 1.0f, -1.0f) * extents);
    positions.push_back(center + Vector3(1.0f, 1.0f, -1.0f) * extents);
    positions.push_back(center + Vector3(1.0f, -1.0f, -1.0f) * extents);

    // 뒷면
    positions.push_back(center + Vector3(-1.0f, -1.0f, 1.0f) * extents);
    positions.push_back(center + Vector3(-1.0f, 1.0f, 1.0f) * extents);
    positions.push_back(center + Vector3(1.0f, 1.0f, 1.0f) * extents);
    positions.push_back(center + Vector3(1.0f, -1.0f, 1.0f) * extents);

    MeshData meshData;
    for (size_t i = 0; i < positions.size(); i++)
    {
        Vertex v;
        v.position = positions[i];
        v.normalModel = positions[i] - center;
        v.normalModel.Normalize();
        v.texcoord = Vector2(0.0f); // 미사용
        meshData.vertices.push_back(v);
    }

    // Line list
    meshData.indices = {
        0, 1, 1, 2, 2, 3, 3, 0, // 앞면
        4, 5, 5, 6, 6, 7, 7, 4, // 뒷면
        0, 4, 1, 5, 2, 6, 3, 7  // 옆면
    };

    return meshData;
}

MeshData GeometryGenerator::MakeWireSphere(const Vector3 center,
                                           const float radius)
{
    MeshData meshData;
    vector<Vertex> &vertices = meshData.vertices;
    vector<uint32_t> &indices = meshData.indices;

    const int numPoints = 30;
    const float dTheta = XM_2PI / float(numPoints);

    // XY plane
    {
        int offset = int(vertices.size());
        Vector3 start(1.0f, 0.0f, 0.0f);
        for (int i = 0; i < numPoints; i++)
        {
            Vertex v;
            v.position =
                center + Vector3::Transform(start, Matrix::CreateRotationZ(
                                                       dTheta * float(i))) *
                             radius;
            vertices.push_back(v);
            indices.push_back(i + offset);
            if (i != 0)
            {
                indices.push_back(i + offset);
            }
        }
        indices.push_back(offset);
    }

    // YZ
    {
        int offset = int(vertices.size());
        Vector3 start(0.0f, 1.0f, 0.0f);
        for (int i = 0; i < numPoints; i++)
        {
            Vertex v;
            v.position =
                center + Vector3::Transform(start, Matrix::CreateRotationX(
                                                       dTheta * float(i))) *
                             radius;
            vertices.push_back(v);
            indices.push_back(i + offset);
            if (i != 0)
            {
                indices.push_back(i + offset);
            }
        }
        indices.push_back(offset);
    }

    // XZ
    {
        int offset = int(vertices.size());
        Vector3 start(1.0f, 0.0f, 0.0f);
        for (int i = 0; i < numPoints; i++)
        {
            Vertex v;
            v.position =
                center + Vector3::Transform(start, Matrix::CreateRotationY(
                                                       dTheta * float(i))) *
                             radius;
            vertices.push_back(v);
            indices.push_back(i + offset);
            if (i != 0)
            {
                indices.push_back(i + offset);
            }
        }
        indices.push_back(offset);
    }

    // for (auto &v : vertices) {
    //     cout << v.position.x << " " << v.position.y << " " << v.position.z
    //          << endl;
    // }

    // for (int i = 0; i < indices.size(); i++) {
    //     cout << indices[i] << " ";
    // }
    // cout << endl;

    return meshData;
}

MeshData GeometryGenerator::MakeCylinder(const float bottomRadius,
                                         const float topRadius, float height,
                                         int numSlices)
{

    // Texture 좌표계때문에 (numSlices + 1) x 2 개의 버텍스 사용

    const float dTheta = -XM_2PI / float(numSlices);

    MeshData meshData;

    vector<Vertex> &vertices = meshData.vertices;

    // 옆면의 바닥 버텍스들 (인덱스 0 이상 numSlices 미만)
    for (int i = 0; i <= numSlices; i++)
    {
        Vertex v;
        v.position =
            Vector3::Transform(Vector3(bottomRadius, -0.5f * height, 0.0f),
                               Matrix::CreateRotationY(dTheta * float(i)));

        // std::cout << v.position.x << " " << v.position.z << std::endl;

        v.normalModel = v.position - Vector3(0.0f, -0.5f * height, 0.0f);
        v.normalModel.Normalize();
        v.texcoord = Vector2(float(i) / numSlices, 1.0f);

        vertices.push_back(v);
    }

    // 옆면의 맨 위 버텍스들 (인덱스 numSlices 이상 2 * numSlices 미만)
    for (int i = 0; i <= numSlices; i++)
    {
        Vertex v;
        v.position =
            Vector3::Transform(Vector3(topRadius, 0.5f * height, 0.0f),
                               Matrix::CreateRotationY(dTheta * float(i)));
        v.normalModel = v.position - Vector3(0.0f, 0.5f * height, 0.0f);
        v.normalModel.Normalize();
        v.texcoord = Vector2(float(i) / numSlices, 0.0f);

        vertices.push_back(v);
    }

    vector<uint32_t> &indices = meshData.indices;

    for (int i = 0; i < numSlices; i++)
    {
        indices.push_back(i);
        indices.push_back(i + numSlices + 1);
        indices.push_back(i + 1 + numSlices + 1);

        indices.push_back(i);
        indices.push_back(i + 1 + numSlices + 1);
        indices.push_back(i + 1);
    }

    return meshData;
}

MeshData GeometryGenerator::MakeSphere(const float radius, const int numSlices,
                                       const int numStacks,
                                       const Vector2 texScale)
{

    // 참고: OpenGL Sphere
    // http://www.songho.ca/opengl/gl_sphere.html
    // Texture 좌표계때문에 (numSlices + 1) 개의 버텍스 사용 (마지막에 닫아주는
    // 버텍스가 중복) Stack은 y 위쪽 방향으로 쌓아가는 방식

    const float dTheta = -XM_2PI / float(numSlices);
    const float dPhi = -XM_PI / float(numStacks);

    MeshData meshData;

    vector<Vertex> &vertices = meshData.vertices;

    for (int j = 0; j <= numStacks; j++)
    {

        // 스택에 쌓일 수록 시작점을 x-y 평면에서 회전 시켜서 위로 올리는 구조
        Vector3 stackStartPoint = Vector3::Transform(
            Vector3(0.0f, -radius, 0.0f), Matrix::CreateRotationZ(dPhi * j));

        for (int i = 0; i <= numSlices; i++)
        {
            Vertex v;

            // 시작점을 x-z 평면에서 회전시키면서 원을 만드는 구조
            v.position = Vector3::Transform(
                stackStartPoint, Matrix::CreateRotationY(dTheta * float(i)));

            v.normalModel = v.position; // 원점이 구의 중심
            v.normalModel.Normalize();
            v.texcoord =
                Vector2(float(i) / numSlices, 1.0f - float(j) / numStacks) *
                texScale;

            // texcoord가 위로 갈수록 증가
            Vector3 biTangent = Vector3(0.0f, 1.0f, 0.0f);

            Vector3 normalOrth =
                v.normalModel - biTangent.Dot(v.normalModel) * v.normalModel;
            normalOrth.Normalize();

            v.tangentModel = biTangent.Cross(normalOrth);
            v.tangentModel.Normalize();

            /*    Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f),
                                   Matrix::CreateRotationY(dTheta *
               float(i)));*/
            // v.biTangentModel = Vector3(0.0f, 1.0f, 0.0f);

            vertices.push_back(v);
        }
    }

    // cout << vertices.size() << endl;

    vector<uint32_t> &indices = meshData.indices;

    for (int j = 0; j < numStacks; j++)
    {

        const int offset = (numSlices + 1) * j;

        for (int i = 0; i < numSlices; i++)
        {

            indices.push_back(offset + i);
            indices.push_back(offset + i + numSlices + 1);
            indices.push_back(offset + i + 1 + numSlices + 1);

            indices.push_back(offset + i);
            indices.push_back(offset + i + 1 + numSlices + 1);
            indices.push_back(offset + i + 1);
        }
    }

    // cout << indices.size() << endl;
    // for (int i = 0; i < indices.size(); i++) {
    //     cout << indices[i] << " ";
    // }
    // cout << endl;

    return meshData;
}

MeshData GeometryGenerator::MakeIcosahedron()
{

    // Luna DX12 교재 참고
    // 등20면체
    // https://mathworld.wolfram.com/Isohedron.html

    const float X = 0.525731f;
    const float Z = 0.850651f;

    MeshData newMesh;

    vector<Vector3> pos = {
        Vector3(-X, 0.0f, Z), Vector3(X, 0.0f, Z),   Vector3(-X, 0.0f, -Z),
        Vector3(X, 0.0f, -Z), Vector3(0.0f, Z, X),   Vector3(0.0f, Z, -X),
        Vector3(0.0f, -Z, X), Vector3(0.0f, -Z, -X), Vector3(Z, X, 0.0f),
        Vector3(-Z, X, 0.0f), Vector3(Z, -X, 0.0f),  Vector3(-Z, -X, 0.0f)};

    for (size_t i = 0; i < pos.size(); i++)
    {
        Vertex v;
        v.position = pos[i];
        v.normalModel = v.position;
        v.normalModel.Normalize();

        newMesh.vertices.push_back(v);
    }

    newMesh.indices = {1,  4,  0, 4,  9, 0, 4, 5,  9, 8, 5, 4,  1,  8, 4,
                       1,  10, 8, 10, 3, 8, 8, 3,  5, 3, 2, 5,  3,  7, 2,
                       3,  10, 7, 10, 6, 7, 6, 11, 7, 6, 0, 11, 6,  1, 0,
                       10, 1,  6, 11, 0, 9, 2, 11, 9, 5, 2, 9,  11, 2, 7};

    return newMesh;
}

MeshData GeometryGenerator::MakeTetrahedron()
{

    // Regular Tetrahedron
    // https://mathworld.wolfram.com/RegularTetrahedron.html

    const float a = 1.0f;
    const float x = sqrt(3.0f) / 3.0f * a;
    const float d = sqrt(3.0f) / 6.0f * a; // = x / 2
    const float h = sqrt(6.0f) / 3.0f * a;

    vector<Vector3> points = {{0.0f, x, 0.0f},
                              {-0.5f * a, -d, 0.0f},
                              {+0.5f * a, -d, 0.0f},
                              {0.0f, 0.0f, h}};

    Vector3 center = Vector3(0.0f);

    for (int i = 0; i < 4; i++)
    {
        center += points[i];
    }
    center /= 4.0f;

    for (int i = 0; i < 4; i++)
    {
        points[i] -= center;
    }

    MeshData meshData;

    for (int i = 0; i < points.size(); i++)
    {

        Vertex v;
        v.position = points[i];
        v.normalModel = v.position; // 중심이 원점
        v.normalModel.Normalize();

        meshData.vertices.push_back(v);
    }

    meshData.indices = {0, 1, 2, 3, 2, 1, 0, 3, 1, 0, 2, 3};

    return meshData;
}
MeshData GeometryGenerator::SubdivideToSphere(const float radius,
                                              MeshData meshData)
{

    using namespace DirectX;
    using DirectX::SimpleMath::Matrix;
    using DirectX::SimpleMath::Vector3;

    // 원점이 중심이라고 가정
    for (auto &v : meshData.vertices)
    {
        v.position = v.normalModel * radius;
    }

    // 구의 표면으로 옮기고 노멀과 texture 좌표 계산
    auto ProjectVertex = [&](Vertex &v)
    {
        v.normalModel = v.position;
        v.normalModel.Normalize();
        v.position = v.normalModel * radius;

        // 주의: 텍스춰가 이음매에서 깨집니다.
        // atan vs atan2
        // https://stackoverflow.com/questions/283406/what-is-the-difference-between-atan-and-atan2-in-c
        // const float theta = atan2f(v.position.z, v.position.x);
        // const float phi = acosf(v.position.y / radius);
        // v.texcoord.x = theta / XM_2PI;
        // v.texcoord.y = phi / XM_PI;
    };

    auto UpdateFaceNormal = [](Vertex &v0, Vertex &v1, Vertex &v2)
    {
        auto faceNormal =
            (v1.position - v0.position).Cross(v2.position - v0.position);
        faceNormal.Normalize();
        v0.normalModel = faceNormal;
        v1.normalModel = faceNormal;
        v2.normalModel = faceNormal;
    };

    // 버텍스가 중복되는 구조로 구현
    MeshData newMesh;
    uint32_t count = 0;
    for (size_t i = 0; i < meshData.indices.size(); i += 3)
    {
        size_t i0 = meshData.indices[i];
        size_t i1 = meshData.indices[i + 1];
        size_t i2 = meshData.indices[i + 2];

        Vertex v0 = meshData.vertices[i0];
        Vertex v1 = meshData.vertices[i1];
        Vertex v2 = meshData.vertices[i2];

        Vertex v3;
        v3.position = (v0.position + v2.position) * 0.5f;
        v3.texcoord = (v0.texcoord + v2.texcoord) * 0.5f;
        ProjectVertex(v3);

        Vertex v4;
        v4.position = (v0.position + v1.position) * 0.5f;
        v4.texcoord = (v0.texcoord + v1.texcoord) * 0.5f;
        ProjectVertex(v4);

        Vertex v5;
        v5.position = (v1.position + v2.position) * 0.5f;
        v5.texcoord = (v1.texcoord + v2.texcoord) * 0.5f;
        ProjectVertex(v5);

        // UpdateFaceNormal(v4, v1, v5);
        // UpdateFaceNormal(v0, v4, v3);
        // UpdateFaceNormal(v3, v4, v5);
        // UpdateFaceNormal(v3, v5, v2);

        newMesh.vertices.push_back(v4);
        newMesh.vertices.push_back(v1);
        newMesh.vertices.push_back(v5);

        newMesh.vertices.push_back(v0);
        newMesh.vertices.push_back(v4);
        newMesh.vertices.push_back(v3);

        newMesh.vertices.push_back(v3);
        newMesh.vertices.push_back(v4);
        newMesh.vertices.push_back(v5);

        newMesh.vertices.push_back(v3);
        newMesh.vertices.push_back(v5);
        newMesh.vertices.push_back(v2);

        for (uint32_t j = 0; j < 12; j++)
        {
            newMesh.indices.push_back(j + count);
        }
        count += 12;
    }

    return newMesh;
}

void GeometryGenerator::Normalize(const Vector3 center,
                                  const float longestLength,
                                  vector<MeshData> &meshes,
                                  AnimationData &aniData)
{

    // 모델의 중심을 원점으로 옮기고 크기를 [-1,1]^3으로 스케일

    using namespace DirectX;

    // Normalize vertices
    Vector3 vmin(1000, 1000, 1000);
    Vector3 vmax(-1000, -1000, -1000);
    for (auto &mesh : meshes)
    {
        for (auto &v : mesh.vertices)
        {
            vmin.x = XMMin(vmin.x, v.position.x);
            vmin.y = XMMin(vmin.y, v.position.y);
            vmin.z = XMMin(vmin.z, v.position.z);
            vmax.x = XMMax(vmax.x, v.position.x);
            vmax.y = XMMax(vmax.y, v.position.y);
            vmax.z = XMMax(vmax.z, v.position.z);
        }
    }

    float dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
    float scale = longestLength / XMMax(XMMax(dx, dy), dz);
    Vector3 translation = -(vmin + vmax) * 0.5f + center;

    for (auto &mesh : meshes)
    {
        for (auto &v : mesh.vertices)
        {
            v.position = (v.position + translation) * scale;
        }

        for (auto &v : mesh.skinnedVertices)
        {
            v.position = (v.position + translation) * scale;
        }
    }

    // 애니메이션 데이터 보정에 사용
    aniData.defaultTransform =
        Matrix::CreateTranslation(translation) * Matrix::CreateScale(scale);
}

vector<MeshData> GeometryGenerator::ReadFromFile(std::string basePath,
                                                 std::string filename,
                                                 bool revertNormals)
{
    ModelLoader modelLoader;
    modelLoader.Load(basePath, filename, revertNormals);

    GeometryGenerator::Normalize(Vector3(0.0f), 1.0f, modelLoader.meshes,
                                 modelLoader.aniData);

    return modelLoader.meshes;
}

auto GeometryGenerator::ReadAnimationFromFile(string basePath, string filename,
                                              bool revertNormals)
    -> tuple<vector<MeshData>, AnimationData>
{

    ModelLoader modelLoader;
    modelLoader.Load(basePath, filename, revertNormals);

    GeometryGenerator::Normalize(Vector3(0.0f), 1.0f, modelLoader.meshes,
                                 modelLoader.aniData);

    return {modelLoader.meshes, modelLoader.aniData};
}
