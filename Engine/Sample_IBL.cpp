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
        renderer.InitCubemaps(L"../Assets/Textures/Cubemaps/HDRI/",
                              L"SampleEnvHDR.dds", L"SampleSpecularHDR.dds",
                              L"SampleDiffuseHDR.dds", L"SampleBrdf.dds");

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

        sphere->SetPSO(Graphics::defaultWirePSO, Graphics::defaultSolidPSO);

        level.AddActor(sphere);
    }

    return true;
}

void Sample_IBL::UpdateGUI()
{
    Engine::UpdateGUI();
    ImGui::SliderInt("Use texture", &sphere->materialConsts.GetCpu().useAlbedoMap, 0, 1);
}

void Sample_IBL::Update(float dt)
{
    Engine::Update(dt);
}

void Sample_IBL::Render()
{
    Engine::Render();
}
