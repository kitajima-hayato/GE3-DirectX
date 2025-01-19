#include "GamePlayScene.h"

GamePlayScene::GamePlayScene()
{
}

GamePlayScene::~GamePlayScene()
{
}

void GamePlayScene::Initialize(DirectXCommon* dxCommon)
{
	// オーディオの初期化
	Audio::GetInstance()->Initialize();
	soundData = Audio::GetInstance()->LoadWave("resources/mokugyo.wav");
	xaudio2_ = Audio::GetInstance()->GetXAudio2();
	Audio::GetInstance()->SoundPlayWave(xaudio2_, soundData);

	// スプライトの初期化
	SpriteCommon::GetInstance()->Initialize(dxCommon);

}

void GamePlayScene::Update()
{

}

void GamePlayScene::Draw()
{
	//Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->DrawSettingCommon();
}

void GamePlayScene::Finalize()
{

	// オーディオの終了処理
	Audio::GetInstance()->SoundUnload(&soundData);
	
	// スプライトの終了処理
	SpriteCommon::GetInstance()->Deletenstance();


}
