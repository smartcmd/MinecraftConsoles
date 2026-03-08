#include "stdafx.h"

#include <assert.h>
#include "../XUI/XUI_HelpControls.h"
#include "XUI_Ctrl_4JList.h"

#include "../../../Minecraft.World/net.minecraft.world.level.h"
#include "../../../Minecraft.World/LevelData.h"
#include "../../MultiPlayerLocalPlayer.h"

#define ALIGN_START		0
#define ALIGN_END		1
typedef struct 
{
	WCHAR wchName[20];
	int iAlign;
	D3DXVECTOR3 vPos;
}
CONTROLDETAILS;

LPCWSTR CScene_Controls::m_LayoutNameA[3]=
{
	L"1",
	L"2",
	L"3",
};
CONTROLDETAILS controlDetailsA[MAX_CONTROLS]=
{
	{
		L"FigA",ALIGN_END // _360_JOY_BUTTON_A
	},
	{
		L"FigB",ALIGN_END // _360_JOY_BUTTON_B
	},
	{
		L"FigX",ALIGN_END // _360_JOY_BUTTON_X
	},
	{
		L"FigY",ALIGN_END // _360_JOY_BUTTON_Y
	},
	{
		L"FigStart",ALIGN_END // _360_JOY_BUTTON_START
	},
	{
		L"FigBack",ALIGN_START // _360_JOY_BUTTON_BACK
	},
	{
		L"FigRB",ALIGN_END // _360_JOY_BUTTON_RB
	},
	{
		L"FigLB",ALIGN_START // _360_JOY_BUTTON_LB
	},
	{
		L"FigRStickButton",ALIGN_END // _360_JOY_BUTTON_RTHUMB
	},
	{
		L"FigLStickButton",ALIGN_START // _360_JOY_BUTTON_LTHUMB
	},
	// Move
	{
		L"FigRStick",ALIGN_END // _360_JOY_BUTTON_RSTICK_RIGHT
	},
	// Look
	{
		L"FigLStick",ALIGN_START // _360_JOY_BUTTON_LSTICK_RIGHT
	},
	{
		L"FigRT",ALIGN_END // RT
	},
	{
		L"FigLT",ALIGN_START // LT
	},
	{
		L"FigDpadR",ALIGN_START // DpadR
	},
	{
		L"FigDpadL",ALIGN_START // DpadL
	},
	{
		L"FigDpad",ALIGN_START // DpadL
	},

};

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_Controls::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	MapChildControls();
	XuiControlSetText(m_SouthPaw,app.GetString(IDS_SOUTHPAW));
	XuiControlSetText(m_InvertLook,app.GetString(IDS_INVERT_LOOK));

	m_iPad=*(int *)pInitData->pvInitData;
	// if we're not in the game, we need to use basescene 0 
	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);
	bool bSplitscreen=(app.GetLocalPlayerCount()>1);
	m_iCurrentTextIndex=0;
	m_bCreativeMode = !bNotInGame && Minecraft::GetInstance()->localplayers[m_iPad] && Minecraft::GetInstance()->localplayers[m_iPad]->abilities.mayfly;

	if(!bNotInGame)
	{
		// disable the build ver display
		m_BuildVer.SetShow(FALSE);
	}

	if(bSplitscreen)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad,32.0f);
		CXuiSceneBase::ShowLogo( m_iPad, FALSE );
	}
	else
	{
		if(bNotInGame)
		{
			CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, TRUE );
		}
		else
		{
			CXuiSceneBase::ShowLogo( m_iPad, TRUE );
		}
	}

	// if we're not in the game, we need to use basescene 0 
	if(bNotInGame)
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
	}
	else
	{
		ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
	}
	
	m_iSchemeTextA[0]=IDS_CONTROLS_SCHEME0;
	m_iSchemeTextA[1]=IDS_CONTROLS_SCHEME1;
	m_iSchemeTextA[2]=IDS_CONTROLS_SCHEME2;

	// get the figures
	HXUIOBJ hObj;
	HRESULT hr;

	// turn off all the figures to start with
	for(int i=0;i<MAX_CONTROLS;i++)
	{
		m_FigA[i].GetChildById(controlDetailsA[i].wchName,&hObj);
		hr=XuiElementGetPosition(hObj,&controlDetailsA[i].vPos);
		if(controlDetailsA[i].iAlign==ALIGN_END)
		{
			float fW,fH;
			hr=XuiElementGetBounds(hObj,&fW,&fH);
			controlDetailsA[i].vPos.x+=fW;
			controlDetailsA[i].vPos.y+=fH;
		}

		// if we're in splitscreen, half all the numbers
		if(bSplitscreen)
		{
			controlDetailsA[i].vPos.x/=2.0f;
			controlDetailsA[i].vPos.y/=2.0f;
		}
		m_FigA[i].SetShow(FALSE);
		m_TextPresenterA[i] = NULL;
	}

	// fill out the layouts list
	VOID *pObj;
	XuiObjectFromHandle( m_SchemeList, &pObj );
	m_pLayoutList = (CXuiCtrl4JList *)pObj;
	CXuiCtrl4JList::LIST_ITEM_INFO ListInfo[3];
	ZeroMemory(ListInfo,sizeof(CXuiCtrl4JList::LIST_ITEM_INFO)*3);

	for(int i=0;i<3;i++)
	{
		ListInfo[i].pwszText=m_LayoutNameA[i];
		ListInfo[i].hXuiBrush=NULL;
		ListInfo[i].fEnabled=TRUE;
		m_pLayoutList->AddData(ListInfo[i]);
	}

	m_bIgnoreNotifies=true;
	m_bIgnoreNotifies=false;
	int iSelected=app.GetGameSettings(m_iPad,eGameSetting_ControlScheme);
	XuiControlSetText(m_SchemeList,app.GetString(IDS_CONTROLS_LAYOUT));
	hr=m_pLayoutList->SetCurSelVisible(iSelected);
	m_iCurrentNavigatedControlsLayout=iSelected;

	LPWSTR layoutString = new wchar_t[ 128 ];
	swprintf( layoutString, 128, L"%ls : %ls", app.GetString( IDS_CURRENT_LAYOUT ),app.GetString(m_iSchemeTextA[iSelected]));	
	XuiControlSetText(m_CurrentLayout,layoutString);

	//PositionAllText(m_iPad);

	swprintf( layoutString, 128, L"%ls%ls", VER_PRODUCTVERSION_STR_W,app.DLCInstallProcessCompleted()?L"_DLC":L" ");	

	m_BuildVer.SetText(layoutString);
	delete [] layoutString;

	// Set check box initial states.
	BOOL bInvertLook = app.GetGameSettings(m_iPad,eGameSetting_ControlInvertLook);
	m_InvertLook.SetCheck( bInvertLook );

	BOOL bSouthPaw = app.GetGameSettings(m_iPad,eGameSetting_ControlSouthPaw);
	m_SouthPaw.SetCheck( bSouthPaw );

	return S_OK;
}



