#pragma once
#include "BaseScene.h"
#include <string>
// シーン工場クラス
class AbstractSceneFactory
{
public:
	// 仮想デストラクタv
	virtual ~AbstractSceneFactory() = default;
	// シーン生成
	virtual BaseScene* CreateScene(const std::string&sceneName) = 0;
};

