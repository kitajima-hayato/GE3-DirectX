#include "Blocks.h"
#include <ImGuiManager.h>

Blocks::Blocks()
{
}

Blocks::~Blocks()
{
	delete block;
	delete model;
}

void Blocks::Initialize(Object3DCommon* object3DCommon, ModelCommon* modelCommon)
{
	block = new Object3D();
	block->Initialize(object3DCommon);

	model = new Model();
	model->Initialize(modelCommon, "resources", "cube.obj");
	ModelManager::GetInstance()->LoadModel("cube.obj");
	block->SetModel("cube.obj");
	
	/*block->SetScale(blockInfo.scale);
	block->SetRotate(blockInfo.rotation);*/
}

void Blocks::Update()
{
	block->Update();
	
	Vector3 blockPos = block->GetTranslate();
	/*Vector3 blockScale = block->GetScale();
	Vector3 blockRotate = block->GetRotate();
	ImGui::DragFloat3("blockPosition", &blockPos.x, 0.1f);
	ImGui::DragFloat3("blockScale", &blockScale.x, 0.1f);
	ImGui::DragFloat3("blockRotate", &blockRotate.x, 0.1f);
	block->SetTranslate(blockPos);
	block->SetScale(blockScale);*/
	//block->SetRotate(blockRotate);

	blockPos.x -= 0.05f;
	block->SetTranslate(blockPos);
}

void Blocks::Draw()
{
	block->Draw();
}

void Blocks::Finalize()
{
	
}

void Blocks::PoPBlock(){
	blockInfo.position = { 6.5f,-3.0f,10.0f };
	blockInfo.scale = { 0.3f,0.3f,1.0f };
	block->SetTranslate(blockInfo.position);
	block->SetScale(blockInfo.scale);
}
