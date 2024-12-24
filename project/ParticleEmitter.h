#pragma once
#include "ParticleManager.h"

class ParticleEmitter
{
public:
	// 既定のコンストラクタ
	
	~ParticleEmitter();
	ParticleEmitter(const std::string& name, const Vector3& transform, uint32_t count);
	// 初期化
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, Camera* camera);
	void Update();
	// パーティクルの生成(Emit)
	void Emit();
	void ShowImGui();
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

