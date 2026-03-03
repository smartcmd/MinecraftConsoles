#include "stdafx.h"
#ifndef __PS3__
//#include <compressapi.h>
#endif // __PS3__

#ifdef __PS3__
#include "PS3\Sentient\SentientManager.h"
#include "StatsCounter.h"
#include "PS3\Social\SocialManager.h"
#include <libsn.h>
#include <libsntuner.h>
#elif defined _DURANGO
#include "Durango\Sentient\SentientManager.h"
#include "StatsCounter.h"
#include "Durango\Social\SocialManager.h"
#include "Durango\Sentient\DynamicConfigurations.h"
#include "Durango\DurangoExtras\xcompress.h"
#elif defined _WINDOWS64
#include "Windows64\Sentient\SentientManager.h"
#include "StatsCounter.h"
#include "Windows64\Social\SocialManager.h"
#include "Windows64\Sentient\DynamicConfigurations.h"
#include "Windows64\Network\WinsockNetLayer.h"
#elif defined __PSVITA__
#include "PSVita\Sentient\SentientManager.h"
#include "StatsCounter.h"
#include "PSVita\Social\SocialManager.h"
#include "PSVita\Sentient\DynamicConfigurations.h"
#include <libperf.h>
#else
#include "Orbis\Sentient\SentientManager.h"
#include "StatsCounter.h"
#include "Orbis\Social\SocialManager.h"
#include "Orbis\Sentient\DynamicConfigurations.h"
#include <perf.h>
#endif

#if !defined(__PS3__) && !defined(__ORBIS__) && !defined(__PSVITA__)
#ifdef _WINDOWS64
//C4JStorage StorageManager;
C_4JProfile ProfileManager;
#endif
#endif // __PS3__
CSentientManager SentientManager;
CXuiStringTable StringTable;

#ifndef _XBOX_ONE
ATG::XMLParser::XMLParser() {}
ATG::XMLParser::~XMLParser() {}
HRESULT    ATG::XMLParser::ParseXMLBuffer(CONST CHAR* strBuffer, UINT uBufferSize) { return S_OK; }
VOID ATG::XMLParser::RegisterSAXCallbackInterface(ISAXCallback* pISAXCallback) {}
#endif

bool	CSocialManager::IsTitleAllowedToPostAnything() { return false; }
bool	CSocialManager::AreAllUsersAllowedToPostImages() { return false; }
bool	CSocialManager::IsTitleAllowedToPostImages() { return false; }

bool	CSocialManager::PostLinkToSocialNetwork(ESocialNetwork eSocialNetwork, DWORD dwUserIndex, bool bUsingKinect) { return false; }
bool	CSocialManager::PostImageToSocialNetwork(ESocialNetwork eSocialNetwork, DWORD dwUserIndex, bool bUsingKinect) { return false; }
CSocialManager* CSocialManager::Instance() { return NULL; }
void CSocialManager::SetSocialPostText(LPCWSTR Title, LPCWSTR Caption, LPCWSTR Desc) {};

DWORD XShowPartyUI(DWORD dwUserIndex) { return 0; }
DWORD XShowFriendsUI(DWORD dwUserIndex) { return 0; }
HRESULT XPartyGetUserList(XPARTY_USER_LIST* pUserList) { return S_OK; }
DWORD XContentGetThumbnail(DWORD dwUserIndex, const XCONTENT_DATA* pContentData, PBYTE pbThumbnail, PDWORD pcbThumbnail, PXOVERLAPPED* pOverlapped) { return 0; }
void XShowAchievementsUI(int i) {}
DWORD XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE Mode) { return 0; }

#ifndef _DURANGO
void PIXAddNamedCounter(int a, char* b, ...) {}
//#define PS3_USE_PIX_EVENTS 
//#define PS4_USE_PIX_EVENTS 
void PIXBeginNamedEvent(int a, char* b, ...)
{
#ifdef PS4_USE_PIX_EVENTS
	char buf[512];
	va_list args;
	va_start(args, b);
	vsprintf(buf, b, args);
	sceRazorCpuPushMarker(buf, 0xffffffff, SCE_RAZOR_MARKER_ENABLE_HUD);

#endif
#ifdef PS3_USE_PIX_EVENTS
	char buf[256];
	wchar_t wbuf[256];
	va_list args;
	va_start(args, b);
	vsprintf(buf, b, args);
	snPushMarker(buf);

	// 	mbstowcs(wbuf,buf,256);
	// 	RenderManager.BeginEvent(wbuf);
	va_end(args);
#endif
}
#if 0//__PSVITA__
if (PixDepth < 64)
{
	char buf[512];
	va_list args;
	va_start(args, b);
	vsprintf(buf, b, args);
	sceRazorCpuPushMarkerWithHud(buf, 0xffffffff, SCE_RAZOR_MARKER_ENABLE_HUD);
}
PixDepth += 1;
#endif


void PIXEndNamedEvent()
{
#ifdef PS4_USE_PIX_EVENTS
	sceRazorCpuPopMarker();
#endif
#ifdef PS3_USE_PIX_EVENTS
	snPopMarker();
	// 	RenderManager.EndEvent();
#endif
#if 0//__PSVITA__
	if (PixDepth <= 64)
	{
		sceRazorCpuPopMarker();
	}
	PixDepth -= 1;
#endif
}
void PIXSetMarkerDeprecated(int a, char* b, ...) {}
#else
// 4J Stu - Removed this implementation in favour of a macro that will convert our string format
// conversion at compile time rather than at runtime
//void PIXBeginNamedEvent(int a, char *b, ...)
//{
//	char buf[256];
//	wchar_t wbuf[256];
//	va_list args;
//	va_start(args,b);
//	vsprintf(buf,b,args);
//
//	mbstowcs(wbuf,buf,256);
//	PIXBeginEvent(a,wbuf);
//}
//
//void PIXEndNamedEvent()
//{
//	PIXEndEvent();
//}
//
//void PIXSetMarkerDeprecated(int a, char *b, ...)
//{
//	char buf[256];
//	wchar_t wbuf[256];
//	va_list args;
//	va_start(args,b);
//	vsprintf(buf,b,args);
//
//	mbstowcs(wbuf,buf,256);
//	PIXSetMarker(a, wbuf);
//}
#endif

