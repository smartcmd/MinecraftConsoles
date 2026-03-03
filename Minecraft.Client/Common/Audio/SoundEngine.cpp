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

#ifdef _WINDOWS64
#include "..\..\Minecraft.Client\Windows64\Windows64_App.h"
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "BinkDecoder.h"
#include <vector>
#include <memory>
#include <mutex>

bool FileExists(const char* filename)
{
    DWORD attrib = GetFileAttributesA(filename);
	return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}
#endif

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
char *SoundEngine::ConvertSoundPathToName(const wstring& name, bool bConvertSpaces) { return NULL; }
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

#ifndef _WINDOWS64
F32 AILCALLBACK custom_falloff_function (HSAMPLE   S, 
										 F32       distance,
										 F32       rolloff_factor,
										 F32       min_dist,
										 F32       max_dist);
#endif

char *SoundEngine::m_szStreamFileA[eStream_Max]=
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
	// add the new music tracks
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

/////////////////////////////////////////////
//
//	ErrorCallback
//
/////////////////////////////////////////////
#ifndef _WINDOWS64
void AILCALL ErrorCallback(S64 i_Id, char const* i_Details)
{
	char *pchLastError=AIL_last_error();

	if(pchLastError[0]!=0)
	{
		app.DebugPrintf("\rErrorCallback Error Category: %s\n", pchLastError);
	}

	if (i_Details)
	{
		app.DebugPrintf("ErrorCallback - Details: %s\n", i_Details);
	}
}
#else
std::vector<MiniAudioSound*> m_activeSounds;
#endif

#ifdef __PSVITA__
// AP - this is the callback when the driver is about to mix. At this point the mutex is locked by Miles so we can now call all Miles functions without
// the possibility of incurring a stall.
static bool SoundEngine_Change = false;				// has tick been called?
static CRITICAL_SECTION SoundEngine_MixerMutex;

void AILCALL MilesMixerCB(HDIGDRIVER dig)
{
	// has the tick function been called since the last callback
	if( SoundEngine_Change )
	{
		SoundEngine_Change = false;

		EnterCriticalSection(&SoundEngine_MixerMutex);

		Minecraft *pMinecraft = Minecraft::GetInstance();
		pMinecraft->soundEngine->updateMiles();
		pMinecraft->soundEngine->playMusicUpdate();

		LeaveCriticalSection(&SoundEngine_MixerMutex);
	}
}
#endif

