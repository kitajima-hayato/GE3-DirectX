#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <array>
#include <dxcapi.h>
#include "WinAPI.h"
// DirectX基盤
class DirectXCommon
{
public:
	//初期化
	void Initialize(WinAPI* winAPI);

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
	void CreateDepthBuffer();
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
	/// SRVの指定番号のCPUディスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	/// <summary>
	/// SRVの指定番号のGPUディスクリプタハンドルを取得する
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

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
	/// 指定番号のCPUディスクリプタハンドルを取得する
	/// </summary>
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	/// <summary>
	/// 指定番号のGPUディスクリプタハンドルを取得する
	/// </summary>
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);


private:
	// WindowsAPI
	WinAPI* winAPI = nullptr;

private:
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
	Microsoft::WRL::ComPtr <ID3D12Resource> resource = nullptr;
	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	// 各ディスクリプターヒープのメンバ変数
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> srvDescriptorHeap;
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap;
	// 各種ディスクリプタサイズ
	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 生成の成果物、フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	//ビューポート矩形の設定保存用メンバ変数
	D3D12_VIEWPORT viewport{};
	// dxcCompilerを初期化
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;

};
