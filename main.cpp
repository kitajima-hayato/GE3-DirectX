#include<Windows.h>
#include<cstdint>
#include<string>
#include<format>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>
#include<dxgidebug.h>
#include<dxcapi.h>
#include"Math.h"
#include"MakeMatrix.h"

#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"
#include<vector>
#include<fstream>
#include<sstream>
#include<wrl.h>
#include "numbers"
#include "Input.h"
#include "WinAPI.h"
#include "DirectXCommon.h"
#include "Logger.h"

using namespace Logger;
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
using namespace std;


struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker()
	{
		//リソースリークチェック
		Microsoft::WRL::ComPtr<IDXGIDebug1>debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};







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




Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
	const std::wstring filePath,
	const wchar_t* profile,
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils,
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler,
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler
) {
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
Microsoft::WRL::ComPtr<ID3D12Resource>
CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes) {
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
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));
	return vertexResource;
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
Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata)
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
		&heapProperties, // Heapの設定
		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定,特になし
		&resourceDesc, // Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST, // データ転送される設定
		nullptr, // Clear最適値 使わないのでnullptr
		IID_PPV_ARGS(&resource) // 作成するResourceポインタへのポインタ
	);
	assert(SUCCEEDED(hr));
	return resource;
}
#pragma endregion
#pragma region テクスチャリソースにデータ転送関数
[[nodiscard]]
Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(
	Microsoft::WRL::ComPtr<ID3D12Resource> texture,
	const DirectX::ScratchImage& mipImages,
	Microsoft::WRL::ComPtr<ID3D12Device> device,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
{
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, static_cast<UINT>(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device, intermediateSize);
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

#pragma endregion










#pragma region Materialファイルの読み込み

MaterialData LoadMaterialTempLateFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData materialData;	//構築するMaterialData
	std::string line;			//ファイルから読んだ１行を格納するもの
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());		//開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;
		//identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}

#pragma endregion
#pragma region objファイルの読み込み
//objファイルの読み込み関数
ModelData LoadObjFile(const string& directoryPath, const string& filename) {
	// 1. 必要な変数宣言
	ModelData modelData;     // 構築するModelData
	vector<Vector4> positions;    // 位置
	vector<Vector3> normals;    // 法線
	vector<Vector2> texcoords;    // テクスチャ座標
	string line;       // ファイルから読んだ1行を格納

	// 2. ファイルを開く
	ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());         // 開けなかったら止める

	while (getline(file, line)) {
		string identifier;
		istringstream s(line);
		s >> identifier; // 先頭の識別子を読む

		// identifierに応じた処理
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];
			// 面は三角形限定。その他は未定。
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは　［位置/UV/法線］　で格納されているので分解してIndexを取得する
				istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					string index;
					getline(v, index, '/');      // 区切りでインデックスうぃ読んでいく
					elementIndices[element] = stoi(index);
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				triangle[faceVertex] = { position,texcoord,normal };
			}
			// 頂点を逆順すろことで、回り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			std::string materialFilename;
			s >> materialFilename;
			modelData.material = LoadMaterialTempLateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}
#pragma endregion


//ウィンドウズアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;



	//ポインタ
	WinAPI* winAPI = nullptr;
	//WindowsAPIの初期化
	winAPI = new WinAPI();
	winAPI->Initialize();

	//入力処理のクラスポインタ
	Input* input = nullptr;
	input = new Input();
	input->Initialize(winAPI);


	//ポインタと初期化
	DirectXCommon* dxCommon = nullptr;
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winAPI);




#pragma region DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;//0から始まる
	descriptorRange[0].NumDescriptors = 1;//数は１つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//offsetを自動計算
