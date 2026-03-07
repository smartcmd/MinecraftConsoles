#include "stdafx.h"
#include "UI.h"
#include "UIComponent_TutorialPopup.h"
#include "../../Common/Tutorial/Tutorial.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../Minecraft.h"
#include "../../../Minecraft.World/net.minecraft.world.level.tile.h"

UIComponent_TutorialPopup::UIComponent_TutorialPopup(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_interactScene = NULL;
	m_lastInteractSceneMoved = NULL;
	m_lastSceneMovedLeft = false;
	m_bAllowFade = false;
	m_iconItem = nullptr;
	m_iconIsFoil = false;

	m_bContainerMenuVisible = false;
	m_bSplitscreenGamertagVisible = false;
	m_iconType = e_ICON_TYPE_IGGY;

	m_labelDescription.init(L"");
}

wstring UIComponent_TutorialPopup::getMoviePath()
{
	switch( m_parentLayer->getViewport() )
	{
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
	case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
	case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
		return L"TutorialPopupSplit";
		break;
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
	default:
		return L"TutorialPopup";
		break;
	}
}

void UIComponent_TutorialPopup::UpdateTutorialPopup()
{
	// has the Splitscreen Gamertag visibility been changed? Re-Adjust Layout to prevent overlaps!
	if(m_bSplitscreenGamertagVisible != (bool)(app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_DisplaySplitscreenGamertags) != 0))
	{
		m_bSplitscreenGamertagVisible = (bool)(app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_DisplaySplitscreenGamertags) != 0);
		handleReload();
	}
}

void UIComponent_TutorialPopup::handleReload()
{
	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_boolean;
	value[0].boolval = (bool)((app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_DisplaySplitscreenGamertags)!=0) && !m_bContainerMenuVisible);	// 4J - TomK - Offset for splitscreen gamertag?
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcAdjustLayout, 1 , value );

	setupIconHolder(m_iconType);
}

void UIComponent_TutorialPopup::SetTutorialDescription(TutorialPopupInfo *info)
{
	m_interactScene = info->interactScene;

	wstring parsed = _SetIcon(info->icon, info->iAuxVal, info->isFoil, info->desc);
	parsed = _SetImage( parsed );
	parsed = ParseDescription(m_iPad, parsed);

	if(parsed.empty())
	{
		_SetDescription( info->interactScene, L"", L"", info->allowFade, info->isReminder );
	}
	else
	{
		_SetDescription( info->interactScene, parsed, info->title, info->allowFade, info->isReminder );
	}
}

void UIComponent_TutorialPopup::RemoveInteractSceneReference(UIScene *scene)
{
	if( m_interactScene == scene )
	{
		m_interactScene = NULL;
	}
}

void UIComponent_TutorialPopup::SetVisible(bool visible)
{
	m_parentLayer->showComponent(0,eUIComponent_TutorialPopup,visible);

	if( visible && m_bAllowFade )
	{
		//Initialise a timer to fade us out again
		app.DebugPrintf("UIComponent_TutorialPopup::SetVisible: setting TUTORIAL_POPUP_FADE_TIMER_ID to %d\n",m_tutorial->GetTutorialDisplayMessageTime());
		addTimer(TUTORIAL_POPUP_FADE_TIMER_ID,m_tutorial->GetTutorialDisplayMessageTime());
	}
}

bool UIComponent_TutorialPopup::IsVisible()
{
	return m_parentLayer->isComponentVisible(eUIComponent_TutorialPopup);
}

void UIComponent_TutorialPopup::handleTimerComplete(int id)
{
	switch(id)
	{
	case TUTORIAL_POPUP_FADE_TIMER_ID:
		SetVisible(false);
		killTimer(id);
		app.DebugPrintf("handleTimerComplete: setting TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID\n");
		addTimer(TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID,TUTORIAL_POPUP_MOVE_SCENE_TIME);
		break;
	case TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID:
		UpdateInteractScenePosition(IsVisible());
		killTimer(id);
		break;
	}
}

