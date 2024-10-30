#pragma once
#include "Math.h"
#include <d3d12.h>
#include <wrl.h>
class SpriteCommon;



class Sprite
{
public:	// メンバ関数
	// 初期化
	void Initialize(SpriteCommon*spriteCommon);
private:	// メンバ関数
	void CreateVertexData();
private:	// メンバ変数
	//
	
	// スプライトの設定用クラス
	SpriteCommon* spriteCommon = nullptr;
	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource;// = CreateBufferResource(device, sizeof(VertexData) * 6);
	Microsoft::WRL::ComPtr <ID3D12Resource> indexResource;// = CreateBufferResource(device, sizeof(uint32_t) * 6);
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexDataSprite = nullptr;
	uint32_t* indexData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	// 
};

