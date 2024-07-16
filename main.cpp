#include<Windows.h>
#include<cstdint>
#include<string>
#include<format>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>
#include<dxgidebug.h>
#include<dxcapi.h>
#include<fstream>
#include<sstream>
#include"Math.h"
#include"MakeMatrix.h"
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"
#include<vector>
#include"numbers"
#pragma comment(lib,"dxcompiler.lib")
#pragma  comment(lib,"dxguid.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

struct ModelData {
	std::vector<VertexData>vertices;
};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwndm, UINT msg, WPARAM wParam, LPARAM lParam);

ID3D12DescriptorHeap* CreateDescropterHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heaptype, UINT numDescripter, bool shaderVisible) {
	//ディスクリプタヒープの生成
	ID3D12DescriptorHeap* descroptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heaptype;//レンダーターゲットビュー
	descriptorHeapDesc.NumDescriptors = numDescripter;//ダブルバッファ用に２つ。多くてもかまわない
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descroptorHeap));//
	//ディスクリプタヒープがつくれなかったので起動できない
	assert(SUCCEEDED(hr));
	return descroptorHeap;
}

std::wstring ConvertString(const std::string& str) {//ワイドストリング
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

std::string ConvertString(const std::wstring& str) {
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


void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}

IDxcBlob* CompileShader(
	//CompilerするShaderファイルへのパス
	const std::wstring& filePath,
	//Coompilerみ使用するProfile
	const wchar_t* profile,
	//初期化で生成したものを３つ
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler
) {
	//これからシェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin ConpilerShader, profile:{}\n", filePath, profile)));
	//hlslファイルを読み込む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	//読めなかったら止める
	assert(SUCCEEDED(hr));
	//読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTFの文字コードであることを通知

	LPCWSTR arguments[] = {
		filePath.c_str(), // コンパイル対象のhislファイル名
		L"-E", L"main", // エントリーポイントの指定。基本的にmain以外にはしない
		L"-T", profile, // Shader Profileの設定
		L"-Zi", L"-Qembed_debug", // デバッグ用の情報を埋め込む
		L"-Od",// 最適化を外しておく
		L"-Zpr",// メモリレイアウトは行優先
	};

	// 実際にShaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,		// 読み込んだファイル
		arguments,					// コンパイルオプション
		_countof(arguments),		// コンパイルオプションの数
		includeHandler,				// includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult) // コンパイル結果
	);
	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));
	//
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		//
		assert(false);
	}
	//
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//
	shaderSource->Release();
	shaderResult->Release();
	//
	return shaderBlob;

}
ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	//頂点リソース用のヒープ設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//
	//
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	//
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;//
	//
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//
	ID3D12Resource* vertexResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));
	return vertexResource;
}



LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
	//メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		//ウィンドウが破棄された
	case WM_DESTROY:
		//OSに対してアプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}
	//標準メッセージの処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}



DirectX::ScratchImage LoadTexture(const std::string& filePath) {

	//テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	//ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));
	//みっぷマップ付きのデータを返す
	return mipImages;
}

#pragma region テクスチャリソース
ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {
	//1. metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);//Textureの幅
	resourceDesc.Height = UINT(metadata.height);//textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);//mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);//奥行or配列Textureの配列数
	resourceDesc.Format = metadata.format;//TextureのFormat
	resourceDesc.SampleDesc.Count = 1; //サンプリングカウント1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);//Textureの次元数。普段使っているのは２次元
	//2. 利用するHeapの設定
	//利用するHeapの設定。非常に特殊な運用。
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//細かい設定を行う
	//heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//WriteBackポリシーでCPUアクセス可能
	//heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//プロセッサの近くには配置
	//3. Resourceを生成する
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,//Heapの設定
		D3D12_HEAP_FLAG_NONE,//Heapの特殊な設定,特になし
		&resourceDesc,//Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST,//データ転送される設定
		nullptr,//Clear最適値 使わないのでnullptr
		IID_PPV_ARGS(&resource));//作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));
	return resource;
}
#pragma endregion
#pragma region テクスチャリソースにデータ転送関数
[[nodiscard]]
ID3D12Resource* UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	std::vector<D3D12_SUBRESOURCE_DATA>subresources;
	DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresources.size()));
	ID3D12Resource* intermediateResource = CreateBufferResource(device, intermediateSize);
	UpdateSubresources(commandList, texture, intermediateResource, 0, 0, UINT(subresources.size()), subresources.data());
	//Textureへの転送後は利用できるよう、D3D12_RESORCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;
}

