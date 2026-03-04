#include "stdafx.h"

#include "IUIScene_BrewingMenu.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"

IUIScene_AbstractContainerMenu::ESceneSection IUIScene_BrewingMenu::GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY )
{
	ESceneSection newSection = eSection;
	int xOffset = 0;

	// Find the new section if there is one
	switch( eSection )
	{
	case eSectionBrewingBottle1:
		if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionBrewingIngredient;
		}
		else if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionBrewingInventory;
			xOffset = BREWING_SCENE_BOTTLE1_SLOT_DOWN_OFFSET;
		}
		else if(eTapDirection == eTapStateLeft)
		{
			newSection = eSectionBrewingBottle3;
		}
		else if(eTapDirection == eTapStateRight)
		{
			newSection = eSectionBrewingBottle2;
		}
		break;
	case eSectionBrewingBottle2:
		if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionBrewingIngredient;
		}
		else if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionBrewingInventory;
			xOffset = BREWING_SCENE_BOTTLE2_SLOT_DOWN_OFFSET;
		}
		else if(eTapDirection == eTapStateLeft)
		{
			newSection = eSectionBrewingBottle1;
		}
		else if(eTapDirection == eTapStateRight)
		{
			newSection = eSectionBrewingBottle3;
		}
		break;
	case eSectionBrewingBottle3:
		if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionBrewingIngredient;
		}
		else if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionBrewingInventory;
			xOffset = BREWING_SCENE_BOTTLE3_SLOT_DOWN_OFFSET;
		}
		else if(eTapDirection == eTapStateLeft)
		{
			newSection = eSectionBrewingBottle2;
		}
		else if(eTapDirection == eTapStateRight)
		{
			newSection = eSectionBrewingBottle1;
		}
		break;
	case eSectionBrewingIngredient:
		if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionBrewingUsing;
			xOffset = BREWING_SCENE_INGREDIENT_SLOT_DOWN_OFFSET;
		}
		else if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionBrewingBottle2;
		}
		break;
	case eSectionBrewingInventory:
		if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionBrewingUsing;
		}
		else if(eTapDirection == eTapStateUp)
		{
			if( *piTargetX <= BREWING_SCENE_BOTTLE1_SLOT_UP_OFFSET)
			{
				newSection = eSectionBrewingBottle1;
			}
			else if( *piTargetX <= BREWING_SCENE_BOTTLE2_SLOT_UP_OFFSET)
			{
				newSection = eSectionBrewingBottle2;
			}
			else if( *piTargetX >= BREWING_SCENE_BOTTLE3_SLOT_UP_OFFSET)
			{
				newSection = eSectionBrewingBottle3;
			}
		}
		break;
	case eSectionBrewingUsing:
		if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionBrewingInventory;
		}
		else if(eTapDirection == eTapStateDown)
		{				
			newSection = eSectionBrewingIngredient;
		}
		break;
	default:
		assert( false );
		break;
	}

	updateSlotPosition(eSection, newSection, eTapDirection, piTargetX, piTargetY, xOffset);

	return newSection;
}

int IUIScene_BrewingMenu::getSectionStartOffset(ESceneSection eSection)
{
	int offset = 0;
	switch( eSection )
	{
	case eSectionBrewingBottle1:
		offset = BrewingStandMenu::BOTTLE_SLOT_START;
		break;
	case eSectionBrewingBottle2:
		offset = BrewingStandMenu::BOTTLE_SLOT_START + 1;
		break;
	case eSectionBrewingBottle3:
		offset = BrewingStandMenu::BOTTLE_SLOT_START + 2;
		break;
	case eSectionBrewingIngredient:
		offset = BrewingStandMenu::INGREDIENT_SLOT;
		break;
	case eSectionBrewingInventory:
		offset = BrewingStandMenu::INV_SLOT_START;
		break;
	case eSectionBrewingUsing:
		offset = BrewingStandMenu::INV_SLOT_START + 27;
		break;
	default:
		assert( false );
		break;
	}
	return offset;
}