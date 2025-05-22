#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>
#include <wrl/client.h> // ComPtr

// Create Shaders
// Create Buffers

inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        // ������� �� ���⿡ breakpoint ����
        throw std::exception();
    }
}

void CheckResult(HRESULT hr, ID3DBlob *errorBlob);