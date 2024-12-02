#include "ImGuiManager.h"
#include "../externals/imgui/imgui_impl_win32.h"
#include "../externals/imgui/imgui_impl_dx12.h"


void ImGuiManager::Initialize(WinAPI* winAPI, DirectXCommon* dxCommon)
{
	// 引数のnullptrチェック
	assert(dxCommon);
	dxCommon_ = dxCommon;
	// ImGuiのコンテキストを生成
	ImGui::CreateContext();
	// ImGuiのスタイルを設定
	ImGui::StyleColorsDark();
	// ImGuiのWin32の初期化
	ImGui_ImplWin32_Init(winAPI->GetHwnd());

	// ディスクリプタヒープ設定
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	// ディスクリプタヒープを生成
	HRESULT result = dxCommon->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvHeap_));
	assert(SUCCEEDED(result));

	// DirectX12用初期化
	ImGui_ImplDX12_Init(
		// デバイス
		dxCommon->GetDevice(),
		// バッファ数 / 型が合うようにキャスト
		static_cast<int>(dxCommon->GetBackBufferCount()),
		// RTVのフォーマット / RTVの設定と一致させる / SRVヒープ
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, srvHeap_.Get(),
		// ImGuiに割り当てるSRVのヒープハンドル / SRVの生成はImGui側がやってくれる
		srvHeap_->GetCPUDescriptorHandleForHeapStart(),
		// ImGuiに割り当てるSRVのヒープハンドル / GPU側のヒープハンドル
		srvHeap_->GetGPUDescriptorHandleForHeapStart()
	);
}

void ImGuiManager::Finalize()
{
	// ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// ディスクリプタヒープを解放
	srvHeap_.Reset();
}

void ImGuiManager::Begin()
{
	// ImGuiのフレームを開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::End()
{
	// ImGuiのフレームを終了
	ImGui::Render();
}

void ImGuiManager::Draw()
{
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	// ディスクリプタヒープの配列をセットするコマンド
	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap_.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	// ImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}
