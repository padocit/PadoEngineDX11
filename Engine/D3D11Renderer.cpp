#include "D3D11Renderer.h"
#include "D3D11Utils.h"
#include "D3D11Common.h"
#include "GeometryGenerator.h"
#include "Actor.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

bool D3D11Renderer::Initialize(const Resolution &resolution, HWND hWnd)
{
    SetResolution(resolution);

    if (!InitDeviceAndSwapChain(hWnd))
    {
        std::cout << "Failed to Initialize D3D device and context."
                  << std::endl;
        return false;
    }
    
    Graphics::InitCommonStates(device);

    CreateBuffers(); // backbuffer 포함

    SetMainViewport();

    // 공통으로 쓰이는 cbuffers
    D3D11Utils::CreateConstBuffer(device, globalConstsCPU,
                                  globalConstsGPU);
    D3D11Utils::CreateConstBuffer(device, reflectGlobalConstsCPU,
                                  reflectGlobalConstsGPU);

    // 그림자맵 렌더링할 때 사용할 GlobalConsts들 별도 생성
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        D3D11Utils::CreateConstBuffer(device, shadowGlobalConstsCPU[i],
                                      shadowGlobalConstsGPU[i]);
    }

    // 후처리 효과용 ConstBuffer
    D3D11Utils::CreateConstBuffer(device, postEffectsConstsCPU,
                                  postEffectsConstsGPU);

    // PostEffect에 사용
    screenSquare = make_shared<Actor>(
        device, context, vector{GeometryGenerator::MakeSquare()});


    // Prepare Compute Shader
    CreateUAVs();
    D3D11Utils::CreateComputeShader(device, L"BlurXGroupCacheCS.hlsl",
                                    blurXGroupCacheCS);
    D3D11Utils::CreateComputeShader(device, L"BlurYGroupCacheCS.hlsl",
                                    blurYGroupCacheCS);
    blurXGroupCacheComputePSO.computeShader = blurXGroupCacheCS;
    blurYGroupCacheComputePSO.computeShader = blurYGroupCacheCS;

    return true;
}

void D3D11Renderer::Update(Camera* camera, float dt)
{
    const Vector3 eyeWorld = camera->GetEyePos();
    const Matrix reflectRow = Matrix::CreateReflection(currentLevel->mirrorPlane);
    const Matrix viewRow = camera->GetViewRow();
    const Matrix projRow = camera->GetProjRow();

    UpdateLights(dt);

    // 공용 cbuffer 업데이트
    UpdateGlobalConstants(dt, eyeWorld, viewRow, projRow, reflectRow);

    // 거울
    if (currentLevel->mirror)
        currentLevel->mirror->UpdateConstantBuffers(device, context);

    // 조명의 위치 반영
    for (int i = 0; i < MAX_LIGHTS; i++)
        lightSphere[i]->UpdateWorldRow(
            Matrix::CreateScale(
                max(0.01f, globalConstsCPU.lights[i].radius)) *
            Matrix::CreateTranslation(globalConstsCPU.lights[i].position));

    // Gui cbuffer -> 사용자 입력이 인식되면 GPU 버퍼 업데이트
    if (postProcessFlag)
        postProcess.combineFilter.UpdateConstantBuffers(context);

    if (postEffectsFlag)
        D3D11Utils::UpdateBuffer(context, postEffectsConstsCPU,
                                 postEffectsConstsGPU);
}