// void *D3DXBUFFER::GetBufferPointer() { return NULL; }
// int D3DXBUFFER::GetBufferSize() { return 0; }
// void D3DXBUFFER::Release() {}

// #ifdef _DURANGO
// void GetLocalTime(SYSTEMTIME *time) {}
// #endif


bool IsEqualXUID(PlayerUID a, PlayerUID b)
{
#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__) || defined(_DURANGO)
	return (a == b);
#else
	return false;
#endif
}

void XMemCpy(void* a, const void* b, size_t s) { memcpy(a, b, s); }
void XMemSet(void* a, int t, size_t s) { memset(a, t, s); }
void XMemSet128(void* a, int t, size_t s) { memset(a, t, s); }
void* XPhysicalAlloc(SIZE_T a, ULONG_PTR  b, ULONG_PTR c, DWORD d) { return malloc(a); }
void XPhysicalFree(void* a) { free(a); }

D3DXVECTOR3::D3DXVECTOR3() {}
D3DXVECTOR3::D3DXVECTOR3(float x, float y, float z) : x(x), y(y), z(z) {}
D3DXVECTOR3& D3DXVECTOR3::operator += (CONST D3DXVECTOR3 & add) { x += add.x; y += add.y; z += add.z; return *this; }

BYTE IQNetPlayer::GetSmallId() { return m_smallId; }
void IQNetPlayer::SendData(IQNetPlayer * player, const void* pvData, DWORD dwDataSize, DWORD dwFlags)
{
	if (WinsockNetLayer::IsActive())
	{
		WinsockNetLayer::SendToSmallId(player->m_smallId, pvData, dwDataSize);
	}
}
bool IQNetPlayer::IsSameSystem(IQNetPlayer * player) { return (this == player) || (!m_isRemote && !player->m_isRemote); }
DWORD IQNetPlayer::GetSendQueueSize(IQNetPlayer * player, DWORD dwFlags) { return 0; }
DWORD IQNetPlayer::GetCurrentRtt() { return 0; }
bool IQNetPlayer::IsHost() { return m_isHostPlayer; }
bool IQNetPlayer::IsGuest() { return false; }
bool IQNetPlayer::IsLocal() { return !m_isRemote; }
PlayerUID IQNetPlayer::GetXuid() { return (PlayerUID)(0xe000d45248242f2e + m_smallId); } // todo: restore to INVALID_XUID once saves support this
LPCWSTR IQNetPlayer::GetGamertag() { return m_gamertag; }
int IQNetPlayer::GetSessionIndex() { return m_smallId; }
bool IQNetPlayer::IsTalking() { return false; }
bool IQNetPlayer::IsMutedByLocalUser(DWORD dwUserIndex) { return false; }
bool IQNetPlayer::HasVoice() { return false; }
bool IQNetPlayer::HasCamera() { return false; }
int IQNetPlayer::GetUserIndex() { return this - &IQNet::m_player[0]; }
void IQNetPlayer::SetCustomDataValue(ULONG_PTR ulpCustomDataValue) {
	m_customData = ulpCustomDataValue;
}
ULONG_PTR IQNetPlayer::GetCustomDataValue() {
	return m_customData;
}

IQNetPlayer IQNet::m_player[MINECRAFT_NET_MAX_PLAYERS];
DWORD IQNet::s_playerCount = 1;
bool IQNet::s_isHosting = true;

QNET_STATE _iQNetStubState = QNET_STATE_IDLE;

void Win64_SetupRemoteQNetPlayer(IQNetPlayer * player, BYTE smallId, bool isHost, bool isLocal)
{
	player->m_smallId = smallId;
	player->m_isRemote = !isLocal;
	player->m_isHostPlayer = isHost;
	swprintf_s(player->m_gamertag, 32, L"Player%d", smallId);
	if (smallId >= IQNet::s_playerCount)
		IQNet::s_playerCount = smallId + 1;
}

static bool Win64_IsActivePlayer(IQNetPlayer* p, DWORD index);

HRESULT IQNet::AddLocalPlayerByUserIndex(DWORD dwUserIndex) { return S_OK; }
IQNetPlayer* IQNet::GetHostPlayer() { return &m_player[0]; }
IQNetPlayer* IQNet::GetLocalPlayerByUserIndex(DWORD dwUserIndex)
{
	if (s_isHosting)
	{
		if (dwUserIndex < MINECRAFT_NET_MAX_PLAYERS &&
			!m_player[dwUserIndex].m_isRemote &&
			Win64_IsActivePlayer(&m_player[dwUserIndex], dwUserIndex))
			return &m_player[dwUserIndex];
		return NULL;
	}
	if (dwUserIndex != 0)
		return NULL;
	for (DWORD i = 0; i < s_playerCount; i++)
	{
		if (!m_player[i].m_isRemote && Win64_IsActivePlayer(&m_player[i], i))
			return &m_player[i];
	}
	return NULL;
}
static bool Win64_IsActivePlayer(IQNetPlayer * p, DWORD index)
{
	if (index == 0) return true;
	return (p->GetCustomDataValue() != 0);
}

