#pragma once

#include "Engine.h"
#include "ConstantBuffer.h"

class Sample_Phong : public Engine
{
public:
    Sample_Phong();

    virtual bool InitLevel() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

private:
    int lightType = 0;
    Light lightFromGUI;
    shared_ptr<Actor> box;
    PhongConstants phongConstsCPU;
    ComPtr<ID3D11Buffer> phongConstsGPU;

    Vector3 boxPos;
    Vector3 boxScale;
};
