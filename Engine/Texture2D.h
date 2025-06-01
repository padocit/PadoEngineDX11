#pragma once

#include "D3D11Utils.h"

// 참고
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Texture.h

class Texture2D
{
public:
    void Initialize(ComPtr<ID3D11Device> &device, UINT newWidth, UINT newHeight,
                    DXGI_FORMAT pixelFormat)
    {
        width = newWidth;
        height = newHeight;

        D3D11Utils::CreateUATexture(device, newWidth, newHeight, pixelFormat,
                                    texture, rtv, srv, uav);
    }

    void Upload(ComPtr<ID3D11Device> &device,
                ComPtr<ID3D11DeviceContext> &context,
                const vector<uint8_t> &data)
    {

        D3D11_TEXTURE2D_DESC desc;
        texture->GetDesc(&desc);

        if (!staging)
        {
            staging = D3D11Utils::CreateStagingTexture(
                device, context, desc.Width, desc.Height, data, desc.Format,
                desc.MipLevels, desc.ArraySize);
        }
        else
        {

            size_t pixelSize = D3D11Utils::GetPixelSize(desc.Format);

            D3D11_MAPPED_SUBRESOURCE ms;
            context->Map(staging.Get(), NULL, D3D11_MAP_WRITE, NULL, &ms);

            const uint8_t *src = (uint8_t *)data.data();
            uint8_t *dst = (uint8_t *)ms.pData;
            for (UINT j = 0; j < desc.Height; j++)
            {
                memcpy(&dst[j * ms.RowPitch],
                       &src[(j * desc.Width) * pixelSize],
                       desc.Width * pixelSize);
            }

            context->Unmap(staging.Get(), NULL);
        }

        context->CopyResource(texture.Get(), staging.Get());
    }

    void Download(ComPtr<ID3D11DeviceContext> &context, vector<uint8_t> &buffer)
    {

        context->CopyResource(staging.Get(), texture.Get());

        D3D11_MAPPED_SUBRESOURCE ms;
        context->Map(staging.Get(), NULL, D3D11_MAP_READ, NULL,
                     &ms); // D3D11_MAP_READ 주의
        memcpy(buffer.data(), (uint8_t *)ms.pData, buffer.size());
        context->Unmap(staging.Get(), NULL);
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

    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11Texture2D> staging;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<ID3D11ShaderResourceView> srv;
    ComPtr<ID3D11UnorderedAccessView> uav;
};
