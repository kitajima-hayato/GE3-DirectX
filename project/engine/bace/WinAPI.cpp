#include "WinAPI.h"
#include <cstdint>
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#pragma comment(lib,"winmm.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwndm, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WinAPI::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		// ウィンドウが破棄された
	case WM_DESTROY:
		// OSに対してアプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}
	// 標準メッセージの処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam); 
}

void WinAPI::Finalize()
{
	CloseWindow(hwnd);
	CoUninitialize();
}

void WinAPI::Initialize()
{
	// システムタイマーの分解能を上げる
	timeBeginPeriod(1);
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
#pragma region Windowの生成

	
	//ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;//上の関数を渡している
	//ウィンドウクラス名
	wc.lpszClassName = L"CG2";
	//インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);


	//ウィンドウクラスを登録
	RegisterClass(&wc);

	
	//ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0,0,WinAPI::kClientWidth,WinAPI::kClientHeight };//RECT レクタングル(矩形)
	//クライアント領域をもとに実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウの生成
	hwnd = CreateWindow(
		wc.lpszClassName,        //利用するクラスメイン
		L"MyEngine",                  //タイトルバーの文字
		WS_OVERLAPPEDWINDOW,     //よく見るウィンドウスタイル
		CW_USEDEFAULT,           //表示X座標
		CW_USEDEFAULT,           //表示Y座標 
		wrc.right - wrc.left,    //ウィンドウ横幅
		wrc.bottom - wrc.top,    //ウィンドウ縦幅
		nullptr,                 //親ウィンドウハンドル
		nullptr,                 //メニューハンドル
		wc.hInstance,            //インスタンスハンドル
		nullptr                  //オプション
	);//ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);
#pragma endregion
}

bool WinAPI::ProcessMessage()
{
	MSG msg{};
	//Windowにメッセージが来てたら最優先で処理させる
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (msg.message == WM_QUIT) {
		return true;
	}
	return false;
}

