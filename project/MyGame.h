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
#include "Blocks.h"
#include "Player.h"
#ifdef _DEBUG
#include "ImGuiManager.h"
#endif
#include <iostream>
#include <cstdlib>
#include <ctime>

// ゲーム全体 : フレームワーククラス継承
class MyGame :public Framework
{
public:
	// コンストラクタ
	MyGame();
	// デストラクタ
	~MyGame();
	// 初期化　
	void Initialize() override;
	// 更新
	void Update()override;
	// 描画
	void Draw()override;
	// 終了処理
	void Finalize()override;
	// 床生成関数
	void CreateFloor();
	// ブロック生成関数
	void CreateHandrance();
	// ランダム
	int GetRandom(int min, int max);
	// 当たり判定
	bool HitCheck(Vector3 pos1, Vector3 pos2, Vector3 scale1, Vector3 scale2);
	// 当たり判定統括
	void HitCheckAll();
	void HandleGameOver();
private:
	struct BlockInfo {
		Vector3 position;
		Vector3 scale;
		Vector3 rotation;
	};

private:
	// ブロック
	std::list<Blocks*>blocks;
	// 障害ブロック
	std::list<Blocks*>hindranceBlocks;

	// プレイヤー
	Player* player = nullptr;
	// スプライト
	std::vector<Sprite*> sprites;
	// 3Dオブジェクト
	
	// 3Dモデル
	std::vector<Model*> models;
	Model* model = nullptr;
	// 3Dオブジェクト
	//Object3D* object3D2 = nullptr;
	//// 3Dモデル
	//Model* model2 = nullptr;

	BlockInfo blockInfo;

private:	// ゲーム類の初期化
	const int num = 100;
	const float popCycle = 11.0f;
	float cycleCount = 0.0f;
	// 並べる間隔
	const float interval = 10.0f;

	Vector3 hitBlockPos;

	bool isJump = false;
	bool isGameOver = false;
	float jumpSpe = 0.1f;
};

