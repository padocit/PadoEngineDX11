// Scene
#pragma once

#include "D3D11Utils.h"

class Actor;
class Level
{
public:
    Level();
    virtual ~Level();

    void AddActor(std::shared_ptr<Actor> newActor);
    void SetSkybox(std::shared_ptr<Actor> newSkybox);

    virtual void Update(ComPtr<ID3D11Device> &device,
                        ComPtr<ID3D11DeviceContext> &context);
    virtual void Render(ComPtr<ID3D11DeviceContext> &context, const bool wired);
    virtual void RenderDepthOnly(ComPtr<ID3D11DeviceContext> &context);

public:
    shared_ptr<Actor> skybox;

public: // юс╫ц
    std::vector<std::shared_ptr<Actor>> actors;
};
