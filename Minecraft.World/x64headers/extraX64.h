#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <functional>

#include "..\..\Minecraft.Client\SkinBox.h"


#include <vector>

#define MULTITHREAD_ENABLE

typedef unsigned char byte;

const int XUSER_INDEX_ANY = 255;
const int XUSER_INDEX_FOCUS = 254;

#ifdef __PSVITA__
const int XUSER_MAX_COUNT = 1;
const int MINECRAFT_NET_MAX_PLAYERS = 4;
#else
const int XUSER_MAX_COUNT = 4;
const int MINECRAFT_NET_MAX_PLAYERS = 8;
#endif



#ifdef __ORBIS__
#include <net.h>
#include <np/np_npid.h>
#include <user_service.h>
#include "..\..\Minecraft.Client\Orbis\Orbis_PlayerUID.h"
#include "..\..\Minecraft.Client\Orbis\Network\SQRNetworkManager_Orbis.h"
typedef SQRNetworkManager_Orbis::SessionID SessionID;
typedef SQRNetworkManager_Orbis::PresenceSyncInfo INVITE_INFO;

#elif defined __PS3__		// defined in the profile lib
#include <np.h>
#include <sysutil/sysutil_userinfo.h>
#include <netex/libnetctl.h>
#include <assert.h>
#include <stdlib.h>
#include "..\..\Minecraft.Client\PS3\PS3_PlayerUID.h"
#include "..\..\Minecraft.Client\PS3\Network\SQRNetworkManager_PS3.h"
typedef SQRNetworkManager::SessionID SessionID;
typedef SQRNetworkManager::PresenceSyncInfo INVITE_INFO;

#elif defined __PSVITA__
#include <np.h>
#include <assert.h>
#include <stdlib.h>
#include "..\..\Minecraft.Client\PSVita\Network\SQRNetworkManager_Vita.h"
#include "..\..\Minecraft.Client\PSVita\Network\SQRNetworkManager_AdHoc_Vita.h"
#include "..\..\Minecraft.Client\PSVita\4JLibs\inc\4J_Profile.h"
typedef SQRNetworkManager_Vita::SessionID SessionID;
typedef SQRNetworkManager_Vita::PresenceSyncInfo INVITE_INFO;

#elif defined _DURANGO
#include "..\..\Minecraft.Client\Durango\4JLibs\inc\4J_Profile.h"
#include "..\..\Minecraft.Client\Durango\Network\DQRNetworkManager.h"
typedef ULONGLONG SessionID;
typedef ULONGLONG GameSessionUID;
typedef DQRNetworkManager::SessionInfo INVITE_INFO;
#else
typedef ULONGLONG PlayerUID;
typedef ULONGLONG SessionID;
typedef PlayerUID GameSessionUID;
class INVITE_INFO;

#endif //  __PS3__

#if !(defined _DURANGO || defined __PSVITA__)
typedef PlayerUID *PPlayerUID;
#endif
typedef struct _XUIOBJ* HXUIOBJ;
typedef struct _XUICLASS* HXUICLASS;
typedef struct _XUIBRUSH* HXUIBRUSH;
typedef struct _XUIDC* HXUIDC;



// #ifdef _DURANGO
// void GetLocalTime(SYSTEMTIME *time);
// #endif

bool IsEqualXUID(PlayerUID a, PlayerUID b);

using namespace std;

// Temporary implementation of lock free stack with quite a bit more locking than you might expect
template <typename T> class XLockFreeStack
{
	std::vector<T *> intStack;
public:
	XLockFreeStack() {
#ifdef __ORBIS__
		OrbisInit();		// For PS4, we need to make sure Ult is set up for the critical sections to be able to initialise
#endif
		InitializeCriticalSectionAndSpinCount(&m_cs,5120);
	}
	~XLockFreeStack() { DeleteCriticalSection( &m_cs ); }
	void Initialize() {}
	void Push(T *data)
	{
		EnterCriticalSection(&m_cs);
		intStack.push_back(data);
		LeaveCriticalSection(&m_cs);
	}
	T *Pop()
	{
		EnterCriticalSection(&m_cs);
		if( intStack.size() )
		{
			T* ret = intStack.back();
			intStack.pop_back();
			LeaveCriticalSection(&m_cs);
			return ret;
		}
		else
		{
			LeaveCriticalSection(&m_cs);
			return NULL;
		}
	}
private:
	CRITICAL_SECTION m_cs;
};

