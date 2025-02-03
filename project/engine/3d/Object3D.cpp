#include "Object3D.h"
#include "Object3DCommon.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include "MakeMatrix.h"
#include "ModelManager.h"
using namespace std;
void Object3D::Initialize()
{
	this->camera = Object3DCommon::GetInstance()->GetDefaultCamera();

	CreateTransformationMatrixData();
	CreateDirectionalLightResource();

	// Transformの初期化
	transform={ { 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f  } };
	}

void Object3D::Update()
{
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 worldViewProjection;
	if (camera) {
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
		worldViewProjection = Multiply(worldMatrix, viewProjectionMatrix);
	}
else {
		worldViewProjection = worldMatrix;
	}
	wvpData->WVP = worldViewProjection;
	wvpData->World = worldMatrix;
}

void Object3D::Draw()
{// 座標変換行列をセット
	Object3DCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	// 平行光源データをセット
	Object3DCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
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
	wvpResource = Object3DCommon::GetInstance()->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	// 変換行列リソ
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 変換行列データの初期化
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();

}

void Object3D::CreateDirectionalLightResource()
{
	// 平行光源リソースを作成
	directionalLightResource = Object3DCommon::GetInstance()->GetDxCommon()->CreateBufferResource(sizeof(DirectionalLight));
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

