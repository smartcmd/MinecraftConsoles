#include "stdafx.h"
#include "../../../Minecraft.World/net.minecraft.world.entity.h"
#include "../../../Minecraft.World/Mth.h"
#include "../../../Minecraft.World/Random.h"
#include "../../../Minecraft.World/LevelData.h"
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "SoundEngine.h"
#include "../../TexturePackRepository.h"
#include "../../TexturePack.h"
#include "../../Common/DLC/DLCAudioFile.h"
#include "../../DLCTexturePack.h"


IXAudio2* g_pXAudio2 = nullptr;       // pointer to XAudio2 instance used by QNet and XACT
IXAudio2MasteringVoice* g_pXAudio2MasteringVoice = nullptr;  // pointer to XAudio2 mastering voice

IXACT3Engine *SoundEngine::m_pXACT3Engine = nullptr;
IXACT3WaveBank	*SoundEngine::m_pWaveBank = nullptr;
IXACT3WaveBank	*SoundEngine::m_pWaveBank2 = nullptr;
IXACT3WaveBank	*SoundEngine::m_pStreamedWaveBank = nullptr;
IXACT3WaveBank	*SoundEngine::m_pStreamedWaveBankAdditional = nullptr;
IXACT3SoundBank	*SoundEngine::m_pSoundBank = nullptr;
IXACT3SoundBank	*SoundEngine::m_pSoundBank2 = nullptr;
CRITICAL_SECTION SoundEngine::m_CS;

X3DAUDIO_HANDLE	 SoundEngine::m_xact3dInstance;
vector<SoundEngine::soundInfo *> SoundEngine::currentSounds;
X3DAUDIO_DSP_SETTINGS	SoundEngine::m_DSPSettings;
X3DAUDIO_EMITTER		SoundEngine::m_emitter;
X3DAUDIO_LISTENER		SoundEngine::m_listeners[4];
int						SoundEngine::m_validListenerCount = 0;

X3DAUDIO_DISTANCE_CURVE_POINT SoundEngine::m_VolumeCurvePoints[2] = {
	{0.0f, 1.0f},
	{1.0f, 0.0f},
};

X3DAUDIO_DISTANCE_CURVE_POINT SoundEngine::m_DragonVolumeCurvePoints[2] = {
	{0.0f, 1.0f},
	{1.0f, 0.5f},
};
X3DAUDIO_DISTANCE_CURVE_POINT SoundEngine::m_VolumeCurvePointsNoDecay[2] = {
	{0.0f, 1.0f},
	{1.0f, 1.0f},
};

X3DAUDIO_DISTANCE_CURVE	SoundEngine::m_VolumeCurve;
X3DAUDIO_DISTANCE_CURVE	SoundEngine::m_DragonVolumeCurve;
X3DAUDIO_DISTANCE_CURVE	SoundEngine::m_VolumeCurveNoDecay;

void SoundEngine::setXACTEngine( IXACT3Engine *pXACT3Engine)
{
	m_pXACT3Engine = pXACT3Engine;
}

void SoundEngine::destroy()
{
}

SoundEngine::SoundEngine()
{
	random = new Random();
	noMusicDelay = random->nextInt(20 * 60 * 10);

	ZeroMemory(&m_MusicInfo,sizeof(soundInfo));
	//bIsPlayingStreamingCDMusic=false;
	//m_bIsPlayingStreamingGameMusic=false;
	SetIsPlayingEndMusic(false);
	SetIsPlayingNetherMusic(false);
	m_VolumeCurve.PointCount = 2;
	m_VolumeCurve.pPoints = m_VolumeCurvePoints;
	m_DragonVolumeCurve.PointCount = 2;
	m_DragonVolumeCurve.pPoints = m_DragonVolumeCurvePoints;
	m_VolumeCurveNoDecay.PointCount = 2;
	m_VolumeCurveNoDecay.pPoints = m_VolumeCurvePointsNoDecay;

	m_bStreamingMusicReady=false;
	m_bStreamingWaveBank1Ready=false;
	m_bStreamingWaveBank2Ready=false;
}

