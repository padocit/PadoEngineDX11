#pragma once

#include "D3D11Utils.h"

__declspec(align(256)) struct PixelConstants
{
    float alpha = 1.0f;
    DirectX::SimpleMath::Vector3 dummy;
};