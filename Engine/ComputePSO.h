#pragma once

#include "D3D11Utils.h"

class ComputePSO
{
public:
    void operator=(const ComputePSO &pso)
    {
        computeShader = pso.computeShader;
    };

public:
    ComPtr<ID3D11ComputeShader> computeShader;
};
