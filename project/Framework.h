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
#include "Audio.h"
#include "SceneManager.h"
#include "AbstractSceneFactory.h"
#include "ParticleManager.h"
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

protected:// Initialize関連
	// ウィンドウAPI
	std::unique_ptr<WinAPI> winAPI;
	// DirectX共通部
	std::unique_ptr<DirectXCommon> dxCommon;
	// ImGui
#ifdef _DEBUG
	std::unique_ptr<ImGuiManager> imGui;
#endif
	// SRVマネージャー
	std::unique_ptr<SrvManager> srvManager;
	// モデル共通部
	std::unique_ptr<ModelCommon> modelCommon;
	// 3Dオブジェクト共通部
	std::unique_ptr<Object3DCommon> object3DCommon;
	// カメラ
	std::unique_ptr<Camera> camera;
public:
	// シーンファクトリー
	std::unique_ptr<AbstractSceneFactory> sceneFactory_ = nullptr;
private:
	// リークチェッカー
	D3DResourceLeakChecker leakCheck;
	

	
};

