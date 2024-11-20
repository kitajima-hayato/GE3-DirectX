#include "Object3D.h"
#include "Object3DCommon.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include "MakeMatrix.h"
using namespace std;
void Object3D::Initialize(Object3DCommon* obj3dCommon)
{
	this->object3DCommon = obj3dCommon;
	modelData = LoadObjFile("resources", "plane.obj");
	CreateVertexResourceData();
	CreateMaterialResource();
	CreateTransformationMatrixData();
}

MaterialData Object3D::LoadMaterialTempLateFile(const std::string& directoryPath, const std::string& filename)
{
	MaterialData materialData;	//構築するMaterialData
	std::string line;			//ファイルから読んだ１行を格納するもの
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());		//開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;
		//identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}

ModelData Object3D::LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	// 1. 必要な変数宣言
	ModelData modelData;     // 構築するModelData
	vector<Vector4> positions;    // 位置
	vector<Vector3> normals;    // 法線
	vector<Vector2> texcoords;    // テクスチャ座標
	string line;       // ファイルから読んだ1行を格納

	// 2. ファイルを開く
	ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());         // 開けなかったら止める

	while (getline(file, line)) {
		string identifier;
		istringstream s(line);
		s >> identifier; // 先頭の識別子を読む

		// identifierに応じた処理
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];
			// 面は三角形限定。その他は未定。
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは　［位置/UV/法線］　で格納されているので分解してIndexを取得する
				istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					string index;
					getline(v, index, '/');      // 区切りでインデックスうぃ読んでいく
					elementIndices[element] = stoi(index);
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				triangle[faceVertex] = { position,texcoord,normal };
			}
			// 頂点を逆順すろことで、回り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			std::string materialFilename;
			s >> materialFilename;
			modelData.material = LoadMaterialTempLateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}

void Object3D::CreateVertexResourceData()
{
	// 頂点バッファーリソースを作成
	vertexResource = object3DCommon->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	// 頂点バッファーリソースにデータを書き込む
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
	// 頂点バッファービューを作成
	// リソースの先頭のアドレスから使用
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//頂点データをリソースにコピー
	memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

}

void Object3D::CreateMaterialResource()
{
	// マテリアルリソースを作成
	materialResource = object3DCommon->GetDxCommon()->CreateBufferResource(sizeof(Material));
	// マテリアルリソースにデータを書き込むためのアドレスを取得してmaterialDataに割り当てる
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// マテリアルデータの初期化
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = 0;
	materialData->uvTransform = MakeIdentity4x4();


}

void Object3D::CreateTransformationMatrixData()
{
	// 変換行列リソースを作成
	wvpResource = object3DCommon->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	// 変換行列リソ
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 変換行列データの初期化
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();

}

void Object3D::CreateDirectionalLightResource()
{
	// 平行光源リソースを作成
	directionalLightResource = object3DCommon->GetDxCommon()->CreateBufferResource(sizeof(DirectionalLight));
	// 平行光源リソースにデータを書き込むためのアドレスを取得してdirectionalLightDataに割り当てる
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// 平行光源データの初期化 / デフォルト値
	// ライトの色
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	// ライトの向き
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	// 輝度
	directionalLightData->intensity = 1.0f;
}

