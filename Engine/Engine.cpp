#include "Engine.h"
#include "GeometryGenerator.h"

// imgui_impl_win32.cpp에 정의된 메시지 처리 함수에 대한 전방 선언
// Vcpkg를 통해 IMGUI를 사용할 경우 빨간줄로 경고가 뜰 수 있음
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

using namespace std;
using DirectX::SimpleMath::Vector3;

// Singleton
std::unique_ptr<Engine> Engine::instance = nullptr;

bool Engine::Create(std::unique_ptr<Engine> sample)
{
    if (instance != nullptr)
        return false;

    instance = std::move(sample);
    return true;
}

Engine* Engine::Get()
{
    return instance.get();
}

// WndProc Callback my own MsgProc
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return Engine::Get()->MsgProc(hWnd, msg, wParam, lParam);
}

Engine::Engine(int Width, int Height) 
    : resolution({Width, Height}), mainWindow(0)
{
    aspect = float(Width) / float(Height);

    camera.SetAspectRatio(aspect);
    camera_debug.SetAspectRatio(aspect);
}

Engine::~Engine()
{
    instance = nullptr;

    // Cleanup
    guiManager.Shutdown();
    DestroyWindow(mainWindow);

    // DX11 Resources = ComPtr 자동해제
}

int Engine::Run()
{
    // Main Message Loop
    while (/*quit == false*/true)
    {
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
        guiManager.BeginFrame("Scene Control");
        UpdateGUI();
        guiManager.EndFrame(resolution);
        //renderer.guiWidth = guiManager.guiWidth;

        // Update
        Update(guiManager.GetDeltaTime());

        // Render
        Render();
    }

    return 0;
}

bool Engine::Initialize()
{
    if (!InitMainWindow(resolution))
        return false;

    if (!InitDirect3D(resolution))
        return false;

    if (!InitGUI())
        return false;

    if (!InitLevel())
        return false;

    SetForegroundWindow(mainWindow);

    return true;
}

// 여러 Sample에서 공통적으로 사용하기 좋은 장면 설정
bool Engine::InitLevel()
{
    return false;
}

void Engine::Update(float dt)
{
    // 카메라 이동
    camera.UpdateKeyboard(dt, keyPressed);
    // ProcessMouseControl();

    renderer.Update(&level, &camera, dt);
}

void Engine::UpdateGUI()
{
    // TODO: SetGUI
    //ImGui::SliderFloat("Alpha", &renderer.pixelShaderConstData.alpha, 0.0f,
    //                   1.0f);
    // ImGui::...
}

void Engine::Render()
{
    renderer.Render(&level);
    guiManager.Render();

    renderer.SwapBuffer(); // Present()
}

LRESULT Engine::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

bool Engine::InitMainWindow(const Resolution &res)
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

bool Engine::InitDirect3D(const Resolution &res)
{
    if (!renderer.Initialize(res, mainWindow))
        return false;

    return true;
}

bool Engine::InitGUI()
{
    if (!guiManager.Initialize(mainWindow, renderer.GetDevice().Get(),
                               renderer.GetContext().Get(), resolution))
        return false;

    return true;
}
