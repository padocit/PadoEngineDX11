#pragma once

#include <string>
#include <vector>

#include "Vertex.h"

struct MeshData
{
    std::vector<Vertex> vertices;
    vector<SkinnedVertex> skinnedVertices;
    std::vector<uint32_t> indices;

    std::string albedoTextureFilename;
    std::string emissiveTextureFilename;
    std::string normalTextureFilename;
    std::string heightTextureFilename;
    std::string aoTextureFilename; // Ambient Occlusion
    std::string metallicTextureFilename;
    std::string roughnessTextureFilename;
    std::string opacityTextureFilename;
};