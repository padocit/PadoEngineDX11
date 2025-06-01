#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>
#include <wrl/client.h> // ComPtr
#include <directxtk/SimpleMath.h>

using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::vector;
using std::wstring;
using std::string;

// Create Shaders
// Create Buffers

inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        // ������� �� ���⿡ breakpoint ����
        throw std::exception();
    }
}

// All static functions
class D3D11Utils 
{
public:

    static void CreateVertexShaderAndInputLayout(
        ComPtr<ID3D11Device> &device, wstring filename,
        const vector<D3D11_INPUT_ELEMENT_DESC> &inputElements,
        ComPtr<ID3D11VertexShader> &m_vertexShader,
        ComPtr<ID3D11InputLayout> &m_inputLayout,
        const vector<D3D_SHADER_MACRO> shaderMacros = {/* Empty default */});
    // ShaderMacros ����� �� ����
    // {D3D_SHADER_MACRO("SKINNED", "1"), D3D_SHADER_MACRO(NULL, NULL)};
    // �� �ڿ� NULL, NULL �ʼ�

    static void CreateHullShader(ComPtr<ID3D11Device> &device,
                                 const wstring &filename,
                                 ComPtr<ID3D11HullShader> &m_hullShader);

    static void CreateDomainShader(ComPtr<ID3D11Device> &device,
                                   const wstring &filename,
                                   ComPtr<ID3D11DomainShader> &m_domainShader);

    static void
    CreateGeometryShader(ComPtr<ID3D11Device> &device, const wstring &filename,
                         ComPtr<ID3D11GeometryShader> &m_geometryShader);

    static void
    CreateComputeShader(ComPtr<ID3D11Device> &device, const wstring &filename,
                        ComPtr<ID3D11ComputeShader> &m_computeShader);

    static void CreatePixelShader(ComPtr<ID3D11Device> &device,
                                  const wstring &filename,
                                  ComPtr<ID3D11PixelShader> &m_pixelShader);

    static void CreateIndexBuffer(ComPtr<ID3D11Device> &device,
                                  const vector<uint32_t> &indices,
                                  ComPtr<ID3D11Buffer> &indexBuffer);


// templates

    template <typename T_VERTEX>
    static void CreateVertexBuffer(ComPtr<ID3D11Device> &device,
                                   const vector<T_VERTEX> &vertices,
                                   ComPtr<ID3D11Buffer> &vertexBuffer)
    {

        // D3D11_USAGE enumeration (d3d11.h)
        // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_usage

        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = UINT(sizeof(T_VERTEX) * vertices.size());
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
        bufferDesc.StructureByteStride = sizeof(T_VERTEX);

        D3D11_SUBRESOURCE_DATA vertexBufferData = {
            0}; // MS �������� �ʱ�ȭ�ϴ� ���
        vertexBufferData.pSysMem = vertices.data();
        vertexBufferData.SysMemPitch = 0;
        vertexBufferData.SysMemSlicePitch = 0;

        ThrowIfFailed(device->CreateBuffer(&bufferDesc, &vertexBufferData,
                                           vertexBuffer.GetAddressOf()));
    }

    template <typename T_INSTANCE>
    static void CreateInstanceBuffer(ComPtr<ID3D11Device> &device,
                                     const vector<T_INSTANCE> &instances,
                                     ComPtr<ID3D11Buffer> &instanceBuffer)
    {

        // �⺻������ VertexBuffer�� ����մϴ�.

        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // <- �ִϸ��̼ǿ� ���
        bufferDesc.ByteWidth = UINT(sizeof(T_INSTANCE) * instances.size());
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // <- CPU���� ����
        bufferDesc.StructureByteStride = sizeof(T_INSTANCE);

        D3D11_SUBRESOURCE_DATA vertexBufferData = {
            0}; // MS �������� �ʱ�ȭ�ϴ� ���
        vertexBufferData.pSysMem = instances.data();
        vertexBufferData.SysMemPitch = 0;
        vertexBufferData.SysMemSlicePitch = 0;

        ThrowIfFailed(device->CreateBuffer(&bufferDesc, &vertexBufferData,
                                           instanceBuffer.GetAddressOf()));
    }

    template <typename T_CONSTANT>
    static void CreateConstBuffer(ComPtr<ID3D11Device> &device,
                                  const T_CONSTANT &constantBufferData,
                                  ComPtr<ID3D11Buffer> &constantBuffer)
    {

        static_assert((sizeof(T_CONSTANT) % 16) == 0,
                      "Constant Buffer size must be 16-byte aligned");

        D3D11_BUFFER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.ByteWidth = sizeof(constantBufferData);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData;
        ZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = &constantBufferData;
        initData.SysMemPitch = 0;
        initData.SysMemSlicePitch = 0;

        ThrowIfFailed(device->CreateBuffer(&desc, &initData,
                                           constantBuffer.GetAddressOf()));
    }

