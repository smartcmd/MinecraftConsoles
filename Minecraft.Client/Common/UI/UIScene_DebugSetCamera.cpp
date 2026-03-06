#include "stdafx.h"

#ifdef _DEBUG_MENUS_ENABLED
#include "UI.h"
#include "UIScene_DebugSetCamera.h"
#include "Minecraft.h"
#include "MultiPlayerLocalPlayer.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"

UIScene_DebugSetCamera::UIScene_DebugSetCamera(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	int playerNo = 0;
	currentPosition = new DebugSetCameraPosition();
	currentPosition->player = playerNo;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if (pMinecraft != NULL)
	{
		Vec3 *vec = pMinecraft->localplayers[playerNo]->getPos(1.0);

		currentPosition->m_camX = vec->x;
		currentPosition->m_camY = vec->y - 1.62;// pMinecraft->localplayers[playerNo]->getHeadHeight();
		currentPosition->m_camZ = vec->z;

		currentPosition->m_yRot = pMinecraft->localplayers[playerNo]->yRot;
		currentPosition->m_elev = pMinecraft->localplayers[playerNo]->xRot;
	}

	WCHAR TempString[256];

	swprintf( (WCHAR *)TempString, 256, L"%.2f", currentPosition->m_camX);
	m_textInputX.init(TempString, eControl_CamX);

	swprintf( (WCHAR *)TempString, 256, L"%.2f", currentPosition->m_camY);
	m_textInputY.init(TempString, eControl_CamY);

	swprintf( (WCHAR *)TempString, 256, L"%.2f", currentPosition->m_camZ);
	m_textInputZ.init(TempString, eControl_CamZ);

	swprintf( (WCHAR *)TempString, 256, L"%.2f", currentPosition->m_yRot);
	m_textInputYRot.init(TempString, eControl_YRot);

	swprintf( (WCHAR *)TempString, 256, L"%.2f", currentPosition->m_elev);
	m_textInputElevation.init(TempString, eControl_Elevation);

	m_checkboxLockPlayer.init(L"Lock Player", eControl_LockPlayer, app.GetFreezePlayers());

	m_buttonTeleport.init(L"Teleport", eControl_Teleport);

	m_labelTitle.init(L"Set Camera Position");
	m_labelCamX.init(L"CamX");
	m_labelCamY.init(L"CamY");
	m_labelCamZ.init(L"CamZ");
	m_labelYRotElev.init(L"Y-Rot & Elevation (Degs)");

}

wstring UIScene_DebugSetCamera::getMoviePath()
{
	return L"DebugSetCamera";
}

#ifdef _WINDOWS64
UIControl_TextInput* UIScene_DebugSetCamera::getTextInputForControl(eControls ctrl)
{
	switch (ctrl)
	{
	case eControl_CamX:      return &m_textInputX;
	case eControl_CamY:      return &m_textInputY;
	case eControl_CamZ:      return &m_textInputZ;
	case eControl_YRot:      return &m_textInputYRot;
	case eControl_Elevation: return &m_textInputElevation;
	default: return NULL;
	}
}

void UIScene_DebugSetCamera::getDirectEditInputs(vector<UIControl_TextInput*> &inputs)
{
	inputs.push_back(&m_textInputX);
	inputs.push_back(&m_textInputY);
	inputs.push_back(&m_textInputZ);
	inputs.push_back(&m_textInputYRot);
	inputs.push_back(&m_textInputElevation);
}

void UIScene_DebugSetCamera::onDirectEditFinished(UIControl_TextInput *input, UIControl_TextInput::EDirectEditResult result)
{
	wstring value = input->getEditBuffer();
	double val = 0;
	if (!value.empty()) val = _fromString<double>(value);

	if (input == &m_textInputX)           currentPosition->m_camX = val;
	else if (input == &m_textInputY)      currentPosition->m_camY = val;
	else if (input == &m_textInputZ)      currentPosition->m_camZ = val;
	else if (input == &m_textInputYRot)   currentPosition->m_yRot = val;
	else if (input == &m_textInputElevation) currentPosition->m_elev = val;
}

