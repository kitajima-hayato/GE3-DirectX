#include "Blocks.h"

Blocks::Blocks():previousPosition({ 0.0f, 0.0f, 0.0f })
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
	

	nedle = new Object3D();
	nedle->Initialize(object3DCommon);

	nedleModel = new Model();
	nedleModel->Initialize(modelCommon, "resources", "Nedle.obj");
	ModelManager::GetInstance()->LoadModel("Nedle.obj");
	nedle->SetModel("Nedle.obj");


	/*block->SetScale(blockInfo.scale);
	block->SetRotate(blockInfo.rotation);*/
}

void Blocks::Update()
{
	previousPosition = blockInfo.position;
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

void Blocks::PoPNedle(Vector3 position)
{
	nedleInfo.position = { position };
	nedleInfo.scale = { 0.3f,0.3f,1.0f };
	nedle->SetTranslate(nedleInfo.position);
	nedle->SetScale(nedleInfo.scale);

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

