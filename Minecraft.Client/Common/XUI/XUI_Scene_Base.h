#pragma once

#include "../media/xuiscene_base.h"
#include "XUI_Ctrl_SlotItem.h"
#include "XUI_CustomMessages.h"
#include "../UI/UIEnums.h"
#include "../../../Minecraft.World/SoundTypes.h"

#define BUTTON_TOOLTIP_A	0
#define BUTTON_TOOLTIP_B	1
#define BUTTON_TOOLTIP_X	2
#define BUTTON_TOOLTIP_Y	3
#define BUTTON_TOOLTIP_LT	4
#define BUTTON_TOOLTIP_RT	5
#define BUTTON_TOOLTIP_LB	6
#define BUTTON_TOOLTIP_RB	7
#define BUTTON_TOOLTIP_LS	8
#define BUTTONS_TOOLTIP_MAX 9

#define SFX_BACK			0
#define SFX_CRAFT			1
#define SFX_CRAFTFAIL		2
#define SFX_FOCUS			3
#define SFX_PRESS			4
#define SFX_SCROLL			5
#define SFX_MAX				6


// This should be our target screen height and width
#define XUI_BASE_SCENE_WIDTH 1280.0f
#define XUI_BASE_SCENE_HEIGHT 720.0f

#define XUI_BASE_SCENE_WIDTH_HALF 640.0f
#define XUI_BASE_SCENE_HEIGHT_HALF 360.0f
#define XUI_BASE_SCENE_WIDTH_QUARTER 320.0f
#define XUI_BASE_SCENE_HEIGHT_QUARTER 180.0f
#define SAFEZONE_HALF_HEIGHT 36.0f
#define SAFEZONE_HALF_WIDTH 64.0f

// How much we scale each base for splitscreen (should be 0.5f)
#define XUI_BASE_SPLITSCREEN_SCALE 1.0f//0.5f // 4J-PB - TODO - move scenes instead

// We make the tooltips bigger as they are unreadable when scaled by the above
#define XUI_BASE_SPLIT_TOOLTIPS_SCALE 1.0f//1.5f

// The percentage of starting size that the tooltips grow by
#define XUI_BASE_SPLIT_TOOLTIPS_DIFF (XUI_BASE_SPLIT_TOOLTIPS_SCALE - 1.0f)

class CXuiSceneBase : public CXuiSceneImpl
{
public:
	enum EBaseScenePosition
	{
		e_BaseScene_NotSet,

		// 1 player
		e_BaseScene_Fullscreen,

		// 2 Player split-screen
		e_BaseScene_Top,
		e_BaseScene_Bottom,
		e_BaseScene_Left,
		e_BaseScene_Right,

		// 3/4 Player split-screen
		e_BaseScene_Top_Left,
		e_BaseScene_Top_Right,
		e_BaseScene_Bottom_Left,
		e_BaseScene_Bottom_Right,
	};

protected:
	static const int m_iTooltipSpacingGap=10;
	static const int m_iTooltipSpacingGapSmall=5;
	D3DXVECTOR3 m_vPosTextInTooltip[BUTTONS_TOOLTIP_MAX];
	D3DXVECTOR3 m_vPosTextInTooltipSmall[BUTTONS_TOOLTIP_MAX];
	D3DXVECTOR3 vLogoPosA[XUSER_MAX_COUNT];

