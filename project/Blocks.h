#pragma once
#include "MyMath.h"
#include "Object3D.h"
#include "Model.h"
#include "ModelManager.h"

class Blocks
{

public:
	Blocks();
	~Blocks();
	void Initialize(Object3DCommon* object3DCommon, ModelCommon* modelCommon);
	void Update();
	void Draw();
	void Finalize();

	void PoPBlock(Vector3 position);

	void PoPNedle(Vector3 position);
	//void Hindrance();

	Vector3 GetTranslate() { return block->GetTranslate(); }
	Vector3 GetScale() { return block->GetScale(); }
	//void GetRotate() { block->GetRotate(); }
	Vector3 GetPreviousTranslate(){return previousPosition;}
	//void PoPBlock(Vector3 position, Vector3 scale, Vector3 rotation);

private:
	struct BlockInfo {
		Vector3 position;
		Vector3 scale;
		Vector3 rotation;
	};

private:
	Object3D* block;
	BlockInfo blockInfo;
	Model* model = nullptr;

	Object3D* nedle;
	BlockInfo nedleInfo;
	Model* nedleModel = nullptr;
private:
	Vector3 previousPosition;
};

