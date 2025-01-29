#pragma once
#include "ModelCommon.h"
#include "Math.h"
#include "TextureManager.h"
#include <string>
#include <vector>
#include <fstream>
#include <cassert>
#include <wrl.h>
#include <d3d12.h>
#include "MakeMatrix.h"
#include "MyMath.h"
using namespace std;
class Model
{
public: // メンバ関数
	// 初期化
	void Initialize(ModelCommon* modelCommon,const string& directorypath,const string&filename);
	// 更新
	void Update();
	// 描画
	void Draw();


	// objファイルの読み込み
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
public: // Getter/Setter
	// モデルデータの取得
	ModelData GetModelData()const { return modelData; }
	// バッファリソースの取得
	Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexResource()const { return vertexResource; }
	// バッファリソースの使い道を補足するバッファービューの取得
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView()const { return vertexBufferView; }
	// マテリアルリソースの取得
	Microsoft::WRL::ComPtr<ID3D12Resource> GetMaterialResource()const { return materialResource; }
	// マテリアルデータの取得
	Material* GetMaterialData()const { return materialData; }
private: // メンバ関数
	// モデルデータの読み込み
	MaterialData LoadMaterialTempLateFile(const std::string& directoryPath, const std::string& filename);
	// 頂点リソースデータの作成
	void CreateVertexResourceData();
	// マテリアルリソースの作成
	void CreateMaterialResource();

private:
	// メンバ変数
	ModelCommon* modelCommon = nullptr;
	// モデルデータ
	ModelData modelData;
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	// バッファリソースの使い道を補足するバッファービュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	// マテリアルデータ
	Material* materialData = nullptr;
};

