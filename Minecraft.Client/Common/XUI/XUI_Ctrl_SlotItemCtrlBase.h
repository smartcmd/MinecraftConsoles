#pragma once
using namespace std;

class Slot;
class ItemInstance;

#include "XUI_CustomMessages.h"

class SlotControlUserDataContainer
{
public:
	SlotControlUserDataContainer() : slot( nullptr ), hProgressBar( nullptr ), m_iPad( -1 ), m_fAlpha(1.0f) {};
	Slot* slot;
	HXUIOBJ hProgressBar;
	float m_fAlpha;
	int m_iPad;
};

// The base class for all controls with the "ItemButton" visual
// This could be a list item or just a button.
// We need this class to be able to easily access all the parts of the visual

class CXuiCtrlSlotItemCtrlBase
{
private:
	// 4J WESTY : Pointer Prototype : Added to support prototype only.
	BOOL	m_bSkipDefaultNavigation;

public:
	// We define a lot of functions as virtual. These should be implemented to call the protected version by
	// passing in the HXUIOBJ of the control as the first parameter. We do not have access to that normally
	// due to the inheritance structure	
	virtual HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled ) = 0;
	
	virtual HRESULT OnDestroy() = 0;

	virtual HRESULT OnCustomMessage_GetSlotItem(CustomMessage_GetSlotItem_Struct *pData, BOOL& bHandled) = 0;

	// 4J WESTY : Pointer Prototype : Added to support prototype only.
	virtual HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled) = 0;

	virtual HRESULT OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled) = 0;

	void SetSlot( HXUIOBJ hObj, Slot* slot );
	void SetAlpha( HXUIOBJ hObj, float fAlpha );
	void SetUserIndex( HXUIOBJ hObj, int iPad );

	bool isEmpty( HXUIOBJ hObj );

	wstring GetItemDescription( HXUIOBJ hObj, vector<wstring> &unformattedStrings );

	shared_ptr<ItemInstance> getItemInstance( HXUIOBJ hObj );
	Slot *getSlot( HXUIOBJ hObj );

	// 4J WESTY : Pointer Prototype : Added to support prototype only.
	int	 GetObjectCount( HXUIOBJ hObj );

	// 4J WESTY : Pointer Prototype : Added to support prototype only.
	void	SetSkipsDefaultNavigation( BOOL bSkipDefaultNavigation )			{ m_bSkipDefaultNavigation = bSkipDefaultNavigation; }

	// 4J WESTY : Pointer Prototype : Added to support prototype only.
	int		GetEmptyStackSpace( HXUIOBJ hObj );		// Returns number of items that can still be stacked into this slot.

	// 4J WESTY : Pointer Prototype : Added to support prototype only.
	bool	IsSameItemAs( HXUIOBJ hThisObj, HXUIOBJ hOtherObj );

protected:	
	HRESULT OnInit( HXUIOBJ hObj, XUIMessageInit* pInitData, BOOL& bHandled );
	
	HRESULT OnDestroy( HXUIOBJ hObj );

	HRESULT OnCustomMessage_GetSlotItem(HXUIOBJ hObj, CustomMessage_GetSlotItem_Struct *pData, BOOL& bHandled);

	// 4J WESTY : Pointer Prototype : Added to support prototype only.
	HRESULT OnControlNavigate(HXUIOBJ hObj, XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);

	HRESULT OnKeyDown(HXUIOBJ hObj, XUIMessageInput *pInputData, BOOL& bHandled);
};