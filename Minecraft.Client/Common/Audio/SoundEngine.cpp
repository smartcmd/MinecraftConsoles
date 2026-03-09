#include "stdafx.h"

#include "SoundEngine.h"
#include "..\Consoles_App.h"
#include "..\..\MultiplayerLocalPlayer.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\Minecraft.World\leveldata.h"
#include "..\..\Minecraft.World\mth.h"
#include "..\..\TexturePackRepository.h"
#include "..\..\DLCTexturePack.h"
#include "Common\DLC\DLCAudioFile.h"

#ifdef __PSVITA__
#include <audioout.h>
#endif

#include "..\..\Minecraft.Client\Windows64\Windows64_App.h"

#include "stb_vorbis.h"

#define MA_NO_DSOUND
#define MA_NO_WINMM
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <vector>
#include <memory>
#include <mutex>
#include <lce_filesystem\lce_filesystem.h>

#ifdef __ORBIS__
#include <audioout.h>
//#define __DISABLE_MILES__			// MGH disabled for now as it crashes if we call sceNpMatching2Initialize
#endif 

// take out Orbis until they are done
#if defined _XBOX 

SoundEngine::SoundEngine() {}
void SoundEngine::init(Options *pOptions)
{
}

void SoundEngine::tick(shared_ptr<Mob> *players, float a)
{
}
void SoundEngine::destroy() {}
void SoundEngine::play(int iSound, float x, float y, float z, float volume, float pitch)
{
	app.DebugPrintf("PlaySound - %d\n",iSound);
}
void SoundEngine::playStreaming(const wstring& name, float x, float y , float z, float volume, float pitch, bool bMusicDelay) {}
void SoundEngine::playUI(int iSound, float volume, float pitch) {}

void SoundEngine::updateMusicVolume(float fVal) {}
void SoundEngine::updateSoundEffectVolume(float fVal) {}

void SoundEngine::add(const wstring& name, File *file) {}
void SoundEngine::addMusic(const wstring& name, File *file) {}
void SoundEngine::addStreaming(const wstring& name, File *file) {}
char *SoundEngine::ConvertSoundPathToName(const wstring& name, bool bConvertSpaces) { return nullptr; }
bool SoundEngine::isStreamingWavebankReady() { return true; }
void SoundEngine::playMusicTick() {};

#else

#ifdef _WINDOWS64
char SoundEngine::m_szSoundPath[]={"Windows64Media\\Sound\\"};
char SoundEngine::m_szMusicPath[]={"music\\"};
char SoundEngine::m_szRedistName[]={"redist64"};
#elif defined _DURANGO
char SoundEngine::m_szSoundPath[]={"Sound\\"};
char SoundEngine::m_szMusicPath[]={"music\\"};
char SoundEngine::m_szRedistName[]={"redist64"};
#elif defined __ORBIS__

#ifdef _CONTENT_PACKAGE
char SoundEngine::m_szSoundPath[]={"Sound/"};
#elif defined _ART_BUILD
char SoundEngine::m_szSoundPath[]={"Sound/"};
#else
// just use the host Durango folder for the sound. In the content package, we'll have moved this in the .gp4 file
char SoundEngine::m_szSoundPath[]={"Durango/Sound/"};
#endif
char SoundEngine::m_szMusicPath[]={"music/"};
char SoundEngine::m_szRedistName[]={"redist64"};
#elif defined __PSVITA__
char SoundEngine::m_szSoundPath[]={"PSVita/Sound/"};
char SoundEngine::m_szMusicPath[]={"music/"};
char SoundEngine::m_szRedistName[]={"redist"};
#elif defined __PS3__
//extern const char* getPS3HomePath();
char SoundEngine::m_szSoundPath[]={"PS3/Sound/"};
char SoundEngine::m_szMusicPath[]={"music/"};
char SoundEngine::m_szRedistName[]={"redist"};

#define USE_SPURS

#ifdef USE_SPURS
#include <cell/spurs.h>
#else
#include <sys/spu_image.h>
#endif

#endif

const char *SoundEngine::m_szStreamFileA[eStream_Max]=
{
	"calm1",
	"calm2",
	"calm3",
	"hal1",
	"hal2",
	"hal3",
	"hal4",
	"nuance1",
	"nuance2",

#ifndef _XBOX
	"creative1",
	"creative2",
	"creative3",
	"creative4",
	"creative5",
	"creative6",
	"menu1",
	"menu2",
	"menu3",
	"menu4",
#endif

	"piano1",
	"piano2",
	"piano3",

	// Nether
	"nether1",
	"nether2",
	"nether3",
	"nether4",

	// The End
	"the_end_dragon_alive",
	"the_end_end",
	
	// CDs
	"11",
	"13",
	"blocks",
	"cat",
	"chirp",
	"far",
	"mall",
	"mellohi",
	"stal",
	"strad",
	"ward",
	"where_are_we_now"
};

std::vector<MiniAudioSound*> m_activeSounds;

/////////////////////////////////////////////
//
//	init
//
/////////////////////////////////////////////
void SoundEngine::init(Options* pOptions)
{
    app.DebugPrintf("---SoundEngine::init\n");

    m_engineConfig = ma_engine_config_init();
    m_engineConfig.listenerCount = MAX_LOCAL_PLAYERS;

    if (ma_engine_init(&m_engineConfig, &m_engine) != MA_SUCCESS)
    {
        app.DebugPrintf("Failed to initialize miniaudio engine\n");
        return;
    }

    ma_engine_set_volume(&m_engine, 1.0f);

    m_MasterMusicVolume = 1.0f;
    m_MasterEffectsVolume = 1.0f;

    m_validListenerCount = 1;
    
    m_bSystemMusicPlaying = false;

    app.DebugPrintf("---miniaudio initialized\n");
	
    return;
}

