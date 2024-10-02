#pragma once
#define DIRECTINPUT_VERSION		0x0800	//DirectInputのバージョン指定
#include <dinput.h>
#include <stdint.h>
#include <Windows.h>
#include <wrl.h>

//LPDIRECTINPUT8 g_pDirectInput = nullptr;			//DirectInputオブジェクト
//LPDIRECTINPUTDEVICE8 g_pKeyboarDevice = nullptr;	//キーボードデバイス
class Input
{
public:
	//初期化
	void Initialize(HINSTANCE hInstance, HWND hwnd);
	//更新
	void Update();
	//特定のキーが押された瞬間を判定
	bool IsTrigger(uint8_t key);
	//特定のキーが話された瞬間を判定
	bool IsRerese(uint8_t key);

	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
private:

	ComPtr<IDirectInputDevice8> keyboard;
};

