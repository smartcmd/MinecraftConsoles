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

	swprintf( (WCHAR *)TempString, 256, L"%f", currentPosition->m_camX);
	m_textInputX.init(TempString, eControl_CamX);

	swprintf( (WCHAR *)TempString, 256, L"%f", currentPosition->m_camY);
	m_textInputY.init(TempString, eControl_CamY);

	swprintf( (WCHAR *)TempString, 256, L"%f", currentPosition->m_camZ);
	m_textInputZ.init(TempString, eControl_CamZ);

	swprintf( (WCHAR *)TempString, 256, L"%f", currentPosition->m_yRot);
	m_textInputYRot.init(TempString, eControl_YRot);

	swprintf( (WCHAR *)TempString, 256, L"%f", currentPosition->m_elev);
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

void UIScene_DebugSetCamera::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
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
		InputManager.RequestKeyboard(L"Enter something",L"",(DWORD)0,25,&UIScene_DebugSetCamera::KeyboardCompleteCallback,this,C_4JInput::EKeyboardMode_Default);
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
	uint16_t pchText[2048];//[128];
	ZeroMemory(pchText, 2048/*128*/ * sizeof(uint16_t) );
	InputManager.GetText(pchText);

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
