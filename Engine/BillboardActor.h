#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

#include "Actor.h"

struct BillboardConsts
{
    float widthWorld;
    Vector3 directionWorld;
};

static_assert((sizeof(BillboardConsts) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");

class BillboardActor : public Actor
{
public:
    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const std::vector<DirectX::SimpleMath::Vector4> &points,
                    const float width,
                    const ComPtr<ID3D11PixelShader> &newPixelShader);

    void SetTexture(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context, string filename);

    void Render(ComPtr<ID3D11DeviceContext> &context) override;
    void RenderNormals() override {};
    void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context) override {};
    void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext> &context) override {};

public:
    ConstantBuffer<BillboardConsts> billboardConsts;

protected:
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11GeometryShader> geometryShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11InputLayout> inputLayout;

    uint32_t indexCount = 0;

    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> textureSRV;
};
