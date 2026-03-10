#include "stdafx.h"
#include "..\..\Common\Tutorial\Tutorial.h"
#include "..\..\MultiplayerLocalPlayer.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\Tile.h"
#include "..\..\..\Minecraft.World\Item.h"
#include "XUI_Ctrl_CraftIngredientSlot.h"
#include "XUI_XZP_Icons.h"

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_TutorialPopup::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	HRESULT hr = S_OK;

	tutorial = static_cast<Tutorial *>(pInitData->pvInitData);
	m_iPad = tutorial->getPad();

	MapChildControls();

	// splitscreen?
	// if we are in splitscreen, then we need to figure out if we want to move this scene
	if(app.GetLocalPlayerCount()>1)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);	
	}

	m_textFontSize = _fromString<int>( m_fontSizeControl.GetText() );
	m_fontSizeControl.SetShow(false);

	m_interactScene = nullptr;
	m_lastSceneMovedLeft = false;
	m_bAllowFade = false;

	SetShow(false);
	XuiSetTimer(m_hObj,TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID,TUTORIAL_POPUP_MOVE_SCENE_TIME);

	return hr;
}

HRESULT CScene_TutorialPopup::OnDestroy()
{
	return S_OK;
}

HRESULT CScene_TutorialPopup::OnTimer(XUIMessageTimer *pData,BOOL& rfHandled)
{
	if( pData->nId == TUTORIAL_POPUP_FADE_TIMER_ID )
	{
		XuiKillTimer(m_hObj,TUTORIAL_POPUP_FADE_TIMER_ID);
		SetShow( false );
		XuiSetTimer(m_hObj,TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID,TUTORIAL_POPUP_MOVE_SCENE_TIME);
	}
	else if( pData->nId == TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID )
	{
		UpdateInteractScenePosition(IsShown() == TRUE);
		XuiKillTimer(m_hObj,TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID);
	}

	return S_OK;
}

void CScene_TutorialPopup::UpdateInteractScenePosition(bool visible)
{
	if( m_interactScene == nullptr ) return;

	// 4J-PB - check this players screen section to see if we should allow the animation
	bool bAllowAnim=false;
	HXUICLASS sceneClass = XuiGetObjectClass( m_interactScene->m_hObj );

	HXUICLASS inventoryClass = XuiFindClass( L"CXuiSceneInventory" );
	HXUICLASS furnaceClass = XuiFindClass( L"CXuiSceneFurnace" );
	HXUICLASS craftingClass = XuiFindClass( L"CXuiSceneCraftingPanel" );
	HXUICLASS trapClass = XuiFindClass( L"CXuiSceneTrap" );
	HXUICLASS containerClass = XuiFindClass( L"CXuiSceneContainer" );
	HXUICLASS creativeInventoryClass = XuiFindClass( L"CXuiSceneInventoryCreative" );
	HXUICLASS enchantingClass = XuiFindClass( L"CXuiSceneEnchant" );
	HXUICLASS brewingClass = XuiFindClass( L"CXuiSceneBrewingStand" );
	HXUICLASS anvilClass = XuiFindClass( L"CXuiSceneAnvil" );
	HXUICLASS tradingClass = XuiFindClass( L"CXuiSceneTrading" );
	BOOL isCraftingScene = XuiClassDerivesFrom( sceneClass, craftingClass );
	BOOL isCreativeScene = XuiClassDerivesFrom( sceneClass, creativeInventoryClass );

	switch(Minecraft::GetInstance()->localplayers[m_iPad]->m_iScreenSection)
	{
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
		bAllowAnim=true;
		break;
	default:
		// anim allowed for everything except the crafting 2x2 and 3x3, and the creative menu
		if(!isCraftingScene && !isCreativeScene)
		{
			bAllowAnim=true;
		}
		break;
	}

	if(bAllowAnim)
	{

		XUITimeline *timeline;
		XUINamedFrame *startFrame;
		XUINamedFrame *endFrame;
		bool movingLeft = false;
		// Also returns TRUE if they are the same (which is what we want)
		if( XuiClassDerivesFrom( sceneClass, inventoryClass ) ||
			XuiClassDerivesFrom( sceneClass, furnaceClass ) ||
			XuiClassDerivesFrom( sceneClass, trapClass ) ||
			XuiClassDerivesFrom( sceneClass, containerClass ) ||
			XuiClassDerivesFrom( sceneClass, enchantingClass ) ||
			XuiClassDerivesFrom( sceneClass, brewingClass ) ||
			XuiClassDerivesFrom( sceneClass, anvilClass ) ||
			XuiClassDerivesFrom( sceneClass, tradingClass ) ||
			isCreativeScene ||
			isCraftingScene == TRUE )
		{
			XuiElementGetTimeline( m_interactScene->m_hObj, &timeline);
			if(visible)
			{
				startFrame = timeline->FindNamedFrame( L"MoveLeft" );
				endFrame = timeline->FindNamedFrame( L"EndMoveLeft" );
				movingLeft = true;
			}
			else
			{
				startFrame = timeline->FindNamedFrame( L"MoveRight" );
				endFrame = timeline->FindNamedFrame( L"EndMoveRight" );
			}

			if( (m_lastInteractSceneMoved != m_interactScene && movingLeft) || ( m_lastInteractSceneMoved == m_interactScene && m_lastSceneMovedLeft != movingLeft ) )
			{
				timeline->Play( startFrame->m_dwFrame, startFrame->m_dwFrame, endFrame->m_dwFrame, FALSE, FALSE );

				m_lastInteractSceneMoved = m_interactScene;
				m_lastSceneMovedLeft = movingLeft;
			}
		}
	}

}

