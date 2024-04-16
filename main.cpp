#include<Windows.h>

//ウィンドウズアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	//出力ウィンドウへの文字出力　実行すると出る下の文字
	OutputDebugStringA("Hello,DirectX!\n");
	return 0;
}