void D3D11Renderer::UpdateLights(float dt)
{
    // 회전하는 lights[1] 업데이트
    static Vector3 lightDev = Vector3(1.0f, 0.0f, 0.0f);
    if (lightRotate)
    {
        lightDev = Vector3::Transform(
            lightDev, Matrix::CreateRotationY(dt * 3.141592f * 0.5f));
    }
    globalConstsCPU.lights[1].position = Vector3(0.0f, 2.8f, 2.0f) + lightDev;
    Vector3 focusPosition = Vector3(0.0f, -0.5f, 1.7f);
    globalConstsCPU.lights[1].direction =
        focusPosition - globalConstsCPU.lights[1].position;
    globalConstsCPU.lights[1].direction.Normalize();

    // 그림자맵을 만들기 위한 시점
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        const auto &light = globalConstsCPU.lights[i];
        if (light.type & LIGHT_SHADOW)
        {
            Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
            if (abs(up.Dot(light.direction) + 1.0f) < 1e-5)
                up = Vector3(1.0f, 0.0f, 0.0f);

            // 그림자맵을 만들 때 필요
            Matrix lightViewRow = XMMatrixLookAtLH(
                light.position, light.position + light.direction, up);

            Matrix lightProjRow = XMMatrixPerspectiveFovLH(
                XMConvertToRadians(120.0f), 1.0f, 0.1f, 10.0f);

            shadowGlobalConstsCPU[i].eyeWorld = light.position;
            shadowGlobalConstsCPU[i].view = lightViewRow.Transpose();
            shadowGlobalConstsCPU[i].proj = lightProjRow.Transpose();
            shadowGlobalConstsCPU[i].invProj =
                lightProjRow.Invert().Transpose();
            shadowGlobalConstsCPU[i].viewProj =
                (lightViewRow * lightProjRow).Transpose();

            // BasicPS <- LIGHT_FRUSTUM_WIDTH 값 확인 (invProj)
            // Vector4 eye(0.0f, 0.0f, 0.0f, 1.0f);
            // Vector4 xLeft(-1.0f, -1.0f, 0.0f, 1.0f);
            // Vector4 xRight(1.0f, 1.0f, 0.0f, 1.0f);
            // eye = Vector4::Transform(eye, lightProjRow);
            // xLeft = Vector4::Transform(xLeft, lightProjRow.Invert());
            // xRight = Vector4::Transform(xRight, lightProjRow.Invert());
            // xLeft /= xLeft.w;
            // xRight /= xRight.w;
            // cout << "LIGHT_FRUSTUM_WIDTH = " << xRight.x - xLeft.x <<
            // endl;

            D3D11Utils::UpdateBuffer(context, shadowGlobalConstsCPU[i],
                                     shadowGlobalConstsGPU[i]);

            // 그림자를 실제로 렌더링할 때 필요
            globalConstsCPU.lights[i].viewProj =
                shadowGlobalConstsCPU[i].viewProj;
            globalConstsCPU.lights[i].invProj =
                shadowGlobalConstsCPU[i].invProj;

            // 반사된 장면에서도 그림자를 그리고 싶다면 조명도 반사시켜서
            // 넣어주면 됩니다.
        }
    }
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
    sd.BufferDesc.RefreshRate.Numerator = 144;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT |
                     DXGI_USAGE_UNORDERED_ACCESS; // Compute Shader
    sd.OutputWindow = hWnd;
    sd.Windowed = TRUE;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 더 빠름 (Flip: MSAA 미지원) // TODO: ImGui 폰트가 두꺼워지는 문제 발견 
    //sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.SampleDesc.Count = 1; // _FLIP_은 MSAA 미지원
    sd.SampleDesc.Quality = 0; // MSAA 끔 (HDR Pipeline)

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
    // MSAA buffer(float), Resolve buffer, PostEffect buffer, Swapchain Backbuffer 초기화
    // HDR Rendering Pipeline: Scene Render -> MSAA(FP, HDR) -> Resolved(FP, HDR) -> PostProcess -> SwapChain BackBuffer(UNORM, SDR)

    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    ThrowIfFailed(device->CreateRenderTargetView(backBuffer.Get(), NULL,
                                                 backBufferRTV.GetAddressOf()));

    // MSAA RTV/SRV (Floating Point, HDR)
    ThrowIfFailed(device->CheckMultisampleQualityLevels(
        DXGI_FORMAT_R16G16B16A16_FLOAT, 4, &numQualityLevels)); // HDR + post process

    D3D11_TEXTURE2D_DESC desc;
    backBuffer->GetDesc(&desc);
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    
    // TODO: 간단한 모션 블러 구현
    // 이전 프레임 저장용
    ThrowIfFailed(
        device->CreateTexture2D(&desc, NULL, prevBuffer.GetAddressOf()));
    ThrowIfFailed(device->CreateRenderTargetView(prevBuffer.Get(), NULL,
                                                   prevRTV.GetAddressOf()));
    ThrowIfFailed(device->CreateShaderResourceView(prevBuffer.Get(), NULL,
                                                     prevSRV.GetAddressOf()));

    desc.MipLevels = desc.ArraySize = 1;

    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    desc.Usage = D3D11_USAGE_DEFAULT; // 스테이징 텍스춰로부터 복사 가능
    desc.MiscFlags = 0;
    desc.CPUAccessFlags = 0;
    if (useMSAA && numQualityLevels)
    {
        desc.SampleDesc.Count = 4;
        desc.SampleDesc.Quality = numQualityLevels - 1;
    }
    else
    {
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
    }

    ThrowIfFailed(
        device->CreateTexture2D(&desc, NULL, floatBuffer.GetAddressOf()));

    ThrowIfFailed(device->CreateRenderTargetView(floatBuffer.Get(), NULL,
                                                   floatRTV.GetAddressOf()));

    // Resolved Buffer RTV/SRV (MSAA -> Resolve) (FP, HDR)
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE |
                     D3D11_BIND_UNORDERED_ACCESS;
    ThrowIfFailed(device->CreateTexture2D(&desc, NULL,
                                            resolvedBuffer.GetAddressOf()));
    ThrowIfFailed(device->CreateTexture2D(
        &desc, NULL, postEffectsBuffer.GetAddressOf()));
    ThrowIfFailed(device->CreateShaderResourceView(
        resolvedBuffer.Get(), NULL, resolvedSRV.GetAddressOf()));

    ThrowIfFailed(device->CreateShaderResourceView(
        postEffectsBuffer.Get(), NULL, postEffectsSRV.GetAddressOf()));
    ThrowIfFailed(device->CreateRenderTargetView(
        resolvedBuffer.Get(), NULL, resolvedRTV.GetAddressOf()));
    ThrowIfFailed(device->CreateRenderTargetView(
        postEffectsBuffer.Get(), NULL, postEffectsRTV.GetAddressOf()));

    CreateDepthBuffers();

    // SRV = postEffects, prev
    // RTV = backBuffer
    postProcess.Initialize(device, context, {postEffectsSRV, prevSRV},
                            {backBufferRTV}, screenWidth, screenHeight,
                            4);
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

    // Depth 전용
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    ThrowIfFailed(device->CreateTexture2D(&desc, NULL,
                                            depthOnlyBuffer.GetAddressOf()));

    // 그림자 Buffers (Depth 전용)
    desc.Width = shadowWidth;
    desc.Height = shadowHeight;
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        ThrowIfFailed(device->CreateTexture2D(
            &desc, NULL, shadowBuffers[i].GetAddressOf()));
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    ZeroMemory(&dsvDesc, sizeof(dsvDesc));
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    ThrowIfFailed(device->CreateDepthStencilView(
        depthOnlyBuffer.Get(), &dsvDesc, depthOnlyDSV.GetAddressOf()));

    // 그림자 DSVs
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        ThrowIfFailed(
            device->CreateDepthStencilView(shadowBuffers[i].Get(), &dsvDesc,
                                             shadowDSVs[i].GetAddressOf()));
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    ThrowIfFailed(device->CreateShaderResourceView(
        depthOnlyBuffer.Get(), &srvDesc, depthOnlySRV.GetAddressOf()));

    // 그림자 SRVs
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        ThrowIfFailed(device->CreateShaderResourceView(
            shadowBuffers[i].Get(), &srvDesc,
            shadowSRVs[i].GetAddressOf()));
    }
}

