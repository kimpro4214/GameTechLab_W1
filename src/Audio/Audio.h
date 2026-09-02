#pragma once

#include <DSound.h>
#include <mmSystem.h>
#include <wrl/client.h>
#include <comdef.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "Winmm.lib")

using Microsoft::WRL::ComPtr;

struct FWaveData {
	std::wstring	FilePath;

	WAVEFORMATEX	FmtFormat;

	MMCKINFO		DataChunk;
	MMCKINFO		FmtChunk;
	MMCKINFO		RiffChunk;

	std::unique_ptr<char[]> DataBufferPtr;
	ComPtr<IDirectSoundBuffer8> DirectSoundBufferPtr;

	FWaveData(const std::wstring &_FilePath) :
		FilePath(_FilePath), FmtFormat({}), DataChunk({}), FmtChunk({}), RiffChunk({})
	{
	}
};


class Audio
{

public:
	// 싱글톤이라 복사/대입 금지
	Audio(const Audio&) = delete;
	Audio& operator=(const Audio&) = delete;
	
	static Audio& GetInstance();

	int Play(std::string name);
	void Initializer(HWND hWindow);
	void Shutdown();

private:
	Audio();
	~Audio();

	HRESULT CreateDirectSound(HWND hWindow);
	HRESULT CreateSoundBuffer(FWaveData& _FWaveData);
	void AllLoadWav();
	int LoadWav(FWaveData& _WaveData);

	std::unordered_map<std::string, FWaveData> WaveDataMap;
	ComPtr<IDirectSound8> DirectSoundPtr;
};
