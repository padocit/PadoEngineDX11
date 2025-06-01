#pragma once

// vcpkg install assimp:x64-windows
// Preprocessor definitions에 NOMINMAX 추가
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

//#include "AnimationClip.h"
#include "MeshData.h"
#include "Vertex.h"
#include "AnimationClip.h"

class ModelLoader
{
public:
    void Load(std::string newBasePath, std::string filename,
              bool newRevertNormals);
    void LoadAnimation(string newBasePath, string filename);

    const aiNode *FindParent(const aiNode *node);

    void ProcessNode(aiNode *node, const aiScene *scene,
                     DirectX::SimpleMath::Matrix tr);

    MeshData ProcessMesh(aiMesh *mesh, const aiScene *scene);

    void ReadAnimation(const aiScene *scene);

    std::string ReadTextureFilename(const aiScene *scene, aiMaterial *material,
                                    aiTextureType type);

    void UpdateTangents();

    // 버텍스의 변형에 직접적으로 참여하는 뼈들의 목록을 만듭니다.
    void FindDeformingBones(const aiScene *scene);
    void UpdateBoneIDs(aiNode *node, int *counter)
    {
        static int id = 0;
        if (node)
        {
            if (aniData.boneNameToId.count(node->mName.C_Str()))
            {
                aniData.boneNameToId[node->mName.C_Str()] = *counter;
                *counter += 1;
            }
            for (UINT i = 0; i < node->mNumChildren; i++)
            {
                UpdateBoneIDs(node->mChildren[i], counter);
            }
        }
    }

public:
    std::string basePath;
    std::vector<MeshData> meshes;

    AnimationData aniData;

    bool isGLTF = false; // gltf or fbx
    bool revertNormals = false;
};
