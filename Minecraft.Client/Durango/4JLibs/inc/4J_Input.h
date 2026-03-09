#pragma once

#define	MAP_STYLE_0		0
#define	MAP_STYLE_1		1
#define	MAP_STYLE_2		2

#define	_360_JOY_BUTTON_A						0x00000001					
#define	_360_JOY_BUTTON_B						0x00000002
#define	_360_JOY_BUTTON_X						0x00000004
#define	_360_JOY_BUTTON_Y						0x00000008

#define	_360_JOY_BUTTON_START					0x00000010
#define	_360_JOY_BUTTON_BACK					0x00000020
#define	_360_JOY_BUTTON_RB						0x00000040
#define	_360_JOY_BUTTON_LB						0x00000080

#define	_360_JOY_BUTTON_RTHUMB					0x00000100
#define	_360_JOY_BUTTON_LTHUMB					0x00000200
#define	_360_JOY_BUTTON_DPAD_UP					0x00000400
#define	_360_JOY_BUTTON_DPAD_DOWN				0x00000800

#define	_360_JOY_BUTTON_DPAD_LEFT				0x00001000
#define	_360_JOY_BUTTON_DPAD_RIGHT				0x00002000
// fake digital versions of analog values				
#define	_360_JOY_BUTTON_LSTICK_RIGHT			0x00004000
#define	_360_JOY_BUTTON_LSTICK_LEFT				0x00008000

#define	_360_JOY_BUTTON_RSTICK_DOWN				0x00010000
#define	_360_JOY_BUTTON_RSTICK_UP				0x00020000
#define	_360_JOY_BUTTON_RSTICK_RIGHT			0x00040000
#define	_360_JOY_BUTTON_RSTICK_LEFT				0x00080000

#define	_360_JOY_BUTTON_LSTICK_DOWN				0x00100000
#define	_360_JOY_BUTTON_LSTICK_UP				0x00200000
#define	_360_JOY_BUTTON_RT						0x00400000
#define	_360_JOY_BUTTON_LT						0x00800000

#define _360_GTC_PLAY							0x01000000
#define _360_GTC_PAUSE							0x02000000
#define _360_GTC_MENU							0x04000000
#define _360_GTC_VIEW							0x08000000
#define _360_GTC_BACK							0x10000000

// Stick axis maps - to allow changes for SouthPaw in-game axis mapping
#define AXIS_MAP_LX								0
#define AXIS_MAP_LY								1
#define AXIS_MAP_RX								2
#define AXIS_MAP_RY								3

// Trigger map - to allow for swap triggers in-game
#define TRIGGER_MAP_0							0
#define TRIGGER_MAP_1							1

typedef struct _STRING_VERIFY_RESPONSE
{
	WORD wNumStrings;
	HRESULT *pStringResult;
} 
STRING_VERIFY_RESPONSE;

class C4JStringTable
{
public:
	LPCWSTR Lookup(LPCWSTR szId) {return nullptr;}
	LPCWSTR Lookup(UINT nIndex) {return nullptr;}
	void Clear();
	HRESULT Load(LPCWSTR szId) {return S_OK;}
};

class C_4JInput
{
public:
	static const int MAX_GAMEPADS = 8;
	enum EKeyboardResult
	{
		EKeyboard_Pending,
		EKeyboard_Cancelled,
		EKeyboard_ResultAccept,
		EKeyboard_ResultDecline,
	};

	enum EKeyboardMode
	{
		EKeyboardMode_Default,
		EKeyboardMode_Numeric,
		EKeyboardMode_Password,
		EKeyboardMode_Alphabet,
		EKeyboardMode_Full,
		EKeyboardMode_Alphabet_Extended,
		EKeyboardMode_IP_Address,
		EKeyboardMode_Phone,
		EKeyboardMode_URL,
		EKeyboardMode_Email

	};

