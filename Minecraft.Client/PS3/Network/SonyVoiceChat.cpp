#include "stdafx.h"




/* SCE CONFIDENTIAL
PlayStation(R)3 Programmer Tool Runtime Library 430.001
* Copyright (C) 2008 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/
#include "SonyVoiceChat.h"
#include <arpa/inet.h>  /* inet_ntoa() */

/* for displaying extra information */
#ifndef _CONTENT_PACKAGE
#define AVC2_GAME_DEBUG
#endif

#ifdef AVC2_GAME_DEBUG
#define INF(...)    printf( "INF:" __VA_ARGS__ )
#define ERR(...)    printf( "ERR:" __VA_ARGS__ )
#else
#define INF(...)
#define ERR(...)
#endif

#define UNUSED_VARIABLE(x)  (void)(x)

//#define DISABLE_VOICE_CHAT

static CellSysutilAvc2InitParam g_chat_avc2param;

EAVCEvent	SonyVoiceChat::sm_event = AVC_EVENT_EPSILON;
EAVCState	SonyVoiceChat::sm_state = AVC_STATE_IDLE;
SQRNetworkManager_PS3* SonyVoiceChat::sm_pNetworkManager;
bool SonyVoiceChat::sm_bEnabled = true;
uint8_t SonyVoiceChat::sm_micStatus = CELL_AVC2_MIC_STATUS_UNKNOWN;
bool	SonyVoiceChat::sm_bLoaded = false;
bool	SonyVoiceChat::sm_bUnloading = false;
unordered_map<SceNpMatching2RoomMemberId, bool> SonyVoiceChat::sm_bTalkingMap;
bool	SonyVoiceChat::sm_bCanStart = false;
bool	SonyVoiceChat::sm_isChatRestricted = false;
int		SonyVoiceChat::sm_currentBitrate = 28000;

void SonyVoiceChat::init( SQRNetworkManager_PS3* pNetMan )
{
	if(sm_state != AVC_STATE_IDLE)
		return;

	sm_pNetworkManager = pNetMan;
	setState(AVC_STATE_CHAT_INIT);
	ProfileManager.GetChatAndContentRestrictions(0,false,&sm_isChatRestricted,NULL,NULL);
}

void SonyVoiceChat::shutdown()
{
	if(	sm_state == AVC_STATE_IDLE ||
		sm_state == AVC_STATE_CHAT_LEAVE ||
		sm_state == AVC_STATE_CHAT_UNLOAD ||
		sm_state == AVC_STATE_CHAT_RESET )
	{
		// we're either shut down already, or in the process
		return;
	}

	setEvent(AVC_EVENT_EXIT_GAME);
}

void SonyVoiceChat::setEnabled( bool bEnabled )
{
	if(sm_bEnabled != bEnabled)
	{
		if(sm_bCanStart)
		{
			if(bEnabled)
				startStream();
			else
				stopStream();
		}
		sm_bEnabled = bEnabled;
	}
}


int SonyVoiceChat::eventcb_load(CellSysutilAvc2EventId event_id, CellSysutilAvc2EventParam event_param, void *userdata)
{
	int ret = CELL_OK;

	UNUSED_VARIABLE( event_param );
	UNUSED_VARIABLE( userdata );

	if( event_id == CELL_AVC2_EVENT_LOAD_SUCCEEDED )
	{
		INF( "<AVC CB>CELL_AVC2_EVENT_LOAD_SUCCEEDED(0x%x), param(0x%x)\n", event_id, (int)event_param );
		setEvent(AVC_EVENT_CHAT_LOAD_SUCCEEDED);
		sm_bLoaded = true;

		// set the packet contention value here
		CellSysutilAvc2Attribute attr;
		memset( &attr, 0x00, sizeof(attr) );
		attr.attr_id = CELL_SYSUTIL_AVC2_ATTRIBUTE_VOICE_PACKET_CONTENTION;
		attr.attr_param.int_param = 3;
		int ret = cellSysutilAvc2SetAttribute(&attr);
		if( ret != CELL_OK )
		{
			app.DebugPrintf("CELL_SYSUTIL_AVC2_ATTRIBUTE_VOICE_PACKET_CONTENTION failed !!! 0x%08x\n", ret);
		}

	}
	else /* if( event_id == CELL_AVC2_EVENT_LOAD_FAILED ) */
	{
		INF( "<AVC CB>CELL_AVC2_EVENT_LOAD_FAILED(0x%x), param(0x%x)\n", event_id, (int)event_param );
		setEvent(AVC_EVENT_CHAT_LOAD_FAILED);
	}
	return ret;
}

