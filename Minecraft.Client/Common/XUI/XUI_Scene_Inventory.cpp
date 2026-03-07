#include "stdafx.h"

#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../../Minecraft.World/net.minecraft.stats.h"
#include "../../../Minecraft.World/net.minecraft.world.effect.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../Common/Tutorial/Tutorial.h"
#include "../../Common/Tutorial/TutorialMode.h"
#include "../../Common/Tutorial/TutorialEnum.h"
#include "../../Minecraft.h"
#include "XUI_Controls.h"
#include "XUI_Scene_Inventory.h"


//--------------------------------------------------------------------------------------
// Name: CXuiSceneInventory::OnInit
// Desc: Message handler for XM_INIT
//--------------------------------------------------------------------------------------
HRESULT CXuiSceneInventory::OnInit( XUIMessageInit *pInitData, BOOL &bHandled )
{
	D3DXVECTOR3 vec;
	MapChildControls();

	Minecraft *pMinecraft = Minecraft::GetInstance();

	InventoryScreenInput *initData = (InventoryScreenInput *)pInitData->pvInitData;
	m_iPad=initData->iPad;
	m_bSplitscreen=initData->bSplitscreen;

	// if we are in splitscreen, then we need to figure out if we want to move this scene

	if(m_bSplitscreen)
	{
		if(m_bSplitscreen)
		{
			app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
		}
	}

#ifdef _XBOX
	if( pMinecraft->localgameModes[initData->iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[initData->iPad];
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Inventory_Menu, this);
	}
#endif

	InventoryMenu *menu = dynamic_cast<InventoryMenu *>( initData->player->inventoryMenu );

#if 0
	// TODO Inventory dimensions need defined as constants
	m_armorGroup->SetData( initData->iPad, menu, 4, 1, InventoryMenu::ARMOR_SLOT_START );
#endif
	InitDataAssociations(initData->iPad, menu);

	initData->player->awardStat(GenericStats::openInventory(), GenericStats::param_noArgs());

	CXuiSceneAbstractContainer::Initialize( initData->iPad, menu, false, InventoryMenu::INV_SLOT_START, eSectionInventoryUsing, eSectionInventoryMax, initData->bNavigateBack );

	delete initData;

	float fWidth;
	m_effectsGroup.GetBounds(&fWidth, &m_effectAreaHeight); // Get total height available for effects display
	m_hEffectDisplayA[0]->GetBounds(&fWidth, &m_effectDisplayHeight); // Get height of one effect

	D3DXVECTOR3 firstEffectPos, secondEffectPos;
	m_hEffectDisplayA[0]->GetPosition(&firstEffectPos);
	m_hEffectDisplayA[1]->GetPosition(&secondEffectPos);
	m_effectDisplaySpacing = firstEffectPos.y - secondEffectPos.y; // Stack from the bottom

	updateEffectsDisplay();
	XuiSetTimer(m_hObj,INVENTORY_UPDATE_EFFECTS_TIMER_ID,INVENTORY_UPDATE_EFFECTS_TIMER_TIME);

	return S_OK;
}

HRESULT CXuiSceneInventory::OnDestroy()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( pMinecraft->localgameModes[m_iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];
		if(gameMode != NULL) gameMode->getTutorial()->changeTutorialState(m_previousTutorialState);
	}

	// 4J Stu - Fix for #11302 - TCR 001: Network Connectivity: Host crashed after being killed by the client while accessing a chest during burst packet loss.
	// We need to make sure that we call closeContainer() anytime this menu is closed, even if it is forced to close by some other reason (like the player dying)
	if(Minecraft::GetInstance()->localplayers[m_iPad] != NULL) Minecraft::GetInstance()->localplayers[m_iPad]->closeContainer();
	return S_OK;
}

HRESULT CXuiSceneInventory::handleCustomTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	if(pTimer->nId == INVENTORY_UPDATE_EFFECTS_TIMER_ID)
	{
		updateEffectsDisplay();
		bHandled = TRUE;
	}
	return S_OK;
}