	// We have a group of these per player
	CXuiScene	m_BasePlayerScene[XUSER_MAX_COUNT];
	// Control and Element wrapper objects.
	CXuiControl m_TooltipGroup[XUSER_MAX_COUNT];
	CXuiControl m_Buttons[XUSER_MAX_COUNT][BUTTONS_TOOLTIP_MAX];
	CXuiControl m_TooltipGroupSmall[XUSER_MAX_COUNT];
	CXuiControl m_ButtonsSmall[XUSER_MAX_COUNT][BUTTONS_TOOLTIP_MAX];
	CXuiControl m_bottomLeftAnchorPoint[XUSER_MAX_COUNT];
	CXuiControl m_topLeftAnchorPoint[XUSER_MAX_COUNT];
	CXuiControl m_SavingIcon;
	CXuiControl m_Background[XUSER_MAX_COUNT];
	CXuiControl m_DarkOverlay[XUSER_MAX_COUNT];
	CXuiControl m_Logo[XUSER_MAX_COUNT];
	CXuiControl m_CrouchIcon[XUSER_MAX_COUNT];
	CXuiControl m_PressStart;
	CXuiControl m_TrialTimer;
	CXuiControl m_SafeArea;
	CXuiControl m_BossHealthGroup[XUSER_MAX_COUNT];
	CXuiControl m_BossHealthText[XUSER_MAX_COUNT];
	CXuiProgressBar *m_pBossHealthProgress;
	CXuiProgressBar m_BossHealthProgress1[XUSER_MAX_COUNT];
	CXuiProgressBar m_BossHealthProgress2[XUSER_MAX_COUNT];
	CXuiProgressBar m_BossHealthProgress3[XUSER_MAX_COUNT];
	CXuiProgressBar m_BossHealthProgress1_small[XUSER_MAX_COUNT];
	CXuiProgressBar m_BossHealthProgress2_small[XUSER_MAX_COUNT];
	CXuiProgressBar m_BossHealthProgress3_small[XUSER_MAX_COUNT];
	int m_ticksWithNoBoss;
	CXuiSound m_SFXA[SFX_MAX];
	HXUIOBJ m_hEmptyQuadrantLogo;
	HXUIOBJ m_hGamerTagA[XUSER_MAX_COUNT];
	CXuiControl m_selectedItemA[XUSER_MAX_COUNT];
	CXuiControl m_selectedItemSmallA[XUSER_MAX_COUNT];

	BOOL m_visible[XUSER_MAX_COUNT][BUTTONS_TOOLTIP_MAX];
	BOOL m_bossHealthVisible[XUSER_MAX_COUNT];
	int m_iWrongTexturePackTickC;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_SKIN_CHANGED( OnSkinChanged )
// 		XUI_ON_XM_DLCINSTALLED_MESSAGE(OnCustomMessage_DLCInstalled)
// 		XUI_ON_XM_DLCLOADED_MESSAGE(OnCustomMessage_DLCMountingComplete)
	XUI_END_MSG_MAP()
	
	// Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiPressStartMessage, m_PressStart)
		MAP_CONTROL(IDC_XuiTrialTimer, m_TrialTimer)
		MAP_CONTROL(IDC_XuiSavingIcon, m_SavingIcon)
		MAP_CONTROL(IDC_SafeArea, m_SafeArea)
		MAP_CONTROL(IDC_XuiSoundXACTBack, m_SFXA[SFX_BACK])
		MAP_CONTROL(IDC_XuiSoundXACTCraft,m_SFXA[SFX_CRAFT])
		MAP_CONTROL(IDC_XuiSoundXACTCraftFail,m_SFXA[SFX_CRAFTFAIL])
		MAP_CONTROL(IDC_XuiSoundXACTPress,m_SFXA[SFX_PRESS])
		MAP_CONTROL(IDC_XuiSoundXACTFocus,m_SFXA[SFX_FOCUS])
		MAP_CONTROL(IDC_XuiSoundXACTScroll,m_SFXA[SFX_SCROLL])
		
		//MAP_CONTROL(IDC_BossHealth, m_BossHealthGroup)
		//BEGIN_MAP_CHILD_CONTROLS(m_BossHealthGroup)
		//	MAP_CONTROL(IDC_TitleText, m_BossHealthText)
		//	MAP_CONTROL(IDC_ProgressBar, m_BossHealthProgress)
		//END_MAP_CHILD_CONTROLS()

