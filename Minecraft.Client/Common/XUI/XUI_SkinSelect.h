#pragma once

#include "../media/xuiscene_skinselect.h"
#include "XUI_CustomMessages.h"
#include "../../../Minecraft.World/Definitions.h"
#include "../../Textures.h"

class DLCPack;

class CXuiCtrlMinecraftSkinPreview;

class CScene_SkinSelect : public CXuiSceneImpl
{
private:
	static WCHAR *wchDefaultNamesA[eDefaultSkins_Count];

	// 4J Stu - How many to show on each side of the main control
	static const BYTE sidePreviewControls = 4;

	enum ESkinSelectNavigation
	{
		eSkinNavigation_Pack,
		eSkinNavigation_Skin,
		
		eSkinNavigation_Count,
	};

protected:
	CXuiControl m_skinDetails, m_text, m_originText;
	CXuiCtrlMinecraftSkinPreview *m_previewControl;
	CXuiCtrlMinecraftSkinPreview *m_previewPreviousControls[sidePreviewControls];
	CXuiCtrlMinecraftSkinPreview *m_previewNextControls[sidePreviewControls];
	CXuiControl m_packGroup, m_charactersGroup;
	CXuiControl m_packLeft, m_packRight, m_packCenter;
	CXuiImageElement m_imagePadlock;
	CXuiElement m_selectedGroup;
	CXuiControl m_selectedText;
	CXuiControl m_timer;
	CXuiElement m_tabGroup;
	CXuiElement m_normalTabs, m_selectedTabs;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_KEYUP( OnKeyUp )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)
		XUI_ON_XM_TIMELINE_END( OnTimelineEnd )
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
		XUI_ON_XM_DLCLOADED_MESSAGE(OnCustomMessage_DLCMountingComplete)
		XUI_ON_XM_BASE_POSITION_CHANGED_MESSAGE(OnBasePositionChanged)
		XUI_ON_XM_DLCINSTALLED_MESSAGE(OnCustomMessage_DLCInstalled)
	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_Timer, m_timer)

		MAP_CONTROL(IDC_PackGroup, m_packGroup)
		BEGIN_MAP_CHILD_CONTROLS( m_packGroup)
			MAP_CONTROL(IDC_Left, m_packLeft)
			MAP_CONTROL(IDC_Center, m_packCenter)
			MAP_CONTROL(IDC_Right, m_packRight)
		END_MAP_CHILD_CONTROLS()
		
		MAP_CONTROL(IDC_TabBar, m_tabGroup)
		BEGIN_MAP_CHILD_CONTROLS( m_tabGroup )
			MAP_CONTROL(IDC_Selected, m_selectedTabs )
			MAP_CONTROL(IDC_Normal, m_normalTabs )
		END_MAP_CHILD_CONTROLS()
		
		MAP_CONTROL(IDC_SelectedGroup, m_selectedGroup)
		BEGIN_MAP_CHILD_CONTROLS( m_selectedGroup )
			MAP_CONTROL(IDC_SelectedText, m_selectedText)
		END_MAP_CHILD_CONTROLS()

		MAP_CONTROL(IDC_Locked, m_imagePadlock)

		MAP_CONTROL(IDC_SkinDetails, m_skinDetails)
		BEGIN_MAP_CHILD_CONTROLS( m_skinDetails)
			MAP_CONTROL(IDC_SkinName, m_text)
			MAP_CONTROL(IDC_OriginName, m_originText)
		END_MAP_CHILD_CONTROLS()
		
		MAP_CONTROL(IDC_Characters, m_charactersGroup)
		BEGIN_MAP_CHILD_CONTROLS( m_charactersGroup )
			MAP_OVERRIDE(IDC_Character, m_previewControl)
			MAP_OVERRIDE(IDC_CharacterPrevious1, m_previewPreviousControls[0])
			MAP_OVERRIDE(IDC_CharacterPrevious2, m_previewPreviousControls[1])
			MAP_OVERRIDE(IDC_CharacterPrevious3, m_previewPreviousControls[2])
			MAP_OVERRIDE(IDC_CharacterPrevious4, m_previewPreviousControls[3])
			MAP_OVERRIDE(IDC_CharacterNext1, m_previewNextControls[0])
			MAP_OVERRIDE(IDC_CharacterNext2, m_previewNextControls[1])
			MAP_OVERRIDE(IDC_CharacterNext3, m_previewNextControls[2])
			MAP_OVERRIDE(IDC_CharacterNext4, m_previewNextControls[3])
		END_MAP_CHILD_CONTROLS()
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled);
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnKeyUp(XUIMessageInput *pInputData, BOOL& bHandled);
	HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );
	HRESULT OnTimelineEnd(HXUIOBJ hObjSource, BOOL& bHandled);
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
	HRESULT OnCustomMessage_DLCInstalled();
	HRESULT OnCustomMessage_DLCMountingComplete();
	HRESULT OnBasePositionChanged();

	int m_iPad;
	D3DXVECTOR3 m_OriginalPosition;
	wstring m_currentSkinPath, m_selectedSkinPath, m_selectedCapePath;
	vector<SKIN_BOX *> *m_vAdditionalSkinBoxes;
	//vector<ModelPart *> *m_vAdditionalModelParts;
	DWORD m_originalSkinId;

	DLCPack *m_currentPack;
	DWORD m_packIndex, m_skinIndex;
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_SkinSelect, L"CScene_SkinSelect", XUI_CLASS_SCENE )

private:
	void handleSkinIndexChanged();
	void handlePackIndexChanged();
	void updatePackDisplay();
	void updateCurrentFocus();
	TEXTURE_NAME getTextureId(int skinIndex);

	int getNextSkinIndex(DWORD sourceIndex);
	int getPreviousSkinIndex(DWORD sourceIndex);

	int getNextPackIndex(DWORD sourceIndex);
	int getPreviousPackIndex(DWORD sourceIndex);

	void updateClipping();
	
	static int UnlockSkinReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);

	void AddFavoriteSkin(int iPad,int iSkinID);

	bool m_bSlidingSkins, m_bAnimatingMove;

	DWORD currentPackCount;

	ESkinSelectNavigation m_currentNavigation;
	bool m_bIgnoreInput;
};
