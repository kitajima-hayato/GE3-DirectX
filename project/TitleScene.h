#pragma once
#include "Audio.h"
#include "SpriteCommon.h"
#include "BaseScene.h"
#include "Sprite.h"
#include "SceneManager.h"
#include <Model.h>
#include <Object3D.h>

class TitleScene :public BaseScene
{
public:
	TitleScene();
	~TitleScene();
	// 初期化
	void Initialize(DirectXCommon* dxCommon)override;
	// 更新
	void Update()override;
	// 描画
	void Draw()override;
	// 終了処理
	void Finalize()override;

private:
	// ゲームオブジェクト
	// 3Dオブジェクト
	// シーン遷移

	ModelCommon* modelCommon = nullptr;
	
	// オーディオ
	// サウンドデータ
	SoundData soundData;
	IXAudio2* xaudio2_;

	// スプライト
	Sprite* sprite_ = nullptr;

	Model* model_ = nullptr;
	Object3D* object3D_ = nullptr;

};