HRESULT CScene_TutorialPopup::_SetDescription(CXuiScene *interactScene, LPCWSTR desc, LPCWSTR title, bool allowFade, bool isReminder)
{	
	HRESULT hr = S_OK;
	m_interactScene = interactScene;
	if( interactScene != m_lastInteractSceneMoved ) m_lastInteractSceneMoved = nullptr;
	if(desc == nullptr)
	{
		SetShow( false );
		XuiSetTimer(m_hObj,TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID,TUTORIAL_POPUP_MOVE_SCENE_TIME);
		XuiKillTimer(m_hObj,TUTORIAL_POPUP_FADE_TIMER_ID);
	}
	else
	{
		SetShow( true );
		XuiSetTimer(m_hObj,TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID,TUTORIAL_POPUP_MOVE_SCENE_TIME);

		if( allowFade )
		{
			//Initialise a timer to fade us out again
			XuiSetTimer(m_hObj,TUTORIAL_POPUP_FADE_TIMER_ID,tutorial->GetTutorialDisplayMessageTime());
		}
		else
		{
			XuiKillTimer(m_hObj,TUTORIAL_POPUP_FADE_TIMER_ID);
		}
		m_bAllowFade = allowFade;

		if(isReminder)
		{
			wstring text(app.GetString( IDS_TUTORIAL_REMINDER ));
			text.append( desc );
			// set the text colour
			wchar_t formatting[40];
			swprintf(formatting, 40, L"<font color=\"#%08x\" size=\"%d\">",app.GetHTMLColour(eHTMLColor_White),m_textFontSize);
			text = formatting + text;

			hr = m_description.SetText( text.c_str() );
		}
		else
		{
			wstring text(desc);
			// set the text colour
			wchar_t formatting[40];
			swprintf(formatting, 40, L"<font color=\"#%08x\" size=\"%d\">",app.GetHTMLColour(eHTMLColor_White),m_textFontSize);
			text = formatting + text;

			hr = m_description.SetText( text.c_str() );
		}

		D3DXVECTOR3 titlePos;
		hr = XuiElementGetPosition( m_title, &titlePos );

		BOOL titleShowAtStart = m_title.IsShown();
		if( title != nullptr && title[0] != 0 )
		{
			m_title.SetText( title );
			m_title.SetShow(TRUE);
		}
		else
		{
			m_title.SetText( L"" );
			m_title.SetShow(FALSE);
		}
		BOOL titleShowAtEnd = m_title.IsShown();
		if(titleShowAtStart != titleShowAtEnd)
		{
			float fHeight, fWidth, fTitleHeight, fDescHeight, fDescWidth;
			m_title.GetBounds(&fWidth,&fTitleHeight);
			GetBounds(&fWidth,&fHeight);


			// 4J Stu - For some reason when we resize the scene it resets the size of the HTML control
			// We don't want that to happen, so get it's size before and set it back after
			m_description.GetBounds(&fDescWidth,&fDescHeight);
			if(titleShowAtEnd)
			{
				titlePos.y += fTitleHeight;

				SetBounds(fWidth, fHeight + fTitleHeight);
			}
			else
			{
				SetBounds(fWidth, fHeight - fTitleHeight);
			}
			XuiElementSetPosition( m_description, &titlePos );
			m_description.SetBounds(fDescWidth, fDescHeight);
		}

		// Check if we need to resize the box
		XUIContentDims contentDims;
		m_description.GetContentDims(&contentDims);

		int heightDiff = contentDims.nContentHeight - contentDims.nPageHeight;

		float fHeight, fWidth;
		GetBounds(&fWidth,&fHeight);
		SetBounds(fWidth, fHeight + heightDiff);

		m_description.GetBounds(&fWidth,&fHeight);
		m_description.SetBounds(fWidth, static_cast<float>(contentDims.nPageHeight + heightDiff));
	}
	return hr;
}