void SoundEngine::SetStreamingSounds(int iOverworldMin, int iOverWorldMax, int iNetherMin, int iNetherMax, int iEndMin, int iEndMax, int iCD1)
{
	m_iStream_Overworld_Min=iOverworldMin;
	m_iStream_Overworld_Max=iOverWorldMax;
	m_iStream_Nether_Min=iNetherMin;
	m_iStream_Nether_Max=iNetherMax;
	m_iStream_End_Min=iEndMin;
	m_iStream_End_Max=iEndMax;
	m_iStream_CD_1=iCD1;

	// array to monitor recently played tracks
	if(m_bHeardTrackA)
	{
		delete [] m_bHeardTrackA;
	}
	m_bHeardTrackA = new bool[iEndMax+1];
	memset(m_bHeardTrackA,0,sizeof(bool)*iEndMax+1);
}

void SoundEngine::updateMiniAudio()
{
    if (m_validListenerCount == 1)
    {
        for (size_t i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
            if (m_ListenerA[i].bValid)
            {
                ma_engine_listener_set_position(
                    &m_engine,
                    0,
                    m_ListenerA[i].vPosition.x,
                    m_ListenerA[i].vPosition.y,
                    m_ListenerA[i].vPosition.z);

                ma_engine_listener_set_direction(
                    &m_engine,
                    0,
                    m_ListenerA[i].vOrientFront.x,
                    m_ListenerA[i].vOrientFront.y,
                    m_ListenerA[i].vOrientFront.z);

                ma_engine_listener_set_world_up(
                    &m_engine,
                    0,
                    0.0f, 1.0f, 0.0f);

                break;
            }
        }
    }
    else
    {
        ma_engine_listener_set_position(&m_engine, 0, 0.0f, 0.0f, 0.0f);
        ma_engine_listener_set_direction(&m_engine, 0, 0.0f, 0.0f, 1.0f);
        ma_engine_listener_set_world_up(&m_engine, 0, 0.0f, 1.0f, 0.0f);
    }

    for (auto it = m_activeSounds.begin(); it != m_activeSounds.end(); )
    {
        MiniAudioSound* s = *it;

        if (!ma_sound_is_playing(&s->sound))
        {
            ma_sound_uninit(&s->sound);
            delete s;
            it = m_activeSounds.erase(it);
            continue;
        }

        float finalVolume = s->info.volume * m_MasterEffectsVolume;
        if (finalVolume > 1.0f)
            finalVolume = 1.0f;

        ma_sound_set_volume(&s->sound, finalVolume);
        ma_sound_set_pitch(&s->sound, s->info.pitch);

        if (s->info.bIs3D)
        {
            if (m_validListenerCount > 1)
            {
                float fClosest=10000.0f;
				int iClosestListener=0;
				float fClosestX=0.0f,fClosestY=0.0f,fClosestZ=0.0f,fDist;
				for( size_t i = 0; i < MAX_LOCAL_PLAYERS; i++ )
				{
					if( m_ListenerA[i].bValid )
					{
						float x,y,z;

						x=fabs(m_ListenerA[i].vPosition.x-s->info.x);
						y=fabs(m_ListenerA[i].vPosition.y-s->info.y);
						z=fabs(m_ListenerA[i].vPosition.z-s->info.z);
						fDist=x+y+z;

						if(fDist<fClosest)
						{
							fClosest=fDist;
							fClosestX=x;
							fClosestY=y;
							fClosestZ=z;
							iClosestListener=i;
						}
					}
				}

                float realDist = sqrtf((fClosestX*fClosestX)+(fClosestY*fClosestY)+(fClosestZ*fClosestZ));
                ma_sound_set_position(&s->sound, 0, 0, realDist);
            }
            else
            {
                ma_sound_set_position(
                    &s->sound,
                    s->info.x,
                    s->info.y,
                    s->info.z);
            }
        }

        ++it;
    }
}

/////////////////////////////////////////////
//
//	tick
//
/////////////////////////////////////////////

#ifdef __PSVITA__
static S32 running = AIL_ms_count();
#endif

void SoundEngine::tick(shared_ptr<Mob> *players, float a)
{
	ConsoleSoundEngine::tick();

	// update the listener positions
	int listenerCount = 0;
	if( players )
	{
		bool bListenerPostionSet = false;
		for( size_t i = 0; i < MAX_LOCAL_PLAYERS; i++ )
		{
			if( players[i] != nullptr )
			{
				m_ListenerA[i].bValid=true;
				F32 x,y,z;
				x=players[i]->xo + (players[i]->x - players[i]->xo) * a;
				y=players[i]->yo + (players[i]->y - players[i]->yo) * a;
				z=players[i]->zo + (players[i]->z - players[i]->zo) * a;

				float yRot = players[i]->yRotO + (players[i]->yRot - players[i]->yRotO) * a;
				float yCos = (float)cos(yRot * Mth::RAD_TO_GRAD);
				float ySin = (float)sin(yRot * Mth::RAD_TO_GRAD);

				// store the listener positions for splitscreen
				m_ListenerA[i].vPosition.x = x;
				m_ListenerA[i].vPosition.y = y;
				m_ListenerA[i].vPosition.z = z;  

				m_ListenerA[i].vOrientFront.x = -ySin;
				m_ListenerA[i].vOrientFront.y = 0;
				m_ListenerA[i].vOrientFront.z = yCos;

				listenerCount++;
			}
			else
			{
				m_ListenerA[i].bValid=false;
			}
		}
	}


	// If there were no valid players set, make up a default listener
	if( listenerCount == 0 )
	{
		m_ListenerA[0].vPosition.x = 0;
		m_ListenerA[0].vPosition.y = 0;
		m_ListenerA[0].vPosition.z = 0;
		m_ListenerA[0].vOrientFront.x = 0;
		m_ListenerA[0].vOrientFront.y = 0;
		m_ListenerA[0].vOrientFront.z = 1.0f;
		listenerCount++;
	}
	m_validListenerCount = listenerCount;

#ifdef __PSVITA__
	// AP - Show that a change has occurred so we know to update the values at the next Mixer callback
	SoundEngine_Change = true;
#else
	updateMiniAudio();
#endif
}

