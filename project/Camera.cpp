#include "Camera.h"

Camera::Camera()
	: transform({ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} })
	, fovY(0.45f)
	, aspectRatio(float(WinAPI::kClientWidth) / float(WinAPI::kClientHeight))
	, nearClip(0.1f)
	, farClip(100.0f)
	, worldMatrix(MakeAffineMatrix(transform.scale,transform.rotate,transform.translate))
	, viewMatrix(Inverse(worldMatrix))
	, projectionMatrix(MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip))
	, viewProjectionMatrix(Multiply(viewMatrix, projectionMatrix))
{

}

void Camera::Initialize()
{
}

void Camera::Update()
{
	// トランスフォームから変換行列を計算
	worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	// ワールドマトリックスの逆行列
	viewMatrix = Inverse(worldMatrix);
	// プロジェクション行列更新
	projectionMatrix = MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	// ビューマトリックスの計算
	viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);

}

void Camera::Draw()
{
}