void D3D11Renderer::CreateUAVs()
{
    D3D11Utils::CreateUATexture(device, screenWidth, screenHeight,
                                DXGI_FORMAT_R16G16B16A16_FLOAT, texA, rtvA,
                                srvA, uavA);

    D3D11Utils::CreateUATexture(device, screenWidth, screenHeight,
                                DXGI_FORMAT_R16G16B16A16_FLOAT, texB, rtvB,
                                srvB, uavB);
}

void D3D11Renderer::ComputeShaderBarrier()
{
    // 참고: BreadcrumbsDirectX-Graphics-Samples (DX12)
    // void CommandContext::InsertUAVBarrier(GpuResource & Resource, bool
    // FlushImmediate)

    // 예제들에서 최대 사용하는 SRV, UAV 갯수가 6개
    ID3D11ShaderResourceView *nullSRV[6] = {
        0,
    };
    context->CSSetShaderResources(0, 6, nullSRV);
    ID3D11UnorderedAccessView *nullUAV[6] = {
        0,
    };
    context->CSSetUnorderedAccessViews(0, 6, nullUAV, NULL);
}

void D3D11Renderer::SetResolution(const Resolution& resolution)
{
    //screenWidth = resolution.width - guiWidth;
    screenWidth = resolution.width;
    screenHeight = resolution.height;
    aspectRatio = resolution.AspectRatio();
}