/////////////////////////////////////////////
//
//	SoundEngine
//
/////////////////////////////////////////////
SoundEngine::SoundEngine()
{
	random = new Random();
	memset(&m_engine, 0, sizeof(ma_engine));
    memset(&m_engineConfig, 0, sizeof(ma_engine_config));
    m_musicStreamActive = false;
	m_StreamState=eMusicStreamState_Idle;
	m_iMusicDelay=0;
	m_validListenerCount=0; 

	m_bHeardTrackA=nullptr;

	// Start the streaming music playing some music from the overworld
	SetStreamingSounds(eStream_Overworld_Calm1,eStream_Overworld_piano3,
		eStream_Nether1,eStream_Nether4,
		eStream_end_dragon,eStream_end_end,
		eStream_CD_1);

	m_musicID=getMusicID(LevelData::DIMENSION_OVERWORLD);

	m_StreamingAudioInfo.bIs3D=false;
	m_StreamingAudioInfo.x=0;
	m_StreamingAudioInfo.y=0;
	m_StreamingAudioInfo.z=0;
	m_StreamingAudioInfo.volume=1;
	m_StreamingAudioInfo.pitch=1;

	memset(CurrentSoundsPlaying,0,sizeof(int)*(eSoundType_MAX+eSFX_MAX));
	memset(m_ListenerA,0,sizeof(AUDIO_LISTENER)*XUSER_MAX_COUNT);

#ifdef __ORBIS__
	m_hBGMAudio=GetAudioBGMHandle();
#endif
}

void SoundEngine::destroy() {}

#ifdef _DEBUG
void SoundEngine::GetSoundName(char *szSoundName,int iSound)
{
	strcpy((char *)szSoundName,"Minecraft/");
	wstring name = wchSoundNames[iSound];
	char *SoundName = (char *)ConvertSoundPathToName(name);
	strcat((char *)szSoundName,SoundName);
}
#endif

/////////////////////////////////////////////
//
//	play
//
/////////////////////////////////////////////
void SoundEngine::play(int iSound, float x, float y, float z, float volume, float pitch)
{
    U8 szSoundName[256];

    if (iSound == -1)
    {
        app.DebugPrintf(6, "PlaySound with sound of -1 !!!!!!!!!!!!!!!\n");
        return;
    }

    strcpy((char*)szSoundName, "Minecraft/");

    wstring name = wchSoundNames[iSound];

    char* SoundName = (char*)ConvertSoundPathToName(name);
    strcat((char*)szSoundName, SoundName);

    app.DebugPrintf(6,
        "PlaySound - %d - %s - %s (%f %f %f, vol %f, pitch %f)\n",
        iSound, SoundName, szSoundName, x, y, z, volume, pitch);

    char basePath[256];
    sprintf_s(basePath, "Windows64Media/Sound/%s", (char*)szSoundName);

    char finalPath[256];
    sprintf_s(finalPath, "%s.wav", basePath);

	const char* extensions[] = { ".ogg", ".wav", ".mp3" };
	size_t extCount = sizeof(extensions) / sizeof(extensions[0]);
	bool found = false;

	for (size_t extIdx = 0; extIdx < extCount; extIdx++)
	{
		char basePlusExt[256];
		sprintf_s(basePlusExt, "%s%s", basePath, extensions[extIdx]);
		
		DWORD attr = GetFileAttributesA(basePlusExt);
		if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY))
		{
			sprintf_s(finalPath, "%s", basePlusExt);
			found = true;
			break;
		}
	}

	if (!found)
	{
		int count = 0;

		for (size_t extIdx = 0; extIdx < extCount; extIdx++)
		{
			for (size_t i = 1; i < 32; i++)
			{
				char numberedPath[256];
				sprintf_s(numberedPath, "%s%d%s", basePath, i, extensions[extIdx]);
				
				DWORD attr = GetFileAttributesA(numberedPath);
				if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY))
				{
					count = i;
				}
			}
		}

		if (count > 0)
		{
			int chosen = (rand() % count) + 1;
			for (size_t extIdx = 0; extIdx < extCount; extIdx++)
			{
				char numberedPath[256];
				sprintf_s(numberedPath, "%s%d%s", basePath, chosen, extensions[extIdx]);
				
				DWORD attr = GetFileAttributesA(numberedPath);
				if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY))
				{
					sprintf_s(finalPath, "%s", numberedPath);
					found = true;
					break;
				}
			}
			if (!found)
			{
				sprintf_s(finalPath, "%s%d.wav", basePath, chosen);
			}
		}
	}

    MiniAudioSound* s = new MiniAudioSound();
    memset(&s->info, 0, sizeof(AUDIO_INFO));

	s->info.x = x;
    s->info.y = y;
   	s->info.z = z;
	
    s->info.volume = volume;
    s->info.pitch = pitch;
    s->info.bIs3D = true;
    s->info.bUseSoundsPitchVal = false;
    s->info.iSound = iSound + eSFX_MAX;

    if (ma_sound_init_from_file(
            &m_engine,
            finalPath,
            MA_SOUND_FLAG_ASYNC,
            nullptr,
            nullptr,
            &s->sound) != MA_SUCCESS)
    {
        app.DebugPrintf("Failed to initialize sound from file: %s\n", finalPath);
        delete s;
        return;
    }

    ma_sound_set_spatialization_enabled(&s->sound, MA_TRUE);

    float finalVolume = volume * m_MasterEffectsVolume;
    if (finalVolume > 1.0f)
        finalVolume = 1.0f;

    ma_sound_set_volume(&s->sound, finalVolume);
    ma_sound_set_pitch(&s->sound, pitch);
    ma_sound_set_position(&s->sound, x, y, z);

    ma_sound_start(&s->sound);

    m_activeSounds.push_back(s);
}

