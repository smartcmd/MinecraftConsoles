#include "stdafx.h"
#include "UI.h"
#include "UIScene_HUD.h"
#include "..\..\Minecraft.h"
#include "..\..\MultiplayerLocalPlayer.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.boss.enderdragon.h"
#include "..\..\EnderDragonRenderer.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.effect.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"

UIScene_HUD::UIScene_HUD(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	m_bSplitscreen = false;

	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_lastActiveSlot = 0;
	m_lastScale = 1;
	m_bToolTipsVisible = true;
	m_lastExpProgress = 0.0f;
	m_lastExpLevel = 0;
	m_lastMaxHealth = 20;
	m_lastHealthBlink = false;
	m_lastHealthPoison = false;
	m_lastMaxFood = 20;
	m_lastFoodPoison = false;
	m_lastAir = 10;
	m_lastArmour = 0;
	m_showHealth = true;
	m_showFood = true;
	m_showAir = true;
	m_showArmour = true;
	m_showExpBar = true;
	m_lastRegenEffect = false;
	m_lastSaturation = 0;
	m_lastDragonHealth = 0.0f;
	m_showDragonHealth = false;
	m_ticksWithNoBoss = 0;
	m_uiSelectedItemOpacityCountDown = 0;
	m_displayName = L"";
	m_lastShowDisplayName = true;

	SetDragonLabel( app.GetString( IDS_BOSS_ENDERDRAGON_HEALTH ) );
	SetSelectedLabel(L"");

	for(unsigned int i = 0; i < CHAT_LINES_COUNT; ++i)
	{
		m_labelChatText[i].init(L"");
	}
	m_labelJukebox.init(L"");

	addTimer(0, 100);
}

wstring UIScene_HUD::getMoviePath()
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
		m_bSplitscreen = true;
		return L"HUDSplit";
		break;
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
	default:
		m_bSplitscreen = false;
		return L"HUD";
		break;
	}
}

void UIScene_HUD::updateSafeZone()
{
	// Distance from edge
	F64 safeTop = 0.0;
	F64 safeBottom = 0.0;
	F64 safeLeft = 0.0;
	F64 safeRight = 0.0;

	switch( m_parentLayer->getViewport() )
	{
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
		safeTop = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();
		safeRight = getSafeZoneHalfWidth();
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
		safeBottom = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();
		safeRight = getSafeZoneHalfWidth();
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
		safeLeft = getSafeZoneHalfWidth();
		safeTop = getSafeZoneHalfHeight();
		safeBottom = getSafeZoneHalfHeight();
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
		safeRight = getSafeZoneHalfWidth();
		safeTop = getSafeZoneHalfHeight();
		safeBottom = getSafeZoneHalfHeight();
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
		safeTop = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
		safeTop = getSafeZoneHalfHeight();
		safeRight = getSafeZoneHalfWidth();
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
		safeBottom = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
		safeBottom = getSafeZoneHalfHeight();
		safeRight = getSafeZoneHalfWidth();
		break;
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
	default:
		safeTop = getSafeZoneHalfHeight();
		safeBottom = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();
		safeRight = getSafeZoneHalfWidth();
		break;
	}
	setSafeZone(safeTop, safeBottom, safeLeft, safeRight);
}

void UIScene_HUD::tick()
{
	UIScene::tick();
	if(getMovie() && app.GetGameStarted())
	{
		Minecraft *pMinecraft = Minecraft::GetInstance();
		if(pMinecraft->localplayers[m_iPad] == NULL || pMinecraft->localgameModes[m_iPad] == NULL)
		{
			return;
		}

		if(pMinecraft->localplayers[m_iPad]->dimension == 1)
		{
			if (EnderDragonRenderer::bossInstance == NULL)
			{
				if(m_ticksWithNoBoss<=20)
				{
					++m_ticksWithNoBoss;
				}
				if( m_ticksWithNoBoss > 20 )
				{
					ShowDragonHealth(false);
				}
			}
			else
			{
				shared_ptr<EnderDragon> boss = EnderDragonRenderer::bossInstance;
				// 4J Stu - Don't clear this here as it's wiped for other players
				//EnderDragonRenderer::bossInstance = nullptr;
				m_ticksWithNoBoss = 0;

				ShowDragonHealth(true);
				SetDragonHealth( (float)boss->getSynchedHealth()/boss->getMaxHealth());
			}
		}
		else
		{
			ShowDragonHealth(false);
		}
	}
}

