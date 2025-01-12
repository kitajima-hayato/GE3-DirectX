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
#include "ModelManager.h"
#include "ImGuiManager.h"

class Framework
{

public:
	virtual ~Framework() = default;
public:	// メンバ関数
	// 初期化
	virtual void Initialize();
	// 更新
	virtual void Update();
	// 描画
	virtual void Draw() = 0;
	// 終了処理
	virtual void Finalize();
	// 実行
	void Run();
	// 終了リクエスト
	bool IsEndRequst() { return isEndRequst;}

protected:
	// 終了リクエスト
	bool isEndRequst = false;

private:
	D3DResourceLeakChecker leakCheck;
protected:// Initialize関連
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
};