#pragma endregion

#pragma region 3-1の関数
ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t heigth) {
	//生成するResourceの設定
	D3D12_RESOURCE_DESC resorceDesc{};
	resorceDesc.Width = width;//Textureの幅
	resorceDesc.Height = heigth;//Textureの高さ
	resorceDesc.MipLevels = 1;//mipmapの数
	resorceDesc.DepthOrArraySize = 1;//奥行or配列Textureの配列数
	resorceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DepthStecilとして利用可能なフォーマット
	resorceDesc.SampleDesc.Count = 1;//サンプリングカウント　１固定
	resorceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元
	resorceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//DepthStecilとして使う通知

	//利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//VRAM上につくる

	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;//1.0f（最大値）でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット。Resourceと合わせる

	//Resourceの生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,//Heapの設定
		D3D12_HEAP_FLAG_NONE,//Heapの特殊な設定.特に無し
		&resorceDesc,//Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度値を書き込む状態にしておく
		&depthClearValue,//Clear最適値
		IID_PPV_ARGS(&resource));//作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));
	return resource;
}
#pragma endregion 

#pragma region DescriptorHandleの取得
D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

#pragma endregion

#pragma region objファイルの読み込み
//objファイルの読み込み関数
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	//1.必要な変数の宣言
	ModelData modelData;			//構築するModelData
	std::vector<Vector4>positions;	//位置
	std::vector<Vector3>normals;	//法線
	std::vector<Vector2>texcoords;	//テクスチャ座標
	std::string line;				//ファイルから飲んだ1行を格納するもの
	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());//ファイルを開けなかったら止める

#pragma region ファイル読み込み
	//3.実際にファイルを読みModelDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;//先頭の識別子を読む
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				//頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');//区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				//要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2 - 1]];
				VertexData vertex={ position,texcoord,normal };
				modelData.vertices.push_back(vertex);
			}
		}
	}
#pragma endregion
	//4.ModelDataを返す

}
#pragma endregion


//ウィンドウズアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	CoInitializeEx(0, COINIT_MULTITHREADED);
	const int kClientWidth = 1280;
	const int kClientHeigth = 720;


#pragma region Windowの生成

	WNDCLASS wc{};
	//ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;//上の関数を渡している
	//ウィンドウクラス名
	wc.lpszClassName = L"CG2";
	//インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	//ウィンドウクラスを登録
	RegisterClass(&wc);
	//クライアント領域のサイズ
	const int32_t kClienWidth = 1280;
	const int32_t kClientHeight = 710;
	//ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0,0,kClienWidth,kClientHeight };//RECT レクタングル(矩形)
	//クライアント領域をもとに実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	//ウィンドウの生成
	HWND hwnd = CreateWindow(
		wc.lpszClassName,        //利用するクラスメイン
		L"CG2",                  //タイトルバーの文字
		WS_OVERLAPPEDWINDOW,     //よく見るウィンドウスタイル
		CW_USEDEFAULT,           //表示X座標
		CW_USEDEFAULT,           //表示Y座標
		wrc.right - wrc.left,    //ウィンドウ横幅
		wrc.bottom - wrc.top,    //ウィンドウ縦幅
		nullptr,                 //親ウィンドウハンドル
		nullptr,                 //メニューハンドル
		wc.hInstance,            //インスタンスハンドル
		nullptr                  //オプション
	);//ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);
#pragma endregion 

#ifdef _DEBUG
	ID3D12Debug1* debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//
		debugController->EnableDebugLayer();
		//
		debugController->SetEnableGPUBasedValidation(TRUE);
	}


#endif // !_DEBUG;



