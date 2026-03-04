// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <assert.h>
#include "..\XUI\XUI_SocialPost.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\SharedConstants.h"
#include "..\..\..\Minecraft.World\Random.h"
#include "..\..\..\Minecraft.Client\SurvivalMode.h"
#include "..\..\..\Minecraft.Client\CreateWorldScreen.h"
#include "..\..\..\Minecraft.World\ConsoleSaveFileIO.h"
#include "..\..\..\Minecraft.World\AABB.h"
#include "..\..\..\Minecraft.World\Vec3.h"
#include "..\..\LocalPlayer.h"


//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_SocialPost::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_iPad = *(int *)pInitData->pvInitData;

	MapChildControls();
	
	XuiControlSetText(m_text,app.GetString(IDS_SOCIAL_TEXT));
	XuiControlSetText(m_LabelCaption,app.GetString(IDS_SOCIAL_LABEL_CAPTION));
	XuiControlSetText(m_EditCaption,app.GetString(IDS_SOCIAL_DEFAULT_CAPTION));
	XuiControlSetText(m_LabelDescription,app.GetString(IDS_SOCIAL_LABEL_DESCRIPTION));
	XuiControlSetText(m_EditDesc,app.GetString(IDS_SOCIAL_DEFAULT_DESCRIPTION));
	XuiControlSetText(m_OK,app.GetString(IDS_CONFIRM_OK));

	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);


	m_EditCaption.SetTextLimit(MAX_SOCIALPOST_CAPTION);
	m_EditDesc.SetTextLimit(MAX_SOCIALPOST_DESC);
	
	// Hardcoded so posts will have this in 
	m_wTitle = L"Minecraft: Xbox 360 Edition";

	m_EditCaption.SetTitleAndText(IDS_NAME_CAPTION,IDS_NAME_CAPTION_TEXT);
	m_EditDesc.SetTitleAndText(IDS_NAME_DESC,IDS_NAME_DESC_TEXT);

	wstring wCaption = m_EditCaption.GetText();
	wstring wDesc = m_EditDesc.GetText();

	// set the caret to the end of the default text
	m_EditCaption.SetCaretPosition((int)wCaption.length());
	m_EditDesc.SetCaretPosition((int)wDesc.length());
	
	BOOL bHasAllText = /*( wTitle.length()!=0) && */(wCaption.length()!=0) && (wDesc.length()!=0);

	m_OK.SetEnable(bHasAllText); 

	if(app.GetLocalPlayerCount()>1)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad,false);
	}
	
	TelemetryManager->RecordMenuShown(m_iPad, eUIScene_SocialPost, 0);

	return S_OK;
}


HRESULT CScene_SocialPost::OnNotifyValueChanged (HXUIOBJ hObjSource, XUINotifyValueChanged* pValueChangedData, BOOL& rfHandled)
{	wstring wCaption = m_EditCaption.GetText();
wstring wDesc = m_EditDesc.GetText();


	if(/*(hObjSource == m_EditTitle) || */(hObjSource == m_EditCaption) || (hObjSource == m_EditDesc))
	{
		//  Enable the done button when we have all of the necessary information
		//wstring wTitle = m_EditTitle.GetText();
		wstring wCaption = m_EditCaption.GetText();
		wstring wDesc = m_EditDesc.GetText();

		BOOL bHasAllText = /*( wTitle.length()!=0) &&*/ (wCaption.length()!=0) && (wDesc.length()!=0);
		m_OK.SetEnable(bHasAllText);        
	}

	return S_OK;
}

HRESULT CScene_SocialPost::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);

	if(pControlNavigateData->hObjDest==NULL)
	{
		pControlNavigateData->hObjDest=pControlNavigateData->hObjSource;
	}

	bHandled=TRUE;
	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_SocialPost::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);
	
	if(hObjPressed==m_OK)
	{
		CSocialManager::Instance()->SetSocialPostText(m_wTitle.c_str(),m_EditCaption.GetText(),m_EditDesc.GetText());
		
		CSocialManager::Instance()->PostImageToSocialNetwork( eFacebook, pNotifyPressData->UserIndex, false);	

		app.NavigateBack(pNotifyPressData->UserIndex);
	}

	return S_OK;
}


HRESULT CScene_SocialPost::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	//HXUIOBJ hFocus=XuiElementGetFocus();
	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_ESCAPE:
		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;

		break;
	}


	return S_OK;
}


HRESULT CScene_SocialPost::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining,false);
}