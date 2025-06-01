#pragma once

#include <directxtk/SimpleMath.h>

struct Vertex
{
    DirectX::SimpleMath::Vector3 position;
    DirectX::SimpleMath::Vector3 normalModel;
    DirectX::SimpleMath::Vector2 texcoord;
    DirectX::SimpleMath::Vector3 tangentModel;
    //DirectX::SimpleMath::Vector3 biTangentModel;  // ���̴����� ���
};

struct SkinnedVertex
{
    Vector3 position;
    Vector3 normalModel;
    Vector2 texcoord;
    Vector3 tangentModel;

    float blendWeights[8] = {0.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 0.0f, 0.0f};  // BLENDWEIGHT0 and 1
    uint8_t boneIndices[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // BLENDINDICES0 and 1

    // boneWeights�� �ִ� 8����� ���� (Luna ���翡���� 4��)
    // bone�� ���� 256�� ���϶�� ���� uint8_t
};
