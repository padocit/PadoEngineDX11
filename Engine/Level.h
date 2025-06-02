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

    virtual void Update(ComPtr<ID3D11Device> &device,
                        ComPtr<ID3D11DeviceContext> &context);
    virtual void Render(ComPtr<ID3D11DeviceContext> &context, const bool wired);

protected:
    std::vector<std::shared_ptr<Actor>> actors;

};
