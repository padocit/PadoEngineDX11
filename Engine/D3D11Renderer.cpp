#include "D3D11Renderer.h"
#include "D3D11Utils.h"
#include "D3D11Common.h"

bool D3D11Renderer::Initialize(const Resolution &resolution, HWND hWnd)
{
    SetScreenSize(resolution);

    if (!InitDeviceAndSwapChain(hWnd))
    {
        std::cout << "Failed to Initialize D3D device and context."
                  << std::endl;
        return false;
    }
     
    Graphics::InitCommonStates(device);

    CreateBuffers(); // backbuffer 포함

    SetMainViewPort();

    // 공통으로 쓰이는 cbuffers
    D3D11Utils::CreateConstBuffer(device, globalConstsCPU,
                                  globalConstsGPU);

    return true;
}

void D3D11Renderer::Update(Level* level, Camera* camera, float dt)
{
    const Vector3 eyeWorld = camera->GetEyePos();
    //const Matrix reflectRow = Matrix::CreateReflection(mirrorPlane);
    const Matrix viewRow = camera->GetViewRow();
    const Matrix projRow = camera->GetProjRow();

    // UpdateLights(dt);

    // 공용 cbuffer 업데이트
    UpdateGlobalConstants(dt, eyeWorld, viewRow, projRow);

    level->Update(device, context);
}

bool D3D11Renderer::InitDeviceAndSwapChain(HWND hWnd)
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
    sd.OutputWindow = hWnd;
    sd.Windowed = TRUE;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    // sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //ImGui 폰트가
    // 두꺼워짐
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    if (numQualityLevels > 0)
    {
        sd.SampleDesc.Count = 4; // how many multisamples
        sd.SampleDesc.Quality = numQualityLevels - 1;
    }
    else
    {
        sd.SampleDesc.Count = 1; // how many multisamples
        sd.SampleDesc.Quality = 0;
    }

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

void D3D11Renderer::CreateBuffers()
{
    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    ThrowIfFailed(device->CreateRenderTargetView(backBuffer.Get(), NULL,
                                                 backBufferRTV.GetAddressOf()));
    //ThrowIfFailed(device->CheckMultisampleQualityLevels(
    //    DXGI_FORMAT_R16G16B16A16_FLOAT, 4, &numQualityLevels)); // HDR + post process

    CreateDepthBuffers();
}

void D3D11Renderer::CreateDepthBuffers()
{
    // DepthStencilView 만들기
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = screenWidth;
    desc.Height = screenHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    if (useMSAA && numQualityLevels > 0)
    {
        desc.SampleDesc.Count = 4;
        desc.SampleDesc.Quality = numQualityLevels - 1;
    }
    else
    {
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
    }
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    ComPtr<ID3D11Texture2D> depthStencilBuffer;
    ThrowIfFailed(
        device->CreateTexture2D(&desc, 0, depthStencilBuffer.GetAddressOf()));
    ThrowIfFailed(device->CreateDepthStencilView(depthStencilBuffer.Get(), NULL,
                                                 defaultDSV.GetAddressOf()));
}

void D3D11Renderer::SetScreenSize(const Resolution& resolution)
{
    screenWidth = resolution.width;
    screenHeight = resolution.height;
    aspectRatio = resolution.AspectRatio();
}

void D3D11Renderer::SetMainViewPort()
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

void D3D11Renderer::Render(Level* level)
{
    SetMainViewPort();

    // 공통으로 사용하는 샘플러 설정
    context->VSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                           Graphics::sampleStates.data());
    context->PSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());
    
    // 공통으로 사용할 텍스춰들: "Common.hlsli"에서 register(t10)부터 시작
    //vector<ID3D11ShaderResourceView *> commonSRVs = {
    //    m_envSRV.Get(), m_specularSRV.Get(), m_irradianceSRV.Get(),
    //    m_brdfSRV.Get()};
    //m_context->PSSetShaderResources(10, UINT(commonSRVs.size()),
    //                                commonSRVs.data());

    Prepare(); 
     
    SetGlobalConsts(globalConstsGPU);
    level->Render(context, drawAsWire);

    context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), NULL);
}

void D3D11Renderer::SwapBuffer()
{
    swapChain->Present(1, 0); // 1: VSync
}

void D3D11Renderer::UpdateGlobalConstants(const float &dt,
                                          const Vector3 &eyeWorld,
                                          const Matrix &viewRow,
                                          const Matrix &projRow)
                                          //const Matrix &refl)
{
    globalConstsCPU.globalTime += dt;
    globalConstsCPU.eyeWorld = eyeWorld;
    globalConstsCPU.view = viewRow.Transpose();
    globalConstsCPU.proj = projRow.Transpose();
    globalConstsCPU.invProj = projRow.Invert().Transpose();
    globalConstsCPU.viewProj = (viewRow * projRow).Transpose();
    globalConstsCPU.invView = viewRow.Invert().Transpose();

    // 그림자 렌더링에 사용
    globalConstsCPU.invViewProj = globalConstsCPU.viewProj.Invert();

    D3D11Utils::UpdateBuffer(context, globalConstsCPU, globalConstsGPU);
}

void D3D11Renderer::SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU)
{
    // 쉐이더와 일관성 유지 cbuffer GlobalConstants : register(b0)
    context->VSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
    context->GSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
}


void D3D11Renderer::SetPipelineState(const D3D11PSO &pso)
{
    context->VSSetShader(pso.vertexShader.Get(), 0, 0);
    context->PSSetShader(pso.pixelShader.Get(), 0, 0);
    context->HSSetShader(pso.hullShader.Get(), 0, 0);
    context->DSSetShader(pso.domainShader.Get(), 0, 0);
    context->GSSetShader(pso.geometryShader.Get(), 0, 0);
    context->CSSetShader(NULL, 0, 0);
    context->IASetInputLayout(pso.inputLayout.Get());
    context->RSSetState(pso.rasterizerState.Get());
    context->OMSetBlendState(pso.blendState.Get(), pso.blendFactor,
                             0xffffffff);
    context->OMSetDepthStencilState(pso.depthStencilState.Get(),
                                    pso.stencilRef);
    context->IASetPrimitiveTopology(pso.primitiveTopology);
}

void D3D11Renderer::Prepare()
{ 
    context->ClearRenderTargetView(backBufferRTV.Get(), clearColor);
    // https://learn.microsoft.com/ko-kr/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_primitive_topology
    context->ClearDepthStencilView(
        defaultDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), defaultDSV.Get());
}

float D3D11Renderer::GetAspectRatio() const
{
    return float(screenWidth - guiWidth) / screenHeight;
}
