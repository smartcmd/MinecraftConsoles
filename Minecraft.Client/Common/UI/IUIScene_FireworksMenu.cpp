#include "stdafx.h"

#include "IUIScene_FireworksMenu.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"

IUIScene_AbstractContainerMenu::ESceneSection IUIScene_FireworksMenu::GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY )
{
	ESceneSection newSection = eSection;
	int xOffset = 0;
	int yOffset = 0;

	// Find the new section if there is one
	switch( eSection )
	{
		case eSectionFireworksIngredients:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionFireworksInventory;
				xOffset = -1;
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionFireworksUsing;
				xOffset = -1;
			}
			else if(eTapDirection == eTapStateLeft)
			{
				newSection = eSectionFireworksResult;
			}
			else if(eTapDirection == eTapStateRight)
			{
				newSection = eSectionFireworksResult;
			}
			break;
		case eSectionFireworksResult:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionFireworksInventory;
				xOffset = -7;
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionFireworksUsing;
				xOffset = -7;
			}
			else if(eTapDirection == eTapStateLeft)
			{
				newSection = eSectionFireworksIngredients;
				yOffset = -1;
				*piTargetX = getSectionColumns(eSectionFireworksIngredients);
			}
			else if(eTapDirection == eTapStateRight)
			{
				newSection = eSectionFireworksIngredients;
				yOffset = -1;
				*piTargetX = 0;
			}
			break;
		case eSectionFireworksInventory:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionFireworksUsing;
			}
			else if(eTapDirection == eTapStateUp)
			{
				if(*piTargetX < 6)
				{
					newSection = eSectionFireworksIngredients;
					xOffset = 1;
				}
				else
				{
					newSection = eSectionFireworksResult;
				}
			}
			break;
		case eSectionFireworksUsing:
			if(eTapDirection == eTapStateDown)
			{
				if(*piTargetX < 6)
				{
					newSection = eSectionFireworksIngredients;
					xOffset = 1;
				}
				else
				{
					newSection = eSectionFireworksResult;
				}
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionFireworksInventory;
			}
			break;
		default:
			assert( false );
			break;
	}

	updateSlotPosition(eSection, newSection, eTapDirection, piTargetX, piTargetY, xOffset, yOffset);

	return newSection;
}

int IUIScene_FireworksMenu::getSectionStartOffset(ESceneSection eSection)
{
	int offset = 0;
	switch( eSection )
	{
		
		case eSectionFireworksIngredients:
			offset = FireworksMenu::CRAFT_SLOT_START;
			break;

		case eSectionFireworksResult:
			offset = FireworksMenu::RESULT_SLOT;
			break;
		case eSectionFireworksInventory:
			offset = FireworksMenu::INV_SLOT_START;
			break;
		case eSectionFireworksUsing:
			offset = FireworksMenu::INV_SLOT_START + 27;
			break;
		default:
			assert( false );
			break;
	}
	return offset;
}