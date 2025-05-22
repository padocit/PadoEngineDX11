#pragma once

#include <Windows.h>
#include <memory>

#include "Common.h"
#include "Renderer.h"

class EngineCore
{
public:
	EngineCore();
	virtual ~EngineCore();

	int Run();

	bool Initialize(int width = 1280, int height = 720);
	void Update();
	void Render();

	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Init
	bool InitMainWindow(const Resolution& res);
	bool InitDirect3D(const Resolution& res);
	bool InitGUI();

	static EngineCore& Get();

private:
    Resolution res;
    Renderer renderer;
	HWND mainWindow;
	bool quit = false;

	// юс╫ц
    Mesh triangle;

	static std::unique_ptr<EngineCore> instance;
};

