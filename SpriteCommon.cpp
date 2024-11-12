#include "SpriteCommon.h"
#include "Logger.h"
#include "Math.h"

void SpriteCommon::Initialize(DirectXCommon* dxCommon)
{
	//引数で受け取ってメンバ変数に記録する
	dxCommon_ = dxCommon;
	CreateGraficsPipeLine();
}

void SpriteCommon::CreateRootSignatrue()
{
#pragma region RootParameter
	//RootSignature作成
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

#pragma region DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;//0から始まる
	descriptorRange[0].NumDescriptors = 1;//数は１つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//offsetを自動計算
#pragma endregion
	
	//RootParamater作成。複数設定できるので配列。今回は結果が１つだけなので長さ１の配列
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

	// ルートシグネチャのシリアライズ結果を格納するBlob
	Microsoft::WRL::ComPtr <ID3DBlob> signatureBlob = nullptr;
	// エラーメッセージが発生した場合に格納するBlob
	Microsoft::WRL::ComPtr <ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
	
}

void SpriteCommon::InitializePixelShaderOutput()
{
	
	
}

void SpriteCommon::DrawSettingCommon()
{
	
	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());  // PSOを設定

	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}


void SpriteCommon::CreateGraficsPipeLine()
{
	// ルートシグネチャの作成
	CreateRootSignatrue();

	// 頂点の位置データを表すセマンティクスを設定
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	// テクスチャ座標データを表すセマンティクスの座標
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	// 法線データを表すセマンティクスを設定
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputLayoutDescs.pInputElementDescs = inputElementDescs;
	inputLayoutDescs.NumElements = _countof(inputElementDescs);

	// レンダーターゲットの書き込みマスクを設定。全ての色チャンネルに書き込みを許可
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;


	// カリングモードを設定
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// ポリゴンのフィルモードを設定。ポリゴンの塗り潰し
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//　頂点シェーダーのコンパイル結果を格納するBlob
	vertexShaderBlob = dxCommon_->CompileShader(
		L"resources/shaders/Object3D.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	// ピクセルシェーダーのコンパイル結果を格納するBlob
	pixelShaderBlob = dxCommon_->CompileShader(
		L"resources/shaders/Object3D.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);


	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


	


	// グラフィックスパイプラインのステートオブジェクトのデスクリプタを設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	// インプットレイアウトの設定
	graphicsPipelineStateDesc.InputLayout = inputLayoutDescs;
	// シェーダーの設定
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };//
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };//
	// ブレンドステートの設定
	graphicsPipelineStateDesc.BlendState = blendDesc;
	// ラスタライザの設定
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	// レンダーターゲットの設定
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// プリミティブとポロ時の設定
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
	HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));

	assert(SUCCEEDED(hr));
}



Microsoft::WRL::ComPtr<ID3D12Resource>
SpriteCommon::CreateSpriteVertexResource()
{
	vertexResource = nullptr;
	vertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * 6);
	return vertexResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource>
SpriteCommon::CreateSpriteIndexResource()
{
	indexResource = nullptr;
	indexResource = dxCommon_->CreateBufferResource(sizeof(uint32_t) * 6);
	return indexResource;
}