void XMemCpy(void *a, const void *b, size_t s);
void XMemSet(void *a, int t, size_t s);
void XMemSet128(void *a, int t, size_t s);
void *XPhysicalAlloc(SIZE_T a, ULONG_PTR  b, ULONG_PTR c, DWORD d);
void XPhysicalFree(void *a);

class DLCManager;

//class LevelGenerationOptions;
class LevelRuleset;
class ModelPart;
class LevelChunk;
class IXACT3Engine;
class XACT_NOTIFICATION;
class ConsoleSchematicFile;

const int XZP_ICON_SHANK_01 = 1;
const int XZP_ICON_SHANK_02 = 2;
const int XZP_ICON_SHANK_03 = 3;

const int XN_SYS_SIGNINCHANGED = 0;
const int XN_SYS_INPUTDEVICESCHANGED = 1;
const int XN_LIVE_CONTENT_INSTALLED = 2;
const int XN_SYS_STORAGEDEVICESCHANGED = 3;

//
// Codes returned for the gamepad input
//

#define VK_PAD_A                        0x5800
#define VK_PAD_B                        0x5801
#define VK_PAD_X                        0x5802
#define VK_PAD_Y                        0x5803
#define VK_PAD_RSHOULDER                0x5804
#define VK_PAD_LSHOULDER                0x5805
#define VK_PAD_LTRIGGER                 0x5806
#define VK_PAD_RTRIGGER                 0x5807

#define VK_PAD_DPAD_UP                  0x5810
#define VK_PAD_DPAD_DOWN                0x5811
#define VK_PAD_DPAD_LEFT                0x5812
#define VK_PAD_DPAD_RIGHT               0x5813
#define VK_PAD_START                    0x5814
#define VK_PAD_BACK                     0x5815
#define VK_PAD_LTHUMB_PRESS             0x5816
#define VK_PAD_RTHUMB_PRESS             0x5817

#define VK_PAD_LTHUMB_UP                0x5820
#define VK_PAD_LTHUMB_DOWN              0x5821
#define VK_PAD_LTHUMB_RIGHT             0x5822
#define VK_PAD_LTHUMB_LEFT              0x5823
#define VK_PAD_LTHUMB_UPLEFT            0x5824
#define VK_PAD_LTHUMB_UPRIGHT           0x5825
#define VK_PAD_LTHUMB_DOWNRIGHT         0x5826
#define VK_PAD_LTHUMB_DOWNLEFT          0x5827

#define VK_PAD_RTHUMB_UP                0x5830
#define VK_PAD_RTHUMB_DOWN              0x5831
#define VK_PAD_RTHUMB_RIGHT             0x5832
#define VK_PAD_RTHUMB_LEFT              0x5833
#define VK_PAD_RTHUMB_UPLEFT            0x5834
#define VK_PAD_RTHUMB_UPRIGHT           0x5835
#define VK_PAD_RTHUMB_DOWNRIGHT         0x5836
#define VK_PAD_RTHUMB_DOWNLEFT          0x5837

const int XUSER_NAME_SIZE = 32;

class IQNetPlayer
{
public:
	BYTE GetSmallId();
	void SendData(IQNetPlayer *player, const void *pvData, DWORD dwDataSize, DWORD dwFlags);
	bool IsSameSystem(IQNetPlayer *player);
	DWORD GetSendQueueSize( IQNetPlayer *player, DWORD dwFlags );
	DWORD GetCurrentRtt();
	bool IsHost();
	bool IsGuest();
	bool IsLocal();
	PlayerUID GetXuid();	
	LPCWSTR GetGamertag();
	int GetSessionIndex();
	bool IsTalking();
	bool IsMutedByLocalUser(DWORD dwUserIndex);
	bool HasVoice();
	bool HasCamera();
	int GetUserIndex();
	void SetCustomDataValue(ULONG_PTR ulpCustomDataValue);
	ULONG_PTR GetCustomDataValue();

    BYTE m_smallId;
    bool m_isRemote;
    bool m_isHostPlayer;
    PlayerUID m_resolvedXuid;
    wchar_t m_gamertag[32];
private:
	ULONG_PTR m_customData;
};

void Win64_SetupRemoteQNetPlayer(IQNetPlayer* player, BYTE smallId, bool isHost, bool isLocal);

const int QNET_GETSENDQUEUESIZE_SECONDARY_TYPE = 0;
const int QNET_GETSENDQUEUESIZE_MESSAGES = 0;
const int QNET_GETSENDQUEUESIZE_BYTES = 0;


