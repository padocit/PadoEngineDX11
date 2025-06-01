#include "ModelLoader.h"

#include <DirectXMesh.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <vector>

using namespace std;
using namespace DirectX::SimpleMath;

string GetExtension(const string filename)
{
    string ext(filesystem::path(filename).extension().string());
    transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

void ModelLoader::Load(string newBasePath, string filename,
                             bool newRevertNormals)
{
    if (GetExtension(filename) == ".gltf")
    {
        isGLTF = true;
        revertNormals = newRevertNormals;
    }

    basePath = newBasePath; // 텍스춰 읽어들일 때 필요

    Assimp::Importer importer;

    const aiScene *pScene = importer.ReadFile(
        basePath + filename,
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
    // ReadFile()에서 경우에 따라서 여러가지 옵션들 설정 가능
    // aiProcess_JoinIdenticalVertices | aiProcess_PopulateArmatureData |
    // aiProcess_SplitByBoneCount |
    // aiProcess_Debone); // aiProcess_LimitBoneWeights

    if (pScene)
    {

        // 1. 모든 메쉬에 대해서 버텍스에 영향을 주는 뼈들의 목록을 만든다.
        FindDeformingBones(pScene);

        // 2. 트리 구조를 따라 업데이트 순서대로 뼈들의 인덱스를 결정한다
        int counter = 0;
        UpdateBoneIDs(pScene->mRootNode, &counter);

        // 3. 업데이트 순서대로 뼈 이름 저장 (boneIdToName)
        aniData.boneIdToName.resize(aniData.boneNameToId.size());
        for (auto &i : aniData.boneNameToId)
            aniData.boneIdToName[i.second] = i.first;

        // 디버깅용
        // cout << "Num boneNameToId : " << m_aniData.boneNameToId.size() <<
        // endl; for (auto &i : m_aniData.boneNameToId) {
        //    cout << "NameId pair : " << i.first << " " << i.second << endl;
        //}
        // cout << "Num boneIdToName : " << m_aniData.boneIdToName.size() <<
        // endl; for (size_t i = 0; i < m_aniData.boneIdToName.size(); i++) {
        //    cout << "BoneId: " << i << " " << m_aniData.boneIdToName[i] <<
        //    endl;
        //}
        // exit(-1);

        // 각 뼈의 부모 인덱스를 저장할 준비
        aniData.boneParents.resize(aniData.boneNameToId.size(), -1);

        Matrix tr; // Initial transformation
        ProcessNode(pScene->mRootNode, pScene, tr);

        // 디버깅용
        // cout << "Num boneIdToName : " << m_aniData.boneIdToName.size() <<
        // endl; for (size_t i = 0; i < m_aniData.boneIdToName.size(); i++) {
        //    cout << "BoneId: " << i << " " << m_aniData.boneIdToName[i]
        //         << " , Parent: "
        //         << (m_aniData.boneParents[i] == -1
        //                 ? "NONE"
        //                 : m_aniData.boneIdToName[m_aniData.boneParents[i]])
        //         << endl;
        //}

        // 애니메이션 정보 읽기
        if (pScene->HasAnimations())
            ReadAnimation(pScene);

        // UpdateNormals(this->meshes); // Vertex Normal을 직접 계산 (참고용)

        UpdateTangents();
    }
    else
    {
        std::cout << "Failed to read file: " << basePath + filename
                  << std::endl;
        auto errorDescription = importer.GetErrorString();
        std::cout << "Assimp error: " << errorDescription << endl;
    }
}

void ModelLoader::LoadAnimation(string newBasePath, string filename)
{
    basePath = newBasePath;

    Assimp::Importer importer;

    const aiScene *pScene = importer.ReadFile(
        basePath + filename,
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

    if (pScene && pScene->HasAnimations())
    {
        ReadAnimation(pScene);
    }
    else
    {
        std::cout << "Failed to read animation from file: "
                  << basePath + filename << std::endl;
        auto errorDescription = importer.GetErrorString();
        std::cout << "Assimp error: " << errorDescription << endl;
    }
}

const aiNode *ModelLoader::FindParent(const aiNode *node)
{
    if (!node)
        return nullptr;
    if (aniData.boneNameToId.count(node->mName.C_Str()) > 0)
        return node;
    return FindParent(node->mParent);
}

void ModelLoader::ProcessNode(aiNode *node, const aiScene *scene,
                                    DirectX::SimpleMath::Matrix tr)
{
    // https://ogldev.org/www/tutorial38/tutorial38.html
    // If a node represents a bone in the hierarchy then the node name must
    // match the bone name.

    // 사용되는 부모 뼈를 찾아서 부모의 인덱스 저장
    if (node->mParent && aniData.boneNameToId.count(node->mName.C_Str()) &&
        FindParent(node->mParent))
    {
        const auto boneId = aniData.boneNameToId[node->mName.C_Str()];
        aniData.boneParents[boneId] =
            aniData.boneNameToId[FindParent(node->mParent)->mName.C_Str()];
    }

    Matrix m(&node->mTransformation.a1);
    m = m.Transpose() * tr;

    for (UINT i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        auto newMesh = this->ProcessMesh(mesh, scene);
        for (auto &v : newMesh.vertices)
        {
            v.position = DirectX::SimpleMath::Vector3::Transform(v.position, m);
        }
        meshes.push_back(newMesh);
    }

    for (UINT i = 0; i < node->mNumChildren; i++)
    {
        this->ProcessNode(node->mChildren[i], scene, m);
    }
}

MeshData ModelLoader::ProcessMesh(aiMesh *mesh, const aiScene *scene)
{
    MeshData newMesh;
    auto &vertices = newMesh.vertices;
    auto &indices = newMesh.indices;
    auto &skinnedVertices = newMesh.skinnedVertices;

    // Walk through each of the mesh's vertices
    for (UINT i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        vertex.normalModel.x = mesh->mNormals[i].x;
        if (isGLTF)
        {
            vertex.normalModel.y = mesh->mNormals[i].z;
            vertex.normalModel.z = -mesh->mNormals[i].y;
        }
        else
        {
            vertex.normalModel.y = mesh->mNormals[i].y;
            vertex.normalModel.z = mesh->mNormals[i].z;
        }

        if (revertNormals)
        {
            vertex.normalModel *= -1.0f;
        }

        vertex.normalModel.Normalize();

        if (mesh->mTextureCoords[0])
        {
            vertex.texcoord.x = (float)mesh->mTextureCoords[0][i].x;
            vertex.texcoord.y = (float)mesh->mTextureCoords[0][i].y;
        }

        vertices.push_back(vertex);
    }

    for (UINT i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (UINT j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    if (mesh->HasBones())
    {

        vector<vector<float>> boneWeights(vertices.size());
        vector<vector<uint8_t>> boneIndices(vertices.size());

        aniData.offsetMatrices.resize(aniData.boneNameToId.size());
        aniData.boneTransforms.resize(aniData.boneNameToId.size());

        int count = 0;
        for (uint32_t i = 0; i < mesh->mNumBones; i++)
        {
            const aiBone *bone = mesh->mBones[i];

            // 디버깅
            // cout << "BoneMap " << count++ << " : " << bone->mName.C_Str()
            //     << " NumBoneWeights = " << bone->mNumWeights << endl;

            const uint32_t boneId = aniData.boneNameToId[bone->mName.C_Str()];

            aniData.offsetMatrices[boneId] =
                Matrix((float *)&bone->mOffsetMatrix).Transpose();

            // 이 뼈가 영향을 주는 Vertex의 개수
            for (uint32_t j = 0; j < bone->mNumWeights; j++)
            {
                aiVertexWeight weight = bone->mWeights[j];
                assert(weight.mVertexId < boneIndices.size());
                boneIndices[weight.mVertexId].push_back(boneId);
                boneWeights[weight.mVertexId].push_back(weight.mWeight);
            }
        }

        // 예전에는 Vertex 하나에 영향을 주는 Bone은 최대 4개
        // 요즘은 더 많을 수도 있는데 모델링 소프트웨어에서 조정하거나
        // 읽어들이면서 weight가 너무 작은 것들은 뺄 수도 있음

        int maxBones = 0;
        for (int i = 0; i < boneWeights.size(); i++)
        {
            maxBones = max(maxBones, int(boneWeights[i].size()));
        }

        cout << "Max number of influencing bones per vertex = " << maxBones
             << endl;

        skinnedVertices.resize(vertices.size());
        for (int i = 0; i < vertices.size(); i++)
        {
            skinnedVertices[i].position = vertices[i].position;
            skinnedVertices[i].normalModel = vertices[i].normalModel;
            skinnedVertices[i].texcoord = vertices[i].texcoord;

            for (int j = 0; j < boneWeights[i].size(); j++)
            {
                skinnedVertices[i].blendWeights[j] = boneWeights[i][j];
                skinnedVertices[i].boneIndices[j] = boneIndices[i][j];
            }
        }

        // 디버깅용 출력 (boneWeights)
        // for (int i = 0; i < boneWeights.size(); i++) {
        //    cout << boneWeights[i].size() << " : ";
        //    for (int j = 0; j < boneWeights[i].size(); j++) {
        //        cout << boneWeights[i][j] << " ";
        //    }
        //    cout << " | ";
        //    for (int j = 0; j < boneIndices[i].size(); j++) {
        //        cout << int(boneIndices[i][j]) << " ";
        //    }
        //    cout << endl;
        //}
    }

    // http://assimp.sourceforge.net/lib_html/materials.html
    if (mesh->mMaterialIndex >= 0)
    {

        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        newMesh.albedoTextureFilename =
            ReadTextureFilename(scene, material, aiTextureType_BASE_COLOR);
        if (newMesh.albedoTextureFilename.empty())
        {
            newMesh.albedoTextureFilename =
                ReadTextureFilename(scene, material, aiTextureType_DIFFUSE);
        }
        newMesh.emissiveTextureFilename =
            ReadTextureFilename(scene, material, aiTextureType_EMISSIVE);
        newMesh.heightTextureFilename =
            ReadTextureFilename(scene, material, aiTextureType_HEIGHT);
        newMesh.normalTextureFilename =
            ReadTextureFilename(scene, material, aiTextureType_NORMALS);
        newMesh.metallicTextureFilename =
            ReadTextureFilename(scene, material, aiTextureType_METALNESS);
        newMesh.roughnessTextureFilename = ReadTextureFilename(
            scene, material, aiTextureType_DIFFUSE_ROUGHNESS);
        newMesh.aoTextureFilename = ReadTextureFilename(
            scene, material, aiTextureType_AMBIENT_OCCLUSION);
        if (newMesh.aoTextureFilename.empty())
        {
            newMesh.aoTextureFilename =
                ReadTextureFilename(scene, material, aiTextureType_LIGHTMAP);
        }
        newMesh.opacityTextureFilename =
            ReadTextureFilename(scene, material, aiTextureType_OPACITY);

        if (!newMesh.opacityTextureFilename.empty())
        {
            cout << newMesh.albedoTextureFilename << endl;
            cout << "Opacity " << newMesh.opacityTextureFilename << endl;
        }

        // 디버깅용
        // for (size_t i = 0; i < 22; i++) {
        //    cout << i << " " << ReadTextureFilename(material,
        //    aiTextureType(i))
        //         << endl;
        //}
    }

    return newMesh;
}

void ModelLoader::ReadAnimation(const aiScene *scene)
{
    aniData.clips.resize(scene->mNumAnimations);

    for (uint32_t i = 0; i < scene->mNumAnimations; i++)
    {

        auto &clip = aniData.clips[i];

        const aiAnimation *ani = scene->mAnimations[i];

        clip.duration = ani->mDuration;
        clip.ticksPerSec = ani->mTicksPerSecond;
        clip.keys.resize(aniData.boneNameToId.size());
        clip.numChannels = ani->mNumChannels;

        for (uint32_t c = 0; c < ani->mNumChannels; c++)
        {
            const aiNodeAnim *nodeAnim = ani->mChannels[c];
            const int boneId =
                aniData.boneNameToId[nodeAnim->mNodeName.C_Str()];
            clip.keys[boneId].resize(nodeAnim->mNumPositionKeys);
            for (uint32_t k = 0; k < nodeAnim->mNumPositionKeys; k++)
            {
                const auto pos = nodeAnim->mPositionKeys[k].mValue;
                const auto rot = nodeAnim->mRotationKeys[k].mValue;
                const auto scale = nodeAnim->mScalingKeys[k].mValue;
                auto &key = clip.keys[boneId][k];
                key.pos = {pos.x, pos.y, pos.z};
                key.rot = Quaternion(rot.x, rot.y, rot.z, rot.w);
                key.scale = {scale.x, scale.y, scale.z};
            }
        }
    }
}

std::string ModelLoader::ReadTextureFilename(const aiScene *scene,
                                                   aiMaterial *material,
                                                   aiTextureType type)
{
    if (material->GetTextureCount(type) > 0)
    {
        aiString filepath;
        material->GetTexture(type, 0, &filepath);

        string fullPath =
            basePath +
            string(filesystem::path(filepath.C_Str()).filename().string());

        // 1. 실제로 파일이 존재하는지 확인
        if (!filesystem::exists(fullPath))
        {
            // 2. 파일이 없을 경우 혹시 fbx 자체에 Embedded인지 확인
            const aiTexture *texture =
                scene->GetEmbeddedTexture(filepath.C_Str());
            if (texture)
            {
                // 3. Embedded texture가 존재하고 png일 경우 저장
                if (string(texture->achFormatHint).find("png") != string::npos)
                {
                    ofstream fs(fullPath.c_str(), ios::binary | ios::out);
                    fs.write((char *)texture->pcData, texture->mWidth);
                    fs.close();
                    // 참고: compressed format일 경우 texture->mHeight가 0
                }
            }
            else
            {
                cout << fullPath << " doesn't exists. Return empty filename."
                     << endl;
            }
        }
        else
        {
            return fullPath;
        }

        return fullPath;
    }
    else
    {
        return "";
    }
}

void ModelLoader::UpdateTangents()
{
    using namespace std;
    using namespace DirectX;

    // https://github.com/microsoft/DirectXMesh/wiki/ComputeTangentFrame

    for (auto &mesh : this->meshes)
    {
        vector<XMFLOAT3> positions(mesh.vertices.size());
        vector<XMFLOAT3> normals(mesh.vertices.size());
        vector<XMFLOAT2> texcoords(mesh.vertices.size());
        vector<XMFLOAT3> tangents(mesh.vertices.size());
        vector<XMFLOAT3> bitangents(mesh.vertices.size());

        for (size_t i = 0; i < mesh.vertices.size(); i++)
        {
            auto &v = mesh.vertices[i];
            positions[i] = v.position;
            normals[i] = v.normalModel;
            texcoords[i] = v.texcoord;
        }

        ComputeTangentFrame(mesh.indices.data(), mesh.indices.size() / 3,
                            positions.data(), normals.data(), texcoords.data(),
                            mesh.vertices.size(), tangents.data(),
                            bitangents.data());

        for (size_t i = 0; i < mesh.vertices.size(); i++)
        {
            mesh.vertices[i].tangentModel = tangents[i];
        }

        if (mesh.skinnedVertices.size() > 0)
        {
            for (size_t i = 0; i < mesh.skinnedVertices.size(); i++)
            {
                mesh.skinnedVertices[i].tangentModel = tangents[i];
            }
        }
    }
}

void ModelLoader::FindDeformingBones(const aiScene *scene)
{
    for (uint32_t i = 0; i < scene->mNumMeshes; i++)
    {
        const auto *mesh = scene->mMeshes[i];
        if (mesh->HasBones())
        {
            for (uint32_t i = 0; i < mesh->mNumBones; i++)
            {
                const aiBone *bone = mesh->mBones[i];

                // bone과 대응되는 node의 이름은 동일
                // 뒤에서 node 이름으로 부모를 찾을 수 있음
                aniData.boneNameToId[bone->mName.C_Str()] = -1;

                // 주의: 뼈의 순서가 업데이트 순서는 아님

                // 기타: bone->mWeights == 0일 경우에도 포함시켰음
                // 기타: bone->mNode = 0이라서 사용 불가
            }
        }
    }
}
