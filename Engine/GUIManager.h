#pragma once

#include "D3D11Utils.h"
#include "Common.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

class GUIManager
{
public:
    bool Initialize(HWND hWnd, ID3D11Device *device,
                    ID3D11DeviceContext *context, const Resolution& resolution);

    void Shutdown();

    void BeginFrame(const char *title, const Resolution &resolution);
    void EndFrame(const Resolution &resolution);
    void Render(); // DX11 RenderDrawData

    inline float GetDeltaTime()
    {
        return ImGui::GetIO().DeltaTime;
    }

public:
    int guiWidth = 0;
};
