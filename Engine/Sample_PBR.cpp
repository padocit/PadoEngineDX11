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
        Vector3 center = Vector3(0.0f, 0.5f, 4.0f); 
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

    // Character
    {
        string path = "../Assets/Characters/UEMannequin/";
        auto meshes = GeometryGenerator::ReadFromFile(path, "MannequinBlenderASCII.fbx");        

        Vector3 center(0.0f, 0.0f, 2.0f);
        character = make_shared<Actor>(renderer.GetDevice(),
                                           renderer.GetContext(), meshes);
        character->materialConsts.GetCpu().invertNormalMapY =
            false; // GLTF는 true
        character->materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
        character->materialConsts.GetCpu().roughnessFactor = 0.3f;
        character->materialConsts.GetCpu().metallicFactor = 0.8f;
        character->UpdateWorldRow(Matrix::CreateTranslation(center));
        character->UpdateConstantBuffers(renderer.GetDevice(),
                                      renderer.GetContext());

        character->isPickable = true; // 마우스로 선택/이동 가능
        character->name = "character";

        character->SetPSO(Graphics::defaultWirePSO, Graphics::defaultSolidPSO);

        level.AddActor(character);
    }

    // 3D Model
    {
        string path = "../Assets/Models/DamagedHelmet/";
        auto meshes =
            GeometryGenerator::ReadFromFile(path, "DamagedHelmet.gltf");

        Vector3 center(3.0f, 0.05f, 2.0f);
        helmet = make_shared<Actor>(renderer.GetDevice(),
                                       renderer.GetContext(), meshes);
        helmet->materialConsts.GetCpu().invertNormalMapY =
            true; // GLTF는 true
        helmet->materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
        helmet->materialConsts.GetCpu().roughnessFactor = 0.3f;
        helmet->materialConsts.GetCpu().metallicFactor = 0.8f;
        helmet->UpdateWorldRow(Matrix::CreateTranslation(center));
        helmet->UpdateConstantBuffers(renderer.GetDevice(),
                                         renderer.GetContext());

        helmet->isPickable = true; // 마우스로 선택/이동 가능
        helmet->name = "helmet";

        helmet->SetPSO(Graphics::defaultWirePSO, Graphics::defaultSolidPSO);

        level.AddActor(helmet);

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

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Character"))
    {
        ImGui::Checkbox("Draw normals", &character->drawNormals);

        int flag = 0;
        flag += ImGui::SliderFloat(
            "Metallic", &character->materialConsts.GetCpu().metallicFactor,
            0.0f,
            1.0f);
        flag += ImGui::SliderFloat(
            "Roughness", &character->materialConsts.GetCpu().roughnessFactor,
            0.0f,
            1.0f);
        flag += ImGui::CheckboxFlags(
            "AlbedoTexture", &character->materialConsts.GetCpu().useAlbedoMap,
            1);
        flag += ImGui::CheckboxFlags(
            "EmissiveTexture",
            &character->materialConsts.GetCpu().useEmissiveMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use NormalMapping",
            &character->materialConsts.GetCpu().useNormalMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use AO", &character->materialConsts.GetCpu().useAOMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use HeightMapping", &character->meshConsts.GetCpu().useHeightMap,
            1);
        flag += ImGui::SliderFloat("HeightScale",
                                   &character->meshConsts.GetCpu().heightScale,
            0.0f, 0.1f);
        flag += ImGui::CheckboxFlags(
            "Use MetallicMap",
            &character->materialConsts.GetCpu().useMetallicMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use RoughnessMap",
            &character->materialConsts.GetCpu().useRoughnessMap, 1);

        if (flag)
        {
            character->UpdateConstantBuffers(renderer.GetDevice(),
                                             renderer.GetContext());
        }


        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Helmet"))
    {
        ImGui::Checkbox("Draw normals", &helmet->drawNormals);

        int flag = 0;
        flag += ImGui::SliderFloat(
            "Metallic", &helmet->materialConsts.GetCpu().metallicFactor,
            0.0f,
            1.0f);
        flag += ImGui::SliderFloat(
            "Roughness", &helmet->materialConsts.GetCpu().roughnessFactor,
            0.0f,
            1.0f);
        flag += ImGui::CheckboxFlags(
            "AlbedoTexture", &helmet->materialConsts.GetCpu().useAlbedoMap,
            1);
        flag += ImGui::CheckboxFlags(
            "EmissiveTexture", &helmet->materialConsts.GetCpu().useEmissiveMap,
            1);
        flag += ImGui::CheckboxFlags(
            "Use NormalMapping", &helmet->materialConsts.GetCpu().useNormalMap,
            1);
        flag += ImGui::CheckboxFlags(
            "Use AO", &helmet->materialConsts.GetCpu().useAOMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use HeightMapping", &helmet->meshConsts.GetCpu().useHeightMap,
            1);
        flag += ImGui::SliderFloat("HeightScale",
                                   &helmet->meshConsts.GetCpu().heightScale,
            0.0f, 0.1f);
        flag += ImGui::CheckboxFlags(
            "Use MetallicMap", &helmet->materialConsts.GetCpu().useMetallicMap,
            1);
        flag += ImGui::CheckboxFlags(
            "Use RoughnessMap",
            &helmet->materialConsts.GetCpu().useRoughnessMap, 1);

        if (flag)
        {
            helmet->UpdateConstantBuffers(renderer.GetDevice(),
                                             renderer.GetContext());
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
