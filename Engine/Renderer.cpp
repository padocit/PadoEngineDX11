#include "Renderer.h"
#include "Vertex.h"

#include <iostream>

bool Renderer::Initialize(const Resolution &res, HWND hWindow)
{
    screenWidth = res.width;
    screenHeight = res.height;
    aspectRatio = res.AspectRatio();

    if (!InitDeviceAndSwapChain(hWindow))
    {
        std::cout << "Failed to Initialize D3D device and context."
                  << std::endl;
        return false;
    }

    InitBackBuffer();
    InitRasterizerStates();
    InitShaders();
    InitConstantBuffer();

    return true;
}

bool Renderer::InitDeviceAndSwapChain(HWND hWindow)
{
    const D3D_DRIVER_TYPE driverType =
        D3D_DRIVER_TYPE_HARDWARE; // D3D_DRIVER_TYPE_WARP;

    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_0, // 더 높은 버전이 먼저 오도록 설정
        D3D_FEATURE_LEVEL_9_3};
    D3D_FEATURE_LEVEL featureLevel;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = screenWidth;
    sd.BufferDesc.Height = screenHeight;
    sd.BufferDesc.Format = backBufferFormat;
    sd.BufferCount = 2;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT |
                     DXGI_USAGE_UNORDERED_ACCESS; // Compute Shader
    sd.OutputWindow = hWindow;
    sd.Windowed = TRUE;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    // sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //ImGui 폰트가
    // 두꺼워짐
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.SampleDesc.Count = 1; // _FLIP_은 MSAA 미지원
    sd.SampleDesc.Quality = 0;

    ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
        0, driverType, 0, createDeviceFlags, featureLevels, 1,
        D3D11_SDK_VERSION, &sd, swapChain.GetAddressOf(), device.GetAddressOf(),
        &featureLevel, context.GetAddressOf()));

    if (featureLevel != D3D_FEATURE_LEVEL_11_0)
    {
        std::cout << "D3D Feature Level 11 unsupported." << std::endl;
        return false;
    }

    return true;
}

void Renderer::InitBackBuffer()
{
    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    ThrowIfFailed(device->CreateRenderTargetView(backBuffer.Get(), NULL,
                                                 backBufferRTV.GetAddressOf()));
    ThrowIfFailed(device->CheckMultisampleQualityLevels(
        DXGI_FORMAT_R16G16B16A16_FLOAT, 4, &numQualityLevels));

    D3D11_TEXTURE2D_DESC desc;
    backBuffer->GetDesc(&desc);
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
}

void Renderer::InitRasterizerStates()
{
    D3D11_RASTERIZER_DESC rasterDesc = {};
    ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));
    rasterDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.MultisampleEnable = true;
    ThrowIfFailed(device->CreateRasterizerState(
        &rasterDesc, rasterizerState.GetAddressOf()));
}

void Renderer::InitShaders()
{
    ComPtr<ID3DBlob> vertexshaderCSO;
    ComPtr<ID3DBlob> pixelshaderCSO;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlobVS;
    HRESULT hr_VS = D3DCompileFromFile(
        L"BasicVS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "vs_5_0", compileFlags, 0, &vertexshaderCSO, &errorBlobVS);
    CheckResult(hr_VS, errorBlobVS.Get());

    device->CreateVertexShader(vertexshaderCSO->GetBufferPointer(),
                               vertexshaderCSO->GetBufferSize(), nullptr,
                               &basicVS);

    ComPtr<ID3DBlob> errorBlobPS;
    HRESULT hr_PS = D3DCompileFromFile(
        L"BasicPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "ps_5_0", compileFlags, 0, &pixelshaderCSO, &errorBlobPS);
    CheckResult(hr_PS, errorBlobPS.Get());

    device->CreatePixelShader(pixelshaderCSO->GetBufferPointer(),
                              pixelshaderCSO->GetBufferSize(), nullptr,
                              &basicPS);

    std::vector<D3D11_INPUT_ELEMENT_DESC> basicIEs = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D11_INPUT_PER_VERTEX_DATA, 0}};

    device->CreateInputLayout(basicIEs.data(), UINT(basicIEs.size()),
                              vertexshaderCSO->GetBufferPointer(),
                              vertexshaderCSO->GetBufferSize(), &basicIL);
}

void Renderer::InitConstantBuffer()
{
    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.ByteWidth = sizeof(constantData) + 0xf & 0xfffffff0; // multiple of 16 bytes
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA initData;
    ZeroMemory(&initData, sizeof(initData));
    initData.pSysMem = &constantData;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;

    ThrowIfFailed(
        device->CreateBuffer(&desc, &initData, constantBuffer.GetAddressOf()));
}

void Renderer::CreateMesh(const MeshData &meshData, Mesh &mesh)
{
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(Vertex) * 3;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = sizeof(Vertex);

    // Fill in the subresource data.
    D3D11_SUBRESOURCE_DATA vertexBufferData;
    vertexBufferData.pSysMem = meshData.vertices.data();
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;

    // Create the vertex buffer.
    ThrowIfFailed(device->CreateBuffer(&bufferDesc, &vertexBufferData,
                                       mesh.vertexBuffer.GetAddressOf()));

    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
    bufferDesc.ByteWidth = UINT(sizeof(uint32_t) * 3);
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.StructureByteStride = sizeof(uint32_t);

    D3D11_SUBRESOURCE_DATA indexBufferData = {0};
    indexBufferData.pSysMem = meshData.indices.data();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;

    device->CreateBuffer(&bufferDesc, &indexBufferData,
                         mesh.indexBuffer.GetAddressOf());

    // TODO: vertexcount, indexcount...
}

void Renderer::SetMainViewPort()
{
    // Set the viewport
    ZeroMemory(&screenViewport, sizeof(D3D11_VIEWPORT));
    screenViewport.TopLeftX = 0;
    screenViewport.TopLeftY = 0;
    screenViewport.Width = float(screenWidth);
    screenViewport.Height = float(screenHeight);
    screenViewport.MinDepth = 0.0f;
    screenViewport.MaxDepth = 1.0f;

    context->RSSetViewports(1, &screenViewport);
}

void Renderer::Render(const Mesh &mesh)
{
    SetMainViewPort();

    Prepare();
    PrepareShader();
    RenderPrimitive(mesh);
    SwapBuffer();
}

void Renderer::SwapBuffer()
{
    swapChain->Present(1, 0); // 1: VSync
}

void Renderer::Prepare()
{
    context->ClearRenderTargetView(backBufferRTV.Get(), clearColor);
    // https://learn.microsoft.com/ko-kr/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_primitive_topology
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->RSSetState(rasterizerState.Get());

    context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), nullptr);
    context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void Renderer::PrepareShader()
{
    context->VSSetShader(basicVS.Get(), nullptr, 0);
    context->PSSetShader(basicPS.Get(), nullptr, 0);
    context->IASetInputLayout(basicIL.Get());

    if (constantBuffer)
    {
        context->VSSetConstantBuffers(0, 1, &constantBuffer);
    }
}

void Renderer::RenderPrimitive(const Mesh &mesh)
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride,
                                &offset);
    context->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(3, 0, 0);
}

float Renderer::GetAspectRatio() const
{
    return static_cast<float>(screenWidth) / screenHeight;
}
