#pragma once

#include "D3D11Utils.h"
#include <directxtk/SimpleMath.h>

// "Common.hlsli"와 동일해야 함
#define MAX_LIGHTS 3
#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

// DirectX-Graphics-Samples/MiniEngine을 따라서 파일이름 변경
// __declspec(align(256)) : DX12에서는 256 align (예습)

__declspec(align(256)) struct PhongConstants
{
    int useBlinnPhong = 0;
    bool useTexture = false;
    DirectX::SimpleMath::Vector3 dummy;
};

// 주로 Vertex/Geometry 쉐이더에서 사용
__declspec(align(256)) struct MeshConstants
{
    Matrix world;
    Matrix worldIT;
    Matrix worldInv;
    int useHeightMap = 0;
    float heightScale = 0.0f;
    float windTrunk = 0.0f;
    float windLeaves = 0.0f;
};

// 주로 Pixel 쉐이더에서 사용
__declspec(align(256)) struct MaterialConstants
{

    Vector3 albedoFactor = Vector3(1.0f, 0.0f, 1.0f);
    float roughnessFactor = 1.0f;
    float metallicFactor = 1.0f;
    Vector3 emissionFactor = Vector3(0.0f);

    // 여러 옵션들에 uint 플래그 하나만 사용할 수도 있습니다.
    int useAlbedoMap = 0;
    int useNormalMap = 0;
    int useAOMap = 0;
    int invertNormalMapY = 0;
    int useMetallicMap = 0;
    int useRoughnessMap = 0;
    int useEmissiveMap = 0;
    float shininess = 1.0f; // phong or dummy

    // 참고 flags 구현
    /* union {
        uint32_t flags;
        struct {
            // UV0 or UV1 for each texture
            uint32_t baseColorUV : 1;
            uint32_t metallicRoughnessUV : 1;
            uint32_t occlusionUV : 1;
            uint32_t emissiveUV : 1;
            uint32_t normalUV : 1;

            // Three special modes
            uint32_t twoSided : 1;
            uint32_t alphaTest : 1;
            uint32_t alphaBlend : 1;

            uint32_t _pad : 8;

            uint32_t alphaRef : 16; // half float
        };
    };*/
};

struct Light
{
    Vector3 radiance = Vector3(5.0f); // strength
    float fallOffStart = 0.0f;
    Vector3 direction = Vector3(0.0f, 0.0f, 1.0f);
    float fallOffEnd = 20.0f;
    Vector3 position = Vector3(0.0f, 0.0f, -2.0f);
    float spotPower = 6.0f;

    // Light type bitmasking
    // ex) LIGHT_SPOT | LIGHT_SHADOW
    uint32_t type = LIGHT_OFF;
    float radius = 0.035f; // 반지름

    float haloRadius = 0.0f;
    float haloStrength = 0.0f;

    Matrix viewProj; // 그림자 렌더링에 필요
    Matrix invProj;  // 그림자 렌더링 디버깅용
};

// register(b1) 사용
__declspec(align(256)) struct GlobalConstants
{
    Matrix view;
    Matrix proj;
    Matrix invProj; // 역프로젝션행렬
    Matrix viewProj;
    Matrix invViewProj; // Proj -> World
    Matrix invView;

    Vector3 eyeWorld;
    float strengthIBL = 1.0f;

    int textureToDraw = 0;   // 0: Env, 1: Specular, 2: Irradiance, 그외: 검은색
    float envLodBias = 0.0f; // 환경맵 LodBias
    float lodBias = 1.0f;    // 다른 물체들 LodBias
    float globalTime = 0.0f;

    Light lights[MAX_LIGHTS];
};

// register(b5) 사용, PostEffectsPS.hlsl
__declspec(align(256)) struct PostEffectsConstants
{
    int mode = 1; // 1: Rendered image, 2: DepthOnly
    float depthScale = 1.0f;
    float fogStrength = 0.0f;
};

__declspec(align(256)) struct VolumeConsts
{
    Vector3 uvwOffset = Vector3(0.0f);
    float lightAbsorption = 5.0f;
    Vector3 lightDir = Vector3(0.0f, 1.0f, 0.0f);
    float densityAbsorption = 10.0f;
    Vector3 lightColor = Vector3(1, 1, 1) * 40.0f;
    float aniso = 0.3f;
};

// bone 개수 제약을 없애기 위해 StructuredBuffer로 교체
// __declspec(align(256)) struct SkinnedConsts {
//    Matrix boneTransforms[52]; // bone 개수
//};

template <typename T_CONSTS> 
class ConstantBuffer
{
public:
    void Initialize(ComPtr<ID3D11Device> &device)
    {
        D3D11Utils::CreateConstBuffer(device, cpu, gpu);
    }

    void Upload(ComPtr<ID3D11DeviceContext> &context)
    {
        D3D11Utils::UpdateBuffer(context, cpu, gpu);
    }

public:
    T_CONSTS &GetCpu()
    {
        return cpu;
    }
    const auto Get()
    {
        return gpu.Get();
    }
    const auto GetAddressOf()
    {
        return gpu.GetAddressOf();
    }

    T_CONSTS cpu;
    ComPtr<ID3D11Buffer> gpu;
};