typedef struct {
    BYTE bFlags;
    BYTE bReserved;
    WORD cProbesXmit;
    WORD cProbesRecv;
    WORD cbData;
    BYTE *pbData;
    WORD wRttMinInMsecs;
    WORD wRttMedInMsecs;
    DWORD dwUpBitsPerSec;
    DWORD dwDnBitsPerSec;
} XNQOSINFO;

typedef struct {
    UINT cxnqos;
    UINT cxnqosPending;
    XNQOSINFO axnqosinfo[1];
} XNQOS;

typedef struct _XOVERLAPPED {
} XOVERLAPPED, *PXOVERLAPPED;

typedef struct _XSESSION_SEARCHRESULT {
} XSESSION_SEARCHRESULT, *PXSESSION_SEARCHRESULT;

typedef struct {
    DWORD dwContextId;
    DWORD dwValue;
} XUSER_CONTEXT, *PXUSER_CONTEXT;

typedef struct _XSESSION_SEARCHRESULT_HEADER {
    DWORD dwSearchResults;
    XSESSION_SEARCHRESULT *pResults;
} XSESSION_SEARCHRESULT_HEADER, *PXSESSION_SEARCHRESULT_HEADER;

typedef struct _XONLINE_FRIEND {
    PlayerUID xuid;
    CHAR szGamertag[XUSER_NAME_SIZE];
    DWORD dwFriendState;
    SessionID sessionID;
    DWORD dwTitleID;
    FILETIME ftUserTime;
    SessionID xnkidInvite;
    FILETIME gameinviteTime;
    DWORD cchRichPresence;
//    WCHAR wszRichPresence[MAX_RICHPRESENCE_SIZE];
} XONLINE_FRIEND, *PXONLINE_FRIEND;

class IQNetCallbacks
{
};

class IQNetGameSearch
{
};


typedef enum _QNET_STATE
{
    QNET_STATE_IDLE,
    QNET_STATE_SESSION_HOSTING,
    QNET_STATE_SESSION_JOINING,
    QNET_STATE_GAME_LOBBY,
    QNET_STATE_SESSION_REGISTERING,
    QNET_STATE_SESSION_STARTING,
    QNET_STATE_GAME_PLAY,
    QNET_STATE_SESSION_ENDING,
    QNET_STATE_SESSION_LEAVING,
    QNET_STATE_SESSION_DELETING
} QNET_STATE, * PQNET_STATE;

class IQNet
{
public:
	HRESULT AddLocalPlayerByUserIndex(DWORD dwUserIndex);
	IQNetPlayer *GetHostPlayer();
	IQNetPlayer *GetLocalPlayerByUserIndex(DWORD dwUserIndex);
	IQNetPlayer *GetPlayerByIndex(DWORD dwPlayerIndex);
	IQNetPlayer *GetPlayerBySmallId(BYTE SmallId);
	IQNetPlayer *GetPlayerByXuid(PlayerUID xuid);
	DWORD GetPlayerCount();
	QNET_STATE GetState();
	bool IsHost();
	HRESULT JoinGameFromInviteInfo(DWORD dwUserIndex, DWORD dwUserMask, const INVITE_INFO *pInviteInfo);
	void HostGame();
    void ClientJoinGame();
    void EndGame();

    static IQNetPlayer m_player[MINECRAFT_NET_MAX_PLAYERS];
    static DWORD s_playerCount;
    static bool s_isHosting;
};

#ifdef _DURANGO
// 4J Stu - We don't want to be doing string conversions at runtime for timing instrumentation, so do this instead
#define PIXBeginNamedEvent(a, b, ...) PIXBeginEvent(a,L ## b, __VA_ARGS__)
#define PIXEndNamedEvent() PIXEndEvent()
#define PIXSetMarkerDeprecated(a, b, ...) PIXSetMarker(a, L ## b, __VA_ARGS__)
#define PIXAddNamedCounter(a, b) PIXReportCounter( L ## b, a)
#else
void PIXAddNamedCounter(int a, char *b, ...);
void PIXBeginNamedEvent(int a, char *b, ...);
void PIXEndNamedEvent();
void PIXSetMarkerDeprecated(int a, char *b, ...);
#endif

void XSetThreadProcessor(HANDLE a, int b);
//BOOL XCloseHandle(HANDLE a);

const int QNET_SENDDATA_LOW_PRIORITY = 0;
const int QNET_SENDDATA_SECONDARY = 0;

