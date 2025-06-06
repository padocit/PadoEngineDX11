#include "Sample_PBR.h"
#include "Actor.h"
#include "GeometryGenerator.h"

using namespace std;

Sample_PBR::Sample_PBR() : Engine()
{
}

bool Sample_PBR::InitLevel()
{
    Engine::camera.Reset(Vector3(0.0f, 0.0f, -1.0f), 0.0f, 0.0f);
    Engine::InitLevel();

    // Cubemap
    {
        renderer.InitCubemaps(
            L"../Assets/Textures/Cubemaps/", L"Atrium_specularIBL.dds",
            L"Atrium_specularIBL.dds", L"Atrium_diffuseIBL.dds",
            L"Atrium_diffuseIBL.dds");

        // 환경 박스 초기화
        MeshData skyboxMesh = GeometryGenerator::MakeBox(40.0f);
        std::reverse(skyboxMesh.indices.begin(), skyboxMesh.indices.end());
        shared_ptr<Actor> skybox = make_shared<Actor>(
            renderer.GetDevice(), renderer.GetContext(), vector{skyboxMesh});
        skybox->name = "SkyBox";
        skybox->SetPSO(Graphics::skyboxWirePSO, Graphics::skyboxSolidPSO);

        level.SetSkybox(skybox);
    }

    // PBR Sphere
    {
        MeshData sphereData = GeometryGenerator::MakeSphere(0.5f, 200, 200);
        Vector3 center = Vector3(0.0f, 0.0f, 1.0f);
        string path = "../Assets/Textures/PBR/PavingStones131/";
        sphereData.albedoTextureFilename = path + "PavingStones131_4K-PNG_Color.png";
        sphereData.normalTextureFilename = path + "PavingStones131_4K-PNG_NormalDX.png";
        sphereData.heightTextureFilename =
            path + "PavingStones131_4K-PNG_Displacement.png";
        sphereData.aoTextureFilename =
            path + "PavingStones131_4K-PNG_AmbientOcclusion.png";
         
        sphere = make_shared<Actor>(renderer.GetDevice(), renderer.GetContext(),
                                    vector{sphereData});
        sphere->UpdateWorldRow(Matrix::CreateTranslation(center));
        sphere->materialConsts.GetCpu().albedoFactor = Vector3(1.0f); // white
        sphere->UpdateConstantBuffers(renderer.GetDevice(),
                                      renderer.GetContext());
        sphere->name = "sphere";

        sphere->SetPSO(Graphics::defaultWirePSO, Graphics::defaultSolidPSO);

        level.AddActor(sphere);
    }

    // cbuffer init

    return true;
}

void Sample_PBR::UpdateGUI()
{
    Engine::UpdateGUI();

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Sphere"))
    {
        ImGui::Checkbox("Draw normals", &sphere->drawNormals);

        int flag = 0;
        flag += ImGui::CheckboxFlags(
            "AlbedoTexture",
            &sphere->materialConsts.GetCpu().useAlbedoMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use NormalMapping",
            &sphere->materialConsts.GetCpu().useNormalMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use AO", &sphere->materialConsts.GetCpu().useAOMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use HeightMapping", &sphere->meshConsts.GetCpu().useHeightMap,
            1);
        flag += ImGui::SliderFloat("HeightScale",
                                   &sphere->meshConsts.GetCpu().heightScale,
            0.0f, 0.1f);


        ImGui::TreePop();
    }
}

void Sample_PBR::Update(float dt)
{
    // cbuffer update

    Engine::Update(dt);
}

void Sample_PBR::Render()
{
    Engine::Render();
}
