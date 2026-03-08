#include "stdafx.h"
#include "UI.h"
#include "UIScene_HUD.h"
#include "UISplitScreenHelpers.h"
#include "BossMobGuiInfo.h"
#include "..\..\Minecraft.h"
#include "..\..\MultiplayerLocalPlayer.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.boss.enderdragon.h"
#include "..\..\EnderDragonRenderer.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"

UIScene_HUD::UIScene_HUD(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	m_bSplitscreen = false;

	// Setup all the Iggy references we need for this scene
	initialiseMovie();

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

		// Is boss present?
		bool noBoss = BossMobGuiInfo::name.empty() || BossMobGuiInfo::displayTicks <= 0;
		if (noBoss) 
		{
			if (m_showDragonHealth)
			{
				// No boss and health is visible
				if(m_ticksWithNoBoss <= 20)
				{
					++m_ticksWithNoBoss;
				}
				else
				{
					ShowDragonHealth(false);
				}
			}
		}
		else
		{
			BossMobGuiInfo::displayTicks--;

			m_ticksWithNoBoss = 0;			
			SetDragonHealth(BossMobGuiInfo::healthProgress);

			if (!m_showDragonHealth)
			{
				SetDragonLabel(BossMobGuiInfo::name);
				ShowDragonHealth(true);
			}
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
	m_lastActiveSlot = -1;
	m_iGuiScale = -1;
	m_bToolTipsVisible = true;
	m_lastExpProgress = 0.0f;
	m_lastExpLevel = 0;
	m_iCurrentHealth = 0;
	m_lastMaxHealth = 20;
	m_lastHealthBlink = false;
	m_lastHealthPoison = false;
	m_iCurrentFood = -1;
	m_lastFoodPoison = false;
	m_lastAir = 10;
	m_currentExtraAir = 0;
	m_lastArmour = 0;
	m_showHealth = true;
	m_showHorseHealth = true;
	m_showFood = true;
	m_showAir = false;	// get's initialised invisible anyways, by setting it to false we ensure it will remain visible when switching in and out of split screen!
	m_showArmour = true;
	m_showExpBar = true;
	m_bRegenEffectEnabled = false;
	m_iFoodSaturation = 0;
	m_lastDragonHealth = 0.0f;
	m_showDragonHealth = false;
	m_ticksWithNoBoss = 0;
	m_uiSelectedItemOpacityCountDown = 0;
	m_displayName = L"";
	m_lastShowDisplayName = true;
	m_bRidingHorse = true;
	m_horseHealth = 1;
	m_lastHealthWither = true;
	m_iCurrentHealthAbsorb = -1;
	m_horseJumpProgress = 1.0f;
	m_iHeartOffsetIndex = -1;
	m_bHealthAbsorbActive = false;
	m_iHorseMaxHealth = -1;

	m_labelDisplayName.setVisible(m_lastShowDisplayName);

	SetDragonLabel(BossMobGuiInfo::name);
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

	SetTooltipsEnabled(((ui.GetMenuDisplayed(ProfileManager.GetPrimaryPad())) || (app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_Tooltips) != 0)));
}

int UIScene_HUD::getPad()
{
	return m_iPad;
}

void UIScene_HUD::SetOpacity(float opacity)
{
	setOpacity(opacity);
}

void UIScene_HUD::SetVisible(bool visible)
{
	setVisible(visible);
}

void UIScene_HUD::SetHudSize(int scale)
{
	if(scale != m_iGuiScale)
	{
		m_iGuiScale = scale;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = scale;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcLoadHud , 1 , value );
	}
}

void UIScene_HUD::SetExpBarProgress(float progress, int xpNeededForNextLevel)
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

void UIScene_HUD::SetHealth(int iHealth, int iLastHealth, bool bBlink, bool bPoison, bool bWither)
{
	int maxHealth = max(iHealth, iLastHealth);
	if(maxHealth != m_lastMaxHealth || bBlink != m_lastHealthBlink || bPoison != m_lastHealthPoison || bWither != m_lastHealthWither)
	{
		m_lastMaxHealth = maxHealth;
		m_lastHealthBlink = bBlink;
		m_lastHealthPoison = bPoison;
		m_lastHealthWither = bWither;

		IggyDataValue result;
		IggyDataValue value[4];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = maxHealth;
		value[1].type = IGGY_DATATYPE_boolean;
		value[1].boolval = bBlink;
		value[2].type = IGGY_DATATYPE_boolean;
		value[2].boolval = bPoison;
		value[3].type = IGGY_DATATYPE_boolean;
		value[3].boolval = bWither;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetHealth , 4 , value );
	}
}