bool UIScene_DebugSetCamera::handleMouseClick(F32 x, F32 y)
{
	UIScene::handleMouseClick(x, y);
	return true; // always consume to prevent Iggy re-entry on empty space
}
#endif

void UIScene_DebugSetCamera::tick()
{
	UIScene::tick();
}

void UIScene_DebugSetCamera::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
#ifdef _WINDOWS64
	if (isDirectEditBlocking()) { handled = true; return; }
#endif
	ui.AnimateKeyPress(iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			navigateBack();
		}
		break;
	case ACTION_MENU_OK:
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
	case ACTION_MENU_PAGEUP:
	case ACTION_MENU_PAGEDOWN:
	case ACTION_MENU_LEFT:
	case ACTION_MENU_RIGHT:
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_DebugSetCamera::handlePress(F64 controlId, F64 childId)
{
#ifdef _WINDOWS64
	if (isDirectEditBlocking()) return;
#endif
	switch((int)controlId)
	{
	case eControl_Teleport:
		app.SetXuiServerAction(	ProfileManager.GetPrimaryPad(),
			eXuiServerAction_SetCameraLocation,
			(void *)currentPosition);
		break;
	case eControl_CamX:
	case eControl_CamY:
	case eControl_CamZ:
	case eControl_YRot:
	case eControl_Elevation:
		m_keyboardCallbackControl = (eControls)((int)controlId);
#ifdef _WINDOWS64
		if (g_KBMInput.IsKBMActive())
		{
			UIControl_TextInput* input = getTextInputForControl(m_keyboardCallbackControl);
			if (input) input->beginDirectEdit(25);
		}
		else
		{
			UIKeyboardInitData kbData;
			kbData.title       = L"Enter value";
			kbData.defaultText = L"";
			kbData.maxChars    = 25;
			kbData.callback    = &UIScene_DebugSetCamera::KeyboardCompleteCallback;
			kbData.lpParam     = this;
			ui.NavigateToScene(m_iPad, eUIScene_Keyboard, &kbData, eUILayer_Fullscreen, eUIGroup_Fullscreen);
		}
#else
		InputManager.RequestKeyboard(L"Enter something",L"",(DWORD)0,25,&UIScene_DebugSetCamera::KeyboardCompleteCallback,this,C_4JInput::EKeyboardMode_Default);
#endif
		break;
	};
}

void UIScene_DebugSetCamera::handleCheckboxToggled(F64 controlId, bool selected)
{
	switch((int)controlId)
	{
	case eControl_LockPlayer:
		app.SetFreezePlayers(selected);
		break;
	}
}

int UIScene_DebugSetCamera::KeyboardCompleteCallback(LPVOID lpParam,bool bRes)
{
	UIScene_DebugSetCamera *pClass=(UIScene_DebugSetCamera *)lpParam;
	uint16_t pchText[2048];
	ZeroMemory(pchText, 2048 * sizeof(uint16_t));
#ifdef _WINDOWS64
	Win64_GetKeyboardText(pchText, 2048);
#else
	InputManager.GetText(pchText);
#endif

	if(pchText[0]!=0)
	{
		wstring value = (wchar_t *)pchText;
		double val = 0; 
		if(!value.empty()) val = _fromString<double>( value );
		switch(pClass->m_keyboardCallbackControl)
		{
		case eControl_CamX:
			pClass->m_textInputX.setLabel(value);
			pClass->currentPosition->m_camX = val;
			break;
		case eControl_CamY:
			pClass->m_textInputY.setLabel(value);
			pClass->currentPosition->m_camY = val;
			break;
		case eControl_CamZ:
			pClass->m_textInputZ.setLabel(value);
			pClass->currentPosition->m_camZ = val;
			break;
		case eControl_YRot:
			pClass->m_textInputYRot.setLabel(value);
			pClass->currentPosition->m_yRot = val;
			break;
		case eControl_Elevation:
			pClass->m_textInputElevation.setLabel(value);
			pClass->currentPosition->m_elev = val;
			break;
		}
	}

	return 0;
}
#endif