int SonyVoiceChat::eventcb_join(CellSysutilAvc2EventId event_id, CellSysutilAvc2EventParam event_param, void *userdata)
{
	int ret = CELL_OK;

	UNUSED_VARIABLE( event_param );
	UNUSED_VARIABLE( userdata );

	if( event_id == CELL_AVC2_EVENT_JOIN_SUCCEEDED )
	{
		INF( "<AVC CB>CELL_AVC2_EVENT_JOIN_SUCCEEDED(0x%x), param(0x%x)\n", event_id, (int)event_param );
		setEvent(AVC_EVENT_CHAT_JOIN_SUCCEEDED);
	}
	else /* if( event_id == CELL_AVC2_EVENT_JOIN_FAILED ) */
	{
		INF( "<AVC CB>CELL_AVC2_EVENT_JOIN_FAILED(0x%x), param(0x%x)\n", event_id, (int)event_param );
		setEvent(AVC_EVENT_ERROR);
	}
	sm_bTalkingMap.clear();
	return ret;
}

int SonyVoiceChat::eventcb_leave( CellSysutilAvc2EventId event_id,	CellSysutilAvc2EventParam event_param, void *userdata)
{
	int ret = CELL_OK;

	UNUSED_VARIABLE( event_param );
	UNUSED_VARIABLE( userdata );

	if( event_id == CELL_AVC2_EVENT_LEAVE_SUCCEEDED )
	{
		INF( "<AVC CB>CELL_AVC2_EVENT_LEAVE_SUCCEEDED(0x%x), param(0x%x)\n", event_id, (int)event_param );
		setState(AVC_STATE_CHAT_LEAVE);
		setEvent(AVC_EVENT_CHAT_LEAVE_SUCCEEDED);
	}
	else /* if( event_id == CELL_AVC2_EVENT_LEAVE_FAILED ) */
	{
		INF( "<AVC CB>CELL_AVC2_EVENT_LEAVE_FAILED(0x%x), param(0x%x)\n", event_id, (int)event_param );
		setState(AVC_STATE_CHAT_LEAVE);
		setEvent(AVC_EVENT_ERROR);
	}
	return ret;
}

int SonyVoiceChat::eventcb_unload(CellSysutilAvc2EventId event_id,	CellSysutilAvc2EventParam event_param, void *userdata)
{
	int ret = CELL_OK;

	UNUSED_VARIABLE( event_param );
	UNUSED_VARIABLE( userdata );

	if( event_id == CELL_AVC2_EVENT_UNLOAD_SUCCEEDED )
	{
		INF( "<AVC CB>CELL_AVC2_EVENT_UNLOAD_SUCCEEDED(0x%x), param(0x%x)\n", event_id, (int)event_param );
		setEvent(AVC_EVENT_CHAT_UNLOAD_SUCCEEDED);
		sm_bLoaded = false;
		sm_bUnloading = false;
	}
	else /* if( event_id == CELL_AVC2_EVENT_UNLOAD_FAILED ) */
	{
		INF( "<AVC CB>CELL_AVC2_EVENT_UNLOAD_FAILED(0x%x), param(0x%x)\n", event_id, (int)event_param );
		setEvent(AVC_EVENT_ERROR);
	}
	return ret;
}

int SonyVoiceChat::eventcb_voiceDetected(CellSysutilAvc2EventId event_id,	CellSysutilAvc2EventParam event_param, void *userdata)
{
	UNUSED_VARIABLE( userdata );

// 	To the upper 32 bits, the room member ID of the player is passed.
// 	In the lower 32 bits, a value of 0 (mute) or a value between 1 (low volume)
// 	and 10 (high volume) is passed as the audio signal value when the notification
// 	method is the level method, or a value of 1 (start of speaking) or 0 (end of speaking)
// 	is stored when the notification method is the trigger method.

	SceNpMatching2RoomMemberId  roomMemberID = (SceNpMatching2RoomMemberId)(event_param >> 32);
	uint32_t volume = (uint32_t)(event_param & 0xffffffff);

// 	The precision of voice detection is not very high. Since the audio signal values may
// 	always be relatively high depending on the audio input device and the noise level in the
// 	room, you should set a large reference value for determining whether or not a player is
// 	speaking. Relatively good results can be obtained when an audio signal value of at
// 	least 9 is used to determine if a player is speaking.
	bool bTalking = false;
	if(volume >= 9)
		bTalking = true;

	sm_bTalkingMap[roomMemberID] = bTalking;
	return CELL_OK;
}