void UIComponent_TutorialPopup::_SetDescription(UIScene *interactScene, const wstring &desc, const wstring &title, bool allowFade, bool isReminder)
{	
	m_interactScene = interactScene;
	app.DebugPrintf("Setting m_interactScene to %08x\n", m_interactScene);
	if( interactScene != m_lastInteractSceneMoved ) m_lastInteractSceneMoved = NULL;
	if(desc.empty())
	{
		SetVisible( false );
		app.DebugPrintf("_SetDescription1: setting TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID\n");
		addTimer(TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID,TUTORIAL_POPUP_MOVE_SCENE_TIME);
		killTimer(TUTORIAL_POPUP_FADE_TIMER_ID);
	}
	else
	{
		SetVisible( true );
		app.DebugPrintf("_SetDescription2: setting TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID\n");
		addTimer(TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID,TUTORIAL_POPUP_MOVE_SCENE_TIME);

		if( allowFade )
		{
			//Initialise a timer to fade us out again
			app.DebugPrintf("_SetDescription: setting TUTORIAL_POPUP_FADE_TIMER_ID\n");
			addTimer(TUTORIAL_POPUP_FADE_TIMER_ID,m_tutorial->GetTutorialDisplayMessageTime());
		}
		else
		{
			app.DebugPrintf("_SetDescription: killing TUTORIAL_POPUP_FADE_TIMER_ID\n");
			killTimer(TUTORIAL_POPUP_FADE_TIMER_ID);
		}
		m_bAllowFade = allowFade;

		if(isReminder)
		{
			wstring text(app.GetString( IDS_TUTORIAL_REMINDER ));
			text.append( desc );
			stripWhitespaceForHtml( text );
			// set the text colour
			wchar_t formatting[40];
			// 4J Stu - Don't set HTML font size, that's set at design time in flash
			//swprintf(formatting, 40, L"<font color=\"#%08x\" size=\"%d\">",app.GetHTMLColour(eHTMLColor_White),m_textFontSize);
			swprintf(formatting, 40, L"<font color=\"#%08x\">",app.GetHTMLColour(eHTMLColor_White));
			text = formatting + text;

			m_labelDescription.setLabel( text, true );
		}
		else
		{
			wstring text(desc);
			stripWhitespaceForHtml( text );
			// set the text colour
			wchar_t formatting[40];
			// 4J Stu - Don't set HTML font size, that's set at design time in flash
			//swprintf(formatting, 40, L"<font color=\"#%08x\" size=\"%d\">",app.GetHTMLColour(eHTMLColor_White),m_textFontSize);
			swprintf(formatting, 40, L"<font color=\"#%08x\">",app.GetHTMLColour(eHTMLColor_White));
			text = formatting + text;

			m_labelDescription.setLabel( text, true );

		}

		m_labelTitle.setLabel( title, true );
		m_labelTitle.setVisible(!title.empty());


		// read host setting if gamertag is visible or not and pass on to Adjust Layout function (so we can offset it to stay clear of the gamertag)
		m_bSplitscreenGamertagVisible = (bool)(app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_DisplaySplitscreenGamertags)!=0);
		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = (m_bSplitscreenGamertagVisible && !m_bContainerMenuVisible);	// 4J - TomK - Offset for splitscreen gamertag?
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcAdjustLayout, 1 , value );
	}
}

