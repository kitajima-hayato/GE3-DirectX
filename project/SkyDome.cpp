#include "SkyDome.h"

SkyDome::SkyDome()
{
}

SkyDome::~SkyDome()
{
	delete skyDome;
	delete model;
}

void SkyDome::Initialize(Object3DCommon* object3DCommon, ModelCommon* modelCommon)
{
	skyDome = new Object3D();
	skyDome->Initialize(object3DCommon);

	model = new Model();
	model->Initialize(modelCommon, "resources", "skydome.obj");
	ModelManager::GetInstance()->LoadModel("skydome.obj");
	skyDome->SetModel("skydome.obj");
}

void SkyDome::Update()
{
	Vector3 rotate = skyDome->GetRotate();
	rotate.y -= 0.01f;
	skyDome->Update();
	skyDome->SetRotate(rotate);
}

void SkyDome::Draw()
{
	skyDome->Draw();
}

void SkyDome::Finalize()
{
}
