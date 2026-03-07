#pragma once
#ifdef _DEBUG_MENUS_ENABLED
#include "UIScene.h"

class UIScene_DebugSetCamera : public UIScene
{
private:
	enum eControls
	{
		eControl_CamX,
		eControl_CamY,
		eControl_CamZ,
		eControl_YRot,
		eControl_Elevation,
		eControl_LockPlayer,
		eControl_Teleport,
	};

	typedef struct _FreezePlayerParam
	{
		int player;
		bool freeze;
	} FreezePlayerParam;

	DebugSetCameraPosition *currentPosition;
	FreezePlayerParam *fpp;

	eControls m_keyboardCallbackControl;
#ifdef _WINDOWS64
	UIControl_TextInput* getTextInputForControl(eControls ctrl);
#endif

public:
	UIScene_DebugSetCamera(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_DebugSetCamera;}

protected:
	UIControl_TextInput m_textInputX, m_textInputY, m_textInputZ, m_textInputYRot, m_textInputElevation;
	UIControl_CheckBox m_checkboxLockPlayer;
	UIControl_Button m_buttonTeleport;
	UIControl_Label m_labelTitle, m_labelCamX, m_labelCamY, m_labelCamZ, m_labelYRotElev;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_textInputX, "CamX")
		UI_MAP_ELEMENT( m_textInputY, "CamY")
		UI_MAP_ELEMENT( m_textInputZ, "CamZ")
		UI_MAP_ELEMENT( m_textInputYRot, "YRot")
		UI_MAP_ELEMENT( m_textInputElevation, "Elevation")
		UI_MAP_ELEMENT( m_checkboxLockPlayer, "LockPlayer")
		UI_MAP_ELEMENT( m_buttonTeleport, "Teleport")

		UI_MAP_ELEMENT( m_labelTitle, "LabelTitle")
		UI_MAP_ELEMENT( m_labelCamX, "LabelCamX")
		UI_MAP_ELEMENT( m_labelCamY, "LabelCamY")
		UI_MAP_ELEMENT( m_labelCamZ, "LabelCamZ")
		UI_MAP_ELEMENT( m_labelYRotElev, "LabelYRotElev")
	UI_END_MAP_ELEMENTS_AND_NAMES()

	virtual wstring getMoviePath();
	virtual void tick();
#ifdef _WINDOWS64
	virtual void getDirectEditInputs(vector<UIControl_TextInput*> &inputs);
	virtual void onDirectEditFinished(UIControl_TextInput *input, UIControl_TextInput::EDirectEditResult result);
	virtual bool handleMouseClick(F32 x, F32 y);
#endif

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

protected:
	void handlePress(F64 controlId, F64 childId);	
	virtual void handleCheckboxToggled(F64 controlId, bool selected);

private:
	static int KeyboardCompleteCallback(LPVOID lpParam,const bool bRes);
};
#endif