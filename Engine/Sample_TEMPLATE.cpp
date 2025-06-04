#include "Sample_TEMPLATE.h"

Sample_TEMPLATE::Sample_TEMPLATE() : Engine() {}

bool Sample_TEMPLATE::InitLevel()
{
    Engine::camera.Reset(Vector3(0.0f, 0.0f, -2.5f), 0.0f, 0.0f);
    Engine::InitLevel();

    // Actor


    // cbuffer init
    sampleConstsCPU.sampleValues = Vector3(0.0f, 0.0f, 0.0f);
    D3D11Utils::CreateConstBuffer(Engine::Get()->GetRenderer().GetDevice(),
                                  sampleConstsCPU, sampleConstsGPU);
    Engine::Get()->GetRenderer().GetContext()->PSSetConstantBuffers(
        3, 1, sampleConstsGPU.GetAddressOf());

    return true;
}

void Sample_TEMPLATE::UpdateGUI()
{
    Engine::UpdateGUI();
    //ImGui::SliderInt("Use texture",
    //                 &sampleActor->materialConsts.GetCpu().useAlbedoMap, 0, 1);
    //ImGui::Checkbox("Draw normals", &sampleActor->drawNormals);

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Sample"))
    {
        ImGui::TreePop();
    }
}

void Sample_TEMPLATE::Update(float dt)
{
    // cbuffer update
    D3D11Utils::UpdateBuffer(Engine::Get()->GetRenderer().GetContext(),
                             sampleConstsCPU, sampleConstsGPU);

    Engine::Update(dt);
}

void Sample_TEMPLATE::Render()
{
    Engine::Render();
}