IQNetPlayer* IQNet::GetPlayerByIndex(DWORD dwPlayerIndex)
{
	DWORD found = 0;
	for (DWORD i = 0; i < s_playerCount; i++)
	{
		if (Win64_IsActivePlayer(&m_player[i], i))
		{
			if (found == dwPlayerIndex) return &m_player[i];
			found++;
		}
	}
	return &m_player[0];
}
IQNetPlayer* IQNet::GetPlayerBySmallId(BYTE SmallId)
{
	for (DWORD i = 0; i < s_playerCount; i++)
	{
		if (m_player[i].m_smallId == SmallId && Win64_IsActivePlayer(&m_player[i], i)) return &m_player[i];
	}
	return NULL;
}
IQNetPlayer* IQNet::GetPlayerByXuid(PlayerUID xuid)
{
	for (DWORD i = 0; i < s_playerCount; i++)
	{
		if (Win64_IsActivePlayer(&m_player[i], i) && m_player[i].GetXuid() == xuid) return &m_player[i];
	}
	return &m_player[0];
}
DWORD IQNet::GetPlayerCount()
{
	DWORD count = 0;
	for (DWORD i = 0; i < s_playerCount; i++)
	{
		if (Win64_IsActivePlayer(&m_player[i], i)) count++;
	}
	return count;
}
QNET_STATE IQNet::GetState() { return _iQNetStubState; }
bool IQNet::IsHost() { return s_isHosting; }
HRESULT IQNet::JoinGameFromInviteInfo(DWORD dwUserIndex, DWORD dwUserMask, const INVITE_INFO * pInviteInfo) { return S_OK; }
void IQNet::HostGame() { _iQNetStubState = QNET_STATE_SESSION_STARTING; s_isHosting = true; }
void IQNet::ClientJoinGame()
{
	_iQNetStubState = QNET_STATE_SESSION_STARTING;
	s_isHosting = false;

	for (int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; i++)
	{
		m_player[i].m_smallId = (BYTE)i;
		m_player[i].m_isRemote = true;
		m_player[i].m_isHostPlayer = false;
		m_player[i].m_gamertag[0] = 0;
		m_player[i].SetCustomDataValue(0);
	}
}
void IQNet::EndGame()
{
	_iQNetStubState = QNET_STATE_IDLE;
	s_isHosting = false;
	s_playerCount = 1;
	for (int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; i++)
	{
		m_player[i].m_smallId = (BYTE)i;
		m_player[i].m_isRemote = false;
		m_player[i].m_isHostPlayer = false;
		m_player[i].m_gamertag[0] = 0;
		m_player[i].SetCustomDataValue(0);
	}
}

DWORD MinecraftDynamicConfigurations::GetTrialTime() { return DYNAMIC_CONFIG_DEFAULT_TRIAL_TIME; }

void XSetThreadProcessor(HANDLE a, int b) {}
// #if !(defined __PS3__) && !(defined __ORBIS__)
// BOOL XCloseHandle(HANDLE a) { return CloseHandle(a); }
// #endif // __PS3__

DWORD XUserGetSigninInfo(
	DWORD dwUserIndex,
	DWORD dwFlags,
	PXUSER_SIGNIN_INFO pSigninInfo
)
{
	return 0;
}

LPCWSTR CXuiStringTable::Lookup(LPCWSTR szId) { return szId; }
LPCWSTR CXuiStringTable::Lookup(UINT nIndex) { return L"String"; }
void CXuiStringTable::Clear() {}
HRESULT CXuiStringTable::Load(LPCWSTR szId) { return S_OK; }

DWORD XUserAreUsersFriends(DWORD dwUserIndex, PPlayerUID pXuids, DWORD dwXuidCount, PBOOL pfResult, void* pOverlapped) { return 0; }

#if defined __ORBIS__ || defined __PS3__ || defined _XBOX_ONE
#else
HRESULT XMemDecompress(
	XMEMDECOMPRESSION_CONTEXT Context,
	VOID * pDestination,
	SIZE_T * pDestSize,
	CONST VOID * pSource,
	SIZE_T SrcSize
)
{
	memcpy(pDestination, pSource, SrcSize);
	*pDestSize = SrcSize;
	return S_OK;

	/*
	DECOMPRESSOR_HANDLE Decompressor    = (DECOMPRESSOR_HANDLE)Context;
	if( Decompress(
		Decompressor,           //  Decompressor handle
		(void *)pSource,		//  Compressed data
		SrcSize,				//  Compressed data size
		pDestination,			//  Decompressed buffer
		*pDestSize,				//  Decompressed buffer size
		pDestSize) )				//  Decompressed data size
	{
		return S_OK;
	}
	else
	*/
	{
		return E_FAIL;
	}
}

HRESULT XMemCompress(
	XMEMCOMPRESSION_CONTEXT Context,
	VOID * pDestination,
	SIZE_T * pDestSize,
	CONST VOID * pSource,
	SIZE_T SrcSize
)
{
	memcpy(pDestination, pSource, SrcSize);
	*pDestSize = SrcSize;
	return S_OK;

	/*
	COMPRESSOR_HANDLE Compressor    = (COMPRESSOR_HANDLE)Context;
	if( Compress(
			Compressor,                  //  Compressor Handle
			(void *)pSource,             //  Input buffer, Uncompressed data
			SrcSize,					 //  Uncompressed data size
			pDestination,                //  Compressed Buffer
			*pDestSize,                  //  Compressed Buffer size
			pDestSize)	)				//  Compressed Data size
	{
		return S_OK;
	}
	else
	*/
	{
		return E_FAIL;
	}
}

