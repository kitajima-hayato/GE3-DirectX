#include "ParticleManager.h"
#include <Logger.h>
#include <MyMath.h>
#include <Framework.cpp>

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
	// NULL検出 // メンバ変数に代入
	assert(dxCommon);
	this->dxCommon = dxCommon;
	assert(srvManager);
	this->srvManager = srvManager;

	// ランダムエンジンの初期化
	std::random_device seedGenaerator;
	std::mt19937 randomEngine(seedGenaerator());

	CreateGraphicsPipeLine();
	// 頂点データの初期化(座標等)
	InitializeVertexData();
	// 頂点リソース生成
	CreateVertexResource();
	// 頂点バッファの生成
	CreateVertexBufferView();
	// インデックスバッファの生成
	CreateVertexBufferView();
	// 頂点リソースに頂点データを書き込む
	WriteVertexResource();
}





void ParticleManager::CreateGraphicsPipeLine()
{
	// ルートシグネチャの作成
	CreateRootSignature();

#pragma region GraphicsPipelineState

	// Particle用のPSOを生成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC particlePipelineStateDesc{};
	particlePipelineStateDesc.pRootSignature = rootSignature.Get();
	particlePipelineStateDesc.InputLayout = inputLayoutDescs;
	particlePipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	particlePipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	particlePipelineStateDesc.BlendState = blendDesc;
	particlePipelineStateDesc.RasterizerState = rasterizerDesc;
	particlePipelineStateDesc.DepthStencilState = depthStencilDesc;
	particlePipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	particlePipelineStateDesc.NumRenderTargets = 1;
	particlePipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	particlePipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	particlePipelineStateDesc.SampleDesc.Count = 1;
	particlePipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// PSOの生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> particlePipelineState = nullptr;
	HRESULT hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&particlePipelineStateDesc, IID_PPV_ARGS(&particlePipelineState));
	assert(SUCCEEDED(hr));
#pragma endregion
}

void ParticleManager::CreateRootSignature()
{

	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
	descriptorRangeForInstancing[0].BaseShaderRegister = 0;
	// 0から始まる
	descriptorRangeForInstancing[0].NumDescriptors = 1;
	// 数は1つ 
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う 
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	//RootSignature作成  Particle
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignatureParticle{};
	descriptionRootSignatureParticle.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//RootParameter作成。複数設定できるので配列。今回は結果１つだけなので長さ１の配列
	D3D12_ROOT_PARAMETER rootParametersParticle[3] = {};
	rootParametersParticle[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	rootParametersParticle[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;		//PixelShaderで使う
	rootParametersParticle[0].Descriptor.ShaderRegister = 0;						//レジスタ番号０とバインド

	rootParametersParticle[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParametersParticle[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParametersParticle[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;
	rootParametersParticle[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);

	rootParametersParticle[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;			//DescriptorTableを使う
	rootParametersParticle[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;						//PixelShaderで使う
	rootParametersParticle[2].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;					//Tableの中身の配列を指定
	rootParametersParticle[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);		//Tableで利用する数


	descriptionRootSignatureParticle.pParameters = rootParametersParticle;					//ルートパラメータ配列へのポインタ
	descriptionRootSignatureParticle.NumParameters = _countof(rootParametersParticle);		//配列の長さ
}

void ParticleManager::InitializeVertexData()
{
	


}

void ParticleManager::CreateVertexResource()
{
	// 頂点リソース生成
	vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) *vertexData.size() );

	// 頂点バッファにデータを書き込む
	void* pVertexDataBegin;
	D3D12_RANGE readRange = {};
	HRESULT hr = vertexResource->Map(0, &readRange, &pVertexDataBegin);
	assert(SUCCEEDED(hr));
	memcpy(pVertexDataBegin, vertexData.data(), sizeof(VertexData) * vertexData.size());
	vertexResource->Unmap(0, nullptr);

}

void ParticleManager::CreateVertexBufferView()
{
	// 頂点リソースの生成
	CreateVertexResource();

	// 頂点バッファビューの作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6; // 頂点数に応じて変更

	// コマンドリストに頂点バッファを設定
	dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
}

void ParticleManager::WriteVertexResource()
{
	// 頂点リソースに頂点データを書き込む
	void* pVertexDataBegin;
	D3D12_RANGE readRange = {};
	HRESULT hr = vertexResource->Map(0, &readRange, &pVertexDataBegin);
	assert(SUCCEEDED(hr));
	memcpy(pVertexDataBegin, vertexData.data(), sizeof(VertexData) * vertexData.size());
	vertexResource->Unmap(0, nullptr);


}