		MAP_CONTROL(IDC_BasePlayer0, m_BasePlayerScene[0])
		BEGIN_MAP_CHILD_CONTROLS(m_BasePlayerScene[0])
			MAP_CONTROL(IDC_BottomLeftAnchorPoint, m_bottomLeftAnchorPoint[0])
			MAP_CONTROL(IDC_TopLeftAnchorPoint, m_topLeftAnchorPoint[0])
			MAP_CONTROL(IDC_Tooltips, m_TooltipGroup[0])
			BEGIN_MAP_CHILD_CONTROLS(m_TooltipGroup[0])
				MAP_CONTROL(IDC_AButton, m_Buttons[0][BUTTON_TOOLTIP_A])
				MAP_CONTROL(IDC_BButton, m_Buttons[0][BUTTON_TOOLTIP_B])
				MAP_CONTROL(IDC_XButton, m_Buttons[0][BUTTON_TOOLTIP_X])
				MAP_CONTROL(IDC_YButton, m_Buttons[0][BUTTON_TOOLTIP_Y])
				MAP_CONTROL(IDC_LTrigger, m_Buttons[0][BUTTON_TOOLTIP_LT])
				MAP_CONTROL(IDC_RTrigger, m_Buttons[0][BUTTON_TOOLTIP_RT])
				MAP_CONTROL(IDC_RBButton, m_Buttons[0][BUTTON_TOOLTIP_RB])
				MAP_CONTROL(IDC_LBButton, m_Buttons[0][BUTTON_TOOLTIP_LB])
				MAP_CONTROL(IDC_LStick,	m_Buttons[0][BUTTON_TOOLTIP_LS])
			END_MAP_CHILD_CONTROLS()
			MAP_CONTROL(IDC_TooltipsSmall, m_TooltipGroupSmall[0])
			BEGIN_MAP_CHILD_CONTROLS(m_TooltipGroupSmall[0])
				MAP_CONTROL(IDC_AButton, m_ButtonsSmall[0][BUTTON_TOOLTIP_A])
				MAP_CONTROL(IDC_BButton, m_ButtonsSmall[0][BUTTON_TOOLTIP_B])
				MAP_CONTROL(IDC_XButton, m_ButtonsSmall[0][BUTTON_TOOLTIP_X])
				MAP_CONTROL(IDC_YButton, m_ButtonsSmall[0][BUTTON_TOOLTIP_Y])
				MAP_CONTROL(IDC_LTrigger, m_ButtonsSmall[0][BUTTON_TOOLTIP_LT])
				MAP_CONTROL(IDC_RTrigger, m_ButtonsSmall[0][BUTTON_TOOLTIP_RT])
				MAP_CONTROL(IDC_RBButton, m_ButtonsSmall[0][BUTTON_TOOLTIP_RB])
				MAP_CONTROL(IDC_LBButton, m_ButtonsSmall[0][BUTTON_TOOLTIP_LB])
				MAP_CONTROL(IDC_LStick,	m_ButtonsSmall[0][BUTTON_TOOLTIP_LS])
			END_MAP_CHILD_CONTROLS()
			MAP_CONTROL(IDC_Background, m_Background[0])
			MAP_CONTROL(IDC_XuiDarkOverlay, m_DarkOverlay[0])
			MAP_CONTROL(IDC_Logo, m_Logo[0])
			MAP_CONTROL(IDC_SelectedItem, m_selectedItemA[0])
			MAP_CONTROL(IDC_SelectedItemSmall, m_selectedItemSmallA[0])
			MAP_CONTROL(IDC_BossHealth, m_BossHealthGroup[0])
			BEGIN_MAP_CHILD_CONTROLS(m_BossHealthGroup[0])
				MAP_CONTROL(IDC_TitleText, m_BossHealthText[0])
				MAP_CONTROL(IDC_ProgressBar1, m_BossHealthProgress1[0])
				MAP_CONTROL(IDC_ProgressBar2, m_BossHealthProgress2[0])
				MAP_CONTROL(IDC_ProgressBar3, m_BossHealthProgress3[0])
				MAP_CONTROL(IDC_ProgressBar1_small, m_BossHealthProgress1_small[0])
				MAP_CONTROL(IDC_ProgressBar2_small, m_BossHealthProgress2_small[0])
				MAP_CONTROL(IDC_ProgressBar3_small, m_BossHealthProgress3_small[0])
			END_MAP_CHILD_CONTROLS()
		END_MAP_CHILD_CONTROLS()
		
