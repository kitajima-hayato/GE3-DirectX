#include "DirectXCommon.h"
#include <cassert>
#include <format>
#include <string>
#include <thread>
#include "Logger.h"
#include "StringUtility.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
using namespace Logger;

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxcompiler.lib")
#pragma comment(lib,"dxguid.lib")

using namespace Microsoft::WRL;




DirectXCommon::DirectXCommon()
{
}
void DirectXCommon::Initialize(WinAPI* winAPI)
{
	// NULL検出
	assert(winAPI);
	// FPS固定初期化
	InitializeFixFPS();

	//借りてきたWinAPIのインスタンスを記録
	this->winAPI_ = winAPI;
	//デバイスの生成
	InitDevice();
	//コマンド関連の初期化
	InitCommand();
	//スワップチェインの生成
	CreateSwapChain();
	//各種ディスクリプターヒープの生成
	CreateDescriptorHeaps();
	//レンダーターゲットビューの初期化
	InitRenderTargetView();
	//深度ステンシルビューの初期化
	InitDepthStencilView();
	//フェンスの初期化
	InitFence();
	//ビューポート矩形の初期化
	InitViewportRect();
	//シザリング矩形の初期化
	InitScissorRect();
	// imguiの初期化
	//InitImGui();

	//DXCコンパイラの生成
	CreateDXCCompiler();


}

void DirectXCommon::InitCommand()
{
#pragma region commandAllocator
	//コマンドアロケーターを起動する
	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドアロケーターの生成が上手くいかなかったので起動できない
	assert(SUCCEEDED(hr));
#pragma endregion
#pragma region CommandList
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf()));
	//コマンドリストの生成が上手くいかなかったので起動できない
	assert(SUCCEEDED(hr));
#pragma endregion
#pragma region CommandQueue
	//コマンドキューを生成する
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//コマンドキューの生成が上手くいかなかったので起動できない
	assert(SUCCEEDED(hr));
#pragma endregion
}

void DirectXCommon::InitDevice()
{
	HRESULT hr;

#pragma region デバッグレイヤーをオン
	//移植前はここにifdefDEBUGがあった
	Microsoft::WRL::ComPtr < ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//
		debugController->EnableDebugLayer();
		//
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#pragma endregion
#pragma region DXGIFactoryの生成
	//DXGIファクトリーの生成

	//HRESULTはWindows系のエラーコードであり
	//関数が成功したか動かをSUCCEEDEDマクロ判定できる
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));//IID_PPV_ARGSは引数を一つにしてくれるおまじない
	//初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、
	//どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));
#pragma endregion
#pragma region アダプター
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	//良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(useAdapter.GetAddressOf())) !=
		DXGI_ERROR_NOT_FOUND; ++i) {
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));//取得できないのは一大事
		//ソフトウェアアダプタで無ければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//採用したアダプタあの情報をログに出力 wstringなので注意
			Log(StringUtility::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter.Reset();//ソフトウェアアダプタの場合は観なかったことにする
	}
	assert(useAdapter != nullptr);//適切なアダプタが見つからなかったら起動できない

