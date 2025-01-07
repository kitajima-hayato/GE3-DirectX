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

	/*title = new Object3D();
	title->Initialize(object3DCommon);

	titleModel = new Model();
	titleModel->Initialize(modelCommon, "resources", "Title.obj");
	ModelManager::GetInstance()->LoadModel("Title.obj");
	title->SetModel("Title.obj");*/




	playerInfo.position = { -3.0f,-2.2f,10.0f };
	playerInfo.scale = { 0.5f,0.5f,1.0f };
	playerInfo.rotation = { player->GetRotate() };


	player->SetTranslate(playerInfo.position);
	player->SetScale(playerInfo.scale);
	player->SetRotate(playerInfo.rotation);
}

void Player::Update()
{   
	// ジャンプ中の処理
	if (!isGrounded) {
		// 重力を適用
		jumpVelocity -= gravity;
		playerInfo.position.y += jumpVelocity;

		// 地面に着地した場合
		if (playerInfo.position.y <= -2.2f) {
			playerInfo.position.y = -2.2f;
			isGrounded = true;
			jumpVelocity = 0.0f;
			jumpCount = 0; // ジャンプ回数をリセット
		}
	}
	Jump();
	player->Update();
#ifdef _DEBUG
	ImGui::DragFloat3("PlayerPos", &playerInfo.position.x, 0.1f);
	ImGui::DragFloat3("PlayerScale", &playerInfo.scale.x, 0.1f);
	ImGui::DragFloat3("PlayerRotate", &playerInfo.rotation.x, 0.1f);
	// フラグの状態を表示
	ImGui::Checkbox("IsJumping", &isJump);
#endif

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
	delete model;
	delete player;

}

void Player::Jump()
{
	if (input->TriggerKey(DIK_SPACE) && (isGrounded || jumpCount < maxJumpCount)) {
		jumpVelocity = jumpPower;
		isGrounded = false;
		jumpCount++;
	}
}


