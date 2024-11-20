#pragma once
#include "Math.h"
#include <string>
#include <d3d12.h>
#include <vector>
#include <wrl.h>
#include "TextureManager.h"
// 3Dオブジェクト
// 3Dオブジェクト共通部前方宣言
class Object3DCommon;
class Object3D
{
	

public:	// メンバ関数
	// 初期化
	void Initialize(Object3DCommon* obj3dCommon);
	// 更新
	void Update();
	// 描画
	void Draw();
	// モデルデータの読み込み
	MaterialData LoadMaterialTempLateFile(const std::string& directoryPath, const std::string& filename);
	// objファイルの読み込み
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

public:	// Getter/Setter

private: // メンバ関数
	// 頂点リソースデータの作成
	void CreateVertexResourceData();
	// マテリアルリソースの作成
	void CreateMaterialResource();
	// 座標変換行列リソースの作成
	void CreateTransformationMatrixData();
	// 平行光源リソースの作成
	void CreateDirectionalLightResource();
private:// メンバ変数
	// 3Dオブジェクト共通部
	Object3DCommon* object3DCommon = nullptr;
	// モデルデータ
	ModelData modelData;
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	// バッファリソースの使い道を補足するバッファービュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	// マテリアルデータ
	Material* materialData = nullptr;
	// バッファリソース / 座標変換行列リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* wvpData = nullptr;
	// バッファリソース / 平行光源リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	// バッファリソース内のデータを指すポインタ
	DirectionalLight* directionalLightData = nullptr;

	// Transform
	Transform transform;
	Transform cameraTransform;
};