HRESULT CScene_TutorialPopup::SetDescription(int iPad, TutorialPopupInfo *info)
{
	HRESULT hr = S_OK;

#ifdef _XBOX
	HXUIOBJ hObj = app.GetCurrentTutorialScene(iPad);
	HXUICLASS thisClass = XuiFindClass( L"CScene_TutorialPopup" );
	HXUICLASS objClass = XuiGetObjectClass( hObj );

	// Also returns TRUE if they are the same (which is what we want)
	if( XuiClassDerivesFrom( objClass, thisClass ) )
	{

		CScene_TutorialPopup *pThis;
		hr = XuiObjectFromHandle(hObj, (void **) &pThis);
		if (FAILED(hr))
			return hr;

		wstring parsed = pThis->_SetIcon(info->icon, info->iAuxVal, info->isFoil, info->desc);
		parsed = pThis->_SetImage( parsed );
		parsed = CScene_TutorialPopup::ParseDescription(iPad, parsed);
		if(parsed.empty())
		{
			hr = pThis->_SetDescription( info->interactScene, nullptr, nullptr, info->allowFade, info->isReminder );
		}
		else
		{
			hr = pThis->_SetDescription( info->interactScene, parsed.c_str(), info->title, info->allowFade, info->isReminder );
		}
	}
#endif
	return hr;
}

