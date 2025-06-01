#pragma once
#include "Engine.h"
class Sample_TEMPLATE : public Engine
{
public:
    Sample_TEMPLATE();

    virtual bool InitLevel() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

private:
};
