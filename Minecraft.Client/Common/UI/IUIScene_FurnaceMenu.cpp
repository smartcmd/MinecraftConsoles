#include "stdafx.h"

#include "IUIScene_FurnaceMenu.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"

IUIScene_AbstractContainerMenu::ESceneSection IUIScene_FurnaceMenu::GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY )
{
	ESceneSection newSection = eSection;
	int xOffset = 0;

	// Find the new section if there is one
	switch( eSection )
	{
		case eSectionFurnaceResult:
			if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionFurnaceUsing;
				xOffset = FURNACE_SCENE_RESULT_SLOT_DOWN_OFFSET;
			}
			else if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionFurnaceInventory;
				xOffset = FURNACE_SCENE_RESULT_SLOT_DOWN_OFFSET;
			}
			else if(eTapDirection == eTapStateLeft)
			{
				newSection = eSectionFurnaceIngredient;
			}
			else if(eTapDirection == eTapStateRight)
			{
				newSection = eSectionFurnaceIngredient;
			}
			break;
		case eSectionFurnaceIngredient:
			if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionFurnaceUsing;
				xOffset = FURNACE_SCENE_FUEL_SLOT_DOWN_OFFSET;
			}
			else if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionFurnaceFuel;
			}
			else if(eTapDirection == eTapStateLeft)
			{
				newSection = eSectionFurnaceResult;
			}
			else if(eTapDirection == eTapStateRight)
			{
				newSection = eSectionFurnaceResult;
			}
			break;
		case eSectionFurnaceFuel:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionFurnaceInventory;
				xOffset = FURNACE_SCENE_FUEL_SLOT_DOWN_OFFSET;
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionFurnaceIngredient;
			}
			else if(eTapDirection == eTapStateLeft)
			{
				newSection = eSectionFurnaceResult;
			}
			else if(eTapDirection == eTapStateRight)
			{
				newSection = eSectionFurnaceResult;
			}
			break;
		case eSectionFurnaceInventory:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionFurnaceUsing;
			}
			else if(eTapDirection == eTapStateUp)
			{
				if( *piTargetX >= FURNACE_SCENE_RESULT_SLOT_UP_OFFSET)
				{
					newSection = eSectionFurnaceResult;
				}
				else
				{
					newSection = eSectionFurnaceFuel;
				}
			}
			break;
		case eSectionFurnaceUsing:
			if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionFurnaceInventory;
			}
			else if(eTapDirection == eTapStateDown)
			{				
				if( *piTargetX >= FURNACE_SCENE_RESULT_SLOT_UP_OFFSET)
				{
					newSection = eSectionFurnaceResult;
				}
				else
				{
					newSection = eSectionFurnaceIngredient;
				}
			}
			break;
		default:
			assert( false );
			break;
	}

	updateSlotPosition(eSection, newSection, eTapDirection, piTargetX, piTargetY, xOffset);

	return newSection;
}

int IUIScene_FurnaceMenu::getSectionStartOffset(ESceneSection eSection)
{
	int offset = 0;
	switch( eSection )
	{
		case eSectionFurnaceResult:
			offset = FurnaceMenu::RESULT_SLOT;
			break;
		case eSectionFurnaceFuel:
			offset = FurnaceMenu::FUEL_SLOT;
			break;
		case eSectionFurnaceIngredient:
			offset = FurnaceMenu::INGREDIENT_SLOT;
			break;
		case eSectionFurnaceInventory:
			offset = FurnaceMenu::INV_SLOT_START;
			break;
		case eSectionFurnaceUsing:
			offset = FurnaceMenu::INV_SLOT_START + 27;
			break;
		default:
			assert( false );
			break;
	}
	return offset;
}