#pragma endregion

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
	Microsoft::WRL::ComPtr <ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr <ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	//hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),IID_PPV_ARGS(&rootSignature));
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

	////
	//Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = nullptr;
	//vertexShaderBlob = CompileShader(L"resources/shaders/Object3D.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	//assert(vertexShaderBlob != nullptr);

	//Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = nullptr;
	//pixelShaderBlob = CompileShader(L"resources/shaders/Object3D.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	//assert(pixelShaderBlob != nullptr);


	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;




	//	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	//	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();//
	//	graphicsPipelineStateDesc.InputLayout = inputLayoutDescs;//
	//	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };//
	//	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };//
	//	graphicsPipelineStateDesc.BlendState = blendDesc;//
	//	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//
	//	//
	//	graphicsPipelineStateDesc.NumRenderTargets = 1;
	//	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//	//
	//	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//	//
	//	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	//	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//	//
	//	//DepthStencilの設定
	//	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	//	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
	//	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	//
	//	assert(SUCCEEDED(hr));
	//
	//	const uint32_t kSubdivision = 64;		//分割数 16or32
	//	Transform uvTransformSprite{
	//		{1.0f,1.0f,1.0f},
	//		{0.0f,0.0f,0.0f},
	//		{0.0f,0.0f,0.0f},
	//	};
	//	//マテリアル用のリソースをつくる今回はcolor1つ分のサイズを用意する
	//	Microsoft::WRL::ComPtr <ID3D12Resource> materialResource = CreateBufferResource(device, sizeof(Material));
	//	//マテリアルデータに書き込む
	//	Material* materialDate = nullptr;
	//	//書き込むためのアドレスを取得
	//	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDate));
	//	//今回は赤を書き込む
	//	materialDate->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//	materialDate->enableLighting = 0;
	//	materialDate->uvTransform = MakeIdentity4x4();
	//
	//
	//
	//	//Sprite用のマテリアルリソースを作る
	//	Microsoft::WRL::ComPtr <ID3D12Resource> materialResourceSprite = CreateBufferResource(device, sizeof(Material));
	//	Material* materialDataSprite = nullptr;
	//	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	//	materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//	materialDataSprite->enableLighting = 0;
	//	materialDataSprite->uvTransform = MakeIdentity4x4();
	//	Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource = CreateBufferResource(device, sizeof(DirectionalLight));
	//	DirectionalLight* directionalLightData = nullptr;
	//	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	//	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	//	directionalLightData->intensity = 1.0f;
	//
	//
	//	//モデル読み込み
	//	ModelData modelData = LoadObjFile("resources", "Bunny.obj");
	//	Microsoft::WRL::ComPtr < ID3D12Resource> vertexResource = CreateBufferResource(device, sizeof(VertexData) * modelData.vertices.size());
	//	//頂点バッファービューを作成する
	//	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();//リソースの先頭のアドレスから使う
	//	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());//使用するリソースのサイズは頂点のサイズ
	//	vertexBufferView.StrideInBytes = sizeof(VertexData);
	//	//頂点リソースにデータを書き込む
	//	VertexData* vertexData = nullptr;
	//	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));//書き込むためのアドレスを取得
	//	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());//頂点データをリソースにコピー
	//
	//
	//
	//#pragma region スフィア用の新規作成
	//
	//	// スフィア用の新規作成
	//	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere = CreateBufferResource(device, sizeof(VertexData) * kSubdivision * kSubdivision * 6);
	//
	//	// マテリアル用のリソースを作成
	//	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSphere = CreateBufferResource(device, sizeof(Material));
	//
	//	// マテリアルデータに書き込む
	//	Material* materialDataSphere = nullptr;
	//	materialResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere));
	//	materialDataSphere->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//	materialDataSphere->enableLighting = 0;
	//	materialDataSphere->uvTransform = MakeIdentity4x4();
	//
	//	// 頂点バッファービューを作成
	//	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	//	vertexBufferViewSphere.BufferLocation = vertexResourceSphere->GetGPUVirtualAddress();
	//	vertexBufferViewSphere.SizeInBytes = sizeof(VertexData) * kSubdivision * kSubdivision * 6;
	//	vertexBufferViewSphere.StrideInBytes = sizeof(VertexData);
	//
	//	// 頂点リソースにデータを書き込む
	//	VertexData* vertexDataSphere = nullptr;
	//	vertexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSphere));
	//
	//	// WVPリソースを作成
	//	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResourceSphere = CreateBufferResource(device, sizeof(TransformationMatrix));
	//	TransformationMatrix* wvpDataSphere = nullptr;
	//	wvpResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataSphere));
	//	wvpDataSphere->WVP = MakeIdentity4x4();
	//	wvpDataSphere->World = MakeIdentity4x4();
	//
	//	Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResourceSphere = CreateBufferResource(device, sizeof(DirectionalLight));
	//	DirectionalLight* directionalLightDataSphere = nullptr;
	//	directionalLightResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightDataSphere));
	//	directionalLightDataSphere->color = { 1.0f,1.0f,1.0f,1.0f };
	//	directionalLightDataSphere->direction = { 0.0f,-1.0f,0.0f };
	//	directionalLightDataSphere->intensity = 1.0f;
	//
	//#pragma endregion
	//
	//	//2-2-8
	//	Microsoft::WRL::ComPtr < ID3D12Resource> wvpResource = CreateBufferResource(device, sizeof(TransformationMatrix));
	//	//
	//	TransformationMatrix* wvpData = nullptr;
	//	//
	//	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//	//
	//	wvpData->WVP = MakeIdentity4x4();
	//	wvpData->World = MakeIdentity4x4();
	//
	//	//
	//
	//
	//
	//
	//
	//	Transform transform{ { 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f  } };
	//	Transform transformSphere{ { 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f  } };
	//	Transform cameraTransform{ { 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -5.0f } };
	//
	//
	//
	//#pragma region エラーが出たらこいつに要注意
	//	//Textureを読んで転送する
	//	DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	//	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	//	Microsoft::WRL::ComPtr < ID3D12Resource> textureResource = CreateTextureResource(device, metadata);
	//	Microsoft::WRL::ComPtr < ID3D12Resource> intermediateResource = UploadTextureData(textureResource, mipImages, device, commandList);
	//
	//#pragma endregion
	//
	//
	//
	//
	//	//metaDataを基にSRVの設定
	//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	//	srvDesc.Format = metadata.format;
	//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	//	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	//
	//	//SRVを作成するDescriptorHeapの場所を決める
	//	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	//	//先頭はImGuiが使っているのでその次を使う
	//	textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//	textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//	//SRVの生成  シェーダーリソースビュー
	//	device->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);
	//
	//
	//
	//
	//
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
	//			vertexDataSphere[Index].position.x = cos(lat) * cos(lon);
	//			vertexDataSphere[Index].position.y = sin(lat);
	//			vertexDataSphere[Index].position.z = cos(lat) * sin(lon);
	//			vertexDataSphere[Index].position.w = 1.0f;
	//			vertexDataSphere[Index].texcoord = { float(lonIndex) / float(kSubdivision) , 1.0f - float(latIndex) / float(kSubdivision) };
	//
	//			vertexDataSphere[Index].normal.x = vertexDataSphere[Index].position.x;
	//			vertexDataSphere[Index].normal.y = vertexDataSphere[Index].position.y;
	//			vertexDataSphere[Index].normal.z = vertexDataSphere[Index].position.z;
	//
	//			//B
	//			vertexDataSphere[Index + 1].position.x = cos(lat + kLatEvery) * cos(lon);
	//			vertexDataSphere[Index + 1].position.y = sin(lat + kLatEvery);
	//			vertexDataSphere[Index + 1].position.z = cos(lat + kLatEvery) * sin(lon);
	//			vertexDataSphere[Index + 1].position.w = 1.0f;
	//			vertexDataSphere[Index + 1].texcoord = { float(lonIndex) / float(kSubdivision) , 1.0f - float(latIndex + 1) / float(kSubdivision) };
	//
	//			vertexDataSphere[Index + 1].normal.x = vertexDataSphere[Index + 1].position.x;
	//			vertexDataSphere[Index + 1].normal.y = vertexDataSphere[Index + 1].position.y;
	//			vertexDataSphere[Index + 1].normal.z = vertexDataSphere[Index + 1].position.z;
	//
	//			//C
	//			vertexDataSphere[Index + 2].position.x = cos(lat) * cos(lon + kLonEvery);
	//			vertexDataSphere[Index + 2].position.y = sin(lat);
	//			vertexDataSphere[Index + 2].position.z = cos(lat) * sin(lon + kLonEvery);
	//			vertexDataSphere[Index + 2].position.w = 1.0f;
	//			vertexDataSphere[Index + 2].texcoord = { float(lonIndex + 1) / float(kSubdivision) , 1.0f - float(latIndex) / float(kSubdivision) };
	//
	//			vertexDataSphere[Index + 2].normal.x = vertexDataSphere[Index + 2].position.x;
	//			vertexDataSphere[Index + 2].normal.y = vertexDataSphere[Index + 2].position.y;
	//			vertexDataSphere[Index + 2].normal.z = vertexDataSphere[Index + 2].position.z;
	//
	//			//C-2
	//			vertexDataSphere[Index + 3] = vertexDataSphere[Index + 2];
	//
	//			vertexDataSphere[Index + 3].normal.x = vertexDataSphere[Index + 3].position.x;
	//			vertexDataSphere[Index + 3].normal.y = vertexDataSphere[Index + 3].position.y;
	//			vertexDataSphere[Index + 3].normal.z = vertexDataSphere[Index + 3].position.z;
	//
	//			//B-2
	//			vertexDataSphere[Index + 4] = vertexDataSphere[Index + 1];
	//
	//			vertexDataSphere[Index + 4].normal.x = vertexDataSphere[Index + 4].position.x;
	//			vertexDataSphere[Index + 4].normal.y = vertexDataSphere[Index + 4].position.y;
	//			vertexDataSphere[Index + 4].normal.z = vertexDataSphere[Index + 4].position.z;
	//
	//
	//			//D
	//			vertexDataSphere[Index + 5].position.x = cos(lat + kLatEvery) * cos(lon + kLonEvery);
	//			vertexDataSphere[Index + 5].position.y = sin(lat + kLatEvery);
	//			vertexDataSphere[Index + 5].position.z = cos(lat + kLatEvery) * sin(lon + kLonEvery);
	//			vertexDataSphere[Index + 5].position.w = 1.0f;
	//			vertexDataSphere[Index + 5].texcoord = { float(lonIndex + 1) / float(kSubdivision) , 1.0f - float((latIndex + 1) / float(kSubdivision)) };
	//
	//			vertexDataSphere[Index + 5].normal.x = vertexDataSphere[Index + 5].position.x;
	//			vertexDataSphere[Index + 5].normal.y = vertexDataSphere[Index + 5].position.y;
	//			vertexDataSphere[Index + 5].normal.z = vertexDataSphere[Index + 5].position.z;
	//		}
	//	}
	//#pragma endregion
	//
	//
	//	//DepthStencilTextureをウィンドウのサイズで作成
	//	Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device, WinAPI::kClientWidth, WinAPI::kClientHeight);
	//
	//
	//
	//
	//	//Sprite用の頂点リソースをつくる
	//	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceSprite = CreateBufferResource(device, sizeof(VertexData) * 6);
	//
	//	//頂点バッファービューを作成する
	//	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	//	//リソースの先頭のアドレスから使う
	//	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//	//使用するリソースのサイズは頂点６個部宇野サイズ
	//	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	//	//頂点当たりのサイズ
	//	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);
	//
	//	VertexData* vertexDataSprite = nullptr;
	//	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	//
	//
	//	/////////////////////////////////////////////////////////////
	//	//Indexのあれやこれや
	//	Microsoft::WRL::ComPtr <ID3D12Resource> indexResourceSprite = CreateBufferResource(device, sizeof(uint32_t) * 6);
	//	uint32_t* indexSpriteData = nullptr;
	//	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(indexSpriteData));
	//
	//	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	//	//リソースの先頭のアドレスから使う
	//	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	//	//使用するリソースのサイズはインデックス６つ分のサイズ
	//	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	//	//インデックスはuint32_tとする
	//	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;
	//	//インデックスリソースにデータを書き込む
	//	uint32_t* indexDataSprite = nullptr;
	//	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	//	indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
	//	indexDataSprite[3] = 1; indexDataSprite[4] = 3; indexDataSprite[5] = 2;
	//
	//	/////////////////////////////////////////////////////////////
	//
	//
	//	vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	//	vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	//	vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };
	//	vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };
	//	vertexDataSprite[4].normal = { 0.0f,0.0f,-1.0f };
	//	vertexDataSprite[5].normal = { 0.0f,0.0f,-1.0f };
	//	//１枚目の三角形
	//	vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };//abc bdc
	//	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	//	vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };
	//	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	//	vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };
	//	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	//	//２枚目の三角形
	//	vertexDataSprite[3].position = vertexDataSprite[1].position;
	//	vertexDataSprite[3].texcoord = vertexDataSprite[1].texcoord;
	//	vertexDataSprite[4].position = { 640.0f,0.0f,0.0f,1.0f };
	//	vertexDataSprite[4].texcoord = { 1.0f,0.0f };
	//	vertexDataSprite[5].position = vertexDataSprite[2].position;
	//	vertexDataSprite[5].texcoord = vertexDataSprite[2].texcoord;
	//
	//
	//	//Sprite用のTrandformaitionMatrix用のリソースをつくる。Matrix4x4 １つ分のサイズを用意する
	//	Microsoft::WRL::ComPtr <ID3D12Resource>
	//		transformationMatrixResourceSprite = CreateBufferResource(device, sizeof(TransformationMatrix));
	//	//データを書き込む
	//	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	//	//書き込むためのアドレスを取得
	//	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//	//単位行列を書き込んでおく
	//	transformationMatrixDataSprite->WVP = MakeIdentity4x4();
	//	transformationMatrixDataSprite->World = MakeIdentity4x4();
	//
	//	//CPUで動かす用のTransformをつくる
	//	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	//
	//	// DSVHeapの先頭にDSVをつくる
	//	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	//
	//
	//
	//	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	//
	//
	//	//2枚目のTextureを読んで転送する
	//	DirectX::ScratchImage mipImages2 = LoadTexture(modelData.material.textureFilePath);
	//	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	//	Microsoft::WRL::ComPtr <ID3D12Resource> textureResource2 = CreateTextureResource(device, metadata2);
	//	Microsoft::WRL::ComPtr <ID3D12Resource> intermediateResource2 = UploadTextureData(textureResource2, mipImages2, device, commandList);
	//
	//	//metaDataを基にSRVの設定
	//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	//	srvDesc2.Format = metadata2.format;
	//	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	//	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);
	//
	//	//SRVを作成するDescriptorHeapの場所を決める
	//	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
	//	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
	//
	//	device->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);
	//
	//	bool useMonsterBall = false;
	//





		//ウィンドウの×ボタンが押されるまでループ
	while (true) {		// ゲームループ
		//Windowのメッセージ処理
		if (winAPI->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}
		//ImGui始まるよ
		/*ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();*/
		//ゲームの処理
		input->Update();


		if (input->TriggerKey(DIK_1)) {
			OutputDebugStringA("Hit_1\n");
		}

		//		//三角形の回転
		//		//transform.rotate.y += 0.03f;//ここコメントアウトすると止まるよ
		//		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		//		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		//
		//		//座標変換<obj>
		//		Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		//		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinAPI::kClientWidth) / float(WinAPI::kClientHeight), 0.1f, 100.0f);
		//		Matrix4x4 worldViewProjection = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		//		wvpData->WVP = worldViewProjection;
		//		wvpData->World = worldMatrix;
		//		//座標変換<sphere>
		//		Matrix4x4 worldMatrixSphere = MakeAffineMatrix(transformSphere.scale, transformSphere.rotate, transformSphere.translate);
		//		Matrix4x4 projectionMatrixSphere = MakePerspectiveFovMatrix(0.45f, float(WinAPI::kClientWidth) / float(WinAPI::kClientHeight), 0.1f, 100.0f);
		//		Matrix4x4 worldViewProjectionSphere = Multiply(worldMatrixSphere, Multiply(viewMatrix, projectionMatrixSphere));
		//		wvpDataSphere->WVP = worldViewProjectionSphere;
		//		wvpDataSphere->World = worldMatrixSphere;
		//		//座標変換<sprite>
		//		Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		//		Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
		//		Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(WinAPI::kClientWidth), float(WinAPI::kClientHeight), 0.0f, 100.0f);
		//		Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
		//		transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
		//		transformationMatrixDataSprite->World = worldMatrixSprite;
		//		//座標変換<UV>
		//		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
		//		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
		//		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
		//		materialDataSprite->uvTransform = uvTransformMatrix;
		//
		//
		//
		//
		//		ImGui::Begin("Settings");
		//		if (ImGui::BeginTabBar("OBJ"))
		//		{
		//			// Objの値変更
		//			if (ImGui::BeginTabItem("OBJ"))
		//			{
		//				ImGui::ColorEdit4("*ObjColor", &materialDate->color.x);
		//				ImGui::DragFloat3("*ObjScale", &transform.scale.x, 0.01f);//InputFloatだと直入力のみ有効
		//				ImGui::DragFloat3("*ObjRotate", &transform.rotate.x, 0.01f);//DragFloatにすればカーソルでも値を変更できる
		//				ImGui::DragFloat3("*ObjTranslate", &transform.translate.x, 0.01f);
		//				ImGui::DragFloat3("*shadow", &directionalLightData->direction.x, 0.01f, -1.0f, 1.0f);
		//				if (ImGui::Button("*Lighting")) {
		//					if (materialDate->enableLighting) {
		//						materialDate->enableLighting = 0;
		//					}
		//					else if (!materialDate->enableLighting) {
		//						materialDate->enableLighting = 1;
		//					}
		//				}if (ImGui::Button("*HalfLambert")) {
		//					if (materialDate->enableLighting) {
		//						materialDate->enableLighting = 2;
		//					}
		//				}
		//				ImGui::EndTabItem();
		//			}
		//			//UVの値変更
		//			if (ImGui::BeginTabItem("UV"))
		//			{
		//				ImGui::DragFloat2("*UVPositionScale", &transformSprite.scale.x, 0.1f);
		//				ImGui::DragFloat2("*UVPositionRotate", &transformSprite.rotate.x, 0.1f);
		//				ImGui::DragFloat2("*UVPositionTranslate", &transformSprite.translate.x, 0.5f);
		//				ImGui::DragFloat2("*UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		//				ImGui::DragFloat2("*UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		//				ImGui::SliderAngle("*UVRotate", &uvTransformSprite.rotate.z, 0.01f);
		//				ImGui::EndTabItem();
		//			}
		//			//Sphereの値変更
		//			if (ImGui::BeginTabItem("Sphere"))
		//			{
		//				ImGui::ColorEdit4("*color", &materialDataSphere->color.x);
		//				ImGui::DragFloat3("*scale", &transformSphere.scale.x, 0.01f);//InputFloatだと直入力のみ有効
		//				ImGui::DragFloat3("*rotate", &transformSphere.rotate.x, 0.01f);//DragFloatにすればカーソルでも値を変更できる
		//				ImGui::DragFloat3("*translate", &transformSphere.translate.x, 0.01f);
		//				ImGui::DragFloat3("*shadow", &directionalLightDataSphere->direction.x, 0.01f, -1.0f, 1.0f);
		//				if (ImGui::Button("*Lighting")) {
		//					if (materialDataSphere->enableLighting) {
		//						materialDataSphere->enableLighting = 0;
		//					}
		//					else if (!materialDataSphere->enableLighting) {
		//						materialDataSphere->enableLighting = 1;
		//					}
		//				}
		//				if (ImGui::Button("*HalfLambert")) {
		//					if (materialDataSphere->enableLighting) {
		//						materialDataSphere->enableLighting = 2;
		//					}
		//
		//				}
		//				ImGui::EndTabItem();
		//			}
		//			ImGui::EndTabItem();
		//		}


//		ImGui::End();
//		ImGui::Render();





		dxCommon->PreDraw();







		//		// RootSignatureを設定。PSOに設定しているけど別途設定が必要
		//		commandList->SetGraphicsRootSignature(rootSignature.Get());
		//		commandList->SetPipelineState(graphicsPipelineState.Get());  // PSOを設定
		//
		//		// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
		//		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//
		//		// スフィア用の設定
		//		commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSphere);  // VBVを設定
		//		commandList->SetGraphicsRootConstantBufferView(0, materialResourceSphere->GetGPUVirtualAddress());
		//		commandList->SetGraphicsRootConstantBufferView(1, wvpResourceSphere->GetGPUVirtualAddress());
		//		commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
		//		commandList->SetGraphicsRootConstantBufferView(3, directionalLightResourceSphere->GetGPUVirtualAddress());
		//		commandList->DrawInstanced(kSubdivision * kSubdivision * 6, 1, 0, 0);
		//
		//
		//		// モデル用の設定 
		//		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);  // VBVを設定
		//		commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		//		commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
		//		commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		//		commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
		//
		//
		//		// スプライト用の設定
		//		commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		//		commandList->IASetIndexBuffer(&indexBufferViewSprite);
		//		commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
		//		commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
		//		commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
		//		commandList->DrawInstanced(6, 1, 0, 0);
		//		commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
		//
		//		// 実際のcommandListのImGuiの描画コマンドを積む
		//		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
		//








		dxCommon->PostDraw();








	}
#pragma region  解放処理

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//CloseHandle(fenceEvent);

	winAPI->Finalize();

#pragma endregion



	////出力ウィンドウへの文字出力　実行すると出る下の文字
	//OutputDebugStringA("Hello,DirectX!\n");

	//解放
	delete input;
	delete winAPI;
	delete dxCommon;
	return 0;
}