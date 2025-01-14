#include "Audio.h"
#include <xaudio2.h> // Add this include for XAudio2
#include <cassert>

Audio::Audio()
{
}

Audio::~Audio()
{
}

void Audio::Initialize()
{
	HRESULT hr;
	hr = XAudio2Create(&xaudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	hr = xaudio2_->CreateMasteringVoice(&masterVoice);
}

SoundData Audio::LoadWave(const char* filename)
{
    // ファイルを開く
    std::ifstream file(filename, std::ios_base::binary);
    // ファイルが開けなかったら検出
    assert(file.is_open());
    // RIFFヘッダーを読み込む
    RIFFHeader riff;
    file.read((char*)&riff, sizeof(riff));
    // ファイルがRIFFかチェック
    assert(strncmp(riff.chunk.chunkID, "RIFF", 4) == 0);
    // タイプがWAVEかチェック
    assert(strncmp(riff.type, "WAVE", 4) == 0);

    // フォーマットチャンクを読み込む
    FormartChunk fmt;
    file.read((char*)&fmt, sizeof(ChunkHeader));
    assert(strncmp(fmt.chunk.chunkID, "fmt ", 4) == 0);
    // チャンク本体の読み込み
    assert(fmt.chunk.chunkSize <= sizeof(fmt.format));
    file.read((char*)&fmt.format, fmt.chunk.chunkSize);

    // データチャンクを探して読み込む
    ChunkHeader data;
    while (file.read((char*)&data, sizeof(data)))
    {
        if (strncmp(data.chunkID, "data", 4) == 0)
        {
            break;
        }
        // 読み取り位置を次のチャンクに移動
        file.seekg(data.chunkSize, std::ios_base::cur);
    }
    assert(strncmp(data.chunkID, "data", 4) == 0);

    // データチャンクのデータ部の読み込み
    char* pBuffer = new char[data.chunkSize];
    file.read(pBuffer, data.chunkSize);

    // ファイルを閉じる
    file.close();

    // データの設定
    SoundData soundData;
    soundData.wfex = fmt.format;
    soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
    soundData.bufferSize = data.chunkSize;

    return soundData;
}


void Audio::SoundUnload(SoundData* soundData)
{
	// バッファの解放
	delete[] soundData->pBuffer;
	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};

}

void Audio::SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData)
{
	HRESULT hr;
	// ソースボイスの作成
	IXAudio2SourceVoice* sourceVoice = nullptr;
	hr = xAudio2->CreateSourceVoice(&sourceVoice, reinterpret_cast<const WAVEFORMATEX*>(&soundData.wfex));
	assert(SUCCEEDED(hr));
	assert(SUCCEEDED(hr));

	// 再生するデータの設定
	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = soundData.bufferSize;
	buffer.pAudioData = soundData.pBuffer;
	buffer.Flags = XAUDIO2_END_OF_STREAM;
		
	// 再生
	hr = sourceVoice->SubmitSourceBuffer(&buffer);
	hr = sourceVoice->Start();
}

void Audio::Play()
{
}

void Audio::Stop()
{
}

void Audio::Finalize()
{
}
