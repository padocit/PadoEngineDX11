// Model
// Render model + Physics model

#pragma once

#include "ConstantBuffer.h"
#include "D3D11Utils.h"
#include "D3D11Common.h"
#include "Mesh.h"
#include "MeshData.h"

#include <directxtk/SimpleMath.h>

// 참고: DirectX-Graphics-Sampels
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Model/Model.h

class Actor
{
public:
    Actor() {}
    Actor(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
          const string &basePath, const string &filename);
    Actor(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
          const vector<MeshData> &meshes);

    virtual void Initialize(ComPtr<ID3D11Device> &device,
                            ComPtr<ID3D11DeviceContext> &context);

    virtual void InitMeshBuffers(ComPtr<ID3D11Device> &device,
                                 const MeshData &meshData,
                                 shared_ptr<Mesh> &newMesh);

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const string &basePath, const string &filename);

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const vector<MeshData> &meshes);

    void UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                               ComPtr<ID3D11DeviceContext> &context);

    virtual void SetPSO(const D3D11PSO &WirePSO, const D3D11PSO &SolidPSO);

    virtual D3D11PSO &GetPSO(const bool wired);
    virtual D3D11PSO &GetDepthOnlyPSO();
    virtual D3D11PSO &GetReflectPSO(const bool wired);

    virtual void Render(ComPtr<ID3D11DeviceContext> &context);
    virtual void UpdateAnimation(ComPtr<ID3D11DeviceContext> &context,
                                 int clipId, int frame);
    virtual void RenderNormals(ComPtr<ID3D11DeviceContext> &context);
    virtual void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context);
    virtual void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext> &context);
    void UpdateWorldRow(const Matrix &newWorldRow);

public:
    Matrix worldRow = Matrix();   // Model(Object) To World 행렬
    Matrix worldITRow = Matrix(); // InverseTranspose

    bool drawNormals = false;
    bool isVisible = true;
    bool castShadow = true;
    bool isPickable = false; // 마우스로 선택/조작 가능 여부

    vector<shared_ptr<Mesh>> meshes;

    ConstantBuffer<MeshConstants> meshConsts;
    ConstantBuffer<MaterialConstants> materialConsts;

    DirectX::BoundingBox boundingBox;
    DirectX::BoundingSphere boundingSphere;

    string name = "NoName";

    D3D11PSO wirePSO = Graphics::defaultSolidPSO;
    D3D11PSO solidPSO = Graphics::defaultWirePSO;

private:
    shared_ptr<Mesh> boundingBoxMesh;
    shared_ptr<Mesh> boundingSphereMesh;
};
