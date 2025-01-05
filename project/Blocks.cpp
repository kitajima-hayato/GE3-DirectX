#include "Blocks.h"

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

void Blocks::PoPBlock(Vector3 position){
	blockInfo.position = { position };
	blockInfo.scale = { 0.3f,0.3f,1.0f };
	block->SetTranslate(blockInfo.position);
	block->SetScale(blockInfo.scale);
}

//void Blocks::Hindrance()
//{
//	int random = GetRandom(0, 3);
//	if (random == 0) {
//		
//	}
//	else if (random == 1) {
//		blockInfo.position = { 6.5f,-2.0f,10.0f };
//		blockInfo.scale = { 0.3f,0.3f,1.0f };
//		block->SetTranslate(blockInfo.position);
//		block->SetScale(blockInfo.scale);
//	}
//	
//	
//	
//}

