#pragma once
#include <string>
#include <wrl/client.h>
#include <d3d12.h>
#include "DirectXCommon.h"
#include "externals/DirectXTex/DirectXTex.h"
// テクスチャ管理クラス
// テクスチャの読み込み、解放を行う
// シングルトンクラス
class TextureManager
{
	// SRVインデックスの開始番号
	static uint32_t kSRVIndexTop;
public:// メンバ関数
	// 初期化
	void Initialize(DirectXCommon*dxCommon);
	// 終了処理
	void Finalize();
	/// <summary>
	/// テクスチャファイルパスの読み込み
	/// <param name ="filePath"テクスチャファイルのパス>
	/// </summary>
	void LoadTexture(const std::string& filePath);
public:	// Getter,Setter
	// シングルトンインスタンスを取得
	static TextureManager* GetInstance();
	// シングルトンインスタンスを解放
	static void DeleteInstance();
	// SRVインデックスの開始番号
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);
	// テクスチャ番号からGPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

	// メタデータを取得
	const DirectX::TexMetadata& GetMetadata(uint32_t textureIndex);

private: // メンバ関数/構造体
	

	//テクスチャ１枚分の情報
	struct TextureData
	{
		std::string filePath;	// ファイル名
		DirectX::TexMetadata metadata;	// テクスチャの情報(幅、高さ)
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;	// テクスチャリソース
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;	// シェーダリソースビューのCPUハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;	// 描画コマンドに必要なGPUハンドル
       
	};

private:	// シングルトン 
	static TextureManager* instance;
	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

private:// メンバ変数
	DirectXCommon* dxCommon = nullptr;
	
	// テクスチャデータ
	std::vector<TextureData> textureDatas;	// vectorにすることで読み込み済みのテクスチャの枚数をカウントできる
	
	
};
