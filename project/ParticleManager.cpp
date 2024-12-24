#include "ParticleManager.h"
#include <Logger.h>
#include <MyMath.h>
#include <numbers>
#include <d3d12.h>

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
    // NULL検出 // メンバ変数に代入
    assert(dxCommon);
    this->dxCommon = dxCommon;
    assert(srvManager);
    this->srvManager = srvManager;

    // インスタンシングデータのメモリ確保
    CreateGraphicsPipeLine();
    // 頂点リソース生成
    CreateVertexResource();
    // 頂点バッファの生成
    CreateVertexBufferView();
    // 頂点リソースに頂点データを書き込む
    WriteVertexResource();
}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string textureFilePath)
{
    // すでに登録されているか確認
    auto it = particleGroups.find(name);
    if (it != particleGroups.end()) {
        Logger::Log("このデータは登録済みです");
        return;
    }
    // 新規登録
    ParticleGroup particleGroup;
    // マテリアルデータにテクスチャファイルパスを設定
    particleGroup.materialData.textureFilePath = textureFilePath;
    // テクスチャを読み込む
    TextureManager::GetInstance()->LoadTexture(particleGroup.materialData.textureFilePath);
    // マテリアルデータにテクスチャのSRVインデックスを記録
    particleGroup.materialData.textureIndex = TextureManager::GetInstance()->GetSrvIndex(particleGroup.materialData.textureFilePath);
    // インスタンシング用リソースの生成
    particleGroup.instancingResource = dxCommon->CreateBufferResource(sizeof(ParticleForGPU) * particleGroup.kNumMaxInstance);
    // インスタンシング用にSRVを確保してSRVインデックスを記録
    // エラー出たらここ怪しいかも問題なかったらこのコメントアウト消す
    particleGroup.instancingDataSRVIndex = srvManager->IsAllocate();
    // SRV生成(StructuredBuffer用設定)
    srvManager->CreateSRVforStructuredBuffer(particleGroup.instancingDataSRVIndex, particleGroup.instancingResource.Get(), particleGroup.kNumMaxInstance, sizeof(ParticleForGPU));
}

void ParticleManager::Update(Camera* camera)
{
    Matrix4x4 cameraMatrix = camera->GetCameraMatrix();
    // ビルボード行列の更新
    Matrix4x4 backFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
    Matrix4x4 billboardMatrix = Multiply(backFrontMatrix, cameraMatrix);
    // ビュー行列とプロジェクション行列をカメラから取得
    Matrix4x4 viewMatrix = camera->GetViewMatrix();
    Matrix4x4 projectionMatrix = camera->GetProjectionMatrix();
    // 全てのパーティクルグループについて処理する

    for (auto& [groupName, particleGroup] : particleGroups) {
        for (auto particleIterater = particleGroup.particles.begin(); particleIterater != particleGroup.particles.end();) {
            // 寿命に達していたらグループから外す
            if ((*particleIterater).lifeTime <= (*particleIterater).currentTime) {
                particleIterater = particleGroup.particles.erase(particleIterater);
                continue;
            }
            // 移動処理(速度を座標に加算)
            (*particleIterater).transform.translate.x += (*particleIterater).velocity.x * kDeltaTime;
            (*particleIterater).transform.translate.y += (*particleIterater).velocity.y * kDeltaTime;
            (*particleIterater).transform.translate.z += (*particleIterater).velocity.z * kDeltaTime;
            // 経過時間を加算
            (*particleIterater).currentTime += kDeltaTime;
            // ワールド行列を計算
            Matrix4x4 worldMatrix = MakeAffineMatrix((*particleIterater).transform.scale, (*particleIterater).transform.rotate, (*particleIterater).transform.translate);

            // ワールドビュープロジェクション行列を合成
            Matrix4x4 wvpMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
            float alphe = 1.0f - ((*particleIterater).currentTime / (*particleIterater).lifeTime);
            // インスタンシング用データ１個分の書き込み
            particleGroup.instancingData->WVP = wvpMatrix;
            particleGroup.instancingData->World = worldMatrix;
            particleGroup.instancingData->color = (*particleIterater).color;
            particleGroup.instancingData->color.w = alphe;
            ++particleIterater;
        }
    }
}