    template <typename T_DATA>
    static void UpdateBuffer(ComPtr<ID3D11DeviceContext> &context,
                             const vector<T_DATA> &bufferData,
                             ComPtr<ID3D11Buffer> &buffer)
    {

        if (!buffer)
        {
            std::cout << "UpdateBuffer() buffer was not initialized."
                      << std::endl;
        }

        D3D11_MAPPED_SUBRESOURCE ms;
        context->Map(buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        memcpy(ms.pData, bufferData.data(), sizeof(T_DATA) * bufferData.size());
        context->Unmap(buffer.Get(), NULL);
    }

    template <typename T_DATA>
    static void UpdateBuffer(ComPtr<ID3D11DeviceContext> &context,
                             const T_DATA &bufferData,
                             ComPtr<ID3D11Buffer> &buffer)
    {

        if (!buffer)
        {
            std::cout << "UpdateBuffer() buffer was not initialized."
                      << std::endl;
        }

        D3D11_MAPPED_SUBRESOURCE ms;
        context->Map(buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        memcpy(ms.pData, &bufferData, sizeof(bufferData));
        context->Unmap(buffer.Get(), NULL);
    }

    static void
    CreateTexture(ComPtr<ID3D11Device> &device,
                  ComPtr<ID3D11DeviceContext> &context,
                  const std::string filename, const bool usSRGB,
                  ComPtr<ID3D11Texture2D> &texture,
                  ComPtr<ID3D11ShaderResourceView> &textureResourceView);

    static void CreateTexture(
        ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
        const std::string albedoFilename, const std::string opacityFilename,
        const bool usSRGB, ComPtr<ID3D11Texture2D> &texture,
        ComPtr<ID3D11ShaderResourceView> &textureResourceView);

    static void CreateUATexture(ComPtr<ID3D11Device> &device, const int width,
                                const int height, const DXGI_FORMAT pixelFormat,
                                ComPtr<ID3D11Texture2D> &texture,
                                ComPtr<ID3D11RenderTargetView> &rtv,
                                ComPtr<ID3D11ShaderResourceView> &srv,
                                ComPtr<ID3D11UnorderedAccessView> &uav);

    static void CreateTexture3D(ComPtr<ID3D11Device> &device, const int width,
                                const int height, const int depth,
                                const DXGI_FORMAT pixelFormat,
                                const vector<float> &initData,
                                ComPtr<ID3D11Texture3D> &texture,
                                ComPtr<ID3D11RenderTargetView> &rtv,
                                ComPtr<ID3D11ShaderResourceView> &srv,
                                ComPtr<ID3D11UnorderedAccessView> &uav);

    static void CreateStagingBuffer(ComPtr<ID3D11Device> &device,
                                    const UINT numElements,
                                    const UINT sizeElement,
                                    const void *initData,
                                    ComPtr<ID3D11Buffer> &buffer);

    static void CopyFromStagingBuffer(ComPtr<ID3D11DeviceContext> &context,
                                      ComPtr<ID3D11Buffer> &buffer, UINT size,
                                      void *dest);

    static void CopyToStagingBuffer(ComPtr<ID3D11DeviceContext> &context,
                                    ComPtr<ID3D11Buffer> &buffer, UINT size,
                                    void *src)
    {
        D3D11_MAPPED_SUBRESOURCE ms;
        context->Map(buffer.Get(), NULL, D3D11_MAP_WRITE, NULL, &ms);
        memcpy(ms.pData, src, size);
        context->Unmap(buffer.Get(), NULL);
    }

    static void CreateStructuredBuffer(ComPtr<ID3D11Device> &device,
                                       const UINT numElements,
                                       const UINT sizeElement,
                                       const void *initData,
                                       ComPtr<ID3D11Buffer> &buffer,
                                       ComPtr<ID3D11ShaderResourceView> &srv,
                                       ComPtr<ID3D11UnorderedAccessView> &uav);

    static void CreateIndirectArgsBuffer(ComPtr<ID3D11Device> &device,
                                         const UINT numElements,
                                         const UINT sizeElement,
                                         const void *initData,
                                         ComPtr<ID3D11Buffer> &buffer);

    static void CreateAppendBuffer(ComPtr<ID3D11Device> &device,
                                   const UINT numElements,
                                   const UINT sizeElement, const void *initData,
                                   ComPtr<ID3D11Buffer> &buffer,
                                   ComPtr<ID3D11ShaderResourceView> &srv,
                                   ComPtr<ID3D11UnorderedAccessView> &uav);

    static void CreateMetallicRoughnessTexture(
        ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
        const std::string metallicFiilename,
        const std::string roughnessFilename, ComPtr<ID3D11Texture2D> &texture,
        ComPtr<ID3D11ShaderResourceView> &srv);

    static void
    CreateTextureArray(ComPtr<ID3D11Device> &device,
                       ComPtr<ID3D11DeviceContext> &context,
                       const std::vector<std::string> filenames,
                       ComPtr<ID3D11Texture2D> &texture,
                       ComPtr<ID3D11ShaderResourceView> &textureResourceView);

    static void CreateDDSTexture(ComPtr<ID3D11Device> &device,
                                 const wchar_t *filename, const bool isCubeMap,
                                 ComPtr<ID3D11ShaderResourceView> &texResView);

    static ComPtr<ID3D11Texture2D>
    CreateStagingTexture(ComPtr<ID3D11Device> &device,
                         ComPtr<ID3D11DeviceContext> &context, const int width,
                         const int height, const std::vector<uint8_t> &image,
                         const DXGI_FORMAT pixelFormat, const int mipLevels,
                         const int arraySize);

    static ComPtr<ID3D11Texture3D>
    CreateStagingTexture3D(ComPtr<ID3D11Device> &device, const int width,
                           const int height, const int depth,
                           const DXGI_FORMAT pixelFormat);

    static void WriteToPngFile(ComPtr<ID3D11Device> &device,
                               ComPtr<ID3D11DeviceContext> &context,
                               ComPtr<ID3D11Texture2D> &textureToWrite,
                               const std::string filename);

    static size_t GetPixelSize(DXGI_FORMAT pixelFormat);

};