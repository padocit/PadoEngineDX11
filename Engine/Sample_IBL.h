#pragma once

#include "Engine.h"

__declspec(align(256)) struct IblConstants
{
    DirectX::SimpleMath::Vector3 fresnelR0;
    float dummy;
};

class Sample_IBL : public Engine
{
public:
    Sample_IBL();

    virtual bool InitLevel() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

private:
    shared_ptr<Actor> sphere;
    IblConstants iblConstsCPU;
    ComPtr<ID3D11Buffer> iblConstsGPU;
};