		MAP_CONTROL(IDC_BasePlayer1, m_BasePlayerScene[1])
		BEGIN_MAP_CHILD_CONTROLS(m_BasePlayerScene[1])
			MAP_CONTROL(IDC_BottomLeftAnchorPoint, m_bottomLeftAnchorPoint[1])
			MAP_CONTROL(IDC_TopLeftAnchorPoint, m_topLeftAnchorPoint[1])
			MAP_CONTROL(IDC_Tooltips, m_TooltipGroup[1])
			BEGIN_MAP_CHILD_CONTROLS(m_TooltipGroup[1])
				MAP_CONTROL(IDC_AButton, m_Buttons[1][BUTTON_TOOLTIP_A])
				MAP_CONTROL(IDC_BButton, m_Buttons[1][BUTTON_TOOLTIP_B])
				MAP_CONTROL(IDC_XButton, m_Buttons[1][BUTTON_TOOLTIP_X])
				MAP_CONTROL(IDC_YButton, m_Buttons[1][BUTTON_TOOLTIP_Y])
				MAP_CONTROL(IDC_LTrigger, m_Buttons[1][BUTTON_TOOLTIP_LT])
				MAP_CONTROL(IDC_RTrigger, m_Buttons[1][BUTTON_TOOLTIP_RT])
				MAP_CONTROL(IDC_RBButton, m_Buttons[1][BUTTON_TOOLTIP_RB])
				MAP_CONTROL(IDC_LBButton, m_Buttons[1][BUTTON_TOOLTIP_LB])
				MAP_CONTROL(IDC_LStick,	m_Buttons[1][BUTTON_TOOLTIP_LS])
			END_MAP_CHILD_CONTROLS()
			MAP_CONTROL(IDC_TooltipsSmall, m_TooltipGroupSmall[1])
			BEGIN_MAP_CHILD_CONTROLS(m_TooltipGroupSmall[1])
				MAP_CONTROL(IDC_AButton, m_ButtonsSmall[1][BUTTON_TOOLTIP_A])
				MAP_CONTROL(IDC_BButton, m_ButtonsSmall[1][BUTTON_TOOLTIP_B])
				MAP_CONTROL(IDC_XButton, m_ButtonsSmall[1][BUTTON_TOOLTIP_X])
				MAP_CONTROL(IDC_YButton, m_ButtonsSmall[1][BUTTON_TOOLTIP_Y])
				MAP_CONTROL(IDC_LTrigger, m_ButtonsSmall[1][BUTTON_TOOLTIP_LT])
				MAP_CONTROL(IDC_RTrigger, m_ButtonsSmall[1][BUTTON_TOOLTIP_RT])
				MAP_CONTROL(IDC_RBButton, m_ButtonsSmall[1][BUTTON_TOOLTIP_RB])
				MAP_CONTROL(IDC_LBButton, m_ButtonsSmall[1][BUTTON_TOOLTIP_LB])				
				MAP_CONTROL(IDC_LStick,	m_ButtonsSmall[1][BUTTON_TOOLTIP_LS])
			END_MAP_CHILD_CONTROLS()
			MAP_CONTROL(IDC_Background, m_Background[1])
			MAP_CONTROL(IDC_XuiDarkOverlay, m_DarkOverlay[1])
			MAP_CONTROL(IDC_Logo, m_Logo[1])
			MAP_CONTROL(IDC_SelectedItem, m_selectedItemA[1])
			MAP_CONTROL(IDC_SelectedItemSmall, m_selectedItemSmallA[1])
			MAP_CONTROL(IDC_BossHealth, m_BossHealthGroup[1])
			BEGIN_MAP_CHILD_CONTROLS(m_BossHealthGroup[1])
				MAP_CONTROL(IDC_TitleText, m_BossHealthText[1])
				MAP_CONTROL(IDC_ProgressBar1, m_BossHealthProgress1[1])
				MAP_CONTROL(IDC_ProgressBar2, m_BossHealthProgress2[1])
				MAP_CONTROL(IDC_ProgressBar3, m_BossHealthProgress3[1])
				MAP_CONTROL(IDC_ProgressBar1_small, m_BossHealthProgress1_small[1])
				MAP_CONTROL(IDC_ProgressBar2_small, m_BossHealthProgress2_small[1])
				MAP_CONTROL(IDC_ProgressBar3_small, m_BossHealthProgress3_small[1])
			END_MAP_CHILD_CONTROLS()
		END_MAP_CHILD_CONTROLS()
		
