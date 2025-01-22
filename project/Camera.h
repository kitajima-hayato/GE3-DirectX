#pragma once
#include "MyMath.h"
#include "MakeMatrix.h"
#include "WinAPI.h"
class Camera
{
public:	// メンバ関数
	Camera();
	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();
		
public:	
	// setter
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }
	void SetFovY(float fov) { this->fovY = fov; }
	void SetAspectRatio(float aspect) { this->aspectRatio = aspect; }
	void SetNearClip(float nearClip) { this->nearClip = nearClip; }
	void SetFarClip(float farClip) { this->farClip = farClip; }
	// getter
	const Matrix4x4& GetCameraMatrix()const { return worldMatrix; }
	const Matrix4x4& GetViewMatrix()const { return viewMatrix; }
	const Matrix4x4& GetProjectionMatrix()const { return projectionMatrix; }
	const Matrix4x4& GetViewProjectionMatrix()const { return viewProjectionMatrix; }
	const Vector3& GetScale()const { return transform.scale; }
	const Vector3& GetRotate()const { return transform.rotate; }
	const Vector3& GetTranslate()const { return transform.translate; }


private:
	Transform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;

	Matrix4x4 projectionMatrix;
	// 水平方向視野角
	float fovY = 0.45f;
	// アスペクト比
	float aspectRatio = float(WinAPI::kClientWidth) / float(WinAPI::kClientHeight);
	// ニアクリップ
	float nearClip = 0.1f;
	// ファークリップ
	float farClip = 100.0f;

	// ビュープロジェクション行列
	Matrix4x4 viewProjectionMatrix;


};