HRESULT XMemCreateCompressionContext(
	XMEMCODEC_TYPE CodecType,
	CONST VOID * pCodecParams,
	DWORD Flags,
	XMEMCOMPRESSION_CONTEXT * pContext
)
{
	/*
	COMPRESSOR_HANDLE Compressor    = NULL;

	HRESULT hr = CreateCompressor(
		COMPRESS_ALGORITHM_XPRESS_HUFF, //  Compression Algorithm
		NULL,                           //  Optional allocation routine
		&Compressor);                   //  Handle

	pContext = (XMEMDECOMPRESSION_CONTEXT *)Compressor;
	return hr;
	*/
	return 0;
}

HRESULT XMemCreateDecompressionContext(
	XMEMCODEC_TYPE CodecType,
	CONST VOID * pCodecParams,
	DWORD Flags,
	XMEMDECOMPRESSION_CONTEXT * pContext
)
{
	/*
	DECOMPRESSOR_HANDLE  Decompressor    = NULL;

	HRESULT hr = CreateDecompressor(
		COMPRESS_ALGORITHM_XPRESS_HUFF, //  Compression Algorithm
		NULL,                           //  Optional allocation routine
		&Decompressor);                   //  Handle

	pContext = (XMEMDECOMPRESSION_CONTEXT *)Decompressor;
	return hr;
	*/
	return 0;
}

void XMemDestroyCompressionContext(XMEMCOMPRESSION_CONTEXT Context)
{
	//	COMPRESSOR_HANDLE Compressor    = (COMPRESSOR_HANDLE)Context;
	//	CloseCompressor(Compressor);
}

void XMemDestroyDecompressionContext(XMEMDECOMPRESSION_CONTEXT Context)
{
	//	DECOMPRESSOR_HANDLE Decompressor    = (DECOMPRESSOR_HANDLE)Context;
	//	CloseDecompressor(Decompressor);
}
#endif

//#ifndef __PS3__
#if !(defined _DURANGO || defined __PS3__ || defined __ORBIS__ || defined __PSVITA__)
DWORD XGetLanguage() { return 1; }
DWORD XGetLocale() { return 0; }
DWORD XEnableGuestSignin(BOOL fEnable) { return 0; }
#endif



