#include "MyGame.h"

MyGame::MyGame()
{
}

MyGame::~MyGame()
{
}

void MyGame::Initialize()
{
	Framework::Initialize();

#pragma region 最初のシーンの初期化
	// 3Dオブジェクトの初期化

	// プレイヤーの初期化
	player = new Player();
	player->Initialize(object3DCommon, modelCommon, input);

	// 乱数生成器のシードを設定
	srand(static_cast<unsigned int>(std::time(nullptr)));

	/*block = new Object3D();
	block->Initialize(object3DCommon);

	model = new Model();
	model->Initialize(modelCommon, "resources", "cube.obj");
	ModelManager::GetInstance()->LoadModel("cube.obj");
	block->SetModel("cube.obj");*/



	/* block->SetTranslate(blockInfo.position);
	 block->SetScale(blockInfo.scale);
	 block->SetRotate(blockInfo.rotation);*/


	 //// モデルをもう一つ読み込む
	 //model2 = new Model();
	 //model2->Initialize(modelCommon, "resources", "plane.obj");

	 //ModelManager::GetInstance()->LoadModel("plane.obj");

	 //object3D2 = new Object3D();
	 //object3D2->Initialize(object3DCommon);
	 //object3D2->SetModel("plane.obj");

	 /*std::vector<Sprite*> sprites;
	 sprites.clear();
	 Sprite* sprite = new Sprite();
	 sprite->Initialize(spriteCommon, "resources/uvChecker.png");
	 Vector2 pos = { 100.0f, 100.0f };
	 sprite->SetPosition(pos);
	 sprites.push_back(sprite);*/

#pragma endregion

}

void MyGame::Update()
{

	Framework::Update();
	if (isGameOver) {
		// ゲームオーバー処理
		HandleGameOver();
		return;
	}


#pragma region ゲームの更新
	// アクターの更新


#pragma endregion

#ifdef _DEBUG // デバッグ時のみ有効ImGuiの処理
	// ImGuiの処理
	imGui->Begin();
#endif

	// ゲームの処理
	// 床の生成
	cycleCount += 1.0f;
	if (cycleCount > popCycle) {
		cycleCount = 0.0f;
		CreateFloor();
		CreateHandrance();
	}

	// ブロックの更新
	for (Blocks* block : blocks) {
		block->Update();
	}
	for (Blocks* block : hindranceBlocks) {
		block->Update();
	}
	HitCheckAll();
	// プレイヤーの更新
	player->Update();

	if (input->TriggerKey(DIK_1)) {
		OutputDebugStringA("Hit_1\n");
	}

	/*for (Sprite* sprite : sprites) {
		sprite->Update();
	}*/

#ifdef _DEBUG
	//ImGui::SetWindowSize(ImVec2(500.0f, 200.0f));
	// ImGuiのデモ
	ImGui::ShowDemoWindow();
	ImGui::Text("Hello, world %d", 123);
	if (ImGui::Button("Save")) {
		OutputDebugStringA("Save\n");
	}
	// ブロックの総数
	// ブロックの総数を表示
	ImGui::Text("Number of blocks: %d", static_cast<int>(blocks.size()));



	Vector3 cameraPos = camera->GetTranslate();
	Vector3 cameraRotate = camera->GetRotate();
	ImGui::DragFloat3("cameraPosition", &cameraPos.x, 0.1f);
	ImGui::DragFloat3("cameraRotate", &cameraRotate.x, 0.1f);
	camera->SetTranslate(cameraPos);
	camera->SetRotate(cameraRotate);
	camera->Update();

	// ブロックの情報を表示
	/*Vector3 blockPos = block->GetTranslate();
	Vector3 blockScale = block->GetScale();
	Vector3 blockRotate = block->GetRotate();
	ImGui::DragFloat3("blockPosition", &blockPos.x, 0.1f);
	ImGui::DragFloat3("blockScale", &blockScale.x, 0.1f);
	ImGui::DragFloat3("blockRotate", &blockRotate.x, 0.1f);
	block->SetTranslate(blockPos);
	block->SetScale(blockScale);
	block->SetRotate(blockRotate);*/




	imGui->End();


#endif
}

void MyGame::Draw()
{
	// DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
	dxCommon->PreDraw();
	srvManager->PreDraw();
	// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	object3DCommon->DrawSettingCommon();

	// 床のブロックの描画
	for (Blocks* block : blocks) {
		block->Draw();
	}
	for (Blocks* block : hindranceBlocks) {
		block->Draw();
	}

	// プレイヤーの描画
	player->Draw();
	/*object3D2->Draw();*/
	//Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	spriteCommon->DrawSettingCommon();

	/*for (Sprite* sprite : sprites) {
		sprite->Draw();
	}*/

#ifdef _DEBUG
	// ImGuiの描画
	imGui->Draw();
#endif
	dxCommon->PostDraw();
}

