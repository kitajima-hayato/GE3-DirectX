#include "MyGame.h"

void MyGame::Initialize()
{
	Framework::Initialize();
	// ゲームプレイシーンの初期化
	scene_ = new TitleScene();
	scene_->Initialize(dxCommon);
#pragma region 最初のシーンの初期化
	// 3Dオブジェクトの初期化

#pragma endregion

}

void MyGame::Update()
{
	Framework::Update();

#pragma region ゲームの更新
	// アクターの更新

	// ゲームプレイシーンの更新
	scene_->Update();
	
#pragma endregion

#ifdef _DEBUG // デバッグ時のみ有効ImGuiの処理
	// ImGuiの処理
	imGui->Begin();
#endif

	
#ifdef _DEBUG
	
	ImGui::Text("Hello, world %d", 123);
	if (ImGui::Button("Save")) {
		OutputDebugStringA("Save\n");
	}


	
	imGui->End();
	

#endif
}

void MyGame::Draw()
{
	// DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
	dxCommon->PreDraw();
	srvManager->PreDraw();
	// ゲームプレイシーンの描画
	scene_->Draw();
	// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	object3DCommon->DrawSettingCommon();
	
	

	
#ifdef _DEBUG
	// ImGuiの描画
	imGui->Draw();
#endif
	dxCommon->PostDraw();
}

void MyGame::Finalize()
{
	
#pragma region  解放処理
#ifdef _DEBUG
	// ImGuiの終了処理
	imGui->Finalize();
#endif
	// CloseHandle(fenceEvent);
	TextureManager::GetInstance()->Finalize();
	// モデルマネージャの終了処理
	ModelManager::GetInstance()->Finalize();
	winAPI->Finalize();

	// ゲームプレイシーンの終了処理
	scene_->Finalize();
	delete scene_;

#pragma endregion

	Framework::Finalize();
}
