#include "ModelCommon.h"

ModelCommon::ModelCommon()
{
}

void ModelCommon::Initialize(DirectXCommon* dxCommon)
{
	//引数で受け取ってメンバ変数に記録する
	this->dxCommon_ = dxCommon;
}
