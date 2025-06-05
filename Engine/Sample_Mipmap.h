// Mipmap -> LOD
#pragma once

#include "Engine.h"

__declspec(align(256)) struct MipmapConstants
{
    float lod;
    DirectX::SimpleMath::Vector3 dummy;
};

class Sample_Mipmap : public Engine
{
public:
    Sample_Mipmap();

    virtual bool InitLevel() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

private:
    shared_ptr<Actor> sphere;
    MipmapConstants mipmapConstsCPU;
    ComPtr<ID3D11Buffer> mipmapConstsGPU;
};
