#include "Input.h"

void Input::Initialize()
{
	//DirectInputの初期化
	
	//HRESULT result=directInput->CreateDevice(GUID_SysKeyboard,&keyboard,NULL)

}

void Input::Update()
{

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
