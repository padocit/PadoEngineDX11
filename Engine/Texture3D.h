#pragma once

#include "D3D11Utils.h"
#include <fp16.h>
#include <random>
#include <vector>

using std::vector;

// Âü°í
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Texture.h

class Texture3D {
  public:
    void Initialize(ComPtr<ID3D11Device> &device, UINT newWidth, UINT newHeight,
                    UINT newDepth, DXGI_FORMAT pixelFormat) {
        Initialize(device, newWidth, newHeight, newDepth, pixelFormat, {});
    }

    void Initialize(ComPtr<ID3D11Device> &device, UINT newWidth, UINT newHeight,
                    UINT newDepth, DXGI_FORMAT pixelFormat,
                    const vector<float> &initData) {

        width = newWidth;
        height = newHeight;
        depth = newDepth;

        D3D11Utils::CreateTexture3D(device, newWidth, newHeight, newDepth,
                                    pixelFormat,
                                    initData, texture, rtv, srv, uav);
    }

    void InitNoiseF16(ComPtr<ID3D11Device> &device) {

        using namespace std;

        DXGI_FORMAT_R16G16B16A16_FLOAT;

        const UINT width = 64;
        const UINT height = 1024;
        const UINT depth = 64;

        vector<float> f32(width * height * depth * 4);

        mt19937 gen(0);
        uniform_real_distribution<float> dp(0.0f, 1.0f);
        for (auto &f : f32) {
            f = dp(gen);
        }

        vector<float> f16(f32.size() / 2);

        uint16_t *f16Ptr = (uint16_t *)f16.data();
        for (int i = 0; i < f32.size(); i++) {
            f16Ptr[i] = fp16_ieee_from_fp32_value(f32[i]);
        }

        Initialize(device, width, height, depth, DXGI_FORMAT_R16G16B16A16_FLOAT,
                   f16);
    }

    void Upload(ComPtr<ID3D11Device> &device,
                ComPtr<ID3D11DeviceContext> &context,
                const vector<float> &data) {

        D3D11_TEXTURE3D_DESC desc;
        texture->GetDesc(&desc);

        if (!staging) {
            staging = D3D11Utils::CreateStagingTexture3D(
                device, desc.Width, desc.Height, desc.Depth, desc.Format);
        }

        size_t pixelSize = D3D11Utils::GetPixelSize(desc.Format);

        D3D11_MAPPED_SUBRESOURCE ms;
        context->Map(staging.Get(), NULL, D3D11_MAP_WRITE, NULL, &ms);
        const uint8_t *src = (uint8_t *)data.data();
        uint8_t *dst = (uint8_t *)ms.pData;
        for (UINT k = 0; k < desc.Depth; k++) {
            for (UINT j = 0; j < desc.Height; j++) {
                memcpy(&dst[j * ms.RowPitch + k * ms.DepthPitch],
                       &src[(j * desc.Width + k * desc.Width * desc.Height) *
                            pixelSize],
                       desc.Width * pixelSize);
            }
        }
        context->Unmap(staging.Get(), NULL);

        context->CopyResource(texture.Get(), staging.Get());
    }

    const auto GetTexture() { return texture.Get(); }
    const auto GetRTV() { return rtv.Get(); }
    const auto GetSRV() { return srv.Get(); }
    const auto GetUAV() { return uav.Get(); }
    const auto GetAddressOfRTV() { return rtv.GetAddressOf(); }
    const auto GetAddressOfSRV() { return srv.GetAddressOf(); }
    const auto GetAddressOfUAV() { return uav.GetAddressOf(); }

  private:
    UINT width = 1;
    UINT height = 1;
    UINT depth = 1;

    ComPtr<ID3D11Texture3D> texture;
    ComPtr<ID3D11Texture3D> staging;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<ID3D11ShaderResourceView> srv;
    ComPtr<ID3D11UnorderedAccessView> uav;
};
