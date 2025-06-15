#include "Level.h"
#include "Actor.h"
#include "Engine.h"
#include "GeometryGenerator.h"
#include "TerrainActor.h"
#include "D3D11Utils.h"

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

void Level::SetMirror(std::shared_ptr<Actor> newMirror,
                      const Vector3 &position, const Vector3 &normal)
{
    mirrorPlane = DirectX::SimpleMath::Plane(position, normal);
    mirror = newMirror;
}

void Level::Update(ComPtr<ID3D11Device> &device,
                   ComPtr<ID3D11DeviceContext> &context)
{
    for (auto &actor : actors)
    {
        actor->UpdateConstantBuffers(device, context);
    }
}

void Level::Render(ComPtr<ID3D11DeviceContext> &context, const bool wired)
{
    // skybox
    if (skybox)
    {
        Engine::Get()->GetRenderer().SetPipelineState(skybox->GetPSO(wired));
        skybox->Render(context);
    }

    // 일반 물체들
    for (auto &actor : actors)
    {
        Engine::Get()->GetRenderer().SetPipelineState(actor->GetPSO(wired));
        actor->Render(context);
    }

    // 거울 반사 그릴 필요 없으면 불투명 거울 그림
    if (mirrorAlpha == 1.0f && mirror)
    {
        Engine::Get()->GetRenderer().SetPipelineState(
            wired ? Graphics::defaultWirePSO : Graphics::defaultSolidPSO);
        mirror->Render(context);
    }

    // 노멀 벡터 그리기
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

    Engine::Get()->GetRenderer().SetPipelineState(Graphics::depthOnlyPSO);
    if (skybox)
        skybox->Render(Engine::Get()->GetRenderer().GetContext());
    if (mirror)
        mirror->Render(Engine::Get()->GetRenderer().GetContext());
}
