#include "stdafx.h"
#include "IUIScene_HopperMenu.h"
#include "../Minecraft.World/net.minecraft.world.inventory.h"

IUIScene_AbstractContainerMenu::ESceneSection IUIScene_HopperMenu::GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY )
{
	ESceneSection newSection = eSection;

	int xOffset = 0;

	// Find the new section if there is one
	switch( eSection )
	{
		case eSectionHopperContents:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionHopperInventory;
				xOffset  = -2;
			}
			else if(eTapDirection == eTapStateUp)
			{
				xOffset = -2;
				newSection = eSectionHopperUsing;
			}
			break;
		case eSectionHopperInventory:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionHopperUsing;
			}
			else if(eTapDirection == eTapStateUp)
			{
				xOffset = 2;
				newSection = eSectionHopperContents;
			}
			break;
		case eSectionHopperUsing:
			if(eTapDirection == eTapStateDown)
			{
				xOffset = 2;
				newSection = eSectionHopperContents;
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionHopperInventory;
			}
			break;
		default:
			assert(false);
			break;
	}

	updateSlotPosition(eSection, newSection, eTapDirection, piTargetX, piTargetY, xOffset);

	return newSection;
}

int IUIScene_HopperMenu::getSectionStartOffset(ESceneSection eSection)
{
	int offset = 0;
	switch( eSection )
	{
		case eSectionHopperContents:
			offset = HopperMenu::CONTENTS_SLOT_START;
			break;
		case eSectionHopperInventory:
			offset = HopperMenu::INV_SLOT_START;
			break;
		case eSectionHopperUsing:
			offset = HopperMenu::USE_ROW_SLOT_START;
			break;
		default:
			assert( false );
			break;
	}
	return offset;
}