#include "ModelManager.h"

ModelManager* ModelManager::instance = nullptr;

ModelManager* ModelManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new ModelManager();
	}
	return instance;
}

void ModelManager::Finalize()
{
	delete instance;
	instance = nullptr;
}

void ModelManager::Initialize(DirectXCommon* dxCommon)
{
	modelCommon = new ModelCommon();
	modelCommon->Initialize(dxCommon);
}

void ModelManager::LoadModel(const string& filePath)
{
	// 読み込み済みモデルを検索
	if (models.contains(filePath))
	{
		// 読み込み済みなら早期リターン
		return;
	}
	// モデルの生成とファイル読み込み、初期化
	unique_ptr<Model>model = make_unique<Model>();
	model->Initialize(modelCommon, "resources", filePath);

	// モデルリストに追加
	//models[filePath] = move(model);
	models.insert(make_pair(filePath, move(model)));
}

Model* ModelManager::FindModel(const string& filePath)
{
	// 読み込み済みモデルを検索
	if (models.contains(filePath))
	{
		// 読み込みモデルを戻り値として返す
		return models.at(filePath).get();
	}
	// 読み込み済みでない場合はnullptrを返す
	return nullptr;
}
