#include "stdafx.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "IUIScene_EnchantingMenu.h"

IUIScene_AbstractContainerMenu::ESceneSection IUIScene_EnchantingMenu::GetSectionAndSlotInDirection( IUIScene_AbstractContainerMenu::ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY )
{
	IUIScene_AbstractContainerMenu::ESceneSection newSection = eSection;
	int xOffset = 0;

	// Find the new section if there is one
	switch( eSection )
	{
		case eSectionEnchantInventory:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionEnchantUsing;
			}
			else if(eTapDirection == eTapStateUp)
			{
				if( *piTargetX >= ENCHANT_SCENE_ENCHANT_BUTTONS_UP_OFFSET)
				{
					newSection = eSectionEnchantButton3;
				}
				else
				{
					newSection = eSectionEnchantSlot;
				}
			}
			break;
		case eSectionEnchantUsing:
			if(eTapDirection == eTapStateDown)
			{
				if( *piTargetX >= ENCHANT_SCENE_ENCHANT_BUTTONS_UP_OFFSET)
				{
					newSection = eSectionEnchantButton1;
				}
				else
				{
					newSection = eSectionEnchantSlot;
				}
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionEnchantInventory;
			}
			break;
		case eSectionEnchantSlot:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionEnchantInventory;
				xOffset = ENCHANT_SCENE_INGREDIENT_SLOT_DOWN_OFFSET;
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionEnchantUsing;
				xOffset = ENCHANT_SCENE_INGREDIENT_SLOT_DOWN_OFFSET;
			}
			else if(eTapDirection == eTapStateLeft || eTapDirection == eTapStateRight)
			{
				newSection = eSectionEnchantButton1;
			}
			break;
		case eSectionEnchantButton1:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionEnchantButton2;
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionEnchantUsing;
				xOffset = ENCHANT_SCENE_ENCHANT_BUTTONS_DOWN_OFFSET;
			}
			else if(eTapDirection == eTapStateLeft || eTapDirection == eTapStateRight)
			{
				newSection = eSectionEnchantSlot;
			}
			break;
		case eSectionEnchantButton2:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionEnchantButton3;
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionEnchantButton1;
			}
			else if(eTapDirection == eTapStateLeft || eTapDirection == eTapStateRight)
			{
				newSection = eSectionEnchantSlot;
			}
			break;
		case eSectionEnchantButton3:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionEnchantInventory;
				xOffset = ENCHANT_SCENE_ENCHANT_BUTTONS_DOWN_OFFSET;
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionEnchantButton2;
			}
			else if(eTapDirection == eTapStateLeft || eTapDirection == eTapStateRight)
			{
				newSection = eSectionEnchantSlot;
			}
			break;
		default:
			assert( false );
			break;
	}

	updateSlotPosition(eSection, newSection, eTapDirection, piTargetX, piTargetY, xOffset);

	return newSection;
}

void IUIScene_EnchantingMenu::handleOtherClicked(int iPad, ESceneSection eSection, int buttonNum, bool quickKey)
{
	int index = -1;
	// Old xui code
#if 0
	HXUIOBJ hFocusObject = GetFocus(iPad);
	if(hFocusObject == m_enchant1->m_hObj) index = 0;
	else if(hFocusObject == m_enchant2->m_hObj) index = 1;
	else if(hFocusObject == m_enchant3->m_hObj) index = 2;
#endif

	switch(eSection)
	{
	case eSectionEnchantButton1:
		index = 0;
		break;
	case eSectionEnchantButton2:
		index = 1;
		break;
	case eSectionEnchantButton3:
		index = 2;
		break;
	};
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if (index >= 0 && m_menu->clickMenuButton(dynamic_pointer_cast<Player>(pMinecraft->localplayers[iPad]), index))
	{
		pMinecraft->localgameModes[iPad]->handleInventoryButtonClick(m_menu->containerId, index);
	}
}

int IUIScene_EnchantingMenu::getSectionStartOffset(ESceneSection eSection)
{
	int offset = 0;
	switch( eSection )
	{
	case eSectionEnchantSlot:
		offset = 0;
		break;
	case eSectionEnchantInventory:
		offset = 1;
		break;
	case eSectionEnchantUsing:
		offset = 1 + 27;
		break;
	default:
		assert( false );
		break;
	};
	return offset;
}

bool IUIScene_EnchantingMenu::IsSectionSlotList( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionEnchantInventory:
		case eSectionEnchantUsing:
		case eSectionEnchantSlot:
			return true;
	}
	return false;
}

EnchantmentMenu *IUIScene_EnchantingMenu::getMenu()
{
	return static_cast<EnchantmentMenu *>(m_menu);
}