/////////////////////////////////////////////// Profile library
#ifdef _WINDOWS64
static void* profileData[4];
static bool s_bProfileIsFullVersion;
void				C_4JProfile::Initialise(DWORD dwTitleID,
	DWORD dwOfferID,
	unsigned short usProfileVersion,
	UINT uiProfileValuesC,
	UINT uiProfileSettingsC,
	DWORD * pdwProfileSettingsA,
	int iGameDefinedDataSizeX4,
	unsigned int* puiGameDefinedDataChangedBitmask)
{
	for (int i = 0; i < 4; i++)
	{
		profileData[i] = new byte[iGameDefinedDataSizeX4 / 4];
		ZeroMemory(profileData[i], sizeof(byte) * iGameDefinedDataSizeX4 / 4);

		// Set some sane initial values!
		GAME_SETTINGS* pGameSettings = (GAME_SETTINGS*)profileData[i];
		pGameSettings->ucMenuSensitivity = 100; //eGameSetting_Sensitivity_InMenu
		pGameSettings->ucInterfaceOpacity = 80; //eGameSetting_Sensitivity_InMenu
		pGameSettings->usBitmaskValues |= 0x0200; //eGameSetting_DisplaySplitscreenGamertags - on
		pGameSettings->usBitmaskValues |= 0x0400; //eGameSetting_Hints - on
		pGameSettings->usBitmaskValues |= 0x1000; //eGameSetting_Autosave - 2
		pGameSettings->usBitmaskValues |= 0x8000; //eGameSetting_Tooltips - on
		pGameSettings->uiBitmaskValues = 0L; // reset
		pGameSettings->uiBitmaskValues |= GAMESETTING_CLOUDS;					//eGameSetting_Clouds - on
		pGameSettings->uiBitmaskValues |= GAMESETTING_ONLINE;					//eGameSetting_GameSetting_Online - on
		pGameSettings->uiBitmaskValues |= GAMESETTING_FRIENDSOFFRIENDS;		//eGameSetting_GameSetting_FriendsOfFriends - on
		pGameSettings->uiBitmaskValues |= GAMESETTING_DISPLAYUPDATEMSG;		//eGameSetting_DisplayUpdateMessage (counter)
		pGameSettings->uiBitmaskValues &= ~GAMESETTING_BEDROCKFOG;			//eGameSetting_BedrockFog - off
		pGameSettings->uiBitmaskValues |= GAMESETTING_DISPLAYHUD;				//eGameSetting_DisplayHUD - on
		pGameSettings->uiBitmaskValues |= GAMESETTING_DISPLAYHAND;			//eGameSetting_DisplayHand - on
		pGameSettings->uiBitmaskValues |= GAMESETTING_CUSTOMSKINANIM;			//eGameSetting_CustomSkinAnim - on
		pGameSettings->uiBitmaskValues |= GAMESETTING_DEATHMESSAGES;			//eGameSetting_DeathMessages - on
		pGameSettings->uiBitmaskValues |= (GAMESETTING_UISIZE & 0x00000800);				// uisize 2
		pGameSettings->uiBitmaskValues |= (GAMESETTING_UISIZE_SPLITSCREEN & 0x00004000);	// splitscreen ui size 3
		pGameSettings->uiBitmaskValues |= GAMESETTING_ANIMATEDCHARACTER;		//eGameSetting_AnimatedCharacter - on

		// TU12
		// favorite skins added, but only set in TU12 - set to FFs
		for (int i = 0; i < MAX_FAVORITE_SKINS; i++)
		{
			pGameSettings->uiFavoriteSkinA[i] = 0xFFFFFFFF;
		}
		pGameSettings->ucCurrentFavoriteSkinPos = 0;
		// Added a bitmask in TU13 to enable/disable display of the Mash-up pack worlds in the saves list
		pGameSettings->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;

		// PS3DEC13
		pGameSettings->uiBitmaskValues &= ~GAMESETTING_PS3EULAREAD;		//eGameSetting_PS3_EULA_Read - off

		// PS3 1.05 - added Greek
		pGameSettings->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT; // use the system language

		// PS Vita - network mode added
		pGameSettings->uiBitmaskValues &= ~GAMESETTING_PSVITANETWORKMODEADHOC;		//eGameSetting_PSVita_NetworkModeAdhoc - off


		// Tutorials for most menus, and a few other things
		pGameSettings->ucTutorialCompletion[0] = 0xFF;
		pGameSettings->ucTutorialCompletion[1] = 0xFF;
		pGameSettings->ucTutorialCompletion[2] = 0xF;

		// Has gone halfway through the tutorial
		pGameSettings->ucTutorialCompletion[28] |= 1 << 0;
	}
}
void				C_4JProfile::SetTrialTextStringTable(CXuiStringTable * pStringTable, int iAccept, int iReject) {}
void				C_4JProfile::SetTrialAwardText(eAwardType AwardType, int iTitle, int iText) {}
int					C_4JProfile::GetLockedProfile() { return 0; }
void				C_4JProfile::SetLockedProfile(int iProf) {}
bool				C_4JProfile::IsSignedIn(int iQuadrant) { return (iQuadrant == 0); }
bool				C_4JProfile::IsSignedInLive(int iProf) { return true; }
bool				C_4JProfile::IsGuest(int iQuadrant) { return false; }
UINT				C_4JProfile::RequestSignInUI(bool bFromInvite, bool bLocalGame, bool bNoGuestsAllowed, bool bMultiplayerSignIn, bool bAddUser, int(*Func)(LPVOID, const bool, const int iPad), LPVOID lpParam, int iQuadrant) { return 0; }
UINT				C_4JProfile::DisplayOfflineProfile(int(*Func)(LPVOID, const bool, const int iPad), LPVOID lpParam, int iQuadrant) { return 0; }
UINT				C_4JProfile::RequestConvertOfflineToGuestUI(int(*Func)(LPVOID, const bool, const int iPad), LPVOID lpParam, int iQuadrant) { return 0; }
void				C_4JProfile::SetPrimaryPlayerChanged(bool bVal) {}
bool				C_4JProfile::QuerySigninStatus(void) { return true; }
void				C_4JProfile::GetXUID(int iPad, PlayerUID * pXuid, bool bOnlineXuid)
{
#ifdef _WINDOWS64
	if (iPad != 0)
	{
		*pXuid = INVALID_XUID;
		return;
	}
	if (IQNet::s_isHosting)
		*pXuid = 0xe000d45248242f2e;
	else
		*pXuid = 0xe000d45248242f2e + WinsockNetLayer::GetLocalSmallId();
#else
	* pXuid = 0xe000d45248242f2e + iPad;
#endif
}
BOOL				C_4JProfile::AreXUIDSEqual(PlayerUID xuid1, PlayerUID xuid2) { return xuid1 == xuid2; }
BOOL				C_4JProfile::XUIDIsGuest(PlayerUID xuid) { return false; }
bool				C_4JProfile::AllowedToPlayMultiplayer(int iProf) { return true; }

#if defined(__ORBIS__)
bool				C_4JProfile::GetChatAndContentRestrictions(int iPad, bool thisQuadrantOnly, bool* pbChatRestricted, bool* pbContentRestricted, int* piAge)
{
	if (pbChatRestricted) *pbChatRestricted = false;
	if (pbContentRestricted) *pbContentRestricted = false;
	if (piAge) *piAge = 100;
	return true;
}
#endif

void				C_4JProfile::StartTrialGame() {}
void				C_4JProfile::AllowedPlayerCreatedContent(int iPad, bool thisQuadrantOnly, BOOL * allAllowed, BOOL * friendsAllowed) {}
BOOL				C_4JProfile::CanViewPlayerCreatedContent(int iPad, bool thisQuadrantOnly, PPlayerUID pXuids, DWORD dwXuidCount) { return true; }
bool				C_4JProfile::GetProfileAvatar(int iPad, int(*Func)(LPVOID lpParam, PBYTE pbThumbnail, DWORD dwThumbnailBytes), LPVOID lpParam) { return false; }
void				C_4JProfile::CancelProfileAvatarRequest() {}
int					C_4JProfile::GetPrimaryPad() { return 0; }
void				C_4JProfile::SetPrimaryPad(int iPad) {}
#ifdef _DURANGO
char fakeGamerTag[32] = "PlayerName";
void				SetFakeGamertag(char* name) { strcpy_s(fakeGamerTag, name); }
#else
char* C_4JProfile::GetGamertag(int iPad) { extern char g_Win64Username[17]; return g_Win64Username; }
wstring				C_4JProfile::GetDisplayName(int iPad) { extern wchar_t g_Win64UsernameW[17]; return g_Win64UsernameW; }
#endif
bool				C_4JProfile::IsFullVersion() { return s_bProfileIsFullVersion; }
void				C_4JProfile::SetSignInChangeCallback(void (*Func)(LPVOID, bool, unsigned int), LPVOID lpParam) {}
void				C_4JProfile::SetNotificationsCallback(void (*Func)(LPVOID, DWORD, unsigned int), LPVOID lpParam) {}
bool				C_4JProfile::RegionIsNorthAmerica(void) { return false; }
bool				C_4JProfile::LocaleIsUSorCanada(void) { return false; }
HRESULT				C_4JProfile::GetLiveConnectionStatus() { return S_OK; }
bool				C_4JProfile::IsSystemUIDisplayed() { return false; }
void				C_4JProfile::SetProfileReadErrorCallback(void (*Func)(LPVOID), LPVOID lpParam) {}
int(*defaultOptionsCallback)(LPVOID, C_4JProfile::PROFILESETTINGS*, const int iPad) = NULL;
LPVOID lpProfileParam = NULL;
int					C_4JProfile::SetDefaultOptionsCallback(int(*Func)(LPVOID, PROFILESETTINGS*, const int iPad), LPVOID lpParam)
{
	defaultOptionsCallback = Func;
	lpProfileParam = lpParam;
	return 0;
}
int					C_4JProfile::SetOldProfileVersionCallback(int(*Func)(LPVOID, unsigned char*, const unsigned short, const int), LPVOID lpParam) { return 0; }

