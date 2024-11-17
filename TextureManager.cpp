#include "TextureManager.h"
#include "DirectXCommon.h"
#include "StringUtility.h"
#include "DirectXCommon.h"
TextureManager* TextureManager::instance = nullptr;
// ImGuiで0を使用しているため１から開始
uint32_t TextureManager::kSRVIndexTop = 1;
TextureManager* TextureManager::GetInstance()
{
	DirectXCommon* dxCommon = new DirectXCommon();
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
	// テクスチャの最大枚数を取得/SRVの最大数を取得
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

void TextureManager::Finalize()
{
}

void TextureManager::LoadTexture(const std::string& filePath)
{

	// 読み込み済みのテクスチャかどうかをチェック
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](const TextureData& textureData) {
			return textureData.filePath == filePath; }
	);
	// テクスチャ枚数上限を超えている場合
	assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);

	if (it != textureDatas.end()) {
		// すでに読み込み済みのテクスチャの場合リターン
		return;
	}



	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));



	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	//テクスチャデータの追加
	textureDatas.resize(textureDatas.size() + 1);
	//　追加したテクスチャデータの参照を取得
	TextureData& textureData = textureDatas.back();
	// ファイル名の保存
	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	// テクスチャリソースの生成
	textureData.resource = dxCommon->CreateTextureResource(textureData.metadata);

	// テクスチャデータの要素数番号をSRVのインデックスとする
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;

	textureData.srvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(srvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	// 2Dテクスチャ
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);
	// Imguiのディスクリプタハンドルを先頭にして使うならここにテコ入れが必要
	///////////////////////////////////////////////////////
	///////////////////////////////////////////////////////

	// SRVを作成
	dxCommon->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);
}