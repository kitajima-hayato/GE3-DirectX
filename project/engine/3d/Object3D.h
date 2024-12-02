#pragma once
#include "Math.h"
#include <string>
#include <d3d12.h>
#include <vector>
#include <wrl.h>
#include "TextureManager.h"
#include "Object3DCommon.h"
#include "Model.h"
#include "Camera.h"
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



public:	// Getter/Setter
	void SetCamera(Camera* camera) { this->camera = camera; }

	void SetModel(Model* model) { this->model = model; }
	void SetModel(const std::string& filePath);
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }

	const Vector3 GetScale()const { return transform.scale; }
	const Vector3 GetRotate()const { return transform.rotate; }
	const Vector3 GetTranslate()const { return transform.translate; }
private: // メンバ関数
	// 座標変換行列リソースの作成
	void CreateTransformationMatrixData();
	// 平行光源リソースの作成
	void CreateDirectionalLightResource();

	
private:// メンバ変数
	// カメラ
	Camera* camera = nullptr;
	// 3Dオブジェクト共通部
	Object3DCommon* object3DCommon = nullptr;
	// モデルデータ
	Model* model = nullptr;
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
};

