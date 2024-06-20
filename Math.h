#pragma once
#include"MakeMatrix.h"
struct Vector2 {
	float x;
	float y;
};

struct Vector4 {
	float x;
	float y;
	float z;
	float w;
};
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};
struct Material {
	Vector4 color;
	int32_t enableLighting;
};
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};
struct DirectionalLight {
	Vector4 color;		//ライトの色
	Vector3 direction;	//ライトの向き
	float intensity;	//輝度
};

