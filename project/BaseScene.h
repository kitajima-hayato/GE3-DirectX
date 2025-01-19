#pragma once
#include "DirectXCommon.h"

class SceneManager;

class BaseScene
{
public:
    virtual ~BaseScene() = default;
    // 初期化
    virtual void Initialize(DirectXCommon* dxCommon) = 0;
    // 更新
    virtual void Update() = 0;
    // 描画
    virtual void Draw() = 0;
    // 終了処理
    virtual void Finalize() = 0;
    // Setter
    virtual void SetSceneManager(SceneManager* sceneManager) { this->sceneManager = sceneManager; }

protected:
    // シーンマネージャー
    SceneManager* sceneManager = nullptr;
};

