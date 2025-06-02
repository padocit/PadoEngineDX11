#pragma once

#include "D3D11Common.h"
#include "IRenderer.h"
#include "Camera.h"
#include "Common.h"
#include "ConstantBuffer.h"
#include "MeshData.h"
#include "Mesh.h"
#include "Level.h"

using Microsoft::WRL::ComPtr;

class D3D11Renderer : public IRenderer
{
public:
	// Init
    bool Initialize(const Resolution &resolution, HWND hWnd) override;

	// Shutdown (임시: 필요 시 작성)
    void Shutdown() override {}

	// Update & Render
    void Update(Level* level, Camera* camera, float dt) override;
    //virtual void UpdateLights(float dt);
    //virtual void RenderDepthOnly();
    //virtual void RenderShadowMaps();
    //virtual void RenderOpaqueObjects();
    //virtual void RenderMirror();
    void Render(Level* level) override;
    void SwapBuffer() override;


	// Screen
    void SetScreenSize(const Resolution &resolution) override;
    void SetMainViewPort() override;
    float GetAspectRatio() const override;

	// Resources
    ComPtr<ID3D11Device>& GetDevice()
    {
        return device;
    }
	ComPtr<ID3D11DeviceContext>& GetContext()
	{
        return context;
	}

public:
    bool InitDeviceAndSwapChain(HWND hWnd);
    
    void UpdateGlobalConstants(const float &dt, const Vector3 &eyeWorld,
                               const Matrix &viewRow, const Matrix &projRow);
                               //const Matrix &refl = Matrix());
    void SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU);

    void SetPipelineState(const D3D11PSO &pso);
    void CreateBuffers();
    void CreateDepthBuffers();

    void Prepare();

private:
	// Properties
	int screenWidth;
	int screenHeight;
	float aspectRatio;
	bool useMSAA = true;
	UINT numQualityLevels = 0;
	bool drawAsWire = false;
    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

	// Backbuffer
	DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Depth buffer
    ComPtr<ID3D11DepthStencilView> defaultDSV;

	// Viewport
    D3D11_VIEWPORT screenViewport;

	// Device
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain> swapChain;
	ComPtr<ID3D11RenderTargetView> backBufferRTV;

	// cbuffer
    GlobalConstants globalConstsCPU;
    ComPtr<ID3D11Buffer> globalConstsGPU;
};

