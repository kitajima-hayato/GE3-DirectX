#pragma once
#include<vector>

#include<string>
#pragma region ベクトル
struct Vector2 {
	float x;
	float y;
};
struct Vector3 {
	float x;
	float y;
	float z;
	Vector3 operator+=(const Vector3& other) const {
		return Vector3(x + other.x, y + other.y, z + other.z);
	}
	Vector3 operator-=(const Vector3& other) const {
		return Vector3(x - other.x, y - other.y, z - other.z);
	}
	Vector3 operator*=(float scale) const {
		return Vector3(x * scale, y * scale, z * scale);
	}
	Vector3 operator/=(float scale) const {
		return Vector3(x / scale, y / scale, z / scale);
	}
};
struct Vector4 {
	float x;
	float y;
	float z;
	float w;
};
#pragma endregion
struct Matrix3x3 {
	float m[3][3];
};
struct Matrix4x4 {
	float m[4][4];
};

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
	float shininess;
};
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};
struct DirectionalLight {
	Vector4 color; //ライトの色
	Vector3 direction; //ライトの向き
	float intensity;	//輝度
};

struct MaterialData {
	std::string textureFilePath;
};
struct ModelData {
	std::vector<VertexData>vertices;
	MaterialData material;
};