/////////////////////////////////////////////
//
//	playUI
//
/////////////////////////////////////////////
void SoundEngine::playUI(int iSound, float volume, float pitch)
{
    U8 szSoundName[256];
    wstring name;

    if (iSound >= eSFX_MAX)
    {
        strcpy((char*)szSoundName, "Minecraft/");
        name = wchSoundNames[iSound];
    }
    else
    {
        strcpy((char*)szSoundName, "Minecraft/UI/");
        name = wchUISoundNames[iSound];
    }

    char* SoundName = (char*)ConvertSoundPathToName(name);
    strcat((char*)szSoundName, SoundName);

    char basePath[256];
    sprintf_s(basePath, "Windows64Media/Sound/Minecraft/UI/%s", ConvertSoundPathToName(name));

    char finalPath[256];
    sprintf_s(finalPath, "%s.wav", basePath);

	const char* extensions[] = { ".ogg", ".wav", ".mp3" };
	size_t count = sizeof(extensions) / sizeof(extensions[0]);
	bool found = false;
	for (size_t i = 0; i < count; i++)
	{
		sprintf_s(finalPath, "%s%s", basePath, extensions[i]);
		if (FileExists(finalPath))
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		app.DebugPrintf("No sound file found for UI sound: %s\n", basePath);
		return;
	}

    MiniAudioSound* s = new MiniAudioSound();
    memset(&s->info, 0, sizeof(AUDIO_INFO));

    s->info.volume = volume;
    s->info.pitch = pitch;
    s->info.bIs3D = false;
    s->info.bUseSoundsPitchVal = true;

    if (ma_sound_init_from_file(
            &m_engine,
            finalPath,
            MA_SOUND_FLAG_ASYNC,
            nullptr,
            nullptr,
            &s->sound) != MA_SUCCESS)
    {
        delete s;
        app.DebugPrintf("ma_sound_init_from_file failed: %s\n", finalPath);
        return;
    }

    ma_sound_set_spatialization_enabled(&s->sound, MA_FALSE);

    float finalVolume = volume * m_MasterEffectsVolume;
    if (finalVolume > 1.0f)
        finalVolume = 1.0f;
	printf("UI Sound volume set to %f\nEffects volume: %f\n", finalVolume, m_MasterEffectsVolume);

    ma_sound_set_volume(&s->sound, finalVolume);
    ma_sound_set_pitch(&s->sound, pitch);

    ma_sound_start(&s->sound);

    m_activeSounds.push_back(s);
}

/////////////////////////////////////////////
//
//	playStreaming
//
/////////////////////////////////////////////
void SoundEngine::playStreaming(const wstring& name, float x, float y , float z, float volume, float pitch, bool bMusicDelay)
{
	// This function doesn't actually play a streaming sound, just sets states and an id for the music tick to play it
	// Level audio will be played when a play with an empty name comes in
	// CD audio will be played when a named stream comes in

	m_StreamingAudioInfo.x=x;
	m_StreamingAudioInfo.y=y;
	m_StreamingAudioInfo.z=z;
	m_StreamingAudioInfo.volume=volume;
	m_StreamingAudioInfo.pitch=pitch;

	if(m_StreamState==eMusicStreamState_Playing)
	{
		m_StreamState=eMusicStreamState_Stop;
	}
	else if(m_StreamState==eMusicStreamState_Opening)
	{
		m_StreamState=eMusicStreamState_OpeningCancel;
	}

	if(name.empty())
	{
		// music, or stop CD
		m_StreamingAudioInfo.bIs3D=false;

		// we need a music id
		// random delay of up to 3 minutes for music
		m_iMusicDelay = random->nextInt(20 * 60 * 3);//random->nextInt(20 * 60 * 10) + 20 * 60 * 10;

#ifdef _DEBUG
		m_iMusicDelay=0;
#endif
		Minecraft *pMinecraft=Minecraft::GetInstance();

		bool playerInEnd=false;
		bool playerInNether=false;

		for(unsigned int i=0;i<MAX_LOCAL_PLAYERS;i++)
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
		if(playerInEnd)
		{
			m_musicID = getMusicID(LevelData::DIMENSION_END);
		}
		else if(playerInNether)
		{
			m_musicID = getMusicID(LevelData::DIMENSION_NETHER);
		}
		else
		{
			m_musicID = getMusicID(LevelData::DIMENSION_OVERWORLD);
		}
	}
	else
	{
		// jukebox
		m_StreamingAudioInfo.bIs3D=true;
		m_musicID=getMusicID(name);
		m_iMusicDelay=0;
	}
}


