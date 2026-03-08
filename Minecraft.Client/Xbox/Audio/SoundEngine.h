#pragma once
class Mob;
class Options;
using namespace std;
#include "../../../Minecraft.World/SoundTypes.h"

#ifdef _XBOX
extern IXAudio2* g_pXAudio2;       // pointer to XAudio2 instance used by QNet and XACT
extern IXAudio2MasteringVoice* g_pXAudio2MasteringVoice;  // pointer to XAudio2 mastering voice
#endif

class SoundEngine : public ConsoleSoundEngine
{
#ifdef _XBOX
	//static const unsigned char *ucSoundNames[eSoundType_MAX][32];
	static const int MAX_POLYPHONY = 30;		// 4J added
	static const int MAX_SAME_SOUNDS_PLAYING = 8; // 4J added
	static IXACT3Engine				*m_pXACT3Engine;
	static X3DAUDIO_HANDLE			m_xact3dInstance;
	static X3DAUDIO_DSP_SETTINGS	m_DSPSettings;
	static X3DAUDIO_EMITTER			m_emitter;
	static X3DAUDIO_LISTENER		m_listeners[4];
	static int						m_validListenerCount;

	static X3DAUDIO_DISTANCE_CURVE	m_VolumeCurve;
	static X3DAUDIO_DISTANCE_CURVE_POINT m_VolumeCurvePoints[2];

	static X3DAUDIO_DISTANCE_CURVE	m_DragonVolumeCurve;
	static X3DAUDIO_DISTANCE_CURVE_POINT m_DragonVolumeCurvePoints[2];

	static X3DAUDIO_DISTANCE_CURVE	m_VolumeCurveNoDecay;
	static X3DAUDIO_DISTANCE_CURVE_POINT m_VolumeCurvePointsNoDecay[2];

	static IXACT3WaveBank	*m_pWaveBank;
	static IXACT3WaveBank	*m_pWaveBank2;
	static IXACT3WaveBank	*m_pStreamedWaveBank;
	static IXACT3WaveBank	*m_pStreamedWaveBankAdditional;
	static IXACT3SoundBank	*m_pSoundBank;
	static IXACT3SoundBank	*m_pSoundBank2;

	static CRITICAL_SECTION		m_CS;

	struct soundInfo
	{
		// 4J-PB - adding the cue index so we can limit the number of the same sounds playing (rain)
		XACTINDEX	idx;
		int			iSoundBank;
		eSOUND_TYPE	eSoundID;
		float		x, y, z;
		float		volume;
		float		pitch;
		IXACT3Cue	*pCue;
		bool		updatePos;
	};

	void update3DPosition( soundInfo *pInfo, bool bPlaceEmitterAtListener = false, bool bIsCDMusic = false);
	static vector<soundInfo *>currentSounds;

	int noMusicDelay;
	Random *random;

	// 4J Added
#endif

#ifdef _XBOX
	char * m_chMusicName;
	soundInfo m_MusicInfo;

	XACTINDEX m_musicIDX;
	bool m_bStreamingMusicReady;
	bool m_bStreamingWaveBank1Ready;
	bool m_bStreamingWaveBank2Ready;

	// to handle volume changes
	XACTCATEGORY m_xactSFX;
	XACTCATEGORY m_xactMusic;
#endif
public:
	SoundEngine();
	virtual void destroy();
	virtual void play(int iSound, float x, float y, float z, float volume, float pitch);
	virtual void playStreaming(const wstring& name, float x, float y , float z, float volume, float pitch, bool bMusicDelay=true);
	virtual void playUI(int iSound, float volume, float pitch);
	virtual void playMusicTick();
	virtual void updateMusicVolume(float fVal);
	virtual void updateSystemMusicPlaying(bool isPlaying);
	virtual void updateSoundEffectVolume(float fVal);
	virtual void init(Options *);
	virtual void tick(shared_ptr<Mob> *players, float a);	// 4J - updated to take array of local players rather than single one
	virtual void add(const wstring& name, File *file);
	virtual void addMusic(const wstring& name, File *file);
	virtual void addStreaming(const wstring& name, File *file);
#ifndef __PS3__
	static void setXACTEngine( IXACT3Engine *pXACT3Engine);
	void CreateStreamingWavebank(const char *pchName, IXACT3WaveBank **ppStreamedWaveBank);
	void CreateSoundbank(const char *pchName, IXACT3SoundBank **ppSoundBank);

#endif // __PS3__
	virtual char *ConvertSoundPathToName(const wstring& name, bool bConvertSpaces=false);
	bool isStreamingWavebankReady();		// 4J Added
#ifdef _XBOX
		bool isStreamingWavebankReady(IXACT3WaveBank *pWaveBank);
#endif
		int initAudioHardware(int iMinSpeakers)	{ return iMinSpeakers;}

private:
#ifndef __PS3__
	static void XACTNotificationCallback( const XACT_NOTIFICATION* pNotification );
#endif // __PS3__
}; 