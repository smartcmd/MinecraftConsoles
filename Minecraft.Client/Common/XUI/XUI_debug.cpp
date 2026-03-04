// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <assert.h>
#include "XUI_Debug.h"
#include "..\..\..\Minecraft.Client\StatsCounter.h"

LPCWSTR CScene_Debug::m_DebugCheckboxTextA[eDebugSetting_Max+1]=
{
	L"Load Saves From Local Folder Mode",
	L"Write Saves To Local Folder Mode",
	L"Freeze Players", //L"Not Used",
	L"Display Safe Area",
	L"Mobs don't attack",
	L"Freeze Time",
	L"Disable Weather",
	L"Craft Anything",
	L"Use DPad for debug",
	L"Mobs don't tick",
	L"Instant Mine",
	L"Show UI Console",
	L"Distributable Save",
	L"Debug Leaderboards",
	L"Height-Water-Biome Maps",
	L"Superflat nether",
	//L"Light/Dark background",
	L"More lightning when thundering",
	L"",
};

LPCWSTR CScene_Debug::m_DebugButtonTextA[eDebugButton_Max+1]=
{
	L"Award Theme",
	L"Award Avatar Item 1",
	L"Award Avatar Item 2",
	L"Award Avatar Item 3",
	L"Award Gamerpic 1",
	L"Award Gamerpic 2",
	L"Check Tips",
	L"Wipe Leaderboards",
	L"",
};



//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_Debug::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_iPad=*(int *)pInitData->pvInitData;

	// set text and enable any debug options required
	int iCheckboxIndex=0;
	int iButtonIndex=0;
	float fWidth=500.0f;
	float fX=240.0f;
	float fY=200.0f;
	float fYInc=5.0f;
	float fControlHeight=25.0f;

	if((!RenderManager.IsHiDef() && !RenderManager.IsWidescreen()) || app.GetLocalPlayerCount()>1)
	{
		fWidth=250.0f;
		fX=30.0f;
		fY=30.0f;
		fYInc=5.0f;
		fControlHeight=15.0f;
	}

	CXuiCheckbox *pCheckbox;
	CXuiControl *pButton;
	m_iTotalCheckboxElements=0;
	m_iTotalButtonElements=0;
	m_bOnCheckboxes=true;

	// count the items
	while((*m_DebugCheckboxTextA[m_iTotalCheckboxElements])!=0)
	{
		m_iTotalCheckboxElements++;
	}

	m_DebugCheckboxDataA= new DEBUGDATA [m_iTotalCheckboxElements];

	while((*m_DebugCheckboxTextA[iCheckboxIndex])!=0)
	{
			XuiCreateObject( XUI_CLASS_CHECKBOX, &m_DebugCheckboxDataA[iCheckboxIndex].hXuiObj );
			XuiObjectFromHandle( m_DebugCheckboxDataA[iCheckboxIndex].hXuiObj, &m_DebugCheckboxDataA[iCheckboxIndex].pvData );

			pCheckbox = (CXuiCheckbox *)m_DebugCheckboxDataA[iCheckboxIndex].pvData;
			//m_phXuiObjA[iElementIndex] = pCheckbox->m_hObj;
			pCheckbox->SetText(m_DebugCheckboxTextA[iCheckboxIndex]);
			pCheckbox->SetShow(TRUE);

			D3DXVECTOR3 vPos;
			float tx,ty;
			pCheckbox->GetBounds(&tx,&ty);
			pCheckbox->SetBounds(fWidth,fControlHeight);
			vPos.x=fX;
			vPos.y=fY;
			vPos.z=0.0f;
			pCheckbox->SetPosition(&vPos);
			fY+=fControlHeight+fYInc;

			XuiElementAddChild(m_hObj,m_DebugCheckboxDataA[iCheckboxIndex].hXuiObj);

		iCheckboxIndex++;
	}
	
	// and the buttons
	if((!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())|| app.GetLocalPlayerCount()>1)
	{
		fWidth=260.0f;
		fX=300.0f;
		fY=30.0f;
		fControlHeight=40.0f;
	}
	else
	{
		fWidth=350.0f;
		fX=750.0f;
		fY=200.0f;
		fControlHeight=40.0f;
	}



	while((*m_DebugButtonTextA[m_iTotalButtonElements])!=0)
	{
		m_iTotalButtonElements++;
	}

	m_DebugButtonDataA= new DEBUGDATA [m_iTotalButtonElements];

	while((*m_DebugButtonTextA[iButtonIndex])!=0)
	{
		XuiCreateObject( XUI_CLASS_BUTTON, &m_DebugButtonDataA[iButtonIndex].hXuiObj );
		XuiObjectFromHandle( m_DebugButtonDataA[iButtonIndex].hXuiObj, &m_DebugButtonDataA[iButtonIndex].pvData );

		pButton = (CXuiControl *)m_DebugButtonDataA[iButtonIndex].pvData;
		//m_phXuiObjA[iElementIndex] = pCheckbox->m_hObj;
		pButton->SetText(m_DebugButtonTextA[iButtonIndex]);
		pButton->SetShow(TRUE);

		D3DXVECTOR3 vPos;
		float tx,ty;
		pButton->GetBounds(&tx,&ty);
		pButton->SetBounds(fWidth,fControlHeight);
		vPos.x=fX;
		vPos.y=fY;
		vPos.z=0.0f;
		pButton->SetPosition(&vPos);
		fY+=fControlHeight+fYInc;

		XuiElementAddChild(m_hObj,m_DebugButtonDataA[iButtonIndex].hXuiObj);

		// if you're not the primary player, disable all these - you need a storage device for them

		if(ProfileManager.GetPrimaryPad()!=m_iPad)
		{
			XuiControlSetEnable(m_DebugButtonDataA[iButtonIndex].hXuiObj,FALSE);
		}
		iButtonIndex++;
	}



	unsigned int uiDebugBitmask=app.GetGameSettingsDebugMask(m_iPad);

	for(int i=0;i<iCheckboxIndex;i++)
	{
		CXuiCheckbox *pCheckbox = (CXuiCheckbox *)m_DebugCheckboxDataA[i].pvData;
		pCheckbox->SetCheck( (uiDebugBitmask&(1<<i)) ?TRUE:FALSE);
	}


	if(m_DebugCheckboxDataA[0].hXuiObj)
	{
		XuiElementSetUserFocus(m_DebugCheckboxDataA[0].hXuiObj,m_iPad);
	}

	m_iCurrentCheckboxElement=0;
	m_iCurrentButtonElement=0;