int SoundEngine::GetRandomishTrack(int iStart,int iEnd)
{
	// 4J-PB - make it more likely that we'll get a track we've not heard for a while, although repeating tracks sometimes is fine

	// if all tracks have been heard, clear the flags
	bool bAllTracksHeard=true;
	int iVal=iStart;
	for(size_t i=iStart;i<=iEnd;i++)
	{
		if(m_bHeardTrackA[i]==false) 
		{
			bAllTracksHeard=false;
			app.DebugPrintf("Not heard all tracks yet\n");
			break;
		}
	}

	if(bAllTracksHeard)
	{
		app.DebugPrintf("Heard all tracks - resetting the tracking array\n");

		for(size_t i=iStart;i<=iEnd;i++)
		{
			m_bHeardTrackA[i]=false;
		}
	}

	// trying to get a track we haven't heard, but not too hard		
	for(size_t i=0;i<=((iEnd-iStart)/2);i++)
	{
		// random->nextInt(1) will always return 0
		iVal=random->nextInt((iEnd-iStart)+1)+iStart;
		if(m_bHeardTrackA[iVal]==false)
		{
			// not heard this
			app.DebugPrintf("(%d) Not heard track %d yet, so playing it now\n",i,iVal);
			m_bHeardTrackA[iVal]=true;
			break;
		}
		else
		{
			app.DebugPrintf("(%d) Skipping track %d already heard it recently\n",i,iVal);
		}
	}

	app.DebugPrintf("Select track %d\n",iVal);
	return iVal;
}
/////////////////////////////////////////////
//
//	getMusicID
//
/////////////////////////////////////////////
int SoundEngine::getMusicID(int iDomain)
{
	int iRandomVal=0;
	Minecraft *pMinecraft=Minecraft::GetInstance();

	// Before the game has started?
	if(pMinecraft==nullptr)
	{
		// any track from the overworld
		return GetRandomishTrack(m_iStream_Overworld_Min,m_iStream_Overworld_Max);
	}

	if(pMinecraft->skins->isUsingDefaultSkin())
	{
		switch(iDomain)
		{
		case LevelData::DIMENSION_END:
			// the end isn't random - it has different music depending on whether the dragon is alive or not, but we've not added the dead dragon music yet
			return m_iStream_End_Min;
		case LevelData::DIMENSION_NETHER:
			return GetRandomishTrack(m_iStream_Nether_Min,m_iStream_Nether_Max);
			//return m_iStream_Nether_Min + random->nextInt(m_iStream_Nether_Max-m_iStream_Nether_Min);
		default: //overworld
			//return m_iStream_Overworld_Min + random->nextInt(m_iStream_Overworld_Max-m_iStream_Overworld_Min);
			return GetRandomishTrack(m_iStream_Overworld_Min,m_iStream_Overworld_Max);
		}
	}
	else
	{
		// using a texture pack - may have multiple End music tracks
		switch(iDomain)
		{
		case LevelData::DIMENSION_END:
			return GetRandomishTrack(m_iStream_End_Min,m_iStream_End_Max);
		case LevelData::DIMENSION_NETHER:
			//return m_iStream_Nether_Min + random->nextInt(m_iStream_Nether_Max-m_iStream_Nether_Min);
			return GetRandomishTrack(m_iStream_Nether_Min,m_iStream_Nether_Max);
		default: //overworld
			//return m_iStream_Overworld_Min + random->nextInt(m_iStream_Overworld_Max-m_iStream_Overworld_Min);
			return GetRandomishTrack(m_iStream_Overworld_Min,m_iStream_Overworld_Max);
		}
	}
}

/////////////////////////////////////////////
//
//	getMusicID
//
/////////////////////////////////////////////
// check what the CD is
int SoundEngine::getMusicID(const wstring& name)
{
	int iCD=0;
	char *SoundName = (char *)ConvertSoundPathToName(name,true);

	// 4J-PB - these will always be the game cds, so use the m_szStreamFileA for this
	for(size_t i=0;i<12;i++)
	{
		if(strcmp(SoundName,m_szStreamFileA[i+eStream_CD_1])==0)
		{
			iCD=i;
			break;
		}
	}

	// adjust for cd start position on normal or mash-up pack
	return iCD+m_iStream_CD_1;
}

/////////////////////////////////////////////
//
//	getMasterMusicVolume
//
/////////////////////////////////////////////
float SoundEngine::getMasterMusicVolume()
{
	if( m_bSystemMusicPlaying )
	{
		return 0.0f;
	}
	else
	{
		return m_MasterMusicVolume;
	}
}

/////////////////////////////////////////////
//
//	updateMusicVolume
//
/////////////////////////////////////////////
void SoundEngine::updateMusicVolume(float fVal)
{
	m_MasterMusicVolume=fVal;
}

/////////////////////////////////////////////
//
//	updateSystemMusicPlaying
//
/////////////////////////////////////////////
void SoundEngine::updateSystemMusicPlaying(bool isPlaying)
{
	m_bSystemMusicPlaying = isPlaying;
}

/////////////////////////////////////////////
//
//	updateSoundEffectVolume
//
/////////////////////////////////////////////
void SoundEngine::updateSoundEffectVolume(float fVal) 
{
	m_MasterEffectsVolume=fVal;
	//AIL_set_variable_float(0,"UserEffectVol",fVal);
}

void SoundEngine::add(const wstring& name, File *file) {}
void SoundEngine::addMusic(const wstring& name, File *file) {}
void SoundEngine::addStreaming(const wstring& name, File *file) {}
bool SoundEngine::isStreamingWavebankReady() { return true; }

int SoundEngine::OpenStreamThreadProc(void* lpParameter)
{
    SoundEngine* soundEngine = (SoundEngine*)lpParameter;

	const char* ext = strrchr(soundEngine->m_szStreamName, '.');
    
    if (soundEngine->m_musicStreamActive)
    {
        ma_sound_stop(&soundEngine->m_musicStream);
        ma_sound_uninit(&soundEngine->m_musicStream);
        soundEngine->m_musicStreamActive = false;
    }

    ma_result result = ma_sound_init_from_file(
            &soundEngine->m_engine,
            soundEngine->m_szStreamName,
            MA_SOUND_FLAG_STREAM,
            nullptr,
            nullptr,
            &soundEngine->m_musicStream);

    if (result != MA_SUCCESS)
    {
		app.DebugPrintf("SoundEngine::OpenStreamThreadProc - Failed to open stream: %s\n", soundEngine->m_szStreamName);
        return 0;
    }

    ma_sound_set_spatialization_enabled(&soundEngine->m_musicStream, MA_FALSE);
    ma_sound_set_looping(&soundEngine->m_musicStream, MA_FALSE);

    soundEngine->m_musicStreamActive = true;

    return 0;
}

/////////////////////////////////////////////
//
//	playMusicTick
//
/////////////////////////////////////////////
void SoundEngine::playMusicTick() 
{
// AP - vita will update the music during the mixer callback
#ifndef __PSVITA__
	playMusicUpdate();
#endif
}

