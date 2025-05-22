#pragma once

#include <d3d11.h>
#include <vector>
#include <wrl/client.h>

struct Mesh
{
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

    UINT vertexCount = 0;
    UINT indexCount = 0;
    UINT stride = 0;
    UINT offset = 0;
};