/* Callback function for handling AV Chat2 Utility events */
void SonyVoiceChat::eventcb( CellSysutilAvc2EventId event_id, CellSysutilAvc2EventParam event_param, void *userdata)
{
	static struct _cb_func_tbl
	{
		CellSysutilAvc2EventId event;
		int (*func)( CellSysutilAvc2EventId event_id,
			CellSysutilAvc2EventParam event_param,
			void *userdata );
	} event_tbl[] =
	{
		{ CELL_AVC2_EVENT_LOAD_SUCCEEDED,                   eventcb_load },
		{ CELL_AVC2_EVENT_LOAD_FAILED,                      eventcb_load },
		{ CELL_AVC2_EVENT_JOIN_SUCCEEDED,                   eventcb_join },
		{ CELL_AVC2_EVENT_JOIN_FAILED,                      eventcb_join },
		{ CELL_AVC2_EVENT_LEAVE_SUCCEEDED,                  eventcb_leave },
		{ CELL_AVC2_EVENT_LEAVE_FAILED,                     eventcb_leave },
		{ CELL_AVC2_EVENT_UNLOAD_SUCCEEDED,                 eventcb_unload },
		{ CELL_AVC2_EVENT_UNLOAD_FAILED,                    eventcb_unload },
		{ CELL_AVC2_EVENT_SYSTEM_NEW_MEMBER_JOINED,         NULL },
		{ CELL_AVC2_EVENT_SYSTEM_MEMBER_LEFT,               NULL },
		{ CELL_AVC2_EVENT_SYSTEM_SESSION_ESTABLISHED,       NULL },
		{ CELL_AVC2_EVENT_SYSTEM_SESSION_CANNOT_ESTABLISHED,NULL },
		{ CELL_AVC2_EVENT_SYSTEM_SESSION_DISCONNECTED,      NULL },
		{ CELL_AVC2_EVENT_SYSTEM_VOICE_DETECTED,            eventcb_voiceDetected },

	};

	int ret = 0;
	for( unsigned int i=0; i<sizeof(event_tbl)/sizeof(struct _cb_func_tbl) ; ++i )
	{
		if( event_tbl[ i ].event == event_id && event_tbl[ i ].func )
		{
			ret = (*event_tbl[ i ].func)( event_id, event_param, userdata );
			if( ret < 0 )
			{
				ERR("ret=0x%x\n", ret );
			}
			break;
		}
	}
}

int SonyVoiceChat::load()
{
	int ret = CELL_OK;
	INF("----------------------------\n");
	INF("| cellSysutilAvc2LoadAsync |\n");
	INF("----------------------------\n");
	ret = cellSysutilAvc2LoadAsync( sm_pNetworkManager->m_matchingContext,
		SYS_MEMORY_CONTAINER_ID_INVALID,
		eventcb,
		NULL,
		&g_chat_avc2param );
	if( ret != CELL_OK )
	{
		ERR( "cellSysutilAvc2LoadAsync: ret=0x%x\n", ret );
		setEvent(AVC_EVENT_ERROR);
		return ret;
	}
	return ret;
}

int SonyVoiceChat::join()
{
	int ret = CELL_OK;

	INF("---------------------------------------------------\n");
	INF("| cellSysutilAvc2JoinChatRequest                   \n");
	INF("---------------------------------------------------\n");
	ret = cellSysutilAvc2JoinChatRequest( &sm_pNetworkManager->m_room );
	if( ret != CELL_OK )
	{
		ERR( "cellSysutilAvc2JoinChatRequest: ret=0x%x\n", ret );
		setEvent(AVC_EVENT_ERROR);
		return ret;
	}
	return ret;
}

int SonyVoiceChat::leave()
{
	int ret = CELL_OK;

	INF("-----------------------------------\n");
	INF("| cellSysutilAvc2LeaveChatRequest |\n");
	INF("-----------------------------------\n");
	ret = cellSysutilAvc2LeaveChatRequest();
	if( ret != CELL_OK )
	{
		ERR( "cellSysutilAvc2LeaveChatRequest() = 0x%x\n", ret );
		setEvent(AVC_EVENT_ERROR);
		return ret;
	}
	return ret;
}

