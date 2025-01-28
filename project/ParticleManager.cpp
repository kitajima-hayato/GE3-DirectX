#include "ParticleManager.h"
#include <Logger.h>
#include "MakeMatrix.h"
#include <TextureManager.h>
#include "WinAPI.h"
#include <numbers>
ParticleManager* ParticleManager::instance = nullptr;
ParticleManager* ParticleManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new ParticleManager();
	}
	return instance;

}

void ParticleManager::DeleteInstance()
{
	if (instance != nullptr)
	{
		delete instance;
		instance = nullptr;
	}
}

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, Camera* camera)
{
	// メンバ変数の初期化
	this->dxCommon = dxCommon;
	this->srvManager = srvManager;
	// カメラの取得
	this->camera = camera;

	// ブレンドモードの設定
	blendMode = BlendMode::kBlendModeAdd;
	// ランダムエンジンの初期化
	InitializeRandomEngine();
	// パイプラインの生成
	CreatePipeline();
	// 頂点データの初期化(座標等)
	InitializeVertexData();
	// マテリアルの初期化
	InitializeMaterial();
}

void ParticleManager::InitializeRandomEngine()
{
	// ランダムエンジンの初期化
	std::random_device rnd;
	randomEngine = std::mt19937(rnd());
}

void ParticleManager::CreatePipeline()
{
	// ルートシグネチャの作成
	CreateRootSignature();
	// グラフィックスパイプラインの設定
	SetGraphicsPipeline();
}

void ParticleManager::CreateRootSignature()
{
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始まる
	descriptorRange[0].NumDescriptors = 1; // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRV
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算


	// 1. RootSignatureの作成

	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	// RootParameter作成。複数設定できるので配列。
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// 1. パーティクルのRootSignatureの作成
	descriptorRangeInstancing[0].BaseShaderRegister = 0;
	descriptorRangeInstancing[0].NumDescriptors = 1;
	descriptorRangeInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeInstancing;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeInstancing);

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);


	// Samplerの設定
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;							// バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;						// 0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;						// 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;										// ありったけのMipmapｗｐ使う
	staticSamplers[0].ShaderRegister = 0;												// レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;					// PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成

	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));


	// 2. InputLayoutの設定
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
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// 3. BlendDtateの設定
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = true;

	SetBlendMode(blendDesc, blendMode);
	currentBlendMode = BlendMode::kBlendModeAdd;  // 現在のブレンドモード
	// α値のブレンド
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	// RasterrizerStateの設定
	// カリングなし
	rasterrizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// 塗りつぶしモード
	rasterrizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	// 4. Shaderをコンパイルする
	vertexShaderBlob = dxCommon->CompileShader(L"resources/shaders/Particle.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	pixelShaderBlob = dxCommon->CompileShader(L"resources/shaders/Particle.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}




void ParticleManager::SetGraphicsPipeline()
{
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterrizerDesc;
	// Depthstencitの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むのか設定（気にしなくて良い）
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 実際に生成
	HRESULT hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
}


void ParticleManager::SetBlendMode(D3D12_BLEND_DESC& desc, BlendMode mode)
{
	switch (mode)
	{
	case kBlendModeAdd:
		desc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		desc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	case kBlendModeSubtract:
		desc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		desc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	case kBlendModeMultiply:
		desc.RenderTarget[0].SrcBlend = D3D12_BLEND_DEST_COLOR;
		desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		desc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		break;
	case kBlendModeScreen:
		desc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_COLOR;
		break;
	default:
		assert(false);
		break;
	}
}

