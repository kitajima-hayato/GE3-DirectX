#include "Framework.h"

void Framework::Initialize()
{
	//WindowsAPIの初期化
	WinAPI::GetInstance()->Initialize();
	// dxCommonの初期化
	dxCommon = make_unique<DirectXCommon>();
	dxCommon->Initialize(WinAPI::GetInstance());

	// 入力処理のクラスポインタ
	
	Input::GetInstance()->Initialize(WinAPI::GetInstance());

	// SRVマネージャーの初期化
	srvManager = make_unique<SrvManager>();
	srvManager->Initialize(dxCommon.get());
	// テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get());


#ifdef _DEBUG
	// ImGuiの初期化
	imGui = make_unique<ImGuiManager>();
	imGui->Initialize(WinAPI::GetInstance(), dxCommon.get());
#endif

	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize(dxCommon.get());
	// 3Dオブジェクト共通部の初期化
	Object3DCommon::GetInstance()->Initialize(dxCommon.get());


	// モデル共通部の初期化
	modelCommon = make_unique<ModelCommon>();
	modelCommon->Initialize(dxCommon.get());

	// カメラ
	camera = make_unique<Camera>();
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera->SetTranslate({ 0.0f, 0.0f, -5.0f });
	Object3DCommon::GetInstance()->SetDefaultCamera(camera.get());
	// パーティクル
	ParticleManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get(), camera.get());


#pragma endregion
}

void Framework::Update()
{
#pragma region WindowsAPIのメッセージ処理
	//Windowのメッセージ処理
	if (WinAPI::GetInstance()->ProcessMessage()) {
		//ゲームループを抜ける
		isEndRequst = true;
		return;
	}
#pragma endregion
	SceneManager::GetInstance()->Update(dxCommon.get());

	Input::GetInstance()->Update();

	ParticleManager::GetInstance()->Update();

	// ImGuiの更新
#ifdef _DEBUG
#endif

	// ESCキーで終了
	if (Input::GetInstance()->TriggerKey(DIK_ESCAPE))
	{
		isEndRequst = true;
	}

}


void Framework::Finalize()
{
	// パーティクルの終了処理 / newとは逆順で

	// ImGuiの終了処理
#ifdef _DEBUG
	imGui->Finalize();
#endif
	WinAPI::GetInstance()->Finalize();

	SceneManager::GetInstance()->Finalize();
	SceneManager::Deletenstance();
	Input::GetInstance()->DeleteInstance();
	TextureManager::GetInstance()->DeleteInstance();
	ModelManager::GetInstance()->Finalize();
	ParticleManager::GetInstance()->DeleteInstance();
	Object3DCommon::GetInstance()->Deletenstance();
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

