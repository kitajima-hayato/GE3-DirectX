#pragma once
#include "SrvManager.h"
#include "DirectXCommon.h"
#include "MyMath.h"
#include "TextureManager.h"
#include "Camera.h"
#include <vector>
#include <random>
class ParticleManager
{
public:
	// 構造体
	struct ParticleGroup {
		MaterialData materialData;		// マテリアルデータMaterialData
		std::list<Particle> particles;	// パーティクルのリスト
		uint32_t instancingDataSRVIndex;			// インスタンシングデータのSRVインデックス
		Microsoft::WRL::ComPtr<ID3D12Resource>instancingResource;	// インスタンシングデータのリソース
		const uint32_t kNumMaxInstance = 100;	// 最大インスタンス数
		ParticleForGPU* instancingData;	// インスタンシングデータを書き込むためのポインタ

	};
public:	// メンバ関数
	// パーティクルの初期化
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);
	// パーティクルグループの生成
	void CreateParticleGroup(const std::string& name, const std::string textureFilePath);
	// 更新
	void Update(Camera* camera);
	// 描画
	void Draw();
	// パーティクルの生成(Emit)
	void Emit(const std::string& name, const Vector3& position, uint32_t count);
	// パーティクルの生成
	Particle MakeParticle(std::mt19937& randomEngine, const Vector3& position);

public:	// シングルトン
	static ParticleManager* instance;
	// シングルトンインスタンスを取得
	static ParticleManager* GetInstance();
	// シングルトンインスタンスを解放
	static void DeleteInstance();


private:	// メンバ関数
	// グラフィックスパイプラインの生成
	void CreateGraphicsPipeLine();
	// ルートシグネチャの作成
	void CreateRootSignature();
	// 頂点リソース生成
	void CreateVertexResource();
	// 頂点バッファビューの(VBV)の作成
	void CreateVertexBufferView();
	// 頂点リソースに頂点データを書き込む
	void WriteVertexResource();
private:
	// 絶対にnewしない
	DirectXCommon* dxCommon;
	// 絶対にnewしない
	SrvManager* srvManager;
	// カメラクラス
	Camera* camera = nullptr;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	// パイプラインステートの作成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
	// 入力レイアウトの記述を設定
	D3D12_INPUT_LAYOUT_DESC inputLayoutDescs{};
	// 入力要素の定義配列を初期化
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	// 頂点シェーダーのコンパイル結果を格納するBlob
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = nullptr;
	// ピクセルシェーダーのコンパイル結果を格納するBlob
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = nullptr;
	// ブレンドステートの設定
	D3D12_BLEND_DESC blendDesc{};
	// ラスタライザーステートの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	// バーテックスバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	// ランダムエンジン
	std::random_device seedGenaerator;
	std::mt19937 randomEngine;
	// 頂点データ
	Microsoft::WRL::ComPtr<ID3D12Resource>vertexResource = nullptr;
	// 頂点データの初期化(座標等)
	std::vector<VertexData> vertexData = {};

	// パーティクルグループコンテナ
	std::unordered_map<std::string, ParticleGroup> particleGroups;
	// Δtを定義６０fos固定
	const float kDeltaTime = 1.0f / 60.0f;
};