void ParticleManager::InitializeVertexData()
{
	// インスタンス用のVertexResourceを作成
	instanceingResource = dxCommon->CreateBufferResource(sizeof(ParticleForGPU) * kMaxParticle);
	// 書き込むためのポインタを取得
	instanceingResource->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));

	// パーティクルの初期化
	for (int i = 0; i < kMaxParticle; i++)
	{
		instancingData[i].WVP = MakeIdentity4x4();
		instancingData[i].World = MakeIdentity4x4();
		instancingData[i].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	// パーティクルの頂点データを初期化
	modelData.vertices.push_back({ .position = {1.0f, 1.0f, 0.0f, 1.0f},   .texcoord = {0.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.vertices.push_back({ .position = {-1.0f, 1.0f, 0.0f, 1.0f},  .texcoord = {1.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.vertices.push_back({ .position = {1.0f, -1.0f, 0.0f, 1.0f},  .texcoord = {0.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.vertices.push_back({ .position = {1.0f, -1.0f, 0.0f, 1.0f},  .texcoord = {0.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.vertices.push_back({ .position = {-1.0f, 1.0f, 0.0f, 1.0f},  .texcoord = {1.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.vertices.push_back({ .position = {-1.0f, -1.0f, 0.0f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f} });

	// バッファービューの作成
	CreateVertexBufferView();
}

void ParticleManager::CreateVertexBufferView()
{
	vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	// 頂点バッファービュー
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 1頂点分のサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	// 頂点数
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	// データの書き込み / マップ
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// データの書き込み
	memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

}

void ParticleManager::InitializeMaterial()
{
	// リソース生成
	materialResource = dxCommon->CreateBufferResource(sizeof(Material));
	// マテリアルデータに書き込み
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// マテリアルデータの初期化
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();
}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string textureFilrPath)
{
	// 登録済みの名前か確認
	if (particleGroups.contains(name))
	{
		// 登録されていたらリターン
		Logger::Log("既に登録されているパーティクルグループ名です");
		return;
	}
	// 新たな空のパーティクルグループを作成しコンテナに登録
	particleGroups[name] = ParticleGroup();
	// マテリアルデータの作成
	ParticleGroup& particleGroup = particleGroups[name];

	// 新たなパーティクルのマテリアルデータを作成
	particleGroup.materialData.textureFilePath = textureFilrPath;
	// テクスチャの読み込み
	TextureManager::GetInstance()->LoadTexture(particleGroup.materialData.textureFilePath);
	// マテリアルデータにテクスチャのSRVインデックスを設定
	particleGroup.materialData.textureIndex = TextureManager::GetInstance()->GetSrvIndex(particleGroup.materialData.textureFilePath);
	// インスタンシング用のリソースを作成
	particleGroup.instancingResource = dxCommon->CreateBufferResource(sizeof(ParticleForGPU) * kMaxParticle);
	particleGroup.srvIndex = srvManager->Allocater();
	// 買いこむためのアドレスを取得
	particleGroup.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroup.instancingData));

	 srvManager->CreateSRVforStructuredBuffer(particleGroup.srvIndex, particleGroup.instancingResource.Get(), kMaxParticle, sizeof(ParticleForGPU));

	 D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	 srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	 srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	 srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	 srvDesc.Buffer.FirstElement = 0;
	 srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	 srvDesc.Buffer.NumElements = kMaxParticle;
	 srvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);

	 srvHandleCPU = srvManager->GetCPUDescriptorHandle(particleGroup.srvIndex);
	 srvHandleGPU = srvManager->GetGPUDescriptorHandle(particleGroup.srvIndex);

	// 3 Particle
	dxCommon->GetDevice()->CreateShaderResourceView(particleGroup.instancingResource.Get(), &srvDesc, srvHandleCPU);
	// インスタンス数を初期化
	particleGroup.kNumInstance = 0;

}

void ParticleManager::Update()
{
	// 行列の更新
	UpdateMatrix();
	// 全てのパーティクルグループの処理
	UpdateParticle();

}

void ParticleManager::UpdateMatrix()
{
	// カメラ行列の取得
	cameraMatrix = MakeAffineMatrix(camera->GetScale(), camera->GetRotate(), camera->GetTranslate());
	// ビュー行列の取得
	viewMatrix = Inverse(cameraMatrix);
	// プロジェクション行列の取得
	projectionMatrix = MakePerspectiveFovMatrix(0.45f, WinAPI::kClientWidth / WinAPI::kClientWidth, 0.1f, 100.0f);
	// ワールド行列の取得
	backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);

	// ビルボード用の行列
	billboardMatrix = Multiply(backToFrontMatrix, cameraMatrix);
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;

	// ビュー・プロジェクション行列を生成
	projectionMatrix = Multiply(viewMatrix, projectionMatrix);
}

void ParticleManager::UpdateParticle()
{
	uint32_t numInstance = 0;

	// 全てのパーティクルグループについて処理
	for (auto& [name, particleGroup] : particleGroups)
	{
		// 各パーティクルを更新
		for (auto particleIterator = particleGroup.particles.begin(); particleIterator != particleGroup.particles.end();)
		{
			// 生存時間を過ぎていたら更新せず描画対象にしない
			if (particleIterator->lifeTime <= particleIterator->currentTime)
			{
				// 消す
				particleIterator = particleGroup.particles.erase(particleIterator);
				continue;
			}

			Matrix4x4 worldMatrixParticle = MakeAffineMatrix(particleIterator->transform.scale, particleIterator->transform.rotate, particleIterator->transform.translate);
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrixParticle, Multiply(viewMatrix, projectionMatrix));
			// インスタンス数が上限を超えていないならインスタンスデータを書き込む
			if (numInstance < kMaxParticle)
			{
				instancingData[numInstance].WVP = worldViewProjectionMatrix;
				instancingData[numInstance].World = worldMatrixParticle;
				instancingData[numInstance].color = particleIterator->color;
				instancingData[numInstance].color.w = 1.0f - (particleIterator->currentTime / particleIterator->lifeTime);

				++numInstance;
			}

			// パーティクルの更新
			particleIterator->transform.translate.x += particleIterator->velocity.x * kDeltaTime;
			particleIterator->transform.translate.y += particleIterator->velocity.y * kDeltaTime;
			particleIterator->transform.translate.z += particleIterator->velocity.z * kDeltaTime;
			particleIterator->currentTime += kDeltaTime;

			// 次のパーティクルに進める
			++particleIterator;
		}

		// インスタンス数を更新
		particleGroup.kNumInstance = numInstance;
	}
}

bool ParticleManager::IsCollision(const AABB& aabb, const Vector3& point)
{
	if (aabb.min.x <= point.x && point.x <= aabb.max.x &&
		aabb.min.y <= point.y && point.y <= aabb.max.y &&
		aabb.min.z <= point.z && point.z <= aabb.max.z) {
		return true;
	}
	return false;
}

void ParticleManager::Draw()
{
	// コマンド : ルートシグネチャを設定
	dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	// コマンド : パイプラインステートオブジェクトを設定
	dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState);
	// コマンド : プリミティブトロポジ(描画形状)を設定
	dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// コマンド : VertexBufferViewを設定
	dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	// 全てのパーティクルグループについて処理
	for (auto& [name, particleGroup] : particleGroups)
	{
		// インスタンシングデータの更新
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, particleGroup.instancingResource->GetGPUVirtualAddress());
		// マテリアルデータの更新
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, materialResource->GetGPUVirtualAddress());
		// シェーダリソースビューの設定
		D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = srvManager->GetGPUDescriptorHandle(particleGroup.materialData.textureIndex);
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureHandle);
		// インスタンシングデータのSRVを設定
		D3D12_GPU_DESCRIPTOR_HANDLE instancingHandle = srvManager->GetGPUDescriptorHandle(particleGroup.srvIndex);
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(1, instancingHandle);
		// 描画
		dxCommon->GetCommandList()->DrawInstanced(6, particleGroup.kNumInstance, 0, 0);
	}
}


void ParticleManager::Emit(const std::string& name, const Vector3& position, uint32_t count)
{
	// 登録済みのパーティクルグループ名かチェック
	auto it = particleGroups.find(name);
	// パーティクルグループが存在することを確認
	assert(it != particleGroups.end()); 
	// 指定されたパーティクルグループにパーティクルを追加
	ParticleGroup& group = it->second;
	// 各パーティクルを生成し追加
	for (uint32_t i = 0; i < count; ++i) {
		Particle newParticle = MakeParticle(randomEngine);
		group.particles.push_back(newParticle);
	}
}


Particle ParticleManager::MakeParticle(std::mt19937& randomEngine) {
	Particle particle;
	std::uniform_real_distribution<float> distPosition(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distVelocity(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);

	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { 0.0f, 3.3f, 0.0f };
	particle.transform.translate = { distPosition(randomEngine), distPosition(randomEngine), distPosition(randomEngine) };
	particle.velocity = { distVelocity(randomEngine), distVelocity(randomEngine), distVelocity(randomEngine) };
	particle.color = { distColor(randomEngine), distColor(randomEngine), distColor(randomEngine), 1.0f };
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;

	return particle;
}