#pragma region Factoryの生成
	//DXGIファクトリーの生成
	IDXGIFactory7* dxgiFactory = nullptr;
	//HRESULTはWindows系のエラーコードであり
	//関数が成功したか動かをSUCCEEDEDマクロ判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));//IID_PPV_ARGSは引数を一つにしてくれるおまじない
	//初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、
	//どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region アダプター
	IDXGIAdapter4* useAdapter = nullptr;
	//良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; i++) {
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));//取得できないのは一大事
		//ソフトウェアアダプタで無ければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//採用したアダプタあの情報をログに出力 wstringなので注意
			Log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;//ソフトウェアアダプタの場合は観なかったことにする
	}
	assert(useAdapter != nullptr);//適切なアダプタが見つからなかったら起動できない

#pragma endregion

#pragma region Deviceの生成
	ID3D12Device* device = nullptr;
	//昨日レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		//採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
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
#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
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
		//解放
		infoQueue->Release();


	}
#endif // _DEBUG

#pragma endregion



#pragma region CommandQueue
	//コマンドキューを生成する
	ID3D12CommandQueue* commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//コマンドキューの生成が上手くいかなかったので起動できない
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region commandAllocator
	//コマンドアロケーターを起動する
	ID3D12CommandAllocator* commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドアロケーターの生成が上手くいかなかったので起動できない
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region CommandList
	//コマンドリストを生成する
	ID3D12GraphicsCommandList* commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	//コマンドリストの生成が上手くいかなかったので起動できない
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region スワップチェインの生成
	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kClienWidth;//画面の幅　ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Height = kClientHeight;//画面の高さ　ウィンドウのクライアント町域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//
	swapChainDesc.SampleDesc.Count = 1;//色の形式
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//マルチサンプルしない
	swapChainDesc.BufferCount = 2;//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタに写したら中身を破壊
	//コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));//
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region DescriptorHEAPの生成
	ID3D12DescriptorHeap* rtvDescriptorHeap = CreateDescropterHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	ID3D12DescriptorHeap* srvDescriptorHeap = CreateDescropterHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	//DSV用のヒープでディスクリプタの数は１。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	ID3D12DescriptorHeap* dsvDescriptorHeap = CreateDescropterHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
	//ID3D12DescriptorHeap* dsvDescriptorHeap = CreateDescropterHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
	

#pragma endregion

	//スワップチェインからリソースを引っ張ってくる
	ID3D12Resource* swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//上手く取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));


#pragma region DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;//0から始まる
	descriptorRange[0].NumDescriptors = 1;//数は１つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//offsetを自動計算
#pragma endregion

	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//RTVを2つ作るのでディスクリプタを２つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//１つ目、最初のところに作る。作る場所を指定してあげる必要がある
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);
	//２つ目のディスクリプタハンドルを作る
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//２つめを作る
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);

	//初期値０でFenceを作成
	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));
	//FenceのSignalを待つためのイベントを作成する

	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);

	//dxcCompilerを初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));
	//現時点でincludeはしないが、includeに対応するための設定を行っておく
	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));


#pragma region RootParameter
	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//RootParamater作成。複数設定できるので配列。今回は結果が１つだけなので長さ１の配列
	//2-2-6
	D3D12_ROOT_PARAMETER rootParamaters[4] = {};
	rootParamaters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParamaters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParamaters[0].Descriptor.ShaderRegister = 0;//レジスタ番号０とバインド
	rootParamaters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//
	rootParamaters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//
	rootParamaters[1].Descriptor.ShaderRegister = 0;//
	rootParamaters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableを使う
	rootParamaters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParamaters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
	rootParamaters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//Tableで利用する数
	rootParamaters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//CBVで使う
	rootParamaters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	rootParamaters[3].Descriptor.ShaderRegister = 1;//レジスタ番号１を使う
	descriptionRootSignature.pParameters = rootParamaters;//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParamaters);//