int SonyVoiceChat::unload()
{
	int ret = CELL_OK;

	INF("------------------------------\n");
	INF("| cellSysutilAvc2UnloadAsync |\n");
	INF("------------------------------\n");

	ret = cellSysutilAvc2UnloadAsync();
	if( ret != CELL_OK )
	{
		ERR( "cellSysutilAvcUnloadAsync() = 0x%x\n", ret );
		setEvent(AVC_EVENT_ERROR);
		return ret;
	}
	sm_bUnloading = true;
	return ret;
}

int SonyVoiceChat::start()
{
	sm_bCanStart = (sm_isChatRestricted == false);

	int ret = CELL_OK;
	if(sm_bEnabled)
		ret = startStream();

	return ret;
}

int SonyVoiceChat::stop()
{
	sm_bCanStart = false;

	int ret = CELL_OK;
	if(sm_bEnabled)
		ret = stopStream();

	setEvent(AVC_EVENT_CHAT_SESSION_STOPPED);


	return ret;
}

int SonyVoiceChat::startStream()
{
	int ret = CELL_OK;

	INF("---------------------------------\n");
	INF("| cellSysutilAvc2StartStreaming |\n");
	INF("---------------------------------\n");
	ret = cellSysutilAvc2StartStreaming();
	if( ret != CELL_OK )
	{
		ERR( "cellSysutilAvc2StartStreaming: ret=0x%x\n", ret );
	}

	ret =  cellSysutilAvc2StartVoiceDetection();
	if( ret != CELL_OK )
	{
		ERR( "cellSysutilAvc2StartVoiceDetection: ret=0x%x\n", ret );
	}
	return ret;
}

int SonyVoiceChat::stopStream()
{
	int ret =  cellSysutilAvc2StopVoiceDetection();
	if( ret != CELL_OK )
	{
		ERR( "cellSysutilAvc2StopVoiceDetection: ret=0x%x\n", ret );
	}

	INF("--------------------------------\n");
	INF("| cellSysutilAvc2StopStreaming |\n");
	INF("--------------------------------\n");
	ret = cellSysutilAvc2StopStreaming();
	if( ret != CELL_OK )
	{
		ERR( "cellSysutilAvc2StopStreaming: ret=0x%x\n", ret );
	}
	return ret;
}

int SonyVoiceChat::initialize(void)
{
	int ret;

	/* Must use cellSysutilAvc2InitParam() for clearing CellSysutilAvc2InitParam struct*/
	ret = cellSysutilAvc2InitParam(CELL_SYSUTIL_AVC2_INIT_PARAM_VERSION, &g_chat_avc2param);
	if( ret != CELL_OK )
	{
		ERR( "cellSysutilAvc2InitParam failed (0x%x)\n", ret );
		return ret;
	}

	/* Setting application specific parameters */
	g_chat_avc2param.media_type = CELL_SYSUTIL_AVC2_VOICE_CHAT;
	g_chat_avc2param.max_players = AVC2_PARAM_DEFAULT_MAX_PLAYERS;
	g_chat_avc2param.voice_param.voice_quality = CELL_SYSUTIL_AVC2_VOICE_QUALITY_NORMAL;
	g_chat_avc2param.voice_param.max_speakers = AVC2_PARAM_DEFAULT_MAX_SPEAKERS;
	g_chat_avc2param.spu_load_average  = 50;
	g_chat_avc2param.streaming_mode.mode = CELL_SYSUTIL_AVC2_STREAMING_MODE_NORMAL;

	setEvent(AVC_EVENT_CHAT_INIT_SUCCEEDED);
	setState(AVC_STATE_CHAT_INIT);

	return ret;
}

int SonyVoiceChat::finalize(void)
{
	setEvent(AVC_EVENT_CHAT_FINALIZE_SUCCEEDED);
	return CELL_OK;
}