	void				Initialise( int iInputStateC, unsigned char ucMapC,unsigned char ucActionC, unsigned char ucMenuActionC );
	void				Tick(void);
	void				SetDeadzoneAndMovementRange(unsigned int uiDeadzoneAnalog, unsigned int uiDeadzoneDigital ,unsigned int uiAnalogMovementRangeMax );
	void				SetGameJoypadMaps(unsigned char ucMap,unsigned char ucAction,unsigned int uiActionVal);
	unsigned int		GetGameJoypadMaps(unsigned char ucMap,unsigned char ucAction);
	void				SetJoypadMapVal(int iPad,unsigned char ucMap);
	unsigned char		GetJoypadMapVal(int iPad);
	void				SetJoypadSensitivity(int iPad, float fSensitivity);
	unsigned int		GetValue(int iPad,unsigned char ucAction, bool bRepeat=false);
	bool				ButtonPressed(int iPad,unsigned char ucAction=255); // toggled
	bool				ButtonReleased(int iPad,unsigned char ucAction); //toggled
	bool				ButtonDown(int iPad,unsigned char ucAction=255); // button held down
	// Functions to remap the axis and triggers for in-game (not menus) - SouthPaw, etc
	void				SetJoypadStickAxisMap(int iPad,unsigned int uiFrom, unsigned int uiTo);
	void				SetJoypadStickTriggerMap(int iPad,unsigned int uiFrom, unsigned int uiTo);
	void				SetKeyRepeatRate(float fRepeatDelaySecs,float fRepeatRateSecs); 
	void				SetDebugSequence( const char *chSequenceA,int( *Func)(LPVOID),LPVOID lpParam );
	FLOAT				GetIdleSeconds(int iPad);
	unsigned int		GetConnectedGamepadCount();
	bool				IsPadConnected(int iPad);
	bool				IsPadLocked(int iPad);

	// In-Game values which may have been remapped due to Southpaw, swap triggers, etc
	float				GetJoypadStick_LX(int iPad, bool bCheckMenuDisplay=true);
	float				GetJoypadStick_LY(int iPad, bool bCheckMenuDisplay=true);
	float				GetJoypadStick_RX(int iPad, bool bCheckMenuDisplay=true);
	float				GetJoypadStick_RY(int iPad, bool bCheckMenuDisplay=true);
	unsigned char		GetJoypadLTrigger(int iPad, bool bCheckMenuDisplay=true);
	unsigned char		GetJoypadRTrigger(int iPad, bool bCheckMenuDisplay=true);

	void				SetMenuDisplayed(int iPad, bool bVal);

// 	EKeyboardResult		RequestKeyboard(UINT uiTitle, UINT uiText, UINT uiDesc, DWORD dwPad, WCHAR *pwchResult, UINT uiResultSize,int( *Func)(LPVOID,const bool),LPVOID lpParam,EKeyboardMode eMode,C4JStringTable *pStringTable=nullptr);
// 	EKeyboardResult		RequestKeyboard(UINT uiTitle, LPCWSTR pwchDefault, UINT uiDesc, DWORD dwPad, WCHAR *pwchResult, UINT uiResultSize,int( *Func)(LPVOID,const bool),LPVOID lpParam, EKeyboardMode eMode,C4JStringTable *pStringTable=nullptr);
	EKeyboardResult		RequestKeyboard(LPCWSTR Title, LPCWSTR Text, DWORD dwPad, int iMaxChars, int( *Func)(LPVOID,const bool),LPVOID lpParam,C_4JInput::EKeyboardMode eMode);
	void				DestroyKeyboard();

	bool				IsCircleCrossSwapped();
	void				GetText(uint16_t *UTF16String);
	// Online check strings against offensive list - TCR 92
	// 	TCR # 092  CMTV Player Text String Verification 
	// 		Requirement Any player-entered text visible to another player on Xbox LIVE must be verified using the Xbox LIVE service before being transmitted. Text that is rejected by the Xbox LIVE service must not be displayed.
	// 
	// 		Remarks
	// 		This requirement applies to any player-entered string that can be exposed to other players on Xbox LIVE. It includes session names, content descriptions, text messages, tags, team names, mottos, comments, and so on.
	// 
	// 		Games may decide to not send the text, blank it out, or use generic text if the text was rejected by the Xbox LIVE service.
	// 
	// 		Games verify the text by calling the XStringVerify function.
	// 
	// 		Exemption It is not required to use the Xbox LIVE service to verify real-time text communication. An example of real-time text communication is in-game text chat.
	// 
	// 		Intent Protect players from inappropriate language.
	bool VerifyStrings(WCHAR **pwStringA,int iStringC,int( *Func)(LPVOID,STRING_VERIFY_RESPONSE *),LPVOID lpParam);
	void CancelQueuedVerifyStrings(int( *Func)(LPVOID,STRING_VERIFY_RESPONSE *),LPVOID lpParam);
	void CancelAllVerifyInProgress(void);

	//bool InputDetected(DWORD dwUserIndex,WCHAR *pwchInput);

	void SetEnabledGtcButtons(long long llEnabledButtons);

	Windows::Xbox::System::User^ GetUserForGamepad(int iPad);

	// Handle PLM events
	void Resume();
};

// Singleton
extern C_4JInput InputManager;