#if defined(__PS3__) || defined(__ORBIS__) || defined(_DURANGO) || defined(__PSVITA__)
#define INVALID_XUID PlayerUID()
#else
const int INVALID_XUID = 0;
#endif
// const int MOJANG_DATA = 0;

// typedef struct _STRING_VERIFY_RESPONSE
// {
//     WORD wNumStrings;
//     HRESULT *pStringResult;
// } STRING_VERIFY_RESPONSE;

const int XCONTENT_MAX_DISPLAYNAME_LENGTH = 256;
const int XCONTENT_MAX_FILENAME_LENGTH = 256;
typedef int XCONTENTDEVICEID;

#if !defined( __PS3__) && !defined(__ORBIS__) && !defined(_DURANGO) && !defined(__PSVITA__)
typedef struct _XCONTENT_DATA
{
    XCONTENTDEVICEID DeviceID;
    DWORD dwContentType;
    WCHAR szDisplayName[XCONTENT_MAX_DISPLAYNAME_LENGTH];
    CHAR szFileName[XCONTENT_MAX_FILENAME_LENGTH];
} XCONTENT_DATA, *PXCONTENT_DATA;
#endif //__PS3__

static const int XMARKETPLACE_CONTENT_ID_LEN = 4;

#ifndef _DURANGO
typedef struct _XMARKETPLACE_CONTENTOFFER_INFO
{
    ULONGLONG qwOfferID;
    ULONGLONG qwPreviewOfferID;
    DWORD dwOfferNameLength;
    WCHAR *wszOfferName;
    DWORD dwOfferType;
    BYTE contentId[XMARKETPLACE_CONTENT_ID_LEN];
    BOOL fIsUnrestrictedLicense;
    DWORD dwLicenseMask;
    DWORD dwTitleID;
    DWORD dwContentCategory;
    DWORD dwTitleNameLength;
    WCHAR *wszTitleName;
    BOOL fUserHasPurchased;
    DWORD dwPackageSize;
    DWORD dwInstallSize;
    DWORD dwSellTextLength;
    WCHAR *wszSellText;
    DWORD dwAssetID;
    DWORD dwPurchaseQuantity;
    DWORD dwPointsPrice;
} XMARKETPLACE_CONTENTOFFER_INFO, *PXMARKETPLACE_CONTENTOFFER_INFO;

typedef enum 
{
    XMARKETPLACE_OFFERING_TYPE_CONTENT = 0x00000002,
    XMARKETPLACE_OFFERING_TYPE_GAME_DEMO = 0x00000020,
    XMARKETPLACE_OFFERING_TYPE_GAME_TRAILER = 0x00000040,
    XMARKETPLACE_OFFERING_TYPE_THEME = 0x00000080,
    XMARKETPLACE_OFFERING_TYPE_TILE = 0x00000800,
    XMARKETPLACE_OFFERING_TYPE_ARCADE = 0x00002000,
    XMARKETPLACE_OFFERING_TYPE_VIDEO = 0x00004000,
    XMARKETPLACE_OFFERING_TYPE_CONSUMABLE = 0x00010000,
    XMARKETPLACE_OFFERING_TYPE_AVATARITEM = 0x00100000
} XMARKETPLACE_OFFERING_TYPE;
#endif // _DURANGO

const int QNET_SENDDATA_RELIABLE = 0;
const int QNET_SENDDATA_SEQUENTIAL = 0;

struct XRNM_SEND_BUFFER
{
	DWORD dwDataSize;
	byte *pbyData;
};

const int D3DBLEND_CONSTANTALPHA = 0;
const int D3DBLEND_INVCONSTANTALPHA = 0;
const int D3DPT_QUADLIST = 0;


typedef struct _XUSER_SIGNIN_INFO {
	PlayerUID                 xuid;
    DWORD dwGuestNumber;
} XUSER_SIGNIN_INFO, *PXUSER_SIGNIN_INFO;

#define XUSER_GET_SIGNIN_INFO_ONLINE_XUID_ONLY      0x00000001
#define XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY     0x00000002

DWORD XUserGetSigninInfo(
         DWORD dwUserIndex,
         DWORD dwFlags,
         PXUSER_SIGNIN_INFO pSigninInfo
);

class CXuiStringTable
{
public:
	LPCWSTR Lookup(LPCWSTR szId);
	LPCWSTR Lookup(UINT nIndex);
	void Clear();
	HRESULT Load(LPCWSTR szId);
};