wstring CScene_TutorialPopup::_SetIcon(int icon, int iAuxVal, bool isFoil, LPCWSTR desc)
{
	wstring temp(desc);

	BOOL iconShowAtStart = m_pCraftingPic->IsShown();

	if( icon != TUTORIAL_NO_ICON )
	{
		bool itemIsFoil = false;
		itemIsFoil = (std::make_shared<ItemInstance>(icon, 1, iAuxVal))->isFoil();
		if(!itemIsFoil) itemIsFoil = isFoil;

		m_pCraftingPic->SetIcon(m_iPad, icon,iAuxVal,1,10,31,false,itemIsFoil);
	}
	else
	{
		wstring openTag(L"{*ICON*}");
		wstring closeTag(L"{*/ICON*}");
		size_t iconTagStartPos = temp.find(openTag);
		size_t iconStartPos = iconTagStartPos + openTag.length();
		if( iconTagStartPos > 0 && iconStartPos < temp.length() )
		{
			size_t iconEndPos = temp.find(closeTag, iconStartPos);

			if(iconEndPos > iconStartPos && iconEndPos < temp.length() )
			{
				wstring id = temp.substr(iconStartPos, iconEndPos - iconStartPos);

				vector<wstring> idAndAux = stringSplit(id,L':');

				int iconId = _fromString<int>(idAndAux[0]);

				if(idAndAux.size() > 1)
				{
					iAuxVal = _fromString<int>(idAndAux[1]);
				}
				else
				{
					iAuxVal = 0;
				}

				bool itemIsFoil = false;
				itemIsFoil = (std::make_shared<ItemInstance>(iconId, 1, iAuxVal))->isFoil();
				if(!itemIsFoil) itemIsFoil = isFoil;

				m_pCraftingPic->SetIcon(m_iPad, iconId,iAuxVal,1,10,31,false,itemIsFoil);

				temp.replace(iconTagStartPos, iconEndPos - iconTagStartPos + closeTag.length(), L"");
			}
		}
	
		// remove any icon text
		else if(temp.find(L"{*CraftingTableIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Tile::workBench->id,0,1,10,31,false);
		}
		else if(temp.find(L"{*SticksIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Item::stick->id,0,1,10,31,false);
		}
		else if(temp.find(L"{*PlanksIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Tile::wood->id,0,1,10,31,false);
		}
		else if(temp.find(L"{*WoodenShovelIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Item::shovel_wood->id,0,1,10,31,false);
		}
		else if(temp.find(L"{*WoodenHatchetIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Item::hatchet_wood->id,0,1,10,31,false);
		}
		else if(temp.find(L"{*WoodenPickaxeIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Item::pickAxe_wood->id,0,1,10,31,false);
		}
		else if(temp.find(L"{*FurnaceIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Tile::furnace_Id,0,1,10,31,false);
		}
		else if(temp.find(L"{*WoodenDoorIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Item::door_wood->id,0,1,10,31,false);
		}
		else if(temp.find(L"{*TorchIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Tile::torch_Id,0,1,10,31,false);
		}
		else if(temp.find(L"{*BoatIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Item::boat_Id,0,1,10,31,false);
		}
		else if(temp.find(L"{*FishingRodIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Item::fishingRod_Id,0,1,10,31,false);
		}
		else if(temp.find(L"{*FishIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Item::fish_raw_Id,0,1,10,31,false);
		}
		else if(temp.find(L"{*MinecartIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Item::minecart_Id,0,1,10,31,false);
		}
		else if(temp.find(L"{*RailIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Tile::rail_Id,0,1,10,31,false);
		}
		else if(temp.find(L"{*PoweredRailIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Tile::goldenRail_Id,0,1,10,31,false);
		}
		else if(temp.find(L"{*StructuresIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, XZP_ICON_STRUCTURES,0,1,10,31,false);
		}
		else if(temp.find(L"{*ToolsIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, XZP_ICON_TOOLS,0,1,10,31,false);
		}
		else if(temp.find(L"{*StoneIcon*}")!=wstring::npos)
		{
			m_pCraftingPic->SetIcon(m_iPad, Tile::rock_Id,0,1,10,31,false);
		}
		else
		{
			// hide the icon slot
			m_pCraftingPic->SetIcon(m_iPad, 0,0,0,0,0,false,false,FALSE);
		}
	}
	
	BOOL iconShowAtEnd = m_pCraftingPic->IsShown();
	if(iconShowAtStart != iconShowAtEnd)
	{
		float fHeight, fWidth, fIconHeight, fDescHeight, fDescWidth;
		m_pCraftingPic->GetBounds(&fWidth,&fIconHeight);
		GetBounds(&fWidth,&fHeight);


		// 4J Stu - For some reason when we resize the scene it resets the size of the HTML control
		// We don't want that to happen, so get it's size before and set it back after
		m_description.GetBounds(&fDescWidth,&fDescHeight);
		if(iconShowAtEnd)
		{
			SetBounds(fWidth, fHeight + fIconHeight);
		}
		else
		{
			SetBounds(fWidth, fHeight - fIconHeight);
		}
		m_description.SetBounds(fDescWidth, fDescHeight);
	}
	
	return temp;
}

wstring CScene_TutorialPopup::_SetImage(wstring &desc)
{

	BOOL imageShowAtStart = m_image.IsShown();

	wstring openTag(L"{*IMAGE*}");
	wstring closeTag(L"{*/IMAGE*}");
	size_t imageTagStartPos = desc.find(openTag);
	size_t imageStartPos = imageTagStartPos + openTag.length();
	if( imageTagStartPos > 0 && imageStartPos < desc.length() )
	{
		size_t imageEndPos = desc.find(closeTag, imageStartPos);

		if(imageEndPos > imageStartPos && imageEndPos < desc.length() )
		{
			wstring id = desc.substr(imageStartPos, imageEndPos - imageStartPos);
			m_image.SetImagePath( id.c_str() );
			m_image.SetShow( TRUE );

			desc.replace(imageTagStartPos, imageEndPos - imageTagStartPos + closeTag.length(), L"");
		}
	}
	else
	{
		// hide the icon slot
		m_image.SetShow( FALSE );
	}
	
	BOOL imageShowAtEnd = m_image.IsShown();
	if(imageShowAtStart != imageShowAtEnd)
	{
		float fHeight, fWidth, fIconHeight, fDescHeight, fDescWidth;
		m_image.GetBounds(&fWidth,&fIconHeight);
		GetBounds(&fWidth,&fHeight);


		// 4J Stu - For some reason when we resize the scene it resets the size of the HTML control
		// We don't want that to happen, so get it's size before and set it back after
		m_description.GetBounds(&fDescWidth,&fDescHeight);
		if(imageShowAtEnd)
		{
			SetBounds(fWidth, fHeight + fIconHeight);
		}
		else
		{
			SetBounds(fWidth, fHeight - fIconHeight);
		}
		m_description.SetBounds(fDescWidth, fDescHeight);
	}
	
	return desc;
}


wstring CScene_TutorialPopup::ParseDescription(int iPad, wstring &text)
{
	text = replaceAll(text, L"{*CraftingTableIcon*}", L"");
	text = replaceAll(text, L"{*SticksIcon*}", L"");
	text = replaceAll(text, L"{*PlanksIcon*}", L"");
	text = replaceAll(text, L"{*WoodenShovelIcon*}", L"");
	text = replaceAll(text, L"{*WoodenHatchetIcon*}", L"");
	text = replaceAll(text, L"{*WoodenPickaxeIcon*}", L"");
	text = replaceAll(text, L"{*FurnaceIcon*}", L"");
	text = replaceAll(text, L"{*WoodenDoorIcon*}", L"");
	text = replaceAll(text, L"{*TorchIcon*}", L"");
	text = replaceAll(text, L"{*MinecartIcon*}", L"");
	text = replaceAll(text, L"{*BoatIcon*}", L"");
	text = replaceAll(text, L"{*FishingRodIcon*}", L"");
	text = replaceAll(text, L"{*FishIcon*}", L"");
	text = replaceAll(text, L"{*RailIcon*}", L"");
	text = replaceAll(text, L"{*PoweredRailIcon*}", L"");
	text = replaceAll(text, L"{*StructuresIcon*}", L"");
	text = replaceAll(text, L"{*ToolsIcon*}", L"");
	text = replaceAll(text, L"{*StoneIcon*}", L"");

	if( app.GetLocalPlayerCount() > 1 )
	{
		// TODO TU-1 - This should really be a string as well rather than hardcoded here
		text = replaceAll(text, L"{*EXIT_PICTURE*}",					L"<div align=\"center\"><img height=\"90\" width=\"160\" src=\"TutorialExitScreenshot.png\"/></div>" );

		// TODO TU-1 - This should also be separate strings, or move these things out of a localisable file so we can add/change them at will
		text = replaceAll(text, L"height=\"30\" width=\"30\"", L"height=\"20\" width=\"20\"");
	}
	else
	{
		text = replaceAll(text, L"{*EXIT_PICTURE*}",					app.GetString( IDS_TUTORIAL_HTML_EXIT_PICTURE ) );
	}
		/*
#define MINECRAFT_ACTION_RENDER_DEBUG		ACTION_INGAME_13
#define MINECRAFT_ACTION_PAUSEMENU			ACTION_INGAME_15
#define MINECRAFT_ACTION_SNEAK_TOGGLE		ACTION_INGAME_17
	*/

	return app.FormatHTMLString(iPad,text);
}

HRESULT CScene_TutorialPopup::_SetVisible(bool show)
{
	HRESULT hr = this->SetShow( show );

	if( show && m_bAllowFade )
	{
		//Initialise a timer to fade us out again
		XuiSetTimer(m_hObj,TUTORIAL_POPUP_FADE_TIMER_ID,tutorial->GetTutorialDisplayMessageTime());
	}

	return hr;
}

bool CScene_TutorialPopup::_IsSceneVisible()
{
	bool isVisible = this->IsShown()==TRUE?true:false;

	return isVisible;
}

HRESULT CScene_TutorialPopup::SetSceneVisible(int iPad, bool show)
{
	HRESULT hr = S_OK;

	HXUIOBJ hObj = app.GetCurrentTutorialScene(iPad);
	HXUICLASS thisClass = XuiFindClass( L"CScene_TutorialPopup" );
	HXUICLASS objClass = XuiGetObjectClass( hObj );

	// Also returns TRUE if they are the same (which is what we want)
	if( XuiClassDerivesFrom( objClass, thisClass ) )
	{
		CScene_TutorialPopup *pThis;
		hr = XuiObjectFromHandle(hObj, static_cast<void **>(&pThis));
		if (FAILED(hr))
			return hr;

		hr = pThis->_SetVisible( show );
	}
	return hr;
}


HRESULT CScene_TutorialPopup::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}

bool CScene_TutorialPopup::IsSceneVisible(int iPad)
{
	bool isVisible = false;
	HRESULT hr;

	HXUIOBJ hObj = app.GetCurrentTutorialScene(iPad);
	HXUICLASS thisClass = XuiFindClass( L"CScene_TutorialPopup" );
	HXUICLASS objClass = XuiGetObjectClass( hObj );

	// Also returns TRUE if they are the same (which is what we want)
	if( XuiClassDerivesFrom( objClass, thisClass ) )
	{
		CScene_TutorialPopup *pThis;
		hr = XuiObjectFromHandle(hObj, static_cast<void **>(&pThis));
		if (FAILED(hr))
			return false;

		isVisible = pThis->_IsSceneVisible();
	}
	return isVisible;
}