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

	

}

void TitleScene::Update()
{
	
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



}

void TitleScene::Finalize()
{
	

	// オーディオの終了処理
	Audio::GetInstance()->SoundUnload(&soundData);

	// スプライトの終了処理
	SpriteCommon::GetInstance()->Deletenstance();
}