// AP - moved to a separate function so it can be called from the mixer callback on Vita
void SoundEngine::playMusicUpdate() 
{
	static float fMusicVol = 0.0f;
	fMusicVol = getMasterMusicVolume();

	switch(m_StreamState)
	{
	case eMusicStreamState_Idle:

		// start a stream playing
		if (m_iMusicDelay > 0)
		{
			m_iMusicDelay--;
			return;
		}

		if (m_musicStreamActive)
		{
			app.DebugPrintf("WARNING: m_musicStreamActive already true in Idle state, resetting to Playing\n");
			m_StreamState = eMusicStreamState_Playing;
			return;
		}

		if(m_musicID!=-1)
		{
			// start playing it


#if ( defined __PS3__  || defined __PSVITA__ || defined __ORBIS__ )

#ifdef __PS3__
			// 4J-PB - Need to check if we are a patched BD build
			if(app.GetBootedFromDiscPatch())
			{
				sprintf(m_szStreamName,"%s/%s",app.GetBDUsrDirPath(m_szMusicPath), m_szMusicPath );		
				app.DebugPrintf("SoundEngine::playMusicUpdate - (booted from disc patch) music path - %s",m_szStreamName);
			}
			else
			{
				sprintf(m_szStreamName,"%s/%s",getUsrDirPath(), m_szMusicPath );
			}
#else
			sprintf(m_szStreamName,"%s/%s",getUsrDirPath(), m_szMusicPath );
#endif

#else
			strcpy((char *)m_szStreamName,m_szMusicPath);
#endif
			// are we using a mash-up pack?
			//if(pMinecraft && !pMinecraft->skins->isUsingDefaultSkin() && pMinecraft->skins->getSelected()->hasAudio())
			if(Minecraft::GetInstance()->skins->getSelected()->hasAudio())
			{
				// It's a mash-up - need to use the DLC path for the music
				TexturePack *pTexPack=Minecraft::GetInstance()->skins->getSelected();
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)pTexPack;
				DLCPack *pack = pDLCTexPack->getDLCInfoParentPack();
				DLCAudioFile *dlcAudioFile = (DLCAudioFile *) pack->getFile(DLCManager::e_DLCType_Audio, 0);

				app.DebugPrintf("Mashup pack \n");

				// build the name

				// if the music ID is beyond the end of the texture pack music files, then it's a CD
				if(m_musicID<m_iStream_CD_1)
				{
					SetIsPlayingStreamingGameMusic(true);
					SetIsPlayingStreamingCDMusic(false);
					m_MusicType=eMusicType_Game;
					m_StreamingAudioInfo.bIs3D=false;
				
#ifdef _XBOX_ONE
					wstring &wstrSoundName=dlcAudioFile->GetSoundName(m_musicID);
					wstring wstrFile=L"TPACK:\\Data\\" + wstrSoundName +L".wav";
					std::wstring mountedPath = StorageManager.GetMountedPath(wstrFile);
					wcstombs(m_szStreamName,mountedPath.c_str(),255);
#else
					wstring &wstrSoundName=dlcAudioFile->GetSoundName(m_musicID);
					char szName[255];
					wcstombs(szName,wstrSoundName.c_str(),255);

#if defined __PS3__ || defined __ORBIS__ || defined __PSVITA__
					string strFile="TPACK:/Data/" + string(szName) + ".wav";
#else
					string strFile="TPACK:\\Data\\" + string(szName) + ".wav";
#endif
					std::string mountedPath = StorageManager.GetMountedPath(strFile);
					strcpy(m_szStreamName,mountedPath.c_str());
#endif
				}
				else
				{
					SetIsPlayingStreamingGameMusic(false);
					SetIsPlayingStreamingCDMusic(true);
					m_MusicType=eMusicType_CD;
					m_StreamingAudioInfo.bIs3D=true;

					// Need to adjust to index into the cds in the game's m_szStreamFileA
					strcat((char *)m_szStreamName,"cds/");
					strcat((char *)m_szStreamName,m_szStreamFileA[m_musicID-m_iStream_CD_1+eStream_CD_1]);
					strcat((char *)m_szStreamName,".wav");
				}
			}
			else
			{	
				// 4J-PB - if this is a PS3 disc patch, we have to check if the music file is in the patch data
#ifdef __PS3__
				if(app.GetBootedFromDiscPatch() && (m_musicID<m_iStream_CD_1))
				{
					// rebuild the path for the music
					strcpy((char *)m_szStreamName,m_szMusicPath);
					strcat((char *)m_szStreamName,"music/");
					strcat((char *)m_szStreamName,m_szStreamFileA[m_musicID]);
					strcat((char *)m_szStreamName,".wav");

					// check if this is in the patch data
					sprintf(m_szStreamName,"%s/%s",app.GetBDUsrDirPath(m_szStreamName), m_szMusicPath );		
					strcat((char *)m_szStreamName,"music/");
					strcat((char *)m_szStreamName,m_szStreamFileA[m_musicID]);
					strcat((char *)m_szStreamName,".wav");

					SetIsPlayingStreamingGameMusic(true);
					SetIsPlayingStreamingCDMusic(false);
					m_MusicType=eMusicType_Game;
					m_StreamingAudioInfo.bIs3D=false;
				}
				else if(m_musicID<m_iStream_CD_1)
				{
					SetIsPlayingStreamingGameMusic(true);
					SetIsPlayingStreamingCDMusic(false);
					m_MusicType=eMusicType_Game;
					m_StreamingAudioInfo.bIs3D=false;
					// build the name
					strcat((char *)m_szStreamName,"music/");
					strcat((char *)m_szStreamName,m_szStreamFileA[m_musicID]);
					strcat((char *)m_szStreamName,".wav");
				}

				else
				{
					SetIsPlayingStreamingGameMusic(false);
					SetIsPlayingStreamingCDMusic(true);
					m_MusicType=eMusicType_CD;
					m_StreamingAudioInfo.bIs3D=true;
					// build the name
					strcat((char *)m_szStreamName,"cds/");
					strcat((char *)m_szStreamName,m_szStreamFileA[m_musicID]);
					strcat((char *)m_szStreamName,".wav");
				}
#else						
				if(m_musicID<m_iStream_CD_1)
				{
					SetIsPlayingStreamingGameMusic(true);
					SetIsPlayingStreamingCDMusic(false);
					m_MusicType=eMusicType_Game;
					m_StreamingAudioInfo.bIs3D=false;
					// build the name
					strcat((char *)m_szStreamName,"music/");
				}
				else
				{
					SetIsPlayingStreamingGameMusic(false);
					SetIsPlayingStreamingCDMusic(true);
					m_MusicType=eMusicType_CD;
					m_StreamingAudioInfo.bIs3D=true;
					// build the name
					strcat((char *)m_szStreamName,"cds/");
				}
				strcat((char *)m_szStreamName,m_szStreamFileA[m_musicID]);
				strcat((char *)m_szStreamName,".wav");
				
#endif
			}

			// 			wstring name = m_szStreamFileA[m_musicID];
			// 			char *SoundName = (char *)ConvertSoundPathToName(name);
			// 			strcat((char *)szStreamName,SoundName);

			FILE* pFile = nullptr;
			
			if (fopen_s(&pFile, reinterpret_cast<char*>(m_szStreamName), "rb") == 0 && pFile)
			{
				fclose(pFile);
			}
			else
			{
				const char* extensions[] = { ".ogg", ".mp3", ".wav" };
				size_t extCount = sizeof(extensions) / sizeof(extensions[0]);
				bool found = false;

				char* dotPos = strrchr(reinterpret_cast<char*>(m_szStreamName), '.');
				if (dotPos != nullptr && (dotPos - reinterpret_cast<char*>(m_szStreamName)) < 250)
				{
					for (size_t i = 0; i < extCount; i++)
					{
						strcpy_s(dotPos, 5, extensions[i]);
						
						if (fopen_s(&pFile, reinterpret_cast<char*>(m_szStreamName), "rb") == 0 && pFile)
						{
							fclose(pFile);
							found = true;
							break;
						}
					}
				}

				if (!found)
				{
					if (dotPos != nullptr)
					{
						strcpy_s(dotPos, 5, ".wav");
					}
					app.DebugPrintf("WARNING: No audio file found for music ID %d (tried .ogg, .mp3, .wav)\n", m_musicID);
					return;
				}
			}

			app.DebugPrintf("Starting streaming - %s\n",m_szStreamName);
			m_openStreamThread = new C4JThread(OpenStreamThreadProc, this, "OpenStreamThreadProc");
			m_openStreamThread->Run();
			m_StreamState = eMusicStreamState_Opening;
		}
		break;

	case eMusicStreamState_Opening:
		if( !m_openStreamThread->isRunning() )
		{
			delete m_openStreamThread;
			m_openStreamThread = nullptr;

			app.DebugPrintf("OpenStreamThreadProc finished. m_musicStreamActive=%d\n", m_musicStreamActive);

			if (!m_musicStreamActive)
			{
				const char* currentExt = strrchr(reinterpret_cast<char*>(m_szStreamName), '.');
				if (currentExt && _stricmp(currentExt, ".wav") == 0)
				{
					const bool isCD = (m_musicID >= m_iStream_CD_1);
					const char* folder = isCD ? "cds/" : "music/";
					
					int n = sprintf_s(reinterpret_cast<char*>(m_szStreamName), 512, "%s%s%s.wav", m_szMusicPath, folder, m_szStreamFileA[m_musicID]);
					
					if (n > 0)
					{
						FILE* pFile = nullptr;
						if (fopen_s(&pFile, reinterpret_cast<char*>(m_szStreamName), "rb") == 0 && pFile)
						{
							fclose(pFile);
													
							m_openStreamThread = new C4JThread(OpenStreamThreadProc, this, "OpenStreamThreadProc");
							m_openStreamThread->Run();
							break;
						}
					}
				}
				
				m_StreamState = eMusicStreamState_Idle;
				break;
			}
			
			if (m_StreamingAudioInfo.bIs3D)
			{
				ma_sound_set_spatialization_enabled(&m_musicStream, MA_TRUE);
				ma_sound_set_position(&m_musicStream, m_StreamingAudioInfo.x, m_StreamingAudioInfo.y, m_StreamingAudioInfo.z);
			}
			else
			{
				ma_sound_set_spatialization_enabled(&m_musicStream, MA_FALSE);
			}

			ma_sound_set_pitch(&m_musicStream, m_StreamingAudioInfo.pitch);

			float finalVolume = m_StreamingAudioInfo.volume * getMasterMusicVolume();

			ma_sound_set_volume(&m_musicStream, finalVolume);
			ma_result startResult = ma_sound_start(&m_musicStream);
			app.DebugPrintf("ma_sound_start result: %d\n", startResult);

			m_StreamState=eMusicStreamState_Playing;
		}
		break;
	case eMusicStreamState_OpeningCancel:
		if( !m_openStreamThread->isRunning() )
		{
			delete m_openStreamThread;
			m_openStreamThread = nullptr;
			m_StreamState = eMusicStreamState_Stop;
		}
		break;
	case eMusicStreamState_Stop:
		if (m_musicStreamActive)
		{
			ma_sound_stop(&m_musicStream);
			ma_sound_uninit(&m_musicStream);
			m_musicStreamActive = false;
		}

		SetIsPlayingStreamingCDMusic(false);
		SetIsPlayingStreamingGameMusic(false);

		m_StreamState = eMusicStreamState_Idle;
	break;
	case eMusicStreamState_Stopping:
		break;
	case eMusicStreamState_Play:
		break;
	case eMusicStreamState_Playing:
		{
			static int frameCount = 0;
			if (frameCount++ % 60 == 0)
			{
				if (m_musicStreamActive)
				{
					bool isPlaying = ma_sound_is_playing(&m_musicStream);
					float vol = ma_sound_get_volume(&m_musicStream);
					bool isAtEnd = ma_sound_at_end(&m_musicStream);
				}
			}
		}
		if(GetIsPlayingStreamingGameMusic())
		{
			//if(m_MusicInfo.pCue!=nullptr)
			{
				bool playerInEnd = false;
				bool playerInNether=false;
				Minecraft *pMinecraft = Minecraft::GetInstance();
				for(unsigned int i = 0; i < MAX_LOCAL_PLAYERS; ++i)
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

				if(playerInEnd && !GetIsPlayingEndMusic())
				{
					m_StreamState=eMusicStreamState_Stop;

					// Set the end track
					m_musicID = getMusicID(LevelData::DIMENSION_END);
					SetIsPlayingEndMusic(true);
					SetIsPlayingNetherMusic(false);					
				}
				else if(!playerInEnd && GetIsPlayingEndMusic())
				{
					if(playerInNether)
					{
						m_StreamState=eMusicStreamState_Stop;

						// Set the end track
						m_musicID = getMusicID(LevelData::DIMENSION_NETHER);
						SetIsPlayingEndMusic(false);
						SetIsPlayingNetherMusic(true);					
					}
					else
					{
						m_StreamState=eMusicStreamState_Stop;

						// Set the end track
						m_musicID = getMusicID(LevelData::DIMENSION_OVERWORLD);
						SetIsPlayingEndMusic(false);
						SetIsPlayingNetherMusic(false);					
					}
				}
				else if (playerInNether && !GetIsPlayingNetherMusic())
				{
					m_StreamState=eMusicStreamState_Stop;
					// set the Nether track
					m_musicID = getMusicID(LevelData::DIMENSION_NETHER);
					SetIsPlayingNetherMusic(true);
					SetIsPlayingEndMusic(false);
				}
				else if(!playerInNether && GetIsPlayingNetherMusic())
				{
					if(playerInEnd)
					{
						m_StreamState=eMusicStreamState_Stop;
						// set the Nether track
						m_musicID = getMusicID(LevelData::DIMENSION_END);
						SetIsPlayingNetherMusic(false);
						SetIsPlayingEndMusic(true);
					}
					else
					{
						m_StreamState=eMusicStreamState_Stop;
						// set the Nether track
						m_musicID = getMusicID(LevelData::DIMENSION_OVERWORLD);
						SetIsPlayingNetherMusic(false);
						SetIsPlayingEndMusic(false);
					}
				}

				// volume change required?
				if (m_musicStreamActive)
				{
					float finalVolume = m_StreamingAudioInfo.volume * fMusicVol;

					ma_sound_set_volume(&m_musicStream, finalVolume);
				}
			}
		}
		else
		{
			// Music disc playing - if it's a 3D stream, then set the position - we don't have any streaming audio in the world that moves, so this isn't
			// required unless we have more than one listener, and are setting the listening position to the origin and setting a fake position
			// for the sound down  the z axis
			if (m_StreamingAudioInfo.bIs3D && m_validListenerCount > 1)
			{
				int iClosestListener = 0;
				float fClosestDist = 1e6f;

				for (size_t i = 0; i < MAX_LOCAL_PLAYERS; i++)
				{
					if (m_ListenerA[i].bValid)
					{
						float dx = m_StreamingAudioInfo.x - m_ListenerA[i].vPosition.x;
						float dy = m_StreamingAudioInfo.y - m_ListenerA[i].vPosition.y;
						float dz = m_StreamingAudioInfo.z - m_ListenerA[i].vPosition.z;
						float dist = sqrtf(dx*dx + dy*dy + dz*dz);

						if (dist < fClosestDist)
						{
							fClosestDist = dist;
							iClosestListener = i;
						}
					}
				}

				float relX = m_StreamingAudioInfo.x - m_ListenerA[iClosestListener].vPosition.x;
				float relY = m_StreamingAudioInfo.y - m_ListenerA[iClosestListener].vPosition.y;
				float relZ = m_StreamingAudioInfo.z - m_ListenerA[iClosestListener].vPosition.z;

				if (m_musicStreamActive)
				{
					ma_sound_set_position(&m_musicStream, relX, relY, relZ);
				}
			}
		}

		break;

	case eMusicStreamState_Completed:
		{	
			// random delay of up to 3 minutes for music
			m_iMusicDelay = random->nextInt(20 * 60 * 3);//random->nextInt(20 * 60 * 10) + 20 * 60 * 10;
			// Check if we have a local player in The Nether or in The End, and play that music if they are
			Minecraft *pMinecraft=Minecraft::GetInstance();
			bool playerInEnd=false;
			bool playerInNether=false;

			for(unsigned int i=0;i<MAX_LOCAL_PLAYERS;i++)
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
			if(playerInEnd)
			{
				m_musicID = getMusicID(LevelData::DIMENSION_END);
				SetIsPlayingEndMusic(true);
				SetIsPlayingNetherMusic(false);
			}
			else if(playerInNether)
			{
				m_musicID = getMusicID(LevelData::DIMENSION_NETHER);
				SetIsPlayingNetherMusic(true);
				SetIsPlayingEndMusic(false);
			}
			else
			{
				m_musicID = getMusicID(LevelData::DIMENSION_OVERWORLD);
				SetIsPlayingNetherMusic(false);
				SetIsPlayingEndMusic(false);
			}

			m_StreamState=eMusicStreamState_Idle;
		}
		break;
	}

	// check the status of the stream - this is for when a track completes rather than is stopped by the user action

	if (m_musicStreamActive)
	{
		if (!ma_sound_is_playing(&m_musicStream) && ma_sound_at_end(&m_musicStream))
		{
			ma_sound_uninit(&m_musicStream);
			m_musicStreamActive = false;

			SetIsPlayingStreamingCDMusic(false);
			SetIsPlayingStreamingGameMusic(false);

			m_StreamState = eMusicStreamState_Completed;
		}
	}
}


/////////////////////////////////////////////
//
//	ConvertSoundPathToName
//
/////////////////////////////////////////////
char *SoundEngine::ConvertSoundPathToName(const wstring& name, bool bConvertSpaces)
{
	static char buf[256];
	assert(name.length()<256);
	for(unsigned int i = 0; i < name.length(); i++ )
	{
		wchar_t c = name[i];
		if(c=='.') c='/';
		if(bConvertSpaces)
		{
			if(c==' ') c='_';
		}
		buf[i] = (char)c;
	}
	buf[name.length()] = 0;
	return buf;
}

#endif