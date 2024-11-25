#pragma once
#include "DirectXCommon.h"
class ModelCommon
{
public:	// メンバ関数
	ModelCommon();
	~ModelCommon();
	// 初期化
	void Initialize(DirectXCommon* dxCommon);
	// 共通描画設定
	void DrawSettingCommon();
public:	// Getter/Setter
	// DirectXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
private:
	// 絶対にnew,deleteしない
	DirectXCommon* dxCommon_;

};

