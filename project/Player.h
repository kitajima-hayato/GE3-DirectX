#pragma once
#include "MyMath.h"
#include "Object3D.h"
#include "Model.h"
#include "ModelManager.h"
#include "Input.h"
class Player
{
public:
	Player();
	~Player();
	// 初期化
	void Initialize(Object3DCommon* object3DCommon, ModelCommon* modelCommon,Input* input);
	// 更新 
	void Update();
	// 描画
	void Draw();
	// 終了処理
	void Finalize();

	// ジャンプ
	void Jump();

	// プレイヤーの情報
	Vector3 GetTranslate() { return playerInfo.position; }
	Vector3 GetScale() { return playerInfo.scale; }
	Vector3 GetRotate() { return playerInfo.rotation; }
	// Setter
	void SetTranslate(Vector3 pos) { playerInfo.position = pos; }

private:
	struct PlayerInfo {
		Vector3 position;
		Vector3 scale;
		Vector3 rotation;
	};

private:
	PlayerInfo playerInfo;
	Object3D* player;
	Model* model = nullptr;

	Input* input;
	bool isJump = false;
	float jumpSpe = 0.1f;
	
};

