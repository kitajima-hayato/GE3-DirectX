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
	Audio::GetInstance()->SoundPlayWave(xaudio2_, soundData);

	// スプライトの初期化
	SpriteCommon::GetInstance()->Initialize(dxCommon);

	sprite_ = new Sprite();
	sprite_->Initialize(SpriteCommon::GetInstance(), "resources/monsterball.png");
	sprite_->SetPosition({ 0.0f,0.0f });
	sprite_->SetRotation(0.0f);

}

void TitleScene::Update()
{
	sprite_->Update();

	// ENTERキーが押されたら
	if(Input::GetInstance()->TriggerKey(DIK_RETURN))
	{
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}
}

void TitleScene::Draw()
{
	//Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->DrawSettingCommon();

	sprite_->Draw();
}

void TitleScene::Finalize()
{
	delete sprite_;

	// オーディオの終了処理
	Audio::GetInstance()->SoundUnload(&soundData);

	// スプライトの終了処理
	SpriteCommon::GetInstance()->Deletenstance();
}
