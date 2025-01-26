#include "TitleScene.h"
#include "Input.h"
#include <ModelManager.h>
TitleScene::TitleScene()
{
	
}

TitleScene::~TitleScene()
{
}

void TitleScene::Initialize(DirectXCommon* dxCommon)
{
	modelCommon = new ModelCommon();
	modelCommon->Initialize(dxCommon);

	// オーディオの初期化
	Audio::GetInstance()->Initialize();
	soundData = Audio::GetInstance()->LoadWave("resources/fanfare.wav");
	xaudio2_ = Audio::GetInstance()->GetXAudio2();
	//Audio::GetInstance()->SoundPlayWave(xaudio2_, soundData);

	// スプライトの初期化
	SpriteCommon::GetInstance()->Initialize(dxCommon);

	object3D_ = new Object3D();
	object3D_->Initialize(Object3DCommon::GetInstance());

	model_ = new Model();
	model_->Initialize(modelCommon,"resources","Sphere.obj");
	ModelManager::GetInstance()->LoadModel("Sphere.obj");
	// 
	object3D_->SetModel("Sphere.obj");

//	ModelManager::GetInstance()->LoadModel("Sphere.obj");

}

void TitleScene::Update()
{
	
	object3D_->Update();

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

	
	object3D_->Draw();
	
}

void TitleScene::Finalize()
{
	delete model_;
	delete object3D_;

	// オーディオの終了処理
	Audio::GetInstance()->SoundUnload(&soundData);

	// スプライトの終了処理
	SpriteCommon::GetInstance()->Deletenstance();
}
