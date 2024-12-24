#include "ParticleEmitter.h"
#ifdef DEBUG
#include <imgui.h>
#endif // DEBUG

ParticleEmitter::~ParticleEmitter()
{
}

ParticleEmitter::ParticleEmitter(const std::string& name, const Vector3& transform, uint32_t count)
	: emitter({ name, transform, count, 1.0f, 0.0f })
{
}

void ParticleEmitter::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, Camera* camera)
{
	assert(dxCommon); assert(srvManager); assert(camera);
	this->dxCommon = dxCommon;
	this->srvManager = srvManager;
	this->camera = camera;
	
	ParticleManager::GetInstance()->CreateParticleGroup("test", "resources/circle.png");

}

void ParticleEmitter::Update()
{
	emitter.frequencyTime += deltaTime;
	if (emitter.frequency <= emitter.frequencyTime) {
		// パーティクルを生成してグループに追加
		Emit();
		emitter.frequencyTime -= emitter.frequency;
	}
	ShowImGui();
}

void ParticleEmitter::Emit()
{
	ParticleManager::GetInstance()->Emit(emitter.name, emitter.transform, emitter.count);
}

void ParticleEmitter::ShowImGui()
{
#ifdef DEBUG
	ImGui::Begin("Particle");
	if (ImGui::Button("Add Particle"))
	{
		Emit();
	}
	ImGui::End();
#endif // DEBUG
}