void SoundEngine::init(Options *pOptions)
{
	InitializeCriticalSection(&m_CS);

	// Iniatialise XACT itself
	HRESULT hr;
	if ( FAILED ( hr = XACT3CreateEngine( 0, &m_pXACT3Engine ) ) )
	{
		app.FatalLoadError();
		assert( false );
		return;
	}

	// Load global settings file
	// 4J-PB - move this to the title update, since we've corrected it to allow sounds to be pitch varied when they weren't before
	HANDLE file;
#ifdef _TU_BUILD
	file = CreateFile("UPDATE:\\res\\audio\\Minecraft.xgs", GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#else
	file = CreateFile("GAME:\\res\\TitleUpdate\\audio\\Minecraft.xgs", GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif
	if( file == INVALID_HANDLE_VALUE )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}
	DWORD dwFileSize = GetFileSize(file,nullptr);
	DWORD bytesRead = 0;
	DWORD memFlags = MAKE_XALLOC_ATTRIBUTES(0,FALSE,TRUE,FALSE,0,XALLOC_PHYSICAL_ALIGNMENT_DEFAULT,XALLOC_MEMPROTECT_READWRITE,FALSE,XALLOC_MEMTYPE_PHYSICAL);
	void *pvGlobalSettings =  XMemAlloc(dwFileSize, memFlags);
	ReadFile(file,pvGlobalSettings,dwFileSize,&bytesRead,nullptr);
	CloseHandle(file);

	XACT_RUNTIME_PARAMETERS EngineParameters = {0};
	EngineParameters.lookAheadTime = XACT_ENGINE_LOOKAHEAD_DEFAULT;
	EngineParameters.fnNotificationCallback = &this->XACTNotificationCallback;
	EngineParameters.pGlobalSettingsBuffer = pvGlobalSettings;
	EngineParameters.globalSettingsBufferSize = dwFileSize;
	EngineParameters.globalSettingsFlags = XACT_FLAG_GLOBAL_SETTINGS_MANAGEDATA;
	EngineParameters.pXAudio2 = g_pXAudio2;
	EngineParameters.pMasteringVoice = g_pXAudio2MasteringVoice;

	if ( FAILED ( hr = m_pXACT3Engine->Initialize( &EngineParameters ) ) )
	{
		app.FatalLoadError();
		assert( false );
		return;
	}

	//	printf("XACT initialisation complete\n");

	// Initialise X3D
	XACT3DInitialize(m_pXACT3Engine,m_xact3dInstance);

	// Set up common structures that can be re-used between sounds & just have required bits updated
	memset(&m_DSPSettings,0,sizeof(X3DAUDIO_DSP_SETTINGS));
	WAVEFORMATEXTENSIBLE format;
	m_pXACT3Engine->GetFinalMixFormat(&format);
	m_DSPSettings.SrcChannelCount = 1;
	m_DSPSettings.DstChannelCount = format.Format.nChannels;
	//	printf("%d channels\n", format.Format.nChannels);
	m_DSPSettings.pMatrixCoefficients = new FLOAT32[m_DSPSettings.SrcChannelCount * m_DSPSettings.DstChannelCount];

	for( int i = 0; i < 4; i++ )
	{
		memset(&m_listeners[i],0,sizeof(X3DAUDIO_LISTENER));
		m_listeners[i].OrientFront.z = 1.0f;
		m_listeners[i].OrientTop.y = 1.0f;
	}
	m_validListenerCount = 1;
	memset(&m_emitter,0,sizeof(X3DAUDIO_EMITTER));
	m_emitter.ChannelCount = 1;
	m_emitter.pVolumeCurve = &m_VolumeCurve;
	m_emitter.pLFECurve = &m_VolumeCurve;
	m_emitter.CurveDistanceScaler = 16.0f;
	m_emitter.OrientFront.z = 1.0f;
	m_emitter.OrientTop.y = 1.0f;

	// Create resident wave bank - leave memory for this managed by xact so it can free it

	file = CreateFile("GAME:\\res\\audio\\resident.xwb", GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if( file == INVALID_HANDLE_VALUE )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}

	dwFileSize = GetFileSize(file,nullptr);
	void *pvWaveBank =  XMemAlloc(dwFileSize, memFlags);
	ReadFile(file,pvWaveBank,dwFileSize,&bytesRead,nullptr);
	CloseHandle(file);

	if ( FAILED( hr = m_pXACT3Engine->CreateInMemoryWaveBank( pvWaveBank, dwFileSize, XACT_FLAG_ENGINE_CREATE_MANAGEDATA, memFlags, &m_pWaveBank ) ) )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}

	// 4J-PB - add new sounds wavebank
#ifdef _TU_BUILD
	file = CreateFile("UPDATE:\\res\\audio\\additional.xwb", GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#else
	file = CreateFile("GAME:\\res\\TitleUpdate\\audio\\additional.xwb", GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif
	if( file == INVALID_HANDLE_VALUE )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}

	dwFileSize = GetFileSize(file,nullptr);
	void *pvWaveBank2 =  XMemAlloc(dwFileSize, memFlags);
	ReadFile(file,pvWaveBank2,dwFileSize,&bytesRead,nullptr);
	CloseHandle(file);

	if ( FAILED( hr = m_pXACT3Engine->CreateInMemoryWaveBank( pvWaveBank2, dwFileSize, XACT_FLAG_ENGINE_CREATE_MANAGEDATA, memFlags, &m_pWaveBank2 ) ) )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}

	// Create streamed sound bank

	file = CreateFile("GAME:\\res\\audio\\streamed.xwb", GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, nullptr);

	if( file == INVALID_HANDLE_VALUE )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}

	XACT_WAVEBANK_STREAMING_PARAMETERS streamParams;
	streamParams.file = file;
	streamParams.offset = 0;
	streamParams.flags = 0;
	streamParams.packetSize = 16;	// Not sure what to pick for this - suggests a "multiple of 16" for DVD playback

	if ( FAILED( hr = m_pXACT3Engine->CreateStreamingWaveBank( &streamParams, &m_pStreamedWaveBank ) ) )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}

	// Create streamed sound bank

	//file = CreateFile("GAME:\\res\\audio\\AdditionalMusic.xwb", GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, nullptr);