#pragma endregion
#pragma region Deviceの生成

	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		//採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(useAdapter.Get(),
			featureLevels[i],
			IID_PPV_ARGS(&device));
		//指定した機能レベルでデバイスが生成されたかを確認
		if (SUCCEEDED(hr)) {
			//生成できたのでログ出力を行ってループを抜ける
			Log(std::format("FeatureLevels : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	//デバイスの生成が上手くいかなかったので生成できない
	assert(device != nullptr);
	Log("Complete create D3D12Device!!!\n");


#pragma endregion
#pragma region エラー時にブレイク
#ifdef _DEBUG // エラー時にブレークを発生させる設定
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
	if (SUCCEEDED(device.As(&infoQueue))) {
		//やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		D3D12_MESSAGE_ID denyIds[] = {
			//
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_INVALID_COMMAND_LIST_TYPE
		};
		//Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//抑制するレベル
		infoQueue->PushStorageFilter(&filter);
	}
#endif  _DEBUG
#pragma endregion



}

void DirectXCommon::CreateSwapChain()
{
#pragma region スワップチェインの生成
	swapChainDesc.Width = WinAPI::kClientWidth;		//画面の幅　ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Height = WinAPI::kClientHeight;	//画面の高さ　ウィンドウのクライアント町域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//
	swapChainDesc.SampleDesc.Count = 1;//色の形式
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//マルチサンプルしない
	swapChainDesc.BufferCount = 2;//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタに写したら中身を破壊
	//コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winAPI_->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	//スワップチェーンの生成が上手くいかなかったので起動できない
	assert(SUCCEEDED(hr));
#pragma endregion


}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateDepthBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height)
{
	// 生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;										// Textureのは幅
	resourceDesc.Height = height;									// Textureの高さ
	resourceDesc.MipLevels = 1;										// mipmapの数
	resourceDesc.DepthOrArraySize = 1;								//奥行き or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;			// Depthstencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1;								//サンプリングカウント。1固定。
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	//2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	// DepthStencilとして使う通知
	//.利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;					// VRAM上に作る

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;						// 1.0f (最大値)　でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;			// フォーマット。Resourceと合わせる

	// Resourceの生成

	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, // Heapの設定
		D3D12_HEAP_FLAG_NONE,//Heapの特殊な設定。特になし。
		&resourceDesc,// Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度値を書き込む状態にしておく
		&depthClearValue,// clear最適値
		IID_PPV_ARGS(&resource));//作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));

	return resource;
}
ComPtr <ID3D12DescriptorHeap>
DirectXCommon::CreateDescriptorHeap(
	D3D12_DESCRIPTOR_HEAP_TYPE heapType,
	UINT numDescriptors,
	bool shaderVisible)
{
	// ディスクリプタヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;//レンダーターゲットビュー
	descriptorHeapDesc.NumDescriptors = numDescriptors;//ダブルバッファ用に２つ。多くてもかまわない
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));//
	//ディスクリプタヒープがつくれなかったので起動できない
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

void DirectXCommon::CreateDescriptorHeaps() 
{
	//descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	///////////////////////////////dsV!!!!!!!!!!!!!!!!!!!!!!!!
#pragma region DescriptorHEAPの生成
	rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	//srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
	// DSV用のヒープでディスクリプタの数は１。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
#pragma endregion

}

void DirectXCommon::InitRenderTargetView()
{
	//スワップチェインからリソースを引っ張ってくる
	HRESULT hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//上手く取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));


	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();


	for (uint32_t i = 0; i < 2; ++i) {
		rtvHandles[i] = rtvStartHandle;

		device->CreateRenderTargetView(swapChainResources[i].Get(), &rtvDesc, rtvHandles[i]);
		rtvStartHandle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}



void DirectXCommon::PreDraw()
{
#pragma region バックバッファの番号取得
	// ここから書き込むバックバッファのインデックスを取得
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
#pragma endregion

#pragma region リソースバリアの設定

	// 今回のバリアは Transition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
	// 遷移前（現在）の ResourceStart
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後の ResourceStart
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// TransitionBarrier を張る
	commandList->ResourceBarrier(1, &barrier);
#pragma endregion 

#pragma region RTVとDSVの設定
	// 描画先のRTVとDSVを指定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
#pragma endregion

#pragma region 画面全体の深度をクリア
	// 指定した深度で画面全体をクリアする
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
#pragma endregion

#pragma region 画面全体の色をクリア
	// 指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };  // 青っぽい色。RGBAの順
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
#pragma endregion





#pragma region ビューポート領域の設定
	commandList->RSSetViewports(1, &viewport);   // Viewportを設定
#pragma endregion

#pragma region シザー矩形の設定
	commandList->RSSetScissorRects(1, &scissorRect);   // Scissorを設定
#pragma endregion
}

void DirectXCommon::PostDraw()
{
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
#pragma region リソースバリアで表示形態に変更
	// 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
		// 今回は RenderTarget から Present にする
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	// TransitionBarrier をは張る
	commandList->ResourceBarrier(1, &barrier);
#pragma endregion
#pragma region グラフィックコマンドをクローズ
	// コマンドリストの内容を確定させる。すべてのコマンドを積んでから Close すること
	HRESULT hr = commandList->Close();
	assert(SUCCEEDED(hr));
#pragma endregion
#pragma region GPUコマンドの実行
	//GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = { commandList.Get()};
	commandQueue->ExecuteCommandLists(1, commandLists);
#pragma endregion
#pragma region GPU画面の交換を通知
	//GPUとOSに画面の交渉を行うように通知する
	swapChain->Present(1, 0);
#pragma endregion
#pragma region Fenceの値を更新
	//Fenceの値を更新
	fenceValue++;
	//GPUがここまでたどり着いた時に、Fenceの値を指定した値に代入するようにSignalを送る
	commandQueue->Signal(fence.Get(), fenceValue);
#pragma endregion
#pragma region コマンド完了待ち
	// Fenceの値が指定したSignal値にたどり着いているか確認する
		//GetCompletedValueの初期値はFence作成時に渡した初期値
	if (fence->GetCompletedValue() < fenceValue) {
		//指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		//イベントを待つ
		WaitForSingleObject(fenceEvent, INFINITE);
	}
#pragma endregion

	// FPS固定
	UpdateFixFPS();

#pragma region コマンドアロケーターのリセット
	//次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
#pragma endregion
#pragma region コマンドリストのリセット
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));
#pragma endregion



}

