#include "Level.h"
#include "Actor.h"
#include "Engine.h"
#include "GeometryGenerator.h"

#include <memory>

using namespace std;

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

void Level::SetSkybox(std::shared_ptr<Actor> newSkybox)
{
    skybox = newSkybox;
}

void Level::Update(ComPtr<ID3D11Device> &device,
                   ComPtr<ID3D11DeviceContext> &context)
{
    for (auto actor : actors)
    {
        actor->UpdateConstantBuffers(device, context);
    }
}

void Level::Render(ComPtr<ID3D11DeviceContext> &context, const bool wired)
{
    // @Refactor: skybox를 그냥 actors 포함?
    if (skybox)
    {
        Engine::Get()->GetRenderer().SetPipelineState(skybox->GetPSO(wired));
        skybox->Render(context);
    }

    for (auto &actor : actors)
    {
        Engine::Get()->GetRenderer().SetPipelineState(actor->GetPSO(wired));
        actor->Render(context);
    }

    Engine::Get()->GetRenderer().SetPipelineState(Graphics::normalsPSO);
    for (auto& actor : actors)
    {
        if (actor->drawNormals)
            actor->RenderNormals();
    }
}

void Level::RenderDepthOnly(ComPtr<ID3D11DeviceContext> &context)
{
    for (const auto &actor : actors)
    {
        Engine::Get()->GetRenderer().SetPipelineState(actor->GetDepthOnlyPSO());
        actor->Render(context);
    }
}
