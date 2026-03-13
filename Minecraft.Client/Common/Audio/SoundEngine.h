#pragma once
class Mob;
class Options;
using namespace std;
#include "../../Minecraft.World/SoundTypes.h"

#include "miniaudio.h"

constexpr float SFX_3D_MIN_DISTANCE = 1.0f;
constexpr float SFX_3D_MAX_DISTANCE = 16.0f;
constexpr float SFX_3D_ROLLOFF = 0.5f;
constexpr float SFX_VOLUME_MULTIPLIER = 1.5f;
constexpr float SFX_MAX_GAIN = 1.5f;

enum eMUSICFILES
{
	eStream_Overworld_Calm1 = 0,
	eStream_Overworld_Calm2,
	eStream_Overworld_Calm3,
	eStream_Overworld_hal1,
	eStream_Overworld_hal2,
	eStream_Overworld_hal3,
	eStream_Overworld_hal4,
	eStream_Overworld_nuance1,
	eStream_Overworld_nuance2,
#ifndef _XBOX
	// Add the new music tracks
	eStream_Overworld_Creative1,
	eStream_Overworld_Creative2,
	eStream_Overworld_Creative3,
	eStream_Overworld_Creative4,
	eStream_Overworld_Creative5,
	eStream_Overworld_Creative6,
	eStream_Overworld_Menu1,
	eStream_Overworld_Menu2,
	eStream_Overworld_Menu3,
	eStream_Overworld_Menu4,
#endif
	eStream_Overworld_piano1,
	eStream_Overworld_piano2,
	eStream_Overworld_piano3, // <-- make piano3 the last overworld one
	// Nether
	eStream_Nether1,
	eStream_Nether2,
	eStream_Nether3,
	eStream_Nether4,
	// The End
	eStream_end_dragon,
	eStream_end_end,
	eStream_CD_1,
	eStream_CD_2,
	eStream_CD_3,
	eStream_CD_4,
	eStream_CD_5,
	eStream_CD_6,
	eStream_CD_7,
	eStream_CD_8,
	eStream_CD_9,
	eStream_CD_10,
	eStream_CD_11,
	eStream_CD_12,
	eStream_Max,
};

enum eMUSICTYPE
{
	eMusicType_None,
	eMusicType_Game,
	eMusicType_CD,
};


enum MUSIC_STREAMSTATE
{
	eMusicStreamState_Idle=0,
	eMusicStreamState_Stop,
	eMusicStreamState_Stopping,
	eMusicStreamState_Opening,
	eMusicStreamState_OpeningCancel,
	eMusicStreamState_Play,
	eMusicStreamState_Playing,
	eMusicStreamState_Completed
};

typedef struct
{
	#ifndef _WINDOWS64
	F32 x,y,z,volume,pitch;
	#else
	float x,y,z,volume,pitch;
	#endif
	int iSound;
	bool bIs3D;	
	bool bUseSoundsPitchVal;	
#ifdef _DEBUG
	char chName[64];
#endif
}
AUDIO_INFO;

#ifdef _WINDOWS64
struct MiniAudioSound
{
    ma_sound sound;
    AUDIO_INFO info;
    bool active;
};

extern std::vector<MiniAudioSound*> m_activeSounds;
#endif

class SoundEngine : public ConsoleSoundEngine
{
	static const int MAX_SAME_SOUNDS_PLAYING = 8; // 4J added
public:
	SoundEngine();
    void destroy() override;
#ifdef _DEBUG
	void GetSoundName(char *szSoundName,int iSound);
#endif
    void play(int iSound, float x, float y, float z, float volume, float pitch) override;
    void playStreaming(const wstring& name, float x, float y , float z, float volume, float pitch, bool bMusicDelay=true) override;
    void playUI(int iSound, float volume, float pitch) override;
    void playMusicTick() override;
    void updateMusicVolume(float fVal) override;
    void updateSystemMusicPlaying(bool isPlaying) override;
    void updateSoundEffectVolume(float fVal) override;
    void init(Options *) override;
    void tick(shared_ptr<Mob> *players, float a) override;	// 4J - updated to take array of local players rather than single one
    void add(const wstring& name, File *file) override;
    void addMusic(const wstring& name, File *file) override;
    void addStreaming(const wstring& name, File *file) override;
    char *ConvertSoundPathToName(const wstring& name, bool bConvertSpaces=false) override;
	bool isStreamingWavebankReady();		// 4J Added
	int getMusicID(int iDomain);
	int getMusicID(const wstring& name);
	void SetStreamingSounds(int iOverworldMin, int iOverWorldMax, int iNetherMin, int iNetherMax, int iEndMin, int iEndMax, int iCD1);
	void updateMiniAudio();
	void playMusicUpdate();

private:
	float getMasterMusicVolume();
	// platform specific functions
#ifdef __PS3__
	int initAudioHardware(int iMinSpeakers);
#else
	int initAudioHardware(int iMinSpeakers) override
    { return iMinSpeakers;}
#endif
	
	int GetRandomishTrack(int iStart,int iEnd);

	ma_engine m_engine;
	ma_engine_config m_engineConfig;
	ma_sound m_musicStream;
	bool m_musicStreamActive;

	static char m_szSoundPath[];
	static char m_szMusicPath[];
	static char m_szRedistName[];
	static const char *m_szStreamFileA[eStream_Max];

	AUDIO_LISTENER m_ListenerA[MAX_LOCAL_PLAYERS];
	int m_validListenerCount;


	Random *random;
	int m_musicID;
	int m_iMusicDelay;
	int m_StreamState;
	int m_MusicType;
	AUDIO_INFO m_StreamingAudioInfo;
	wstring m_CDMusic;
	BOOL m_bSystemMusicPlaying;
	float m_MasterMusicVolume;
	float m_MasterEffectsVolume;

	C4JThread *m_openStreamThread;
	static int OpenStreamThreadProc( void* lpParameter );
	char m_szStreamName[255];
	int CurrentSoundsPlaying[eSoundType_MAX+eSFX_MAX];

	// streaming music files - will be different for mash-up packs
	int m_iStream_Overworld_Min,m_iStream_Overworld_Max;
	int m_iStream_Nether_Min,m_iStream_Nether_Max;
	int m_iStream_End_Min,m_iStream_End_Max;
	int m_iStream_CD_1;
	bool *m_bHeardTrackA;

#ifdef __ORBIS__
	int32_t m_hBGMAudio;
#endif
}; 