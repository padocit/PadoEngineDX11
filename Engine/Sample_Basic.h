#pragma once
#include "Engine.h"
class Sample_Basic : public Engine
{
public:
    Sample_Basic();

    virtual bool InitLevel() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

private:
    Mesh box;
};
