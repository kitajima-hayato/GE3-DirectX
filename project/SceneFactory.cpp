#include "SceneFactory.h"
#include "TitleScene.h"
#include "GamePlayScene.h"
BaseScene* SceneFactory::CreateScene(const std::string& sceneName)
{
    // 次のシーン生成
	BaseScene* scene = nullptr;
	if (sceneName == "TITLE")
	{
		scene = new TitleScene();
	}
	else if (sceneName == "GAMEPLAY")
	{
		scene = new GamePlayScene();
	}
	return scene;
}
