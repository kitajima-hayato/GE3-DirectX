#include "ParticleEmitter.h"



void ParticleEmitter::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, Camera* camera)
{
	assert(dxCommon); assert(srvManager); assert(camera);
	this->dxCommon = dxCommon;
	this->srvManager = srvManager;
	this->camera = camera;

	ParticleManager::GetInstance()->CreateParticleGroup("test", "Resources/Textures/circle.png");
}

void ParticleEmitter::Update()
{
	emitter.frequencyTime += deltaTime;
	if (emitter.frequency <= emitter.frequencyTime) {
		// パーティクルを生成してグループに追加
		Emit();
		emitter.frequencyTime -= emitter.frequency;
	}
}

void ParticleEmitter::Emit()
{
	ParticleManager::GetInstance()->Emit(emitter.name, emitter.transform, emitter.count);
}
