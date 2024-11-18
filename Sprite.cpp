#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath)
{
	this->spriteCommon = spriteCommon;
	CreateVertexResourceData();
	CreateMaterialResource();
	CreateTransformationMatrixData();
	// 単位行列を書き込んでおく
	TextureManager::GetInstance()->LoadTexture(textureFilePath);
	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
	transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	AdjustTextureSizee();
}

void Sprite::Update()
{
#pragma region 頂点データの設定


	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;

	// 左右反転
	if (isFlipX) {
		left = -left;
		right = -right;
	}

	// 上下反転
	if (isFlipY) {
		top = -top;
		bottom = -bottom;
	}

	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetadata(textureIndex);
	
	float texleft = textureLeftTop.x / metadata.width;
	float texright = (textureLeftTop.x + textureSize.x) / metadata.width;
	float textop = textureLeftTop.y / metadata.height;
	float texbottom = (textureLeftTop.y + textureSize.y) / metadata.height;

	// 1枚目の三角形
	// 左上
	vertexData[0].position = { left, bottom, 0.0f, 1.0f };
	vertexData[0].texcoord = { texleft, texbottom };
	// 左下
	vertexData[1].position = { left, top, 0.0f, 1.0f };
	vertexData[1].texcoord = { texleft, textop };
	//	右下
	vertexData[2].position = { right, bottom, 0.0f, 1.0f };
	vertexData[2].texcoord = { texright, texbottom };

	// 2枚目の三角形
	// 左上
	vertexData[3].position = vertexData[1].position;
	vertexData[3].texcoord = vertexData[1].texcoord;
	// 右上
	vertexData[4].position = { right, top, 0.0f, 1.0f };
	vertexData[4].texcoord = { texright, textop };
	// 右下
	vertexData[5].position = vertexData[2].position;
	vertexData[5].texcoord = vertexData[2].texcoord;
#pragma endregion
	// インデックスリソースにデータを書き込む(6個分)
	for (int i = 0; i < 6; ++i) {
		vertexData[i].normal = { 0.0f, 0.0f, -1.0f };
	}


	// Transform情報を作る

	transform.translate = { position.x, position.y, 0.0f };
	transform.rotate = { 0.0f, 0.0f, rotation };
	transform.scale = { size.x, size.y, 1.0f };
	// TransformからWorld行列を作る
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	// ViewMatrixを作って単位行列を代入
	Matrix4x4 viewMatrix = MakeIdentity4x4();

	// ProjectionMatrixを作って平行投影行列を代入
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(WinAPI::kClientWidth), float(WinAPI::kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

	// WVP行列を計算
	transformationMatrix->WVP = worldProjectionMatrix;
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
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex));

	// 描画
	spriteCommon->GetDxCommon()->GetCommandList()->DrawInstanced(6, 1, 0, 0);
	//spriteCommon->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::CreateVertexResourceData()
{

	// VertexResourceを作る
	vertexResource = spriteCommon->CreateSpriteVertexResource();
	// VertexBufferViewを作成する(値を設定するだけ)
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 6個分のデータを書き込むので6倍
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	// 1頂点分のサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// IndexResourceを作る
	indexResource = spriteCommon->CreateSpriteIndexResource();

	// IndexBufferViewを作成する
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	// 6個分のデータを書き込むので6倍
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	// 1頂点分のサイズ
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// IndexResourceにデータを書き込むためのアドレスを取得してindexDataに割り当てる
	indexData = nullptr;	// nullptrを代入しておく
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));	// マップする

	// 頂点リソースにデータを書き込む(4点分)
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;





	// VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
	vertexData = nullptr;	// nullptrを代入しておく
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));	// マップする






}


void Sprite::CreateMaterialResource() {
	// マテリアルデータを作成する
	materialResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(Material));
	// マテリアルリソースにデータを書き込むためのアドレスを取得してmaterialに割り当てる
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));	// マップする

	// マテリアルデータの初期化
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();
}

void Sprite::CreateTransformationMatrixData()
{
	// 変換行列データを作成する
	transformationMatrixResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));

	// 変換行列リソースにデータを書き込むためのアドレスを取得してtransformationMatrixに割り当てる
	transformationMatrix = nullptr; // nullptrを代入しておく
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrix)); // マップする

	//単位行列を書き込んでおく
	transformationMatrix->WVP = MakeIdentity4x4();
	transformationMatrix->World = MakeIdentity4x4();
}

void Sprite::AdjustTextureSizee()
{
	// テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetadata(textureIndex);
	textureSize = { static_cast<float>(metadata.width),static_cast<float>(metadata.height) };
	// 画像サイズをテクスチャサイズに合わせる
	size = textureSize;
}

//void Sprite::DrawSetting()
//{
//	spriteCommon->GetDxCommon()->LoadTexture("resources/uvChecker.png");
//	spriteCommon->DrawSettingCommon();
//	//spriteCommon->GetDxCommon()->CreateTextureResource();
//}
