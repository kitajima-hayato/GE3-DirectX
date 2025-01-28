#include "ParticleEmitter.h"

ParticleEmitter::ParticleEmitter()
    : currentTime(0.0f)
{
    emitter.transform.translate = { 0.0f, 0.0f, 0.0f };
    emitter.count = 2;
    emitter.frequency = 1.0f;
    emitter.frequencyTime = 0.0f;
}

ParticleEmitter::~ParticleEmitter()
{
   
}


void ParticleEmitter::Update()
{
    // 時刻を進める
    currentTime += kDeltaTime;

    // 発生頻度より大きいなら発生
    if (currentTime >= emitter.frequency)
    {
		// パーティクルを発生させる
        ParticleManager::GetInstance()->Emit("Particle", emitter.transform.translate, emitter.count);
        // カレントタイムをリセット
        currentTime = 0.0f;

    }
}

void ParticleEmitter::Emit()
{
    // パーティクルグループを作成
	ParticleManager::GetInstance()->CreateParticleGroup("Particle", "resources/checkerBoard.png");
    // パーティクルを発生させる
    ParticleManager::GetInstance()->Emit("Particle", emitter.transform.translate, emitter.count);
}
