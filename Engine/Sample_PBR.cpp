#include "Sample_PBR.h"
#include "Actor.h"
#include "TerrainActor.h"
#include "GeometryGenerator.h"

using namespace std;

Sample_PBR::Sample_PBR() : Engine()
{
}

bool Sample_PBR::InitLevel()
{
    Engine::camera.Reset(Vector3(0.0f, 0.5f, -1.0f), 0.0f, 0.0f);
    Engine::InitLevel();

    // Cubemap
    {
        renderer.InitCubemaps(
            L"../Assets/Textures/Cubemaps/HDRI/", L"IndoorEnvHDR.dds",
            L"IndoorSpecularHDR.dds", L"IndoorDiffuseHDR.dds",
            L"IndoorBrdf.dds");

        // 환경 박스 초기화
        MeshData skyboxMesh = GeometryGenerator::MakeBox(40.0f);
        std::reverse(skyboxMesh.indices.begin(), skyboxMesh.indices.end());
        shared_ptr<Actor> skybox = make_shared<Actor>(
            renderer.GetDevice(), renderer.GetContext(), vector{skyboxMesh});
        skybox->name = "SkyBox";
        skybox->SetPSO(Graphics::skyboxWirePSO, Graphics::skyboxSolidPSO);

        level.SetSkybox(skybox);
    }

    // Terrain (ground)
    {
        //MeshData terrainData =
        //    GeometryGenerator::MakeSquareGridTerrain(4, 4, 5.0); // Square vs SquareGrid
        MeshData terrainData =
            GeometryGenerator::MakeSquareTerrain(5.0); // Square vs SquareGrid
        string path = "../Assets/Textures/PBR/PavingStones131/";
        terrainData.albedoTextureFilename =
            path + "PavingStones131_4K-PNG_Color.png";
        terrainData.normalTextureFilename =
            path + "PavingStones131_4K-PNG_NormalDX.png";
        terrainData.heightTextureFilename =
            path + "PavingStones131_4K-PNG_Displacement.png";
        terrainData.aoTextureFilename =
            path + "PavingStones131_4K-PNG_AmbientOcclusion.png";
        terrainData.emissiveTextureFilename = "";
        terrainData.metallicTextureFilename = "";
        terrainData.roughnessTextureFilename =
            path + "PavingStones131_4K-PNG_Roughness.png";

        terrain = make_shared<TerrainActor>(
            renderer.GetDevice(), renderer.GetContext(), vector{terrainData});
        terrain->materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
        terrain->name = "terrain";

        terrain->materialConsts.GetCpu().metallicFactor = 0.0f;

        Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
        terrain->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
                               Matrix::CreateTranslation(position));

        terrain->SetPSO(Graphics::terrainWirePSO, Graphics::terrainSolidPSO);

        level.AddActor(terrain);
    }    

    // PBR Sphere <- UE PBR
    {
        MeshData sphereData = GeometryGenerator::MakeSphere(0.5f, 200, 200, {2.0f, 2.0f});  // texScale 2.0f 
        Vector3 center = Vector3(0.0f, 0.5f, 1.0f); 
        string path = "../Assets/Textures/PBR/worn-painted-metal-ue/";
        sphereData.albedoTextureFilename = path + "worn-painted-metal_albedo.png";
        sphereData.normalTextureFilename = path + "worn-painted-metal_normal-dx.png";
        sphereData.heightTextureFilename =
            path + "worn-painted-metal_height.png";
        sphereData.aoTextureFilename =
            path + "worn-painted-metal_ao.png";
        sphereData.emissiveTextureFilename = "";
        sphereData.metallicTextureFilename =
            path + "worn-painted-metal_metallic.png";
        sphereData.roughnessTextureFilename =
            path + "worn-painted-metal_roughness.png";
         
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
    if (ImGui::TreeNode("Terrain"))
    {
        ImGui::Checkbox("Draw normals", &terrain->drawNormals);

        int flag = 0;
        flag += ImGui::SliderFloat(
            "Metallic", &terrain->materialConsts.GetCpu().metallicFactor, 0.0f,
            1.0f);
        flag += ImGui::SliderFloat(
            "Roughness", &terrain->materialConsts.GetCpu().roughnessFactor,
            0.0f,
            1.0f);

        flag += ImGui::CheckboxFlags(
            "AlbedoTexture", &terrain->materialConsts.GetCpu().useAlbedoMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use NormalMapping", &terrain->materialConsts.GetCpu().useNormalMap,
            1);
        flag += ImGui::CheckboxFlags(
            "Use AO", &terrain->materialConsts.GetCpu().useAOMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use HeightMapping", &terrain->meshConsts.GetCpu().useHeightMap,
            1);
        flag += ImGui::SliderFloat("HeightScale",
                                   &terrain->meshConsts.GetCpu().heightScale,
            0.0f, 1.0f);
        flag += ImGui::CheckboxFlags(
            "Use RoughnessMap",
            &terrain->materialConsts.GetCpu().useRoughnessMap, 1);

        if (flag)
        {
            terrain->UpdateConstantBuffers(renderer.GetDevice(),
                                          renderer.GetContext());
        }

        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Sphere"))
    {
        ImGui::Checkbox("Draw normals", &sphere->drawNormals);

        int flag = 0;
        flag += ImGui::SliderFloat(
            "Metallic", &sphere->materialConsts.GetCpu().metallicFactor, 0.0f,
            1.0f);
        flag += ImGui::SliderFloat(
            "Roughness", &sphere->materialConsts.GetCpu().roughnessFactor,
            0.0f,
            1.0f);
        flag += ImGui::CheckboxFlags(
            "AlbedoTexture",
            &sphere->materialConsts.GetCpu().useAlbedoMap, 1);
        flag += ImGui::CheckboxFlags(
            "EmissiveTexture",
            &sphere->materialConsts.GetCpu().useEmissiveMap, 1);
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
        flag += ImGui::CheckboxFlags(
            "Use MetallicMap",
            &sphere->materialConsts.GetCpu().useMetallicMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use RoughnessMap",
            &sphere->materialConsts.GetCpu().useRoughnessMap, 1);

        if (flag)
        {
            sphere->UpdateConstantBuffers(renderer.GetDevice(), renderer.GetContext());
        }


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
