/*
 * Unreal Engine PBR
 * Ground <- Mirror (Wireframe = Tessellation)
 * Post Effect: Fog
 * Post Process: Bloom, Exposure, Gamma
 */
#pragma once
#include "Engine.h"
#include "TerrainActor.h"

class Sample_Basic : public Engine
{
public:
    Sample_Basic();

    virtual bool InitLevel() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

private:
    shared_ptr<Actor> sphere;
    shared_ptr<Actor> character;
    shared_ptr<Actor> helmet;
};