void MyGame::Finalize()
{
#pragma region  解放処理
#ifdef _DEBUG
	// ImGuiの終了処理
	imGui->Finalize();
#endif
	// CloseHandle(fenceEvent);
	TextureManager::GetInstance()->Finalize();
	// モデルマネージャの終了処理
	ModelManager::GetInstance()->Finalize();
	winAPI->Finalize();

#pragma endregion


	delete model;
	delete player;

	Framework::Finalize();
}

void MyGame::CreateFloor()
{
	// ブロックモデルを一定間隔で並べる

	// ブロックの生成
	Blocks* newBlock = new Blocks();
	newBlock->Initialize(object3DCommon, modelCommon);
	// ブロックの発生位置を設定
	Vector3 floarBlockPos = { 6.5f, -3.0f, 10.0f };
	newBlock->PoPBlock(floarBlockPos);
	// ブロックをリストに追加
	blocks.push_back(newBlock);

	// 左の画面外に出たらデリート
	if (blocks.size() > num) {
		Blocks* oldBlock = blocks.front();
		delete oldBlock;
		blocks.pop_front();
	}
}

void MyGame::CreateHandrance()
{
	// ブロックモデルを一定間隔で並べる
	int val = GetRandom(0, 2);
	if (val == 0) {
	}
	if (val == 1) {

		// ブロックの生成
		Blocks* newBlock = new Blocks();
		newBlock->Initialize(object3DCommon, modelCommon);
		// ブロックの発生位置を設定
		Vector3 HadranceBlockPos = { 6.5f, -2.4f, 10.0f };
		newBlock->PoPBlock(HadranceBlockPos);
		// ブロックをリストに追加
		hindranceBlocks.push_back(newBlock);
		val = GetRandom(1, 2);
		if (val == 2) {
			// ブロックの生成
			Blocks* newBlock = new Blocks();
			newBlock->Initialize(object3DCommon, modelCommon);
			// ブロックの発生位置を設定
			Vector3 HadranceBlockPos = { 6.5f, -1.8f, 10.0f };
			newBlock->PoPBlock(HadranceBlockPos);
			// ブロックをリストに追加
			hindranceBlocks.push_back(newBlock);
		}
	}
	if (hindranceBlocks.size() >= num) {
		Blocks* oldBlock = hindranceBlocks.front();
		delete oldBlock;
		hindranceBlocks.pop_front();
	}

}

int MyGame::GetRandom(int min, int max)
{
	return min + rand() % (max - min + 1);
}

bool MyGame::HitCheck(Vector3 pos1, Vector3 pos2, Vector3 scale1, Vector3 scale2)
{
	// 3Dモデルの中心を基準にした当たり判定
	return (pos1.x - scale1.x / 2 < pos2.x + scale2.x / 2 &&
		pos1.x + scale1.x / 2 > pos2.x - scale2.x / 2 &&
		pos1.y - scale1.y / 2 < pos2.y + scale2.y / 2 &&
		pos1.y + scale1.y / 2 > pos2.y - scale2.y / 2 &&
		pos1.z - scale1.z / 2 < pos2.z + scale2.z / 2 &&
		pos1.z + scale1.z / 2 > pos2.z - scale2.z / 2);
}

void MyGame::HitCheckAll()
{
	// プレイヤーとブロックの当たり判定
	Vector3 playerPos = player->GetTranslate();
	Vector3 playerScale = player->GetScale();
	bool isColliding = false;

	for (Blocks* block : hindranceBlocks) {
		Vector3 blockPos = block->GetTranslate();
		Vector3 blockScale = block->GetScale();
		if (HitCheck(playerPos, blockPos, playerScale, blockScale)) {
			// プレイヤーがブロックに当たったらゲームオーバー
			isGameOver = true;
			break;
		}
	}

	// プレイヤーがブロックに当たっていない場合、プレイヤーの位置をリセット
	if (!isColliding) {
		player->SetTranslate(playerPos);
	}
}
void MyGame::HandleGameOver()
{
	// ゲームオーバー時の処理を実装
	// 例: メッセージを表示、リスタート、メニューに戻るなど
	OutputDebugStringA("Game Over\n");
	// 必要に応じて他の処理を追加
}