#include "GUIManager.h"

bool GUIManager::Initialize(HWND hWnd, ID3D11Device *device,
                            ID3D11DeviceContext *context,
                            const Resolution &resolution)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.DisplaySize = ImVec2(float(resolution.width), float(resolution.height));
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    if (!ImGui_ImplDX11_Init(device, context))
    {
        return false;
    }

    if (!ImGui_ImplWin32_Init(hWnd))
    {
        return false;
    }

    return true;
}

void GUIManager::Shutdown()
{
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void GUIManager::BeginFrame(const char *title)
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();
    ImGui::Begin(title);

    // Framerate by ImGui
    ImGui::Text("Average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void GUIManager::EndFrame()
{
    ImGui::End();
    ImGui::Render();
}

void GUIManager::Render()
{
    // GUI ������
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
