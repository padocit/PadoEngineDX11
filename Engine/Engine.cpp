#include "Engine.h"
#include "GeometryGenerator.h"

// imgui_impl_win32.cpp�� ���ǵ� �޽��� ó�� �Լ��� ���� ���� ����
// Vcpkg�� ���� IMGUI�� ����� ��� �����ٷ� ��� �� �� ����
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

Engine *Engine::Get()
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

    // DX11 Resources = ComPtr �ڵ�����
}

int Engine::Run()
{
    // Main Message Loop
    while (/*quit == false*/ true)
    {
        // Handle Input
        MSG msg = {0};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (WM_QUIT == msg.message)
            {
                quit = true;
                break;
            }
        }

        // GUI
        guiManager.BeginFrame("Scene Control", resolution);
        UpdateGUI();
        guiManager.EndFrame(resolution);
        renderer.guiWidth = guiManager.guiWidth;
        camera.SetAspectRatio(renderer.GetAspectRatio());

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

// ���� Sample���� ���������� ����ϱ� ���� ��� ����
bool Engine::InitLevel()
{
    return false;
}

void Engine::Update(float dt)
{
    // ī�޶� �̵�
    camera.UpdateKeyboard(dt, keyPressed);
    // ProcessMouseControl();

    renderer.Update(&level, &camera, dt);
}

void Engine::UpdateGUI()
{
    // �������� ���� GUI
    ImGui::Checkbox("FPV (F key)", &camera.useFirstPersonView);
}

void Engine::Render()
{
    renderer.Render(&level);
    guiManager.Render();

    renderer.SwapBuffer(); // Present()
}

void Engine::OnMouseMove(int mouseX, int mouseY)
{

    mouseX = mouseX;
    mouseY = mouseY;

    // ���콺 Ŀ���� ��ġ�� NDC�� ��ȯ
    // ���콺 Ŀ���� ���� ��� (0, 0), ���� �ϴ�(width-1, height-1)
    // NDC�� ���� �ϴ��� (-1, -1), ���� ���(1, 1)
    mouseNdcX = mouseX * 2.0f / resolution.width - 1.0f;
    mouseNdcY = -mouseY * 2.0f / resolution.height + 1.0f;

    // Ŀ���� ȭ�� ������ ������ ��� ���� ����
    // ���ӿ����� Ŭ������ ���� ���� �ֽ��ϴ�.
    mouseNdcX = std::clamp(mouseNdcX, -1.0f, 1.0f);
    mouseNdcY = std::clamp(mouseNdcY, -1.0f, 1.0f);

    // ī�޶� ���� ȸ��
    camera.UpdateMouse(mouseNdcX, mouseNdcY);
}

void Engine::OnMouseClick(int mouseX, int mouseY)
{
    mouseX = mouseX;
    mouseY = mouseY;

    mouseNdcX = mouseX * 2.0f / resolution.width - 1.0f;
    mouseNdcY = -mouseY * 2.0f / resolution.height + 1.0f;
}

LRESULT Engine::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        // Reset Swapchain
        if (renderer.GetSwapChain())
        {
            resolution.width = int(LOWORD(lParam));
            resolution.height = int(HIWORD(lParam));
            renderer.guiWidth = 0;

            // ������ Minimize ��� screenWidth/Height == 0
            if (resolution.width && resolution.height)
            {

                cout << "Resize SwapChain to " << resolution.width << " "
                     << resolution.height << endl;

                renderer.SetResolution(resolution);
                renderer.SetScreenSize((UINT)LOWORD(lParam),
                                       (UINT)HIWORD(lParam));
                camera.SetAspectRatio(renderer.GetAspectRatio());
                // m_postProcess.Initialize(
                //     m_device, m_context, {m_postEffectsSRV, m_prevSRV},
                //     {m_backBufferRTV}, m_screenWidth, m_screenHeight, 4);
            }
        }
        break;
    case WM_SYSCOMMAND:
        break;
    case WM_MOUSEMOVE:
        OnMouseMove(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_KEYDOWN:
        keyPressed[wParam] = true;
        if (wParam == VK_ESCAPE) // ESCŰ ����
        {
            DestroyWindow(hwnd);
        }
        break;
    case WM_KEYUP:
        if (wParam == 'F') // FŰ 1��Ī ����
        {
            camera.useFirstPersonView = !camera.useFirstPersonView;
        }
        if (wParam == 'C') // CŰ ȭ�� ĸ��
        {
            renderer.CaptureScreen();
        }
        //if (wParam == 'P') // PŰ �ִϸ��̼� �Ͻ�����
        //{
        //    pauseAnimation = !pauseAnimation;
        //}
        if (wParam == 'Z') // ī�޶� ���� ���
        {
            camera.PrintView();
        }

        keyPressed[wParam] = false;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
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

    if (!RegisterClassEx(&wc))
    {
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

    if (!mainWindow)
    {
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