void DirectXCommon::WaitCommand()
{
	
#pragma region グラフィックコマンドをクローズ
	// コマンドリストの内容を確定させる。すべてのコマンドを積んでから Close すること
	HRESULT hr = commandList->Close();
	assert(SUCCEEDED(hr));
#pragma endregion
#pragma region GPUコマンドの実行
	//GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(1, commandLists);
#pragma endregion

#pragma region Fenceの値を更新
	//Fenceの値を更新
	fenceValue++;
	//GPUがここまでたどり着いた時に、Fenceの値を指定した値に代入するようにSignalを送る
	commandQueue->Signal(fence.Get(), fenceValue);
#pragma endregion
#pragma region コマンド完了待ち
	// Fenceの値が指定したSignal値にたどり着いているか確認する
		//GetCompletedValueの初期値はFence作成時に渡した初期値
	if (fence->GetCompletedValue() < fenceValue) {
		//指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		//イベントを待つ
		WaitForSingleObject(fenceEvent, INFINITE);
	}
#pragma endregion
#pragma region コマンドアロケーターのリセット
	//次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
#pragma endregion
#pragma region コマンドリストのリセット
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));
#pragma endregion



}

//D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index)
//{
//	return GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
//}

//D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index)
//{
//	return GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
//}

void DirectXCommon::InitDepthStencilView()
{

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//Format.基本的にはResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2dTexture

	Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilResource;
	depthStencilResource = CreateDepthBuffer(device, WinAPI::kClientWidth, WinAPI::kClientHeight);
	//DSVの設定									 

	//DSVHeapの先頭にDSVをつくる
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

}

void DirectXCommon::InitFence()
{
	HRESULT hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));
	assert(SUCCEEDED(hr));


	assert(fenceEvent != nullptr);
}

void DirectXCommon::InitViewportRect()
{
	// ビューポート矩形の設定
	viewport.Width = WinAPI::kClientWidth;
	viewport.Height = WinAPI::kClientHeight;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

void DirectXCommon::InitScissorRect()
{

	scissorRect.left = 0;
	scissorRect.right = WinAPI::kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinAPI::kClientHeight;
}

void DirectXCommon::CreateDXCCompiler()
{

	// DXCユーティリティの生成
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(dxcUtils.GetAddressOf()));
	assert(SUCCEEDED(hr));
	// DXCコンパイラの生成
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(dxcCompiler.GetAddressOf()));
	assert(SUCCEEDED(hr));

	// デフォルトインクルードハンドラの生成
	// 現時点でincludeはしないが、includeに対応するための設定を行っておく
	hr = dxcUtils->CreateDefaultIncludeHandler(includeHandler.GetAddressOf());
	assert(SUCCEEDED(hr));

}

