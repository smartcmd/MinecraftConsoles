#include "stdafx.h"
#include "..\..\Minecraft.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.effect.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.ai.attributes.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.monster.h"
#include "IUIScene_HUD.h"

#include "UI.h"

IUIScene_HUD::IUIScene_HUD()
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
	m_showAir = true;
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
	m_bIsJumpable = false;
}

void IUIScene_HUD::updateFrameTick()
{
	int iPad = getPad();
	Minecraft *pMinecraft = Minecraft::GetInstance();

	int iGuiScale;	

	if(pMinecraft->localplayers[iPad]->m_iScreenSection == C4JRender::VIEWPORT_TYPE_FULLSCREEN)
	{
		iGuiScale=app.GetGameSettings(iPad,eGameSetting_UISize);
	}
	else
	{
		iGuiScale=app.GetGameSettings(iPad,eGameSetting_UISizeSplitscreen);
	}
	SetHudSize(iGuiScale);

	SetDisplayName(ProfileManager.GetDisplayName(iPad));

	SetTooltipsEnabled(((ui.GetMenuDisplayed(ProfileManager.GetPrimaryPad())) || (app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_Tooltips) != 0)));

	SetActiveSlot(pMinecraft->localplayers[iPad]->inventory->selected);

	if (pMinecraft->localgameModes[iPad]->canHurtPlayer())
	{
		renderPlayerHealth();
	}
	else
	{
		//SetRidingHorse(false, 0);
		shared_ptr<Entity> riding = pMinecraft->localplayers[iPad]->riding;
		if(riding == nullptr)
		{
			SetRidingHorse(false, false, 0);
		}
		else
		{
			SetRidingHorse(true, pMinecraft->localplayers[iPad]->isRidingJumpable(), 0);
		}
		ShowHorseHealth(false);
		m_horseHealth = 0;
		ShowHealth(false);
		ShowFood(false);
		ShowAir(false);
		ShowArmour(false);		
		ShowExpBar(false);
		SetHealthAbsorb(0);			
	}

	if(pMinecraft->localplayers[iPad]->isRidingJumpable())
	{
		SetHorseJumpBarProgress(pMinecraft->localplayers[iPad]->getJumpRidingScale());
	}
	else if (pMinecraft->localgameModes[iPad]->hasExperience())
	{
		// Update xp progress
		ShowExpBar(true);

		SetExpBarProgress(pMinecraft->localplayers[iPad]->experienceProgress, pMinecraft->localplayers[iPad]->getXpNeededForNextLevel());

		// Update xp level
		SetExpLevel(pMinecraft->localplayers[iPad]->experienceLevel);
	}
	else
	{
		ShowExpBar(false);
		SetExpLevel(0);
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
		if(ui.GetMenuDisplayed(iPad) && (ucAlpha<15))
		{
			ucAlpha=15;
		}

		// check if we have the timer running for the opacity
		unsigned int uiOpacityTimer=app.GetOpacityTimer(iPad);
		if(uiOpacityTimer!=0)
		{
			if(uiOpacityTimer<10)
			{
				float fStep=(80.0f-static_cast<float>(ucAlpha))/10.0f;
				fVal=0.01f*(80.0f-((10.0f-static_cast<float>(uiOpacityTimer))*fStep));
			}
			else
			{
				fVal=0.01f*80.0f;
			}
		}
		else
		{
			fVal=0.01f*static_cast<float>(ucAlpha);
		}
	}
	else
	{
		// if we are in a menu, set the minimum opacity for tooltips to 15%
		if(ui.GetMenuDisplayed(iPad) && (ucAlpha<15))
		{
			ucAlpha=15;
		}
		fVal=0.01f*static_cast<float>(ucAlpha);
	}
	SetOpacity(fVal);

	bool bDisplayGui=app.GetGameStarted() && !ui.GetMenuDisplayed(iPad) && !(app.GetXuiAction(iPad)==eAppAction_AutosaveSaveGameCapturedThumbnail) && app.GetGameSettings(iPad,eGameSetting_DisplayHUD)!=0;
	if(bDisplayGui && pMinecraft->localplayers[iPad] != nullptr)
	{
		SetVisible(true);			
	}
	else
	{
		SetVisible(false);	
	}
}

