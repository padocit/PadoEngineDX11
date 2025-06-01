#pragma once

#include "D3D11Utils.h"

// 참고: DirectX_Graphic-Samples 미니엔진
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/PipelineState.h

// 참고: D3D12_GRAPHICS_PIPELINE_STATE_DESC
// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_graphics_pipeline_state_desc

class D3D11PSO
{
public:
    void operator=(const D3D11PSO &pso);
    void SetBlendFactor(const float newBlendFactor[4]);

public:
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11HullShader> hullShader;
    ComPtr<ID3D11DomainShader> domainShader;
    ComPtr<ID3D11GeometryShader> geometryShader;
    ComPtr<ID3D11InputLayout> inputLayout;

    ComPtr<ID3D11BlendState> blendState;
    ComPtr<ID3D11DepthStencilState> depthStencilState;
    ComPtr<ID3D11RasterizerState> rasterizerState;

    float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    UINT stencilRef = 0;

    D3D11_PRIMITIVE_TOPOLOGY primitiveTopology =
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};
