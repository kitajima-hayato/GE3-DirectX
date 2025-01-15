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

	// SRVマネージャーの初期化
	srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);
	// テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize(dxCommon, srvManager);

	
	

#ifdef _DEBUG
	// ImGuiの初期化
	imGui = new ImGuiManager();
	imGui->Initialize(winAPI, dxCommon);
#endif

	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize(dxCommon);
	// 3Dオブジェクト共通部の初期化
	object3DCommon = new Object3DCommon();
	object3DCommon->Initialize(dxCommon);

	// モデル共通部の初期化
	modelCommon = new ModelCommon();
	modelCommon->Initialize(dxCommon);

	// カメラ
	camera = new Camera();
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera->SetTranslate({ 0.0f, 0.0f, -5.0f });
	object3DCommon->SetDefaultCamera(camera);


#pragma endregion


}

void Framework::Update()
{
#pragma region WindowsAPIのメッセージ処理
	//Windowのメッセージ処理
	if (winAPI->ProcessMessage()) {
		//ゲームループを抜ける
		isEndRequst = true;
		return;
	}
#pragma endregion

	input->Update();


	// ESCキーで終了
	if (input->TriggerKey(DIK_ESCAPE)) {
		isEndRequst = true;
	}

}


void Framework::Finalize()
{

	delete input;
	delete dxCommon;
	winAPI->Finalize();
	delete srvManager;
	TextureManager::GetInstance()->Finalize();
	delete modelCommon;
	delete object3DCommon;
	delete camera;

	ModelManager::GetInstance()->Finalize();
#ifdef _DEBUG
		delete	imGui;
#endif
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

