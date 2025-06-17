#include "Actor.h"
#include "GeometryGenerator.h"
#include "Sample_Basic.h"
#include "TerrainActor.h"

using namespace std;

Sample_Basic::Sample_Basic() : Engine()
{
}

bool Sample_Basic::InitLevel()
{
    Engine::camera.Reset(Vector3(0.0f, 0.5f, -1.0f), 0.0f, 0.0f);
    Engine::InitLevel();

    // Cubemap 
    {
        renderer.InitCubemaps(L"../Assets/Textures/Cubemaps/HDRI/",
                              L"SampleEnvHDR.dds", L"SampleSpecularHDR.dds",
                              L"SampleDiffuseHDR.dds", L"SampleBrdf.dds");
    }
     
    // Mirror ground
    {
        // MeshData terrainData =
        //     GeometryGenerator::MakeSquareGridTerrain(4, 4, 5.0); // Square vs
        //     SquareGrid
        MeshData mirrorData = GeometryGenerator::MakeSquare(
            5.0, {10.0f, 10.0f}); // Square vs SquareGrid
        string path = "../Assets/Textures/PBR/stringy-marble-ue/";
        mirrorData.albedoTextureFilename = path + "stringy_marble_albedo.png";
        mirrorData.emissiveTextureFilename = "";
        mirrorData.aoTextureFilename = path + "stringy_marble_ao.png";
        mirrorData.metallicTextureFilename = path + "stringy_marble_Metallic.png"; // 그냥 까만텍스쳐라서 Mirror Metallic factor 반응 X
        mirrorData.normalTextureFilename = path + "stringy_marble_Normal-dx.png";
        mirrorData.roughnessTextureFilename = path + "stringy_marble_Roughness.png";

        auto mirror = make_shared<Actor>(
            renderer.GetDevice(), renderer.GetContext(), vector{mirrorData});
        mirror->materialConsts.GetCpu().albedoFactor = Vector3(0.7f);
        mirror->materialConsts.GetCpu().emissionFactor = Vector3(0.0f);
        mirror->materialConsts.GetCpu().metallicFactor = 0.5f;
        mirror->materialConsts.GetCpu().roughnessFactor = 0.3f;
        mirror->name = "terrain";

        Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
        mirror->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
                                Matrix::CreateTranslation(position));

        mirror->SetPSO(Graphics::defaultWirePSO, Graphics::defaultSolidPSO);

        level->SetMirror(mirror, position, Vector3(0.0f, 1.0f, 0.0f));
    }

    // PBR Sphere <- UE PBR
    {
        MeshData sphereData = GeometryGenerator::MakeSphere(
            0.5f, 200, 200, {2.0f, 2.0f}); // texScale 2.0f
        Vector3 center = Vector3(0.0f, 0.5f, 4.0f);
        string path = "../Assets/Textures/PBR/worn-painted-metal-ue/";
        sphereData.albedoTextureFilename =
            path + "worn-painted-metal_albedo.png";
        sphereData.normalTextureFilename =
            path + "worn-painted-metal_normal-dx.png";
        sphereData.heightTextureFilename =
            path + "worn-painted-metal_height.png";
        sphereData.aoTextureFilename = path + "worn-painted-metal_ao.png";
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
    //    auto meshes = GeometryGenerator::ReadFromFile(path,
    //    "MannequinBlenderASCII.fbx");

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

    //    character->SetPSO(Graphics::defaultWirePSO,
    //    Graphics::defaultSolidPSO);

    //    level->AddActor(character);
    //}

    // 3D Model
    {
        string path = "../Assets/Models/DamagedHelmet/";
        auto meshes =
            GeometryGenerator::ReadFromFile(path, "DamagedHelmet.gltf");

        Vector3 center(0.0f, 0.05f, 2.0f);
        helmet = make_shared<Actor>(renderer.GetDevice(), renderer.GetContext(),
                                    meshes);
        helmet->materialConsts.GetCpu().invertNormalMapY = true; // GLTF는 true
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

void Sample_Basic::UpdateGUI()
{
    Engine::UpdateGUI();

    //ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    //if (ImGui::TreeNode("Terrain"))
    //{
    //    ImGui::Checkbox("Draw normals", &terrain->drawNormals);

    //    int flag = 0;
    //    flag += ImGui::SliderFloat(
    //        "Metallic", &terrain->materialConsts.GetCpu().metallicFactor, 0.0f,
    //        1.0f);
    //    flag += ImGui::SliderFloat(
    //        "Roughness", &terrain->materialConsts.GetCpu().roughnessFactor,
    //        0.0f, 1.0f);

    //    flag += ImGui::CheckboxFlags(
    //        "AlbedoTexture", &terrain->materialConsts.GetCpu().useAlbedoMap, 1);
    //    flag += ImGui::CheckboxFlags(
    //        "Use NormalMapping", &terrain->materialConsts.GetCpu().useNormalMap,
    //        1);
    //    flag += ImGui::CheckboxFlags(
    //        "Use AO", &terrain->materialConsts.GetCpu().useAOMap, 1);
    //    flag += ImGui::CheckboxFlags(
    //        "Use HeightMapping", &terrain->meshConsts.GetCpu().useHeightMap, 1);
    //    flag += ImGui::SliderFloat("HeightScale",
    //                               &terrain->meshConsts.GetCpu().heightScale,
    //                               0.0f, 1.0f);
    //    flag += ImGui::CheckboxFlags(
    //        "Use RoughnessMap",
    //        &terrain->materialConsts.GetCpu().useRoughnessMap, 1);

    //    if (flag)
    //    {
    //        terrain->UpdateConstantBuffers(renderer.GetDevice(),
    //                                       renderer.GetContext());
    //    }

    //    ImGui::TreePop();
    //}

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Mirror"))
    {
        auto &mirrorAlpha = level->mirrorAlpha;
        auto &mirror = level->mirror;

        ImGui::SliderFloat("Alpha", &mirrorAlpha, 0.0f, 1.0f);
        const float blendColor[4] = {mirrorAlpha, mirrorAlpha,
                                     mirrorAlpha, 1.0f};
        if (renderer.drawAsWire)
            Graphics::mirrorBlendWirePSO.SetBlendFactor(blendColor);
        else
            Graphics::mirrorBlendSolidPSO.SetBlendFactor(blendColor);

        ImGui::SliderFloat("Metallic",
                           &mirror->materialConsts.GetCpu().metallicFactor,
                           0.0f, 1.0f);
        ImGui::SliderFloat("Roughness",
                           &mirror->materialConsts.GetCpu().roughnessFactor,
                           0.0f, 1.0f);

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
                &pickedModel->materialConsts.GetCpu().metallicFactor, 0.0f,
                1.0f);
            flag += ImGui::SliderFloat(
                "Roughness",
                &pickedModel->materialConsts.GetCpu().roughnessFactor, 0.0f,
                1.0f);
            flag += ImGui::CheckboxFlags(
                "AlbedoTexture",
                &pickedModel->materialConsts.GetCpu().useAlbedoMap, 1);
            flag += ImGui::CheckboxFlags(
                "EmissiveTexture",
                &pickedModel->materialConsts.GetCpu().useEmissiveMap, 1);
            flag += ImGui::CheckboxFlags(
                "Use NormalMapping",
                &pickedModel->materialConsts.GetCpu().useNormalMap, 1);
            flag += ImGui::CheckboxFlags(
                "Use AO", &pickedModel->materialConsts.GetCpu().useAOMap, 1);
            flag += ImGui::CheckboxFlags(
                "Use HeightMapping",
                &pickedModel->meshConsts.GetCpu().useHeightMap, 1);
            flag += ImGui::SliderFloat(
                "HeightScale", &pickedModel->meshConsts.GetCpu().heightScale,
                0.0f, 0.1f);
            flag += ImGui::CheckboxFlags(
                "Use MetallicMap",
                &pickedModel->materialConsts.GetCpu().useMetallicMap, 1);
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

void Sample_Basic::Update(float dt)
{
    // cbuffer update

    Engine::Update(dt);
}

void Sample_Basic::Render()
{
    Engine::Render();
}