#ifdef _TU_BUILD
	file = CreateFile("UPDATE:\\res\\audio\\AdditionalMusic.xwb", GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#else
	file = CreateFile("GAME:\\res\\TitleUpdate\\audio\\AdditionalMusic.xwb", GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif
	if( file == INVALID_HANDLE_VALUE )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}

	streamParams.file = file;
	streamParams.offset = 0;
	streamParams.flags = 0;
	streamParams.packetSize = 16;	// Not sure what to pick for this - suggests a "multiple of 16" for DVD playback

	if ( FAILED( hr = m_pXACT3Engine->CreateStreamingWaveBank( &streamParams, &m_pStreamedWaveBankAdditional ) ) )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}

	// Create sound bank - leave memory for this managed by xact so it can free it
	// 4J-PB - updated for the TU
	//file = CreateFile("GAME:\\res\\audio\\minecraft.xsb", GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#ifdef _TU_BUILD
	file = CreateFile("UPDATE:\\res\\audio\\minecraft.xsb", GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#else
	file = CreateFile("GAME:\\res\\TitleUpdate\\audio\\minecraft.xsb", GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif
	if( file == INVALID_HANDLE_VALUE )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}
	dwFileSize = GetFileSize(file,nullptr);
	void *pvSoundBank =  XMemAlloc(dwFileSize, memFlags);
	ReadFile(file,pvSoundBank,dwFileSize,&bytesRead,nullptr);
	CloseHandle(file);

	if ( FAILED( hr = m_pXACT3Engine->CreateSoundBank( pvSoundBank, dwFileSize, XACT_FLAG_ENGINE_CREATE_MANAGEDATA, memFlags, &m_pSoundBank ) ) )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}

	// Create sound bank2 - leave memory for this managed by xact so it can free it

#ifdef _TU_BUILD
	file = CreateFile("UPDATE:\\res\\audio\\additional.xsb", GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#else
	file = CreateFile("GAME:\\res\\TitleUpdate\\audio\\additional.xsb", GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif
	if( file == INVALID_HANDLE_VALUE )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}
	dwFileSize = GetFileSize(file,nullptr);
	void *pvSoundBank2 =  XMemAlloc(dwFileSize, memFlags);
	ReadFile(file,pvSoundBank2,dwFileSize,&bytesRead,nullptr);
	CloseHandle(file);

	if ( FAILED( hr = m_pXACT3Engine->CreateSoundBank( pvSoundBank2, dwFileSize, XACT_FLAG_ENGINE_CREATE_MANAGEDATA, memFlags, &m_pSoundBank2 ) ) )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}

	XACT_NOTIFICATION_DESCRIPTION desc = {0};
	desc.flags = XACT_FLAG_NOTIFICATION_PERSIST;
	desc.type = XACTNOTIFICATIONTYPE_WAVEBANKPREPARED;
	desc.pvContext=this;
	m_pXACT3Engine->RegisterNotification(&desc);

	// get the category to manage the sfx (Default)
	m_xactSFX = m_pXACT3Engine->GetCategory("Default");
	m_xactMusic = m_pXACT3Engine->GetCategory("Music");
}

void SoundEngine::CreateStreamingWavebank(const char *pchName, IXACT3WaveBank **ppStreamedWaveBank)
{
	// Create streamed sound bank
	HRESULT hr;

	HANDLE file = CreateFile(pchName, GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, nullptr);

	if( file == INVALID_HANDLE_VALUE )
	{ 
		app.FatalLoadError();
		assert(false);
		return;
	}

	XACT_WAVEBANK_STREAMING_PARAMETERS streamParams;
	streamParams.file = file;
	streamParams.offset = 0;
	streamParams.flags = 0;
	streamParams.packetSize = 16;	// Not sure what to pick for this - suggests a "multiple of 16" for DVD playback

	if ( FAILED( hr = m_pXACT3Engine->CreateStreamingWaveBank( &streamParams, ppStreamedWaveBank ) ) )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}
}