void ParticleManager::Draw()
{
    // コマンド : ルートシグネチャを設定
    dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
    // コマンド : PSO(Pipeline State Object)を設定
    dxCommon->GetCommandList()->SetPipelineState(pipelineState.Get());
    // コマンド : プリミティブトポロジー(描画計上)を設定
    dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // コマンド : VBV(Vertex Buffer View)を設定
    dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
    // 全てのパーティクルグループについて処理する
    for (auto& [groupName, particleGroup] : particleGroups) {
        // コマンド : テクスチャのSRVのDescriptorTableを設定
        dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(2, particleGroup.materialData.textureIndex);
        // コマンド : インスタンシングデータのSRVのDescriptorTableを設定
        dxCommon->GetCommandList()->SetGraphicsRoot32BitConstant(3, particleGroup.materialData.textureIndex, 0);
        // DrawCall(インスタンシング描画)
        dxCommon->GetCommandList()->DrawInstanced(UINT(particleGroup.particles.size()), particleGroup.kNumMaxInstance, 0, 0);
    }
}

void ParticleManager::Emit(const std::string& name, const Vector3& position, uint32_t count)
{
    // パーティクルグループを取得
    auto it = particleGroups.find(name);
    if (it == particleGroups.end()) {
        Logger::Log("パーティクルグループが見つかりません");
        return;
    }

    // パーティクルを生成
    for (uint32_t i = 0; i < count; ++i) {
        it->second.particles.push_back(MakeParticle(randomEngine, position));
    }
}

Particle ParticleManager::MakeParticle(std::mt19937& randomEngine, const Vector3& position) {
    std::uniform_real_distribution<float> distVelocity(-1.0f, 1.0f);
    std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
    std::uniform_real_distribution<float> distTime(1.0f, 3.0f);

    Particle particle;

    particle.transform.scale = { 1.0f, 1.0f, 1.0f };
    particle.transform.rotate = { 0.0f, 3.3f, 0.0f };
    // 位置を引数のpositionで設定
    particle.transform.translate = position;
    // 速度を[-1~1], 色は[0~1]の範囲でランダムに生成
    particle.velocity = { distVelocity(randomEngine), distVelocity(randomEngine), distVelocity(randomEngine) };
    particle.color = { distColor(randomEngine), distColor(randomEngine), distColor(randomEngine), 1.0f };
    particle.lifeTime = distTime(randomEngine);
    particle.currentTime = 0;

    return particle;
}

ParticleManager* ParticleManager::GetInstance()
{
    static ParticleManager instance;
    return &instance;
}

void ParticleManager::CreateGraphicsPipeLine()
{
    // ルートシグネチャの作成
    CreateRootSignature();

    // αのブレンド設定
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

    // カリング設定
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    // 三角形の塗りつぶし
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

#pragma region GraphicsPipelineState
    vertexShaderBlob = dxCommon->CompileShader(L"resources/shaders/Particle.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);
    pixelShaderBlob = dxCommon->CompileShader(L"resources/shaders/Particle.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);

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
    pipelineState = nullptr;
    HRESULT hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&particlePipelineStateDesc, IID_PPV_ARGS(&pipelineState));
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

    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 1;
    // 0から始まる
    descriptorRange[0].NumDescriptors = 1;
    // 数は1つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    //RootSignature作成  Particle
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignatureParticle{};
    descriptionRootSignatureParticle.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    //RootParameter作成。複数設定できるので配列。今回は結果１つだけなので長さ１の配列
    D3D12_ROOT_PARAMETER rootParametersParticle[4] = {};
    rootParametersParticle[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
    rootParametersParticle[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
    rootParametersParticle[0].Descriptor.ShaderRegister = 0; //レジスタ番号０とバインド

    rootParametersParticle[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParametersParticle[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParametersParticle[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;
    rootParametersParticle[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);

    rootParametersParticle[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う
    rootParametersParticle[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
    rootParametersParticle[2].DescriptorTable.pDescriptorRanges = descriptorRange; //Tableの中身の配列を指定
    rootParametersParticle[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

    rootParametersParticle[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //DescriptorTableを使う
    rootParametersParticle[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
    rootParametersParticle[3].Descriptor.ShaderRegister = 0; //Tableで利用する数

    descriptionRootSignatureParticle.pParameters = rootParametersParticle; //ルートパラメータ配列へのポインタ
    descriptionRootSignatureParticle.NumParameters = _countof(rootParametersParticle); //配列の長さ

    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う
    staticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
    descriptionRootSignatureParticle.pStaticSamplers = staticSamplers;
    descriptionRootSignatureParticle.NumStaticSamplers = _countof(staticSamplers);

    // ルートシグネチャのシリアライズと作成
    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignatureParticle, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    assert(SUCCEEDED(hr));
    hr = dxCommon->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));
}

void ParticleManager::CreateVertexResource()
{
    // 頂点リソース生成
    vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * vertexData.size());

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
    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(VertexData);
    vertexBufferView.SizeInBytes = sizeof(VertexData) * 6; // 頂点数に応じて変更
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