HRESULT CScene_Controls::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{

	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) return S_OK;

	if(pTransition->dwTransType == XUI_TRANSITION_TO || pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{	
		int iSelected=app.GetGameSettings(m_iPad,eGameSetting_ControlScheme);
		// and update the highlight on the current selection
		m_pLayoutList->SetBorder(iSelected,TRUE);

		PositionAllText(m_iPad);
	}

	return S_OK;
}

void CScene_Controls::PositionAllText(int iPad)
{
	// turn off all the figures to start with
	for(int i=0;i<MAX_CONTROLS;i++)
	{
		m_FigA[i].SetShow(FALSE);
	}

	m_iCurrentTextIndex=0;
	if(m_bCreativeMode)
	{
		PositionText(iPad,IDS_CONTROLS_JUMPFLY,MINECRAFT_ACTION_JUMP);
	}
	else
	{
		PositionText(iPad,IDS_CONTROLS_JUMP,MINECRAFT_ACTION_JUMP);
	}
	PositionText(iPad,IDS_CONTROLS_INVENTORY,MINECRAFT_ACTION_INVENTORY);
	PositionText(iPad,IDS_CONTROLS_PAUSE,MINECRAFT_ACTION_PAUSEMENU);
	if(m_bCreativeMode)
	{
		PositionText(iPad,IDS_CONTROLS_SNEAKFLY,MINECRAFT_ACTION_SNEAK_TOGGLE);
	}
	else
	{
		PositionText(iPad,IDS_CONTROLS_SNEAK,MINECRAFT_ACTION_SNEAK_TOGGLE);
	}
	PositionText(iPad,IDS_CONTROLS_USE,MINECRAFT_ACTION_USE);
	PositionText(iPad,IDS_CONTROLS_ACTION,MINECRAFT_ACTION_ACTION);
	PositionText(iPad,IDS_CONTROLS_HELDITEM,MINECRAFT_ACTION_RIGHT_SCROLL);
	PositionText(iPad,IDS_CONTROLS_HELDITEM,MINECRAFT_ACTION_LEFT_SCROLL);
	PositionText(iPad,IDS_CONTROLS_DROP,MINECRAFT_ACTION_DROP);
	PositionText(iPad,IDS_CONTROLS_CRAFTING,MINECRAFT_ACTION_CRAFTING);

	PositionText(iPad,IDS_CONTROLS_THIRDPERSON,MINECRAFT_ACTION_RENDER_THIRD_PERSON);
	PositionText(iPad,IDS_CONTROLS_PLAYERS,MINECRAFT_ACTION_GAME_INFO);

	// Swap for southpaw.
	if ( app.GetGameSettings(m_iPad,eGameSetting_ControlSouthPaw) )
	{
		// Move
		PositionText(iPad,IDS_CONTROLS_LOOK,MINECRAFT_ACTION_RIGHT);
		// Look
		PositionText(iPad,IDS_CONTROLS_MOVE,MINECRAFT_ACTION_LOOK_RIGHT);
	}
	else // Normal right handed.
	{
		// Move
		PositionText(iPad,IDS_CONTROLS_MOVE,MINECRAFT_ACTION_RIGHT);
		// Look
		PositionText(iPad,IDS_CONTROLS_LOOK,MINECRAFT_ACTION_LOOK_RIGHT);
	}

	// If we're in controls mode 1, and creative mode show the dpad for Creative Mode
	if(m_bCreativeMode && (m_iCurrentNavigatedControlsLayout==0))
	{
		PositionTextDirect(iPad,IDS_CONTROLS_DPAD,16,true);
	}
	else
	{
		PositionTextDirect(iPad,IDS_CONTROLS_DPAD,16,false);
	}
}

