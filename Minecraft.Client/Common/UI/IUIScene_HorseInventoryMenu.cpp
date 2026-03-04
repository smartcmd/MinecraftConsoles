#include "stdafx.h"
#include "IUIScene_HorseInventoryMenu.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.animal.h"

IUIScene_AbstractContainerMenu::ESceneSection IUIScene_HorseInventoryMenu::GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY )
{
	ESceneSection newSection = eSection;

	int xOffset = 0;
	int yOffset = 0;

	// Find the new section if there is one
	switch( eSection )
	{
	case eSectionHorseUsing:
		if(eTapDirection == eTapStateDown)
		{
			if(m_horse->isChestedHorse() &&  *piTargetX >= 4)
			{
				newSection = eSectionHorseChest;
				xOffset = 4;
			}
			else
			{
				newSection = eSectionHorseSaddle;
			}
		}
		else if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionHorseInventory;
		}
		break;
	case eSectionHorseInventory:
		if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionHorseUsing;
		}
		else if(eTapDirection == eTapStateUp)
		{
			if(m_horse->isChestedHorse() && *piTargetX >= 4)
			{
				xOffset = 4;
				newSection = eSectionHorseChest;
			}
			else if(m_horse->canWearArmor())
			{
				newSection = eSectionHorseArmor;
			}
			else
			{
				newSection = eSectionHorseSaddle;
			}
		}
		break;
	case eSectionHorseChest:
		if(eTapDirection == eTapStateDown)
		{
			xOffset = -4;
			newSection = eSectionHorseInventory;
		}
		else if(eTapDirection == eTapStateUp)
		{
			xOffset = -4;
			newSection = eSectionHorseUsing;
		}
		else if(eTapDirection == eTapStateLeft)
		{
			if(*piTargetX < 0)
			{
				if(m_horse->canWearArmor() && *piTargetY == 1)
				{
					newSection = eSectionHorseArmor;
				}
				else if( *piTargetY == 0)
				{
					newSection = eSectionHorseSaddle;
				}
			}
		}
		else if(eTapDirection == eTapStateRight)
		{
			if(*piTargetX >= getSectionColumns(eSectionHorseChest))
			{
				if(m_horse->canWearArmor() && *piTargetY == 1)
				{
					newSection = eSectionHorseArmor;
				}
				else if( *piTargetY == 0)
				{
					newSection = eSectionHorseSaddle;
				}
			}
		}
		break;
	case eSectionHorseArmor:
		if(eTapDirection == eTapStateDown)
		{
			if(m_horse->isChestedHorse())
			{
				newSection = eSectionHorseChest;
			}
			else
			{
				newSection = eSectionHorseInventory;
			}
		}
		else if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionHorseSaddle;
		}
		else if(eTapDirection == eTapStateRight)
		{
			if(m_horse->isChestedHorse())
			{
				yOffset = -1;
				*piTargetX = 0;
				newSection = eSectionHorseChest;
			}
		}
		else if(eTapDirection == eTapStateLeft)
		{
			if(m_horse->isChestedHorse())
			{
				yOffset = -1;
				*piTargetX = getSectionColumns(eSectionHorseChest);
				newSection = eSectionHorseChest;
			}
		}
		break;
	case eSectionHorseSaddle:
		if(eTapDirection == eTapStateDown)
		{
			if(m_horse->canWearArmor())
			{
				newSection = eSectionHorseArmor;
			}
			else
			{
				newSection = eSectionHorseInventory;
			}
		}
		else if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionHorseUsing;
		}
		else if(eTapDirection == eTapStateRight)
		{
			if(m_horse->isChestedHorse())
			{
				*piTargetX = 0;
				newSection = eSectionHorseChest;
			}
		}
		else if(eTapDirection == eTapStateLeft)
		{
			if(m_horse->isChestedHorse())
			{
				*piTargetX = getSectionColumns(eSectionHorseChest);
				newSection = eSectionHorseChest;
			}
		}
		break;
	default:
		assert(false);
		break;
	}

	updateSlotPosition(eSection, newSection, eTapDirection, piTargetX, piTargetY, xOffset, yOffset);

	return newSection;
}

// TODO: Offset will vary by type of horse, add in once horse menu and horse entity are implemented
int IUIScene_HorseInventoryMenu::getSectionStartOffset(ESceneSection eSection)
{
	int offset = 0;
	switch( eSection )
	{
	case eSectionHorseSaddle:
		offset = EntityHorse::INV_SLOT_SADDLE;
		break;
	case eSectionHorseArmor:
		offset = EntityHorse::INV_SLOT_ARMOR;
		break;
	case eSectionHorseChest:
		offset = EntityHorse::INV_BASE_COUNT;
		break;
	case eSectionHorseInventory:
		offset = EntityHorse::INV_BASE_COUNT;
		if(m_horse->isChestedHorse())
		{
			offset += EntityHorse::INV_DONKEY_CHEST_COUNT;
		}
		break;
	case eSectionHorseUsing:
		offset = EntityHorse::INV_BASE_COUNT + 27;
		if(m_horse->isChestedHorse())
		{
			offset += EntityHorse::INV_DONKEY_CHEST_COUNT;
		}
		break;
	default:
		assert( false );
		break;
	}
	return offset;
}

bool IUIScene_HorseInventoryMenu::IsSectionSlotList( ESceneSection eSection )
{
	switch( eSection )
	{
	case eSectionHorseChest:
		if(!m_horse->isChestedHorse())
			return false;
		else
			return true;
	case eSectionHorseArmor:
		if(!m_horse->canWearArmor())
			return false;
		else
			return true;
	case eSectionHorseSaddle:
	case eSectionHorseInventory:
	case eSectionHorseUsing:
		return true;
	}
	return false;
}

bool IUIScene_HorseInventoryMenu::IsVisible( ESceneSection eSection )
{
	switch( eSection )
	{
	case eSectionHorseChest:
		if(!m_horse->isChestedHorse())
			return false;
		else
			return true;
	case eSectionHorseArmor:
		if(!m_horse->canWearArmor())
			return false;
		else
			return true;
	case eSectionHorseSaddle:
	case eSectionHorseInventory:
	case eSectionHorseUsing:
		return true;
	}
	return false;
}