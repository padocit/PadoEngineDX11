#include "TerrainActor.h"

TerrainActor::TerrainActor() : Actor()
{
}

TerrainActor::TerrainActor(ComPtr<ID3D11Device> &device,
                           ComPtr<ID3D11DeviceContext> &context,
                           const string &basePath, const string &filename)
    : Actor(device, context, basePath, filename)
{
}

TerrainActor::TerrainActor(ComPtr<ID3D11Device> &device,
                           ComPtr<ID3D11DeviceContext> &context,
                           const vector<MeshData> &meshes)
    : Actor(device, context, meshes)
{
}

void TerrainActor::Render(ComPtr<ID3D11DeviceContext> &context)
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


            context->DSSetConstantBuffers(1, 2, constBuffers);
            context->DSSetShaderResources(0, 1, mesh->heightSRV.GetAddressOf());


            context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                        &mesh->stride, &mesh->offset);

            context->Draw(mesh->indexCount, 0);

            // Release resources
            ID3D11ShaderResourceView *nulls[3] = {NULL, NULL, NULL};
            context->PSSetShaderResources(5, 3, nulls);
        }
    }
}
