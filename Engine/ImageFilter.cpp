#include "ImageFilter.h"


ImageFilter::ImageFilter(ComPtr<ID3D11Device> &device,
                         ComPtr<ID3D11DeviceContext> &context,
                         ComPtr<ID3D11PixelShader> &pixelShader, int width,
                         int height)
{
    Initialize(device, context, pixelShader, width, height);
}

void ImageFilter::Initialize(ComPtr<ID3D11Device> &device,
                             ComPtr<ID3D11DeviceContext> &context,
                             ComPtr<ID3D11PixelShader> &PixelShader, int width,
                             int height)
{

    ThrowIfFailed(PixelShader.CopyTo(pixelShader.GetAddressOf()));

    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = float(width);
    viewport.Height = float(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    constData.dx = 1.0f / width;
    constData.dy = 1.0f / height;

    D3D11Utils::CreateConstBuffer(device, constData, constBuffer);
}

void ImageFilter::UpdateConstantBuffers(ComPtr<ID3D11DeviceContext> &context)
{

    D3D11Utils::UpdateBuffer(context, constData, constBuffer);
}

void ImageFilter::Render(ComPtr<ID3D11DeviceContext> &context) const
{

    assert(shaderResources.size() > 0);
    assert(renderTargets.size() > 0);

    context->RSSetViewports(1, &viewport);
    context->OMSetRenderTargets(UINT(renderTargets.size()),
                                renderTargets.data(), NULL);
    context->PSSetShader(pixelShader.Get(), 0, 0);
    context->PSSetShaderResources(0, UINT(shaderResources.size()),
                                  shaderResources.data());
    context->PSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());
}

void ImageFilter::SetShaderResources(
    const std::vector<ComPtr<ID3D11ShaderResourceView>> &resources)
{

    shaderResources.clear();
    for (const auto &res : resources)
    {
        shaderResources.push_back(res.Get());
    }
}

void ImageFilter::SetRenderTargets(
    const std::vector<ComPtr<ID3D11RenderTargetView>> &targets)
{

    renderTargets.clear();
    for (const auto &tar : targets)
    {
        renderTargets.push_back(tar.Get());
    }
}
