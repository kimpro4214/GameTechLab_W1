#include "pch.h"
#include <Audio/Audio.h>

Audio::Audio()
{
    TCHAR DropFilePath[] = TEXT("sound/drop.wav");
    FWaveData Drop(DropFilePath);
    WaveDataMap.emplace("Drop", std::move(Drop));

    TCHAR ClickFilePath[] = TEXT("sound/click.wav");
    FWaveData Click(ClickFilePath);
    WaveDataMap.emplace("Click", std::move(Click));

    TCHAR MergePath[] = TEXT("sound/merge.wav");
    FWaveData Merge(MergePath);
    WaveDataMap.emplace("Merge", std::move(Merge));

    TCHAR StorePath[] = TEXT("sound/store.wav");
    FWaveData Store(StorePath);
    WaveDataMap.emplace("Store", std::move(Store));
}

Audio::~Audio()
{

}

Audio& Audio::GetInstance()
{
    static Audio Instance;
    return Instance;
}

HRESULT Audio::CreateDirectSound(HWND hWindow)
{
    HRESULT Hr = DirectSoundCreate8(NULL, DirectSoundPtr.GetAddressOf(), NULL);
    if (FAILED(Hr))
    {
        return (Hr);
    }

    Hr = DirectSoundPtr->SetCooperativeLevel(hWindow, DSSCL_PRIORITY);
    if (FAILED(Hr))
    {
        return (Hr);
    }
    return (Hr);
}

HRESULT Audio::CreateSoundBuffer(FWaveData& _FWaveData)
{
    DSBUFFERDESC Dsbdesc;
    LPDIRECTSOUNDBUFFER DsbPtr = NULL;

    HRESULT hr;

    memset(&Dsbdesc, 0, sizeof(DSBUFFERDESC));
    Dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    Dsbdesc.dwFlags =
        DSBCAPS_CTRLVOLUME |
        DSBCAPS_GLOBALFOCUS;

    Dsbdesc.dwBufferBytes = _FWaveData.DataChunk.cksize;
    Dsbdesc.lpwfxFormat = &_FWaveData.FmtFormat;

    hr = DirectSoundPtr->CreateSoundBuffer(&Dsbdesc, &DsbPtr, NULL);

    if (SUCCEEDED(hr))
    {
        hr = DsbPtr->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)_FWaveData.DirectSoundBufferPtr.GetAddressOf());
        DsbPtr->Release();
    }

    return hr;
}

bool Audio::AllLoadWav()
{
    HRESULT Hr;
    for (auto& [key, value] : WaveDataMap)
    {
        if (LoadWav(value) == -1)
        {
            return (false);
        }
        Hr = CreateSoundBuffer(value);
        if (FAILED(Hr))
        {
            return (false);
        }
    }
    return (true);
}

int Audio::LoadWav(FWaveData& _WaveData)
{
    
    if (_WaveData.FilePath.empty())
    {
        return (-1);
    }

    HMMIO Hmmio = mmioOpen(const_cast<LPTSTR>(_WaveData.FilePath.c_str()), NULL, MMIO_READ | MMIO_ALLOCBUF);

    if (Hmmio == nullptr)
    {
        return (-1);
    }

    _WaveData.RiffChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    MMRESULT MmResult = mmioDescend(Hmmio, &_WaveData.RiffChunk, NULL, MMIO_FINDRIFF);
    if (MmResult != MMSYSERR_NOERROR)
    {
        mmioClose(Hmmio, 0);
        return (-1);
    }

    _WaveData.FmtChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    MmResult = mmioDescend(Hmmio, &_WaveData.FmtChunk, NULL, MMIO_FINDCHUNK);
    if (MmResult != MMSYSERR_NOERROR)
    {
        mmioClose(Hmmio, 0);
        return (-1);
    }
    memset(&_WaveData.FmtFormat, 0, sizeof(_WaveData.FmtFormat));

    mmioRead(Hmmio, (HPSTR)&_WaveData.FmtFormat, _WaveData.FmtChunk.cksize);

    mmioAscend(Hmmio, &_WaveData.FmtChunk, 0);


    _WaveData.DataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    MmResult = mmioDescend(Hmmio, &_WaveData.DataChunk, NULL, MMIO_FINDCHUNK);
    if (MmResult != MMSYSERR_NOERROR)
    {
        mmioClose(Hmmio, 0);
        return (-1);
    }

    
    try
    {
        _WaveData.DataBufferPtr = std::make_unique<char[]>(_WaveData.DataChunk.cksize);
    }
    catch(std::bad_alloc e)
    {
        mmioClose(Hmmio, 0);
        return (-1);
    }

    mmioRead(Hmmio, (HPSTR)_WaveData.DataBufferPtr.get(), _WaveData.DataChunk.cksize);

    mmioClose(Hmmio, 0);
    return(0);

}


int Audio::Play(std::string name)
{
    auto Iteration = WaveDataMap.find(name);
    if (Iteration == WaveDataMap.end())
    {
        return (-1);
    }

    FWaveData& WaveData = Iteration->second;

    if (!WaveData.DirectSoundBufferPtr)
    {
        return (-1);
    }


    LPVOID LpvWrite = nullptr;
    DWORD DwLength = 0;

    if (DS_OK == WaveData.DirectSoundBufferPtr->Lock(
        0, // 락 시작 오프셋. 
        0, // 락 크기; 플래그 때문에 무시됨. 
        &LpvWrite, // 락의 첫 번째 부분 주소. 
        &DwLength, // 락의 첫 번째 부분 크기. 
        NULL, // 래핑 부분 주소는 필요 없음. 
        NULL, // 래핑 부분 크기도 필요 없음. 
        DSBLOCK_ENTIREBUFFER)) // 플래그. 
    {
        DWORD dwCopyLength = std::min(WaveData.DataChunk.cksize, DwLength);

        memcpy(LpvWrite, WaveData.DataBufferPtr.get(), dwCopyLength);
        WaveData.DirectSoundBufferPtr->Unlock(
            LpvWrite, // 락 시작 주소. 
            DwLength, // 락 크기. 
            NULL, // 래핑 부분 없음. 
            0); // 래핑 부분 크기 없음. 
    }
    else
    {
        return (-1);
    }


    WaveData.DirectSoundBufferPtr->SetCurrentPosition(0);
    HRESULT hr = WaveData.DirectSoundBufferPtr->Play(
        0,  // Unused.
        0,  // Priority for voice management.
        0); // Flags.
    if (FAILED(hr))
    {
        return(hr);
    }
    return (0);
}

bool Audio::Initialize(HWND HWindow)
{
    HRESULT Hr;

    Hr = CreateDirectSound(HWindow);
    if (FAILED(Hr))
    {
        return (false);
    }

    if (!AllLoadWav())
    {
        return (false);
    }
    return (true);
}

void Audio::Shutdown()
{
	WaveDataMap.clear();
	DirectSoundPtr.Reset();
}