#pragma endregion


	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0～1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0;//レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


	//
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//
	ID3D12RootSignature* rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;


	D3D12_INPUT_LAYOUT_DESC inputLayoutDescs{};
	inputLayoutDescs.pInputElementDescs = inputElementDescs;
	inputLayoutDescs.NumElements = _countof(inputElementDescs);

	//
	D3D12_BLEND_DESC blendDesc{};
	//
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	//
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//
	IDxcBlob* vertexShaderBlob = CompileShader(L"Object3D.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = CompileShader(L"Object3D.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);


	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効かする
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;




	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature;//
	graphicsPipelineStateDesc.InputLayout = inputLayoutDescs;//
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };//
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };//
	graphicsPipelineStateDesc.BlendState = blendDesc;//
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//
	//
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//
	//DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));

	assert(SUCCEEDED(hr));

	const uint32_t kSubdivision = 64;		//分割数 16or32
	Transform uvTransformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};
	ID3D12Resource* vertexResource = CreateBufferResource(device, sizeof(VertexData) * kSubdivision * kSubdivision * 6);
	//マテリアル用のリソースをつくる今回はcolor1つ分のサイズを用意する
	ID3D12Resource* materialResource = CreateBufferResource(device, sizeof(Material));
	//マテリアルデータに書き込む
	Material* materialDate = nullptr;
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDate));
	//今回は赤を書き込む
	materialDate->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDate->enableLighting = true;
	materialDate->uvTransform = MakeIdentity4x4();



	//Sprite用のマテリアルリソースを作る
	ID3D12Resource* materialResourceSprite = CreateBufferResource(device, sizeof(Material));
	Material* materialDataSprite = nullptr;
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDataSprite->enableLighting = false;
	materialDataSprite->uvTransform = MakeIdentity4x4();
	ID3D12Resource* directionalLightResource = CreateBufferResource(device, sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;


	//
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//
	vertexBufferView.SizeInBytes = sizeof(VertexData) * kSubdivision * kSubdivision * 6;
	//
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	//頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	//2-2-8
	ID3D12Resource* wvpResource = CreateBufferResource(device, sizeof(TransformationMatrix));
	//
	TransformationMatrix* wvpData = nullptr;
	//
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();

	//
	D3D12_VIEWPORT viewport{};
	viewport.Width = kClienWidth;
	viewport.Height = kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//
	D3D12_RECT scissorRect{};
	//
	scissorRect.left = 0;
	scissorRect.right = kClienWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;


	Transform transform{ { 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f  } };
	Transform cameraTransform{ { 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -5.0f } };



#pragma region エラーが出たらこいつに要注意
	//Textureを読んで転送する
	DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	ID3D12Resource* textureResource = CreateTextureResource(device, metadata);
	ID3D12Resource* intermediateResource = UploadTextureData(textureResource, mipImages, device, commandList);

#pragma endregion
	//Imguiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(device, swapChainDesc.BufferCount, rtvDesc.Format, srvDescriptorHeap,
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());




	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	//先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの生成  シェーダーリソースビュー
	device->CreateShaderResourceView(textureResource, &srvDesc, textureSrvHandleCPU);