void SoundEngine::CreateSoundbank(const char *pchName, IXACT3SoundBank **ppSoundBank)
{
	HRESULT hr;
	HANDLE file = CreateFile(pchName, GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if( file == INVALID_HANDLE_VALUE )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}
	DWORD dwFileSize = GetFileSize(file,nullptr);
	DWORD bytesRead = 0;
	DWORD memFlags = MAKE_XALLOC_ATTRIBUTES(0,FALSE,TRUE,FALSE,0,XALLOC_PHYSICAL_ALIGNMENT_DEFAULT,XALLOC_MEMPROTECT_READWRITE,FALSE,XALLOC_MEMTYPE_PHYSICAL);
	void *pvSoundBank =  XMemAlloc(dwFileSize, memFlags);
	ReadFile(file,pvSoundBank,dwFileSize,&bytesRead,nullptr);
	CloseHandle(file);

	if ( FAILED( hr = m_pXACT3Engine->CreateSoundBank( pvSoundBank, dwFileSize, XACT_FLAG_ENGINE_CREATE_MANAGEDATA, memFlags, ppSoundBank ) ) )
	{
		app.FatalLoadError();
		assert(false);
		return;
	}
}

bool SoundEngine::isStreamingWavebankReady()
{
	if(m_bStreamingMusicReady==false)
	{
		DWORD dwState;
		m_pSoundBank->GetState(&dwState);
		if(dwState&XACT_WAVEBANKSTATE_PREPARED)
		{
			m_bStreamingWaveBank1Ready=true;
		}
		m_pSoundBank2->GetState(&dwState);
		if(dwState&XACT_WAVEBANKSTATE_PREPARED)
		{
			m_bStreamingWaveBank2Ready=true;
		}

		if(m_bStreamingWaveBank1Ready && m_bStreamingWaveBank2Ready)
		{
			m_bStreamingMusicReady=true;
		}
	}

	return m_bStreamingMusicReady;
}

#ifdef _XBOX
bool SoundEngine::isStreamingWavebankReady(IXACT3WaveBank *pWaveBank)
{
	DWORD dwState;
	pWaveBank->GetState(&dwState);
	if(dwState&XACT_WAVEBANKSTATE_PREPARED)
	{
		return true;
	}
	else
	{
		return false;
	}
}
#endif

void SoundEngine::XACTNotificationCallback( const XACT_NOTIFICATION* pNotification )
{
	if(pNotification->pvContext!= nullptr)
	{
		if(pNotification->type==XACTNOTIFICATIONTYPE_WAVEBANKPREPARED)
		{
			SoundEngine *pSoundEngine=static_cast<SoundEngine *>(pNotification->pvContext);
			if(pNotification->waveBank.pWaveBank==pSoundEngine->m_pStreamedWaveBank)
			{
				pSoundEngine->m_bStreamingWaveBank1Ready=true;
			}
			if(pNotification->waveBank.pWaveBank==pSoundEngine->m_pStreamedWaveBankAdditional)
			{
				pSoundEngine->m_bStreamingWaveBank2Ready=true;
			}

			if(pSoundEngine->m_bStreamingWaveBank1Ready && pSoundEngine->m_bStreamingWaveBank2Ready)
			{
				pSoundEngine->m_bStreamingMusicReady=true;
			}
		}
	}
}

char *SoundEngine::ConvertSoundPathToName(const wstring& name, bool bConvertSpaces)
{
	static char buf[256];
	assert(name.length()<256);
	for(unsigned int i = 0; i < name.length(); i++ )
	{
		wchar_t c = name[i];
		if(c=='.') c='_';
		buf[i] = static_cast<char>(c);
	}
	buf[name.length()] = 0;
	return buf;
}

