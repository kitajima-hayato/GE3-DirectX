#include "MyGame.h"

MyGame::MyGame() {
}

MyGame::~MyGame() {
}

void MyGame::Initialize() {
    Framework::Initialize();
#pragma region 最初のシーンの初期化
    // 3Dオブジェクトの初期化

    // モデルの初期化
    skyDome = new SkyDome();
    skyDome->Initialize(object3DCommon, modelCommon);

    // プレイヤーの初期化
    player = new Player();
    player->Initialize(object3DCommon, modelCommon, input);

    // 乱数生成器のシードを設定
    srand(static_cast<unsigned int>(std::time(nullptr)));

    // スプライトの初期化
    sprite = new Sprite();
    sprite->Initialize(spriteCommon, "resources/colerJump.png");
    float windowWidth = static_cast<float>(WinAPI::kClientWidth);
    Vector2 pos = { windowWidth, 50.0f };  // 画面右端に初期位置を設定
    sprite->SetSize({ 200.0f, 100.0f });  // スプライトの実際のサイズに応じて調整
    sprite->SetAnchorPoint({ 0.5f, 0.5f });  // 中心を基準点に設定
    sprite->SetPosition(pos);

    // sprite2
    sprite2 = new Sprite();
    sprite2->Initialize(spriteCommon, "resources/gameover.png");

    // ウィンドウのサイズを取得
    float windowHeight = static_cast<float>(WinAPI::kClientHeight);

    // スプライトのサイズをウィンドウのサイズに合わせる
    sprite2->SetSize({ windowWidth, windowHeight });

    // スプライトの位置をウィンドウの中心に設定
    Vector2 centerPos = { windowWidth / 2.0f, windowHeight / 2.0f };
    sprite2->SetAnchorPoint({ 0.5f, 0.5f });
    sprite2->SetPosition(centerPos);

    // 足場のブロックを生成
    CreateInitialFloor();

#pragma endregion

    Vector3 cameraPos = camera->GetTranslate();
    Vector3 cameraRotate = camera->GetRotate();
    camera->SetTranslate(cameraPos);
    camera->SetRotate(cameraRotate);
}

void MyGame::Update() {
    Framework::Update();

    if (isGameOver) {
        // ゲームオーバー時にスペースキーが押されたらゲームをリセット
        if (input->TriggerKey(DIK_SPACE)) {
            ResetGame();
        }
        return;
    }

#pragma region ゲームの更新
    // アクターの更新
    skyDome->Update();

#pragma endregion

#ifdef _DEBUG // デバッグ時のみ有効ImGuiの処理
    // ImGuiの処理
    imGui->Begin();
#endif

    // スプライトの位置を更新
    Vector2 spritePos = sprite->GetPosition();
    spritePos.x -= spriteSpeed; // 左方向に移動
    if (spritePos.x < -sprite->GetSize().x / 2) { // スプライトの幅の半分を考慮
        spritePos.x = static_cast<float>(WinAPI::kClientWidth) + sprite->GetSize().x / 2; // 画面左に到達したら右端に戻す
    }
    sprite->SetPosition(spritePos);

    sprite->Update();
    sprite2->Update();

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

    camera->Update();
#ifdef _DEBUG
    imGui->End();
#endif
}

void MyGame::Draw() {
    // DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
    dxCommon->PreDraw();
    srvManager->PreDraw();
    // 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
    object3DCommon->DrawSettingCommon();

    skyDome->Draw();

    // 床のブロックの描画
    for (Blocks* block : blocks) {
        block->Draw();
    }
    for (Blocks* block : hindranceBlocks) {
        block->Draw();
    }

    // プレイヤーの描画
    player->Draw();
    //Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
    spriteCommon->DrawSettingCommon();

    sprite->Draw();
    if (isGameOver) {
        // ゲームオーバー処理
        sprite2->Draw();
    }
#ifdef _DEBUG
    // ImGuiの描画
    imGui->Draw();
#endif
    dxCommon->PostDraw();
}

void MyGame::Finalize() {
#pragma region 解放処理
#ifdef _DEBUG
    // ImGuiの終了処理
    imGui->Finalize();
#endif
    // CloseHandle(fenceEvent);
    TextureManager::GetInstance()->Finalize();
    // モデルマネージャの終了処理
    ModelManager::GetInstance()->Finalize();
    winAPI->Finalize();

    // ブロックの解放
    for (Blocks* block : blocks) {
        delete block;
    }
    for (Blocks* block : hindranceBlocks) {
        delete block;
    }

    // スプライトの解放
    delete sprite;
    delete sprite2;

    // プレイヤーの解放
    delete player;

    // SkyDomeの解放
    delete skyDome;

    Framework::Finalize();
}

