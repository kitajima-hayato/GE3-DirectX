#include "Sprite.h"
#include "SpriteCommon.h"
void Sprite::Initialize(SpriteCommon* spriteCommon)
{
	this->spriteCommon = spriteCommon;
	
}

void Sprite::CreateVertexData()
{
	// VertexResourceを作る
	vertexResource = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * 6);

	// IndexResourceを作る
	// VertexBufferViewを作成する(値を設定するだけ)
	// IndexBufferViewを作成する
	// VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
	// IndexResourceにデータを書き込むためのアドレスを取得してindexDataに割り当てる

}