void SoundEngine::play(int iSound, float x, float y, float z, float volume, float pitch)
{
	if(iSound==-1)
	{
		app.DebugPrintf(6,"PlaySound with sound of -1 !!!!!!!!!!!!!!!\n");
		return;
	}

	bool bSoundbank1=(iSound<=eSoundType_STEP_SAND);

	if( (m_pSoundBank == nullptr ) || (m_pSoundBank2 == nullptr))return;

	if( currentSounds.size() > MAX_POLYPHONY )
	{
		return;
	}
	wstring name = wchSoundNames[iSound];
	//const unsigned char *name=ucSoundNames[iSound];

	char *xboxName = ConvertSoundPathToName(name);
	XACTINDEX idx;

	if(bSoundbank1)
	{
		idx = m_pSoundBank->GetCueIndex(xboxName);
	}
	else
	{
		idx = m_pSoundBank2->GetCueIndex(xboxName);
	}

	if( idx == XACTINDEX_INVALID )
	{
#ifndef _CONTENT_PACKAGE
#ifdef _DEBUG
		__debugbreak();
#endif
		//wprintf(L"WARNING: Sound cue not found - %ls\n", name.c_str() );
		app.DebugPrintf("Not found: %s\n",xboxName);
#endif
		return;
	}

	// 4J-PB - check how many of this cue are already playing and ignore if there are loads
	int iSameSoundC=0;
	for( unsigned int i = 0; i < currentSounds.size(); i++ )
	{
		SoundEngine::soundInfo *info = currentSounds[i];

		if((info->idx==idx) && (info->iSoundBank==(bSoundbank1?0:1)))
		{
			iSameSoundC++;
		}
	}

	if(iSameSoundC>MAX_SAME_SOUNDS_PLAYING)
	{
		return;
	}

	IXACT3Cue *cueInstance;
	HRESULT hr;
	MemSect(31);

	if(bSoundbank1)
	{
		if( FAILED( hr = m_pSoundBank->Prepare(idx, 0, 0, &cueInstance ) ) )
		{
			MemSect(0);
			//		printf("Sound prep failed\n");
			return;
		}
	}
	else
	{
		if( FAILED( hr = m_pSoundBank2->Prepare(idx, 0, 0, &cueInstance ) ) )
		{
			MemSect(0);
			//		printf("Sound prep failed\n");
			return;
		}
	}

	MemSect(0);

	// Register to receive callbacks for cues stopping so we can keep a track of active sounds

	soundInfo *info = new soundInfo();
	info->idx = idx;
	info->eSoundID = static_cast<eSOUND_TYPE>(iSound);
	info->iSoundBank = bSoundbank1?0:1;
	info->x = x;
	info->y = y;
	info->z = z;
	info->volume = volume;//*m_fSoundEffectsVolume;
	info->pitch = pitch;
	info->pCue = cueInstance;
	info->updatePos = true;
	EnterCriticalSection(&m_CS);
	currentSounds.push_back(info);
	LeaveCriticalSection(&m_CS);

	XACTVARIABLEINDEX vidx = cueInstance->GetVariableIndex("Pitch");
	if( vidx != XACTVARIABLEINDEX_INVALID )
	{
		// Convert pitch multiplier to semitones
		float semiTones = (log(pitch)/log(2.0f)) * 12.0f;
		cueInstance->SetVariable( vidx, semiTones );
	}

	update3DPosition(info);
	cueInstance->Play();
}

void SoundEngine::playUI(int iSound, float, float)
{
	bool bSoundBank1=(iSound<=eSoundType_STEP_SAND);

	if( (m_pSoundBank == nullptr ) || (m_pSoundBank2 == nullptr)) return;

	if( currentSounds.size() > MAX_POLYPHONY )
	{
		return;
	}
	wstring name = wchSoundNames[iSound];

	char *xboxName = static_cast<char *>(ConvertSoundPathToName(name));

	XACTINDEX idx = m_pSoundBank->GetCueIndex(xboxName);

	if( idx == XACTINDEX_INVALID )
	{
		// check soundbank 2
		idx = m_pSoundBank2->GetCueIndex(xboxName);
		if( idx == XACTINDEX_INVALID )
		{
#ifndef _CONTENT_PACKAGE
			printf("Not found UI: %s\n",xboxName);
#endif
			return;
		}
		bSoundBank1=false;
	}

	IXACT3Cue *cueInstance;
	HRESULT hr;

	if(bSoundBank1)
	{
		if( FAILED( hr = m_pSoundBank->Prepare(idx, 0, 0, &cueInstance ) ) )
		{
			//		printf("Sound prep failed\n");
			return;
		}
	}
	else
	{
		if( FAILED( hr = m_pSoundBank2->Prepare(idx, 0, 0, &cueInstance ) ) )
		{
			//		printf("Sound prep failed\n");
			return;
		}
	}

	// Add sound info just so we can detect end of this sound
	soundInfo *info = new soundInfo();
	info->eSoundID = static_cast<eSOUND_TYPE>(0);
	info->iSoundBank = bSoundBank1?0:1;
	info->idx =idx;
	info->x = 0.0f;
	info->y = 0.0f;
	info->z = 0.0f;
	info->volume = 0.0f;
	info->pitch = 0.0f;
	info->pCue = cueInstance;
	info->updatePos = false;
	EnterCriticalSection(&m_CS);
	currentSounds.push_back(info);
	LeaveCriticalSection(&m_CS);

	cueInstance->Play();
}

