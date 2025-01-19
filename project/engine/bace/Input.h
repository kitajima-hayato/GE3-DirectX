#pragma once
#define DIRECTINPUT_VERSION		0x0800	//DirectInputのバージョン指定
#include <dinput.h>
#include <stdint.h>
#include <Windows.h>
#include <wrl.h>
#include "WinAPI.h"

class Input
{
public:
	static Input* GetInstance();
	static void DeleteInstance();
private:
	static Input* instance;
	Input() = default;
	~Input() = default;
	Input(Input&) = delete;
	Input& operator=(Input&) = delete;

public:

	//初期化 
	void Initialize(WinAPI*winAPI);
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

	//WindowsAPI
	WinAPI* winAPI = nullptr;

	
};