void MyGame::CreateFloor() {
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

void MyGame::CreateHandrance() {
    // ブロックモデルを一定間隔で並べる
    int val = GetRandom(0, 20);
    if (val == 0) {
        // ブロックの生成
        Blocks* newBlock1 = new Blocks();
        newBlock1->Initialize(object3DCommon, modelCommon);
        // ブロックの発生位置を設定
        Vector3 HadranceBlockPos1 = { 6.5f, -2.4f, 10.0f };
        newBlock1->PoPBlock(HadranceBlockPos1);
        // ブロックをリストに追加
        hindranceBlocks.push_back(newBlock1);

        // ブロックの生成
        Blocks* newBlock2 = new Blocks();
        newBlock2->Initialize(object3DCommon, modelCommon);
        // ブロックの発生位置を設定
        Vector3 HadranceBlockPos2 = { 6.5f, -1.8f, 10.0f };
        newBlock2->PoPBlock(HadranceBlockPos2);
        // ブロックをリストに追加
        hindranceBlocks.push_back(newBlock2);

        Blocks* newBlock3 = new Blocks();
        newBlock3->Initialize(object3DCommon, modelCommon);
        // ブロックの発生位置を設定
        Vector3 HadranceBlockPos3 = { 6.5f, -1.2f, 10.0f };
        newBlock3->PoPBlock(HadranceBlockPos3);
        // ブロックをリストに追加
        hindranceBlocks.push_back(newBlock3);
    }
    if (val == 1) {
        // ブロックの生成
        Blocks* newBlock1 = new Blocks();
        newBlock1->Initialize(object3DCommon, modelCommon);
        // ブロックの発生位置を設定
        Vector3 HadranceBlockPos1 = { 6.5f, -2.4f, 10.0f };
        newBlock1->PoPBlock(HadranceBlockPos1);
        // ブロックをリストに追加
        hindranceBlocks.push_back(newBlock1);

        // ブロックの生成
        Blocks* newBlock2 = new Blocks();
        newBlock2->Initialize(object3DCommon, modelCommon);
        // ブロックの発生位置を設定
        Vector3 HadranceBlockPos2 = { 6.5f, -1.8f, 10.0f };
        newBlock2->PoPBlock(HadranceBlockPos2);
        // ブロックをリストに追加
        hindranceBlocks.push_back(newBlock2);

        if (val == 2) {
        }
    }
    if (hindranceBlocks.size() >= num) {
        Blocks* oldBlock = hindranceBlocks.front();
        delete oldBlock;
        hindranceBlocks.pop_front();
    }
}

int MyGame::GetRandom(int min, int max) {
    return min + rand() % (max - min + 1);
}

bool MyGame::HitCheck(Vector3 pos1, Vector3 pos2, Vector3 scale1, Vector3 scale2) {
    // 3Dモデルの中心を基準にした当たり判定
    return (pos1.x - scale1.x / 2 < pos2.x + scale2.x / 2 &&
        pos1.x + scale1.x / 2 > pos2.x - scale2.x / 2 &&
        pos1.y - scale1.y / 2 < pos2.y + scale2.y / 2 &&
        pos1.y + scale1.y / 2 > pos2.y - scale2.y / 2 &&
        pos1.z - scale1.z / 2 < pos2.z + scale2.z / 2 &&
        pos1.z + scale1.z / 2 > pos2.z - scale2.z / 2);
}

void MyGame::HitCheckAll() {
    // プレイヤーとブロックの当たり判定
    Vector3 playerPos = player->GetTranslate();
    Vector3 playerScale = player->GetScale();
    bool isColliding = false;

    for (Blocks* block : hindranceBlocks) {
        Vector3 blockPos = block->GetTranslate();
        Vector3 blockScale = block->GetScale();
        if (HitCheck({
            playerPos.x - 0.71f,
            playerPos.y - 0.71f,
            playerPos.z - 0.71f
            }, {
            blockPos.x - 0.71f,
            blockPos.y - 0.71f,
            blockPos.z - 0.71f
            }, { 0.71f ,1.001f ,0.71f }, { 0.71f ,1.001f ,0.71f })) {
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

void MyGame::HandleGameOver() {
    // ゲームオーバー時の処理を実装
    // 例: メッセージを表示、リスタート、メニューに戻るなど
    OutputDebugStringA("Game Over\n");
    // 必要に応じて他の処理を追加
}

void MyGame::ResetGame() {
    // ゲームの状態をリセット
    isGameOver = false;
    cycleCount = 0.0f;

    // プレイヤーの初期化
    delete player;
    player = new Player();
    player->Initialize(object3DCommon, modelCommon, input);

    // ブロックのクリア
    for (Blocks* block : blocks) {
        delete block;
    }
    blocks.clear();

    for (Blocks* block : hindranceBlocks) {
        delete block;
    }
    hindranceBlocks.clear();

    // 足場のブロックを生成
    CreateInitialFloor();

    // スプライトの位置をリセット
    float windowWidth = static_cast<float>(WinAPI::kClientWidth);
    Vector2 pos = { windowWidth, 50.0f };  // 画面右端に初期位置を設定
    sprite->SetPosition(pos);
}

void MyGame::CreateInitialFloor() {
    // 初期の足場のブロックを生成
    for (int i = 0; i < 10; ++i) { // ブロックの数を増やす
        Blocks* newBlock = new Blocks();
        newBlock->Initialize(object3DCommon, modelCommon);
        Vector3 floarBlockPos = { 6.5f - i * 1.0f, -3.0f, 10.0f }; // ブロックの間隔を狭くする
        newBlock->PoPBlock(floarBlockPos);
        blocks.push_back(newBlock);
    }
}
