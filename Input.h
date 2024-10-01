#pragma once
#include <stdint.h>
#include <Windows.h>
//LPDIRECTINPUT8 g_pDirectInput = nullptr;			//DirectInputオブジェクト
//LPDIRECTINPUTDEVICE8 g_pKeyboarDevice = nullptr;	//キーボードデバイス
class Input
{
private:

	//using Microsoft::WRL::ComPtr;
	
public:
	//初期化
	void Initialize(HINSTANCE hInstance, HWND hwnd);
	//更新
	void Update();
	//特定のキーが押された瞬間を判定
	bool IsTrigger(uint8_t key);
	//特定のキーが話された瞬間を判定
	bool IsRerese(uint8_t key);
};

