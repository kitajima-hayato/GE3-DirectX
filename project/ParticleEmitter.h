#pragma once
#include "ParticleManager.h"
class ParticleEmitter
{
public:
	ParticleEmitter();
	~ParticleEmitter();
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, Camera* camera);
	void Update();
	
	void Emit();



private:
	DirectXCommon* dxCommon = nullptr;
	SrvManager* srvManager = nullptr;
	Camera* camera = nullptr;

	struct Emitter {
		std::string name;
		Vector3 transform;
		uint32_t count;
		float frequency;
		float frequencyTime;
	};

	Emitter emitter;

	const float deltaTime = 1.0f / 60.0f;

};