void SoundEngine::playStreaming(const wstring& name, float x, float y, float z, float vol, float pitch, bool bMusicDelay)
{
	IXACT3SoundBank *pSoundBank=nullptr;
	
	bool bSoundBank2=false;
	MemSect(34);
	if(m_MusicInfo.pCue!=nullptr)
	{
		m_MusicInfo.pCue->Stop(0);
		m_MusicInfo.pCue->Destroy();
		m_MusicInfo.pCue = nullptr;
	}

	m_MusicInfo.volume = 1.0f;//m_fMusicVolume;
	m_MusicInfo.pitch = 1.0f;

	SetIsPlayingEndMusic(false);
	SetIsPlayingNetherMusic(false);

	if(name.empty())
	{
		SetIsPlayingStreamingCDMusic(false);
		SetIsPlayingStreamingGameMusic(false);// will be set to true when the sound is started in the tick
		if(bMusicDelay)
		{
			noMusicDelay = random->nextInt(20 * 60 * 10) + 20 * 60 * 10;
		}
		else
		{
			noMusicDelay=0;
		}
		// Check if we have a local player in The Nether or in The End, and play that music if they are
		Minecraft *pMinecraft=Minecraft::GetInstance();
		bool playerInEnd=false;
		bool playerInNether=false;

		for(unsigned int i=0;i<XUSER_MAX_COUNT;i++)
		{
			if(pMinecraft->localplayers[i]!=nullptr)
			{
				if(pMinecraft->localplayers[i]->dimension==LevelData::DIMENSION_END)
				{
					playerInEnd=true;
				}
				else if(pMinecraft->localplayers[i]->dimension==LevelData::DIMENSION_NETHER)
				{
					playerInNether=true;
				}
			}
		}
		TexturePack *pTexPack=Minecraft::GetInstance()->skins->getSelected();

		if(Minecraft::GetInstance()->skins->isUsingDefaultSkin() || pTexPack->hasAudio()==false)
		{
			if(playerInEnd || playerInNether)
			{
				pSoundBank=m_pSoundBank2;
			}
			else
			{
				pSoundBank=m_pSoundBank;
			}
		}
		else
		{
			// get the dlc texture pack
			DLCTexturePack *pDLCTexPack=static_cast<DLCTexturePack *>(pTexPack);
			pSoundBank=pDLCTexPack->m_pSoundBank;

			// check we can play the sound
			if(isStreamingWavebankReady(pDLCTexPack->m_pStreamedWaveBank)==false)
			{
				return;
			}
		}

		if(playerInEnd)
		{
			m_musicIDX = pSoundBank->GetCueIndex("the_end_dragon");
			SetIsPlayingEndMusic(true);
			bSoundBank2=true;
		}
		else if(playerInNether)
		{
			m_musicIDX = pSoundBank->GetCueIndex("nether");
			SetIsPlayingNetherMusic(true);
			bSoundBank2=true;
		}
		else
		{
			m_musicIDX = pSoundBank->GetCueIndex("music");
		}
	}
	else
	{
		pSoundBank=m_pSoundBank;
		SetIsPlayingStreamingCDMusic(true);
		SetIsPlayingStreamingGameMusic(false);

		m_musicIDX = pSoundBank->GetCueIndex(ConvertSoundPathToName(name));
	}

	HRESULT hr;

	if( FAILED( hr = pSoundBank->Prepare(m_musicIDX, 0, 0, &m_MusicInfo.pCue ) ) )
	{
		//		printf("Sound prep failed\n");
		m_musicIDX = XACTINDEX_INVALID; // don't do anything in the tick
		m_MusicInfo.pCue=nullptr;
		MemSect(0);
		return;
	}
	

	if(GetIsPlayingStreamingCDMusic())
	{
		m_MusicInfo.x = x;
		m_MusicInfo.y = y;
		m_MusicInfo.z = z;
		m_MusicInfo.updatePos = true;
		update3DPosition(&m_MusicInfo, false);
		m_MusicInfo.pCue->Play();
	}
	else
	{
		// don't play the game music - it will start playing in the tick when noMusicDelay is 0

		m_MusicInfo.x = 0.0f; // will be overridden by the bPlaceEmitterAtListener
		m_MusicInfo.y = 0.0f; // will be overridden by the bPlaceEmitterAtListener
		m_MusicInfo.z = 0.0f; // will be overridden by the bPlaceEmitterAtListener
		m_MusicInfo.updatePos = false;

		update3DPosition(&m_MusicInfo, true);
	}

	MemSect(0);
}
void SoundEngine::playMusicTick()
{
	if( (m_pSoundBank == nullptr ) || (m_pSoundBank2 == nullptr)) return;

	if( m_musicIDX == XACTINDEX_INVALID )
	{
		//		printf("Not found music\n");
		return;
	}

	// check to see if the sound has stopped playing
	DWORD state;
	HRESULT hr;
	if(m_MusicInfo.pCue!=nullptr)
	{
		if( FAILED( hr = m_MusicInfo.pCue->GetState(&state) ) )
		{
			assert(false);
		}
		else
		{
			if( state == XACT_CUESTATE_STOPPED )
			{
				// remove the sound and reset the music
				playStreaming(L"", 0, 0, 0, 0, 0);
				return;
			}
		}
	}

	if(GetIsPlayingStreamingGameMusic())
	{
		if(m_MusicInfo.pCue!=nullptr)
		{
			bool playerInEnd = false;
			bool playerInNether=false;
			Minecraft *pMinecraft = Minecraft::GetInstance();
			for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
			{
				if(pMinecraft->localplayers[i]!=nullptr)
				{
					if(pMinecraft->localplayers[i]->dimension==LevelData::DIMENSION_END)
					{
						playerInEnd=true;
					}
					else if(pMinecraft->localplayers[i]->dimension==LevelData::DIMENSION_NETHER)
					{
						playerInNether=true;
					}
				}
			}

			if((playerInEnd && !GetIsPlayingEndMusic()) ||(!playerInEnd && GetIsPlayingEndMusic()))
			{
				// remove the sound and reset the music
				playStreaming(L"", 0, 0, 0, 0, 0);
			}
			else if ((playerInNether && !GetIsPlayingNetherMusic()) ||(!playerInNether && GetIsPlayingNetherMusic()))
			{
				// remove the sound and reset the music
				playStreaming(L"", 0, 0, 0, 0, 0);
			}
		}
		// not positional so doesn't need ticked
		return;
	}

	// is this cd music? If so, we need to tick it
	if(GetIsPlayingStreamingCDMusic())
	{
		update3DPosition(&m_MusicInfo, false, true);
	}
	else
	{
		if (noMusicDelay > 0)
		{
			noMusicDelay--;
			return;
		}

		if(m_MusicInfo.pCue!=nullptr)
		{
			update3DPosition(&m_MusicInfo, true);
			SetIsPlayingStreamingGameMusic(true);
			// and play the game music here
			m_MusicInfo.pCue->Play();
		}
	}
}

