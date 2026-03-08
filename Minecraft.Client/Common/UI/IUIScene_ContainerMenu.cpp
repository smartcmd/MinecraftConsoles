#include "stdafx.h"
#include "IUIScene_ContainerMenu.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"

IUIScene_AbstractContainerMenu::ESceneSection IUIScene_ContainerMenu::GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY )
{
	ESceneSection newSection = eSection;

	// Find the new section if there is one
	switch( eSection )
	{
		case eSectionContainerChest:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionContainerInventory;
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionContainerUsing;
			}
			break;
		case eSectionContainerInventory:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionContainerUsing;
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionContainerChest;
			}
			break;
		case eSectionContainerUsing:
			if(eTapDirection == eTapStateDown)
			{
				newSection = eSectionContainerChest;
			}
			else if(eTapDirection == eTapStateUp)
			{
				newSection = eSectionContainerInventory;
			}
			break;
		default:
			assert( false );
			break;
	}

	updateSlotPosition(eSection, newSection, eTapDirection, piTargetX, piTargetY, 0);

	return newSection;
}

int IUIScene_ContainerMenu::getSectionStartOffset(ESceneSection eSection)
{
	int offset = 0;
	switch( eSection )
	{
		case eSectionContainerChest:
			offset = 0;
			break;
		case eSectionContainerInventory:
			offset = m_menu->getSize() - (27+9);
			break;
		case eSectionContainerUsing:
			offset = m_menu->getSize() - 9;
			break;
		default:
			assert( false );
			break;
	}
	return offset;
}