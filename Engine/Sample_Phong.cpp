#include "Actor.h"
#include "GeometryGenerator.h"
#include "Sample_Phong.h"

using namespace std;

Sample_Phong::Sample_Phong() 
    : Engine(1600, 900)
{
}

bool Sample_Phong::InitLevel()
{
    Engine::camera.Reset(Vector3(0.0f, 0.0f, -2.5f), 0.0f, 0.0f);
    Engine::InitLevel();

    // 박스
    { 
        MeshData boxData = GeometryGenerator::MakeBox(0.5f);
        Vector3 center(0.0f, 0.0f, 1.0f);
        boxData.albedoTextureFilename = "../Assets/Textures/box.png";

        box = make_shared<Actor>(
            renderer.GetDevice(), renderer.GetContext(), vector{boxData});
        box->UpdateWorldRow(Matrix::CreateTranslation(center));
        box->materialConsts.GetCpu().albedoFactor = Vector3(0.1f); // gray
        box->UpdateConstantBuffers(renderer.GetDevice(),
                                        renderer.GetContext());
        box->name = "Box";

        // PSO
        box->SetPSO(Graphics::phongWirePSO, Graphics::phongSolidPSO);

        level.AddActor(box);
    }

    // cbuffer (useBlinnPhong) 초기화
    phongConstsCPU.useBlinnPhong = false;
    D3D11Utils::CreateConstBuffer(Engine::Get()->GetRenderer().GetDevice(),
                                  phongConstsCPU,
                                  phongConstsGPU);
    Engine::Get()->GetRenderer().GetContext()->PSSetConstantBuffers(
        3, 1, phongConstsGPU.GetAddressOf());


    // light 세기 조정
    lightFromGUI.radiance = Vector3(1.0f);

    return true;
}

void Sample_Phong::UpdateGUI()
{
    Engine::UpdateGUI();

    // Material(Actor), useBlinnPhong (phongConsts) GUI 연결
    ImGui::Checkbox("Use texture", &phongConstsCPU.useTexture);
    ImGui::Checkbox("Draw Normals", &box->drawNormals);

    if (ImGui::RadioButton("Phong", phongConstsCPU.useBlinnPhong == 0))
    {
        phongConstsCPU.useBlinnPhong = 0;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("BlinnPhong", phongConstsCPU.useBlinnPhong == 1))
    {
        phongConstsCPU.useBlinnPhong = 1;
    }

    ImGui::SliderFloat3("Ambient", &box->materialConsts.GetCpu().albedoFactor.x,
                        0.0f, 1.0f);
    ImGui::SliderFloat("Diffuse", &box->materialConsts.GetCpu().roughnessFactor,
                       0.0f, 1.0f);
    ImGui::SliderFloat("Specular", &box->materialConsts.GetCpu().metallicFactor,
                       0.0f, 1.0f);


    if (ImGui::RadioButton("Directional Light", lightType == 0))
    {
        lightType = 0;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Point Light", lightType == 1))
    {
        lightType = 1;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Spot Light", lightType == 2))
    {
        lightType = 2;
    }

    ImGui::SliderFloat3("Light Position", &lightFromGUI.position.x, -5.0f,
                        5.0f);

    ImGui::SliderFloat3("Light Direction", &lightFromGUI.direction.x, -1.0f,
                        1.0f);

    ImGui::SliderFloat("Light fallOffStart", &lightFromGUI.fallOffStart, 0.0f,
                       5.0f);

    ImGui::SliderFloat("Light fallOffEnd", &lightFromGUI.fallOffEnd, 0.0f,
                       10.0f);

    ImGui::SliderFloat("Light spotPower", &lightFromGUI.spotPower, 1.0f,
                       512.0f);


}

void Sample_Phong::Update(float dt)
{
    // 여러 개 조명 사용 예시
    auto *lights = Engine::Get()->GetRenderer().globalConstsCPU.lights;

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        // 다른 조명 끄기
        if (i != lightType)
        {
            lights[i].radiance *= 0.0f;
        }
        else
        {
            lights[i] = lightFromGUI;
        }
    }


    // phongConsts GPU 업데이트
    D3D11Utils::UpdateBuffer(Engine::Get()->GetRenderer().GetContext(),
                             phongConstsCPU, phongConstsGPU);

    Engine::Update(dt);
}

void Sample_Phong::Render()
{
    Engine::Render();
}
