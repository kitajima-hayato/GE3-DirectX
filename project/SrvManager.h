#pragma once
#include "DirectXCommon.h"
class SrvManager
{
public:
	void Initialize(DirectXCommon* dxCommon);
private:
	// 絶対にnewしない
	DirectXCommon* dxCommon;
};