//#pragma region スフィアの表示
//	//Sphereの頂点情報//////////////////////
//	const float kLatEvery = std::numbers::pi_v<float> / float(kSubdivision);			//緯度分割１つ分の角度
//	const float kLonEvery = (std::numbers::pi_v<float>*2.0f) / float(kSubdivision);	//経度分割１つ分の角度	//緯度の方向に分割　-π/2~ π/2
//
//	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
//		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;//θ
//		//経度の方向に分割しながら線を描く
//		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
//
//			float u = float(lonIndex) / float(kSubdivision);
//			float v = 1.0f - float(latIndex) / float(kSubdivision);
//
//
//			uint32_t Index = (latIndex * kSubdivision + lonIndex) * 6;
//			float lon = lonIndex * kLonEvery;//φ
//
//			//頂点にデータを入力する。
//			//A
//			vertexData[Index].position.x = cos(lat) * cos(lon);
//			vertexData[Index].position.y = sin(lat);
//			vertexData[Index].position.z = cos(lat) * sin(lon);
//			vertexData[Index].position.w = 1.0f;
//			vertexData[Index].texcoord = { float(lonIndex) / float(kSubdivision) , 1.0f - float(latIndex) / float(kSubdivision) };
//
//			vertexData[Index].normal.x = vertexData[Index].position.x;
//			vertexData[Index].normal.y = vertexData[Index].position.y;
//			vertexData[Index].normal.z = vertexData[Index].position.z;
//
//			//B
//			vertexData[Index + 1].position.x = cos(lat + kLatEvery) * cos(lon);
//			vertexData[Index + 1].position.y = sin(lat + kLatEvery);
//			vertexData[Index + 1].position.z = cos(lat + kLatEvery) * sin(lon);
//			vertexData[Index + 1].position.w = 1.0f;
//			vertexData[Index + 1].texcoord = { float(lonIndex) / float(kSubdivision) , 1.0f - float(latIndex + 1) / float(kSubdivision) };
//
//			vertexData[Index + 1].normal.x = vertexData[Index + 1].position.x;
//			vertexData[Index + 1].normal.y = vertexData[Index + 1].position.y;
//			vertexData[Index + 1].normal.z = vertexData[Index + 1].position.z;
//
//			//C
//			vertexData[Index + 2].position.x = cos(lat) * cos(lon + kLonEvery);
//			vertexData[Index + 2].position.y = sin(lat);
//			vertexData[Index + 2].position.z = cos(lat) * sin(lon + kLonEvery);
//			vertexData[Index + 2].position.w = 1.0f;
//			vertexData[Index + 2].texcoord = { float(lonIndex + 1) / float(kSubdivision) , 1.0f - float(latIndex) / float(kSubdivision) };
//
//			vertexData[Index + 2].normal.x = vertexData[Index + 2].position.x;
//			vertexData[Index + 2].normal.y = vertexData[Index + 2].position.y;
//			vertexData[Index + 2].normal.z = vertexData[Index + 2].position.z;
//
//			//C-2
//			vertexData[Index + 3] = vertexData[Index + 2];
//
//			vertexData[Index + 3].normal.x = vertexData[Index + 3].position.x;
//			vertexData[Index + 3].normal.y = vertexData[Index + 3].position.y;
//			vertexData[Index + 3].normal.z = vertexData[Index + 3].position.z;
//
//			//B-2
//			vertexData[Index + 4] = vertexData[Index + 1];
//
//			vertexData[Index + 4].normal.x = vertexData[Index + 4].position.x;
//			vertexData[Index + 4].normal.y = vertexData[Index + 4].position.y;
//			vertexData[Index + 4].normal.z = vertexData[Index + 4].position.z;
//
//
//			//D
//			vertexData[Index + 5].position.x = cos(lat + kLatEvery) * cos(lon + kLonEvery);
//			vertexData[Index + 5].position.y = sin(lat + kLatEvery);
//			vertexData[Index + 5].position.z = cos(lat + kLatEvery) * sin(lon + kLonEvery);
//			vertexData[Index + 5].position.w = 1.0f;
//			vertexData[Index + 5].texcoord = { float(lonIndex + 1) / float(kSubdivision) , 1.0f - float((latIndex + 1) / float(kSubdivision)) };
//
//			vertexData[Index + 5].normal.x = vertexData[Index + 5].position.x;
//			vertexData[Index + 5].normal.y = vertexData[Index + 5].position.y;
//			vertexData[Index + 5].normal.z = vertexData[Index + 5].position.z;
//		}
//	}
//#pragma endregion
	
	//モデル読み込み
	ModelData modelData = LoadObjFile("resources", "plane.obj");
	//頂点リソースを作る
	ID3D12Resource* vertexResource = CreateBufferResource(device, sizeof(VertexData) * modelData.vertices.size());
	//頂点バッファービューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();//リソースの先頭のアドレスから使う
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());//使用するリソースのサイズは頂点のサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);//1頂点当たりのサイズ

	//頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(vertexData));//書き込むためのアドレスを取得
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData)* modelData.vertices.size());//頂点データをリソースにコピー


	//DepthStencilTextureをウィンドウのサイズで作成
	ID3D12Resource* depthStencilResource = CreateDepthStencilTextureResource(device, kClientWidth, kClientHeigth);


	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//Format.基本的にはResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2dTexture
	//DSVHeapの先頭にDSVをつくる
	device->CreateDepthStencilView(depthStencilResource, &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());


	//Sprite用の頂点リソースをつくる
	ID3D12Resource* vertexResourceSprite = CreateBufferResource(device, sizeof(VertexData) * 6);

	//頂点バッファービューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	//リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点６個部宇野サイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	//頂点当たりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));


	/////////////////////////////////////////////////////////////
	//Indexのあれやこれや
	ID3D12Resource* indexResourceSprite = CreateBufferResource(device, sizeof(uint32_t) * 6);
	uint32_t* indexSpriteData = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(indexSpriteData));

	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	//リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス６つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;
	//インデックスリソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
	indexDataSprite[3] = 1; indexDataSprite[4] = 3; indexDataSprite[5] = 2;

	/////////////////////////////////////////////////////////////


	vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[4].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[5].normal = { 0.0f,0.0f,-1.0f };
	//１枚目の三角形
	vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };//abc bdc
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	//２枚目の三角形
	vertexDataSprite[3].position = vertexDataSprite[1].position;
	vertexDataSprite[3].texcoord = vertexDataSprite[1].texcoord;
	vertexDataSprite[4].position = { 640.0f,0.0f,0.0f,1.0f };
	vertexDataSprite[4].texcoord = { 1.0f,0.0f };
	vertexDataSprite[5].position = vertexDataSprite[2].position;
	vertexDataSprite[5].texcoord = vertexDataSprite[2].texcoord;


	//Sprite用のTrandformaitionMatrix用のリソースをつくる。Matrix4x4 １つ分のサイズを用意する
	ID3D12Resource* transformationMatrixResourceSprite = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//単位行列を書き込んでおく
	transformationMatrixDataSprite->WVP = MakeIdentity4x4();
	transformationMatrixDataSprite->World = MakeIdentity4x4();

	//CPUで動かす用のTransformをつくる
	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	// DSVHeapの先頭にDSVをつくる
	device->CreateDepthStencilView(depthStencilResource, &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();



	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	//2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages2 = LoadTexture("resources/monsterBall.png");
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	ID3D12Resource* textureResource2 = CreateTextureResource(device, metadata2);
	ID3D12Resource* intermediateResource2 = UploadTextureData(textureResource2, mipImages2, device, commandList);

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);

	device->CreateShaderResourceView(textureResource2, &srvDesc2, textureSrvHandleCPU2);

	bool useMonsterBall = true;


	MSG msg{};
	//ウィンドウの×ボタンが押されるまでループ
	while (msg.message != WM_QUIT) {

		//Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//ImGui始まるよ
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			//ゲームの処理

			//三角形の回転
			transform.rotate.y += 0.03f;//ここコメントアウトすると止まるよ

			//座標変換
			Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrix = Inverse(cameraMatrix);
			Matrix4x4 projectionMatirx = MakePerspectiveFovMatrix(0.45f, float(kClienWidth) / float(kClientHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProhection = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatirx));
			wvpData->WVP = worldViewProhection;
			wvpData->World = worldMatrix;

			//Sprite用のWorldViewMatrixをつくる
			Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
			Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeigth), 0.0f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
			transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
			transformationMatrixDataSprite->World = worldMatrixSprite;

			Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			materialDataSprite->uvTransform = uvTransformMatrix;



			ImGui::Begin("SetColor");
			ImGui::ColorEdit4("*materialData", &materialDate->color.x);
			ImGui::DragFloat3("*scale", &transform.scale.x);//InputFloatだと直入力のみ有効
			ImGui::DragFloat3("*rotate", &transform.rotate.x);//DragFloatにすればカーソルでも値を変更できる
			ImGui::DragFloat3("*translate", &transform.translate.x);
			ImGui::DragFloat3("*shadow", &directionalLightData->direction.x, 0.01f, -1.0f, 1.0f);
			ImGui::Checkbox("useMonsterBall", &useMonsterBall);
			ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
			ImGui::End();
			ImGui::Render();



			// ここから書き込むバックバッファのインデックスを取得
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();


			// TransitionBarrier の設定
			D3D12_RESOURCE_BARRIER barrier{};
			// 今回のバリアは Transition
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			// Noneにしておく
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			// バリアを張る対象のリソース。現在のバックバッファに対して行う
			barrier.Transition.pResource = swapChainResources[backBufferIndex];
			// 遷移前（現在）の ResourceStart
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			// 遷移後の ResourceStart
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			// TransitionBarrier を張る
			commandList->ResourceBarrier(1, &barrier);


			// 描画先のRTVとDSVを指定する
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
			// 指定した色で画面全体をクリアする
			float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };  // 青っぽい色。RGBAの順
			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
			// 指定した深度で画面全体をクリアする
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


			// 描画用のDescriptorHeapの設定
			ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
			commandList->SetDescriptorHeaps(1, descriptorHeaps);

			commandList->RSSetViewports(1, &viewport);   // Viewportを設定
			commandList->RSSetScissorRects(1, &scissorRect);   // Scissorを設定
			// RootSignatureを設定。PSOに設定しているけど別途設定が必要
			commandList->SetGraphicsRootSignature(rootSignature);
			commandList->SetPipelineState(graphicsPipelineState);  // PSOを設定
			commandList->IASetVertexBuffers(0, 1, &vertexBufferView);  // VBVを設定
			// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// マテリアルCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
			// wvp用のCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

			// SRVのDescriptorTableの先頭を設定。2はrootPrameter[2]である。
			commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
			commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
			// 描画！（DrawCall/ドローコール）。3頂点で1つのインスタンス。インスタンスについては今後
			//commandList->DrawInstanced(kSubdivision * kSubdivision * 6, 1, 0, 0);
			commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

			commandList->IASetIndexBuffer(&indexBufferViewSprite);//IBVを設定
			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);  // VBVを設定
			// マテリアルCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
			// TransformationMatrixCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
			// 描画！（DrawCall/ドローコール）
			//commandList->DrawInstanced(6, 1, 0, 0);
			//commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);



			// 実際のcommandListのImGuiの描画コマンドを積む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);


			// 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
			// 今回は RenderTarget から Present にする
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			// TransitionBarrier をは張る
			commandList->ResourceBarrier(1, &barrier);


			// コマンドリストの内容を確定させる。すべてのコマンドを積んでから Close すること
			hr = commandList->Close();
			assert(SUCCEEDED(hr));

			//GPUにコマンドリストの実行を行わせる
			ID3D12CommandList* commandLists[] = { commandList };
			commandQueue->ExecuteCommandLists(1, commandLists);
			//GPUとOSに画面の交渉を行うように通知する
			swapChain->Present(1, 0);
			//Fenceの値を更新
			fenceValue++;
			//GPUがここまでたどり着いた時に、Fenceの値を指定した値に代入するようにSignalを送る
			commandQueue->Signal(fence, fenceValue);
			//Fenceの値が指定したSignal値にたどり着いているか確認する
			//GetCompletedValueの初期値はFence作成時に渡した初期値
			if (fence->GetCompletedValue() < fenceValue) {
				//指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
				fence->SetEventOnCompletion(fenceValue, fenceEvent);
				//イベントを待つ
				WaitForSingleObject(fenceEvent, INFINITE);
			}
			//次のフレーム用のコマンドリストを準備
			hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator, nullptr);
			assert(SUCCEEDED(hr));


		}
	}
