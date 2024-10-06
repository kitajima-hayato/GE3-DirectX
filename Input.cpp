#include "Input.h"
#include <cassert>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
	ComPtr<IDirectInput8>directInput = nullptr;
	//DirectInputの初期化
	HRESULT result = DirectInput8Create(
		hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));
	//キーボードデバイスの作成
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	//入力データ形式のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	//排他制御のレベルのセット
	result = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::Update()
{
	HRESULT result;
	//前回のキー入力を保存
	memcpy(keyPre, key, sizeof(key));

	////全キーの入力状態を取得する
	result = keyboard->Acquire();
	result = keyboard->GetDeviceState(sizeof(key), key);

}
bool Input::RereseKey(BYTE keyNumber)
{
	// 前フレームで押されていて、今フレームで押されていない場合
	return (keyPre[keyNumber] != 0) && (key[keyNumber] == 0);
	return false;
}
bool Input::PushKey(BYTE keyNumber)
{
	//指定キーを押していればtrueを返す
	 // 今フレームでキーが押されている場合
	return key[keyNumber] != 0;
}
bool Input::TriggerKey(BYTE keyNumber)
{
	// 前フレームで押されておらず、今フレームで押されている場合
	return (keyPre[keyNumber] == 0) && (key[keyNumber] != 0);
}
////特定のキーが押された瞬間を実装する
//bool Input::IsTrigger(uint8_t key)
//{
//	//現在押されていて、前フレームでは押されていなかった場合
//	return (GetKeyboardState(key)&)
//}
////特定のキーが話された瞬間を判定する
//bool Input::IsRerese(uint8_t key)
//{
//	return false;
//}
