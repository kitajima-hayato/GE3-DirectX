#pragma once
#include <Windows.h>
#include <cstdint>
// シングルトンクラス
class WinAPI
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	//定数
	//クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

	static WinAPI* GetInstance();
	static void DeleteInstance();

public:
	//終了
	void Finalize();
	
	//初期化
	void Initialize();
	
	//メッセージの処理
	bool ProcessMessage();

	//getter
	HWND GetHwnd() const { return hwnd; }
	HINSTANCE GetHInstance()const { return wc.hInstance; }
private:
	static WinAPI* instance;
	WinAPI() = default;
	~WinAPI() = default;
	WinAPI(WinAPI&) = delete;
	WinAPI& operator=(WinAPI&) = delete;

	//ウィンドウハンドル
	HWND hwnd = nullptr;

	//ウィンドウクラスの設定
	WNDCLASS wc{};

};