#if !defined(__ORBIS__) && !defined(_XBOX_ONE)
typedef VOID * XMEMDECOMPRESSION_CONTEXT;
typedef VOID * XMEMCOMPRESSION_CONTEXT;

typedef enum _XMEMCODEC_TYPE
{
    XMEMCODEC_DEFAULT = 0,
    XMEMCODEC_LZX = 1
} XMEMCODEC_TYPE;

HRESULT XMemDecompress(
         XMEMDECOMPRESSION_CONTEXT Context,
         VOID *pDestination,
         SIZE_T *pDestSize,
         CONST VOID *pSource,
         SIZE_T SrcSize
);


HRESULT XMemCompress(
         XMEMCOMPRESSION_CONTEXT Context,
         VOID *pDestination,
         SIZE_T *pDestSize,
         CONST VOID *pSource,
         SIZE_T SrcSize
);

HRESULT XMemCreateCompressionContext(
         XMEMCODEC_TYPE CodecType,
         CONST VOID *pCodecParams,
         DWORD Flags,
         XMEMCOMPRESSION_CONTEXT *pContext
);

HRESULT XMemCreateDecompressionContext(
         XMEMCODEC_TYPE CodecType,
         CONST VOID *pCodecParams,
         DWORD Flags,
         XMEMDECOMPRESSION_CONTEXT *pContext
);

typedef struct _XMEMCODEC_PARAMETERS_LZX {
    DWORD Flags;
    DWORD WindowSize;
    DWORD CompressionPartitionSize;
} XMEMCODEC_PARAMETERS_LZX;

void XMemDestroyCompressionContext(XMEMCOMPRESSION_CONTEXT Context);
void XMemDestroyDecompressionContext(XMEMDECOMPRESSION_CONTEXT Context);
#endif

typedef struct {
    BYTE type;
    union {
        LONG nData;
        LONGLONG i64Data;
        double dblData;
        struct {
            DWORD cbData;
            LPWSTR pwszData;
        }string;
        float fData;
        struct {
            DWORD cbData;
            PBYTE pbData;
        }binary;
        FILETIME ftData;
    };
} XUSER_DATA, *PXUSER_DATA;

typedef struct {
    DWORD dwPropertyId;
    XUSER_DATA value;
} XUSER_PROPERTY, *PXUSER_PROPERTY;

// these need to match apwstrLocaleCode
// const int XC_LANGUAGE_ENGLISH		=1;
// const int XC_LANGUAGE_JAPANESE		=2;
// const int XC_LANGUAGE_GERMAN		=3;
// const int XC_LANGUAGE_FRENCH		=4;
// const int XC_LANGUAGE_SPANISH		=5;
// const int XC_LANGUAGE_ITALIAN		=6;
// const int XC_LANGUAGE_KOREAN		=7;
// const int XC_LANGUAGE_TCHINESE		=8;
// const int XC_LANGUAGE_PORTUGUESE	=9;
// const int XC_LANGUAGE_BRAZILIAN		=10;
// #if defined __PS3__ || defined __PSVITA__ || defined __ORBIS__
// const int XC_LANGUAGE_RUSSIAN		=11;
// // more PS3
// const int XC_LANGUAGE_DUTCH			=12;
// const int XC_LANGUAGE_FINISH		=13;
// const int XC_LANGUAGE_SWEDISH		=14;
// const int XC_LANGUAGE_DANISH		=15;
// const int XC_LANGUAGE_NORWEGIAN		=16;
// const int XC_LANGUAGE_POLISH		=17;
// const int XC_LANGUAGE_TURKISH		=18;
// const int XC_LANGUAGE_LATINAMERICANSPANISH		=19;
// 
// const int XC_LANGUAGE_GREEK			=20;
// #else
// const int XC_LANGUAGE_UKENGLISH	=11;
// const int XC_LANGUAGE_MEXICANSPANISH=12;
// #endif