void UIScene_HUD::SetFood(int iFood, int iLastFood, bool bPoison)
{
	// Ignore iLastFood as food doesn't flash
	int maxFood = iFood; //, iLastFood);
	if(maxFood != m_iCurrentFood || bPoison != m_lastFoodPoison)
	{
		m_iCurrentFood = maxFood;
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

void UIScene_HUD::SetAir(int iAir, int extra)
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

void UIScene_HUD::ShowHorseHealth(bool show)
{
	if(show != m_showHorseHealth)
	{
		app.DebugPrintf("ShowHorseHealth to %s\n", show?"TRUE":"FALSE");
		m_showHorseHealth = show;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = show;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcShowHorseHealth , 1 , value );
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
	if(bEnabled != m_bRegenEffectEnabled)
	{
		app.DebugPrintf("SetRegenerationEffect to %s\n", bEnabled?"TRUE":"FALSE");
		m_bRegenEffectEnabled = bEnabled;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = bEnabled;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetRegenerationEffect , 1 , value );
	}
}

void UIScene_HUD::SetFoodSaturationLevel(int iSaturation)
{
	if(iSaturation != m_iFoodSaturation)
	{
		app.DebugPrintf("Set saturation to %d\n", iSaturation);
		m_iFoodSaturation = iSaturation;

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


void UIScene_HUD::SetRidingHorse(bool ridingHorse, bool bIsJumpable, int maxHorseHealth)
{
	if(m_bRidingHorse != ridingHorse || maxHorseHealth != m_iHorseMaxHealth)
	{
		app.DebugPrintf("SetRidingHorse to %s\n", ridingHorse?"TRUE":"FALSE");
		m_bRidingHorse = ridingHorse;
		m_bIsJumpable = bIsJumpable;
		m_iHorseMaxHealth = maxHorseHealth;

		IggyDataValue result;
		IggyDataValue value[3];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = ridingHorse;
		value[1].type = IGGY_DATATYPE_boolean;
		value[1].boolval = bIsJumpable;
		value[2].type = IGGY_DATATYPE_number;
		value[2].number = maxHorseHealth;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetRidingHorse , 3 , value );
	}
}

void UIScene_HUD::SetHorseHealth(int health, bool blink /*= false*/)
{
	if(m_bRidingHorse && m_horseHealth != health)
	{
		app.DebugPrintf("SetHorseHealth to %d\n", health);
		m_horseHealth = health;

		IggyDataValue result;
		IggyDataValue value[2];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = health;
		value[1].type = IGGY_DATATYPE_boolean;
		value[1].boolval = blink;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetHorseHealth , 2 , value );
	}
}

void UIScene_HUD::SetHorseJumpBarProgress(float progress)
{
	if(m_bRidingHorse && m_horseJumpProgress != progress)
	{
		app.DebugPrintf("SetHorseJumpBarProgress to %f\n", progress);
		m_horseJumpProgress = progress;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = progress;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetHorseJumpBarProgress , 1 , value );
	}
}

void UIScene_HUD::SetHealthAbsorb(int healthAbsorb)
{
	if(m_iCurrentHealthAbsorb != healthAbsorb)
	{
		app.DebugPrintf("SetHealthAbsorb to %d\n", healthAbsorb);
		m_iCurrentHealthAbsorb = healthAbsorb;

		IggyDataValue result;
		IggyDataValue value[2];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = healthAbsorb > 0;
		value[1].type = IGGY_DATATYPE_number;
		value[1].number = healthAbsorb;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetHealthAbsorb , 2 , value );
	}
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

		bool needsYTile = false;
		switch( viewport )
		{
		case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
		case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
			tileHeight = (S32)(ui.getScreenHeight());
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
		case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
			tileWidth = (S32)(ui.getScreenWidth());
			needsYTile = true;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
			needsYTile = true;
			break;
		}

		F32 scale;
		ComputeTileScale(tileWidth, tileHeight, m_movieWidth, m_movieHeight, needsYTile, scale, tileYStart);

		IggyPlayerSetDisplaySize( getMovie(), (S32)(m_movieWidth * scale), (S32)(m_movieHeight * scale) );

		repositionHud(tileWidth, tileHeight, scale);

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
#if 0 // def _WINDOWS64 // Use Iggy chat until Gui::render has visual parity
				// Chat drawn by Gui::render with color codes. Hides Iggy chat to avoid double chats.
				m_controlLabelBackground[i].setOpacity(0);
				m_labelChatText[i].setOpacity(0);
				m_labelChatText[i].setLabel(L"");
#else
				m_controlLabelBackground[i].setOpacity(opacity);
				m_labelChatText[i].setOpacity(opacity);
				m_labelChatText[i].setLabel( pGui->getMessagesCount(m_iPad) ? pGui->getMessage(m_iPad,i) : L"" );
#endif
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

void UIScene_HUD::repositionHud(S32 tileWidth, S32 tileHeight, F32 scale)
{
	if(!m_bSplitscreen) return;

	// Pass the visible tile area in SWF coordinates so ActionScript
	// positions elements (crosshair, hotbar, etc.) centered in the
	// actually visible region, not the raw viewport.
	S32 visibleW = (S32)(tileWidth / scale);
	S32 visibleH = (S32)(tileHeight / scale);

	app.DebugPrintf(app.USER_SR, "Reposition HUD: tile %dx%d, scale %.3f, visible SWF %dx%d\n", tileWidth, tileHeight, scale, visibleW, visibleH );

	IggyDataValue result;
	IggyDataValue value[2];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = visibleW;
	value[1].type = IGGY_DATATYPE_number;
	value[1].number = visibleH;
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

		updateFrameTick();
	}
}