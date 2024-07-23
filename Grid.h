#pragma once
#include"MakeMatrix.h"
#include <Novice.h>

struct Sphere {
	Vector3 center;	//!<中心点
	float radius;	//!<半径
};

//Grid線関数
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix);
//SphereDraw関数
void DrawSphere(const Sphere& sphers, const Matrix4x4& viewPrijection, const Matrix4x4& viewportMatrix, uint32_t color);