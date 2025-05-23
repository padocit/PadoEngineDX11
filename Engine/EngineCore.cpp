#include "EngineCore.h"

// imgui_impl_win32.cpp에 정의된 메시지 처리 함수에 대한 전방 선언
// Vcpkg를 통해 IMGUI를 사용할 경우 빨간줄로 경고가 뜰 수 있음
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

using namespace std;
using DirectX::SimpleMath::Vector3;

// Singleton (Lazy)
std::unique_ptr<EngineCore> EngineCore::instance = nullptr;
EngineCore &EngineCore::Get()
{
    if (instance == nullptr) {
        instance = make_unique<EngineCore>();
    }
    return *instance;
}

// WndProc Callback my own MsgProc
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return EngineCore::Get().MsgProc(hWnd, msg, wParam, lParam);
}

EngineCore::EngineCore() : mainWindow(0)
{
}

EngineCore::~EngineCore()
{
    instance = nullptr;

    // Cleanup
    guiManager.Shutdown();
    DestroyWindow(mainWindow);

    // DX11 Resources = ComPtr 자동해제
}

int EngineCore::Run()
{
    // Main Message Loop
    while (quit == false) {
        // Handle Input
        MSG msg = {0};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (WM_QUIT == msg.message) {
                quit = true;
                break;
            }
        }

        // GUI
        UpdateGUI();

        // Update
        Update();

        // Render
        Render();
    }

    return 0;
}

bool EngineCore::Initialize(int width, int height)
{
    resolution = {width, height};

    if (!InitMainWindow(resolution))
        return false;

    if (!InitDirect3D(resolution))
        return false;

    if (!InitGUI())
        return false;

    SetForegroundWindow(mainWindow);

    // MeshData (vertices, indices) 생성
    MeshData triangleData;
    triangleData.vertices.push_back(
        {Vector3(0.0f, 0.5f, 0.0f), Vector3(1.0f, 0.0f, 0.0f)});
    triangleData.vertices.push_back(
        {Vector3(0.7f, -0.5f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)});
    triangleData.vertices.push_back(
        {Vector3(-0.7f, -0.5f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)});

    triangleData.indices.push_back(0);
    triangleData.indices.push_back(1);
    triangleData.indices.push_back(2);

    // Mesh (vertices, indices buffer) 생성
    renderer.CreateMesh(triangleData, triangle);

    return true;
}

void EngineCore::Update()
{
    renderer.Update(); // cbuffer update
}

void EngineCore::UpdateGUI()
{
    guiManager.BeginFrame("Scene Control");

    // TODO: SetGUI
    ImGui::SliderFloat("Alpha", &renderer.pixelShaderConstData.alpha, 0.0f,
                       1.0f);

    guiManager.EndFrame();
}

void EngineCore::Render()
{
    renderer.Render(triangle);
    guiManager.Render();

    renderer.SwapBuffer(); // Present()
}

LRESULT EngineCore::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        // Reset Swapchain

        break;
    case WM_SYSCOMMAND:
        break;
    case WM_MOUSEMOVE:
        break;
        // TODO: ...
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool EngineCore::InitMainWindow(const Resolution &res)
{
    WNDCLASSEX wc = {sizeof(WNDCLASSEX),
                     CS_CLASSDC,
                     WndProc,
                     0L,
                     0L,
                     GetModuleHandle(NULL),
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     L"HongLabGraphics", // lpszClassName, L-string
                     NULL};

    if (!RegisterClassEx(&wc)) {
        cout << "RegisterClassEx() failed." << endl;
        return false;
    }

    RECT wr = {0, 0, res.width, res.height};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);
    mainWindow =
        CreateWindow(wc.lpszClassName, L"PadoEngine", WS_OVERLAPPEDWINDOW,
                     100,                // Window upper-left X
                     100,                // Window upper-left Y
                     wr.right - wr.left, // Window width
                     wr.bottom - wr.top, // Window height
                     NULL, NULL, wc.hInstance, NULL);

    if (!mainWindow) {
        cout << "CreateWindow() failed." << endl;
        return false;
    }

    ShowWindow(mainWindow, SW_SHOWDEFAULT);
    UpdateWindow(mainWindow);

    return true;
}

bool EngineCore::InitDirect3D(const Resolution &res)
{
    if (!renderer.Initialize(res, mainWindow))
        return false;

    return true;
}

bool EngineCore::InitGUI()
{
    if (!guiManager.Initialize(mainWindow, renderer.GetDevice(),
                               renderer.GetContext(), resolution))
        return false;

    return true;
}
