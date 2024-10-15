#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinAPI.h"
// DirectX基盤
class DirectXCommon
{
public:
	//初期化
	void Initialize(WinAPI* winAPI);
	//コマンド関連の初期化
	void InitCommand();
	//デバイスの初期化
	void InitDevice();

	// SwapChainの生成
	void CreateSwapChain();
	// 深度バッファの生成
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
	// スワップチェイン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
	// スワップチェインリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> swapChainResources[2];
	// リソースの生成
	Microsoft::WRL::ComPtr <ID3D12Resource> resource = nullptr;
	// 各ディスクリプターヒープのメンバ変数
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> srvDescriptorHeap;
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap;
};