void SoundEngine::updateMusicVolume(float fVal)
{
	XACTVOLUME xactVol=fVal;
	HRESULT hr=m_pXACT3Engine->SetVolume(m_xactMusic,fVal);
}

void SoundEngine::updateSystemMusicPlaying(bool isPlaying)
{
}

void SoundEngine::updateSoundEffectVolume(float fVal)
{
	XACTVOLUME xactVol=fVal;
	HRESULT hr=m_pXACT3Engine->SetVolume(m_xactSFX,fVal);
}

void SoundEngine::update3DPosition(SoundEngine::soundInfo *pInfo, bool bPlaceEmitterAtListener,bool bIsCDMusic)
{
	X3DAUDIO_LISTENER *listener = &m_listeners[0];	// Default case for single listener

	if( ( m_validListenerCount > 1 ) && !bPlaceEmitterAtListener )
	{
		// More than one listener. Find out which one is closest
		float nearDistSq =  ( listener->Position.x - pInfo->x ) * ( listener->Position.x - pInfo->x ) +
			( listener->Position.y - pInfo->y ) * ( listener->Position.y - pInfo->y ) +
			( listener->Position.z + pInfo->z ) * ( listener->Position.z + pInfo->z );

		for( int i = 1; i < m_validListenerCount; i++ )
		{
			float distSq =  ( m_listeners[i].Position.x - pInfo->x ) * ( m_listeners[i].Position.x - pInfo->x ) +
				( m_listeners[i].Position.y - pInfo->y ) * ( m_listeners[i].Position.y - pInfo->y ) +
				( m_listeners[i].Position.z + pInfo->z ) * ( m_listeners[i].Position.z + pInfo->z );
			if( distSq < nearDistSq )
			{
				listener = &m_listeners[i];
				nearDistSq = distSq;
			}
		}

		// More than one listener, don't do directional sounds - point our listener towards the sound
		float xzDist =  sqrtf( ( listener->Position.x - pInfo->x ) * ( listener->Position.x - pInfo->x ) +
			( listener->Position.z + pInfo->z ) * ( listener->Position.z + pInfo->z ) );
		// Don't orientate if its too near to work out a distance
		if( xzDist > 0.001f)
		{
			listener->OrientFront.x = ( pInfo->x - listener->Position.x ) / xzDist;
			listener->OrientFront.y = 0.0f;
			listener->OrientFront.z = ( - pInfo->z - listener->Position.z ) / xzDist;
		}
	}

	if(bPlaceEmitterAtListener)
	{
		m_emitter.Position.x = listener->Position.x;
		m_emitter.Position.y = listener->Position.y;
		m_emitter.Position.z = listener->Position.z;
	}
	else
	{
		// Update the position of the emitter - we aren't dynamically changing anything else
		m_emitter.Position.x = pInfo->x;
		m_emitter.Position.y = pInfo->y;
		m_emitter.Position.z = -pInfo->z;	// Flipped sign of z as x3daudio is expecting left handed coord system
	}

	// If this is the CD music, then make the distance scaler 4 x normal
	if(bIsCDMusic)
	{
		m_emitter.CurveDistanceScaler=64.0f;
	}
	else
	{
		switch(pInfo->eSoundID)
		{
			// Is this the Dragon?
		case eSoundType_MOB_ENDERDRAGON_GROWL:
		case eSoundType_MOB_ENDERDRAGON_MOVE:
		case eSoundType_MOB_ENDERDRAGON_END:
		case eSoundType_MOB_ENDERDRAGON_HIT:
			m_emitter.CurveDistanceScaler=100.0f;
			break;
		case eSoundType_MOB_GHAST_MOAN:
		case eSoundType_MOB_GHAST_SCREAM:
		case eSoundType_MOB_GHAST_DEATH:
		case eSoundType_MOB_GHAST_CHARGE:
		case eSoundType_MOB_GHAST_FIREBALL:
			m_emitter.CurveDistanceScaler=30.0f;
			break;
		}
	}

	// 10000.0f is passed as the volume for thunder... treat this as a special case, and use a volume curve that doesn't decay with distance
	// rather than just trying to guess at making something really really loud...
	if( pInfo->volume == 10000.0f )
	{
		m_emitter.pVolumeCurve = &m_VolumeCurveNoDecay;
	}
	else
	{
		m_emitter.pVolumeCurve = &m_VolumeCurve;
	}

	// Calculate all the 3D things
	XACT3DCalculate( m_xact3dInstance, listener, &m_emitter, &m_DSPSettings );

	// Put volume curve back to default in case something else is depending on this
	m_emitter.pVolumeCurve = &m_VolumeCurve;
	//m_emitter.pLFECurve = &m_VolumeCurve;
	m_emitter.CurveDistanceScaler=16.0f;
	// Apply our general volume too by scaling the calculated coefficients - so long as this isn't our special case of 10000.0f (see comment above)
	if( pInfo->volume != 10000.0f )
	{
		for(unsigned int i = 0; i < m_DSPSettings.DstChannelCount; i++ )
		{
			m_DSPSettings.pMatrixCoefficients[i] *= pInfo->volume;
		}
	}

	// Finally apply to the cue
	XACT3DApply( &m_DSPSettings, pInfo->pCue);
}

