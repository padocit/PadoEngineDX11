#include "Sample_IBL.h"
#include "GeometryGenerator.h"
#include "Actor.h"

using namespace std;

Sample_IBL::Sample_IBL() 
    : Engine()
{
}

bool Sample_IBL::InitLevel()
{
    Engine::camera.Reset(Vector3(0.0f, 0.0f, -1.0f), 0.0f, 0.0f);
    Engine::InitLevel();

    // Cubemap
    {
        renderer.InitCubemaps(L"../Assets/Textures/Cubemaps/",
                              L"Atrium_specularIBL.dds", L"Atrium_specularIBL.dds",
                              L"Atrium_diffuseIBL.dds", L"Atrium_diffuseIBL.dds");

        // 환경 박스 초기화
        MeshData skyboxMesh = GeometryGenerator::MakeBox(40.0f);
        std::reverse(skyboxMesh.indices.begin(), skyboxMesh.indices.end());
        shared_ptr<Actor> skybox = make_shared<Actor>(
            renderer.GetDevice(), renderer.GetContext(), vector{skyboxMesh});
        skybox->name = "SkyBox";
        skybox->SetPSO(Graphics::skyboxWirePSO, Graphics::skyboxSolidPSO);

        level.SetSkybox(skybox);
    }

    // Sphere
    {
        MeshData sphereData = GeometryGenerator::MakeSphere(0.5f, 200, 200);
        Vector3 center = Vector3(0.0f, 0.0f, 1.0f);
        sphereData.albedoTextureFilename = "../Assets/Textures/earth.jpg";

        sphere = make_shared<Actor>(renderer.GetDevice(), renderer.GetContext(),
                                    vector{sphereData});
        sphere->UpdateWorldRow(Matrix::CreateTranslation(center));
        sphere->materialConsts.GetCpu().albedoFactor = Vector3(1.0f); // white
        sphere->UpdateConstantBuffers(renderer.GetDevice(),
                                      renderer.GetContext());
        sphere->name = "sphere";

        sphere->SetPSO(Graphics::defaultWirePSO, Graphics::iblSolidPSO);

        level.AddActor(sphere);
    }

    // cbuffer
    iblConstsCPU.fresnelR0 = Vector3(1.0, 0.71, 0.29); // Gold
    D3D11Utils::CreateConstBuffer(Engine::Get()->GetRenderer().GetDevice(),
                                  iblConstsCPU, iblConstsGPU);
    Engine::Get()->GetRenderer().GetContext()->PSSetConstantBuffers(
        3, 1, iblConstsGPU.GetAddressOf());

    return true;
}

void Sample_IBL::UpdateGUI()
{
    Engine::UpdateGUI();
    ImGui::SliderInt("Use texture", &sphere->materialConsts.GetCpu().useAlbedoMap, 0, 1);
    ImGui::Checkbox("Draw normals", &sphere->drawNormals);

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Sample IBL"))
    {
        ImGui::SliderFloat3("FresnelR0", &iblConstsCPU.fresnelR0.x, 0.0f, 1.0f);
        ImGui::SliderFloat("Diffuse",
                           &sphere->materialConsts.GetCpu().roughnessFactor,
                           0.0f, 3.0f);
        ImGui::SliderFloat("Specular",
                           &sphere->materialConsts.GetCpu().metallicFactor,
                           0.0f, 3.0f);
        ImGui::SliderFloat("Shininess",
                           &sphere->materialConsts.GetCpu().shininess, 0.01f,
                           25.0f);

        ImGui::TreePop();
    }
}

void Sample_IBL::Update(float dt)
{
    // cbuffer
    D3D11Utils::UpdateBuffer(Engine::Get()->GetRenderer().GetContext(),
                             iblConstsCPU, iblConstsGPU);

    Engine::Update(dt);
}

void Sample_IBL::Render()
{
    Engine::Render();
}
