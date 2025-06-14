/*
* PBR (Unreal Engine PBR)
* Ground - Tessellation
*/
#pragma once
#include "Engine.h"
#include "TerrainActor.h"

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
    shared_ptr<Actor> character;
    shared_ptr<Actor> helmet;
    shared_ptr<TerrainActor> terrain;
};
