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
#include "Framework.h"
#ifdef _DEBUG
#include "ImGuiManager.h"
#endif
#include "GamePlayScene.h"

// ゲーム全体 : フレームワーククラス継承
class MyGame :public Framework
{
public:	// メンバ関数
	// 初期化
	void Initialize() override;
	// 更新
	void Update()override;
	// 描画
	void Draw()override;
	// 終了処理
	void Finalize()override;


private:	// メンバ変数
	// ゲームプレイシーンクラス
	GamePlayScene* scene_ = nullptr;

};