		MAP_CONTROL(IDC_BasePlayer2, m_BasePlayerScene[2])
		BEGIN_MAP_CHILD_CONTROLS(m_BasePlayerScene[2])
			MAP_CONTROL(IDC_BottomLeftAnchorPoint, m_bottomLeftAnchorPoint[2])
			MAP_CONTROL(IDC_TopLeftAnchorPoint, m_topLeftAnchorPoint[2])			
			MAP_CONTROL(IDC_Tooltips, m_TooltipGroup[2])
			BEGIN_MAP_CHILD_CONTROLS(m_TooltipGroup[2])
				MAP_CONTROL(IDC_AButton, m_Buttons[2][BUTTON_TOOLTIP_A])
				MAP_CONTROL(IDC_BButton, m_Buttons[2][BUTTON_TOOLTIP_B])
				MAP_CONTROL(IDC_XButton, m_Buttons[2][BUTTON_TOOLTIP_X])
				MAP_CONTROL(IDC_YButton, m_Buttons[2][BUTTON_TOOLTIP_Y])
				MAP_CONTROL(IDC_LTrigger, m_Buttons[2][BUTTON_TOOLTIP_LT])
				MAP_CONTROL(IDC_RTrigger, m_Buttons[2][BUTTON_TOOLTIP_RT])
				MAP_CONTROL(IDC_RBButton, m_Buttons[2][BUTTON_TOOLTIP_RB])
				MAP_CONTROL(IDC_LBButton, m_Buttons[2][BUTTON_TOOLTIP_LB])				
				MAP_CONTROL(IDC_LStick,	m_Buttons[2][BUTTON_TOOLTIP_LS])
			END_MAP_CHILD_CONTROLS()
			MAP_CONTROL(IDC_TooltipsSmall, m_TooltipGroupSmall[2])
			BEGIN_MAP_CHILD_CONTROLS(m_TooltipGroupSmall[2])
				MAP_CONTROL(IDC_AButton, m_ButtonsSmall[2][BUTTON_TOOLTIP_A])
				MAP_CONTROL(IDC_BButton, m_ButtonsSmall[2][BUTTON_TOOLTIP_B])
				MAP_CONTROL(IDC_XButton, m_ButtonsSmall[2][BUTTON_TOOLTIP_X])
				MAP_CONTROL(IDC_YButton, m_ButtonsSmall[2][BUTTON_TOOLTIP_Y])
				MAP_CONTROL(IDC_LTrigger, m_ButtonsSmall[2][BUTTON_TOOLTIP_LT])
				MAP_CONTROL(IDC_RTrigger, m_ButtonsSmall[2][BUTTON_TOOLTIP_RT])
				MAP_CONTROL(IDC_RBButton, m_ButtonsSmall[2][BUTTON_TOOLTIP_RB])
				MAP_CONTROL(IDC_LBButton, m_ButtonsSmall[2][BUTTON_TOOLTIP_LB])
				MAP_CONTROL(IDC_LStick,	m_ButtonsSmall[2][BUTTON_TOOLTIP_LS])
			END_MAP_CHILD_CONTROLS()
			MAP_CONTROL(IDC_Background, m_Background[2])
			MAP_CONTROL(IDC_XuiDarkOverlay, m_DarkOverlay[2])
			MAP_CONTROL(IDC_Logo, m_Logo[2])
			MAP_CONTROL(IDC_SelectedItem, m_selectedItemA[2])
			MAP_CONTROL(IDC_SelectedItemSmall, m_selectedItemSmallA[2])
			MAP_CONTROL(IDC_BossHealth, m_BossHealthGroup[2])
			BEGIN_MAP_CHILD_CONTROLS(m_BossHealthGroup[2])
				MAP_CONTROL(IDC_TitleText, m_BossHealthText[2])
				MAP_CONTROL(IDC_ProgressBar1, m_BossHealthProgress1[2])
				MAP_CONTROL(IDC_ProgressBar2, m_BossHealthProgress2[2])
				MAP_CONTROL(IDC_ProgressBar3, m_BossHealthProgress3[2])
				MAP_CONTROL(IDC_ProgressBar1_small, m_BossHealthProgress1_small[2])
				MAP_CONTROL(IDC_ProgressBar2_small, m_BossHealthProgress2_small[2])
				MAP_CONTROL(IDC_ProgressBar3_small, m_BossHealthProgress3_small[2])
			END_MAP_CHILD_CONTROLS()
		END_MAP_CHILD_CONTROLS()
		
