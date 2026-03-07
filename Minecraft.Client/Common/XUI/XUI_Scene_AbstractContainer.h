#pragma once
#include "../UI/IUIScene_AbstractContainerMenu.h"
#include "XUI_CustomMessages.h"
#include "../../Common/Tutorial/TutorialEnum.h"

class CXuiCtrlSlotList;
class CXuiCtrlSlotItem;
class AbstractContainerMenu;
class Slot;

class CXuiSceneAbstractContainer : public CXuiSceneImpl, public virtual IUIScene_AbstractContainerMenu
{
public:
	BOOL isPauseScreen() {
		return FALSE;
	};

	D3DXVECTOR3 GetCursorScreenPosition();

protected:
	virtual void PlatformInitialize(int iPad, int startIndex);
	virtual void InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex = 0);

	CXuiCtrlSlotList* m_inventoryControl;
	CXuiCtrlSlotList* m_useRowControl;
	CXuiCtrlSlotItem* m_pointerControl;
	CXuiControl m_InventoryText;
	
	HRESULT OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled);
	// Timer function to poll stick input and update pointer position.
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
	HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );

protected:
	// 4J JEV - Wanted to override onClick method in XUI_Scene_Inventory_Creative,
	// so am making this protected.


protected:
	int	m_iPad;
	D3DXVECTOR3 m_OriginalPosition;
	bool m_bIgnoreKeyPresses;

private:
	int mapVKToAction(int vk);

protected:
	virtual HRESULT handleCustomTimer( XUIMessageTimer *pTimer, BOOL& bHandled );

public:
	int getPad() { return m_iPad; }

#ifdef USE_POINTER_ACCEL
	float	m_fPointerVelX;
	float	m_fPointerVelY;

	float	m_fPointerAccelX;
	float	m_fPointerAccelY;
#endif


	HXUIOBJ m_hPointerTextMeasurer;
	HXUIOBJ m_hPointerText;
	HXUIOBJ m_hPointerTextBkg;
	HXUIOBJ m_hPointerImg;
	
	virtual int getSectionColumns(ESceneSection eSection);
	virtual int getSectionRows(ESceneSection eSection);
	virtual CXuiControl*	GetSectionControl( ESceneSection eSection ) = 0;
	virtual CXuiCtrlSlotList*	GetSectionSlotList( ESceneSection eSection ) = 0;
	virtual void GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition );
	virtual void GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize );
	void handleSectionClick(ESceneSection eSection);
	virtual bool doesSectionTreeHaveFocus(ESceneSection eSection);
	virtual void setSectionFocus(ESceneSection eSection, int iPad);
	void setSectionSelectedSlot(ESceneSection eSection, int x, int y);
	void setFocusToPointer(int iPad);
	void SetPointerText(const wstring &description, vector<wstring> &unformattedStrings, bool newSlot);
	virtual shared_ptr<ItemInstance> getSlotItem(ESceneSection eSection, int iSlot);
	virtual bool isSlotEmpty(ESceneSection eSection, int iSlot);
	virtual void adjustPointerForSafeZone();
};