void UIScene_HUD::customDraw(IggyCustomDrawCallbackRegion *region)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->localplayers[m_iPad] == NULL || pMinecraft->localgameModes[m_iPad] == NULL) return;

	int slot = -1;
	swscanf((wchar_t*)region->name,L"slot_%d",&slot);
	if (slot == -1)
	{
		app.DebugPrintf("This is not the control we are looking for\n");
	}
	else
	{
		Slot *invSlot = pMinecraft->localplayers[m_iPad]->inventoryMenu->getSlot(InventoryMenu::USE_ROW_SLOT_START + slot);
		shared_ptr<ItemInstance> item = invSlot->getItem();
		if(item != NULL)
		{
			unsigned char ucAlpha=app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_InterfaceOpacity);
			float fVal;

			if(ucAlpha<80)
			{
				// check if we have the timer running for the opacity
				unsigned int uiOpacityTimer=app.GetOpacityTimer(m_iPad);
				if(uiOpacityTimer!=0)
				{
					if(uiOpacityTimer<10)
					{
						float fStep=(80.0f-(float)ucAlpha)/10.0f;
						fVal=0.01f*(80.0f-((10.0f-(float)uiOpacityTimer)*fStep));
					}
					else
					{
						fVal=0.01f*80.0f;
					}
				}
				else
				{
					fVal=0.01f*(float)ucAlpha;
				}
			}
			else
			{
				fVal=0.01f*(float)ucAlpha;
			}
			customDrawSlotControl(region,m_iPad,item,fVal,item->isFoil(),true);
		}
	}
}

void UIScene_HUD::handleReload()
{
	m_lastActiveSlot = 0;
	m_lastScale = 1;
	m_bToolTipsVisible = true;
	m_lastExpProgress = 0.0f;
	m_lastExpLevel = 0;
	m_lastMaxHealth = 20;
	m_lastHealthBlink = false;
	m_lastHealthPoison = false;
	m_lastMaxFood = 20;
	m_lastFoodPoison = false;
	m_lastAir = 10;
	m_lastArmour = 0;
	m_showHealth = true;
	m_showFood = true;
	m_showAir = true;
	m_showArmour = true;
	m_showExpBar = true;
	m_lastRegenEffect = false;
	m_lastSaturation = 0;
	m_lastDragonHealth = 0.0f;
	m_showDragonHealth = false;
	m_ticksWithNoBoss = 0;
	m_uiSelectedItemOpacityCountDown = 0;
	m_displayName = L"";

	m_labelDisplayName.setVisible(m_lastShowDisplayName);

	SetDragonLabel( app.GetString( IDS_BOSS_ENDERDRAGON_HEALTH ) );
	SetSelectedLabel(L"");

	for(unsigned int i = 0; i < CHAT_LINES_COUNT; ++i)
	{
		m_labelChatText[i].init(L"");
	}
	m_labelJukebox.init(L"");

	int iGuiScale;	
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->localplayers[m_iPad] == NULL || pMinecraft->localplayers[m_iPad]->m_iScreenSection == C4JRender::VIEWPORT_TYPE_FULLSCREEN)
	{
		iGuiScale=app.GetGameSettings(m_iPad,eGameSetting_UISize);
	}
	else
	{
		iGuiScale=app.GetGameSettings(m_iPad,eGameSetting_UISizeSplitscreen);
	}
	SetHudSize(iGuiScale);

	SetDisplayName(ProfileManager.GetDisplayName(m_iPad));

	repositionHud();

	SetTooltipsEnabled(((ui.GetMenuDisplayed(ProfileManager.GetPrimaryPad())) || (app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_Tooltips) != 0)));
}