		MAP_CONTROL(IDC_BasePlayer3, m_BasePlayerScene[3])
		BEGIN_MAP_CHILD_CONTROLS(m_BasePlayerScene[3])
			MAP_CONTROL(IDC_BottomLeftAnchorPoint, m_bottomLeftAnchorPoint[3])
			MAP_CONTROL(IDC_TopLeftAnchorPoint, m_topLeftAnchorPoint[3])
			MAP_CONTROL(IDC_Tooltips, m_TooltipGroup[3])
			BEGIN_MAP_CHILD_CONTROLS(m_TooltipGroup[3])
				MAP_CONTROL(IDC_AButton, m_Buttons[3][BUTTON_TOOLTIP_A])
				MAP_CONTROL(IDC_BButton, m_Buttons[3][BUTTON_TOOLTIP_B])
				MAP_CONTROL(IDC_XButton, m_Buttons[3][BUTTON_TOOLTIP_X])
				MAP_CONTROL(IDC_YButton, m_Buttons[3][BUTTON_TOOLTIP_Y])
				MAP_CONTROL(IDC_LTrigger, m_Buttons[3][BUTTON_TOOLTIP_LT])
				MAP_CONTROL(IDC_RTrigger, m_Buttons[3][BUTTON_TOOLTIP_RT])
				MAP_CONTROL(IDC_RBButton, m_Buttons[3][BUTTON_TOOLTIP_RB])
				MAP_CONTROL(IDC_LBButton, m_Buttons[3][BUTTON_TOOLTIP_LB])
				MAP_CONTROL(IDC_LStick,	m_Buttons[3][BUTTON_TOOLTIP_LS])
			END_MAP_CHILD_CONTROLS()
			MAP_CONTROL(IDC_TooltipsSmall, m_TooltipGroupSmall[3])
			BEGIN_MAP_CHILD_CONTROLS(m_TooltipGroupSmall[3])
				MAP_CONTROL(IDC_AButton, m_ButtonsSmall[3][BUTTON_TOOLTIP_A])
				MAP_CONTROL(IDC_BButton, m_ButtonsSmall[3][BUTTON_TOOLTIP_B])
				MAP_CONTROL(IDC_XButton, m_ButtonsSmall[3][BUTTON_TOOLTIP_X])
				MAP_CONTROL(IDC_YButton, m_ButtonsSmall[3][BUTTON_TOOLTIP_Y])
				MAP_CONTROL(IDC_LTrigger, m_ButtonsSmall[3][BUTTON_TOOLTIP_LT])
				MAP_CONTROL(IDC_RTrigger, m_ButtonsSmall[3][BUTTON_TOOLTIP_RT])
				MAP_CONTROL(IDC_RBButton, m_ButtonsSmall[3][BUTTON_TOOLTIP_RB])
				MAP_CONTROL(IDC_LBButton, m_ButtonsSmall[3][BUTTON_TOOLTIP_LB])
				MAP_CONTROL(IDC_LStick,	m_Buttons[3][BUTTON_TOOLTIP_LS])
			END_MAP_CHILD_CONTROLS()
			MAP_CONTROL(IDC_Background, m_Background[3])
			MAP_CONTROL(IDC_XuiDarkOverlay, m_DarkOverlay[3])
			MAP_CONTROL(IDC_Logo, m_Logo[3])
			MAP_CONTROL(IDC_SelectedItem, m_selectedItemA[3])
			MAP_CONTROL(IDC_SelectedItemSmall, m_selectedItemSmallA[3])
			MAP_CONTROL(IDC_BossHealth, m_BossHealthGroup[3])
			BEGIN_MAP_CHILD_CONTROLS(m_BossHealthGroup[3])
				MAP_CONTROL(IDC_TitleText, m_BossHealthText[3])
				MAP_CONTROL(IDC_ProgressBar1, m_BossHealthProgress1[3])
				MAP_CONTROL(IDC_ProgressBar2, m_BossHealthProgress2[3])
				MAP_CONTROL(IDC_ProgressBar3, m_BossHealthProgress3[3])
				MAP_CONTROL(IDC_ProgressBar1_small, m_BossHealthProgress1_small[3])
				MAP_CONTROL(IDC_ProgressBar2_small, m_BossHealthProgress2_small[3])
				MAP_CONTROL(IDC_ProgressBar3_small, m_BossHealthProgress3_small[3])
			END_MAP_CHILD_CONTROLS()
		END_MAP_CHILD_CONTROLS()
		
