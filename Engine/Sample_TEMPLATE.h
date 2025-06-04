#pragma once
#include "Engine.h"

__declspec(align(256)) struct SampleConstants
{
    DirectX::SimpleMath::Vector3 sampleValues;
    float dummy;
};

class Sample_TEMPLATE : public Engine
{
public:
    Sample_TEMPLATE();

    virtual bool InitLevel() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

private:
    shared_ptr<Actor> sampleActor;
    SampleConstants sampleConstsCPU;
    ComPtr<ID3D11Buffer> sampleConstsGPU;
};
