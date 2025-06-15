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

        level->SetSkybox(skybox);
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

        level->AddActor(terrain);
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
        sphere->isPickable = true;
        sphere->name = "sphere";

        sphere->SetPSO(Graphics::defaultWirePSO, Graphics::defaultSolidPSO);

        level->AddActor(sphere);
    }

    // Character
    //{
    //    string path = "../Assets/Characters/UEMannequin/";
    //    auto meshes = GeometryGenerator::ReadFromFile(path, "MannequinBlenderASCII.fbx");        

    //    Vector3 center(0.0f, 0.0f, 2.0f);
    //    character = make_shared<Actor>(renderer.GetDevice(),
    //                                       renderer.GetContext(), meshes);
    //    character->materialConsts.GetCpu().invertNormalMapY =
    //        false; // GLTF는 true
    //    character->materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
    //    character->UpdateWorldRow(Matrix::CreateTranslation(center));
    //    character->UpdateConstantBuffers(renderer.GetDevice(),
    //                                  renderer.GetContext());

    //    character->isPickable = true; // 마우스로 선택/이동 가능
    //    character->name = "character";

    //    character->SetPSO(Graphics::defaultWirePSO, Graphics::defaultSolidPSO);

    //    level->AddActor(character);
    //}

    // 3D Model
    {
        string path = "../Assets/Models/DamagedHelmet/";
        auto meshes =
            GeometryGenerator::ReadFromFile(path, "DamagedHelmet.gltf");

        Vector3 center(0.0f, 0.05f, 2.0f);
        helmet = make_shared<Actor>(renderer.GetDevice(),
                                       renderer.GetContext(), meshes);
        helmet->materialConsts.GetCpu().invertNormalMapY =
            true; // GLTF는 true
        helmet->materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
        helmet->UpdateWorldRow(Matrix::CreateTranslation(center));
        helmet->UpdateConstantBuffers(renderer.GetDevice(),
                                         renderer.GetContext());

        helmet->isPickable = true; // 마우스로 선택/이동 가능
        helmet->name = "helmet";

        helmet->SetPSO(Graphics::defaultWirePSO, Graphics::defaultSolidPSO);

        level->AddActor(helmet);

    }

    // cbuffer init

    return true;
}

void Sample_PBR::UpdateGUI()
{
    Engine::UpdateGUI();

    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    if (ImGui::TreeNode("Terrain"))
    {
        ImGui::Checkbox("Draw normals", &terrain->drawNormals);

        int flag = 0;
        flag += ImGui::SliderFloat(
            "Metallic", &terrain->materialConsts.GetCpu().metallicFactor, 0.0f,
            1.0f);
        flag += ImGui::SliderFloat(
            "Roughness", &terrain->materialConsts.GetCpu().roughnessFactor,
            0.0f, 1.0f);

        flag += ImGui::CheckboxFlags(
            "AlbedoTexture", &terrain->materialConsts.GetCpu().useAlbedoMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use NormalMapping", &terrain->materialConsts.GetCpu().useNormalMap,
            1);
        flag += ImGui::CheckboxFlags(
            "Use AO", &terrain->materialConsts.GetCpu().useAOMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use HeightMapping", &terrain->meshConsts.GetCpu().useHeightMap, 1);
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
    if (ImGui::TreeNode("Material"))
    {

        int flag = 0;

        if (pickedModel)
        {
            flag += ImGui::SliderFloat(
                "Metallic",
                &pickedModel->materialConsts.GetCpu().metallicFactor,
                0.0f,
                1.0f);
            flag += ImGui::SliderFloat(
                "Roughness",
                &pickedModel->materialConsts.GetCpu().roughnessFactor,
                0.0f,
                1.0f);
            flag += ImGui::CheckboxFlags(
                "AlbedoTexture",
                &pickedModel->materialConsts.GetCpu().useAlbedoMap,
                1);
            flag += ImGui::CheckboxFlags(
                "EmissiveTexture",
                &pickedModel->materialConsts.GetCpu().useEmissiveMap,
                1);
            flag += ImGui::CheckboxFlags(
                "Use NormalMapping",
                &pickedModel->materialConsts.GetCpu().useNormalMap,
                1);
            flag += ImGui::CheckboxFlags(
                "Use AO", &pickedModel->materialConsts.GetCpu().useAOMap, 1);
            flag += ImGui::CheckboxFlags(
                "Use HeightMapping",
                &pickedModel->meshConsts.GetCpu().useHeightMap,
                1);
            flag += ImGui::SliderFloat("HeightScale", &pickedModel->meshConsts.GetCpu().heightScale,
                0.0f, 0.1f);
            flag += ImGui::CheckboxFlags(
                "Use MetallicMap",
                &pickedModel->materialConsts.GetCpu().useMetallicMap,
                1);
            flag += ImGui::CheckboxFlags(
                "Use RoughnessMap",
                &pickedModel->materialConsts.GetCpu().useRoughnessMap, 1);

            if (flag)
            {
                pickedModel->UpdateConstantBuffers(renderer.GetDevice(),
                                                 renderer.GetContext());
            }
            ImGui::Checkbox("Draw normals", &pickedModel->drawNormals);
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
