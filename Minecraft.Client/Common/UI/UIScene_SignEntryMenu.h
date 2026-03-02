#pragma once

#include "UIScene.h"

class SignTileEntity;

class UIScene_SignEntryMenu : public UIScene
{
private:
	enum EControls
	{
		// Lines should be 0-3
		eControl_Line1,
		eControl_Line2,
		eControl_Line3,
		eControl_Line4,
		eControl_Confirm
	};

	shared_ptr<SignTileEntity> m_sign;
	int m_iEditingLine;
	bool m_bConfirmed;
	bool m_bIgnoreInput;

	UIControl_Button m_buttonConfirm;
	UIControl_Label m_labelMessage;
	UIControl_TextInput m_textInputLines[4];
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_buttonConfirm, "Confirm")
		UI_MAP_ELEMENT( m_labelMessage, "Message")

		UI_MAP_ELEMENT( m_textInputLines[0], "Line1")
		UI_MAP_ELEMENT( m_textInputLines[1], "Line2")
		UI_MAP_ELEMENT( m_textInputLines[2], "Line3")
		UI_MAP_ELEMENT( m_textInputLines[3], "Line4")
	UI_END_MAP_ELEMENTS_AND_NAMES()
public:
	UIScene_SignEntryMenu(int iPad, void *initData, UILayer *parentLayer);
	virtual ~UIScene_SignEntryMenu();

	virtual EUIScene getSceneType() { return eUIScene_SignEntryMenu;}
	virtual void updateTooltips();

	virtual void tick();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

protected:
	void handlePress(F64 controlId, F64 childId);
	static int KeyboardCompleteCallback(LPVOID lpParam,const bool bRes);
	virtual void handleDestroy();
};