HRESULT CScene_Controls::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	HRESULT hr=S_OK;

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_ESCAPE:

		app.CheckGameSettingsChanged(true,pInputData->UserIndex);
		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
	break;
	}

	return hr;
}

void CScene_Controls::PositionText(int iPad,int iTextID, unsigned char ucAction)
{
	// position the text depending on the control id
	//int iTextWidth;
	XUIRect xuiRect;
	LPCWSTR pwchText;
	D3DXVECTOR3 vpos,vScale;
	HXUIOBJ hFigGroup;
	unsigned int uiVal=InputManager.GetGameJoypadMaps(m_iCurrentNavigatedControlsLayout,ucAction);

	// get the visual of the fig group and use any scaling in it to adjust the text positions
	XuiElementGetChildById(m_hObj,L"FigGroup",&hFigGroup);
	XuiElementGetScale(hFigGroup,&vScale);

	// check the width of the control with the text set in it
	if(m_TextPresenterA[m_iCurrentTextIndex]==NULL)
	{
 		HXUIOBJ hObj=NULL;
 		HRESULT hr=XuiControlGetVisual(m_TextA[m_iCurrentTextIndex].m_hObj,&hObj);
		hr=XuiElementGetChildById(hObj,L"Text",&m_TextPresenterA[m_iCurrentTextIndex]);
	}

	pwchText=app.GetString(iTextID);
	HRESULT hr=XuiTextPresenterMeasureText(m_TextPresenterA[m_iCurrentTextIndex], pwchText, &xuiRect);
	m_TextA[m_iCurrentTextIndex].SetBounds(xuiRect.right,xuiRect.bottom);

	int iControlDetailsIndex=0;
	switch(uiVal)
	{
	case _360_JOY_BUTTON_A:
		iControlDetailsIndex=0;
		break;
	case _360_JOY_BUTTON_B:
		iControlDetailsIndex=1;
		break;
	case _360_JOY_BUTTON_X:
		iControlDetailsIndex=2;
		break;
	case _360_JOY_BUTTON_Y:
		iControlDetailsIndex=3;
		break;
	case _360_JOY_BUTTON_START:
		iControlDetailsIndex=4;
		break;
	case _360_JOY_BUTTON_BACK:
		iControlDetailsIndex=5;
		break;
	case _360_JOY_BUTTON_RB:
		iControlDetailsIndex=6;
		break;
	case _360_JOY_BUTTON_LB:
		iControlDetailsIndex=7;
		break;
	case _360_JOY_BUTTON_RTHUMB:
		iControlDetailsIndex=8;
		break;	
 	case _360_JOY_BUTTON_LTHUMB:
 		iControlDetailsIndex=9;
 		break;	
		// Look
	case _360_JOY_BUTTON_RSTICK_RIGHT:
		iControlDetailsIndex=10;
		break;
		// Move
	case _360_JOY_BUTTON_LSTICK_RIGHT:
		iControlDetailsIndex=11;
		break;	
	case _360_JOY_BUTTON_RT:
		iControlDetailsIndex=12;
		break;
		// Move
	case _360_JOY_BUTTON_LT:
		iControlDetailsIndex=13;
		break;	
	case _360_JOY_BUTTON_DPAD_RIGHT:
		iControlDetailsIndex=14;
		break;	
	case _360_JOY_BUTTON_DPAD_LEFT:
		iControlDetailsIndex=15;
		break;	
	}

	hr=m_FigA[iControlDetailsIndex].SetShow(TRUE);
	// position the control depending on the text width

	if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
	{
		// 480 mode - we need to scale the text positions
		if(controlDetailsA[iControlDetailsIndex].iAlign==ALIGN_END)
		{
			vpos.x=(controlDetailsA[iControlDetailsIndex].vPos.x + 5.0f)* vScale.x;
		}
		else
		{
			vpos.x=(controlDetailsA[iControlDetailsIndex].vPos.x - 5.0f)* vScale.x - xuiRect.right;
		}

		vpos.y=(controlDetailsA[iControlDetailsIndex].vPos.y * vScale.y) - (xuiRect.bottom/2.0f);	
	}
	else
	{
		if(controlDetailsA[iControlDetailsIndex].iAlign==ALIGN_END)
		{
			vpos.x=(controlDetailsA[iControlDetailsIndex].vPos.x + 5.0f);
		}
		else
		{
			vpos.x=(controlDetailsA[iControlDetailsIndex].vPos.x - 5.0f) - xuiRect.right;
		}

		vpos.y=(controlDetailsA[iControlDetailsIndex].vPos.y ) - (xuiRect.bottom/2.0f);
	}

	vpos.x=floor(vpos.x);
	vpos.y=floor(vpos.y);
	vpos.z=0.0f;

	m_TextA[m_iCurrentTextIndex].SetPosition(&vpos);
	m_TextA[m_iCurrentTextIndex].SetText(pwchText);
	m_TextA[m_iCurrentTextIndex].SetShow(TRUE);
	m_iCurrentTextIndex++;
}