void UIScene_HUD::SetHudSize(int scale)
{
	if(scale != m_lastScale)
	{
		m_lastScale = scale;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = scale;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcLoadHud , 1 , value );
	}
}

void UIScene_HUD::SetExpBarProgress(float progress)
{
	if(progress != m_lastExpProgress)
	{
		m_lastExpProgress = progress;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = progress;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetExpBarProgress , 1 , value );
	}
}

void UIScene_HUD::SetExpLevel(int level)
{
	if(level != m_lastExpLevel)
	{
		m_lastExpLevel = level;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = level;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetPlayerLevel , 1 , value );
	}
}

void UIScene_HUD::SetActiveSlot(int slot)
{
	if(slot != m_lastActiveSlot)
	{
		m_lastActiveSlot = slot;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = slot;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetActiveSlot , 1 , value );
	}
}

void UIScene_HUD::SetHealth(int iHealth, int iLastHealth, bool bBlink, bool bPoison)
{
	int maxHealth = max(iHealth, iLastHealth);
	if(maxHealth != m_lastMaxHealth || bBlink != m_lastHealthBlink || bPoison != m_lastHealthPoison)
	{
		m_lastMaxHealth = maxHealth;
		m_lastHealthBlink = bBlink;
		m_lastHealthPoison = bPoison;

		IggyDataValue result;
		IggyDataValue value[3];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = maxHealth;
		value[1].type = IGGY_DATATYPE_boolean;
		value[1].boolval = bBlink;
		value[2].type = IGGY_DATATYPE_boolean;
		value[2].boolval = bPoison;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetHealth , 3 , value );
	}
}

void UIScene_HUD::SetFood(int iFood, int iLastFood, bool bPoison)
{
	// Ignore iLastFood as food doesn't flash
	int maxFood = iFood; //, iLastFood);
	if(maxFood != m_lastMaxFood || bPoison != m_lastFoodPoison)
	{
		m_lastMaxFood = maxFood;
		m_lastFoodPoison = bPoison;

		IggyDataValue result;
		IggyDataValue value[2];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = maxFood;
		value[1].type = IGGY_DATATYPE_boolean;
		value[1].boolval = bPoison;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetFood , 2 , value );
	}
}

void UIScene_HUD::SetAir(int iAir)
{
	if(iAir != m_lastAir)
	{
		app.DebugPrintf("SetAir to %d\n", iAir);
		m_lastAir = iAir;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = iAir;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetAir , 1 , value );
	}
}

void UIScene_HUD::SetArmour(int iArmour)
{
	if(iArmour != m_lastArmour)
	{
		app.DebugPrintf("SetArmour to %d\n", iArmour);
		m_lastArmour = iArmour;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = iArmour;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetArmour , 1 , value );
	}
}

void UIScene_HUD::ShowHealth(bool show)
{
	if(show != m_showHealth)
	{
		app.DebugPrintf("ShowHealth to %s\n", show?"TRUE":"FALSE");
		m_showHealth = show;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = show;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcShowHealth , 1 , value );
	}
}

void UIScene_HUD::ShowFood(bool show)
{
	if(show != m_showFood)
	{
		app.DebugPrintf("ShowFood to %s\n", show?"TRUE":"FALSE");
		m_showFood = show;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = show;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcShowFood , 1 , value );
	}
}

void UIScene_HUD::ShowAir(bool show)
{
	if(show != m_showAir)
	{
		app.DebugPrintf("ShowAir to %s\n", show?"TRUE":"FALSE");
		m_showAir = show;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = show;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcShowAir , 1 , value );
	}
}

void UIScene_HUD::ShowArmour(bool show)
{
	if(show != m_showArmour)
	{
		app.DebugPrintf("ShowArmour to %s\n", show?"TRUE":"FALSE");
		m_showArmour = show;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = show;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcShowArmour , 1 , value );
	}
}

