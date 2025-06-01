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

    virtual void Render();

protected:
    std::vector<std::shared_ptr<Actor>> actors;
};