// To store the dashboard preferences for controller flipped, etc.
C_4JProfile::PROFILESETTINGS ProfileSettingsA[XUSER_MAX_COUNT];

C_4JProfile::PROFILESETTINGS* C_4JProfile::GetDashboardProfileSettings(int iPad) { return &ProfileSettingsA[iPad]; }
void				C_4JProfile::WriteToProfile(int iQuadrant, bool bGameDefinedDataChanged, bool bOverride5MinuteLimitOnProfileWrites) {}
void				C_4JProfile::ForceQueuedProfileWrites(int iPad) {}
void* C_4JProfile::GetGameDefinedProfileData(int iQuadrant)
{
	// 4J Stu - Don't reset the options when we call this!!
	//defaultOptionsCallback(lpProfileParam, (C_4JProfile::PROFILESETTINGS *)profileData[iQuadrant], iQuadrant);
	//pApp->SetDefaultOptions(pSettings,iPad);

	return profileData[iQuadrant];
}
void				C_4JProfile::ResetProfileProcessState() {}
void				C_4JProfile::Tick(void) {}
void				C_4JProfile::RegisterAward(int iAwardNumber, int iGamerconfigID, eAwardType eType, bool bLeaderboardAffected,
	CXuiStringTable * pStringTable, int iTitleStr, int iTextStr, int iAcceptStr, char* pszThemeName, unsigned int ulThemeSize) {
}
int					C_4JProfile::GetAwardId(int iAwardNumber) { return 0; }
eAwardType			C_4JProfile::GetAwardType(int iAwardNumber) { return eAwardType_Achievement; }
bool				C_4JProfile::CanBeAwarded(int iQuadrant, int iAwardNumber) { return false; }
void				C_4JProfile::Award(int iQuadrant, int iAwardNumber, bool bForce) {}
bool				C_4JProfile::IsAwardsFlagSet(int iQuadrant, int iAward) { return false; }
void				C_4JProfile::RichPresenceInit(int iPresenceCount, int iContextCount) {}
void				C_4JProfile::RegisterRichPresenceContext(int iGameConfigContextID) {}
void				C_4JProfile::SetRichPresenceContextValue(int iPad, int iContextID, int iVal) {}
void				C_4JProfile::SetCurrentGameActivity(int iPad, int iNewPresence, bool bSetOthersToIdle) {}
void				C_4JProfile::DisplayFullVersionPurchase(bool bRequired, int iQuadrant, int iUpsellParam) {}
void				C_4JProfile::SetUpsellCallback(void (*Func)(LPVOID lpParam, eUpsellType type, eUpsellResponse response, int iUserData), LPVOID lpParam) {}
void				C_4JProfile::SetDebugFullOverride(bool bVal) { s_bProfileIsFullVersion = bVal; }
void				C_4JProfile::ShowProfileCard(int iPad, PlayerUID targetUid) {}