void CScene_Controls::PositionTextDirect(int iPad,int iTextID, int iControlDetailsIndex, bool bShow)
{
	// position the text depending on the control id
	//int iTextWidth;
	XUIRect xuiRect;
	LPCWSTR pwchText;
	D3DXVECTOR3 vpos,vScale;
	HXUIOBJ hFigGroup;

	if(bShow==false)
	{
		m_TextA[m_iCurrentTextIndex++].SetShow(FALSE);
		return;
	}
	// get the visual of the fig group and use any scaling in it to adjust the text positions
	XuiElementGetChildById(m_hObj,L"FigGroup",&hFigGroup);
	XuiElementGetScale(hFigGroup,&vScale);

	// check the width of the control with the text set in it	
	if(m_TextPresenterA[m_iCurrentTextIndex]==NULL)
	{
 		HXUIOBJ hObj=NULL;
 		HRESULT hr=XuiControlGetVisual(m_TextA[m_iCurrentTextIndex].m_hObj,&hObj);
		hr=XuiElementGetChildById(hObj,L"Text",&m_TextPresenterA[m_iCurrentTextIndex]);
	}

	pwchText=app.GetString(iTextID);
	HRESULT hr=XuiTextPresenterMeasureText(m_TextPresenterA[m_iCurrentTextIndex], pwchText, &xuiRect);
	m_TextA[m_iCurrentTextIndex].SetBounds(xuiRect.right,xuiRect.bottom);



	hr=m_FigA[iControlDetailsIndex].SetShow(TRUE);
	// position the control depending on the text width

	if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
	{
		// 480 mode - we need to scale the text positions
		if(controlDetailsA[iControlDetailsIndex].iAlign==ALIGN_END)
		{
			vpos.x=(controlDetailsA[iControlDetailsIndex].vPos.x + 5.0f)* vScale.x;
		}
		else
		{
			vpos.x=(controlDetailsA[iControlDetailsIndex].vPos.x - 5.0f)* vScale.x - xuiRect.right;
		}

		vpos.y=(controlDetailsA[iControlDetailsIndex].vPos.y * vScale.y) - (xuiRect.bottom/2.0f);	
	}
	else
	{
		if(controlDetailsA[iControlDetailsIndex].iAlign==ALIGN_END)
		{
			vpos.x=(controlDetailsA[iControlDetailsIndex].vPos.x + 5.0f);
		}
		else
		{
			vpos.x=(controlDetailsA[iControlDetailsIndex].vPos.x - 5.0f) - xuiRect.right;
		}

		vpos.y=(controlDetailsA[iControlDetailsIndex].vPos.y ) - (xuiRect.bottom/2.0f);
	}

	vpos.x=floor(vpos.x);
	vpos.y=floor(vpos.y);
	vpos.z=0.0f;

	m_TextA[m_iCurrentTextIndex].SetPosition(&vpos);
	m_TextA[m_iCurrentTextIndex].SetText(pwchText);
	m_TextA[m_iCurrentTextIndex].SetShow(TRUE);
	m_iCurrentTextIndex++;
}