#ifdef _DEBUG
	// testing the upload of the player custom skin
//  	wstring wsTemp;
//  
// 	wsTemp=L"Deadmau5_Skin.png";
// 	app.UploadFileToGlobalStorage(m_iPad,C4JStorage::eGlobalStorage_TitleUser,&wsTemp);
// 	wsTemp=L"Mojang_Cape.png";
// 	app.UploadFileToGlobalStorage(m_iPad,C4JStorage::eGlobalStorage_TitleUser,&wsTemp);
// 	wsTemp=L"4J_Cape.png";
// 	app.UploadFileToGlobalStorage(m_iPad,C4JStorage::eGlobalStorage_TitleUser,&wsTemp);
#endif

	return S_OK;
}

HRESULT CScene_Debug::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	HRESULT hr=S_OK;
	int iCurrentBitmaskIndex=0;
	unsigned int uiDebugBitmask=0L;

	// Explicitly handle X button presses
	if (pInputData->dwKeyCode == VK_PAD_B)
	{
		// check all the settings
		for(int i=0;i<m_iTotalCheckboxElements;i++)
		{
			CXuiCheckbox *pCheckbox = (CXuiCheckbox *)m_DebugCheckboxDataA[i].pvData;
			uiDebugBitmask|=pCheckbox->IsChecked()?(1<<iCurrentBitmaskIndex):0;
			iCurrentBitmaskIndex++;
		}

		if(uiDebugBitmask!=app.GetGameSettingsDebugMask(pInputData->UserIndex))
		{
			app.SetGameSettingsDebugMask(pInputData->UserIndex,uiDebugBitmask);
			if(app.DebugSettingsOn())
			{
				app.ActionDebugMask(pInputData->UserIndex);		
			}
			else
			{
				// force debug mask off
				app.ActionDebugMask(pInputData->UserIndex,true);
			}

			app.CheckGameSettingsChanged(true,pInputData->UserIndex);
		}
		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
	}


	return hr;
}