// matching Xbox 360
const int XC_LANGUAGE_ENGLISH             =0x01;
const int XC_LANGUAGE_JAPANESE            =0x02;
const int XC_LANGUAGE_GERMAN              =0x03;
const int XC_LANGUAGE_FRENCH              =0x04;
const int XC_LANGUAGE_SPANISH             =0x05;
const int XC_LANGUAGE_ITALIAN             =0x06;
const int XC_LANGUAGE_KOREAN              =0x07;
const int XC_LANGUAGE_TCHINESE            =0x08;
const int XC_LANGUAGE_PORTUGUESE          =0x09;
const int XC_LANGUAGE_POLISH              =0x0B;
const int XC_LANGUAGE_RUSSIAN             =0x0C;
const int XC_LANGUAGE_SWEDISH             =0x0D;
const int XC_LANGUAGE_TURKISH             =0x0E;
const int XC_LANGUAGE_BNORWEGIAN          =0x0F;
const int XC_LANGUAGE_DUTCH               =0x10;
const int XC_LANGUAGE_SCHINESE            =0x11;

// for Sony
// const int XC_LANGUAGE_LATINAMERICANSPANISH	=0xF0; // 4J-JEV: Now differentiated via XC_LOCALE_LATIN_AMERICA
const int XC_LANGUAGE_FINISH				=0xF1;
const int XC_LANGUAGE_GREEK					=0xF2;
const int XC_LANGUAGE_DANISH				=0xF3;

// for Xbox One
const int XC_LANGUAGE_CZECH					=0xF4;
const int XC_LANGUAGE_SLOVAK				=0xF5;

const int XC_LOCALE_AUSTRALIA             =1;
const int XC_LOCALE_AUSTRIA               =2;
const int XC_LOCALE_BELGIUM               =3;
const int XC_LOCALE_BRAZIL                =4;
const int XC_LOCALE_CANADA                =5;
const int XC_LOCALE_CHILE                 =6;
const int XC_LOCALE_CHINA                 =7;
const int XC_LOCALE_COLOMBIA              =8;
const int XC_LOCALE_CZECH_REPUBLIC        =9;
const int XC_LOCALE_DENMARK               =10;
const int XC_LOCALE_FINLAND               =11;
const int XC_LOCALE_FRANCE                =12;
const int XC_LOCALE_GERMANY               =13;
const int XC_LOCALE_GREECE                =14;
const int XC_LOCALE_HONG_KONG             =15;
const int XC_LOCALE_HUNGARY               =16;
const int XC_LOCALE_INDIA                 =17;
const int XC_LOCALE_IRELAND               =18;
const int XC_LOCALE_ITALY                 =19;
const int XC_LOCALE_JAPAN                 =20;
const int XC_LOCALE_KOREA                 =21;
const int XC_LOCALE_MEXICO                =22;
const int XC_LOCALE_NETHERLANDS           =23;
const int XC_LOCALE_NEW_ZEALAND           =24;
const int XC_LOCALE_NORWAY                =25;
const int XC_LOCALE_POLAND                =26;
const int XC_LOCALE_PORTUGAL              =27;
const int XC_LOCALE_SINGAPORE             =28;
const int XC_LOCALE_SLOVAK_REPUBLIC       =29;
const int XC_LOCALE_SOUTH_AFRICA          =30;
const int XC_LOCALE_SPAIN                 =31;
const int XC_LOCALE_SWEDEN                =32;
const int XC_LOCALE_SWITZERLAND           =33;
const int XC_LOCALE_TAIWAN                =34;
const int XC_LOCALE_GREAT_BRITAIN         =35;
const int XC_LOCALE_UNITED_STATES         =36;
const int XC_LOCALE_RUSSIAN_FEDERATION    =37;
const int XC_LOCALE_WORLD_WIDE            =38;
const int XC_LOCALE_TURKEY                =39;
const int XC_LOCALE_ARGENTINA             =40;
const int XC_LOCALE_SAUDI_ARABIA          =41;
const int XC_LOCALE_ISRAEL                =42;
const int XC_LOCALE_UNITED_ARAB_EMIRATES  =43;

// for Sony
const int XC_LOCALE_LATIN_AMERICA		=240;



#if !(defined _DURANGO || defined __PS3__ || defined __ORBIS__ || defined __PSVITA__)
DWORD XGetLanguage();
DWORD XGetLocale();
DWORD XEnableGuestSignin(BOOL fEnable);
#endif

class D3DXVECTOR3
{
public:
	D3DXVECTOR3();
	D3DXVECTOR3(float,float,float);
	float x,y,z,pad;
	D3DXVECTOR3& operator += ( CONST D3DXVECTOR3& add );
};

#define QNET_E_SESSION_FULL 0
#define QNET_USER_MASK_USER0 1
#define QNET_USER_MASK_USER1 2
#define QNET_USER_MASK_USER2 4
#define QNET_USER_MASK_USER3 8
