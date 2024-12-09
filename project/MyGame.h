#pragma once
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
#include "srvManager.h"
#ifdef _DEBUG
#include "ImGuiManager.h"
#endif

// ゲーム全体
class MyGame
{
public:
	// 初期化　
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();
	// 終了処理
	void Finalize();

	// 終了フラグのチェック
	bool IsEndRequst() { return isEndRequst; }

private: // Initialize関連
	// ウィンドウAPI
	WinAPI* winAPI = nullptr;
	// DirectX共通部
	DirectXCommon* dxCommon = nullptr;
	// 入力処理
	Input* input = nullptr;
	// ImGui
#ifdef _DEBUG
	ImGuiManager* imGui = nullptr;
#endif
	// SRVマネージャー
	SrvManager* srvManager = nullptr;
	// テクスチャマネージャー
	TextureManager* textureManager = nullptr;
	// 3Dモデルマネージャー
	ModelManager* modelManager = nullptr;
	// モデル共通部
	ModelCommon* modelCommon = nullptr;
	// スプライト共通部
	SpriteCommon* spriteCommon = nullptr;
	// 3Dオブジェクト共通部
	Object3DCommon* object3DCommon = nullptr;
	// カメラ
	Camera* camera = nullptr;
private: 
	// スプライト
	std::vector<Sprite*> sprites;
	// 3Dオブジェクト
	Object3D* object3D = nullptr;
	// 3Dモデル
	Model* model = nullptr;
	// 3Dオブジェクト
	Object3D* object3D2 = nullptr;
	// 3Dモデル
	Model* model2 = nullptr;

	// ゲーム終了フラグ
	bool isEndRequst = false;

};

