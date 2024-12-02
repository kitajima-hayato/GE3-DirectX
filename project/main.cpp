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
#include "Model.h"
#include "ModelCommon.h"
#include "ModelManager.h"
#include "ImGuiManager.h"
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
using namespace Logger;
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

	// ImGuiの初期化
	ImGuiManager* imGui = new ImGuiManager();
	imGui->Initialize(winAPI,dxCommon);

	// テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize(dxCommon);
	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize(dxCommon);
	
	// スプライト共通部の初期化
	SpriteCommon* spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	// 3Dオブジェクト共通部の初期化
	Object3DCommon* object3DCommon = new Object3DCommon();
	object3DCommon->Initialize(dxCommon);

	// モデル共通部の初期化
	ModelCommon* modelCommon = new ModelCommon();
	modelCommon->Initialize(dxCommon);


#pragma endregion 

#pragma region 最初のシーンの初期化
	// 
	Object3D* object3D = new Object3D();
	object3D->Initialize(object3DCommon);

	Model* model = new Model();
	model->Initialize(modelCommon,"resources","axis.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");
	object3D->SetModel("axis.obj");

	// モデルをもう一つ読み込む
	Model* model2 = new Model();
	model2->Initialize(modelCommon, "resources", "plane.obj");
	ModelManager::GetInstance()->LoadModel("plane.obj");
	Object3D* object3D2 = new Object3D();
	object3D2->Initialize(object3DCommon);
	object3D2->SetModel("plane.obj");


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
	
#pragma endregion

	// ウィンドウの×ボタンが押されるまでループ
	while (true) {		// ゲームループ
		//Windowのメッセージ処理
		if (winAPI->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}
		// ImGuiの処理
		imGui->Begin();

		//ImGui始まるよ
		/*ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();*/

		//ゲームの処理
		
		object3D->Update();
		object3D2->Update();
		if (input->TriggerKey(DIK_1)) {
			OutputDebugStringA("Hit_1\n");
		}

		for (Sprite* sprite : sprites) {
			sprite->Update();
		}

	

		imGui->End();
		
		// DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
		dxCommon->PreDraw();


		// ImGuiの描画


		
		// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
		object3DCommon->DrawSettingCommon();
		object3D->Draw();
		object3D2->Draw();
		//Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
		spriteCommon->DrawSettingCommon();

		

		// ImGuiの描画
		

       
		
       
		for (Sprite* sprite : sprites) {
			sprite->Draw();
		}
	

		// ImGuiの描画
		imGui->Draw();
		dxCommon->PostDraw();

	}
	
#pragma region  解放処理
	// ImGuiの終了処理
	imGui->Finalize();
	// CloseHandle(fenceEvent);
	TextureManager::GetInstance()->Finalize();
	// モデルマネージャの終了処理
	ModelManager::GetInstance()->Finalize();
	winAPI->Finalize();

#pragma endregion



	////出力ウィンドウへの文字出力　実行すると出る下の文字
	//OutputDebugStringA("Hello,DirectX!\n");

	//解放
	
	for (Sprite* sprite : sprites) {
		delete sprite;
	} 
	delete model2;
	delete object3D2;
	delete model;
	delete object3D;
	delete object3DCommon;
	delete spriteCommon;
	delete imGui;
	delete dxCommon;
	delete input;
	delete winAPI;
	return 0;
}