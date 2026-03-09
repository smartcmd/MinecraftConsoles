#include "stdafx.h"
#include "XUI_HUD.h"
#include "..\..\Minecraft.h"
#include "..\..\Gui.h"
#include "..\..\MultiplayerLocalPlayer.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\Random.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.effect.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.material.h"

HRESULT CXuiSceneHud::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_iPad = *static_cast<int *>(pInitData->pvInitData);

	MapChildControls();

	XuiElementGetPosition(m_hObj,&m_OriginalPosition);
	
	m_tickCount = 0;

	return S_OK;
}

HRESULT CXuiSceneHud::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	
	app.ReloadHudScene(m_iPad, bJoining);

	return S_OK;
}

HRESULT CXuiSceneHud::OnCustomMessage_TickScene()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->localplayers[m_iPad] == nullptr || pMinecraft->localgameModes[m_iPad] == nullptr) return S_OK;

	++m_tickCount;

	int iGuiScale;	

	if(pMinecraft->localplayers[m_iPad]->m_iScreenSection == C4JRender::VIEWPORT_TYPE_FULLSCREEN)
	{
		iGuiScale=app.GetGameSettings(m_iPad,eGameSetting_UISize);
	}
	else
	{
		iGuiScale=app.GetGameSettings(m_iPad,eGameSetting_UISizeSplitscreen);
	}

	int startFrame = 0;
	switch(iGuiScale)
	{
	case 0:
		XuiElementFindNamedFrame(m_hObj, L"ScaleSmall", &startFrame);
		break;
	case 1:
		XuiElementFindNamedFrame(m_hObj, L"Normal", &startFrame);
		break;
	case 2:
		XuiElementFindNamedFrame(m_hObj, L"ScaleLarge", &startFrame);
		break;
	}
	if(startFrame >= 0) XuiElementPlayTimeline( m_hObj, startFrame, startFrame, startFrame, FALSE, TRUE);

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

	// Update inventory
	float opacity = 1.0f;
	GetOpacity(&opacity);
	int selected = pMinecraft->localplayers[m_iPad]->inventory->selected;
	for(unsigned int j = 0; j < 9; ++j)
	{
		m_hotbarIcon[j]->SetSlot(m_hotbarIcon[j]->m_hObj,pMinecraft->localplayers[m_iPad]->inventoryMenu->getSlot(InventoryMenu::USE_ROW_SLOT_START + j));
		m_hotbarIcon[j]->SetUserIndex(m_hotbarIcon[j]->m_hObj, m_iPad );

		if(j == selected)
		{
			m_hotbarIcon[j]->SetEnable(FALSE); //Makes the selected overlay display
		}
		else
		{
			m_hotbarIcon[j]->SetEnable(TRUE); //Hides the selected overlay display
		}

		m_hotbarIcon[j]->SetAlpha( m_hotbarIcon[j]->m_hObj, opacity );
	}

	// Update xp progress
	if (pMinecraft->localgameModes[m_iPad]->canHurtPlayer())
	{
		int xpNeededForNextLevel = pMinecraft->localplayers[m_iPad]->getXpNeededForNextLevel();
		int progress = static_cast<int>(pMinecraft->localplayers[m_iPad]->experienceProgress * xpNeededForNextLevel);

		m_ExperienceProgress.SetShow(TRUE);
		m_ExperienceProgress.SetRange(0,xpNeededForNextLevel);
		m_ExperienceProgress.SetValue(progress);
	}
	else
	{
		m_ExperienceProgress.SetShow(FALSE);
	}

	// Update xp level
	if (pMinecraft->localgameModes[m_iPad]->hasExperience() && pMinecraft->localplayers[m_iPad]->experienceLevel > 0)
	{
		m_xpLevel.SetShow(TRUE);

		wchar_t formatted[10];
		swprintf(formatted, 10, L"%d",pMinecraft->localplayers[m_iPad]->experienceLevel);

		m_xpLevel.SetText(formatted);
	}
	else
	{
		m_xpLevel.SetShow(FALSE);
	}

	if (pMinecraft->localgameModes[m_iPad]->canHurtPlayer())
	{
		m_random.setSeed(m_tickCount * 312871);

		// Update health
		int heartOffsetIndex = -1;
		if (pMinecraft->localplayers[m_iPad]->hasEffect(MobEffect::regeneration))
		{
			heartOffsetIndex = m_tickCount % 25;
		}

		bool blink = pMinecraft->localplayers[m_iPad]->invulnerableTime / 3 % 2 == 1;
		if (pMinecraft->localplayers[m_iPad]->invulnerableTime < 10) blink = false;
		int iHealth = pMinecraft->localplayers[m_iPad]->getHealth();
		int iLastHealth = pMinecraft->localplayers[m_iPad]->lastHealth;
		bool bHasPoison = pMinecraft->localplayers[m_iPad]->hasEffect(MobEffect::poison); 
		for (int icon = 0; icon < Player::MAX_HEALTH / 2; icon++)
		{
			if(blink)
			{
				if (icon * 2 + 1 < iLastHealth || icon * 2 + 1 < iHealth)
				{
					// Full
					if(bHasPoison)
					{
						m_healthIcon[icon].PlayVisualRange(L"FullPoisonFlash",nullptr,L"FullPoisonFlash");
					}
					else
					{
						m_healthIcon[icon].PlayVisualRange(L"FullFlash",nullptr,L"FullFlash");
					}
				}
				else if (icon * 2 + 1 == iLastHealth || icon * 2 + 1 == iHealth)
				{
					// Half
					if(bHasPoison)
					{
						m_healthIcon[icon].PlayVisualRange(L"HalfPoisonFlash",nullptr,L"HalfPoisonFlash");
					}
					else
					{
						m_healthIcon[icon].PlayVisualRange(L"HalfFlash",nullptr,L"HalfFlash");
					}
				}
				else
				{
					// Empty
					m_healthIcon[icon].PlayVisualRange(L"NormalFlash",nullptr,L"NormalFlash");
				}
			}
			else
			{
				if (icon * 2 + 1 < iHealth)
				{
					// Full
					if(bHasPoison)
					{
						m_healthIcon[icon].PlayVisualRange(L"FullPoison",nullptr,L"FullPoison");
					}
					else
					{
						m_healthIcon[icon].PlayVisualRange(L"Full",nullptr,L"Full");
					}
				}
				else if (icon * 2 + 1 == iHealth)
				{
					// Half
					if(bHasPoison)
					{
						m_healthIcon[icon].PlayVisualRange(L"HalfPoison",nullptr,L"HalfPoison");
					}
					else
					{
						m_healthIcon[icon].PlayVisualRange(L"Half",nullptr,L"Half");
					}
				}
				else
				{
					// Empty
					m_healthIcon[icon].PlayVisualRange(L"Normal",nullptr,L"Normal");
				}
			}

			float yo = 0;
			if (iHealth <= 4)
			{
				yo = static_cast<float>(m_random.nextInt(2)) * (iGuiScale+1);
			}
			if (icon == heartOffsetIndex)
			{
				yo = -2.0f * (iGuiScale+1);
			}

			D3DXVECTOR3 pos;
			m_healthIcon[icon].GetPosition(&pos);
			// TODO - 4J Stu - This should be scaled based on gui scale and overall size (ie full, split or 480)
			pos.y = yo;
			m_healthIcon[icon].SetPosition(&pos);

		}

		// Update food
		bool foodBlink = false;
		FoodData *foodData = pMinecraft->localplayers[m_iPad]->getFoodData();
		int food = foodData->getFoodLevel();
		int oldFood = foodData->getLastFoodLevel();
		bool hasHungerEffect = pMinecraft->localplayers[m_iPad]->hasEffect(MobEffect::hunger);
		int saturationLevel = pMinecraft->localplayers[m_iPad]->getFoodData()->getSaturationLevel();
		for (int icon = 0; icon < 10; icon++)
		{
			if(foodBlink)
			{
				if (icon * 2 + 1 < oldFood || icon * 2 + 1 < food)
				{
					// Full
					if(hasHungerEffect)
					{
						m_foodIcon[icon].PlayVisualRange(L"FullPoisonFlash",nullptr,L"FullPoisonFlash");
					}
					else
					{
						m_foodIcon[icon].PlayVisualRange(L"FullFlash",nullptr,L"FullFlash");
					}
				}
				else if (icon * 2 + 1 == oldFood || icon * 2 + 1 == food)
				{
					// Half
					if(hasHungerEffect)
					{
						m_foodIcon[icon].PlayVisualRange(L"HalfPoisonFlash",nullptr,L"HalfPoisonFlash");
					}
					else
					{
						m_foodIcon[icon].PlayVisualRange(L"HalfFlash",nullptr,L"HalfFlash");
					}
				}
				else
				{
					// Empty
					m_foodIcon[icon].PlayVisualRange(L"NormalFlash",nullptr,L"NormalFlash");
				}
			}
			else
			{
				if (icon * 2 + 1 < food)
				{
					// Full
					if(hasHungerEffect)
					{
						m_foodIcon[icon].PlayVisualRange(L"FullPoison",nullptr,L"FullPoison");
					}
					else
					{
						m_foodIcon[icon].PlayVisualRange(L"Full",nullptr,L"Full");
					}
				}
				else if (icon * 2 + 1 == food)
				{
					// Half
					if(hasHungerEffect)
					{
						m_foodIcon[icon].PlayVisualRange(L"HalfPoison",nullptr,L"HalfPoison");
					}
					else
					{
						m_foodIcon[icon].PlayVisualRange(L"Half",nullptr,L"Half");
					}
				}
				else
				{
					// Empty
					if(hasHungerEffect)
					{
						m_foodIcon[icon].PlayVisualRange(L"NormalPoison",nullptr,L"NormalPoison");
					}
					else
					{
						m_foodIcon[icon].PlayVisualRange(L"Normal",nullptr,L"Normal");
					}
				}
			}

				
			float yo = 0;
			if (saturationLevel <= 0)
			{
				if ((m_tickCount % (food * 3 + 1)) == 0)
				{
					yo = static_cast<float>(m_random.nextInt(3) - 1) * (iGuiScale+1);
				}
			}

			D3DXVECTOR3 pos;
			m_foodIcon[icon].GetPosition(&pos);
			// TODO - 4J Stu - This should be scaled based on gui scale and overall size (ie full, split or 480)
			pos.y = yo;
			m_foodIcon[icon].SetPosition(&pos);
		}

		// Update armour
		int armor = pMinecraft->localplayers[m_iPad]->getArmorValue();
		if(armor > 0)
		{
			m_armourGroup.SetShow(TRUE);
			for (int icon = 0; icon < 10; icon++)
			{
				if (icon * 2 + 1 < armor) m_armourIcon[icon].PlayVisualRange(L"Full",nullptr,L"Full");
				else if (icon * 2 + 1 == armor) m_armourIcon[icon].PlayVisualRange(L"Half",nullptr,L"Half");
				else if (icon * 2 + 1 > armor) m_armourIcon[icon].PlayVisualRange(L"Normal",nullptr,L"Normal");
			}
		}
		else
		{
			m_armourGroup.SetShow(FALSE);
		}

		// Update air
		if (pMinecraft->localplayers[m_iPad]->isUnderLiquid(Material::water))
		{
			m_airGroup.SetShow(TRUE);
			int count = static_cast<int>(ceil((pMinecraft->localplayers[m_iPad]->getAirSupply() - 2) * 10.0f / Player::TOTAL_AIR_SUPPLY));
			int extra = static_cast<int>(ceil((pMinecraft->localplayers[m_iPad]->getAirSupply()) * 10.0f / Player::TOTAL_AIR_SUPPLY)) - count;
			for (int icon = 0; icon < 10; icon++)
			{
				// Air bubbles
				if (icon < count)
				{
					m_airIcon[icon].SetShow(TRUE);
					m_airIcon[icon].PlayVisualRange(L"Bubble",nullptr,L"Bubble");
				}
				else if(icon < count + extra)
				{
					m_airIcon[icon].SetShow(TRUE);
					m_airIcon[icon].PlayVisualRange(L"Pop",nullptr,L"Pop");
				}
				else m_airIcon[icon].SetShow(FALSE);
			}
		}
		else
		{
			m_airGroup.SetShow(FALSE);
		}
	}
	else
	{
		m_healthGroup.SetShow(FALSE);
		m_foodGroup.SetShow(FALSE);
		m_armourGroup.SetShow(FALSE);
		m_airGroup.SetShow(FALSE);
	}
	return S_OK;
}

HRESULT CXuiSceneHud::OnCustomMessage_DLCInstalled()
{
	// mounted DLC may have changed
	bool bPauseMenuDisplayed=false;
	bool bInGame=(Minecraft::GetInstance()->level!=nullptr);
	// ignore this if we have menus up - they'll deal with it
	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		if(app.IsPauseMenuDisplayed(i))
		{
			bPauseMenuDisplayed=true;
			break;
		}
	}

	if(bInGame && !bPauseMenuDisplayed)
	{
		app.StartInstallDLCProcess(ProfileManager.GetPrimaryPad());
	}

	// this will send a CustomMessage_DLCMountingComplete when done
	return S_OK;
}


HRESULT CXuiSceneHud::OnCustomMessage_DLCMountingComplete()
{	
	// A join from invite may have installed the trial pack during the game.
	// Need to switch to the texture pack if it's not being used yet, and turn off background downloads

	// what texture pack are we using?
	// 	TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
	// 	if(tPack->getDLCParentPackId()!=app.GetRequiredTexturePackID())
	// 	{
	// 		app.DebugPrintf("DLC install finished, and the game is using a texture pack that isn't the required one\n");
	// 	}
	return S_OK;
}