void IUIScene_HUD::renderPlayerHealth()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	int iPad = getPad();

	ShowHealth(true);

	SetRegenerationEffect(pMinecraft->localplayers[iPad]->hasEffect(MobEffect::regeneration));

	// Update health
	bool blink = pMinecraft->localplayers[iPad]->invulnerableTime / 3 % 2 == 1;
	if (pMinecraft->localplayers[iPad]->invulnerableTime < 10) blink = false;
	int currentHealth = static_cast<int>(ceil(pMinecraft->localplayers[iPad]->getHealth()));
    int oldHealth = static_cast<int>(ceil(pMinecraft->localplayers[iPad]->lastHealth));
	bool bHasPoison = pMinecraft->localplayers[iPad]->hasEffect(MobEffect::poison);
	bool bHasWither = pMinecraft->localplayers[iPad]->hasEffect(MobEffect::wither);
	AttributeInstance *maxHealthAttribute = pMinecraft->localplayers[iPad]->getAttribute(SharedMonsterAttributes::MAX_HEALTH);
	float maxHealth = static_cast<float>(maxHealthAttribute->getValue());
	float totalAbsorption = pMinecraft->localplayers[iPad]->getAbsorptionAmount();

	// Update armour
	int armor = pMinecraft->localplayers[iPad]->getArmorValue();

	SetHealth(currentHealth, oldHealth, blink, bHasPoison || bHasWither, bHasWither);
	SetHealthAbsorb(totalAbsorption);

	if(armor > 0)
	{
		ShowArmour(true);
		SetArmour(armor);
	}
	else
	{
		ShowArmour(false);
	}

	shared_ptr<Entity> riding = pMinecraft->localplayers[iPad]->riding;

	if(riding == nullptr || riding && !riding->instanceof(eTYPE_LIVINGENTITY))
	{
		SetRidingHorse(false, false, 0);

		ShowFood(true);
		ShowHorseHealth(false);
		m_horseHealth = 0;

		// Update food
		//bool foodBlink = false;
		FoodData *foodData = pMinecraft->localplayers[iPad]->getFoodData();
		int food = foodData->getFoodLevel();
		int oldFood = foodData->getLastFoodLevel();
		bool hasHungerEffect = pMinecraft->localplayers[iPad]->hasEffect(MobEffect::hunger);
		int saturationLevel = pMinecraft->localplayers[iPad]->getFoodData()->getSaturationLevel();

		SetFood(food, oldFood, hasHungerEffect);
		SetFoodSaturationLevel(saturationLevel);

		// Update air
		if (pMinecraft->localplayers[iPad]->isUnderLiquid(Material::water))
		{
			ShowAir(true);
			int count = static_cast<int>(ceil((pMinecraft->localplayers[iPad]->getAirSupply() - 2) * 10.0f / Player::TOTAL_AIR_SUPPLY));
			int extra = static_cast<int>(ceil((pMinecraft->localplayers[iPad]->getAirSupply()) * 10.0f / Player::TOTAL_AIR_SUPPLY)) - count;
			SetAir(count, extra);
		}
		else
		{
			ShowAir(false);
		}
	}
	else if(riding->instanceof(eTYPE_LIVINGENTITY) )
	{
		shared_ptr<LivingEntity> living = dynamic_pointer_cast<LivingEntity>(riding);
		int riderCurrentHealth = static_cast<int>(ceil(living->getHealth()));
		float maxRiderHealth = living->getMaxHealth();

		SetRidingHorse(true, pMinecraft->localplayers[iPad]->isRidingJumpable(), maxRiderHealth);
		SetHorseHealth(riderCurrentHealth);
		ShowHorseHealth(true);
	}
}