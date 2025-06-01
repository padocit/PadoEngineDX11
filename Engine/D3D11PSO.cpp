#include "D3D11PSO.h"

void D3D11PSO::operator=(const D3D11PSO &pso)
{
    vertexShader = pso.vertexShader;
    pixelShader = pso.pixelShader;
    hullShader = pso.hullShader;
    domainShader = pso.domainShader;
    geometryShader = pso.geometryShader;
    inputLayout = pso.inputLayout;
    blendState = pso.blendState;
    depthStencilState = pso.depthStencilState;
    rasterizerState = pso.rasterizerState;
    stencilRef = pso.stencilRef;
    for (int i = 0; i < 4; i++)
        blendFactor[i] = pso.blendFactor[i];
    primitiveTopology = pso.primitiveTopology;
}

void D3D11PSO::SetBlendFactor(const float newBlendFactor[4])
{
    memcpy(blendFactor, blendFactor, sizeof(float) * 4);
}