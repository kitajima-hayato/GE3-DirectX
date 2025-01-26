#pragma once
#include "DirectXCommon.h"
#include "Camera.h"
// 3Dオブジェクト共通部
// シングルトンクラス
class Object3DCommon
{
public:	// メンバ関数
	// 初期化
	void Initialize(DirectXCommon* dxCommon);
	// 共通描画設定
	void DrawSettingCommon();
public:	// Getter/Setter
	static Object3DCommon* GetInstance();
	Object3DCommon() = default;
	~Object3DCommon() = default;
	Object3DCommon(Object3DCommon&) = delete;
	Object3DCommon& operator=(Object3DCommon&) = delete;
	static void Deletenstance();

	// DirectXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	Camera* GetDefaultCamera()const { return defaultCamera; }
private:	// メンバ関数
	// ルートシグネチャの作成
	void CreateRootSignatrue();
	// グラフィックスパイプラインの生成
	void CreateGraficsPipeLine();

private:	// メンバ変数
	// カメラ
	Camera* defaultCamera = nullptr;
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
	// 入力要素の定義配列を初期化
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	// シングルトンインスタンス
	static Object3DCommon* instance;
};

