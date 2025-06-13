#include "Actor.h"
#include "GeometryGenerator.h"

#include <filesystem>

using namespace std;
using namespace DirectX;

Actor::Actor(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
             const std::string &basePath, const std::string &filename)
{
    Initialize(device, context, basePath, filename);
}

Actor::Actor(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
             const std::vector<MeshData> &meshes)
{
    Initialize(device, context, meshes);
}

void Actor::Initialize(ComPtr<ID3D11Device> &device,
                       ComPtr<ID3D11DeviceContext> &context)
{
    std::cout << "Model::Initialize(ComPtr<ID3D11Device> &device, "
                 "ComPtr<ID3D11DeviceContext> &context) was not implemented."
              << std::endl;
    exit(-1);
}

void Actor::InitMeshBuffers(ComPtr<ID3D11Device> &device,
                            const MeshData &meshData, shared_ptr<Mesh> &newMesh)
{

    D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
                                   newMesh->vertexBuffer);
    newMesh->indexCount = UINT(meshData.indices.size());
    newMesh->vertexCount = UINT(meshData.vertices.size());
    newMesh->stride = UINT(sizeof(Vertex));
    D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                                  newMesh->indexBuffer);
}

void Actor::Initialize(ComPtr<ID3D11Device> &device,
                       ComPtr<ID3D11DeviceContext> &context,
                       const std::string &basePath, const std::string &filename)
{
    auto meshes = GeometryGenerator::ReadFromFile(basePath, filename);
    Initialize(device, context, meshes);
}

BoundingBox GetBoundingBox(const vector<Vertex> &vertices)
{

    if (vertices.size() == 0)
        return BoundingBox();

    Vector3 minCorner = vertices[0].position;
    Vector3 maxCorner = vertices[0].position;

    for (size_t i = 1; i < vertices.size(); i++)
    {
        minCorner = Vector3::Min(minCorner, vertices[i].position);
        maxCorner = Vector3::Max(maxCorner, vertices[i].position);
    }

    Vector3 center = (minCorner + maxCorner) * 0.5f;
    Vector3 extents = maxCorner - center;

    return BoundingBox(center, extents);
}

void ExtendBoundingBox(const BoundingBox &inBox, BoundingBox &outBox)
{

    Vector3 minCorner = Vector3(inBox.Center) - Vector3(inBox.Extents);
    Vector3 maxCorner = Vector3(inBox.Center) - Vector3(inBox.Extents);

    minCorner = Vector3::Min(minCorner,
                             Vector3(outBox.Center) - Vector3(outBox.Extents));
    maxCorner = Vector3::Max(maxCorner,
                             Vector3(outBox.Center) + Vector3(outBox.Extents));

    outBox.Center = (minCorner + maxCorner) * 0.5f;
    outBox.Extents = maxCorner - outBox.Center;
}

