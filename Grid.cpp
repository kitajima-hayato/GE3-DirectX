#include"Grid.h"

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;		//Grid半分の幅
	const uint32_t  kSubdivision = 10;			//分割数
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);//1つ分の長さ
	//奥から手前への線を順々に引いていく

	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		float x = -kGridHalfWidth + (xIndex * kGridEvery);
		unsigned int color = 0xAAAAAAFF;
		Vector3 start{ x,0.0f,-kGridHalfWidth };
		Vector3 end{ x,0.0f,kGridHalfWidth };

		Vector3 startScreen = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
		Vector3 endScreen = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);
		if (x == 0.0f) {
			color = BLACK;
		}
		Novice::DrawLine((int)startScreen.x, (int)startScreen.y, (int)endScreen.x, (int)endScreen.y, color);
	}
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHalfWidth + (zIndex * kGridEvery);
		unsigned int color = 0xAAAAAAFF;
		Vector3 start{ -kGridHalfWidth,0.0f,z };
		Vector3 end{ kGridHalfWidth,0.0f,z };

		Vector3 startScreen = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
		Vector3 endScreen = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);
		if (z == 0.0f) {
			color = BLACK;
		}
		Novice::DrawLine((int)startScreen.x, (int)startScreen.y, (int)endScreen.x, (int)endScreen.y, color);

	}

}

void DrawSphere(const Sphere& sphers, const Matrix4x4& viewProjection, const Matrix4x4& viewportMatrix, uint32_t color) {
	const uint32_t kSubdivision = 16;		//分割数 16or32
	const float kLatEvery = (float)M_PI / kSubdivision;			//緯度分割１つ分の角度
	const float kLonEvery = ((float)M_PI * 2) / kSubdivision;	//経度分割１つ分の角度	//緯度の方向に分割　-π/2~ π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -(float)M_PI / 2.0f + kLatEvery * latIndex;//現在の緯度
		//経度の方向に分割0~2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;//現在の緯度
			//world座標系でのa,b,cを求める
			Vector3 a, b, c;
			a = { std::cos(lat) * std::cos(lon),
				  std::sin(lat),
				  std::cos(lat) * sin(lon) };
			b = { std::cos(lat + kLatEvery) * std::cos(lon),
				  std::sin(lat + kLatEvery) ,
				  std::cos(lat + kLatEvery) * std::sin(lon) };
			c = { std::cos(lat) * std::cos(lon + kLonEvery),
				  std::sin(lat),
				  std::cos(lat) * std::sin(lon + kLonEvery) };

			Vector3 startPA = { sphers.radius * a.x + sphers.center.x ,sphers.radius * a.y + sphers.center.y,sphers.radius * a.z + sphers.center.z };
			Vector3 startPB = { sphers.radius * b.x + sphers.center.x ,sphers.radius * b.y + sphers.center.y,sphers.radius * b.z + sphers.center.z };
			Vector3 startPC = { sphers.radius * c.x + sphers.center.x ,sphers.radius * c.y + sphers.center.y,sphers.radius * c.z + sphers.center.z };


			Vector3 startScreenA = Transform(Transform(startPA, viewProjection), viewportMatrix);
			Vector3 endScreenB = Transform(Transform(startPB, viewProjection), viewportMatrix);
			Vector3 endScreenC = Transform(Transform(startPC, viewProjection), viewportMatrix);


			Novice::DrawLine((int)startScreenA.x, (int)startScreenA.y, (int)endScreenB.x, (int)endScreenB.y, color);
			Novice::DrawLine((int)startScreenA.x, (int)startScreenA.y, (int)endScreenC.x, (int)endScreenC.y, color);
			//a,b,cをScreen座標系まで変換
			//ab,acで線を引く
		}



		//DrawLineをつくる
	}
}