void D3D11Renderer::SetMainViewport()
{
    // Set the viewport
    ZeroMemory(&screenViewport, sizeof(D3D11_VIEWPORT));
    screenViewport.TopLeftX = 0;
    screenViewport.TopLeftY = 0;
    //screenViewport.Width = float(screenWidth - guiWidth);
    screenViewport.Width = float(screenWidth);
    screenViewport.Height = float(screenHeight);
    screenViewport.MinDepth = 0.0f;
    screenViewport.MaxDepth = 1.0f;

    context->RSSetViewports(1, &screenViewport);
}

void D3D11Renderer::SetMainViewportNoGUIWidth()
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

void D3D11Renderer::SetShadowViewport()
{
    // Set the viewport
    D3D11_VIEWPORT shadowViewport;
    ZeroMemory(&shadowViewport, sizeof(D3D11_VIEWPORT));
    shadowViewport.TopLeftX = 0;
    shadowViewport.TopLeftY = 0;
    shadowViewport.Width = float(shadowWidth);
    shadowViewport.Height = float(shadowHeight);
    shadowViewport.MinDepth = 0.0f;
    shadowViewport.MaxDepth = 1.0f;

    context->RSSetViewports(1, &shadowViewport); // @refactor: PSO에 포함시킬 수도 있음
}

void D3D11Renderer::SetScreenSize(UINT width, UINT height)
{
    backBufferRTV.Reset();
    swapChain->ResizeBuffers(0,     // 현재 개수 유지
                             width, // 해상도 변경
                             height,
                             DXGI_FORMAT_UNKNOWN, // 현재 포맷 유지
                             0);
    CreateBuffers();
    SetMainViewport();
}

void D3D11Renderer::CaptureScreen()
{
    ComPtr<ID3D11Texture2D> backBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    D3D11Utils::WriteToPngFile(device, context, backBuffer, "captured.png");
}

void D3D11Renderer::ResetPostProcess()
{
    postProcess.Initialize(device, context, {postEffectsSRV, prevSRV},
                           {backBufferRTV}, screenWidth, screenHeight, 4);
}

