#pragma once
#include "DirectXCommon.h"
class SpriteCommon
{
public:		// メンバ関数
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 共通描画設定
	/// </summary>
	void DrawSettingCommon();
public:		//	Getter,Setter
	DirectXCommon* GetDxCommon()const { return dxCommon_; }

public:	// Spriteクラスに向けたラッパー関数(wrapper function)
	void CreateSpriteVertexResource();
private:	// メンバ関数
	/// <summary>
	/// ルートシグネチャの作成
	/// </summary>
	void CreateRootSignatrue();

	/// <summary>
	/// グラフィックスパイプラインの生成
	/// </summary>
	void CreateGraficsPipeLine();
	/// <summary>
	/// PSO
	/// </summary>
	void InitializePixelShaderOutput();
	

private:	// メンバ変数
	// 絶対にnew,deleteしない
	DirectXCommon* dxCommon_;
	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	// 入力レイアウトの記述を設定
	D3D12_INPUT_LAYOUT_DESC inputLayoutDescs{};
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
	// ルートシグネチャ
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	// パイプラインステートの作成
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
};