void UIScene_HUD::ShowExpBar(bool show)
{
	if(show != m_showExpBar)
	{
		app.DebugPrintf("ShowExpBar to %s\n", show?"TRUE":"FALSE");
		m_showExpBar = show;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = show;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcShowExpbar , 1 , value );
	}
}

void UIScene_HUD::SetRegenerationEffect(bool bEnabled)
{
	if(bEnabled != m_lastRegenEffect)
	{
		app.DebugPrintf("SetRegenerationEffect to %s\n", bEnabled?"TRUE":"FALSE");
		m_lastRegenEffect = bEnabled;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = bEnabled;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetRegenerationEffect , 1 , value );
	}
}

void UIScene_HUD::SetFoodSaturationLevel(int iSaturation)
{
	if(iSaturation != m_lastSaturation)
	{
		app.DebugPrintf("Set saturation to %d\n", iSaturation);
		m_lastSaturation = iSaturation;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = iSaturation;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetFoodSaturationLevel , 1 , value );
	}
}

void UIScene_HUD::SetDragonHealth(float health)
{
	if(health != m_lastDragonHealth)
	{
		app.DebugPrintf("Set dragon health to %f\n", health);
		m_lastDragonHealth = health;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = health;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetDragonHealth , 1 , value );
	}
}

void UIScene_HUD::SetDragonLabel(const wstring &label)
{
	IggyDataValue result;
	IggyDataValue value[1];
	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].type = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetDragonLabel , 1 , value );
}

void UIScene_HUD::ShowDragonHealth(bool show)
{
	if(show != m_showDragonHealth)
	{
		app.DebugPrintf("ShowDragonHealth to %s\n", show?"TRUE":"FALSE");
		m_showDragonHealth = show;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = show;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcShowDragonHealth , 1 , value );
	}
}

void UIScene_HUD::SetSelectedLabel(const wstring &label)
{
	// 4J Stu - Timing here is kept the same as on Xbox360, even though we do it differently now and do the fade out in Flash rather than directly setting opacity
	if(!label.empty()) m_uiSelectedItemOpacityCountDown = SharedConstants::TICKS_PER_SECOND * 3;

	IggyDataValue result;
	IggyDataValue value[1];
	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].type = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetSelectedLabel , 1 , value );
}

void UIScene_HUD::HideSelectedLabel()
{
	IggyDataValue result;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcHideSelectedLabel , 0 , NULL );
}

