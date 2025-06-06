#pragma once
#include "Engine.h"

class Sample_PBR : public Engine
{
public:
    Sample_PBR();

    virtual bool InitLevel() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

private:
    shared_ptr<Actor> sphere;
};
