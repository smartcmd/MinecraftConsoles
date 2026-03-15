#pragma once

#include "..\..\..\Minecraft.World\SoundTypes.h"

#ifdef _XBOX

#elif defined (__PS3__)
#undef __in
#undef __out
#include "..\..\PS3\Miles\include\mss.h"
#elif defined (__PSVITA__)
#include "..\..\PSVITA\Miles\include\mss.h"
#elif defined _DURANGO
// 4J Stu - Temp define to get Miles to link, can likely be removed when we get a new version of Miles
#define _SEKRIT
#include "..\..\Durango\Miles\include\mss.h"
#elif defined _WINDOWS64
#else // PS4
// 4J Stu - Temp define to get Miles to link, can likely be removed when we get a new version of Miles
#define _SEKRIT2
#include "..\..\Orbis\Miles\include\mss.h"
#endif

typedef struct
{
	float x,y,z;
}
AUDIO_VECTOR;

typedef struct
{
	bool bValid;
	AUDIO_VECTOR vPosition;
	AUDIO_VECTOR vOrientFront;
}
AUDIO_LISTENER;

class Options;

class ConsoleSoundEngine
{
public:

	ConsoleSoundEngine() : m_bIsPlayingStreamingCDMusic(false),m_bIsPlayingStreamingGameMusic(false), m_bIsPlayingEndMusic(false),m_bIsPlayingNetherMusic(false){};
	virtual void tick(shared_ptr<Mob> *players, float a) =0;
	virtual void destroy()=0;
	virtual void play(int iSound, float x, float y, float z, float volume, float pitch) =0;
	virtual void playStreaming(const wstring& name, float x, float y , float z, float volume, float pitch, bool bMusicDelay=true) =0;
	virtual void playUI(int iSound, float volume, float pitch) =0;
	virtual void updateMusicVolume(float fVal) =0;
	virtual void updateSystemMusicPlaying(bool isPlaying) = 0;
	virtual void updateSoundEffectVolume(float fVal) =0;
	virtual void init(Options *) =0 ;
	virtual void add(const wstring& name, File *file) =0;
	virtual void addMusic(const wstring& name, File *file) =0;
	virtual void addStreaming(const wstring& name, File *file)  =0;
	virtual char *ConvertSoundPathToName(const wstring& name, bool bConvertSpaces) =0;
	virtual void playMusicTick() =0;

	virtual bool GetIsPlayingStreamingCDMusic()				;
	virtual bool GetIsPlayingStreamingGameMusic()			;
	virtual void SetIsPlayingStreamingCDMusic(bool bVal)	;
	virtual void SetIsPlayingStreamingGameMusic(bool bVal)	;
	virtual bool GetIsPlayingMenuMusic();
	virtual bool GetIsPlayingEndMusic()						;
	virtual bool GetIsPlayingNetherMusic()					;
	virtual void SetIsPlayingEndMusic(bool bVal)			;
	virtual void SetIsPlayingMenuMusic(bool bVal);
	virtual void SetIsPlayingNetherMusic(bool bVal)			;
	static const WCHAR *wchSoundNames[eSoundType_MAX];
	static const WCHAR *wchUISoundNames[eSFX_MAX];

public:
	void tick();
	void schedule(int iSound, float x, float y, float z, float volume, float pitch, int delayTicks);

private:
	class ScheduledSound
	{
	public:
		int iSound;
		float x, y, z;
		float volume, pitch;
		int delay;

	public:
		ScheduledSound(int iSound, float x, float y, float z, float volume, float pitch, int delay);
	};

	vector<ScheduledSound *> scheduledSounds;

private:
	// platform specific functions

	virtual int initAudioHardware(int iMinSpeakers)=0;

	bool m_bIsPlayingStreamingCDMusic;
	bool m_bIsPlayingStreamingGameMusic;
	bool m_bIsPlayingMenuMusic;
	bool m_bIsPlayingEndMusic;
	bool m_bIsPlayingNetherMusic;
};