void D3D11Renderer::RenderBegin()
{
    SetMainViewport();

    // 공통으로 사용하는 샘플러 설정
    context->VSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                           Graphics::sampleStates.data());
    context->PSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                           Graphics::sampleStates.data());
    context->HSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                           Graphics::sampleStates.data());
    context->DSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                           Graphics::sampleStates.data());

    // 공통으로 사용할 텍스춰들: "Common.hlsli"에서 register(t10)부터 시작
    vector<ID3D11ShaderResourceView *> commonSRVs = {
        envSRV.Get(), specularSRV.Get(), irradianceSRV.Get(), brdfSRV.Get()};
    context->PSSetShaderResources(10, UINT(commonSRVs.size()),
                                  commonSRVs.data());
}

void D3D11Renderer::RenderDepthOnly()
{
    context->OMSetRenderTargets(0, NULL, // DepthOnly라서 RTV 불필요
                                depthOnlyDSV.Get());
    context->ClearDepthStencilView(depthOnlyDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f,
                                   0);
    SetGlobalConsts(globalConstsGPU);
    currentLevel->RenderDepthOnly(context);
}

void D3D11Renderer::RenderShadowMaps()
{
    // 쉐도우 맵을 다른 쉐이더에서 SRV 해제
    ID3D11ShaderResourceView *nulls[2] = {0, 0};
    context->PSSetShaderResources(15, 2, nulls);

    // 그림자맵 해상도
    SetShadowViewport(); 
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (globalConstsCPU.lights[i].type & LIGHT_SHADOW)
        {
            context->OMSetRenderTargets(0, NULL, // DepthOnly라서 RTV 불필요
                                          shadowDSVs[i].Get());
            context->ClearDepthStencilView(shadowDSVs[i].Get(),
                                             D3D11_CLEAR_DEPTH, 1.0f, 0);
            SetGlobalConsts(shadowGlobalConstsGPU[i]);

            for (const auto &actor : currentLevel->actors)
            {
                if (actor->castShadow && actor->isVisible)
                {
                    SetPipelineState(actor->GetDepthOnlyPSO());
                    actor->Render(context);
                }
            }

            if (currentLevel->mirror && currentLevel->mirror->castShadow)
                currentLevel->mirror->Render(context);
        }
    }
}

void D3D11Renderer::RenderOpaqueObjects()
{
    // 다시 렌더링 해상도로 되돌리기
    SetMainViewport();

    // floatRTV
    //context->ClearRenderTargetView(backBufferRTV.Get(), clearColor);
    context->ClearRenderTargetView(floatRTV.Get(), clearColor);
    context->OMSetRenderTargets(1, floatRTV.GetAddressOf(), defaultDSV.Get());

    // 그림자맵들도 공용 텍스춰들 뒤에 추가
    // 주의: 마지막 shadowDSV를 RenderTarget에서 해제한 후 설정
    vector<ID3D11ShaderResourceView *> newShadowSRVs;
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        newShadowSRVs.push_back(shadowSRVs[i].Get());
    }
    context->PSSetShaderResources(15, UINT(newShadowSRVs.size()),
                                    newShadowSRVs.data());

    context->ClearDepthStencilView(
        defaultDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    SetGlobalConsts(globalConstsGPU);

    // Draw (skybox, actors, mirror, normal)
    currentLevel->Render(context, drawAsWire);
}

void D3D11Renderer::RenderMirror()
{
    if (currentLevel->mirrorAlpha < 1.0f && currentLevel->mirror) // 거울 반사 그려야하는 상황
    {
        // 거울 위치만 StencilBuffer에 1로 표시
        SetPipelineState(Graphics::stencilMaskPSO);
        currentLevel->mirror->Render(context);

        // 거울 위치에 반사된 물체, skybox 렌더링
        SetGlobalConsts(reflectGlobalConstsGPU);
        context->ClearDepthStencilView(defaultDSV.Get(), D3D11_CLEAR_DEPTH,
                                         1.0f, 0);

        for (auto& actor : currentLevel->actors)
        {
            SetPipelineState(actor->GetReflectPSO(drawAsWire));
            actor->Render(context);
        }

        SetPipelineState(drawAsWire ? Graphics::reflectSkyboxWirePSO
                                    : Graphics::reflectSkyboxSolidPSO);
        currentLevel->skybox->Render(context);

        // 거울 자체의 재질을 "Blend"로 그림
        SetPipelineState(drawAsWire ? Graphics::mirrorBlendWirePSO
                                               : Graphics::mirrorBlendSolidPSO);
        SetGlobalConsts(globalConstsGPU);
        currentLevel->mirror->Render(context);

    } // end of if (mirrorAlpha < 1.0f)
}