void Actor::Initialize(ComPtr<ID3D11Device> &device,
                       ComPtr<ID3D11DeviceContext> &context,
                       const vector<MeshData> &meshes)
{

    // 일반적으로는 Mesh들이 mesh/materialConsts를 각자 소유 가능
    // 여기서는 한 Model 안의 여러 Mesh들이 Consts를 모두 공유

    meshConsts.GetCpu().world = Matrix();
    meshConsts.Initialize(device);
    materialConsts.Initialize(device);

    for (const auto &meshData : meshes)
    {
        auto newMesh = std::make_shared<Mesh>();

        InitMeshBuffers(device, meshData, newMesh);

        if (!meshData.albedoTextureFilename.empty())
        {
            if (filesystem::exists(meshData.albedoTextureFilename))
            {
                if (!meshData.opacityTextureFilename.empty())
                {
                    D3D11Utils::CreateTexture(
                        device, context, meshData.albedoTextureFilename,
                        meshData.opacityTextureFilename, false,
                        newMesh->albedoTexture, newMesh->albedoSRV);
                }
                else
                {
                    D3D11Utils::CreateTexture(
                        device, context, meshData.albedoTextureFilename, true,
                        newMesh->albedoTexture, newMesh->albedoSRV);
                }

                materialConsts.GetCpu().useAlbedoMap = true;
            }
            else
            {
                cout << meshData.albedoTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.emissiveTextureFilename.empty())
        {
            if (filesystem::exists(meshData.emissiveTextureFilename))
            {
                D3D11Utils::CreateTexture(
                    device, context, meshData.emissiveTextureFilename, true,
                    newMesh->emissiveTexture, newMesh->emissiveSRV);
                materialConsts.GetCpu().useEmissiveMap = true;
            }
            else
            {
                cout << meshData.emissiveTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.normalTextureFilename.empty())
        {
            if (filesystem::exists(meshData.normalTextureFilename))
            {
                D3D11Utils::CreateTexture(
                    device, context, meshData.normalTextureFilename, false,
                    newMesh->normalTexture, newMesh->normalSRV);
                materialConsts.GetCpu().useNormalMap = true;
            }
            else
            {
                cout << meshData.normalTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.heightTextureFilename.empty())
        {
            if (filesystem::exists(meshData.heightTextureFilename))
            {
                D3D11Utils::CreateTexture(
                    device, context, meshData.heightTextureFilename, false,
                    newMesh->heightTexture, newMesh->heightSRV);
                meshConsts.GetCpu().useHeightMap = true;
            }
            else
            {
                cout << meshData.heightTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.aoTextureFilename.empty())
        {
            if (filesystem::exists(meshData.aoTextureFilename))
            {
                D3D11Utils::CreateTexture(device, context,
                                          meshData.aoTextureFilename, false,
                                          newMesh->aoTexture, newMesh->aoSRV);
                materialConsts.GetCpu().useAOMap = true;
            }
            else
            {
                cout << meshData.aoTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        // GLTF 방식으로 Metallic과 Roughness를 한 텍스춰에 넣음
        // Green : Roughness, Blue : Metallic(Metalness)
        if (!meshData.metallicTextureFilename.empty() ||
            !meshData.roughnessTextureFilename.empty())
        {

            if (filesystem::exists(meshData.metallicTextureFilename) &&
                filesystem::exists(meshData.roughnessTextureFilename))
            {

                D3D11Utils::CreateMetallicRoughnessTexture(
                    device, context, meshData.metallicTextureFilename,
                    meshData.roughnessTextureFilename,
                    newMesh->metallicRoughnessTexture,
                    newMesh->metallicRoughnessSRV);
            }
            else
            {
                cout << meshData.metallicTextureFilename << " or "
                     << meshData.roughnessTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.metallicTextureFilename.empty())
        {
            materialConsts.GetCpu().useMetallicMap = true;
        }

        if (!meshData.roughnessTextureFilename.empty())
        {
            materialConsts.GetCpu().useRoughnessMap = true;
        }

        newMesh->meshConstsGPU = meshConsts.Get();
        newMesh->materialConstsGPU = materialConsts.Get();

        this->meshes.push_back(newMesh);
    }

    // Initialize Bounding Box
    {
        boundingBox = GetBoundingBox(meshes[0].vertices);
        for (size_t i = 1; i < meshes.size(); i++)
        {
            auto bb = GetBoundingBox(meshes[0].vertices);
            ExtendBoundingBox(bb, boundingBox);
        }
        auto meshData = GeometryGenerator::MakeWireBox(
            boundingBox.Center,
            Vector3(boundingBox.Extents) + Vector3(1e-3f));
        boundingBoxMesh = std::make_shared<Mesh>();
        D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
                                       boundingBoxMesh->vertexBuffer);
        boundingBoxMesh->indexCount = UINT(meshData.indices.size());
        boundingBoxMesh->vertexCount = UINT(meshData.vertices.size());
        boundingBoxMesh->stride = UINT(sizeof(Vertex));
        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                                      boundingBoxMesh->indexBuffer);
        boundingBoxMesh->meshConstsGPU = meshConsts.Get();
        boundingBoxMesh->materialConstsGPU = materialConsts.Get();
    }

    // Initialize Bounding Sphere
    {
        float maxRadius = 0.0f;
        for (auto &mesh : meshes)
        {
            for (auto &v : mesh.vertices)
            {
                maxRadius = max(
                    (Vector3(boundingBox.Center) - v.position).Length(),
                    maxRadius);
            }
        }
        maxRadius += 1e-2f; // 살짝 크게 설정
        boundingSphere = BoundingSphere(boundingBox.Center, maxRadius);
        auto meshData = GeometryGenerator::MakeWireSphere(
            boundingSphere.Center, boundingSphere.Radius);
        boundingSphereMesh = std::make_shared<Mesh>();
        D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
                                       boundingSphereMesh->vertexBuffer);
        boundingSphereMesh->indexCount = UINT(meshData.indices.size());
        boundingSphereMesh->vertexCount = UINT(meshData.vertices.size());
        boundingSphereMesh->stride = UINT(sizeof(Vertex));
        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                                      boundingSphereMesh->indexBuffer);
        boundingSphereMesh->meshConstsGPU = meshConsts.Get();
        boundingSphereMesh->materialConstsGPU = materialConsts.Get();
    }
}

void Actor::UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                                  ComPtr<ID3D11DeviceContext> &context)
{
    if (isVisible)
    {
        meshConsts.Upload(context);
        materialConsts.Upload(context);
    }
}

void Actor::SetPSO(const D3D11PSO &WirePSO, const D3D11PSO &SolidPSO)
{
    wirePSO = WirePSO;
    solidPSO = SolidPSO;
}

D3D11PSO &Actor::GetPSO(const bool wired)
{
    return wired ? wirePSO : solidPSO;
}

D3D11PSO &Actor::GetDepthOnlyPSO()
{
    return Graphics::depthOnlyPSO;
}

D3D11PSO &Actor::GetReflectPSO(const bool wired)
{
    return wired ? Graphics::reflectWirePSO : Graphics::reflectSolidPSO;
}

void Actor::Render(ComPtr<ID3D11DeviceContext> &context)
{
    if (isVisible)
    {
        for (const auto &mesh : meshes)
        {

            ID3D11Buffer *constBuffers[2] = {mesh->meshConstsGPU.Get(),
                                             mesh->materialConstsGPU.Get()};
            context->VSSetConstantBuffers(1, 2, constBuffers);

            context->VSSetShaderResources(0, 1, mesh->heightSRV.GetAddressOf());

            // 물체 렌더링할 때 여러가지 텍스춰 사용 (t0 부터시작)
            vector<ID3D11ShaderResourceView *> resViews = {
                mesh->albedoSRV.Get(), mesh->normalSRV.Get(), mesh->aoSRV.Get(),
                mesh->metallicRoughnessSRV.Get(), mesh->emissiveSRV.Get()};
            context->PSSetShaderResources(0, // register(t0)
                                          UINT(resViews.size()),
                                          resViews.data());
            context->PSSetConstantBuffers(1, 2, constBuffers);

            // Volume Rendering
            if (mesh->densityTex.GetSRV())
                context->PSSetShaderResources(
                    5, 1, mesh->densityTex.GetAddressOfSRV());
            if (mesh->lightingTex.GetSRV())
                context->PSSetShaderResources(
                    6, 1, mesh->lightingTex.GetAddressOfSRV());

            context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                        &mesh->stride, &mesh->offset);
            context->IASetIndexBuffer(mesh->indexBuffer.Get(),
                                      DXGI_FORMAT_R32_UINT, 0);

            context->DrawIndexed(mesh->indexCount, 0, 0);

            // Release resources
            ID3D11ShaderResourceView *nulls[3] = {NULL, NULL, NULL};
            context->PSSetShaderResources(5, 3, nulls);
        }
    }
}

void Actor::UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, int clipId,
                            int frame)
{
    // class SkinnedMeshModel에서 override
    cout << "Model::UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, "
            "int clipId, int frame) was not implemented."
         << endl;
    exit(-1);
}

