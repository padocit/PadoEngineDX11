#include "Sample_Mipmap.h"
#include "GeometryGenerator.h"
#include "Actor.h"

using namespace std;

Sample_Mipmap::Sample_Mipmap() 
    : Engine()
{
}

bool Sample_Mipmap::InitLevel()
{
    Engine::camera.Reset(Vector3(0.0f, 0.0f, -1.0f), 0.0f, 0.0f);
    Engine::InitLevel();

    // skybox
    {
        renderer.InitCubemaps(
            L"../Assets/Textures/Cubemaps/", L"DGarden_specularIBL.dds",
            L"DGarden_specularIBL.dds", L"DGarden_diffuseIBL.dds",
            L"DGarden_diffuseIBL.dds");

        MeshData skyboxMesh = GeometryGenerator::MakeBox(40.0f);
        std::reverse(skyboxMesh.indices.begin(), skyboxMesh.indices.end());
        shared_ptr<Actor> skybox = make_shared<Actor>(
            renderer.GetDevice(), renderer.GetContext(), vector{skyboxMesh});
        skybox->name = "SkyBox";
        skybox->SetPSO(Graphics::skyboxWirePSO, Graphics::skyboxSolidPSO);

        level.SetSkybox(skybox);
    }

    // Sphere: SampleLevel() - Manual LOD
    {
        MeshData sphereData = GeometryGenerator::MakeSphere(0.3f, 100, 100);
        Vector3 center = Vector3(0.0f, 0.0f, 1.0f);
        sphereData.albedoTextureFilename = "../Assets/Textures/earth.jpg";

        sphere = make_shared<Actor>(renderer.GetDevice(), renderer.GetContext(),
                                    vector{sphereData});
        sphere->UpdateWorldRow(Matrix::CreateTranslation(center));
        sphere->UpdateConstantBuffers(renderer.GetDevice(),
                                      renderer.GetContext());
        sphere->name = "sphere";

        sphere->SetPSO(Graphics::defaultWirePSO, Graphics::mipmapSolidPSO);

        level.AddActor(sphere);
    }

    // Ground: Sample() - Auto LOD
    {
        MeshData groundData = GeometryGenerator::MakeSquare(20.0f, Vector2(8.0f));
        groundData.albedoTextureFilename = "../Assets/Textures/blender_uv_grid_2k.png";

        shared_ptr<Actor> ground = make_shared<Actor>(renderer.GetDevice(), renderer.GetContext(),
                                    vector{groundData});
        ground->materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
        ground->name = "ground";

        Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
        ground->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
                                 Matrix::CreateTranslation(position));

        ground->SetPSO(Graphics::defaultWirePSO, Graphics::defaultSolidPSO);
        
        level.AddActor(ground);
    }  

    // cbuffer init
    mipmapConstsCPU.lod = 0.0f;
    D3D11Utils::CreateConstBuffer(Engine::Get()->GetRenderer().GetDevice(),
                                  mipmapConstsCPU, mipmapConstsGPU);
    Engine::Get()->GetRenderer().GetContext()->PSSetConstantBuffers(
        3, 1, mipmapConstsGPU.GetAddressOf());

    return true;
}

void Sample_Mipmap::UpdateGUI()
{
    Engine::UpdateGUI();

    ImGui::Checkbox("Draw Normals", &sphere->drawNormals);
    ImGui::SliderInt("Use texture",
                     &sphere->materialConsts.GetCpu().useAlbedoMap, 0, 1);

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Sample Mipmap"))
    {
        ImGui::SliderFloat("LOD Level", &mipmapConstsCPU.lod, 0.0f, 8.0f);
        ImGui::TreePop();
    }

}

void Sample_Mipmap::Update(float dt)
{
    // cbuffer update
    D3D11Utils::UpdateBuffer(Engine::Get()->GetRenderer().GetContext(),
                             mipmapConstsCPU, mipmapConstsGPU);

    Engine::Update(dt);
}

void Sample_Mipmap::Render()
{
    Engine::Render();
}