void SonyVoiceChat::tick()
{
#ifdef DISABLE_VOICE_CHAT
	return;
#endif

	static state_transition_table tbl[] =
	{
		/*  now state						event									func					after the transition state */
		{ AVC_STATE_CHAT_INIT,				AVC_EVENT_EPSILON,						initialize,		AVC_STATE_CHAT_INIT },
		{ AVC_STATE_CHAT_INIT,				AVC_EVENT_EXIT_GAME,					invoke_epsilon_event,	AVC_STATE_CHAT_RESET },
		{ AVC_STATE_CHAT_INIT,				AVC_EVENT_ERROR,						invoke_epsilon_event,	AVC_STATE_CHAT_RESET },
		{ AVC_STATE_CHAT_INIT,				AVC_EVENT_LAN_DISCONNECT,				invoke_epsilon_event,	AVC_STATE_CHAT_RESET },
		{ AVC_STATE_CHAT_INIT,				AVC_EVENT_LAN_DISCONNECT,				invoke_epsilon_event,	AVC_STATE_CHAT_RESET },
		{ AVC_STATE_CHAT_INIT,				AVC_EVENT_CHAT_INIT_SUCCEEDED,			invoke_epsilon_event,	AVC_STATE_CHAT_LOAD },
		{ AVC_STATE_CHAT_INIT,				AVC_EVENT_NP2_ROOM_DESTROY_OR_KICKEDOUT,invoke_epsilon_event,	AVC_STATE_CHAT_RESET },

		{ AVC_STATE_CHAT_LOAD,				AVC_EVENT_EPSILON,						load,				AVC_STATE_CHAT_LOAD },
		{ AVC_STATE_CHAT_LOAD,				AVC_EVENT_EXIT_GAME,					invoke_epsilon_event,	AVC_STATE_CHAT_RESET },
		{ AVC_STATE_CHAT_LOAD,				AVC_EVENT_ERROR,						invoke_epsilon_event,	AVC_STATE_CHAT_RESET },
		{ AVC_STATE_CHAT_LOAD,				AVC_EVENT_LAN_DISCONNECT,				invoke_epsilon_event,	AVC_STATE_CHAT_RESET },
		{ AVC_STATE_CHAT_LOAD,				AVC_EVENT_CHAT_LOAD_SUCCEEDED,			invoke_epsilon_event,	AVC_STATE_CHAT_JOIN },
		{ AVC_STATE_CHAT_LOAD,				AVC_EVENT_NP2_ROOM_DESTROY_OR_KICKEDOUT,invoke_epsilon_event,	AVC_STATE_CHAT_RESET },

		{ AVC_STATE_CHAT_JOIN,				AVC_EVENT_EPSILON,						join,				AVC_STATE_CHAT_JOIN },
		{ AVC_STATE_CHAT_JOIN,				AVC_EVENT_EXIT_GAME,					invoke_epsilon_event,	AVC_STATE_CHAT_UNLOAD },
		{ AVC_STATE_CHAT_JOIN,				AVC_EVENT_ERROR,						invoke_epsilon_event,	AVC_STATE_CHAT_UNLOAD },
		{ AVC_STATE_CHAT_JOIN,				AVC_EVENT_LAN_DISCONNECT,				invoke_epsilon_event,	AVC_STATE_CHAT_UNLOAD },
		{ AVC_STATE_CHAT_JOIN,				AVC_EVENT_CHAT_JOIN_SUCCEEDED,			invoke_epsilon_event,	AVC_STATE_CHAT_SESSION_PROCESSING },
		{ AVC_STATE_CHAT_JOIN,				AVC_EVENT_NP2_ROOM_DESTROY_OR_KICKEDOUT,invoke_epsilon_event,	AVC_STATE_CHAT_UNLOAD },

		{ AVC_STATE_CHAT_SESSION_PROCESSING,AVC_EVENT_EPSILON,						start,				AVC_STATE_CHAT_SESSION_PROCESSING },
		{ AVC_STATE_CHAT_SESSION_PROCESSING,AVC_EVENT_EXIT_GAME,					stop,				AVC_STATE_CHAT_SESSION_PROCESSING },
		{ AVC_STATE_CHAT_SESSION_PROCESSING,AVC_EVENT_ERROR,						stop,				AVC_STATE_CHAT_SESSION_PROCESSING },
		{ AVC_STATE_CHAT_SESSION_PROCESSING,AVC_EVENT_LAN_DISCONNECT,				stop,				AVC_STATE_CHAT_SESSION_PROCESSING },
		{ AVC_STATE_CHAT_SESSION_PROCESSING,AVC_EVENT_CHAT_SESSION_STOPPED,			invoke_epsilon_event,	AVC_STATE_CHAT_LEAVE },
		{ AVC_STATE_CHAT_SESSION_PROCESSING,AVC_EVENT_NP2_ROOM_DESTROY_OR_KICKEDOUT,stop,				AVC_STATE_CHAT_SESSION_PROCESSING },

		{ AVC_STATE_CHAT_LEAVE,				AVC_EVENT_EPSILON,						leave,				AVC_STATE_CHAT_LEAVE },
		{ AVC_STATE_CHAT_LEAVE,				AVC_EVENT_ERROR,						invoke_epsilon_event,	AVC_STATE_CHAT_UNLOAD },
		{ AVC_STATE_CHAT_LEAVE,				AVC_EVENT_CHAT_LEAVE_SUCCEEDED,			invoke_epsilon_event,	AVC_STATE_CHAT_UNLOAD },

		{ AVC_STATE_CHAT_UNLOAD,			AVC_EVENT_EPSILON,						unload,			AVC_STATE_CHAT_UNLOAD },
		{ AVC_STATE_CHAT_UNLOAD,			AVC_EVENT_ERROR,						invoke_epsilon_event,	AVC_STATE_CHAT_RESET },
		{ AVC_STATE_CHAT_UNLOAD,			AVC_EVENT_CHAT_UNLOAD_SUCCEEDED,		invoke_epsilon_event,	AVC_STATE_CHAT_RESET },

		{ AVC_STATE_CHAT_RESET,				AVC_EVENT_EPSILON,						finalize,			AVC_STATE_CHAT_RESET },
		{ AVC_STATE_CHAT_RESET,				AVC_EVENT_ERROR,						invoke_epsilon_event,	AVC_STATE_CHAT_LEAVE },
		{ AVC_STATE_CHAT_RESET,				AVC_EVENT_CHAT_FINALIZE_SUCCEEDED,		invoke_epsilon_event,	AVC_STATE_IDLE },

	};
	do_state_transition( &tbl[0], sizeof( tbl ) / sizeof( state_transition_table ) );

	setBitRate();
}



