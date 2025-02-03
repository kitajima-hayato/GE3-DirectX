#include "SceneManager.h"

static SceneManager* instance = nullptr;


SceneManager* SceneManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new SceneManager();
	}
	return instance;
}

void SceneManager::Deletenstance()
{
	if (instance != nullptr)
	{
		delete instance;
		instance = nullptr;
	}
}

void SceneManager::Update(DirectXCommon* dxCommon)
{
    // 次のシーンが予約されている場合
    if (nextScene_) {
        // 今のシーンの終了処理
        if (scene_) {
            scene_->Finalize();
            delete scene_;
        }
        // 次のシーンの初期化
        scene_ = nextScene_;
        nextScene_ = nullptr;
        // シーンマネージャーの設定
        scene_->SetSceneManager(this);
        scene_->Initialize(dxCommon);
    }
    // 実行中シーンを更新
    if (scene_) {
        scene_->Update();
    }
}


void SceneManager::Draw()
{
    // 実行中シーンが存在する場合のみ描画
    if (scene_) {
        scene_->Draw();
    }
}

void SceneManager::Finalize()
{
    // 最後のシーンの終了処理
    // 使えるようなら
    if (scene_)
    {
        scene_->Finalize();
        delete scene_;
    }
}

void SceneManager::ChangeScene(const std::string& sceneName)
{
    assert(nextScene_ == nullptr);
    assert(sceneFactory_);

	// 次のシーン生成
	nextScene_ = sceneFactory_->CreateScene(sceneName);
}




