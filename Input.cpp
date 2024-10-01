#define DIRECTINPUT_VERSION		0x0800	//DirectInputのバージョン指定
#include "Input.h"
#include <dinput.h>
#include <wrl.h>
#include <cassert>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
using namespace Microsoft::WRL;

void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
	ComPtr<IDirectInput8>directInput = nullptr;
	ComPtr<IDirectInputDevice8> keyboard;
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
	////全キーの入力状態を取得する
	//BYTE key[256] = {};
	//keyboard->GetDeviceState(sizeof(key), key);
	//keyboard->Acquire();
	////数字キーの０が押されていたら
	//if (key[DIK_0]) {
	//	OutputDebugStringA("Hit 0\n");//出力ウィンドウにHit 0を出力
	//}
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
