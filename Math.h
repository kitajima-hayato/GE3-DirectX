#pragma once
struct Vector2 {
	float x;
	float y;
};
struct Vector3 {
	float x;
	float y;
	float z;
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