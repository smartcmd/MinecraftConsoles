#include "stdafx.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "XUI_DebugSetCamera.h"
#include "../../../Minecraft.World/ChunkSource.h"

// #include "../../Xbox/4JLibs/inc/4J_Input.h"

#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"

#ifndef _CONTENT_PACKAGE
HRESULT CScene_DebugSetCamera::OnInit( XUIMessageInit *pInitData, BOOL &bHandled )
{
	MapChildControls();

	m_camX	.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);
	m_camY	.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);
	m_camZ	.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);
	m_yRot	.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);
	m_elevation	.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);

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

	m_camX.SetKeyboardType(C_4JInput::EKeyboardMode_Full);
	m_camY.SetKeyboardType(C_4JInput::EKeyboardMode_Full);
	m_camZ.SetKeyboardType(C_4JInput::EKeyboardMode_Full);
	m_yRot.SetKeyboardType(C_4JInput::EKeyboardMode_Full);
	m_elevation.SetKeyboardType(C_4JInput::EKeyboardMode_Full);

	m_camX.SetText((CONST WCHAR *) std::to_wstring(currentPosition->m_camX).c_str());
	m_camY.SetText((CONST WCHAR *) std::to_wstring(currentPosition->m_camY + 1.62).c_str());
	m_camZ.SetText((CONST WCHAR *) std::to_wstring(currentPosition->m_camZ).c_str());

	m_yRot.SetText((CONST WCHAR *) std::to_wstring(currentPosition->m_yRot).c_str());
	m_elevation.SetText((CONST WCHAR *) std::to_wstring(currentPosition->m_elev).c_str());

	//fpp = new FreezePlayerParam();
	//fpp->player = playerNo;
	//fpp->freeze = true;

	//m_lockPlayer.SetCheck( !fpp->freeze );

	m_lockPlayer.SetCheck( app.GetFreezePlayers() );

	return S_OK;
}

HRESULT CScene_DebugSetCamera::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if (hObjPressed == m_teleport)
	{
		app.SetXuiServerAction(	ProfileManager.GetPrimaryPad(),
								eXuiServerAction_SetCameraLocation,
								(void *)currentPosition);
		rfHandled = TRUE;
	}
	else if (hObjPressed == m_lockPlayer)
	{
		app.SetFreezePlayers( m_lockPlayer.IsChecked() );

		rfHandled = TRUE;
	}

	return S_OK;
}

HRESULT CScene_DebugSetCamera::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_B:
	case VK_PAD_START:
	case VK_PAD_BACK:
		NavigateBack();

		//delete currentPosition;
		//currentPosition = NULL;

		rfHandled = TRUE;
		break;
	}
	return S_OK;
}

HRESULT CScene_DebugSetCamera::OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL &bHandled)
{

	// Text Boxes
	if (hObjSource == m_camX)
	{
		double iVal = 0;
		wstring value = m_camX.GetText();
		if(!value.empty()) iVal = _fromString<double>( value );
		currentPosition->m_camX = iVal;
		bHandled = TRUE;
	}
	else if (hObjSource == m_camY)
	{
		double iVal = 0;
		wstring value = m_camY.GetText();
		if(!value.empty()) iVal = _fromString<double>( value );
		currentPosition->m_camY = iVal - 1.62;
		bHandled = TRUE;
	}
	else if (hObjSource == m_camZ)
	{
		double iVal = 0;
		wstring value = m_camZ.GetText();
		if(!value.empty()) iVal = _fromString<double>( value );
		currentPosition->m_camZ = iVal;
		bHandled = TRUE;
	}
	else if (hObjSource == m_yRot)
	{
		double iVal = 0;
		wstring value = m_yRot.GetText();
		if(!value.empty()) iVal = _fromString<double>( value );
		currentPosition->m_yRot = iVal;
		bHandled = TRUE;
	}
	else if (hObjSource == m_elevation)
	{
		double iVal = 0;
		wstring value = m_elevation.GetText();
		if(!value.empty()) iVal = _fromString<double>( value );
		currentPosition->m_elev = iVal;
		bHandled = TRUE;
	}

	return S_OK;
}
#endif