/////////////////////////////////////////////
//
//	init
//
/////////////////////////////////////////////
void SoundEngine::init(Options* pOptions)
{
    app.DebugPrintf("---SoundEngine::init\n");

#ifdef _WINDOWS64
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

#else
#ifdef __ORBIS__
	C4JThread::PushAffinityAllCores();
#endif 
#if defined _DURANGO || defined __ORBIS__ || defined __PS3__ || defined __PSVITA__
	Register_RIB(BinkADec);
#endif

	char *redistpath;

#if (defined __PSVITA__)// || defined _DURANGO || defined __ORBIS__ )
	redistpath=AIL_set_redist_directory(m_szRedistName);
#endif

	app.DebugPrintf("---SoundEngine::init - AIL_startup\n");
	S32 ret = AIL_startup();

	int iNumberOfChannels=initAudioHardware(8);

	// Create a driver to render our audio - 44khz, 16 bit,
#ifdef __PS3__	
	//	On the Sony PS3, the driver is always opened in 48 kHz, 32-bit floating point. The only meaningful configurations are MSS_MC_STEREO, MSS_MC_51_DISCRETE, and MSS_MC_71_DISCRETE. 
	m_hDriver = AIL_open_digital_driver( 48000, 16, iNumberOfChannels, AIL_OPEN_DIGITAL_USE_SPU0 );
#elif defined __PSVITA__

	// maximum of 16 samples
	AIL_set_preference(DIG_MIXER_CHANNELS, 16);

	m_hDriver = AIL_open_digital_driver( 48000, 16, MSS_MC_STEREO, 0 );

	// AP - For some reason the submit thread defaults to a priority of zero (invalid). Make sure it has the highest priority to avoid audio breakup. 
	SceUID		threadID;
	AIL_platform_property( m_hDriver, PSP2_SUBMIT_THREAD, &threadID, 0, 0);
	S32 g_DefaultCPU = sceKernelGetThreadCpuAffinityMask(threadID);
	S32 Old = sceKernelChangeThreadPriority(threadID, 64);

	// AP - register a callback when the mixer starts
	AILMIXERCB temp = AIL_register_mix_callback(m_hDriver, MilesMixerCB);

	InitializeCriticalSection(&SoundEngine_MixerMutex);

#elif defined(__ORBIS__)
	m_hDriver = AIL_open_digital_driver( 48000, 16, 2, 0 );
	app.DebugPrintf("---SoundEngine::init - AIL_open_digital_driver\n");

#endif
	if (m_hDriver == 0)
	{
		app.DebugPrintf("Couldn't open digital sound driver. (%s)\n", AIL_last_error());
		AIL_shutdown();
#ifdef __ORBIS__
		C4JThread::PopAffinity();
#endif 
		return;
	}
	app.DebugPrintf("---SoundEngine::init - driver opened\n");

#ifdef __PSVITA__

	// set high falloff power for maximum spatial effect in software mode
	AIL_set_speaker_configuration( m_hDriver, 0, 0, 4.0F );

#endif

	AIL_set_event_error_callback(ErrorCallback);

	AIL_set_3D_rolloff_factor(m_hDriver,1.0);

	// Create an event system tied to that driver - let Miles choose memory defaults.
	//if (AIL_startup_event_system(m_hDriver, 0, 0, 0) == 0)
	// 4J-PB - Durango complains that the default memory (64k)isn't enough
	// Error: MilesEvent: Out of event system memory (pool passed to event system startup exhausted).
	// AP - increased command buffer from the default 5K to 20K for Vita

	if (AIL_startup_event_system(m_hDriver, 1024*20, 0, 1024*128) == 0)
	{
		app.DebugPrintf("Couldn't init event system (%s).\n", AIL_last_error());
		AIL_close_digital_driver(m_hDriver);
		AIL_shutdown();
#ifdef __ORBIS__
		C4JThread::PopAffinity();
#endif 
		app.DebugPrintf("---SoundEngine::init - AIL_startup_event_system failed\n");
		return;
	}
	char szBankName[255];
#if defined __PS3__ 
	if(app.GetBootedFromDiscPatch())
	{
		char szTempSoundFilename[255];
		sprintf(szTempSoundFilename,"%s%s",m_szSoundPath, "Minecraft.msscmp" );

		app.DebugPrintf("SoundEngine::playMusicUpdate - (booted from disc patch) looking for %s\n",szTempSoundFilename);
		sprintf(szBankName,"%s/%s",app.GetBDUsrDirPath(szTempSoundFilename), m_szSoundPath );
		app.DebugPrintf("SoundEngine::playMusicUpdate - (booted from disc patch) music path - %s\n",szBankName);
	}
	else
	{
		sprintf(szBankName,"%s/%s",getUsrDirPath(), m_szSoundPath );
	}
	
#elif defined __PSVITA__
	sprintf(szBankName,"%s/%s",getUsrDirPath(), m_szSoundPath );
#elif defined __ORBIS__
	sprintf(szBankName,"%s/%s",getUsrDirPath(), m_szSoundPath );
#else
	strcpy((char *)szBankName,m_szSoundPath);
#endif

	strcat((char *)szBankName,"Minecraft.msscmp");

	m_hBank=AIL_add_soundbank(szBankName, 0);

	if(m_hBank == NULL)
	{
		char *Error=AIL_last_error();
		app.DebugPrintf("Couldn't open soundbank: %s (%s)\n", szBankName, Error);
		AIL_close_digital_driver(m_hDriver);
		AIL_shutdown();
#ifdef __ORBIS__
		C4JThread::PopAffinity();
#endif 
		return;
	}

	//#ifdef _DEBUG
	HMSSENUM token = MSS_FIRST;
	char const* Events[1] = {0};
	S32 EventCount = 0;
	while (AIL_enumerate_events(m_hBank, &token, 0, &Events[0]))
	{
		app.DebugPrintf(4,"%d - %s\n", EventCount, Events[0]);

		EventCount++;
	}
	//#endif

	U64 u64Result;
	u64Result=AIL_enqueue_event_by_name("Minecraft/CacheSounds");

	m_MasterMusicVolume=1.0f;
	m_MasterEffectsVolume=1.0f;

	//AIL_set_variable_float(0,"UserEffectVol",1);

	m_bSystemMusicPlaying = false;

	m_openStreamThread = NULL;

#ifdef __ORBIS__
	C4JThread::PopAffinity();
#endif

#ifdef __PSVITA__
	// AP - By default the mixer won't start up and nothing will process. Kick off a blank sample to force the mixer to start up. 
	HSAMPLE Sample = AIL_allocate_sample_handle(m_hDriver);
	AIL_init_sample(Sample, DIG_F_STEREO_16);
	static U64 silence = 0;
	AIL_set_sample_address(Sample, &silence, sizeof(U64));
	AIL_start_sample(Sample);

	// wait for 1 mix...
	AIL_release_sample_handle(Sample);
#endif

#endif // !_WINDOWS64
}

