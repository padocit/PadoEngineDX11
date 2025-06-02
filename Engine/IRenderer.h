// Interface of D3D Renderer
#pragma once

#include "D3D11Utils.h"

class Level;
class Camera;
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
    virtual void Update(Level* level, Camera* camera, float dt) = 0;
    virtual void Render(Level* level) = 0;
    virtual void SwapBuffer() = 0;

    // 화면 설정
    virtual void SetResolution(const Resolution &resolution) = 0;
    virtual void SetMainViewPort() = 0;
    virtual float GetAspectRatio() const = 0;
};
