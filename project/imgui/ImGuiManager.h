#pragma once

#include "../externals/imgui/imgui.h"
#include "../engine/bace/WinAPI.h" 
#include <d3d12.h>
#include "../engine/bace/DirectXCommon.h"

class ImGuiManager
{

public:
	
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(WinAPI*winAPI,DirectXCommon*dxCommon);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize(); 
	
	/// <summary>
	/// ImGui受付開始
	/// <summary>
	void Begin();

	/// <summary>
	/// ImGui受付終了
	/// <summary>
	void End();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private:
	DirectXCommon* dxCommon_ = nullptr;
	// 
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_ = nullptr;


};
