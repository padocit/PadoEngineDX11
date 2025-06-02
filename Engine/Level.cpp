#include "Level.h"
#include "Actor.h"
#include "Engine.h"

Level::Level()
{
}

Level::~Level()
{
}

void Level::AddActor(std::shared_ptr<Actor> newActor)
{
    actors.push_back(newActor);
}

void Level::Update(ComPtr<ID3D11Device> &device,
                   ComPtr<ID3D11DeviceContext> &context)
{
    for (auto actor : actors)
    {
        actor->UpdateConstantBuffers(device, context);
    }
}

void Level::Render(ComPtr<ID3D11DeviceContext> &context)
{
    for (auto actor : actors)
    {
        actor->Render(context);
    }
}