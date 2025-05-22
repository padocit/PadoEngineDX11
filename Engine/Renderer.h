#pragma once

#include "D3D11Utils.h"
#include "Common.h"
#include "ConstantBuffers.h"
#include "MeshData.h"
#include "Mesh.h"

using Microsoft::WRL::ComPtr;

class Renderer
{
public:
	// Init
	bool Initialize(const Resolution& res, HWND hWindow);

	// Render
    void Render(const Mesh &mesh);
    void SwapBuffer();

	// Create
    void CreateMesh(const MeshData &meshData, Mesh &mesh);

	void SetMainViewPort();

	float GetAspectRatio() const;


private:
    bool InitDeviceAndSwapChain(HWND hWindow);
    void InitBackBuffer();
    void InitRasterizerStates();
    void InitShaders();
    void InitConstantBuffer();

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
    Constants constantData;
    ComPtr<ID3D11Buffer> constantBuffer;

	// Depth buffer

};

