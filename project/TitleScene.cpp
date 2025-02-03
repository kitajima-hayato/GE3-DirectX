#include "TitleScene.h"
#include "Input.h"
TitleScene::TitleScene()
{
	
}

TitleScene::~TitleScene()
{
}

void TitleScene::Initialize(DirectXCommon* dxCommon)
{
	// オーディオの初期化
	Audio::GetInstance()->Initialize();
	soundData = Audio::GetInstance()->LoadWave("resources/fanfare.wav");
	xaudio2_ = Audio::GetInstance()->GetXAudio2();
	// Audio::GetInstance()->SoundPlayWave(xaudio2_, soundData);

	// スプライトの初期化
	SpriteCommon::GetInstance()->Initialize(dxCommon);

	/*sprite_ = make_unique<Sprite>();
	sprite_->Initialize("resources/monsterball.png");
	sprite_->SetPosition({ 0.0f,0.0f });
	sprite_->SetRotation(0.0f);*/
	// パーティクルグループを作成
	//ParticleManager::GetInstance()->CreateParticleGroup("Particle", "resources/checkerBoard.png");

	// パーティクルエミッターの初期化
	/*particleEmitter = make_unique<ParticleEmitter>();
	particleEmitter->SetTransform({ {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{1.0f,1.0f,1.0f} });
	particleEmitter->SetParticleName("Particle");*/

	imGui = make_unique<ImGuiManager>();
	imGui->Initialize(WinAPI::GetInstance(), dxCommon);



	// モデル
	object3D = make_unique<Object3D>();
	object3D->Initialize();
	ModelManager::GetInstance()->LoadModel("plane.obj");
	object3D->SetModel("plane.obj");
	object3D->SetTranslate({ 0.0f,0.0f,0.0f });
	object3D->SetRotate({ 0.0f,0.0f,0.0f });
	object3D->SetScale({ 0.01f,0.01f,0.01f });

}

void TitleScene::Update()
{
	//sprite_->Update();
	//particleEmitter->Update();
	object3D->Update();

	ImGui::Begin("plane");
	ImGui::Text("This is a plane.");
	// ENTERキーが押されたら
    Vector3 translate = object3D->GetTranslate();
    ImGui::SliderFloat3("Translate", &translate.x, -10.0f, 10.0f);
    object3D->SetTranslate(translate);


	if(Input::GetInstance()->TriggerKey(DIK_RETURN))
	{
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}
}

void TitleScene::Draw()
{
	//Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->DrawSettingCommon();

	//sprite_->Draw();
	object3D->Draw();

	// パーティクルの描画
	ParticleManager::GetInstance()->Draw();
}

void TitleScene::Finalize()
{

	// オーディオの終了処理
	Audio::GetInstance()->SoundUnload(&soundData);

	// スプライトの終了処理
	SpriteCommon::GetInstance()->Deletenstance();
}