/////////////////////////////////////////////// Storage library
//#ifdef _WINDOWS64
#if 0
C4JStorage::C4JStorage() {}
void								C4JStorage::Tick() {}
C4JStorage::EMessageResult			C4JStorage::RequestMessageBox(UINT uiTitle, UINT uiText, UINT * uiOptionA, UINT uiOptionC, DWORD dwPad, int(*Func)(LPVOID, int, const C4JStorage::EMessageResult), LPVOID lpParam, C4JStringTable * pStringTable, WCHAR * pwchFormatString, DWORD dwFocusButton) { return C4JStorage::EMessage_Undefined; }
C4JStorage::EMessageResult			C4JStorage::GetMessageBoxResult() { return C4JStorage::EMessage_Undefined; }
bool								C4JStorage::SetSaveDevice(int(*Func)(LPVOID, const bool), LPVOID lpParam, bool bForceResetOfSaveDevice) { return true; }
void								C4JStorage::Init(LPCWSTR pwchDefaultSaveName, char* pszSavePackName, int iMinimumSaveSize, int(*Func)(LPVOID, const ESavingMessage, int), LPVOID lpParam) {}
void								C4JStorage::ResetSaveData() {}
void								C4JStorage::SetDefaultSaveNameForKeyboardDisplay(LPCWSTR pwchDefaultSaveName) {}
void								C4JStorage::SetSaveTitle(LPCWSTR pwchDefaultSaveName) {}
LPCWSTR								C4JStorage::GetSaveTitle() { return L""; }
bool								C4JStorage::GetSaveUniqueNumber(INT * piVal) { return true; }
bool								C4JStorage::GetSaveUniqueFilename(char* pszName) { return true; }
void								C4JStorage::SetSaveUniqueFilename(char* szFilename) {}
void								C4JStorage::SetState(ESaveGameControlState eControlState, int(*Func)(LPVOID, const bool), LPVOID lpParam) {}
void								C4JStorage::SetSaveDisabled(bool bDisable) {}
bool								C4JStorage::GetSaveDisabled(void) { return false; }
unsigned int						C4JStorage::GetSaveSize() { return 0; }
void								C4JStorage::GetSaveData(void* pvData, unsigned int* pulBytes) {}
PVOID								C4JStorage::AllocateSaveData(unsigned int ulBytes) { return new char[ulBytes]; }
void								C4JStorage::SaveSaveData(unsigned int ulBytes, PBYTE pbThumbnail, DWORD cbThumbnail, PBYTE pbTextData, DWORD dwTextLen) {}
void								C4JStorage::CopySaveDataToNewSave(PBYTE pbThumbnail, DWORD cbThumbnail, WCHAR * wchNewName, int (*Func)(LPVOID lpParam, bool), LPVOID lpParam) {}
void								C4JStorage::SetSaveDeviceSelected(unsigned int uiPad, bool bSelected) {}
bool								C4JStorage::GetSaveDeviceSelected(unsigned int iPad) { return true; }
C4JStorage::ELoadGameStatus			C4JStorage::DoesSaveExist(bool* pbExists) { return C4JStorage::ELoadGame_Idle; }
bool								C4JStorage::EnoughSpaceForAMinSaveGame() { return true; }
void								C4JStorage::SetSaveMessageVPosition(float fY) {}
//C4JStorage::ESGIStatus				C4JStorage::GetSavesInfo(int iPad,bool ( *Func)(LPVOID, int, CACHEINFOSTRUCT *, int, HRESULT),LPVOID lpParam,char *pszSavePackName) { return C4JStorage::ESGIStatus_Idle; }
C4JStorage::ESaveGameState			C4JStorage::GetSavesInfo(int iPad, int (*Func)(LPVOID lpParam, SAVE_DETAILS * pSaveDetails, const bool), LPVOID lpParam, char* pszSavePackName) { return C4JStorage::ESaveGame_Idle; }

void								C4JStorage::GetSaveCacheFileInfo(DWORD dwFile, XCONTENT_DATA & xContentData) {}
void								C4JStorage::GetSaveCacheFileInfo(DWORD dwFile, PBYTE * ppbImageData, DWORD * pdwImageBytes) {}
C4JStorage::ESaveGameState			C4JStorage::LoadSaveData(PSAVE_INFO pSaveInfo, int(*Func)(LPVOID lpParam, const bool, const bool), LPVOID lpParam) { return C4JStorage::ESaveGame_Idle; }
C4JStorage::EDeleteGameStatus		C4JStorage::DeleteSaveData(PSAVE_INFO pSaveInfo, int(*Func)(LPVOID lpParam, const bool), LPVOID lpParam) { return C4JStorage::EDeleteGame_Idle; }
PSAVE_DETAILS						C4JStorage::ReturnSavesInfo() { return NULL; }

void								C4JStorage::RegisterMarketplaceCountsCallback(int (*Func)(LPVOID lpParam, C4JStorage::DLC_TMS_DETAILS*, int), LPVOID lpParam) {}
void								C4JStorage::SetDLCPackageRoot(char* pszDLCRoot) {}
C4JStorage::EDLCStatus				C4JStorage::GetDLCOffers(int iPad, int(*Func)(LPVOID, int, DWORD, int), LPVOID lpParam, DWORD dwOfferTypesBitmaskT) { return C4JStorage::EDLC_Idle; }
DWORD								C4JStorage::CancelGetDLCOffers() { return 0; }
void								C4JStorage::ClearDLCOffers() {}
XMARKETPLACE_CONTENTOFFER_INFO& C4JStorage::GetOffer(DWORD dw) { static XMARKETPLACE_CONTENTOFFER_INFO retval = { 0 }; return retval; }
int									C4JStorage::GetOfferCount() { return 0; }
DWORD								C4JStorage::InstallOffer(int iOfferIDC, ULONGLONG * ullOfferIDA, int(*Func)(LPVOID, int, int), LPVOID lpParam, bool bTrial) { return 0; }
DWORD								C4JStorage::GetAvailableDLCCount(int iPad) { return 0; }
XCONTENT_DATA& C4JStorage::GetDLC(DWORD dw) { static XCONTENT_DATA retval = { 0 }; return retval; }
C4JStorage::EDLCStatus				C4JStorage::GetInstalledDLC(int iPad, int(*Func)(LPVOID, int, int), LPVOID lpParam) { return C4JStorage::EDLC_Idle; }
DWORD								C4JStorage::MountInstalledDLC(int iPad, DWORD dwDLC, int(*Func)(LPVOID, int, DWORD, DWORD), LPVOID lpParam, LPCSTR szMountDrive) { return 0; }
DWORD								C4JStorage::UnmountInstalledDLC(LPCSTR szMountDrive) { return 0; }
C4JStorage::ETMSStatus				C4JStorage::ReadTMSFile(int iQuadrant, eGlobalStorage eStorageFacility, C4JStorage::eTMS_FileType eFileType, WCHAR * pwchFilename, BYTE * *ppBuffer, DWORD * pdwBufferSize, int(*Func)(LPVOID, WCHAR*, int, bool, int), LPVOID lpParam, int iAction) { return C4JStorage::ETMSStatus_Idle; }
bool								C4JStorage::WriteTMSFile(int iQuadrant, eGlobalStorage eStorageFacility, WCHAR * pwchFilename, BYTE * pBuffer, DWORD dwBufferSize) { return true; }
bool								C4JStorage::DeleteTMSFile(int iQuadrant, eGlobalStorage eStorageFacility, WCHAR * pwchFilename) { return true; }
void								C4JStorage::StoreTMSPathName(WCHAR * pwchName) {}
unsigned int						C4JStorage::CRC(unsigned char* buf, int len) { return 0; }