HRESULT CScene_Controls::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	if( ( !m_bIgnoreNotifies )&&( hObjSource==m_pLayoutList->m_hObj ) )
	{
		m_iCurrentNavigatedControlsLayout=pNotifySelChangedData->iItem;
		PositionAllText(m_iPad);
	}
	if(pNotifySelChangedData->iOldItem!=-1 && hObjSource==m_SchemeList )
	{
		CXuiSceneBase::PlayUISFX(eSFX_Focus);
	}
	return S_OK;
}




HRESULT CScene_Controls::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	// Handle check box changes.
	if ( hObjPressed == m_InvertLook.m_hObj )
	{
		BOOL bIsChecked = m_InvertLook.IsChecked();
		app.SetGameSettings(m_iPad,eGameSetting_ControlInvertLook,(unsigned char)( bIsChecked ) );
	}
	else if ( hObjPressed == m_SouthPaw.m_hObj )
	{
		BOOL bIsChecked = m_SouthPaw.IsChecked();
		app.SetGameSettings(m_iPad,eGameSetting_ControlSouthPaw,(unsigned char)( bIsChecked ) );
		PositionAllText(m_iPad);
	}
	else if( hObjPressed == m_SchemeList)
	{
		// check what's been selected
		app.SetGameSettings(m_iPad,eGameSetting_ControlScheme,(unsigned char)m_SchemeList.GetCurSel());
		LPWSTR layoutString = new wchar_t[ 128 ];
		swprintf( layoutString, 128, L"%ls : %ls", app.GetString( IDS_CURRENT_LAYOUT ),app.GetString(m_iSchemeTextA[m_SchemeList.GetCurSel()]) );	
		
		XuiControlSetText(m_CurrentLayout,layoutString);
		delete [] layoutString;
		// and update the highlight on the current selection
		for(int i=0;i<m_pLayoutList->GetItemCount();i++)
		{
			m_pLayoutList->SetBorder(i,FALSE);
		}
		m_pLayoutList->SetBorder(m_SchemeList.GetCurSel(),TRUE);
	}
	return S_OK;
}

HRESULT CScene_Controls::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining,32.0f);
}
