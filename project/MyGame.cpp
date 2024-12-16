#include "MyGame.h"

void MyGame::Initialize()
{
	Framework::Initialize();

#pragma region 最初のシーンの初期化
	// 3Dオブジェクトの初期化
	object3D = new Object3D();
	object3D->Initialize(object3DCommon);

	model = new Model();
	model->Initialize(modelCommon, "resources", "axis.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");
	object3D->SetModel("axis.obj");

	// モデルをもう一つ読み込む
	model2 = new Model();
	model2->Initialize(modelCommon, "resources", "plane.obj");
	
	ModelManager::GetInstance()->LoadModel("plane.obj");
	
	object3D2 = new Object3D();
	object3D2->Initialize(object3DCommon);
	object3D2->SetModel("plane.obj");

	std::vector<Sprite*> sprites;
	sprites.clear();
	Sprite* sprite = new Sprite();
	sprite->Initialize(spriteCommon, "resources/uvChecker.png");
	Vector2 pos = { 100.0f, 100.0f };
	sprite->SetPosition(pos);
	sprites.push_back(sprite);

#pragma endregion

}

void MyGame::Update()
{
	Framework::Update();
#pragma region ゲームの更新
	// アクターの更新


#pragma endregion

#ifdef _DEBUG // デバッグ時のみ有効ImGuiの処理
	// ImGuiの処理
	imGui->Begin();
#endif

	// ゲームの処理
	object3D->Update();
	object3D2->Update();
	if (input->TriggerKey(DIK_1)) {
		OutputDebugStringA("Hit_1\n");
	}

	for (Sprite* sprite : sprites) {
		sprite->Update();
	}

#ifdef _DEBUG
	ImGui::SetWindowSize(ImVec2(500.0f, 200.0f));
	// ImGuiのデモ
	ImGui::ShowDemoWindow();
	ImGui::Text("Hello, world %d", 123);
	if (ImGui::Button("Save")) {
		OutputDebugStringA("Save\n");
	}


	Vector3 cameraPos = camera->GetTranslate();
	Vector3 cameraRotate = camera->GetRotate();
	ImGui::DragFloat3("cameraPosition", &cameraPos.x, 0.1f);
	ImGui::DragFloat3("cameraRotate", &cameraRotate.x, 0.1f);
	camera->SetTranslate(cameraPos);
	camera->SetRotate(cameraRotate);
	camera->Update();
	imGui->End();
	

#endif
}

void MyGame::Draw()
{
	// DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
	dxCommon->PreDraw();
	srvManager->PreDraw();
	// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	object3DCommon->DrawSettingCommon();
	object3D->Draw();
	object3D2->Draw();
	//Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	spriteCommon->DrawSettingCommon();

	for (Sprite* sprite : sprites) {
		sprite->Draw();
	}

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

#pragma endregion

	//解放
	for (Sprite* sprite : sprites) {
		delete sprite;
	}
	delete model2;
	delete object3D2;
	delete model;
	delete object3D;
	Framework::Finalize();
}



