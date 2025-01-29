#pragma once
#include "ParticleManager.h"
using namespace std;
class ParticleEmitter
{
public:
    ParticleEmitter();
    ~ParticleEmitter();
    // 更新
    void Update();
	// パーティクルの発生
    void Emit();

	// Getter,Setter
    void SetTransform(const Transform& transform) { emitter.transform = transform; }
	Transform GetTransform()const { return emitter.transform; }

	void SetParticleName(const string& name) { particleName = name; }

private:
    // パーティクル構造体
    struct Emitter {
        Transform transform;	// エミッタのTransform
        uint32_t count;			// 生成するパーティクルの数
        float frequency;		// 生成する頻度
        float frequencyTime;	// 生成する時間
    };
    
    // カレントタイム
    float currentTime;
    // フレームごとの時間間隔（例: 60FPSの場合）
	const float kDeltaTime = 1.0f / 60.0f;
    // パーティクル発生間隔
    const float emitInterval = 1.0f;
    
	// 描画上限数 / インスタンス数
    const uint32_t kNumMaxInstance = 100;
    // パーティクルのリスト
	std::list<Particle> particles;
	// Emitter
	Emitter emitter;

    // パーティクルネーム
    string particleName;
};