struct PTMSPP_FILEDATA;
C4JStorage::ETMSStatus				C4JStorage::TMSPP_ReadFile(int iPad, C4JStorage::eGlobalStorage eStorageFacility, C4JStorage::eTMS_FILETYPEVAL eFileTypeVal, LPCSTR szFilename, int(*Func)(LPVOID, int, int, PTMSPP_FILEDATA, LPCSTR)/*=NULL*/, LPVOID lpParam/*=NULL*/, int iUserData/*=0*/) { return C4JStorage::ETMSStatus_Idle; }
#endif // _WINDOWS64

#endif // __PS3__

/////////////////////////////////////////////////////// Sentient manager

HRESULT CSentientManager::Init() { return S_OK; }
HRESULT CSentientManager::Tick() { return S_OK; }
HRESULT CSentientManager::Flush() { return S_OK; }
BOOL CSentientManager::RecordPlayerSessionStart(DWORD dwUserId) { return true; }
BOOL CSentientManager::RecordPlayerSessionExit(DWORD dwUserId, int exitStatus) { return true; }
BOOL CSentientManager::RecordHeartBeat(DWORD dwUserId) { return true; }
BOOL CSentientManager::RecordLevelStart(DWORD dwUserId, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, DWORD numberOfLocalPlayers, DWORD numberOfOnlinePlayers) { return true; }
BOOL CSentientManager::RecordLevelExit(DWORD dwUserId, ESen_LevelExitStatus levelExitStatus) { return true; }
BOOL CSentientManager::RecordLevelSaveOrCheckpoint(DWORD dwUserId, INT saveOrCheckPointID, INT saveSizeInBytes) { return true; }
BOOL CSentientManager::RecordLevelResume(DWORD dwUserId, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, DWORD numberOfLocalPlayers, DWORD numberOfOnlinePlayers, INT saveOrCheckPointID) { return true; }
BOOL CSentientManager::RecordPauseOrInactive(DWORD dwUserId) { return true; }
BOOL CSentientManager::RecordUnpauseOrActive(DWORD dwUserId) { return true; }
BOOL CSentientManager::RecordMenuShown(DWORD dwUserId, INT menuID, INT optionalMenuSubID) { return true; }
BOOL CSentientManager::RecordAchievementUnlocked(DWORD dwUserId, INT achievementID, INT achievementGamerscore) { return true; }
BOOL CSentientManager::RecordMediaShareUpload(DWORD dwUserId, ESen_MediaDestination mediaDestination, ESen_MediaType mediaType) { return true; }
BOOL CSentientManager::RecordUpsellPresented(DWORD dwUserId, ESen_UpsellID upsellId, INT marketplaceOfferID) { return true; }
BOOL CSentientManager::RecordUpsellResponded(DWORD dwUserId, ESen_UpsellID upsellId, INT marketplaceOfferID, ESen_UpsellOutcome upsellOutcome) { return true; }
BOOL CSentientManager::RecordPlayerDiedOrFailed(DWORD dwUserId, INT lowResMapX, INT lowResMapY, INT lowResMapZ, INT mapID, INT playerWeaponID, INT enemyWeaponID, ETelemetryChallenges enemyTypeID) { return true; }
BOOL CSentientManager::RecordEnemyKilledOrOvercome(DWORD dwUserId, INT lowResMapX, INT lowResMapY, INT lowResMapZ, INT mapID, INT playerWeaponID, INT enemyWeaponID, ETelemetryChallenges enemyTypeID) { return true; }
BOOL CSentientManager::RecordSkinChanged(DWORD dwUserId, DWORD dwSkinId) { return true; }
BOOL CSentientManager::RecordBanLevel(DWORD dwUserId) { return true; }
BOOL CSentientManager::RecordUnBanLevel(DWORD dwUserId) { return true; }
INT CSentientManager::GetMultiplayerInstanceID() { return 0; }
INT CSentientManager::GenerateMultiplayerInstanceId() { return 0; }
void CSentientManager::SetMultiplayerInstanceId(INT value) {}

////////////////////////////////////////////////////////  Stats counter

/*
StatsCounter::StatsCounter() {}
void StatsCounter::award(Stat *stat, unsigned int difficulty, unsigned int count) {}
bool StatsCounter::hasTaken(Achievement *ach) { return true; }
bool StatsCounter::canTake(Achievement *ach) { return true; }
unsigned int StatsCounter::getValue(Stat *stat, unsigned int difficulty) { return 0; }
unsigned int StatsCounter::getTotalValue(Stat *stat) { return 0; }
void StatsCounter::tick(int player) {}
void StatsCounter::parse(void* data) {}
void StatsCounter::clear() {}
void StatsCounter::save(int player, bool force) {}
void StatsCounter::flushLeaderboards() {}
void StatsCounter::saveLeaderboards() {}
void StatsCounter::setupStatBoards() {}
#ifdef _DEBUG
void StatsCounter::WipeLeaderboards() {}
#endif
*/