void DirectXCommon::InitImGui()
{
	////Imguiの初期化
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();
	//ImGui::StyleColorsDark();
	//ImGui_ImplWin32_Init(winAPI_->GetHwnd());
	//ImGui_ImplDX12_Init(device.Get(), swapChainDesc.BufferCount, rtvDesc.Format, srvDescriptorHeap.Get(),
	//	srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());


}


Microsoft::WRL::ComPtr<IDxcBlob> DirectXCommon::CompileShader(
	const std::wstring& filePath, const wchar_t* profile)
{
	// これからシェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompileShader, profile:{}\n", filePath, profile)));

	// hlslファイルを読み込む
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	// 読めなかったら止める
	assert(SUCCEEDED(hr));

	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; // UTFの文字コードであることを通知

	LPCWSTR arguments[] = {
		filePath.c_str(), // コンパイル対象のhlslファイル名
		L"-E", L"main", // エントリーポイントの指定。基本的にmain以外にはしない
		L"-T", profile, // Shader Profileの設定
		L"-Zi", L"-Qembed_debug", // デバッグ用の情報を埋め込む
		L"-Od", // 最適化を外しておく
		L"-Zpr", // メモリレイアウトは行優先
	};

	// 実際にShaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer, // 読み込んだファイル
		arguments, // コンパイルオプション
		_countof(arguments), // コンパイルオプションの数
		includeHandler.Get(), // includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult) // コンパイル結果
	);
	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		assert(false);
	}

	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	return shaderBlob;
}




std::wstring DirectXCommon::ConvertString(const std::string& str)
{
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string DirectXCommon::ConvertString(const std::wstring& str)
{
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

Microsoft::WRL::ComPtr<ID3D12Resource>
DirectXCommon::CreateBufferResource(size_t sizeInBytes)
{
	//頂点リソース用のヒープ設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//
	//　リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// リソースの次元をバッファに設定
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	// バッファサイズの指定
	vertexResourceDesc.Width = sizeInBytes;
	
	vertexResourceDesc.Height = 1;				// バッファのサイズは常に１
	vertexResourceDesc.DepthOrArraySize = 1;	// 深さまたは配列サイズの１
	vertexResourceDesc.MipLevels = 1;			// ミップレベル
	vertexResourceDesc.SampleDesc.Count = 1;	// サンプリング数
	
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//　バッファリソースの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, 
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));
	return vertexResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResource(const DirectX::TexMetadata& metadata)
{
	// 1. metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = static_cast<UINT>(metadata.width); // Textureの幅
	resourceDesc.Height = static_cast<UINT>(metadata.height); // Textureの高さ
	resourceDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels); // mipmapの数
	resourceDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize); // 奥行or配列Textureの配列数
	resourceDesc.Format = metadata.format; // TextureのFormat
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント1固定
	resourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension); // Textureの次元数。普段使っているのは2次元

	// 2. 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // 細かい設定を行う

	// 3. Resourceを生成する
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,				// Heapの設定
		D3D12_HEAP_FLAG_NONE,			// Heapの特殊な設定,特になし
		&resourceDesc,					// Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST, // データ転送される設定
		nullptr,						// Clear最適値 使わないのでnullptr
		IID_PPV_ARGS(&resource)			// 作成するResourceポインタへのポインタ
	);

	assert(SUCCEEDED(hr));
	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages)
{
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, static_cast<UINT>(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(intermediateSize);
	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

	// Textureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);

	return intermediateResource;
}

void DirectXCommon::InitializeFixFPS()
{
	// 現地時間を記録する
	reference_ = std::chrono::steady_clock::now();
}

void DirectXCommon::UpdateFixFPS()
{
	// 1/60秒ぴったりの時間
	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
	// 1/60秒よりわずかに短い時間
	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

	// 現地時間を取得する
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	// 前回記録からの経過時間を取得する
	std::chrono::microseconds elapsed =
		std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	// 1/60秒 (よりわずかに短い時間)盾居ない場合
	if (elapsed < kMinCheckTime) {
		// 1/60秒経過するまで微小なスリープを繰り返す
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
			// 1マイクロ秒リソース
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
	// 現在の時間を記録する
	reference_ = std::chrono::steady_clock::now();

}