void D3D11Renderer::Render()
{
    RenderBegin();

    RenderDepthOnly();
    RenderShadowMaps();
    RenderOpaqueObjects();
    RenderMirror();

    RenderEnd();
}

void D3D11Renderer::RenderEnd()
{
    // Example의 Render()에서 RT 설정을 해주지 않았을 경우에도
    // 백 버퍼에 GUI를 그리기위해 RT 설정
    // 예) Render()에서 ComputeShader만 사용
    SetMainViewport();
    context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), NULL);
}

void D3D11Renderer::PostRender()
{
    // Resolve MSAA texture
    context->ResolveSubresource(resolvedBuffer.Get(), 0,
                                  floatBuffer.Get(), 0,
                                  DXGI_FORMAT_R16G16B16A16_FLOAT);

    // 1. PostEffects (globalConstsGPU 사용)
    D3D11Renderer::SetMainViewportNoGUIWidth();
    D3D11Renderer::SetPipelineState(Graphics::postEffectsPSO);
    D3D11Renderer::SetGlobalConsts(globalConstsGPU);
    vector<ID3D11ShaderResourceView *> postEffectsSRVs = {
        resolvedSRV.Get(), depthOnlySRV.Get()};

    // Shadow Map 확인용 (임시)
    //D3D11Renderer::SetGlobalConsts(shadowGlobalConstsGPU[1]);
    //vector<ID3D11ShaderResourceView *> postEffectsSRVs = {resolvedSRV.Get(),
    //                                                      shadowSRVs[1].Get()};

    context->PSSetShaderResources(20, // 주의: Startslot 20
                                    UINT(postEffectsSRVs.size()),
                                    postEffectsSRVs.data());
    context->OMSetRenderTargets(1, postEffectsRTV.GetAddressOf(), NULL);
    context->PSSetConstantBuffers(5, // register(b5)
                                    1, postEffectsConstsGPU.GetAddressOf());
    screenSquare->Render(context);

    ID3D11ShaderResourceView *nulls[2] = {0, 0};
    context->PSSetShaderResources(20, 2, nulls);

    // 2. Compute Shader 후처리 (예 - 가우시안 블러)
    if (blurStrength > 0.0f)
    {
        D3D11Renderer::RenderCS();
    }

    // 3. PostProcess 후처리 (블룸 같은 순수 이미지 처리)
    D3D11Renderer::SetPipelineState(Graphics::postProcessingPSO);
    postProcess.Render(context);

    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    context->CopyResource(
        prevBuffer.Get(),
        backBuffer.Get()); // 모션 블러 효과를 위해 렌더링 결과 보관
}

void D3D11Renderer::RenderCS()
{
    // 복사
    context->CopyResource(texA.Get(), postEffectsBuffer.Get());

    int numBlurs = int(blurStrength * 1000.0f);
    for (int i = 0; i < numBlurs; i++)
    {
        ComputeShaderBlur();
    }

    // texA -> 다시 postEffectsBuffer로 복사
    context->CopyResource(postEffectsBuffer.Get(), texA.Get());
}

