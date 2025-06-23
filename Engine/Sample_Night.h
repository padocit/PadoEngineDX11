#pragma once
#include "Engine.h"
#include "TerrainActor.h"
#include "BillboardActor.h"

class Sample_Night : public Engine
{
public:
    Sample_Night();

    virtual bool InitLevel() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

private:
    shared_ptr<TerrainActor> terrain;
    shared_ptr<Actor> sphere;
    shared_ptr<Actor> character;
    shared_ptr<Actor> helmet;
    shared_ptr<Actor> streetLamp;
    vector<shared_ptr<BillboardActor>> trees;
};
