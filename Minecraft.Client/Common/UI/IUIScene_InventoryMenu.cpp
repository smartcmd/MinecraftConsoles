#include "stdafx.h"

#include "IUIScene_InventoryMenu.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"

IUIScene_AbstractContainerMenu::ESceneSection IUIScene_InventoryMenu::GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY )
{
	ESceneSection newSection = eSection;

	// Find the new section if there is one
	switch( eSection )
	{
		case eSectionInventoryArmor:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionInventoryInventory;
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionInventoryUsing;
			}
			break;
		case eSectionInventoryInventory:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionInventoryUsing;
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionInventoryArmor;
			}
			break;
		case eSectionInventoryUsing:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionInventoryArmor;
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionInventoryInventory;
			}
			break;
		default:
			assert( false );
			break;
	}

	updateSlotPosition(eSection, newSection, eTapDirection, piTargetX, piTargetY, 0);

	return newSection;
}

int IUIScene_InventoryMenu::getSectionStartOffset(ESceneSection eSection)
{
	int offset = 0;
	switch( eSection )
	{
		case eSectionInventoryArmor:
			offset = InventoryMenu::ARMOR_SLOT_START;
			break;
		case eSectionInventoryInventory:
			offset = InventoryMenu::INV_SLOT_START;
			break;
		case eSectionInventoryUsing:
			offset = InventoryMenu::INV_SLOT_START + 27;
			break;
		default:
			assert( false );
			break;
	}
	return offset;
}