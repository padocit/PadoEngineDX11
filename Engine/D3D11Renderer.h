#pragma once

#include "IRenderer.h"
#include "Common.h"
#include "ConstantBuffers.h"
#include "MeshData.h"
#include "Mesh.h"

using Microsoft::WRL::ComPtr;

class D3D11Renderer : public IRenderer
{
public:
	// Init
    bool Initialize(const Resolution &resolution, HWND hWnd) override;

	// Shutdown (임시: 필요 시 작성)
    void Shutdown() override {}

	// Update & Render
    void Update() override;
    void Render(const Mesh &mesh) override;
    void SwapBuffer() override;

	// Create Resource
    void CreateMesh(const MeshData &meshData, Mesh &mesh) override;

	// Screen
    void SetScreenSize(const Resolution &resolution) override;
    void SetMainViewPort() override;
    float GetAspectRatio() const override;

	// Resources
    ID3D11Device* const GetDevice() const
    {
        return device.Get();
    }
	ID3D11DeviceContext* const GetContext() const
	{
        return context.Get();
	}

	// 임시 public
    PixelConstants pixelShaderConstData;


private:
    bool InitDeviceAndSwapChain(HWND hWnd);
    void InitBackBuffer();
    void InitRasterizerStates();
    void InitShaders();
    void InitConstantBuffer();

	void UpdateConstantBuffer();

    void Prepare();
    void PrepareShader();
    void RenderPrimitive(const Mesh &mesh);


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

	// Viewport
    D3D11_VIEWPORT screenViewport;

	// Device
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain> swapChain;
	ComPtr<ID3D11RenderTargetView> backBufferRTV;

	// Rasterizer State
    ComPtr<ID3D11RasterizerState> rasterizerState;

	// Shaders
    ComPtr<ID3D11VertexShader> basicVS;
    ComPtr<ID3D11PixelShader> basicPS;

	// Input Layouts
    ComPtr<ID3D11InputLayout> basicIL;

	// Constant buffers
    ComPtr<ID3D11Buffer> pixelShaderConstBuffer;

	// Depth buffer

};

