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

    // 加算演算子
    Vector3 operator+(const Vector3& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }

    // 減算演算子
    Vector3 operator-(const Vector3& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }

    // スカラー乗算演算子
    Vector3 operator*(float scalar) const {
        return { x * scalar, y * scalar, z * scalar };
    }

    // スカラー除算演算子
    Vector3 operator/(float scalar) const {
        return { x / scalar, y / scalar, z / scalar };
    }

    // 加算代入演算子
    Vector3& operator+=(const Vector3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    // 減算代入演算子
    Vector3& operator-=(const Vector3& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    // スカラー乗算代入演算子
    Vector3& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    // スカラー除算代入演算子
    Vector3& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    // 等価演算子
    bool operator==(const Vector3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    // 不等価演算子
    bool operator!=(const Vector3& other) const {
        return !(*this == other);
    }
};

struct Vector4 {
	float x;
	float y;
	float z;
	float w;
};
#pragma endregion
#pragma region 行列
struct Matrix3x3 {
	float m[3][3];
};
struct Matrix4x4 {
	float m[4][4];
};
#pragma endregion

struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex = 0;
};
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};
struct ModelData {
	std::vector<VertexData>vertices;
	MaterialData material;
};
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
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
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};
struct SpriteData {
	Vector2 pos;
	Vector2 size;
	float rotate;
	Vector4 color;
};
struct AABB {
	Vector3 min;//最小点
	Vector3 max;//最大点
};
struct AccelerationField {
	Vector3 acceleration;
	AABB area;
};

#pragma region パーティクル構造体
// パーティクル構造体

struct Particle
{
	Transform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};
struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};


#pragma endregion