void D3D11Renderer::ComputeShaderBlur()
{
    context->CSSetSamplers(0, 1, Graphics::pointClampSS.GetAddressOf());

    const UINT tgx = UINT(ceil(screenWidth / 256.0f));
    const UINT tgy = UINT(ceil(screenHeight / 256.0f));

    // Horizontal X-Blur, A to B
    SetPipelineState(blurXGroupCacheComputePSO);
    context->CSSetShaderResources(0, 1, srvA.GetAddressOf());
    context->CSSetUnorderedAccessViews(0, 1, uavB.GetAddressOf(), NULL);
    context->Dispatch(tgx, screenHeight, 1);
    ComputeShaderBarrier();

    // Vertical Y-Blur, B to A
    SetPipelineState(blurYGroupCacheComputePSO);
    context->CSSetShaderResources(0, 1, srvB.GetAddressOf());
    context->CSSetUnorderedAccessViews(0, 1, uavA.GetAddressOf(), NULL);
    context->Dispatch(screenWidth, tgy, 1);
    ComputeShaderBarrier();
}

void D3D11Renderer::SwapBuffer()
{
    swapChain->Present(1, 0); // 1: VSync
}

void D3D11Renderer::SetCurrentLevel(const shared_ptr<Level> &newLevel)
{
    currentLevel = newLevel;
}

void D3D11Renderer::InitCubemaps(wstring basePath, wstring envFilename,
                                 wstring specularFilename,
                                 wstring irradianceFilename,
                                 wstring brdfFilename)
{
    // BRDF LookUp Table은 CubeMap이 아니라 2D 텍스춰 입니다.
    D3D11Utils::CreateDDSTexture(device, (basePath + envFilename).c_str(),
                                 true, envSRV);
    D3D11Utils::CreateDDSTexture(
        device, (basePath + specularFilename).c_str(), true, specularSRV);
    D3D11Utils::CreateDDSTexture(device,
                                 (basePath + irradianceFilename).c_str(), true,
                                 irradianceSRV);
    D3D11Utils::CreateDDSTexture(device, (basePath + brdfFilename).c_str(),
                                 false, brdfSRV);
}

void D3D11Renderer::UpdateGlobalConstants(const float &dt,
                                          const Vector3 &eyeWorld,
                                          const Matrix &viewRow,
                                          const Matrix &projRow,
                                          const Matrix &refl)
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

    // 거울 반사 렌더링
    reflectGlobalConstsCPU = globalConstsCPU;
    memcpy(&reflectGlobalConstsCPU, &globalConstsCPU,
           sizeof(globalConstsCPU));
    reflectGlobalConstsCPU.view = (refl * viewRow).Transpose();
    reflectGlobalConstsCPU.viewProj = (refl * viewRow * projRow).Transpose();
    // 그림자 렌더링에 사용 (TODO: 광원의 위치도 반사시킨 후에 계산해야 함)
    reflectGlobalConstsCPU.invViewProj =
        reflectGlobalConstsCPU.viewProj.Invert();


    D3D11Utils::UpdateBuffer(context, globalConstsCPU, globalConstsGPU);
    D3D11Utils::UpdateBuffer(context, reflectGlobalConstsCPU, reflectGlobalConstsGPU);
}

void D3D11Renderer::UpdatePostEffectsConstants()
{
    D3D11Utils::UpdateBuffer(context, postEffectsConstsCPU,
                             postEffectsConstsGPU);
}

void D3D11Renderer::SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU)
{
    // 쉐이더와 일관성 유지 cbuffer GlobalConstants : register(b0)
    context->VSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
    context->HSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
    context->DSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
    context->GSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
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

void D3D11Renderer::SetPipelineState(const ComputePSO &pso)
{
    context->VSSetShader(NULL, 0, 0);
    context->PSSetShader(NULL, 0, 0);
    context->HSSetShader(NULL, 0, 0);
    context->DSSetShader(NULL, 0, 0);
    context->GSSetShader(NULL, 0, 0);
    context->CSSetShader(pso.computeShader.Get(), 0, 0);
}

float D3D11Renderer::GetAspectRatio() const
{
    //return float(screenWidth - guiWidth) / screenHeight;
    return float(screenWidth) / screenHeight;
}
