#pragma once
#include "SrvManager.h"
#include "DirectXCommon.h"
#include "MyMath.h"
#include <vector>
#include <random>
class ParticleManager
{
public:
	// パーティクルの初期化
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

private:	// メンバ関数
	// グラフィックスパイプラインの生成
	void CreateGraphicsPipeLine();
	// ルートシグネチャの作成
	void CreateRootSignature();
	// 頂点データの初期化(座標等)
	void InitializeVertexData();
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
	// ルートシグネチャ
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;

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
	// パイプラインステートの作成
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	// 頂点データ
	Microsoft::WRL::ComPtr<ID3D12Resource>vertexResource = nullptr;
	// 頂点データの初期化(座標等)
	std::vector<VertexData> vertexData = {};
};

