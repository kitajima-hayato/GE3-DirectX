#include "Player.h"
#include <ImGuiManager.h>
Player::Player()
{
}

Player::~Player()
{
	delete player;
	delete model;
}

void Player::Initialize(Object3DCommon* object3DCommon, ModelCommon* modelCommon, Input* input)
{
	player = new Object3D();
	player->Initialize(object3DCommon);

	model = new Model();
	model->Initialize(modelCommon, "resources", "Player.obj");
	ModelManager::GetInstance()->LoadModel("Player.obj");
	player->SetModel("Player.obj");
	this->input = input;

	playerInfo.position = { -3.0f,-2.2f,10.0f };
	playerInfo.scale = { 0.5f,0.5f,1.0f };
	playerInfo.rotation = { player->GetRotate() };


	player->SetTranslate(playerInfo.position);
	player->SetScale(playerInfo.scale);
	player->SetRotate(playerInfo.rotation);
}

void Player::Update()
{
	player->Update();
	Jump();

	ImGui::DragFloat3("PlayerPos", &playerInfo.position.x, 0.1f);
	ImGui::DragFloat3("PlayerScale", &playerInfo.scale.x, 0.1f);
	ImGui::DragFloat3("PlayerRotate", &playerInfo.rotation.x, 0.1f);
	// フラグの状態を表示
	ImGui::Checkbox("IsJumping", &isJump);


	//Vector3 playerPos = player->GetTranslate();

	player->SetTranslate(playerInfo.position);
	player->SetScale(playerInfo.scale);
	player->SetRotate(playerInfo.rotation);
}

void Player::Draw()
{
	player->Draw();
}

void Player::Finalize()
{
}

void Player::Jump()
{
	Vector3 playerPos = player->GetTranslate();
	// Spaceキーが押されたらジャンプ
	if (input->TriggerKey(DIK_SPACE)) {
		isJump = true;
	}

	// ジャンプ中
	if (isJump) {
		jumpSpe += 0.1f;
		if (jumpSpe >= 0.5f)jumpSpe = 0.5f;
		playerInfo.position.y += jumpSpe;
	}

	// 一定の高さまで行ったら降りる
	if (player->GetTranslate().y >= 0.0f) {
		isJump = false;
	}
	// 落下中 // 着地中
	if (!isJump) {

		if (playerInfo.position.y <= -2.2f)jumpSpe = 0.0f;
		else {
			jumpSpe = 0.1f;
			if (jumpSpe <= 0.1f)jumpSpe += 0.1f;
		}
		playerInfo.position.y -= jumpSpe;
	}

	player->SetTranslate(playerInfo.position);
}
