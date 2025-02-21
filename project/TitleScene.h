#pragma once
#include "Audio.h"
#include "SpriteCommon.h"
#include "BaseScene.h"
#include "Sprite.h"
#include "SceneManager.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
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
	// スレッド用の関数
	void LoadAudio();
	void LoadSprite();

private:

	// オーディオ
	// サウンドデータ
	SoundData soundData;
	IXAudio2* xaudio2_;

	// スプライト
	std::unique_ptr<Sprite> sprite_;

	// パーティクルエミッター
	std::unique_ptr<ParticleEmitter> particleEmitter;

};

