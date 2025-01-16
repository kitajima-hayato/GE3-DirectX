#pragma once
#include "Audio.h"
#include "SpriteCommon.h"

class TitleScene
{
public:
	TitleScene();
	~TitleScene();
	// 初期化
	void Initialize(DirectXCommon* dxCommon);
	// 更新
	void Update();
	// 描画
	void Draw();
	// 終了処理
	void Finalize();

private:
	// ゲームオブジェクト
	// 3Dオブジェクト
	// シーン遷移

	// オーディオ
	// サウンドデータ
	SoundData soundData;
	IXAudio2* xaudio2_;
};