	END_CONTROL_MAP()

	HRESULT	OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnTimer(XUIMessageTimer *pData,BOOL& rfHandled);
	HRESULT OnSkinChanged(BOOL& bHandled);
// 	HRESULT OnCustomMessage_DLCInstalled();
// 	HRESULT OnCustomMessage_DLCMountingComplete();

public:
	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CXuiSceneBase, L"CXuiSceneBase", XUI_CLASS_SCENE )

private:
	void						_TickAllBaseScenes();
	HRESULT						_SetTooltipText( unsigned int iPad, unsigned int tooltip, int iTextID );
	HRESULT						_SetEnableTooltips( unsigned int iPad, BOOL bVal );
	HRESULT						_ShowTooltip( unsigned int iPad, unsigned int tooltip, bool show );
	HRESULT						_SetTooltipsEnabled( unsigned int iPad, bool bA = true, bool bB = true, bool bX = true, bool bY = true, bool bLT = true, bool bRT = true,  bool bLB=true, bool bRB = true, bool bLS = true);
	HRESULT						_RefreshTooltips( unsigned int iPad);
	HRESULT						_EnableTooltip( unsigned int iPad, unsigned int tooltip, bool enable );
	HRESULT						_ShowSavingMessage( unsigned int iPad, C4JStorage::ESavingMessage eVal );
	HRESULT						_ShowBackground( unsigned int iPad, BOOL bShow );
	HRESULT						_ShowDarkOverlay( unsigned int iPad, BOOL bShow );
	HRESULT						_ShowLogo( unsigned int iPad, BOOL bShow );
	HRESULT						_ShowPressStart(unsigned int iPad);
	HRESULT						_UpdateAutosaveCountdownTimer(unsigned int uiSeconds);
	HRESULT 					_ShowAutosaveCountdownTimer(BOOL bVal);
	HRESULT						_UpdateTrialTimer(unsigned int iPad);
	HRESULT						_ShowTrialTimer(BOOL bVal);
	void						_ReduceTrialTimerValue();
	HRESULT						_HidePressStart();
	HRESULT						_ShowSafeArea( BOOL bShow );
	HRESULT						_ShowOtherPlayersBaseScene(int iPad, bool show);
	bool						_PressStartPlaying(unsigned int iPad);
	HRESULT						_SetPlayerBaseScenePosition( unsigned int iPad, EBaseScenePosition position );
	void						_UpdateSelectedItemPos( unsigned int iPad);
	EBaseScenePosition			_GetPlayerBasePosition(int iPad);
	HRESULT						_AnimateKeyPress(DWORD userIndex, DWORD dwKeyCode);
	HXUIOBJ						_GetPlayerBaseScene(int iPad) {return m_BasePlayerScene[iPad].m_hObj;}
	HRESULT						_PlayUISFX(ESoundEffect eSound);
	void						_SetEmptyQuadrantLogo(int iPad,EBaseScenePosition ePos);
	HRESULT						_DisplayGamertag( unsigned int iPad, BOOL bDisplay );
	void						_SetSelectedItem( unsigned int iPad, const wstring& name);
	void						_HideAllGameUIElements();
	bool						_GetBaseSceneSafeZone( unsigned int iPad, D3DXVECTOR2 &origin, float &width, float &height);

	void						ReLayout( unsigned int iPad );

