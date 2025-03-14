#pragma once
#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")
#include <fstream>
#include <wrl.h>
// シングルトンクラス
struct ChunkHeader
{
	char chunkID[4];
	uint32_t chunkSize;
};
struct RIFFHeader
{
	ChunkHeader chunk;
	char type[4];
};
struct FormartChunk {
	ChunkHeader chunk;
	WAVEFORMATEX format;
};
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファのサイズ
	unsigned int bufferSize;
};
class Audio
{
public:
	static Audio* GetInstance();
	static void DeleteInstance();
	
private:
	static Audio* instance;
	Audio() = default;
	~Audio() = default;
	Audio(Audio&) = delete;
	Audio& operator=(Audio&) = delete;


public:
	void Initialize();
	SoundData LoadWave(const char* filename);
	void SoundUnload(SoundData* soundData);
	void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData);

public:
	IXAudio2* GetXAudio2() const { return xaudio2_.Get(); }

private:
	Microsoft::WRL::ComPtr<IXAudio2> xaudio2_;
	// ↓の型にはRelease()が無いので、ComPtrを使わない
	IXAudio2MasteringVoice* masterVoice;
};
