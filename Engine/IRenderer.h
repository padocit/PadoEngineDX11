// Interface of D3D Renderer
#pragma once

#include "D3D11Utils.h"

struct Mesh;
struct MeshData;
struct Resolution;

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    // 초기화 및 종료
    virtual bool Initialize(const Resolution &resolution, HWND hWnd) = 0;
    virtual void Shutdown() = 0;

    // 업데이트 및 렌더링
    virtual void Update() = 0;
    virtual void Render(const Mesh &mesh) = 0;
    virtual void SwapBuffer() = 0;

    // 리소스 생성
    virtual void CreateMesh(const MeshData &meshData, Mesh &mesh) = 0;

    // 화면 설정
    virtual void SetScreenSize(const Resolution &resolution) = 0;
    virtual void SetMainViewPort() = 0;
    virtual float GetAspectRatio() const = 0;
};
