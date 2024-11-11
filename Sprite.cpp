#include "Sprite.h"
#include "SpriteCommon.h"
void Sprite::Initialize(SpriteCommon* spriteCommon)
{
	this->spriteCommon = spriteCommon;
	CreateVertexData();
	CreateMaterialData();
	CreateTransformationMatrixData();
}

void Sprite::Update()
{
	// 頂点リソースにデータを書き込む(4点分)
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;
	// インデックスリソースにデータを書き込む(6個分)
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };
	vertexData[4].normal = { 0.0f,0.0f,-1.0f };
	vertexData[5].normal = { 0.0f,0.0f,-1.0f };
	// Transform情報を作る
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	// TransformからWorld行列を作る
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	// ViewMatrixを作って単位行列を代入
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	// ProjectionMatrixを作って平行投影行列を代入
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(WinAPI::kClientWidth), float(WinAPI::kClientHeight), 0.0f, 100.0f);

	// WVP行列を計算
	transformationMatrix->WVP = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrix->World = worldMatrix;
}

void Sprite::Draw()
{
	// VertexbufferViewを設定
	spriteCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

	// IndexBufferViewを設定
	spriteCommon->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	// マテリアルCBufferの場所を設定
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// 座標変換行列CBufferの場所を設定
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	// SRVのDescriptorHeapの場所を設定
	//auto srvHandle = spriteCommon->GetDxCommon()->GetSrvDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, spriteCommon->GetDxCommon()->GetSrvDescriptorHeap()->GetGPUDescriptorHandleForHeapStart()
	);
	// 描画
	//commandList->DrawInstanced(6, 1, 0, 0);
	spriteCommon->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::CreateVertexData()
{
	// VertexResourceを作る
	vertexResource = spriteCommon->CreateSpriteVertexResource();

	// IndexResourceを作る
	indexResource = spriteCommon->CreateSpriteIndexResource();
	// VertexBufferViewを作成する(値を設定するだけ)
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// IndexBufferViewを作成する
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
	vertexData = nullptr;	// nullptrを代入しておく
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));	// マップする

	// IndexResourceにデータを書き込むためのアドレスを取得してindexDataに割り当てる
	indexData = nullptr;	// nullptrを代入しておく
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));	// マップする
}


void Sprite::CreateMaterialData() {
	// マテリアルデータを作成する
	materialResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(Material));
	// マテリアルリソースにデータを書き込むためのアドレスを取得してmaterialに割り当てる
	materialData = nullptr;	// nullptrを代入しておく
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));	// マップする

	// マテリアルデータの初期化
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();
}

void Sprite::CreateTransformationMatrixData()
{
	// 変換行列データを作成する
	transformationMatrixResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(Matrix4x4));

	// 変換行列リソースにデータを書き込むためのアドレスを取得してtransformationMatrixに割り当てる
	transformationMatrix = nullptr; // nullptrを代入しておく
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrix)); // マップする

	//単位行列を書き込んでおく
	transformationMatrix->WVP = MakeIdentity4x4();
	transformationMatrix->World = MakeIdentity4x4();
}
	