#pragma region  解放処理
	CoUninitialize();
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	CloseHandle(fenceEvent);
	fence->Release();
	rtvDescriptorHeap->Release();
	srvDescriptorHeap->Release();
	swapChainResources[0]->Release();
	swapChainResources[1]->Release();
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	device->Release();
	useAdapter->Release();
	dxgiFactory->Release();

	vertexResource->Release();
	graphicsPipelineState->Release();
	signatureBlob->Release();
	if (errorBlob) {
		errorBlob->Release();
	}
	rootSignature->Release();
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();
	materialResource->Release();
	wvpResource->Release();
	textureResource->Release();
	intermediateResource->Release();
	depthStencilResource->Release();
	dsvDescriptorHeap->Release();
	vertexResourceSprite->Release();
	transformationMatrixResourceSprite->Release();
	textureResource2->Release();
	intermediateResource2->Release();
	materialResourceSprite->Release();
	directionalLightResource->Release();
	indexResourceSprite->Release();
#pragma endregion

#ifdef _DEBUG
	debugController->Release();

#endif
	CloseWindow(hwnd);

	//出力ウィンドウへの文字出力　実行すると出る下の文字
	OutputDebugStringA("Hello,DirectX!\n");
	//リソースリークチェック
	IDXGIDebug1* debug;
	//ここでの例外エラーはリリース忘れだからリリースしろばか！
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
		//ここでの例外エラーはリリース忘れだからリリースしろ
	}
	return 0;
}