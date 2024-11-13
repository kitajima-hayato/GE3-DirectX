#include "TextureManager.h"
#include "DirectXCommon.h"

TextureManager* TextureManager::instance = nullptr;

TextureManager* TextureManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new TextureManager();
	}
	return instance;
}

void TextureManager::DeleteInstance()
{
	if (instance != nullptr) {
		delete instance;
		instance = nullptr;
	}
}

void TextureManager::Initialize()
{
	// テクスチャの最大枚数を取得
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}