void SonyVoiceChat::do_state_transition( state_transition_table *tbl, int tbl_size )
{
	int ret = CELL_OK;

// 	if( sm_event == AVC_EVENT_LAN_DISCONNECT ||
// 		sm_event == AVC_EVENT_EXIT_GAME ||
// 		sm_event == AVC_EVENT_ERROR ||
// 		sm_event == AVC_EVENT_FATAL_ERROR )
// 	{
// 		g_gamedata.finalize = 1;
// 	}
// 	if( sm_event == AVC_EVENT_FATAL_ERROR )
// 	{
// 		g_gamedata.exit = true;
// 	}
	bool bCalledFunc = false;
	for( int i = 0; i < tbl_size; i++ )
	{
		if( sm_state == ( tbl + i )->state )
		{
			if( sm_event == ( tbl + i )->event )
			{
				sm_event = AVC_EVENT_NON;
				ret = (*( tbl + i )->func)();
				bCalledFunc = true;
				if( ret != CELL_OK )
				{
					ERR("ret = 0x%x\n", ret );
				}

				setState(( tbl + i )->new_state);
				break;
			}
		}
	}
	if(bCalledFunc == false)
	{
 		assert(	(sm_event == AVC_EVENT_NON) ||
				(sm_state == AVC_STATE_IDLE && sm_event == AVC_EVENT_EPSILON) );
	}
}

int SonyVoiceChat::invoke_epsilon_event(void)
{
	setEvent(AVC_EVENT_EPSILON);

	return CELL_OK;
}


bool SonyVoiceChat::hasMicConnected(const SceNpMatching2RoomMemberId *players_id)
{
	CellSysutilAvc2PlayerInfo players_info;
	int err = cellSysutilAvc2GetPlayerInfo(players_id, &players_info);
	if(err == CELL_OK)
	{
		if(players_info.connected && players_info.joined)
		{
			if(players_info.mic_attached == CELL_AVC2_MIC_STATUS_ATTACHED_ON)
				return true;
		}
	}

	return false;
}

void SonyVoiceChat::mute( bool bMute )
{
	if(sm_bLoaded && !sm_bUnloading)
	{
		int err = cellSysutilAvc2SetVoiceMuting(bMute);
		assert(err == CELL_OK);
	}
}

void SonyVoiceChat::mutePlayer( const SceNpMatching2RoomMemberId member_id, bool bMute ) /*Turn chat audio from a specified player on or off */
{
	if(sm_bLoaded && !sm_bUnloading)
	{
		int err = cellSysutilAvc2SetPlayerVoiceMuting(member_id, bMute);
		assert(err == CELL_OK);
	}
}

