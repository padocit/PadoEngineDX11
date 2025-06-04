#pragma once

#include "D3D11Utils.h"
#include "D3D11PSO.h"

// 참고: DirectX_Graphic-Samples 미니엔진
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/GraphicsCommon.h

namespace Graphics
{
// Samplers
extern ComPtr<ID3D11SamplerState> linearWrapSS;
extern ComPtr<ID3D11SamplerState> linearClampSS;
extern ComPtr<ID3D11SamplerState> pointClampSS;
extern ComPtr<ID3D11SamplerState> shadowPointSS;
extern ComPtr<ID3D11SamplerState> shadowCompareSS;
extern ComPtr<ID3D11SamplerState> pointWrapSS;
extern ComPtr<ID3D11SamplerState> linearMirrorSS;
extern std::vector<ID3D11SamplerState *> sampleStates;

// Rasterizer States
// CCW: Counter-Clockwise (반시계 방향을 의미합니다.)
extern ComPtr<ID3D11RasterizerState> solidRS; // front only
extern ComPtr<ID3D11RasterizerState> solidCcwRS;
extern ComPtr<ID3D11RasterizerState> wireRS;
extern ComPtr<ID3D11RasterizerState> wireCcwRS;
extern ComPtr<ID3D11RasterizerState> postProcessingRS;
extern ComPtr<ID3D11RasterizerState> solidBothRS; // front and back
extern ComPtr<ID3D11RasterizerState> wireBothRS;
extern ComPtr<ID3D11RasterizerState> solidBothCcwRS;
extern ComPtr<ID3D11RasterizerState> wireBothCcwRS;

// Depth Stencil States
extern ComPtr<ID3D11DepthStencilState> drawDSS;       // 일반적으로 그리기
extern ComPtr<ID3D11DepthStencilState> maskDSS;       // 스텐실버퍼에 표시
extern ComPtr<ID3D11DepthStencilState> drawMaskedDSS; // 스텐실 표시된 곳만

// Shaders
extern ComPtr<ID3D11VertexShader> basicVS;
extern ComPtr<ID3D11VertexShader> skinnedVS; // basicVS.hlsl에 SKINNED 매크로
extern ComPtr<ID3D11VertexShader> skyboxVS;
extern ComPtr<ID3D11VertexShader> samplingVS;
extern ComPtr<ID3D11VertexShader> normalVS;
extern ComPtr<ID3D11VertexShader> depthOnlyVS;
extern ComPtr<ID3D11VertexShader> depthOnlySkinnedVS;
extern ComPtr<ID3D11VertexShader> grassVS;
extern ComPtr<ID3D11VertexShader> billboardVS;
extern ComPtr<ID3D11PixelShader> basicPS;
extern ComPtr<ID3D11PixelShader> phongPS;
extern ComPtr<ID3D11PixelShader> iblPS;
extern ComPtr<ID3D11PixelShader> skyboxPS;
extern ComPtr<ID3D11PixelShader> combinePS;
extern ComPtr<ID3D11PixelShader> bloomDownPS;
extern ComPtr<ID3D11PixelShader> bloomUpPS;
extern ComPtr<ID3D11PixelShader> normalPS;
extern ComPtr<ID3D11PixelShader> depthOnlyPS;
extern ComPtr<ID3D11PixelShader> postEffectsPS;
extern ComPtr<ID3D11PixelShader> volumeSmokePS;
extern ComPtr<ID3D11PixelShader> grassPS;
extern ComPtr<ID3D11PixelShader> oceanPS;
extern ComPtr<ID3D11PixelShader> volumetricFirePS;
extern ComPtr<ID3D11PixelShader> gameExplosionPS;
extern ComPtr<ID3D11GeometryShader> normalGS;
extern ComPtr<ID3D11GeometryShader> billboardGS;

// Input Layouts
extern ComPtr<ID3D11InputLayout> basicIL;
extern ComPtr<ID3D11InputLayout> skinnedIL;
extern ComPtr<ID3D11InputLayout> samplingIL;
extern ComPtr<ID3D11InputLayout> skyboxIL;
extern ComPtr<ID3D11InputLayout> postProcessingIL;
extern ComPtr<ID3D11InputLayout> grassIL;
extern ComPtr<ID3D11InputLayout> billboardIL;

// Blend States
extern ComPtr<ID3D11BlendState> mirrorBS;
extern ComPtr<ID3D11BlendState> accumulateBS;
extern ComPtr<ID3D11BlendState> alphaBS;

// Graphics Pipeline States
extern D3D11PSO defaultSolidPSO;
extern D3D11PSO skinnedSolidPSO;
extern D3D11PSO defaultWirePSO;
extern D3D11PSO skinnedWirePSO;
extern D3D11PSO phongSolidPSO;
extern D3D11PSO phongWirePSO;
extern D3D11PSO iblSolidPSO;
extern D3D11PSO stencilMaskPSO;
extern D3D11PSO reflectSolidPSO;
extern D3D11PSO reflectSkinnedSolidPSO;
extern D3D11PSO reflectWirePSO;
extern D3D11PSO reflectSkinnedWirePSO;
extern D3D11PSO mirrorBlendSolidPSO;
extern D3D11PSO mirrorBlendWirePSO;
extern D3D11PSO skyboxSolidPSO;
extern D3D11PSO skyboxWirePSO;
extern D3D11PSO reflectSkyboxSolidPSO;
extern D3D11PSO reflectSkyboxWirePSO;
extern D3D11PSO normalsPSO;
extern D3D11PSO depthOnlyPSO;
extern D3D11PSO depthOnlySkinnedPSO;
extern D3D11PSO postEffectsPSO;
extern D3D11PSO postProcessingPSO;
extern D3D11PSO boundingBoxPSO;
extern D3D11PSO grassSolidPSO;
extern D3D11PSO grassWirePSO;
extern D3D11PSO oceanPSO;

// 주의: 초기화 느려서 필요한 경우에만 초기화
extern D3D11PSO volumeSmokePSO;

void InitCommonStates(ComPtr<ID3D11Device> &device);

// 내부적으로 InitCommonStates()에서 사용
void InitSamplers(ComPtr<ID3D11Device> &device);
void InitRasterizerStates(ComPtr<ID3D11Device> &device);
void InitBlendStates(ComPtr<ID3D11Device> &device);
void InitDepthStencilStates(ComPtr<ID3D11Device> &device);
void InitPipelineStates(ComPtr<ID3D11Device> &device);
void InitShaders(ComPtr<ID3D11Device> &device);

// 주의: 초기화가 느려서 필요한 경우에만 초기화
void InitVolumeShaders(ComPtr<ID3D11Device> &device);

} // namespace Graphics
