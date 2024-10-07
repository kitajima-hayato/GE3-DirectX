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
	
	


	//特定のキーが話された瞬間を判定
	bool RereseKey(BYTE keyNumber);
	
	//キーが押されているか確認
	bool PushKey(BYTE keyNumber);
	//キーのトリガー
	bool TriggerKey(BYTE keyNumber);
	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
private:
	ComPtr<IDirectInputDevice8> keyboard;
	ComPtr<IDirectInput8>directInput = nullptr;
	BYTE key[256] = {};
	BYTE keyPre[256] = {};
};

