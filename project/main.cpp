#include<Windows.h>
#include<cstdint>
#include<string>
#include<format>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>
#include<dxgidebug.h>
#include<dxcapi.h>
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"
#include<wrl.h>
#include "Framework.h"
#include "MyGame.h"
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
//ウィンドウズアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	
	MyGame game;
	game.Run();
	return 0;
}
