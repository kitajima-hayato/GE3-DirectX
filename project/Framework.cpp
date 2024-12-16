#include "Framework.h"

void Framework::Initialize()
{
	//WindowsAPIの初期化
	winAPI = new WinAPI();
	winAPI->Initialize();
	// dxCommonの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winAPI);

	// 入力処理のクラスポインタ
	input = new Input();
	input->Initialize(winAPI);

	// オーディオの初期化
	// audio = new Audio();
	// audio->Initialize();
}

void Framework::Update()
{
#pragma region WindowsAPIのメッセージ処理
	//Windowのメッセージ処理
	if (winAPI->ProcessMessage()) {
		//ゲームループを抜ける
		return;
	}
#pragma endregion

	input->Update();

}

void Framework::Finalize()
{
	// delete audio;
	delete input;
	delete dxCommon;
	winAPI->Finalize();
	delete winAPI;
}

void Framework::Run()
{
	// ゲームの初期化
	Initialize();
	// ゲームのメインループ
	while (true) {
		// ゲームの更新
		Update();
		if (IsEndRequst()) {
			break;
		}
		// ゲームの描画
		Draw();
	}
	// ゲームの終了処理
	Finalize();
}

