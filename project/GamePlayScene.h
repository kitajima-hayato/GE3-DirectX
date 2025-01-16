#pragma once
#include "Audio.h"
#include "SpriteCommon.h"
#include "BaseScene.h"
class GamePlayScene :public BaseScene
{
public:
	GamePlayScene();
	~GamePlayScene();
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

	// オーディオ
	// サウンドデータ
	SoundData soundData;
	IXAudio2* xaudio2_;
};