void SonyVoiceChat::muteLocalPlayer( bool bMute ) /*Turn microphone input on or off */
{
	if(sm_bLoaded && !sm_bUnloading)
	{
		int err = cellSysutilAvc2SetVoiceMuting(bMute);
		assert(err == CELL_OK);
	}
}


bool SonyVoiceChat::isMuted()
{
	if(sm_bLoaded && !sm_bUnloading)
	{
		uint8_t bMute;
		int err = cellSysutilAvc2GetVoiceMuting(&bMute);
		assert(err == CELL_OK);
		return bMute;
	}
	return false;
}

bool SonyVoiceChat::isMutedPlayer( const SceNpMatching2RoomMemberId member_id)
{
	if(sm_bLoaded && !sm_bUnloading)
	{
		uint8_t bMute;
		int err = cellSysutilAvc2GetPlayerVoiceMuting(member_id, &bMute);
		assert(err == CELL_OK);
		return bMute;
	}
	return false;
}

bool SonyVoiceChat::isMutedLocalPlayer()
{
	if(sm_bLoaded && !sm_bUnloading)
	{
		uint8_t bMute;
		int err = cellSysutilAvc2GetVoiceMuting(&bMute);
		assert(err == CELL_OK);
		return bMute;
	}
	return false;
}

void SonyVoiceChat::setVolume( float vol )
{
	if(sm_bLoaded && !sm_bUnloading)
	{
		// The volume level can be set to a value in the range 0.0 to 40.0.
		// Volume levels are linear values such that if 1.0 is specified, the
		// volume level will be 1 times the reference power (0dB), and if 0.5
		// is specified, the volume level will be 0.5 times the reference power
		// (-6dB). If 0.0 is specified, chat audio will no longer be audible.

		int err =cellSysutilAvc2SetSpeakerVolumeLevel(vol * 40.0f);
		assert(err==CELL_OK);
	}
}

float SonyVoiceChat::getVolume()
{
	if(sm_bLoaded && !sm_bUnloading)
	{
		float vol;
		int err = cellSysutilAvc2GetSpeakerVolumeLevel(&vol);
		assert(err == CELL_OK);
		return (vol / 40.0f);
	}
	return 0;
}

bool SonyVoiceChat::isTalking( SceNpMatching2RoomMemberId* players_id )
{
    auto it = sm_bTalkingMap.find(*players_id);
    if (it != sm_bTalkingMap.end() )
		return it->second;
	return false;
}

void SonyVoiceChat::setBitRate()
{
	if(sm_state != AVC_STATE_CHAT_SESSION_PROCESSING)
		return;

	int numPlayers = sm_pNetworkManager->GetPlayerCount();
// 	This internal attribute represents the maximum voice bit rate. attr_param
//	is an integer value. The units are bps, and the specifiable values are
//	4000, 8000, 16000, 20000, 24000, and 28000. The initial value is 28000.

	static int bitRates[8] = {	28000, 28000,
								28000, 28000,
								24000, 20000,
								16000, 16000};
	int bitRate = bitRates[numPlayers-1];
	if(bitRate == sm_currentBitrate)
		return;

	CellSysutilAvc2Attribute attr;
	memset( &attr, 0x00, sizeof(attr) );
	attr.attr_id = CELL_SYSUTIL_AVC2_ATTRIBUTE_VOICE_MAX_BITRATE;
	attr.attr_param.int_param = bitRate;
	int ret = cellSysutilAvc2SetAttribute(&attr);
	if( ret == CELL_OK )
	{
		sm_currentBitrate = bitRate;
	}
	else
	{
 		app.DebugPrintf("SonyVoiceChat::setBitRate failed !!!   0x%08x\n", ret);
	}
}



#define CASE_STR_VALUE(s) case s: return #s;

const char* getStateString(EAVCState state)
{

	switch(state)
	{

		CASE_STR_VALUE(AVC_STATE_IDLE);
		CASE_STR_VALUE(AVC_STATE_CHAT_INIT)
		CASE_STR_VALUE(AVC_STATE_CHAT_LOAD);
		CASE_STR_VALUE(AVC_STATE_CHAT_JOIN);
		CASE_STR_VALUE(AVC_STATE_CHAT_SESSION_PROCESSING);
		CASE_STR_VALUE(AVC_STATE_CHAT_LEAVE);
		CASE_STR_VALUE(AVC_STATE_CHAT_RESET);
		CASE_STR_VALUE(AVC_STATE_CHAT_UNLOAD);
		CASE_STR_VALUE(AVC_STATE_EXIT);
	default:
		return "unknown state";
	}
}

