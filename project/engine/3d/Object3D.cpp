#include "Object3D.h"
#include "Object3DCommon.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include "MakeMatrix.h"
#include "ModelManager.h"
using namespace std;
void Object3D::Initialize(Object3DCommon* obj3dCommon)
{
	this->object3DCommon = obj3dCommon;
	

	CreateTransformationMatrixData();
	CreateDirectionalLightResource();

	// Transformの初期化
	transform={ { 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f  } };
	cameraTransform={ { 1.0f, 1.0f, 1.0f },{ 0.3f, 0.0f, 0.0f }, { 0.0f, 4.0f, -10.0f } };
}

void Object3D::Update()
{
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinAPI::kClientWidth) / float(WinAPI::kClientHeight), 0.1f, 100.0f);
	Matrix4x4 worldViewProjection = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	wvpData->WVP = worldViewProjection;
	wvpData->World = worldMatrix;
}

void Object3D::Draw()
{// 座標変換行列をセット
	object3DCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	// 平行光源データをセット
	object3DCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	// 3Dモデルが割り当てられていれば描画する
	if (model) {
		model->Draw();
	}
}

void Object3D::SetModel(const std::string& filePath)
{
	// モデルを検索してセットする
	model = ModelManager::GetInstance()->FindModel(filePath);
}





void Object3D::CreateTransformationMatrixData()
{
	// 変換行列リソースを作成
	wvpResource = object3DCommon->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	// 変換行列リソ
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 変換行列データの初期化
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();

}

void Object3D::CreateDirectionalLightResource()
{
	// 平行光源リソースを作成
	directionalLightResource = object3DCommon->GetDxCommon()->CreateBufferResource(sizeof(DirectionalLight));
	// 平行光源リソースにデータを書き込むためのアドレスを取得してdirectionalLightDataに割り当てる
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// 平行光源データの初期化 / デフォルト値
	// ライトの色
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	// ライトの向き
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	// 輝度
	directionalLightData->intensity = 1.0f;
}

