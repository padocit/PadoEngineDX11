#pragma once
#include "Actor.h"
class TerrainActor : public Actor
{
public:
    TerrainActor();
    TerrainActor(ComPtr<ID3D11Device> &device,
                 ComPtr<ID3D11DeviceContext> &context, const string &basePath,
                 const string &filename);
    TerrainActor(ComPtr<ID3D11Device> &device,
                 ComPtr<ID3D11DeviceContext> &context,
                 const vector<MeshData> &meshes);

    void Render(ComPtr<ID3D11DeviceContext> &context) override;
};
