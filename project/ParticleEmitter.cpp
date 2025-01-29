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
        Emit();
        // カレントタイムをリセット
        currentTime = 0.0f;
    }
}

void ParticleEmitter::Emit()
{
    // パーティクルを発生させる
    ParticleManager::GetInstance()->Emit(particleName, emitter.transform.translate, emitter.count);
}
