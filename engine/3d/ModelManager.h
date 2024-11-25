#pragma once
#include "ModelCommon.h"
#include "Math.h"
#include "TextureManager.h"
#include <string>
#include <vector>
#include <fstream>
#include "Model.h"
#include <map>
using namespace std;
// Path: ModelManager.h
class ModelManager
{
public: // メンバ関数
	// シングルトンインスタンスを取得
	static ModelManager* GetInstance();
	// シングルトンインスタンスを解放
	void Finalize();

	// 初期化
	void Initialize(DirectXCommon* dxCommon);
	// モデルファイルの読み込み
	void LoadModel(const string& filePath);
public: // Getter/Setter
	// モデルデータの取得
	Model* FindModel(const string& filePath);

private: // メンバ変数シングルトン
	static ModelManager* instance;
	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(ModelManager&) = delete;
	ModelManager& operator=(ModelManager&) = delete;
private: // メンバ変数
	ModelCommon* modelCommon = nullptr;
	// モデルデータのリスト
	map<string, unique_ptr<Model>> models;

};

