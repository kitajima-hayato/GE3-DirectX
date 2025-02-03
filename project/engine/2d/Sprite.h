#pragma once
#include "MyMath.h"
#include <d3d12.h>
#include <wrl.h>
#include "MakeMatrix.h"
class SpriteCommon;
class Sprite
{
public:	// メンバ関数
	// 初期化
	void Initialize(std::string textureFilePath);
	void Update();
	void Draw();

public:	// Getter/Setter
	// Position
	const Vector2& GetPosition()const { return position; }
	void SetPosition(const Vector2& position) { this->position = position; }
	// Rotation
	float GetRotation()const { return rotation; }
	void SetRotation(float rotation) { this->rotation = rotation; }
	// color
	const Vector4& GetColor()const { return materialData->color; }
	void SetColor(const Vector4& color) { materialData->color = color; }
	// size
	const Vector2& GetSize()const { return size; }
	void SetSize(const Vector2& size) { this->size = size; }

	// AnchorPoint
	const Vector2& GetAnchorPoint()const { return anchorPoint; }
	void SetAnchorPoint(const Vector2& anchorPoint) { this->anchorPoint = anchorPoint; }

	// FlipX
	bool GetFlipX()const { return isFlipX; }
	void SetFlipX(bool isFlipX) { this->isFlipX = isFlipX; }
	// FlipY
	bool GetFlipY()const { return isFlipY; }
	void SetFlipY(bool isFlipY) { this->isFlipY = isFlipY; }
	// TextureLeftTop
	const Vector2& GetTextureLeftTop()const { return textureLeftTop; }
	void SetTextureLeftTop(const Vector2& textureLeftTop) { this->textureLeftTop = textureLeftTop; }
	// TextureSize
	const Vector2& GetTextureSize()const { return textureSize; }
	void SetTextureSize(const Vector2& textureSize) { this->textureSize = textureSize; }


private:	// メンバ関数
	void CreateVertexResourceData();
	void CreateMaterialResource();
	void CreateTransformationMatrixData();
	// テクスチャサイズをイメージに合わせる
	void AdjustTextureSizee();

	//void DrawSetting();
private:	// メンバ変数
	std::string textureFilePath;
	
	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr <ID3D12Resource> indexResource;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResource = nullptr;
	// バッファリソース内のデータを示すポインタ
	Material* materialData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource>transformationMatrixResource;
	// バッファリソース内のデータを示すポインタ
	TransformationMatrix* transformationMatrix = nullptr;

	// 座標
	Vector2 position = { 0.0f,0.0f };
	float rotation = 0.0f;
	// 色はmaterialDataに格納されているのでそちらを参照
	Vector2 size = { 640.0f,360.0f };

	// テクスチャ番号
	uint32_t textureIndex = 0;

	// テクスチャトランスフォーム
	Transform transform;

	// アンカーポイント
	Vector2 anchorPoint = { 0.0f,0.0f };
	// 左右上下の反転フラグ
	// 左右フリップ
	bool isFlipX = false;
	// 上下フリップ
	bool isFlipY = false;
	// テクスチャ左上座標
	Vector2 textureLeftTop = { 0.0f,0.0f };
	// テクスチャ切り出しサイズ
	Vector2 textureSize = { 100.0f,100.0f };
};

