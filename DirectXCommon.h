#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <array>
#include <dxcapi.h>
#include <chrono>
#include "WinAPI.h"
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"
// 最大SRV数(最大テクスチャ枚数)


// DirectX基盤
class DirectXCommon
{
public:
	static uint32_t kMaxSRVCount;

	/// <summary>
	// コンストラクタ
	/// <summary>
	DirectXCommon();
	/// <summary>
	//初期化
	/// <summary>
	void Initialize(WinAPI* winAPI);
	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画後処理
	/// </summary>
	void PostDraw();

	/// <summary>
	/// シェーダーのコンパイル
	/// </summary>
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile);

	/// <summary>
	/// バッファリソースの生成
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource>
		CreateBufferResource(size_t sizeInBytes);

	/// <summary>
	/// テクスチャファイルパスの読み込み
	/// <param name ="filePath"テクスチャファイルのパス>
	/// </summary>
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);

	/// <summary>
	/// テクスチャリソースの生成
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource>CreateTextureResource(const DirectX::TexMetadata& metadata);

public:		// Getter,Setter
	/// <summary>
	/// getter
	/// </summary>
	ID3D12Device* GetDevice() const { return device.Get(); }
	ID3D12GraphicsCommandList* GetCommandList()const { return commandList.Get(); }
	/// <summary>
	/// DxcUtils取得関数
	/// </summary>
	Microsoft::WRL::ComPtr <IDxcUtils> 
		GetDxcUtils()const { return dxcUtils; }

	/// <summary>
	/// DxcCompiler取得関数
	/// </summary>
	Microsoft::WRL::ComPtr<IDxcCompiler3> 
		GetDxcCompiler()const { return dxcCompiler; }

	/// <summary>
	/// インクルードハンドラ取得関数
	/// </summary>
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> 
		GetIncludeHandler()const { return includeHandler; }

	/// <summary>
	/// srvDescriptorHeap取得関数
	/// </summary>
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> 
		GetSrvDescriptorHeap()const { return srvDescriptorHeap; }

	/// <summary>
	/// 指定番号のCPUディスクリプタハンドルを取得する
	/// </summary>
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	/// <summary>
	/// 指定番号のGPUディスクリプタハンドルを取得する
	/// </summary>
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// テクスチャデータの転送
	/// [[nodiscard]]とは、戻り値が無視されると警告を出す
	/// </summary>
	[[nodiscard]]
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData
	(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages);

	/// <summary>
	/// SRVの指定番号のCPUディスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	/// <summary>
	/// SRVの指定番号のGPUディスクリプタハンドルを取得する
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

private:	// 内部処理専用関数
	/// <summary>
	/// コマンド関連の初期化
	/// </summary>
	void InitCommand();

	/// <summary>
	/// デバイスの初期化
	/// </summary>
	void InitDevice();

	/// <summary>
	/// SwapChainの生成
	/// </summary>
	void CreateSwapChain();

	/// <summary>
	/// 深度バッファの生成
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);
	
	/// <summary>
	/// ディスクリプターヒープ作成関数
	/// </summary>
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap>  CreateDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE heapType,
		UINT numDescriptors,
		bool shaderVisible);
	
	/// <summary>
	/// 各種ディスクリプターヒープの生成
	/// </summary>
	
	void CreateDescriptorHeaps();
	/// <summary>
	/// レンダーターゲットビューの初期化
	/// </summary>
	void InitRenderTargetView();
	
	/// <summary>
	/// 深度ステンシルビューの初期化
	/// </summary>
	void InitDepthStencilView();

	/// <summary>
	/// フェンスの生成
	/// </summary>
	void InitFence();

	/// <summary>
	/// ビューポート矩形の初期化
	/// </summary>
	void InitViewportRect();

	/// <summary>
	/// シザリング矩形の初期化
	/// </summary>
	void InitScissorRect();

	/// <summary>
	/// DXCコンパイラの生成
	/// </summary>
	void CreateDXCCompiler();

	/// <summary>
	/// ImGuiの初期化
	/// </summary>
	void InitImGui();

	/// <summary>
	/// シェーダーコンパイル関数
	/// </summary>
	static std::wstring ConvertString(const std::string& str);
	static std::string ConvertString(const std::wstring& str);
	
	/// <summary>
	/// FPS固定初期化
	/// </summary>
	void InitializeFixFPS();
	
	/// <summary>
	/// FPS固定更新
	/// </summary>
	void UpdateFixFPS();


private:
	// WindowsAPI
	WinAPI* winAPI_ = nullptr;

private:		// メンバ変数
	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	// コマンドアロケーター
	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> commandAllocator;
	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	// コマンドキュー
	Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue;
	// スワップチェインの生成
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	// スワップチェイン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
	// スワップチェインリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;
	// リソースの生成
	Microsoft::WRL::ComPtr <ID3D12Resource> resource;
	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	// 各ディスクリプターヒープのメンバ変数
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> srvDescriptorHeap;
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap;
	// 各種ディスクリプタサイズ
	uint32_t descriptorSizeSRV;
	uint32_t descriptorSizeRTV;

	// 生成の成果物、フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	//ビューポート矩形の設定保存用メンバ変数
	D3D12_VIEWPORT viewport{};
	// dxcCompilerを初期化
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	// インクルードハンドラ
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;
	// TransitionBarrier の設定
	D3D12_RESOURCE_BARRIER barrier{};
	// RTVを2つ作るのでディスクリプタを2つ用意
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> rtvHandles;
	// FenceのSignalを待つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	// シザリング矩形の設定
	D3D12_RECT scissorRect{};
	// 初期値0でFenceを作成
	uint64_t fenceValue = 0;
	// ここから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex = 2;
	// 深度リソース
	Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilResource;
	// 記録時間(FPS固定用)
	std::chrono::steady_clock::time_point reference_;

	
};

