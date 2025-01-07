#pragma once
#include "Object3D.h"
#include "Model.h"
#include "ModelManager.h"
#include "MyMath.h"
class SkyDome
{
public:
	SkyDome();
	~SkyDome();
	void Initialize(Object3DCommon* object3DCommon, ModelCommon* modelCommon);
	void Update();
	void Draw();
	void Finalize();

private:
	Object3D* skyDome;
	Model* model = nullptr;
};

