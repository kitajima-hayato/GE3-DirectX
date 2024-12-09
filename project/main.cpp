#include<Windows.h>
#include<cstdint>
#include<string>
#include<format>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>
#include<dxgidebug.h>
#include<dxcapi.h>
#include"Math.h"
#include"MakeMatrix.h"

#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"
#include<vector>
#include<fstream>
#include<sstream>
#include<wrl.h>
#include "numbers"

#include "MyGame.h"
#ifdef _DEBUG
#include "ImGuiManager.h"
#endif

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
using namespace Logger;
using namespace std;

//ウィンドウズアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	


	MyGame game;

	
	// ゲームの初期化
	game.Initialize();


	// ウィンドウの×ボタンが押されるまでループ
	while (true) {		// ゲームループ
		// ゲームの更新
		game.Update();
		






		// ウィンドウの×ボタンが押されたらループを抜ける
		if (game.IsEndRequst()) {
			break;
		}
		// ゲームの描画
		game.Draw();
	}
	// ゲームの終了処理
	game.Finalize();

	return 0;
}
