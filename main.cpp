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
#include "D3DResourceLeakChecker.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "Object3D.h"
#include "Object3DCommon.h"

using namespace Logger;
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
using namespace std;






//ウィンドウズアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;


#pragma region 基盤システムの初期化
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

	// テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize(dxCommon);

	// スプライト共通部の初期化
	SpriteCommon* spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	// 3Dオブジェクト共通部の初期化
	Object3DCommon* object3DCommon = new Object3DCommon();
	object3DCommon->Initialize(dxCommon);


#pragma endregion 


#pragma region 最初のシーンの初期化
	// 
	Object3D* object3D = new Object3D();
	object3D->Initialize(object3DCommon);

	std::vector<Sprite*> sprites;
	//std::vector<Sprite*> sprites2;
    sprites.clear();
	//sprites2.clear();
	Sprite* sprite = new Sprite();
	sprite->Initialize(spriteCommon, "resources/uvChecker.png");
	Vector2 pos = { 0.0f, 0.0f };
	sprite->SetPosition(pos);
	//Vector2 size = { 100.0f, 100.0f };
	//sprite->SetSize(size);
	sprites.push_back(sprite);
	
	/*for (uint32_t i = 0; i < 5; ++i) {
		Sprite* sprite = new Sprite();
		sprite->Initialize(spriteCommon, "resources/uvChecker.png");
		Vector2 pos = { 100.0f * i, 100.0f };
		sprite->SetPosition(pos);
		Vector2 size = { 100.0f, 100.0f };
		sprite->SetSize(size);
		sprites.push_back(sprite);
	}
	for (uint32_t i = 0; i < 5; ++i) {
		Sprite* sprite2 = new Sprite();
		sprite2->Initialize(spriteCommon, "resources/monsterball.png");
		Vector2 pos = { 100.0f * i, 200.0f };
		sprite2->SetPosition(pos);
		Vector2 size = { 100.0f, 100.0f };
		sprite2->SetSize(size);
		sprites.push_back(sprite2);
	}*/


#pragma endregion



	


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
	//　
	


	/*::ScratchImage mipImages;
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr < ID3D12Resource> textureResource = dxCommon->CreateTextureResource(metadata);
	Microsoft::WRL::ComPtr < ID3D12Resource> intermediateResource = dxCommon->UploadTextureData(textureResource, mipImages);
	//2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages2 = DirectXCommon::LoadTexture("resources/monsterball.png");
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr <ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(metadata2);
	Microsoft::WRL::ComPtr <ID3D12Resource> intermediateResource2 = dxCommon->UploadTextureData(textureResource2, mipImages2);
	
	#pragma endregion
	
	
	
	
	metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetSrvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSrvDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	//先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの生成  シェーダーリソースビュー
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);
	*/


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
	//	//DepthStencilTextureをウィンドウのサイズで作成
	//	Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device, WinAPI::kClientWidth, WinAPI::kClientHeight);
	//
	//
	//
	//
	//	//Sprite用の頂点リソースをつくる
	//
	//	//頂点バッファービューを作成する
	//
	//
	//	/////////////////////////////////////////////////////////////
	//
	//	//インデックスリソースにデータを書き込む
	//	uint32_t* indexDataSprite = nullptr;
	//	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	//	
	//	
	//
	//	/////////////////////////////////////////////////////////////
	//
	//
	//
	//	//Sprite用のTransformaitionMatrix用のリソースをつくる。Matrix4x4 １つ分のサイズを用意する
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

	//	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	//
	//

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

		for (Sprite* sprite : sprites) {
			sprite->Update();
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
				//ImGui::Begin("Settings");
				//if (ImGui::BeginTabBar("OBJ"))
				//{
				//	// Objの値変更
				//	if (ImGui::BeginTabItem("Sprite"))
				//	{
		//ImGui::DragFloat2("*spritePos", &pos.x);


				//		
				//		ImGui::EndTabItem();
				//	}
				//	
				//}

		/*ImGui::End();
		ImGui::Render();*/



		//DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
		dxCommon->PreDraw();
		
		// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
		object3DCommon->DrawSettingCommon();

		//Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
		spriteCommon->DrawSettingCommon();

		
		//// positionの変更
		//Vector2 pos = sprite->GetPosition();
		//pos.x += 0.1f;
		//sprite->SetPosition(pos);
		//// rotationの変更
		//float angle = sprite->GetRotation();
		//angle += 0.01f;
		//sprite->SetRotation(angle);
		//// colorの変更
		//Vector4 color = sprite->GetColor();
		//color.x += 0.1f;
		//if (color.x > 1.0f) color.x = 0.0f;
		//sprite->SetColor(color);
		//// scaleの変更
		//Vector2 size = sprite->GetSize();
		//size.x -= 0.1f;
		//size.y += 0.1f;
		//sprite->SetSize(size);

		//スプライトの複数描画
       
		
       
		for (Sprite* sprite : sprites) {
			sprite->Draw();
		}
		/*for (Sprite* sprite2 : sprites2) {
			sprite2->Draw();
		}*/

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

		dxCommon->PostDraw();


	}
#pragma region  解放処理

	/*ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();*/

	//CloseHandle(fenceEvent);
	TextureManager::GetInstance()->Finalize();
	winAPI->Finalize();

#pragma endregion



	////出力ウィンドウへの文字出力　実行すると出る下の文字
	//OutputDebugStringA("Hello,DirectX!\n");

	//解放
	/*for (Sprite* sprite2 : sprites2) {
		delete sprite2;
	}*/
	for (Sprite* sprite : sprites) {
		delete sprite;
	}
	delete object3D;
	delete object3DCommon;
	delete spriteCommon;
	delete dxCommon;
	delete input;
	delete winAPI;
	return 0;
}