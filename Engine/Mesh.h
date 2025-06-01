#pragma once

#include <d3d11.h>
#include <vector>
#include <wrl/client.h>

#include "Texture3D.h"

struct Mesh
{
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer> meshConstsGPU;
    Microsoft::WRL::ComPtr<ID3D11Buffer> materialConstsGPU;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> albedoTexture;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> emissiveTexture;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> normalTexture;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> heightTexture;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> aoTexture;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> metallicRoughnessTexture;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> albedoSRV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> emissiveSRV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalSRV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> heightSRV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> aoSRV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metallicRoughnessSRV;

    // 3D Textures
    Texture3D densityTex;
    Texture3D lightingTex;

    UINT vertexCount = 0;
    UINT indexCount = 0;
    UINT stride = 0;
    UINT offset = 0;
};