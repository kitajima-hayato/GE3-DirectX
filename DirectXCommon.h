#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

// DirectX基盤
class DirectXCommon
{
public:
	//初期化
	void Initialize();

	//デバイスの初期化
	void InitDevice();

private:
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	Microsoft::WRL::ComPtr<ID3D12Device> device;


};