const char* getEventString(EAVCEvent state)
{
	switch(state)
	{
		CASE_STR_VALUE(AVC_EVENT_NON);
		CASE_STR_VALUE(AVC_EVENT_EPSILON);
		CASE_STR_VALUE(AVC_EVENT_LAN_DISCONNECT);
		CASE_STR_VALUE(AVC_EVENT_ONLINE);
		CASE_STR_VALUE(AVC_EVENT_OFFLINE);
		CASE_STR_VALUE(AVC_EVENT_EXIT_GAME);
		CASE_STR_VALUE(AVC_EVENT_ROOM_CREATE);
		CASE_STR_VALUE(AVC_EVENT_ROOM_SEARCH);
		CASE_STR_VALUE(AVC_EVENT_ERROR);
		CASE_STR_VALUE(AVC_EVENT_FATAL_ERROR);
		CASE_STR_VALUE(AVC_EVENT_NETDIALOG_FINISHED);
		CASE_STR_VALUE(AVC_EVENT_NETDIALOG_UNLOADED);
		CASE_STR_VALUE(AVC_EVENT_NP2_INIT_SUCCEEDED);
		CASE_STR_VALUE(AVC_EVENT_NP2_FINALIZE_SUCCEEDED);
		CASE_STR_VALUE(AVC_EVENT_NP2_START_CONTEXT_SUCCEEDED);
		CASE_STR_VALUE(AVC_EVENT_NP2_STOP_CONTEXT_SUCCEEDED);
		CASE_STR_VALUE(AVC_EVENT_NP2_ROOM_DESTROY_OR_KICKEDOUT);
		CASE_STR_VALUE(AVC_EVENT_NP2_ROOM_MEMBER_LEFT);
		CASE_STR_VALUE(AVC_EVENT_NP2_ROOM_MEMBER_JOINED);
		CASE_STR_VALUE(AVC_EVENT_CHAT_LOAD_SUCCEEDED);
		CASE_STR_VALUE(AVC_EVENT_CHAT_LOAD_FAILED);
		CASE_STR_VALUE(AVC_EVENT_CHAT_JOIN_SUCCEEDED);
		CASE_STR_VALUE(AVC_EVENT_CHAT_JOIN_FAILED);
		CASE_STR_VALUE(AVC_EVENT_CHAT_LEAVE_SUCCEEDED);
		CASE_STR_VALUE(AVC_EVENT_CHAT_LEAVE_FAILED);
		CASE_STR_VALUE(AVC_EVENT_CHAT_UNLOAD_SUCCEEDED);
		CASE_STR_VALUE(AVC_EVENT_CHAT_UNLOAD_FAILED);
		CASE_STR_VALUE(AVC_EVENT_CHAT_INIT_SUCCEEDED);
		CASE_STR_VALUE(AVC_EVENT_CHAT_FINALIZE_SUCCEEDED);
		CASE_STR_VALUE(AVC_EVENT_CHAT_SESSION_STOPPED);
		CASE_STR_VALUE(AVC_EVENT_CREATE_JOIN_ROOM_SUCCEEDED);
		CASE_STR_VALUE(AVC_EVENT_SEARCH_ROOM_SUCCEEDED);
		CASE_STR_VALUE(AVC_EVENT_JOIN_ROOM_SUCCEEDED);
		CASE_STR_VALUE(AVC_EVENT_LEAVE_ROOM_SUCCEEDED);
		CASE_STR_VALUE(AVC_EVENT_SIGNALING_ESTABLISHED);
		CASE_STR_VALUE(AVC_EVENT_SIGNALING_DEAD);
		CASE_STR_VALUE(AVC_EVENT_UI_CLOSE_SUCCEEDED);
	default:
		return "unknown event";
	}
}

void SonyVoiceChat::printStateAndEvent()
{
	app.DebugPrintf("==============    State %20s, Event %20s\n", getStateString(sm_state), getEventString(sm_event));
}


void SonyVoiceChat::setEvent( EAVCEvent event )
{
	sm_event = event;
	printStateAndEvent();
}

void SonyVoiceChat::setState( EAVCState state )
{
	sm_state = state;
	printStateAndEvent();
}