void SoundEngine::tick(shared_ptr<Mob> *players, float a)
{
	if( m_pXACT3Engine == nullptr ) return;

	// Creater listener array from the local players
	int listenerCount = 0;
	bool doPosUpdate = true;
	if( players )
	{
		for( int i = 0; i < 4; i++ )
		{
			if( players[i] != nullptr )
			{
				float yRot = players[i]->yRotO + (players[i]->yRot - players[i]->yRotO) * a;

				m_listeners[listenerCount].Position.x = static_cast<float>(players[i]->xo + (players[i]->x - players[i]->xo) * a);
				m_listeners[listenerCount].Position.y = static_cast<float>(players[i]->yo + (players[i]->y - players[i]->yo) * a);
				m_listeners[listenerCount].Position.z = -static_cast<float>(players[i]->zo + (players[i]->z - players[i]->zo) * a);	// Flipped sign of z as x3daudio is expecting left handed coord system

				float yCos = (float)cos(-yRot * Mth::RAD_TO_GRAD - PI);
				float ySin = (float)sin(-yRot * Mth::RAD_TO_GRAD - PI);

				m_listeners[listenerCount].OrientFront.x = -ySin;
				m_listeners[listenerCount].OrientFront.y = 0;
				m_listeners[listenerCount].OrientFront.z = yCos;	// Flipped sign of z as x3daudio is expecting left handed coord system

				listenerCount++;
			}
		}
	}
	// If there were no valid players set, make up a default listener
	if( listenerCount == 0 )
	{
		doPosUpdate = false;	// Don't bother updating positions of sounds already placed
		m_listeners[listenerCount].Position.x = 0;
		m_listeners[listenerCount].Position.y = 0;
		m_listeners[listenerCount].Position.z = 0;
		m_listeners[listenerCount].OrientFront.x = 0;
		m_listeners[listenerCount].OrientFront.y = 0;
		m_listeners[listenerCount].OrientFront.z = 1.0f;
		listenerCount++;
	}
	m_validListenerCount = listenerCount;

	EnterCriticalSection(&m_CS);
	for( unsigned int i = 0; i < currentSounds.size(); i++ )
	{
		SoundEngine::soundInfo *info = currentSounds[i];

		DWORD state;
		HRESULT hr;
		if( FAILED( hr = info->pCue->GetState(&state) ) )
		{
			assert(false);
		}
		else
		{
			if( state == XACT_CUESTATE_STOPPED )
			{
				info->pCue->Destroy();
				delete currentSounds[i];
				currentSounds[i] = currentSounds.back();
				currentSounds.pop_back();
			}
			else
			{
				if( info->updatePos )
				{
					if( doPosUpdate )
					{
						update3DPosition(info);
					}
				}
			}
		}
	}

	LeaveCriticalSection(&m_CS);
	m_pXACT3Engine->DoWork();
}

void SoundEngine::add(const wstring& name, File *file)
{
}

void SoundEngine::addMusic(const wstring& name, File *file)
{
}

void SoundEngine::addStreaming(const wstring& name, File *file)
{
}
