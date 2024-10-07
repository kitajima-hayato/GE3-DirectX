#pragma once
#include <Windows.h>
class WinAPI
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	//クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

public:
	//初期化
	void Initialize();
	//更新
	void Update();
private:

};