CXuiControl* CXuiSceneInventory::GetSectionControl( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionInventoryArmor:
			return (CXuiControl *)m_armorGroup;
			break;
		case eSectionInventoryInventory:
			return (CXuiControl *)m_inventoryControl;
			break;
		case eSectionInventoryUsing:
			return (CXuiControl *)m_useRowControl;
			break;
		default:
			assert( false );
			break;
	}
	return NULL;
}

CXuiCtrlSlotList* CXuiSceneInventory::GetSectionSlotList( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionInventoryArmor:
			return m_armorGroup;
			break;
		case eSectionInventoryInventory:
			return m_inventoryControl;
			break;
		case eSectionInventoryUsing:
			return m_useRowControl;
			break;
		default:
			assert( false );
			break;
	}
	return NULL;
}

// 4J Stu - Added to support auto-save. Need to re-associate on a navigate back
void CXuiSceneInventory::InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex /*= 0*/)
{
	// TODO Inventory dimensions need defined as constants
	m_armorGroup->SetData( iPad, menu, 4, 1, InventoryMenu::ARMOR_SLOT_START );

	CXuiSceneAbstractContainer::InitDataAssociations(iPad, menu, InventoryMenu::INV_SLOT_START);
}

void CXuiSceneInventory::updateEffectsDisplay()
{
	// Update with the current effects
	Minecraft *pMinecraft = Minecraft::GetInstance();
	shared_ptr<LocalPlayer> player = pMinecraft->localplayers[m_iPad];

	if(player == NULL) return;

	vector<MobEffectInstance *> *activeEffects = player->getActiveEffects();

	// Work out how to arrange the effects
	int effectCount = (int)activeEffects->size();

	// Total size of all effects + spacing, minus spacing for the last effect
	float fHeight = (effectCount * m_effectDisplaySpacing) - (m_effectDisplaySpacing - m_effectDisplayHeight);
	float fNextEffectYOffset = m_effectDisplaySpacing;

	if(fHeight > m_effectAreaHeight)
	{
		fNextEffectYOffset = (m_effectAreaHeight + 1) / effectCount;
		fNextEffectYOffset = floor(fNextEffectYOffset);
	}

	// Fill out details for display
	D3DXVECTOR3 position;
	m_hEffectDisplayA[0]->GetPosition(&position);
    auto it = activeEffects->begin();
    for(unsigned int i = 0; i < MAX_EFFECTS; ++i)
	{
		if(it != activeEffects->end())
		{
			m_hEffectDisplayA[i]->SetShow(TRUE);

			if(i > 0) position.y -= fNextEffectYOffset; // Stack from the bottom
			m_hEffectDisplayA[i]->SetPosition(&position);

			MobEffectInstance *effect = *it;

			MobEffect *mobEffect = MobEffect::effects[effect->getId()];
			if (mobEffect->hasIcon())
			{
				m_hEffectDisplayA[i]->setIcon(mobEffect->getIcon());
            }

			wstring effectString = app.GetString( effect->getDescriptionId() );//I18n.get(effect.getDescriptionId()).trim();
			if (effect->getAmplifier() > 0)
			{
				wstring potencyString = L"";
				switch(effect->getAmplifier())
				{
				case 1:
					potencyString = L" ";
					potencyString += app.GetString( IDS_POTION_POTENCY_1 );
					break;
				case 2:
					potencyString = L" ";
					potencyString += app.GetString( IDS_POTION_POTENCY_2 );
					break;
				case 3:
					potencyString = L" ";
					potencyString += app.GetString( IDS_POTION_POTENCY_3 );
					break;
				default:
					potencyString = app.GetString( IDS_POTION_POTENCY_0 );
					break;
				}
				effectString += potencyString;
			}
			m_hEffectDisplayA[i]->setName(effectString);

			wstring durationString = MobEffect::formatDuration(effect);
			m_hEffectDisplayA[i]->setDuration(durationString);

			++it;
		}
		else
		{
			m_hEffectDisplayA[i]->SetShow(FALSE);
		}
	}
	delete activeEffects;
}