#ifdef __ORBIS__
// void SoundEngine::SetHandle(int32_t hAudio)
// {
// 	//m_hAudio=hAudio;
// }
#endif

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

// AP - moved to a separate function so it can be called from the mixer callback on Vita
#ifndef _WINDOWS64
void SoundEngine::updateMiles()
{
#ifdef __PSVITA__ 
	//CD - We must check for Background Music [BGM] at any point
	//If it's playing disable our audio, otherwise enable
	int NoBGMPlaying = sceAudioOutGetAdopt(SCE_AUDIO_OUT_PORT_TYPE_BGM);
	updateSystemMusicPlaying( !NoBGMPlaying );
#elif defined __ORBIS__
	// is the system playing background music?
	SceAudioOutPortState outPortState;
	sceAudioOutGetPortState(m_hBGMAudio,&outPortState);
	updateSystemMusicPlaying( outPortState.output==SCE_AUDIO_OUT_STATE_OUTPUT_UNKNOWN );
#endif

	if( m_validListenerCount == 1 )
	{
		for( int i = 0; i < MAX_LOCAL_PLAYERS; i++ )
		{
			// set the listener as the first player we find
			if( m_ListenerA[i].bValid )
			{
				AIL_set_listener_3D_position(m_hDriver,m_ListenerA[i].vPosition.x,m_ListenerA[i].vPosition.y,-m_ListenerA[i].vPosition.z);  // Flipped sign of z as Miles is expecting left handed coord system
				AIL_set_listener_3D_orientation(m_hDriver,-m_ListenerA[i].vOrientFront.x,m_ListenerA[i].vOrientFront.y,m_ListenerA[i].vOrientFront.z,0,1,0);   // Flipped sign of z as Miles is expecting left handed coord system
				break;
			}
		}
	}
	else
	{
		// 4J-PB - special case for splitscreen
		// the shortest distance between any listener and a sound will be used to play a sound a set distance away down the z axis.
		// The listener position will be set to 0,0,0, and the orientation will be facing down the z axis

		AIL_set_listener_3D_position(m_hDriver,0,0,0);
		AIL_set_listener_3D_orientation(m_hDriver,0,0,1,0,1,0);
	}

	AIL_begin_event_queue_processing();

	// Iterate over the sounds
	S32 StartedCount = 0, CompletedCount = 0, TotalCount = 0;
	HMSSENUM token = MSS_FIRST;
	MILESEVENTSOUNDINFO SoundInfo;
	int Playing = 0;
	while (AIL_enumerate_sound_instances(0, &token, 0, 0, 0, &SoundInfo))
	{
		AUDIO_INFO* game_data= (AUDIO_INFO*)( SoundInfo.UserBuffer );

		if( SoundInfo.Status == MILESEVENT_SOUND_STATUS_PLAYING )
		{
			Playing += 1;
		}

		if ( SoundInfo.Status != MILESEVENT_SOUND_STATUS_COMPLETE )
		{
			// apply the master volume
			// watch for the 'special' volume levels
			bool isThunder = false;
			if( game_data->volume == 10000.0f )
			{
				isThunder = true;
			}
			if(game_data->volume>1) 
			{
				game_data->volume=1;
			}
			AIL_set_sample_volume_levels( SoundInfo.Sample, game_data->volume*m_MasterEffectsVolume, game_data->volume*m_MasterEffectsVolume);

			float distanceScaler = 16.0f;
			switch(SoundInfo.Status)
			{
			case MILESEVENT_SOUND_STATUS_PENDING:
				// 4J-PB - causes the falloff to be calculated on the PPU instead of the SPU, and seems to resolve our distorted sound issue
				AIL_register_falloff_function_callback(SoundInfo.Sample,&custom_falloff_function);

				if(game_data->bIs3D)
				{			
					AIL_set_sample_is_3D( SoundInfo.Sample, 1 );

					int iSound = game_data->iSound - eSFX_MAX;
					switch(iSound)
					{
						// Is this the Dragon?
						case eSoundType_MOB_ENDERDRAGON_GROWL:
						case eSoundType_MOB_ENDERDRAGON_MOVE:
						case eSoundType_MOB_ENDERDRAGON_END:
						case eSoundType_MOB_ENDERDRAGON_HIT:
							distanceScaler=100.0f;
							break;
						case eSoundType_FIREWORKS_BLAST:
						case eSoundType_FIREWORKS_BLAST_FAR:
						case eSoundType_FIREWORKS_LARGE_BLAST:
						case eSoundType_FIREWORKS_LARGE_BLAST_FAR:
							distanceScaler=100.0f;
							break;
						case eSoundType_MOB_GHAST_MOAN:
						case eSoundType_MOB_GHAST_SCREAM:
						case eSoundType_MOB_GHAST_DEATH:
						case eSoundType_MOB_GHAST_CHARGE:
						case eSoundType_MOB_GHAST_FIREBALL:
							distanceScaler=30.0f;
							break;
					}

					// Set a special distance scaler for thunder, which we respond to by having no attenutation
					if( isThunder )
					{
						distanceScaler = 10000.0f;
					}
				}
				else
				{
					AIL_set_sample_is_3D( SoundInfo.Sample, 0 );
				}

				AIL_set_sample_3D_distances(SoundInfo.Sample,distanceScaler,1,0);
				// set the pitch
				if(!game_data->bUseSoundsPitchVal)
				{
					AIL_set_sample_playback_rate_factor(SoundInfo.Sample,game_data->pitch);
				}

				if(game_data->bIs3D)
				{			
					if(m_validListenerCount>1)
					{
						float fClosest=10000.0f;
						int iClosestListener=0;
						float fClosestX=0.0f,fClosestY=0.0f,fClosestZ=0.0f,fDist;
						// need to calculate the distance from the sound to the nearest listener - use Manhattan Distance as the decision
						for( int i = 0; i < MAX_LOCAL_PLAYERS; i++ )
						{
							if( m_ListenerA[i].bValid )
							{
								float x,y,z;

								x=fabs(m_ListenerA[i].vPosition.x-game_data->x);
								y=fabs(m_ListenerA[i].vPosition.y-game_data->y);
								z=fabs(m_ListenerA[i].vPosition.z-game_data->z);
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

						// our distances in the world aren't very big, so floats rather than casts to doubles should be fine
						fDist=sqrtf((fClosestX*fClosestX)+(fClosestY*fClosestY)+(fClosestZ*fClosestZ));
						AIL_set_sample_3D_position( SoundInfo.Sample, 0, 0, fDist );

						//app.DebugPrintf("Playing sound %d %f from nearest listener [%d]\n",SoundInfo.EventID,fDist,iClosestListener);
					}
					else
					{
						AIL_set_sample_3D_position( SoundInfo.Sample, game_data->x, game_data->y, -game_data->z );  // Flipped sign of z as Miles is expecting left handed coord system
					}
				}
				break;

			default:
				if(game_data->bIs3D)
				{	
					if(m_validListenerCount>1)
					{
						float fClosest=10000.0f;
						int iClosestListener=0;
						float fClosestX=0.0f,fClosestY=0.0f,fClosestZ=0.0f,fDist;
						// need to calculate the distance from the sound to the nearest listener - use Manhattan Distance as the decision
						for( int i = 0; i < MAX_LOCAL_PLAYERS; i++ )
						{
							if( m_ListenerA[i].bValid )
							{
								float x,y,z;

								x=fabs(m_ListenerA[i].vPosition.x-game_data->x);
								y=fabs(m_ListenerA[i].vPosition.y-game_data->y);
								z=fabs(m_ListenerA[i].vPosition.z-game_data->z);
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
						// our distances in the world aren't very big, so floats rather than casts to doubles should be fine
						fDist=sqrtf((fClosestX*fClosestX)+(fClosestY*fClosestY)+(fClosestZ*fClosestZ));
						AIL_set_sample_3D_position( SoundInfo.Sample, 0, 0, fDist );

						//app.DebugPrintf("Playing sound %d %f from nearest listener [%d]\n",SoundInfo.EventID,fDist,iClosestListener);
					}
					else
					{
						AIL_set_sample_3D_position( SoundInfo.Sample, game_data->x, game_data->y, -game_data->z );  // Flipped sign of z as Miles is expecting left handed coord system
					}
				}				
				break;
			}			
		}
	}
	AIL_complete_event_queue_processing();
}
#else
void SoundEngine::updateMiniAudio()
{
    if (m_validListenerCount == 1)
    {
        for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
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

        float finalVolume = s->info.volume;
        if (finalVolume > 1.0f)
            finalVolume = 1.0f;

        ma_sound_set_volume(&s->sound, finalVolume);

        if (!s->info.bUseSoundsPitchVal)
        {
            ma_sound_set_pitch(&s->sound, s->info.pitch);
        }

        if (s->info.bIs3D)
        {
            if (m_validListenerCount > 1)
            {
                float closest = 10000.0f;

                for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
                {
                    if (m_ListenerA[i].bValid)
                    {
                        float dx = fabs(m_ListenerA[i].vPosition.x - s->info.x);
                        float dy = fabs(m_ListenerA[i].vPosition.y - s->info.y);
                        float dz = fabs(m_ListenerA[i].vPosition.z - s->info.z);

                        float dist = dx + dy + dz;
                        if (dist < closest)
                            closest = dist;
                    }
                }

                float realDist = sqrtf(closest * closest);
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
#endif

//#define DISTORTION_TEST
#ifdef DISTORTION_TEST
static float fVal=0.0f;
#endif
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
#ifdef __DISABLE_MILES__
	return;
#endif

#ifdef __PSVITA__
	EnterCriticalSection(&SoundEngine_MixerMutex);
#endif

	// update the listener positions
	int listenerCount = 0;
#ifdef DISTORTION_TEST
	float fX,fY,fZ;
#endif
	if( players )
	{
		bool bListenerPostionSet=false;
		for( int i = 0; i < MAX_LOCAL_PLAYERS; i++ )
		{
			if( players[i] != NULL )
			{
				m_ListenerA[i].bValid=true;
				F32 x,y,z;
				x=players[i]->xo + (players[i]->x - players[i]->xo) * a;
				y=players[i]->yo + (players[i]->y - players[i]->yo) * a;
				z=players[i]->zo + (players[i]->z - players[i]->zo) * a;

				float yRot = players[i]->yRotO + (players[i]->yRot - players[i]->yRotO) * a;
				float yCos = (float)cos(-yRot * Mth::RAD_TO_GRAD - PI);
				float ySin = (float)sin(-yRot * Mth::RAD_TO_GRAD - PI);

				// store the listener positions for splitscreen
				m_ListenerA[i].vPosition.x = x;
				m_ListenerA[i].vPosition.y = y;
				m_ListenerA[i].vPosition.z = z;  

				m_ListenerA[i].vOrientFront.x = ySin;
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

	LeaveCriticalSection(&SoundEngine_MixerMutex);
#else
#ifndef _WINDOWS64
	updateMiles();
#else
	updateMiniAudio();
#endif
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
#ifndef _WINDOWS64
	m_hStream=0;
#else
	memset(&m_engine, 0, sizeof(ma_engine));
    memset(&m_engineConfig, 0, sizeof(ma_engine_config));
    binkaDecoder = nullptr;
    m_musicStreamActive = false;
#endif
	m_StreamState=eMusicStreamState_Idle;
	m_iMusicDelay=0;
	m_validListenerCount=0; 

	m_bHeardTrackA=NULL;

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

#ifdef DISTORTION_TEST
    wstring name = wchSoundNames[eSoundType_MOB_ENDERDRAGON_GROWL];
#else
    wstring name = wchSoundNames[iSound];
#endif

    char* SoundName = (char*)ConvertSoundPathToName(name);
    strcat((char*)szSoundName, SoundName);

    app.DebugPrintf(6,
        "PlaySound - %d - %s - %s (%f %f %f, vol %f, pitch %f)\n",
        iSound, SoundName, szSoundName, x, y, z, volume, pitch);

#ifdef _WINDOWS64

    char basePath[256];
    sprintf_s(basePath, "Windows64Media/Sound/%s", (char*)szSoundName);

    char finalPath[256];
    sprintf_s(finalPath, "%s.wav", basePath);

    if (!FileExists(finalPath))
    {
        int count = 0;

        for (int i = 1; i < 32; i++)
        {
            char numberedFolder[256];
            sprintf_s(numberedFolder, "%s%d", basePath, i);

            DWORD attr = GetFileAttributesA(numberedFolder);

            if (attr != INVALID_FILE_ATTRIBUTES &&
                (attr & FILE_ATTRIBUTE_DIRECTORY))
            {
                count++;
            }
            else
            {
                break;
            }
        }

        char chosenFolder[256];

        if (count == 0)
        {
            sprintf_s(chosenFolder, "%s", basePath);
        }
        else
        {
            int chosen = (rand() % count) + 1;
            sprintf_s(chosenFolder, "%s%d", basePath, chosen);
        }

        char searchPattern[256];
        sprintf_s(searchPattern, "%s\\*.wav", chosenFolder);

        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA(searchPattern, &findData);

        if (hFind == INVALID_HANDLE_VALUE)
        {
            sprintf_s(searchPattern, "%s\\*.mp3", chosenFolder);
            hFind = FindFirstFileA(searchPattern, &findData);

            if (hFind == INVALID_HANDLE_VALUE)
            {
                printf("No .wav or .mp3 files found in %s\n", chosenFolder);
                return;
            }
        }

        sprintf_s(finalPath, "%s\\%s", chosenFolder, findData.cFileName);
        FindClose(hFind);
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
            NULL,
            NULL,
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

#else

    S32 token = AIL_enqueue_event_start();
    AIL_enqueue_event_buffer(&token, &AudioInfo, sizeof(AUDIO_INFO), 0);
    AIL_enqueue_event_end_named(token, (char*)szSoundName);

#endif
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

#ifdef _WINDOWS64

    char basePath[256];
    sprintf_s(basePath, "Windows64Media/Sound/Minecraft/UI/%s", ConvertSoundPathToName(name));

    char finalPath[256];
    sprintf_s(finalPath, "%s.wav", basePath);

    if (!FileExists(finalPath))
    {
        sprintf_s(finalPath, "%s.wav", basePath);
		if (!FileExists(finalPath))
		{
			sprintf_s(finalPath, "%s.mp3", basePath);
			if (!FileExists(finalPath))
			{
				app.DebugPrintf("No sound file found for UI sound: %s\n", basePath);
				return;
			}
		}
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
            NULL,
            NULL,
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

    ma_sound_set_volume(&s->sound, finalVolume);
    ma_sound_set_pitch(&s->sound, pitch);

    ma_sound_start(&s->sound);

    m_activeSounds.push_back(s);

#else

    AUDIO_INFO AudioInfo;
    memset(&AudioInfo, 0, sizeof(AUDIO_INFO));

    AudioInfo.volume = volume;
    AudioInfo.pitch = pitch;
    AudioInfo.bUseSoundsPitchVal = true;

    if (iSound >= eSFX_MAX)
        AudioInfo.iSound = iSound + eSFX_MAX;
    else
        AudioInfo.iSound = iSound;

#ifdef _DEBUG
    strncpy(AudioInfo.chName, (char*)szSoundName, 64);
#endif

    S32 token = AIL_enqueue_event_start();
    AIL_enqueue_event_buffer(&token, &AudioInfo, sizeof(AUDIO_INFO), 0);
    AIL_enqueue_event_end_named(token, (char*)szSoundName);

#endif
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
			if(pMinecraft->localplayers[i]!=NULL)
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
	for(int i=iStart;i<=iEnd;i++)
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

		for(int i=iStart;i<=iEnd;i++)
		{
			m_bHeardTrackA[i]=false;
		}
	}

	// trying to get a track we haven't heard, but not too hard		
	for(int i=0;i<=((iEnd-iStart)/2);i++)
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
	if(pMinecraft==NULL)
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
	for(int i=0;i<12;i++)
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
#ifdef __DISABLE_MILES__
    return 0;
#endif

    SoundEngine* soundEngine = (SoundEngine*)lpParameter;

#ifdef _WINDOWS64
	const char* ext = strrchr(soundEngine->m_szStreamName, '.');
    bool isBinkaFile = (ext && _stricmp(ext, ".binka") == 0);
    
    if (isBinkaFile)
    {
        if (!soundEngine->binkaDecoder)
            soundEngine->binkaDecoder = new BinkDecoder();

        if (!soundEngine->binkaDecoder->open(soundEngine->m_szStreamName))
        {
            delete soundEngine->binkaDecoder;
            soundEngine->binkaDecoder = nullptr;
            return 0;
        }
    }

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
            NULL,
            NULL,
            &soundEngine->m_musicStream);

    if (result != MA_SUCCESS)
    {
        if (isBinkaFile && soundEngine->binkaDecoder)
        {
            delete soundEngine->binkaDecoder;
            soundEngine->binkaDecoder = nullptr;
        }

        return 0;
    }

    ma_sound_set_spatialization_enabled(&soundEngine->m_musicStream, MA_FALSE);
    ma_sound_set_looping(&soundEngine->m_musicStream, MA_FALSE);

    soundEngine->m_musicStreamActive = true;
    
    return 0;

#else
    soundEngine->m_hStream = AIL_open_stream(soundEngine->m_hDriver, soundEngine->m_szStreamName, 0);

    if (soundEngine->m_hStream == 0)
    {
        app.DebugPrintf(
            "SoundEngine::OpenStreamThreadProc - Could not open - %s\n",
            soundEngine->m_szStreamName);
    }
#endif

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
	//return;
	static bool firstCall = true;
	static float fMusicVol = 0.0f;
	if( firstCall )
	{
		fMusicVol = getMasterMusicVolume();
		firstCall = false;
	}

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
					wstring wstrFile=L"TPACK:\\Data\\" + wstrSoundName +L".binka";
					std::wstring mountedPath = StorageManager.GetMountedPath(wstrFile);
					wcstombs(m_szStreamName,mountedPath.c_str(),255);
#else
					wstring &wstrSoundName=dlcAudioFile->GetSoundName(m_musicID);
					char szName[255];
					wcstombs(szName,wstrSoundName.c_str(),255);

#if defined __PS3__ || defined __ORBIS__ || defined __PSVITA__
					string strFile="TPACK:/Data/" + string(szName) + ".binka";
#else
					string strFile="TPACK:\\Data\\" + string(szName) + ".binka";
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
					strcat((char *)m_szStreamName,".binka");
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
					strcat((char *)m_szStreamName,".binka");

					// check if this is in the patch data
					sprintf(m_szStreamName,"%s/%s",app.GetBDUsrDirPath(m_szStreamName), m_szMusicPath );		
					strcat((char *)m_szStreamName,"music/");
					strcat((char *)m_szStreamName,m_szStreamFileA[m_musicID]);
					strcat((char *)m_szStreamName,".binka");

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
					strcat((char *)m_szStreamName,".binka");
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
					strcat((char *)m_szStreamName,".binka");
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
				strcat((char *)m_szStreamName,".binka");
				
#endif
			}

			// 			wstring name = m_szStreamFileA[m_musicID];
			// 			char *SoundName = (char *)ConvertSoundPathToName(name);
			// 			strcat((char *)szStreamName,SoundName);

			const bool isCD = (m_musicID >= m_iStream_CD_1);
			const char* folder = isCD ? "cds/" : "music/";

			FILE* pFile = nullptr;
			if (fopen_s(&pFile, reinterpret_cast<char*>(m_szStreamName), "rb") == 0 && pFile)
			{
				fclose(pFile);
			}
			else
			{
				const char* extensions[] = { ".wav", ".mp3" }; // custom audio formats can be implemented via ma_audio_buffer and whatnot
				size_t count = sizeof(extensions) / sizeof(extensions[0]);
				bool found = false;

				for (size_t i = 0; i < count; i++)
				{
					int n = sprintf_s(reinterpret_cast<char*>(m_szStreamName), 512, "%s%s%s%s", m_szMusicPath, folder, m_szStreamFileA[m_musicID], extensions[i]);
					if (n < 0) continue;

					if (fopen_s(&pFile, reinterpret_cast<char*>(m_szStreamName), "rb") == 0 && pFile)
					{
						fclose(pFile);
						found = true;
						break;
					}
				}

				if (!found)
				{
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
			m_openStreamThread = NULL;

			app.DebugPrintf("OpenStreamThreadProc finished. m_musicStreamActive=%d\n", m_musicStreamActive);

#ifdef _WINDOWS64
        if (!m_musicStreamActive)
        {
            const char* currentExt = strrchr(reinterpret_cast<char*>(m_szStreamName), '.');
            if (currentExt && _stricmp(currentExt, ".binka") == 0)
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

        float finalVolume = m_StreamingAudioInfo.volume * m_MasterMusicVolume;

        ma_sound_set_volume(&m_musicStream, finalVolume);
        ma_result startResult = ma_sound_start(&m_musicStream);
        app.DebugPrintf("ma_sound_start result: %d\n", startResult);

#else
        HSAMPLE hSample = AIL_stream_sample_handle(m_hStream);

        AIL_register_falloff_function_callback(
            hSample,
            &custom_falloff_function);

        if (m_StreamingAudioInfo.bIs3D)
        {
            AIL_set_sample_3D_distances(hSample, 64.0f, 1, 0);

            if (m_validListenerCount > 1)
            {
                float fClosest = 10000.0f;
                int iClosestListener = 0;
                float fClosestX = 0.0f, fClosestY = 0.0f, fClosestZ = 0.0f, fDist;

                for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
                {
                    if (m_ListenerA[i].bValid)
                    {
                        float x = fabs(m_ListenerA[i].vPosition.x - m_StreamingAudioInfo.x);
                        float y = fabs(m_ListenerA[i].vPosition.y - m_StreamingAudioInfo.y);
                        float z = fabs(m_ListenerA[i].vPosition.z - m_StreamingAudioInfo.z);

                        fDist = x + y + z;

                        if (fDist < fClosest)
                        {
                            fClosest = fDist;
                            fClosestX = x;
                            fClosestY = y;
                            fClosestZ = z;
                            iClosestListener = i;
                        }
                    }
                }

                fDist = sqrtf((fClosestX * fClosestX) +
                              (fClosestY * fClosestY) +
                              (fClosestZ * fClosestZ));

                AIL_set_sample_3D_position(hSample, 0, 0, fDist);
            }
            else
            {
                AIL_set_sample_3D_position(
                    hSample,
                    m_StreamingAudioInfo.x,
                    m_StreamingAudioInfo.y,
                    -m_StreamingAudioInfo.z);
            }
        }
        else
        {
            AIL_set_sample_is_3D(hSample, 0);
        }

        AIL_set_sample_playback_rate_factor(
            hSample,
            m_StreamingAudioInfo.pitch);

        AIL_set_sample_volume_levels(
            hSample,
            m_StreamingAudioInfo.volume * getMasterMusicVolume(),
            m_StreamingAudioInfo.volume * getMasterMusicVolume());

        AIL_start_stream(m_hStream);
#endif

			m_StreamState=eMusicStreamState_Playing;
		}
		break;
	case eMusicStreamState_OpeningCancel:
		if( !m_openStreamThread->isRunning() )
		{
			delete m_openStreamThread;
			m_openStreamThread = NULL;
			m_StreamState = eMusicStreamState_Stop;
		}
		break;
	case eMusicStreamState_Stop:
#ifdef _WINDOWS64
		if (m_musicStreamActive)
		{
			ma_sound_stop(&m_musicStream);
			ma_sound_uninit(&m_musicStream);
			m_musicStreamActive = false;
		}
#else
		AIL_pause_stream(m_hStream, 1);
		AIL_close_stream(m_hStream);
		m_hStream = 0;
#endif

		SetIsPlayingStreamingCDMusic(false);
		SetIsPlayingStreamingGameMusic(false);

		m_StreamState = eMusicStreamState_Idle;
	break;
	case eMusicStreamState_Stopping:
		break;
	case eMusicStreamState_Play:
		break;
	case eMusicStreamState_Playing:
#ifdef _WINDOWS64
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
#endif
		if(GetIsPlayingStreamingGameMusic())
		{
			//if(m_MusicInfo.pCue!=NULL)
			{
				bool playerInEnd = false;
				bool playerInNether=false;
				Minecraft *pMinecraft = Minecraft::GetInstance();
				for(unsigned int i = 0; i < MAX_LOCAL_PLAYERS; ++i)
				{
					if(pMinecraft->localplayers[i]!=NULL)
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
				if (fMusicVol != getMasterMusicVolume())
				{
#ifdef _WINDOWS64
					if (m_musicStreamActive)
					{
						float finalVolume = m_StreamingAudioInfo.volume * fMusicVol;

						ma_sound_set_volume(&m_musicStream, finalVolume);
					}

#else
					HSAMPLE hSample = AIL_stream_sample_handle(m_hStream);

					float finalVolume = m_StreamingAudioInfo.volume * fMusicVol;
					AIL_set_sample_volume_levels(hSample, finalVolume, finalVolume);

#endif
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

				for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
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

#ifdef _WINDOWS64
				if (m_musicStreamActive)
				{
					ma_sound_set_position(&m_musicStream, relX, relY, relZ);
				}
#else
				HSAMPLE hSample = AIL_stream_sample_handle(m_hStream);
				AIL_set_sample_3D_position(hSample, relX, relY, relZ);
#endif
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
				if(pMinecraft->localplayers[i]!=NULL)
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

#ifdef _WINDOWS64
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
#else
	if (m_hStream != 0)
	{
		if (AIL_stream_status(m_hStream) == SMP_DONE)
		{
			AIL_close_stream(m_hStream);
			m_hStream = 0;

			SetIsPlayingStreamingCDMusic(false);
			SetIsPlayingStreamingGameMusic(false);

			m_StreamState = eMusicStreamState_Completed;
		}
	}
#endif
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


#ifndef _WINDOWS64
F32 AILCALLBACK custom_falloff_function (HSAMPLE   S, 
										 F32       distance,
										 F32       rolloff_factor,
										 F32       min_dist,
										 F32       max_dist)
{
	F32 result;

	// This is now emulating the linear fall-off function that we used on the Xbox 360. The parameter which is passed as "max_dist" is the only one actually used,
	// and is generally used as CurveDistanceScaler is used on XACT on the Xbox. A special value of 10000.0f is passed for thunder, which has no attenuation

	if( max_dist == 10000.0f )
	{
		return 1.0f;
	}

	result = 1.0f - ( distance / max_dist );
	if( result < 0.0f ) result = 0.0f;
	if( result > 1.0f ) result = 1.0f;

	return result;
}
#endif