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
	"piano1",
	"piano2",
	"piano3", // 11

#ifndef _XBOX
	"creative1",
	"creative2",
	"creative3",
	"creative4",
	"creative5",
	"creative6", // 17

	"menu1",
	"menu2",
	"menu3",
	"menu4", // 21
#endif

	// Nether
	"nether1",
	"nether2",
	"nether3",
	"nether4", // 25

	// The End
	"the_end_dragon_alive",
	"the_end_end", // 27
	
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

void SoundEngine::SetStreamingSounds(int iMenuMin, int iMenuMax,
	int iOverworldSurvivalMin, int iOverWorldSurvivalMax,
	int iOverworldCreativeMin, int iOverWorldCreativeMax,
	int iNetherMin, int iNetherMax,
	int iEndMin, int iEndMax,
	int iCD1)
{
	using Domain = MusicTrackManager::Domain;

	m_musicTrackManager.setDomainRange(Domain::Menu, iMenuMin, iMenuMax);
	m_musicTrackManager.setDomainRange(Domain::OverworldSurvival, iOverworldSurvivalMin, iOverWorldSurvivalMax);
	m_musicTrackManager.setDomainRange(Domain::OverworldCreative, iOverworldCreativeMin, iOverWorldCreativeMax);
	m_musicTrackManager.setDomainRange(Domain::Nether, iNetherMin, iNetherMax);
	m_musicTrackManager.setDomainRange(Domain::End, iEndMin, iEndMax);

	m_iStream_CD_1 = iCD1;
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
SoundEngine::SoundEngine(): random(new Random()), m_musicTrackManager(random), m_currentMusicDomain(MusicTrackManager::Domain::Menu)
{
	memset(&m_engine, 0, sizeof(ma_engine));
    memset(&m_engineConfig, 0, sizeof(ma_engine_config));
    m_musicStreamActive = false;
	m_StreamState=eMusicStreamState_Idle;
	m_iMusicDelay=0;
	m_validListenerCount=0; 

    m_musicTrackManager.setDomainRange(MusicTrackManager::Domain::Menu,
        eStream_Overworld_Menu1,
        eStream_Overworld_Menu4);

    m_musicTrackManager.setDomainRange(MusicTrackManager::Domain::OverworldSurvival,
        eStream_Overworld_Calm1,
        eStream_Overworld_piano3);

    m_musicTrackManager.setDomainRange(MusicTrackManager::Domain::OverworldCreative,
        eStream_Overworld_Creative1,
        eStream_Overworld_Creative6);

    m_musicTrackManager.setDomainRange(MusicTrackManager::Domain::Nether,
        eStream_Nether1,
        eStream_Nether4);

    m_musicTrackManager.setDomainRange(MusicTrackManager::Domain::End,
        eStream_end_dragon,
        eStream_end_end);

    m_iStream_CD_1 = eStream_CD_1;

    m_musicID = getTrackForDomain(MusicTrackManager::Domain::Menu);

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
MusicTrackManager::Domain SoundEngine::determineCurrentMusicDomain() const
{
	Minecraft* mc = Minecraft::GetInstance();
	if (!mc || !mc->level)
		return MusicTrackManager::Domain::Menu;

	bool inEnd = false, inNether = false, creative = false;

	for (unsigned int i = 0; i < MAX_LOCAL_PLAYERS; ++i)
	{
		auto player = mc->localplayers[i];
		if (!player) continue;

		if (player->dimension == LevelData::DIMENSION_END)
			inEnd = true;
		else if (player->dimension == LevelData::DIMENSION_NETHER)
			inNether = true;

		if (player->level->getLevelData()->getGameType()->isCreative())
			creative = true;
	}

	if (inEnd) return MusicTrackManager::Domain::End;
	if (inNether) return MusicTrackManager::Domain::Nether;
	if (creative) return MusicTrackManager::Domain::OverworldCreative;
	return MusicTrackManager::Domain::OverworldSurvival;
}

void SoundEngine::playStreaming(const wstring& name, float x, float y, float z, float volume, float pitch, bool bMusicDelay)
{
	m_StreamingAudioInfo.x = x;
	m_StreamingAudioInfo.y = y;
	m_StreamingAudioInfo.z = z;
	m_StreamingAudioInfo.volume = volume;
	m_StreamingAudioInfo.pitch = pitch;

	if (m_StreamState == eMusicStreamState_Playing)
		m_StreamState = eMusicStreamState_Stop;
	else if (m_StreamState == eMusicStreamState_Opening)
		m_StreamState = eMusicStreamState_OpeningCancel;

	if (name.empty())
	{
		m_StreamingAudioInfo.bIs3D = false;
		m_iMusicDelay = bMusicDelay ? random->nextInt(20 * 60 * 3) : 0;

#ifdef _DEBUG
		m_iMusicDelay = 0;
#endif

		MusicTrackManager::Domain domain = determineCurrentMusicDomain();
        m_currentMusicDomain = domain;
        m_musicID = getTrackForDomain(domain);
	}
	else
	{
		// Disk (jukebox)
		m_StreamingAudioInfo.bIs3D = true;
		m_musicID = getMusicID(name);
		m_iMusicDelay = 0;
	}
}

/////////////////////////////////////////////
//
//	getTrackForDomain(MusicTrackManager::Domain domain)
//
/////////////////////////////////////////////
int SoundEngine::getTrackForDomain(MusicTrackManager::Domain domain)
{
    if (domain == MusicTrackManager::Domain::End)
        return m_musicTrackManager.getDomainMin(domain);
    else
        return m_musicTrackManager.selectTrack(domain);
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

//=============================================================================
// playMusicUpdate
// Called every frame (or via mixer callback on Vita) to manage streaming music.
// Handles state machine for background music and music discs (jukebox).
//=============================================================================
void SoundEngine::playMusicUpdate()
{
    // Cache the current master music volume (may be zero if system music is playing)
    float masterVolume = getMasterMusicVolume();

    //-------------------------------------------------------------------------
    // State machine for streaming audio (background music or discs)
    //-------------------------------------------------------------------------
    switch (m_StreamState)
    {
        //---------------------------------------------------------------------
        // IDLE – no stream is open; waiting for a delay or ready to start
        //---------------------------------------------------------------------
    case eMusicStreamState_Idle:
    {
        // If a delay is active (e.g., between tracks), decrement and wait
        if (m_iMusicDelay > 0)
        {
            m_iMusicDelay--;
            return;
        }

        // Sanity check: if a stream is already active, something went wrong
        if (m_musicStreamActive)
        {
            app.DebugPrintf("WARNING: m_musicStreamActive already true in Idle state, resetting to Playing\n");
            m_StreamState = eMusicStreamState_Playing;
            return;
        }

        // If no track ID is selected, fallback to a default (should not happen)
        if (m_musicID == -1)
        {
            m_musicID = m_musicTrackManager.selectTrack(MusicTrackManager::Domain::Menu);
        }

        // Build the full file path for the selected music ID.
        // This block handles platform-specific paths, DLC/mash-up packs, and CD music.
        // The result is stored in m_szStreamName.
        {
            // Start with the base music path (platform‑dependent)
#if (defined __PS3__ || defined __PSVITA__ || defined __ORBIS__)
#ifdef __PS3__
            // PS3 special case: booted from disc patch vs. installed data
            if (app.GetBootedFromDiscPatch())
            {
                sprintf(m_szStreamName, "%s/%s", app.GetBDUsrDirPath(m_szMusicPath), m_szMusicPath);
                app.DebugPrintf("SoundEngine::playMusicUpdate - (booted from disc patch) music path - %s", m_szStreamName);
            }
            else
            {
                sprintf(m_szStreamName, "%s/%s", getUsrDirPath(), m_szMusicPath);
            }
#else
            // Other consoles (Vita, Orbis)
            sprintf(m_szStreamName, "%s/%s", getUsrDirPath(), m_szMusicPath);
#endif
#else
                // Windows / Durango – plain relative path
            strcpy((char*)m_szStreamName, m_szMusicPath);
#endif

            // Check if a mash‑up pack (DLC) is active and has custom audio
            if (Minecraft::GetInstance()->skins->getSelected()->hasAudio())
            {
                // Mash‑up pack: use DLC audio files
                TexturePack* pTexPack = Minecraft::GetInstance()->skins->getSelected();
                DLCTexturePack* pDLCTexPack = (DLCTexturePack*)pTexPack;
                DLCPack* pack = pDLCTexPack->getDLCInfoParentPack();
                DLCAudioFile* dlcAudioFile = (DLCAudioFile*)pack->getFile(DLCManager::e_DLCType_Audio, 0);

                app.DebugPrintf("Mashup pack\n");

                // Determine whether this is game music (track index < first CD) or a CD
                if (m_musicID < m_iStream_CD_1)
                {
                    // Game music from the mash‑up pack
                    SetIsPlayingStreamingGameMusic(true);
                    SetIsPlayingStreamingCDMusic(false);
                    m_MusicType = eMusicType_Game;
                    m_StreamingAudioInfo.bIs3D = false;

#ifdef _XBOX_ONE
                    // Xbox One: use StorageManager to resolve TPACK path
                    wstring& wstrSoundName = dlcAudioFile->GetSoundName(m_musicID);
                    wstring wstrFile = L"TPACK:\\Data\\" + wstrSoundName + L".wav";
                    std::wstring mountedPath = StorageManager.GetMountedPath(wstrFile);
                    wcstombs(m_szStreamName, mountedPath.c_str(), 255);
#else
                    // Other platforms: convert wstring to char and build TPACK path
                    wstring& wstrSoundName = dlcAudioFile->GetSoundName(m_musicID);
                    char szName[255];
                    wcstombs(szName, wstrSoundName.c_str(), 255);

#if defined __PS3__ || defined __ORBIS__ || defined __PSVITA__
                    string strFile = "TPACK:/Data/" + string(szName) + ".wav";
#else
                    string strFile = "TPACK:\\Data\\" + string(szName) + ".wav";
#endif
                    std::string mountedPath = StorageManager.GetMountedPath(strFile);
                    strcpy(m_szStreamName, mountedPath.c_str());
#endif
                }
                else
                {
                    // CD track from the mash‑up pack
                    SetIsPlayingStreamingGameMusic(false);
                    SetIsPlayingStreamingCDMusic(true);
                    m_MusicType = eMusicType_CD;
                    m_StreamingAudioInfo.bIs3D = true;

                    // Append "cds/" and the base filename (from the global stream file array)
                    strcat((char*)m_szStreamName, "cds/");
                    strcat((char*)m_szStreamName, m_szStreamFileA[m_musicID - m_iStream_CD_1 + eStream_CD_1]);
                    strcat((char*)m_szStreamName, ".wav");
                }
            }
            else
            {
                // No mash‑up pack – use standard Minecraft music files
#ifdef __PS3__
                    // PS3 disc patch handling
                if (app.GetBootedFromDiscPatch() && (m_musicID < m_iStream_CD_1))
                {
                    // Rebuild path for patch data
                    strcpy((char*)m_szStreamName, m_szMusicPath);
                    strcat((char*)m_szStreamName, "music/");
                    strcat((char*)m_szStreamName, m_szStreamFileA[m_musicID]);
                    strcat((char*)m_szStreamName, ".wav");

                    // Check if file exists in patch area; if not, fallback to original path
                    sprintf(m_szStreamName, "%s/%s", app.GetBDUsrDirPath(m_szStreamName), m_szMusicPath);
                    strcat((char*)m_szStreamName, "music/");
                    strcat((char*)m_szStreamName, m_szStreamFileA[m_musicID]);
                    strcat((char*)m_szStreamName, ".wav");

                    SetIsPlayingStreamingGameMusic(true);
                    SetIsPlayingStreamingCDMusic(false);
                    m_MusicType = eMusicType_Game;
                    m_StreamingAudioInfo.bIs3D = false;
                }
                else if (m_musicID < m_iStream_CD_1)
                {
                    // Standard game music (non‑CD)
                    SetIsPlayingStreamingGameMusic(true);
                    SetIsPlayingStreamingCDMusic(false);
                    m_MusicType = eMusicType_Game;
                    m_StreamingAudioInfo.bIs3D = false;
                    strcat((char*)m_szStreamName, "music/");
                    strcat((char*)m_szStreamName, m_szStreamFileA[m_musicID]);
                    strcat((char*)m_szStreamName, ".wav");
            }
                else
                {
                    // CD music
                    SetIsPlayingStreamingGameMusic(false);
                    SetIsPlayingStreamingCDMusic(true);
                    m_MusicType = eMusicType_CD;
                    m_StreamingAudioInfo.bIs3D = true;
                    strcat((char*)m_szStreamName, "cds/");
                    strcat((char*)m_szStreamName, m_szStreamFileA[m_musicID]);
                    strcat((char*)m_szStreamName, ".wav");
                }
#else
                    // Non‑PS3 platforms
                if (m_musicID < m_iStream_CD_1)
                {
                    // Game music
                    SetIsPlayingStreamingGameMusic(true);
                    SetIsPlayingStreamingCDMusic(false);
                    m_MusicType = eMusicType_Game;
                    m_StreamingAudioInfo.bIs3D = false;
                    strcat((char*)m_szStreamName, "music/");
                }
                else
                {
                    // CD music
                    SetIsPlayingStreamingGameMusic(false);
                    SetIsPlayingStreamingCDMusic(true);
                    m_MusicType = eMusicType_CD;
                    m_StreamingAudioInfo.bIs3D = true;
                    strcat((char*)m_szStreamName, "cds/");
                }
                // Append the base filename (from global array) and extension
                strcat((char*)m_szStreamName, m_szStreamFileA[m_musicID]);
                strcat((char*)m_szStreamName, ".wav");
#endif
        }

            // Verify that the file exists; if not, try alternative extensions (.ogg, .mp3)
            FILE* pFile = nullptr;
            if (fopen_s(&pFile, reinterpret_cast<char*>(m_szStreamName), "rb") == 0 && pFile)
            {
                fclose(pFile);
            }
            else
            {
                // File not found – try changing the extension
                const char* extensions[] = { ".ogg", ".mp3", ".wav" };
                size_t extCount = sizeof(extensions) / sizeof(extensions[0]);
                bool found = false;

                // Find the position of the current extension (assumed to be ".wav")
                char* dotPos = strrchr(reinterpret_cast<char*>(m_szStreamName), '.');
                if (dotPos != nullptr && (dotPos - reinterpret_cast<char*>(m_szStreamName)) < 250)
                {
                    for (size_t i = 0; i < extCount; ++i)
                    {
                        strcpy_s(dotPos, 5, extensions[i]);  // Replace extension
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
                    // Restore original extension for error message
                    if (dotPos != nullptr)
                        strcpy_s(dotPos, 5, ".wav");
                    app.DebugPrintf("WARNING: No audio file found for music ID %d (tried .ogg, .mp3, .wav)\n", m_musicID);
                    return;  // Abort – stay in Idle
                }
            }
    } // end file path construction

        app.DebugPrintf("Starting streaming - %s\n", m_szStreamName);

        // Launch a thread to open the audio file (prevents blocking the main thread)
        m_openStreamThread = new C4JThread(OpenStreamThreadProc, this, "OpenStreamThreadProc");
        m_openStreamThread->Run();
        m_StreamState = eMusicStreamState_Opening;
        break;
    }

    //---------------------------------------------------------------------
    // OPENING – waiting for the background thread to finish opening the file
    //---------------------------------------------------------------------
    case eMusicStreamState_Opening:
    {
        if (!m_openStreamThread->isRunning())
        {
            // Thread finished
            delete m_openStreamThread;
            m_openStreamThread = nullptr;

            app.DebugPrintf("OpenStreamThreadProc finished. m_musicStreamActive=%d\n", m_musicStreamActive);

            // If the stream failed to open, try a fallback (e.g., if we used a numbered variant)
            if (!m_musicStreamActive)
            {
                const char* currentExt = strrchr(reinterpret_cast<char*>(m_szStreamName), '.');
                if (currentExt && _stricmp(currentExt, ".wav") == 0)
                {
                    // Attempt to rebuild the path using the base filename (without number)
                    const bool isCD = (m_musicID >= m_iStream_CD_1);
                    const char* folder = isCD ? "cds/" : "music/";
                    int n = sprintf_s(reinterpret_cast<char*>(m_szStreamName), 512, "%s%s%s.wav",
                        m_szMusicPath, folder, m_szStreamFileA[m_musicID]);
                    if (n > 0)
                    {
                        FILE* pFile = nullptr;
                        if (fopen_s(&pFile, reinterpret_cast<char*>(m_szStreamName), "rb") == 0 && pFile)
                        {
                            fclose(pFile);
                            // Retry opening with the new path
                            m_openStreamThread = new C4JThread(OpenStreamThreadProc, this, "OpenStreamThreadProc");
                            m_openStreamThread->Run();
                            break;  // stay in Opening
                        }
                    }
                }
                // No fallback worked – go back to Idle
                m_StreamState = eMusicStreamState_Idle;
                break;
            }

            // Stream opened successfully; configure spatialization, pitch, volume
            if (m_StreamingAudioInfo.bIs3D)
            {
                ma_sound_set_spatialization_enabled(&m_musicStream, MA_TRUE);
                ma_sound_set_position(&m_musicStream,
                    m_StreamingAudioInfo.x,
                    m_StreamingAudioInfo.y,
                    m_StreamingAudioInfo.z);
            }
            else
            {
                ma_sound_set_spatialization_enabled(&m_musicStream, MA_FALSE);
            }

            ma_sound_set_pitch(&m_musicStream, m_StreamingAudioInfo.pitch);

            float finalVolume = m_StreamingAudioInfo.volume * masterVolume;
            ma_sound_set_volume(&m_musicStream, finalVolume);

            // Start playback
            ma_result startResult = ma_sound_start(&m_musicStream);
            app.DebugPrintf("ma_sound_start result: %d\n", startResult);

            m_StreamState = eMusicStreamState_Playing;
        }
        break;
    }

    //---------------------------------------------------------------------
    // OPENINGCANCEL – user requested stop while opening
    //---------------------------------------------------------------------
    case eMusicStreamState_OpeningCancel:
    {
        if (!m_openStreamThread->isRunning())
        {
            delete m_openStreamThread;
            m_openStreamThread = nullptr;
            m_StreamState = eMusicStreamState_Stop;
        }
        break;
    }

    //---------------------------------------------------------------------
    // STOP – actively stop the current stream
    //---------------------------------------------------------------------
    case eMusicStreamState_Stop:
    {
        if (m_musicStreamActive)
        {
            ma_sound_stop(&m_musicStream);
            ma_sound_uninit(&m_musicStream);
            m_musicStreamActive = false;
        }

        // Clear flags indicating what type of music was playing
        SetIsPlayingStreamingCDMusic(false);
        SetIsPlayingStreamingGameMusic(false);

        m_StreamState = eMusicStreamState_Idle;
        break;
    }

    //---------------------------------------------------------------------
    // PLAYING – the stream is actively playing
    //---------------------------------------------------------------------
    case eMusicStreamState_Playing:
    {
        // Optional periodic debug logging (once per second at 60 fps)
        static int frameCount = 0;
        if (frameCount++ % 60 == 0)
        {
            if (m_musicStreamActive)
            {
                bool isPlaying = ma_sound_is_playing(&m_musicStream);
                float vol = ma_sound_get_volume(&m_musicStream);
                bool isAtEnd = ma_sound_at_end(&m_musicStream);
                // (debug info could be printed here if needed)
            }
        }

        // Separate handling for game music (background) and CD music (jukebox)
        if (GetIsPlayingStreamingGameMusic())
        {
            // Background music: check if the musical context has changed
            MusicTrackManager::Domain currentDomain = determineCurrentMusicDomain();

            // If the domain changed, we need to stop the current track
            // so that a new one (appropriate for the new domain) will start.
            if (currentDomain != m_currentMusicDomain)
            {
                m_StreamState = eMusicStreamState_Stop;
                // Store the new domain for when we restart
                m_currentMusicDomain = currentDomain;
                m_musicID = getTrackForDomain(currentDomain);
                break;
            }

            // Update volume if master volume changed
            if (m_musicStreamActive)
            {
                float finalVolume = m_StreamingAudioInfo.volume * masterVolume;
                ma_sound_set_volume(&m_musicStream, finalVolume);
            }
        }
        else
        {
            // Music disc playing (jukebox)
            if (m_StreamingAudioInfo.bIs3D && m_validListenerCount > 1)
            {
                // For split‑screen, we need to position the sound relative to the closest listener.
                // (The engine's listener is set to the origin; we simulate distance by moving the sound.)
                int iClosestListener = 0;
                float fClosestDist = 1e6f;

                for (size_t i = 0; i < MAX_LOCAL_PLAYERS; ++i)
                {
                    if (m_ListenerA[i].bValid)
                    {
                        float dx = m_StreamingAudioInfo.x - m_ListenerA[i].vPosition.x;
                        float dy = m_StreamingAudioInfo.y - m_ListenerA[i].vPosition.y;
                        float dz = m_StreamingAudioInfo.z - m_ListenerA[i].vPosition.z;
                        float dist = sqrtf(dx * dx + dy * dy + dz * dz);

                        if (dist < fClosestDist)
                        {
                            fClosestDist = dist;
                            iClosestListener = i;
                        }
                    }
                }

                // Compute sound position relative to that listener
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
    }

    //---------------------------------------------------------------------
    // COMPLETED – the current track reached its end naturally
    //---------------------------------------------------------------------
    case eMusicStreamState_Completed:
    {
        // Set a random delay (0–3 minutes) before playing the next track
        m_iMusicDelay = random->nextInt(20 * 60 * 3);

        // Determine the current musical context
        m_currentMusicDomain = determineCurrentMusicDomain();

        // Select a new track ID appropriate for that domain
        m_musicID = getTrackForDomain(m_currentMusicDomain);

        // Update the flags that track which domain is active (for compatibility)
        // These are used elsewhere; we keep them in sync.
        SetIsPlayingMenuMusic(m_currentMusicDomain == MusicTrackManager::Domain::Menu);
        SetIsPlayingEndMusic(m_currentMusicDomain == MusicTrackManager::Domain::End);
        SetIsPlayingNetherMusic(m_currentMusicDomain == MusicTrackManager::Domain::Nether);

        m_StreamState = eMusicStreamState_Idle;
        break;
    }

    //---------------------------------------------------------------------
    // States that require no action (STOPPING, PLAY – unused)
    //---------------------------------------------------------------------
    case eMusicStreamState_Stopping:
    case eMusicStreamState_Play:
        break;
}

    //-------------------------------------------------------------------------
    // End-of‑stream detection (common to all states)
    // If the stream is active but has finished playing, transition to Completed.
    //-------------------------------------------------------------------------
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