void Actor::RenderNormals()
{
    ComPtr<ID3D11DeviceContext> &context =
        Engine::Get()->GetRenderer().GetContext();
    for (const auto &mesh : meshes)
    {
        ID3D11Buffer *constBuffers[2] = {mesh->meshConstsGPU.Get(),
                                         mesh->materialConstsGPU.Get()};
        context->GSSetConstantBuffers(1, 2, constBuffers);
        context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                    &mesh->stride, &mesh->offset);
        context->Draw(mesh->vertexCount, 0);
    }
}

void Actor::RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context)
{
    ID3D11Buffer *constBuffers[2] = {
        boundingBoxMesh->meshConstsGPU.Get(),
        boundingBoxMesh->materialConstsGPU.Get()};
    context->VSSetConstantBuffers(1, 2, constBuffers);
    context->IASetVertexBuffers(
        0, 1, boundingBoxMesh->vertexBuffer.GetAddressOf(),
        &boundingBoxMesh->stride, &boundingBoxMesh->offset);
    context->IASetIndexBuffer(boundingBoxMesh->indexBuffer.Get(),
                              DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(boundingBoxMesh->indexCount, 0, 0);
}

void Actor::RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext> &context)
{
    ID3D11Buffer *constBuffers[2] = {
        boundingBoxMesh->meshConstsGPU.Get(),
        boundingBoxMesh->materialConstsGPU.Get()};
    context->VSSetConstantBuffers(1, 2, constBuffers);
    context->IASetVertexBuffers(
        0, 1, boundingSphereMesh->vertexBuffer.GetAddressOf(),
        &boundingSphereMesh->stride, &boundingSphereMesh->offset);
    context->IASetIndexBuffer(boundingSphereMesh->indexBuffer.Get(),
                              DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(boundingSphereMesh->indexCount, 0, 0);
}

void Actor::UpdateWorldRow(const Matrix &newWorldRow)
{
    this->worldRow = newWorldRow;
    this->worldITRow = newWorldRow;
    worldITRow.Translation(Vector3(0.0f));
    worldITRow = worldITRow.Invert().Transpose();

    // 바운딩스피어 위치 업데이트
    // 스케일까지 고려하고 싶다면 x, y, z 스케일 중 가장 큰 값으로 스케일
    // 구(sphere)라서 회전은 고려할 필요 없음
    boundingSphere.Center = this->worldRow.Translation();

    meshConsts.GetCpu().world = newWorldRow.Transpose();
    meshConsts.GetCpu().worldIT = worldITRow.Transpose();
    meshConsts.GetCpu().worldInv = meshConsts.GetCpu().world.Invert();
}
