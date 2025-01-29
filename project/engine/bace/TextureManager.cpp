#include "TextureManager.h"
#include "DirectXCommon.h"
#include "StringUtility.h"
#include "DirectXCommon.h"
TextureManager* TextureManager::instance = nullptr;
// ImGuiで0を使用しているため１から開始
uint32_t TextureManager::kSRVIndexTop = 1;
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

const DirectX::TexMetadata& TextureManager::GetMetadata(const std::string& filePath)

{
	// テクスチャデータを検索
	auto it = textureDatas.find(filePath);

	// テクスチャが見つからない場合はエラー
	assert(it != textureDatas.end());

	// 見つかったテクスチャデータのメタデータを返す
	return it->second.metadata;
}


uint32_t TextureManager::GetSrvIndex(const std::string& filePath)
{
	// テクスチャが存在するか確認
	auto it = textureDatas.find(filePath);
	if (it == textureDatas.end()) {
		// なかったらエラーメッセージ
		throw std::runtime_error("Texture not found for filePath: " + filePath);
	}
	if (it != textureDatas.end()) {
		// 読み込み済みなら要素番号を返す
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
		return textureIndex;
	}
	assert(0);
	return 0;
}

//D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath)
//{
//	// テクスチャが存在するか確認
//	auto it = textureDatas.find(filePath);
//	if (it == textureDatas.end()) {
//		// なかったらエラーメッセージ
//		
//		throw std::runtime_error("Texture not found for filePath: " + filePath);
//	}
//
//	// テクスチャデータの参照を取得
//	TextureData& textureData = it->second;
//	return textureData.srvHandleGPU;
//}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath)
{
	// テクスチャデータを検索
	auto it = textureDatas.find(filePath);

	// テクスチャが見つからない場合は例外を投げる
	if (it == textureDatas.end()) {
		throw std::runtime_error("Texture not found for filePath: " + filePath);
	}
	// 見つかったテクスチャデータのGPUハンドルを返す
	return it->second.srvHandleGPU;
}




void TextureManager::Initialize(DirectXCommon* dxCommon,SrvManager*srvManager)
{
	this->dxCommon = dxCommon;
	this->srvManager = srvManager;
	// テクスチャの最大枚数を取得/SRVの最大数を取得
	textureDatas.reserve(SrvManager::kMaxSRVCount);
}

void TextureManager::Finalize()
{// テクスチャデータのクリア
	textureDatas.clear();
}

void TextureManager::LoadTexture(const std::string& filePath)
{
	// 読み込み済みのテクスチャかどうかをチェック
	if (textureDatas.contains(filePath)) {
		// 読み込み済みなら何もしない
		return;
	}

	// テクスチャ枚数上限を超えている場合
	assert(srvManager->IsAllocate());

	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// テクスチャデータの追加
	TextureData textureData;
	textureData.metadata = mipImages.GetMetadata();
	// テクスチャリソースの生成
	textureData.resource = dxCommon->CreateTextureResource(textureData.metadata);
	// 中間リソースの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommon->UploadTextureData(textureData.resource, mipImages);
	// コマンドのキックを待つ
	dxCommon->WaitCommand();

	// テクスチャデータの要素数番号をSRVのインデックスとする
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size()) + kSRVIndexTop;

	// SRV確保
	textureData.srvIndex = srvManager->Allocate();
	textureData.srvHandleCPU = srvManager->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager->GetGPUDescriptorHandle(textureData.srvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	// SRVを作成
	dxCommon->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	// textureDatasに追加
	textureDatas.emplace(filePath, std::move(textureData));

}