void UIScene_HUD::render(S32 width, S32 height, C4JRender::eViewportType viewport)
{
	if(m_bSplitscreen)
	{
		S32 xPos = 0;
		S32 yPos = 0;
		switch( viewport )
		{
		case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
			yPos = (S32)(ui.getScreenHeight() / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
			xPos = (S32)(ui.getScreenWidth() / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
			xPos = (S32)(ui.getScreenWidth() / 2);
			yPos = (S32)(ui.getScreenHeight() / 2);
			break;
		}
		ui.setupRenderPosition(xPos, yPos);

		S32 tileXStart = 0;
		S32 tileYStart = 0;
		S32 tileWidth = width;
		S32 tileHeight = height;

		switch( viewport )
		{
		case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
		case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
			tileHeight = (S32)(ui.getScreenHeight());
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
			tileWidth = (S32)(ui.getScreenWidth());
			tileYStart = (S32)(m_movieHeight / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
			tileWidth = (S32)(ui.getScreenWidth());
			tileYStart = (S32)(m_movieHeight / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
			tileYStart = (S32)(m_movieHeight / 2);
			break;
		}

		IggyPlayerSetDisplaySize( getMovie(), m_movieWidth, m_movieHeight );
		
		m_renderWidth = tileWidth;
		m_renderHeight = tileHeight;

		IggyPlayerDrawTilesStart ( getMovie() );
		IggyPlayerDrawTile ( getMovie() ,
			tileXStart ,
			tileYStart ,
			tileXStart + tileWidth ,
			tileYStart + tileHeight ,
			0 ); 
		IggyPlayerDrawTilesEnd ( getMovie() );
	}
	else
	{
		UIScene::render(width, height, viewport);
	}
}

void UIScene_HUD::handleTimerComplete(int id)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	
	bool anyVisible = false;
	if(pMinecraft->localplayers[m_iPad]!= NULL)
	{
		Gui *pGui = pMinecraft->gui;
		//DWORD messagesToDisplay = min( CHAT_LINES_COUNT, pGui->getMessagesCount(m_iPad) );
		for( unsigned int i = 0; i < CHAT_LINES_COUNT; ++i )
		{
			float opacity = pGui->getOpacity(m_iPad, i);
			if( opacity > 0 )
			{
				m_controlLabelBackground[i].setOpacity(opacity);
				m_labelChatText[i].setOpacity(opacity);
				m_labelChatText[i].setLabel( pGui->getMessagesCount(m_iPad) ? pGui->getMessage(m_iPad,i) : L"" );

				anyVisible = true;
			}
			else
			{
				m_controlLabelBackground[i].setOpacity(0);
				m_labelChatText[i].setOpacity(0);
				m_labelChatText[i].setLabel(L"");
			}
		}
		if(pGui->getJukeboxOpacity(m_iPad) > 0) anyVisible = true;
		m_labelJukebox.setOpacity( pGui->getJukeboxOpacity(m_iPad) );
		m_labelJukebox.setLabel( pGui->getJukeboxMessage(m_iPad) );
	}
	else
	{
		for( unsigned int i = 0; i < CHAT_LINES_COUNT; ++i )
		{
			m_controlLabelBackground[i].setOpacity(0);
			m_labelChatText[i].setOpacity(0);
			m_labelChatText[i].setLabel(L"");
		}
		m_labelJukebox.setOpacity( 0 );
	}

	//setVisible(anyVisible);
}

void UIScene_HUD::repositionHud()
{
	if(!m_bSplitscreen) return;

	S32 width = 0;
	S32 height = 0;
	m_parentLayer->getRenderDimensions( width, height );

	switch( m_parentLayer->getViewport() )
	{
	case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
	case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
		height = (S32)(ui.getScreenHeight());
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
		width = (S32)(ui.getScreenWidth());
		break;
	}

	app.DebugPrintf(app.USER_SR, "Reposition HUD with dims %d, %d\n", width, height );

	IggyDataValue result;
	IggyDataValue value[2];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = width;
	value[1].type = IGGY_DATATYPE_number;
	value[1].number = height;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcRepositionHud , 2 , value );
}

void UIScene_HUD::ShowDisplayName(bool show)
{
	m_lastShowDisplayName = show;
	m_labelDisplayName.setVisible(show);
}

void UIScene_HUD::SetDisplayName(const wstring &displayName)
{
	if(displayName.compare(m_displayName) != 0)
	{
		m_displayName = displayName;
		
		IggyDataValue result;
		IggyDataValue value[1];
		IggyStringUTF16 stringVal;
		stringVal.string = (IggyUTF16*)displayName.c_str();
		stringVal.length = displayName.length();
		value[0].type = IGGY_DATATYPE_string_UTF16;
		value[0].string16 = stringVal;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetDisplayName , 1 , value );

		m_labelDisplayName.setVisible(m_lastShowDisplayName);
	}
}

void UIScene_HUD::SetTooltipsEnabled(bool bEnabled)
{
	if(m_bToolTipsVisible != bEnabled)
	{
		m_bToolTipsVisible = bEnabled;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = bEnabled;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetTooltipsEnabled , 1 , value );
	}
}

