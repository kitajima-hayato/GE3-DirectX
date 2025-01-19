#pragma once
#include "BaseScene.h"
#include "AbstractSceneFactory.h"
// シーン管理クラス
// シングルトンクラス
class SceneManager
{
public:
	// インスタンスの取得
	static SceneManager* GetInstance();
	// コンストラクタ
	SceneManager() = default;
	// デストラクタ 
	~SceneManager() = default;
	// コピーコンストラクタ
	SceneManager(SceneManager&) = delete;
	// 代入演算子のオーバーロード
	SceneManager& operator=(SceneManager&) = delete;

	// 更新
	void Update(DirectXCommon*dxCommon);
	// 描画
	void Draw();
	// 終了処理
	void Finalize();

public:
	// シーンファクトリーのSetter
	void SetSceneFactory(AbstractSceneFactory* sceneFactory) { sceneFactory_ = sceneFactory; }
	// シーンチェンジ
	void ChangeScene(const std::string& sceneName);

private:
	// 今のシーン
	BaseScene* scene_ = nullptr;
	// 次のシーン
	BaseScene* nextScene_ = nullptr;

	// シーンファクトリー // 借り物
	AbstractSceneFactory* sceneFactory_ = nullptr;
	
};

