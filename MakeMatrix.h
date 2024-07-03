#pragma once
#include <assert.h>
#include <cmath>
#include <stdio.h>
#include"Math.h"



struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};
Matrix4x4 Add(const Matrix4x4& mt1, const Matrix4x4& mt2);

Matrix4x4 Subtract(const Matrix4x4& mt1, const Matrix4x4& mt2);

Matrix4x4 Multiply(const Matrix4x4& mt1, const Matrix4x4& mt2);

Matrix4x4 Inverse(const Matrix4x4& mt1);

//Matrix4x4 Transpose(const Matrix4x4& mt1);

Matrix4x4 MekeIdentity4x4();
// 回転
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
// 拡大
Matrix4x4 MakeScaleMatrix(const Vector3& scale);
// 同時座標変換

// 回転X
Matrix4x4 MakeRotateXMatrix(float radian);
// 回転Y
Matrix4x4 MakeRotateYMatrix(float radian);
// 回転Z
Matrix4x4 MakeRotateZMatrix(float radian);

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);


//正射影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
	