private:
	static CXuiSceneBase		*Instance;
	int							m_iCurrentTooltipTextID[XUSER_MAX_COUNT][BUTTONS_TOOLTIP_MAX];
	HXUIOBJ						hTooltipText[XUSER_MAX_COUNT][BUTTONS_TOOLTIP_MAX];
	HXUIOBJ						hTooltipTextSmall[XUSER_MAX_COUNT][BUTTONS_TOOLTIP_MAX];
	EBaseScenePosition			m_playerBaseScenePosition[XUSER_MAX_COUNT];
	bool						m_bCrouching[XUSER_MAX_COUNT];
	int							m_iQuadrantsMask;	
	unsigned int				m_uiSelectedItemOpacityCountDown[XUSER_MAX_COUNT];

public:
	static DWORD				m_dwTrialTimerLimitSecs;

public:
	static CXuiSceneBase		*GetInstance() { return Instance; }
	static void					TickAllBaseScenes();
	static HRESULT				SetTooltipText( unsigned int iPad, unsigned int tooltip, int iTextID );
	static HRESULT				SetEnableTooltips( unsigned int iPad, BOOL bVal );
	static HRESULT				ShowTooltip( unsigned int iPad, unsigned int tooltip, bool show );
	static HRESULT				SetTooltips( unsigned int iPad, int iA, int iB=-1, int iX=-1, int iY=-1 , int iLT=-1, int iRT=-1, int iLB=-1, int iRB=-1, int iLS=-1, bool forceUpdate = false);
	static HRESULT				RefreshTooltips( unsigned int iPad);
	static HRESULT				EnableTooltip( unsigned int iPad, unsigned int tooltip, bool enable );
	static HRESULT				SetTooltipsEnabled( unsigned int iPad, bool bA = true, bool bB = true, bool bX = true, bool bY = true, bool bLT = true, bool bRT = true, bool bLB = true, bool bRB=true, bool bLS=true);
	static HRESULT				AnimateKeyPress(DWORD userIndex, DWORD dwKeyCode);
	static HRESULT				ShowSavingMessage( unsigned int iPad, C4JStorage::ESavingMessage eVal);
	static HRESULT				ShowBackground( unsigned int iPad, BOOL bShow );
	static HRESULT				ShowDarkOverlay( unsigned int iPad, BOOL bShow );
	static HRESULT				ShowLogo( unsigned int iPad, BOOL bShow );
	static HRESULT				UpdateAutosaveCountdownTimer(unsigned int uiSeconds);
	static HRESULT 				ShowAutosaveCountdownTimer(BOOL bVal);
	static HRESULT				UpdateTrialTimer(unsigned int iPad);
	static HRESULT				ShowTrialTimer(BOOL bVal);
	static void					ReduceTrialTimerValue();
	static HRESULT				HidePressStart();
	static HRESULT				ShowSafeArea( BOOL bShow );
	static HRESULT				ShowOtherPlayersBaseScene(int iPad, bool show);

	static HRESULT				ShowPressStart(unsigned int iPad);
	static bool					PressStartPlaying(unsigned int iPad);
	static HRESULT				SetPlayerBaseScenePosition( unsigned int iPad, EBaseScenePosition position );
	static HRESULT				SetPlayerBasePositions(EBaseScenePosition pad0, EBaseScenePosition pad1, EBaseScenePosition pad2, EBaseScenePosition pad3);
	static HRESULT				UpdatePlayerBasePositions();
	static EBaseScenePosition	GetPlayerBasePosition(int iPad);
	static void					UpdateSelectedItemPos(int iPad);


	static HXUIOBJ				GetPlayerBaseScene(int iPad);
	static HRESULT				PlayUISFX(ESoundEffect eSound);
	static void					SetEmptyQuadrantLogo(int iSection);
	static HRESULT				DisplayGamertag( unsigned int iPad, BOOL bDisplay );
	static void					SetSelectedItem( unsigned int iPad, const wstring &name);
	static void					HideAllGameUIElements();

	// Returns details on the fully transformed (ie screen space) base scene position, adjusted for safe zones
	static bool					GetBaseSceneSafeZone( unsigned int iPad, D3DXVECTOR2 &origin, float &width, float &height);

#ifndef _XBOX
	static void					CreateBaseSceneInstance();
#endif
};