void UIScene_HUD::handleGameTick()
{
	if(getMovie() && app.GetGameStarted())
	{
		Minecraft *pMinecraft = Minecraft::GetInstance();
		if(pMinecraft->localplayers[m_iPad] == NULL || pMinecraft->localgameModes[m_iPad] == NULL)
		{
			m_parentLayer->showComponent(m_iPad, eUIScene_HUD,false);
			return;
		}
		m_parentLayer->showComponent(m_iPad, eUIScene_HUD,true);

		int iGuiScale;	

		if(pMinecraft->localplayers[m_iPad]->m_iScreenSection == C4JRender::VIEWPORT_TYPE_FULLSCREEN)
		{
			iGuiScale=app.GetGameSettings(m_iPad,eGameSetting_UISize);
		}
		else
		{
			iGuiScale=app.GetGameSettings(m_iPad,eGameSetting_UISizeSplitscreen);
		}
		SetHudSize(iGuiScale);

		SetDisplayName(ProfileManager.GetDisplayName(m_iPad));
		
		SetTooltipsEnabled(((ui.GetMenuDisplayed(ProfileManager.GetPrimaryPad())) || (app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_Tooltips) != 0)));

#if TO_BE_IMPLEMENTED
		// Move the whole hud group if we are not in fullscreen
		if(pMinecraft->localplayers[m_iPad]->m_iScreenSection != C4JRender::VIEWPORT_TYPE_FULLSCREEN)
		{
			int iTooltipsYOffset = 0;
			// if tooltips are off, set the y offset to zero
			if(app.GetGameSettings(m_iPad,eGameSetting_Tooltips)==0)	
			{	
				switch(iGuiScale)
				{
				case 0:
					iTooltipsYOffset=28;//screenHeight/10;
					break;
				case 2:
					iTooltipsYOffset=28;//screenHeight/10;
					break;
				case 1:
				default:
					iTooltipsYOffset=28;//screenHeight/10;
					break;
				}
			}

			float fHeight, fWidth;
			GetBounds(&fWidth, &fHeight);

			int iSafezoneYHalf = 0;
			switch(pMinecraft->localplayers[m_iPad]->m_iScreenSection)
			{
			case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
				break;
			case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
				iSafezoneYHalf = -fHeight/10;// 5%  (need to treat the whole screen is 2x this screen)
				break;
			case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
				iSafezoneYHalf = (fHeight/2)-(fHeight/10);// 5% (need to treat the whole screen is 2x this screen)
				break;
			case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
				iSafezoneYHalf = (fHeight/2)-(fHeight/10);// 5% (need to treat the whole screen is 2x this screen)
				break;
			case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
				break;
			case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
				break;
			case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
				iSafezoneYHalf = -fHeight/10; // 5% (the whole screen is 2x this screen)
				break;
			case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
				iSafezoneYHalf = -fHeight/10; // 5%  (the whole screen is 2x this screen)
				break;
			};

			D3DXVECTOR3 pos;
			m_hudGroup.GetPosition(&pos);
			pos.y = iTooltipsYOffset + iSafezoneYHalf;
			m_hudGroup.SetPosition(&pos);
		}
#endif
		SetActiveSlot(pMinecraft->localplayers[m_iPad]->inventory->selected);

		// Update xp progress
		if (pMinecraft->localgameModes[m_iPad]->canHurtPlayer())
		{
			ShowExpBar(true);
			int xpNeededForNextLevel = pMinecraft->localplayers[m_iPad]->getXpNeededForNextLevel();
			int progress = (int)(pMinecraft->localplayers[m_iPad]->experienceProgress  *xpNeededForNextLevel);
			SetExpBarProgress((float)progress/xpNeededForNextLevel);
		}
		else
		{
			ShowExpBar(false);
		}

		// Update xp level
		if (pMinecraft->localgameModes[m_iPad]->hasExperience() && pMinecraft->localplayers[m_iPad]->experienceLevel > 0)
		{
			SetExpLevel(pMinecraft->localplayers[m_iPad]->experienceLevel);
		}
		else
		{
			SetExpLevel(0);
		}

		if (pMinecraft->localgameModes[m_iPad]->canHurtPlayer())
		{
			ShowHealth(true);
			ShowFood(true);

			SetRegenerationEffect(pMinecraft->localplayers[m_iPad]->hasEffect(MobEffect::regeneration));

			// Update health
			bool blink = pMinecraft->localplayers[m_iPad]->invulnerableTime / 3 % 2 == 1;
			if (pMinecraft->localplayers[m_iPad]->invulnerableTime < 10) blink = false;
			int iHealth = pMinecraft->localplayers[m_iPad]->getHealth();
			int iLastHealth = pMinecraft->localplayers[m_iPad]->lastHealth;
			bool bHasPoison = pMinecraft->localplayers[m_iPad]->hasEffect(MobEffect::poison);
			SetHealth(iHealth, iLastHealth, blink, bHasPoison);

			// Update food
			//bool foodBlink = false;
			FoodData *foodData = pMinecraft->localplayers[m_iPad]->getFoodData();
			int food = foodData->getFoodLevel();
			int oldFood = foodData->getLastFoodLevel();
			bool hasHungerEffect = pMinecraft->localplayers[m_iPad]->hasEffect(MobEffect::hunger);
			int saturationLevel = pMinecraft->localplayers[m_iPad]->getFoodData()->getSaturationLevel();
			SetFood(food, oldFood, hasHungerEffect);
			SetFoodSaturationLevel(saturationLevel);

			// Update armour
			int armor = pMinecraft->localplayers[m_iPad]->getArmorValue();
			if(armor > 0)
			{
				ShowArmour(true);
				SetArmour(armor);
			}
			else
			{
				ShowArmour(false);
			}

			// Update air
			if (pMinecraft->localplayers[m_iPad]->isUnderLiquid(Material::water))
			{
				ShowAir(true);
				int count = (int) ceil((pMinecraft->localplayers[m_iPad]->getAirSupply() - 2) * 10.0f / Player::TOTAL_AIR_SUPPLY);
				SetAir(count);
			}
			else
			{
				ShowAir(false);
			}
		}
		else
		{
			ShowHealth(false);
			ShowFood(false);
			ShowAir(false);
			ShowArmour(false);
		}

		if(m_uiSelectedItemOpacityCountDown>0)
		{
			--m_uiSelectedItemOpacityCountDown;

			// 4J Stu - Timing here is kept the same as on Xbox360, even though we do it differently now and do the fade out in Flash rather than directly setting opacity
			if(m_uiSelectedItemOpacityCountDown < (SharedConstants::TICKS_PER_SECOND * 1) )
			{
				HideSelectedLabel();
				m_uiSelectedItemOpacityCountDown = 0;
			}
		}

		unsigned char ucAlpha=app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_InterfaceOpacity);
		float fVal;

		if(ucAlpha<80)
		{
			// if we are in a menu, set the minimum opacity for tooltips to 15%
			if(ui.GetMenuDisplayed(m_iPad) && (ucAlpha<15))
			{
				ucAlpha=15;
			}

			// check if we have the timer running for the opacity
			unsigned int uiOpacityTimer=app.GetOpacityTimer(m_iPad);
			if(uiOpacityTimer!=0)
			{
				if(uiOpacityTimer<10)
				{
					float fStep=(80.0f-(float)ucAlpha)/10.0f;
					fVal=0.01f*(80.0f-((10.0f-(float)uiOpacityTimer)*fStep));
				}
				else
				{
					fVal=0.01f*80.0f;
				}
			}
			else
			{
				fVal=0.01f*(float)ucAlpha;
			}
		}
		else
		{
			// if we are in a menu, set the minimum opacity for tooltips to 15%
			if(ui.GetMenuDisplayed(m_iPad) && (ucAlpha<15))
			{
				ucAlpha=15;
			}
			fVal=0.01f*(float)ucAlpha;
		}
		setOpacity(fVal);

		bool bDisplayGui=app.GetGameStarted() && !ui.GetMenuDisplayed(m_iPad) && !(app.GetXuiAction(m_iPad)==eAppAction_AutosaveSaveGameCapturedThumbnail) && app.GetGameSettings(m_iPad,eGameSetting_DisplayHUD)!=0;
		if(bDisplayGui && pMinecraft->localplayers[m_iPad] != NULL)
		{
			setVisible(true);			
		}
		else
		{
			setVisible(false);	
		}
	}
}
