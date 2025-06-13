#pragma once

#include "D3D11Common.h"
#include "Camera.h"
#include "Common.h"
#include "ConstantBuffer.h"
#include "MeshData.h"
#include "Mesh.h"
#include "Level.h"
#include "PostProcess.h"

using Microsoft::WRL::ComPtr;

class Actor;
class D3D11Renderer /* : public IRenderer*/
{
public:
	// Init
    bool Initialize(const Resolution &resolution, HWND hWnd);

	// Shutdown (임시: 필요 시 작성)
    void Shutdown() {}

	// Update & Render
    void Update(Level* level, Camera* camera, float dt);
    //virtual void UpdateLights(float dt);
    virtual void RenderDepthOnly(Level *level);
    //virtual void RenderShadowMaps();
    //virtual void RenderOpaqueObjects();
    //virtual void RenderMirror();
    void Render(Level* level);
    void PostRender();
    void SwapBuffer();

	// Screen
    void SetResolution(const Resolution &resolution);
    void SetMainViewPort();
    void SetMainViewPortNoGUIWidth();
    float GetAspectRatio() const;
    void SetScreenSize(UINT width, UINT height);

    void CaptureScreen();

    void ResetPostProcess();

	// Resources
    ComPtr<ID3D11Device>& GetDevice()
    {
        return device;
    }
	ComPtr<ID3D11DeviceContext>& GetContext()
	{
        return context;
	}
    ComPtr<IDXGISwapChain>& GetSwapChain()
    {
        return swapChain;
    }

public:
    bool InitDeviceAndSwapChain(HWND hWnd);
    

    void InitCubemaps(wstring basePath, wstring envFilename,
                      wstring specularFilename, wstring irradianceFilename,
                      wstring brdfFilename);

    void UpdateGlobalConstants(const float &dt, const Vector3 &eyeWorld,
                               const Matrix &viewRow, const Matrix &projRow);
                               //const Matrix &refl = Matrix());
    void SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU);

    void SetPipelineState(const D3D11PSO &pso);
    void CreateBuffers();
    void CreateDepthBuffers();

    void Prepare();

public:
    // GUI 연동
    GlobalConstants globalConstsCPU;
    int guiWidth = 0;
    bool drawAsWire = false;
    int postProcessFlag = 0;
    int postEffectsFlag = 0;
    PostEffectsConstants postEffectsConstsCPU;
    ComPtr<ID3D11Buffer> postEffectsConstsGPU;
    bool useMSAA = true;

    // Render(FP, HDR) -> PostEffects -> PostProcess
    PostProcess postProcess;


private:
	// Properties
	int screenWidth;
	int screenHeight;
	float aspectRatio;
	UINT numQualityLevels = 0;
    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    
    // 공용 텍스처(IBL)
    ComPtr<ID3D11ShaderResourceView> envSRV;
    ComPtr<ID3D11ShaderResourceView> irradianceSRV;
    ComPtr<ID3D11ShaderResourceView> specularSRV;
    ComPtr<ID3D11ShaderResourceView> brdfSRV;


	// Backbuffer
    // HDR Rendering Pipeline => SwapChain = SDR(UNORM) 사용
	DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    // HDR Rendering Pipeline: Scene Render -> MSAA(FP, HDR) -> Resolved(FP, HDR) -> PostProcess -> SwapChain BackBuffer(UNORM, SDR)
    ComPtr<ID3D11Texture2D> floatBuffer;
    ComPtr<ID3D11Texture2D> resolvedBuffer;
    ComPtr<ID3D11Texture2D> postEffectsBuffer;
    ComPtr<ID3D11Texture2D> prevBuffer; // TODO: 모션 블러 효과
    ComPtr<ID3D11RenderTargetView> floatRTV;
    ComPtr<ID3D11RenderTargetView> resolvedRTV;
    ComPtr<ID3D11RenderTargetView> postEffectsRTV;
    ComPtr<ID3D11RenderTargetView> prevRTV;
    ComPtr<ID3D11ShaderResourceView> resolvedSRV;
    ComPtr<ID3D11ShaderResourceView> postEffectsSRV;
    ComPtr<ID3D11ShaderResourceView> prevSRV;

    // Depth buffer
    ComPtr<ID3D11DepthStencilView> defaultDSV; 
    ComPtr<ID3D11Texture2D> depthOnlyBuffer; // No MSAA
    ComPtr<ID3D11DepthStencilView> depthOnlyDSV;
    ComPtr<ID3D11ShaderResourceView> depthOnlySRV;

    // Shadow maps
    int shadowWidth = 1280;
    int shadowHeight = 1280;
    ComPtr<ID3D11Texture2D> shadowBuffers[MAX_LIGHTS]; // No MSAA
    ComPtr<ID3D11DepthStencilView> shadowDSVs[MAX_LIGHTS];
    ComPtr<ID3D11ShaderResourceView> shadowSRVs[MAX_LIGHTS];


	// Viewport
    D3D11_VIEWPORT screenViewport;

	// Device
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain> swapChain;
	ComPtr<ID3D11RenderTargetView> backBufferRTV;

	// cbuffer
    ComPtr<ID3D11Buffer> globalConstsGPU;

    // 여러 예제들 공용
    std::shared_ptr<Actor> screenSquare; // PostEffect에 사용
};