HRESULT CScene_Debug::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	if(m_bOnCheckboxes)
	{
		// If it's not from the current control, ignore it
		if(m_DebugCheckboxDataA[m_iCurrentCheckboxElement].hXuiObj!=pControlNavigateData->hObjSource) return S_OK;

		switch(pControlNavigateData->nControlNavigate)
		{
		case XUI_CONTROL_NAVIGATE_UP:
			if(m_iCurrentCheckboxElement>0)
			{
				m_iCurrentCheckboxElement--;
				XuiElementSetUserFocus(m_DebugCheckboxDataA[m_iCurrentCheckboxElement].hXuiObj, m_iPad);
			}
			break;	
		case XUI_CONTROL_NAVIGATE_DOWN:
			if((m_iCurrentCheckboxElement+1)<m_iTotalCheckboxElements)
			{
				m_iCurrentCheckboxElement++;
				XuiElementSetUserFocus(m_DebugCheckboxDataA[m_iCurrentCheckboxElement].hXuiObj, m_iPad);
			}
			break;
		case XUI_CONTROL_NAVIGATE_RIGHT:
			m_bOnCheckboxes=false;
			if(m_iCurrentCheckboxElement<m_iTotalButtonElements)
			{
				m_iCurrentButtonElement=m_iCurrentCheckboxElement;
			}
			XuiElementSetUserFocus(m_DebugButtonDataA[m_iCurrentButtonElement].hXuiObj, m_iPad);
			break;	
		}
	}
	else
	{
		// If it's not from the current control, ignore it
		if(m_DebugButtonDataA[m_iCurrentButtonElement].hXuiObj!=pControlNavigateData->hObjSource) return S_OK;

		switch(pControlNavigateData->nControlNavigate)
		{
		case XUI_CONTROL_NAVIGATE_UP:
			if(m_iCurrentButtonElement>0)
			{
				m_iCurrentButtonElement--;
				XuiElementSetUserFocus(m_DebugButtonDataA[m_iCurrentButtonElement].hXuiObj, m_iPad);
			}
			break;	
		case XUI_CONTROL_NAVIGATE_DOWN:
			if((m_iCurrentButtonElement+1)<m_iTotalButtonElements)
			{
				m_iCurrentButtonElement++;
				XuiElementSetUserFocus(m_DebugButtonDataA[m_iCurrentButtonElement].hXuiObj, m_iPad);
			}
			break;

		case XUI_CONTROL_NAVIGATE_LEFT:
			m_bOnCheckboxes=true;
			if(m_iCurrentButtonElement<m_iTotalCheckboxElements)
			{
				m_iCurrentCheckboxElement=m_iCurrentButtonElement;
			}
			XuiElementSetUserFocus(m_DebugCheckboxDataA[m_iCurrentCheckboxElement].hXuiObj, m_iPad);
			break;
		}
	}
	


	return S_OK;
}
//----------------------------------------------------------------------------------
// Updates the UI when the list selection changes.
//----------------------------------------------------------------------------------
HRESULT CScene_Debug::OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChanged, BOOL& bHandled )
{

	return S_OK;
}
//----------------------------------------------------------------------------------
// Updates the UI when the list selection changes.
//----------------------------------------------------------------------------------
HRESULT CScene_Debug::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{


	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_Debug::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	int iButton=0;

	while((iButton<m_iTotalButtonElements) && m_DebugButtonDataA[iButton].hXuiObj!=hObjPressed)
	{
		iButton++;
	}

	if(iButton==m_iTotalButtonElements)
	{
		// it's not a button then
		return S_OK;
	}

	// you need to be the primary player to be able to earn these, since you need a storage device

	switch(iButton)
	{
	case eDebugButton_Theme:
		ProfileManager.Award( pNotifyPressData->UserIndex, eAward_socialPost );
		break;
	case eDebugButton_Avatar_Item_1:
		ProfileManager.Award( pNotifyPressData->UserIndex, eAward_eatPorkChop );
		break;
	case eDebugButton_Avatar_Item_2:
		ProfileManager.Award( pNotifyPressData->UserIndex, eAward_play100Days );
		break;
	case eDebugButton_Avatar_Item_3:
		ProfileManager.Award( pNotifyPressData->UserIndex, eAward_arrowKillCreeper );
		break;
	case eDebugButton_Gamerpic_1:
		ProfileManager.Award( pNotifyPressData->UserIndex, eAward_mine100Blocks );
		break;
	case eDebugButton_Gamerpic_2:
		ProfileManager.Award( pNotifyPressData->UserIndex, eAward_kill10Creepers );
		break;
	case eDebugButton_CheckTips:
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_DebugTips);
		break;

	case eDebugButton_WipeLeaderboards:
#ifdef _DEBUG
// commenting out so it doesn't get done by mistake
//		Minecraft::GetInstance()->stats[pNotifyPressData->UserIndex]->WipeLeaderboards();
#endif
		break;

	}

	return S_OK;
}