wstring UIComponent_TutorialPopup::_SetIcon(int icon, int iAuxVal, bool isFoil, LPCWSTR desc)
{
	wstring temp(desc);

	bool isFixedIcon = false;
	
	m_iconIsFoil = isFoil;
	if( icon != TUTORIAL_NO_ICON )
	{
		m_iconIsFoil = false;
		m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(icon,1,iAuxVal));
	}
	else
	{
		m_iconItem = nullptr;
		wstring openTag(L"{*ICON*}");
		wstring closeTag(L"{*/ICON*}");
		int iconTagStartPos = (int)temp.find(openTag);
		int iconStartPos = iconTagStartPos + (int)openTag.length();
		if( iconTagStartPos > 0 && iconStartPos < (int)temp.length() )
		{
			int iconEndPos = (int)temp.find( closeTag, iconStartPos );

			if(iconEndPos > iconStartPos && iconEndPos < (int)temp.length() )
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
				m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(iconId,1,iAuxVal));

				temp.replace(iconTagStartPos, iconEndPos - iconTagStartPos + closeTag.length(), L"");
			}
		}
	
		// remove any icon text
		else if(temp.find(L"{*CraftingTableIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Tile::workBench_Id,1,0));
		}
		else if(temp.find(L"{*SticksIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Item::stick_Id,1,0));
		}
		else if(temp.find(L"{*PlanksIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Tile::wood_Id,1,0));
		}
		else if(temp.find(L"{*WoodenShovelIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Item::shovel_wood_Id,1,0));
		}
		else if(temp.find(L"{*WoodenHatchetIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Item::hatchet_wood_Id,1,0));
		}
		else if(temp.find(L"{*WoodenPickaxeIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Item::pickAxe_wood_Id,1,0));
		}
		else if(temp.find(L"{*FurnaceIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Tile::furnace_Id,1,0));
		}
		else if(temp.find(L"{*WoodenDoorIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Item::door_wood,1,0));
		}
		else if(temp.find(L"{*TorchIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Tile::torch_Id,1,0));
		}
		else if(temp.find(L"{*BoatIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Item::boat_Id,1,0));
		}
		else if(temp.find(L"{*FishingRodIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Item::fishingRod_Id,1,0));
		}
		else if(temp.find(L"{*FishIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Item::fish_raw_Id,1,0));
		}
		else if(temp.find(L"{*MinecartIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Item::minecart_Id,1,0));
		}
		else if(temp.find(L"{*RailIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Tile::rail_Id,1,0));
		}
		else if(temp.find(L"{*PoweredRailIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Tile::goldenRail_Id,1,0));
		}
		else if(temp.find(L"{*StructuresIcon*}")!=wstring::npos)
		{
			isFixedIcon = true;
			setupIconHolder(e_ICON_TYPE_STRUCTURES);
		}
		else if(temp.find(L"{*ToolsIcon*}")!=wstring::npos)
		{
			isFixedIcon = true;
			setupIconHolder(e_ICON_TYPE_TOOLS);
		}
		else if(temp.find(L"{*StoneIcon*}")!=wstring::npos)
		{
			m_iconItem = shared_ptr<ItemInstance>(new ItemInstance(Tile::stone_Id,1,0));
		}
		else
		{
			m_iconItem = nullptr;
		}
	}
	if(!isFixedIcon && m_iconItem != NULL) setupIconHolder(e_ICON_TYPE_IGGY);
	m_controlIconHolder.setVisible( isFixedIcon || m_iconItem != NULL);

	return temp;
}

wstring UIComponent_TutorialPopup::_SetImage(wstring &desc)
{
	// 4J Stu - Unused
#if 0
	BOOL imageShowAtStart = m_image.IsShown();

	wstring openTag(L"{*IMAGE*}");
	wstring closeTag(L"{*/IMAGE*}");
	int imageTagStartPos = (int)desc.find(openTag);
	int imageStartPos = imageTagStartPos + (int)openTag.length();
	if( imageTagStartPos > 0 && imageStartPos < (int)desc.length() )
	{
		int imageEndPos = (int)desc.find( closeTag, imageStartPos );

		if(imageEndPos > imageStartPos && imageEndPos < (int)desc.length() )
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
#endif
	return desc;
}


wstring UIComponent_TutorialPopup::ParseDescription(int iPad, wstring &text)
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

	bool exitScreenshot = false;
	size_t pos = text.find(L"{*EXIT_PICTURE*}");
	if(pos != wstring::npos) exitScreenshot = true;
	text = replaceAll(text, L"{*EXIT_PICTURE*}", L"");
	m_controlExitScreenshot.setVisible(exitScreenshot);
		/*
#define MINECRAFT_ACTION_RENDER_DEBUG		ACTION_INGAME_13
#define MINECRAFT_ACTION_PAUSEMENU			ACTION_INGAME_15
#define MINECRAFT_ACTION_SNEAK_TOGGLE		ACTION_INGAME_17
	*/

	return app.FormatHTMLString(iPad,text);
}

void UIComponent_TutorialPopup::UpdateInteractScenePosition(bool visible)
{
	if( m_interactScene == NULL ) return;

	// 4J-PB - check this players screen section to see if we should allow the animation
	bool bAllowAnim=false;
	bool isCraftingScene = (m_interactScene->getSceneType() == eUIScene_Crafting2x2Menu) || (m_interactScene->getSceneType() == eUIScene_Crafting3x3Menu);
	bool isCreativeScene = (m_interactScene->getSceneType() == eUIScene_CreativeMenu);
	bool isTradingScene = (m_interactScene->getSceneType() == eUIScene_TradingMenu);
	switch(Minecraft::GetInstance()->localplayers[m_iPad]->m_iScreenSection)
	{
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
		bAllowAnim=true;
		break;
	default:
		// anim allowed for everything except the crafting 2x2 and 3x3, and the creative menu
		if(!isCraftingScene && !isCreativeScene && !isTradingScene)
		{
			bAllowAnim=true;
		}
		break;
	}

	if(bAllowAnim)
	{
		bool movingLeft = visible;

		if( (m_lastInteractSceneMoved != m_interactScene && movingLeft) || ( m_lastInteractSceneMoved == m_interactScene && m_lastSceneMovedLeft != movingLeft ) )
		{
			if(movingLeft)
			{
				m_interactScene->slideLeft();
			}
			else
			{
				m_interactScene->slideRight();
			}

			m_lastInteractSceneMoved = m_interactScene;
			m_lastSceneMovedLeft = movingLeft;
		}
	}

}

void UIComponent_TutorialPopup::render(S32 width, S32 height, C4JRender::eViewportType viewport)
{
	if(viewport != C4JRender::VIEWPORT_TYPE_FULLSCREEN)
	{
		S32 xPos = 0;
		S32 yPos = 0;
		switch( viewport )
		{
		case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
			xPos = (S32)(ui.getScreenWidth() / 2);
			yPos = (S32)(ui.getScreenHeight() / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
			yPos = (S32)(ui.getScreenHeight() / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
		case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
			xPos = (S32)(ui.getScreenWidth() / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
			xPos = (S32)(ui.getScreenWidth() / 2);
			yPos = (S32)(ui.getScreenHeight() / 2);
			break;
		}
		//Adjust for safezone
		switch( viewport )
		{
			case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
			case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
			case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
			case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
			case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
				yPos += getSafeZoneHalfHeight();
				break;
		}
		switch( viewport )
		{
			case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
			case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
			case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
			case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
			case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
				xPos -= getSafeZoneHalfWidth();
				break;
		}
		ui.setupRenderPosition(xPos, yPos);

		IggyPlayerSetDisplaySize( getMovie(), width, height );
		IggyPlayerDraw( getMovie() );
	}
	else
	{
		UIScene::render(width, height, viewport);
	}
}

void UIComponent_TutorialPopup::customDraw(IggyCustomDrawCallbackRegion *region)
{
	if(m_iconItem != NULL) customDrawSlotControl(region,m_iPad,m_iconItem,1.0f,m_iconItem->isFoil() || m_iconIsFoil,false);
}

void UIComponent_TutorialPopup::setupIconHolder(EIcons icon)
{
	app.DebugPrintf("Setting icon holder to %d\n", icon);
	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = (F64)icon;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetupIconHolder , 1 , value );

	m_iconType = icon;
}
