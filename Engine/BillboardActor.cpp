#include "BillboardActor.h"

#include <numeric>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using std::vector;

void BillboardActor::Initialize(ComPtr<ID3D11Device> &device,
                                ComPtr<ID3D11DeviceContext> &context,
                                const std::vector<Vector4> &points,
                                const float width,
                                const ComPtr<ID3D11PixelShader> &newPixelShader)
{

    BillboardActor::castShadow = false;

    D3D11Utils::CreateVertexBuffer(device, points, vertexBuffer);

    indexCount = uint32_t(points.size());
    vertexShader = Graphics::billboardVS;
    geometryShader = Graphics::billboardGS;
    inputLayout = Graphics::billboardIL;
    pixelShader = newPixelShader;

    billboardConsts.GetCpu().widthWorld = width;
    billboardConsts.Initialize(device);
    meshConsts.GetCpu().world = Matrix();
    meshConsts.Initialize(device);
    materialConsts.Initialize(device);
}

void BillboardActor::SetTexture(ComPtr<ID3D11Device> &device,
                                ComPtr<ID3D11DeviceContext> &context,
                                string filename)
{
    D3D11Utils::CreateTexture(device, context, filename, true, texture,
                              textureSRV);
}

void BillboardActor::Render(ComPtr<ID3D11DeviceContext> &context)
{
    if (isVisible)
    {
        // 편의상 PSO 설정을 Render()에서 바꾸는 방식
        context->IASetInputLayout(inputLayout.Get());
        context->VSSetShader(vertexShader.Get(), 0, 0);
        context->PSSetShader(pixelShader.Get(), 0, 0);
        context->PSSetShaderResources(0, 1, textureSRV.GetAddressOf());
        ID3D11Buffer *constBuffers[2] = {this->meshConsts.Get(),
                                         this->materialConsts.Get()};
        context->VSSetConstantBuffers(1, 2, constBuffers);
        context->VSSetConstantBuffers(3, 1, billboardConsts.GetAddressOf());
        context->PSSetConstantBuffers(3, 1, billboardConsts.GetAddressOf());
        context->GSSetConstantBuffers(3, 1, billboardConsts.GetAddressOf());
        context->GSSetShader(geometryShader.Get(), 0, 0);
        context->RSSetState(Graphics::solidBothRS.Get());
        context->OMSetBlendState(Graphics::alphaBS.Get(),
                                 Graphics::defaultSolidPSO.blendFactor,
                                 0xffffffff);
        UINT stride = sizeof(Vector4); // sizeof(Vertex);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(),
                                    &stride, &offset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        context->Draw(indexCount, 0);
        context->GSSetShader(NULL, 0, 0);
    }
}
