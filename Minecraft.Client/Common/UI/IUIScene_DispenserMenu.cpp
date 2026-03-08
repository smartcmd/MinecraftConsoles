#include "stdafx.h"

#include "IUIScene_DispenserMenu.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"

IUIScene_AbstractContainerMenu::ESceneSection IUIScene_DispenserMenu::GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY )
{
	ESceneSection newSection = eSection;
	int xOffset = 0;

	// Find the new section if there is one
	switch( eSection )
	{
	case eSectionTrapTrap:
		if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionTrapInventory;
			xOffset = -TRAP_SCENE_TRAP_SLOT_OFFSET;
		}
		else if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionTrapUsing;
			xOffset = -TRAP_SCENE_TRAP_SLOT_OFFSET;
		}
		break;
	case eSectionTrapInventory:
		if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionTrapUsing;
		}
		else if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionTrapTrap;
			xOffset = TRAP_SCENE_TRAP_SLOT_OFFSET;
		}
		break;
	case eSectionTrapUsing:
		if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionTrapTrap;
			xOffset = TRAP_SCENE_TRAP_SLOT_OFFSET;
		}
		else if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionTrapInventory;
		}
		break;
	default:
		assert( false );
		break;
	}

	updateSlotPosition(eSection, newSection, eTapDirection, piTargetX, piTargetY, xOffset);

	return newSection;
}

int IUIScene_DispenserMenu::getSectionStartOffset(ESceneSection eSection)
{
	int offset = 0;
	switch( eSection )
	{
	case eSectionTrapTrap:
		offset = 0;
		break;
	case eSectionTrapInventory:
		offset = 9;
		break;
	case eSectionTrapUsing:
		offset = 9 + 27;
		break;
	default:
		assert( false );
		break;
	}
	return offset;
}