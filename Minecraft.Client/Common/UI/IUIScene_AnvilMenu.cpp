#include "stdafx.h"
#include "IUIScene_AnvilMenu.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\InputOutputStream.h"
#include "..\..\..\Minecraft.World\net.minecraft.network.packet.h"
#include "..\..\Minecraft.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "..\..\ClientConnection.h"

IUIScene_AnvilMenu::IUIScene_AnvilMenu()
{
	m_inventory = nullptr;
	m_repairMenu = NULL;
	m_itemName = L"";
}

IUIScene_AbstractContainerMenu::ESceneSection IUIScene_AnvilMenu::GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY )
{
	ESceneSection newSection = eSection;
	int xOffset = 0;

	// Find the new section if there is one
	switch( eSection )
	{
	case eSectionAnvilItem1:
		if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionAnvilName;
		}
		else if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionAnvilInventory;
			xOffset = ANVIL_SCENE_ITEM1_SLOT_DOWN_OFFSET;
		}
		else if(eTapDirection == eTapStateLeft)
		{
			newSection = eSectionAnvilResult;
		}
		else if(eTapDirection == eTapStateRight)
		{
			newSection = eSectionAnvilItem2;
		}
		break;
	case eSectionAnvilItem2:
		if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionAnvilName;
		}
		else if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionAnvilInventory;
			xOffset = ANVIL_SCENE_ITEM2_SLOT_DOWN_OFFSET;
		}
		else if(eTapDirection == eTapStateLeft)
		{
			newSection = eSectionAnvilItem1;
		}
		else if(eTapDirection == eTapStateRight)
		{
			newSection = eSectionAnvilResult;
		}
		break;
	case eSectionAnvilResult:
		if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionAnvilName;
		}
		else if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionAnvilInventory;
			xOffset = ANVIL_SCENE_RESULT_SLOT_DOWN_OFFSET;
		}
		else if(eTapDirection == eTapStateLeft)
		{
			newSection = eSectionAnvilItem2;
		}
		else if(eTapDirection == eTapStateRight)
		{
			newSection = eSectionAnvilItem1;
		}
		break;
	case eSectionAnvilName:
		if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionAnvilUsing;
			xOffset = ANVIL_SCENE_ITEM2_SLOT_UP_OFFSET;
		}
		else if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionAnvilItem2;
		}
		break;
	case eSectionAnvilInventory:
		if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionAnvilUsing;
		}
		else if(eTapDirection == eTapStateUp)
		{
			if( *piTargetX <= ANVIL_SCENE_ITEM1_SLOT_UP_OFFSET)
			{
				newSection = eSectionAnvilItem1;
			}
			else if( *piTargetX <= ANVIL_SCENE_ITEM2_SLOT_UP_OFFSET)
			{
				newSection = eSectionAnvilItem2;
			}
			else if( *piTargetX >= ANVIL_SCENE_RESULT_SLOT_UP_OFFSET)
			{
				newSection = eSectionAnvilResult;
			}
		}
		break;
	case eSectionAnvilUsing:
		if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionAnvilInventory;
		}
		else if(eTapDirection == eTapStateDown)
		{
			if( *piTargetX <= ANVIL_SCENE_ITEM1_SLOT_UP_OFFSET)
			{
				newSection = eSectionAnvilItem1;
			}
			else if( *piTargetX <= ANVIL_SCENE_ITEM2_SLOT_UP_OFFSET)
			{
				newSection = eSectionAnvilName;
			}
			else if( *piTargetX >= ANVIL_SCENE_RESULT_SLOT_UP_OFFSET)
			{
				newSection = eSectionAnvilName;
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

int IUIScene_AnvilMenu::getSectionStartOffset(ESceneSection eSection)
{
	int offset = 0;
	switch( eSection )
	{
	case eSectionAnvilItem1:
		offset = MerchantMenu::PAYMENT1_SLOT;
		break;
	case eSectionAnvilItem2:
		offset = MerchantMenu::PAYMENT2_SLOT;
		break;
	case eSectionAnvilResult:
		offset = MerchantMenu::RESULT_SLOT;
		break;
	case eSectionAnvilInventory:
		offset = MerchantMenu::INV_SLOT_START;
		break;
	case eSectionAnvilUsing:
		offset = MerchantMenu::USE_ROW_SLOT_START;
		break;
	default:
		assert( false );
		break;
	}
	return offset;
}

void IUIScene_AnvilMenu::handleOtherClicked(int iPad, ESceneSection eSection, int buttonNum, bool quickKey)
{
	switch(eSection)
	{
	case eSectionAnvilName:
		handleEditNamePressed();
		break;
	};
}

bool IUIScene_AnvilMenu::IsSectionSlotList( ESceneSection eSection )
{
	switch( eSection )
	{
	case eSectionAnvilUsing:
	case eSectionAnvilInventory:
	case eSectionAnvilItem1:
	case eSectionAnvilItem2:
	case eSectionAnvilResult:
		return true;
	}
	return false;
}

void IUIScene_AnvilMenu::handleTick()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	bool canAfford = true;
	wstring m_costString = L"";

	if(m_repairMenu->cost > 0)
	{
		if(m_repairMenu->cost >= 40 && !pMinecraft->localplayers[getPad()]->abilities.instabuild)
		{
			m_costString = app.GetString(IDS_REPAIR_EXPENSIVE);
			canAfford = false;
		}
		else if(!m_repairMenu->getSlot(RepairMenu::RESULT_SLOT)->hasItem())
		{
			// Do nothing
		}
		else
		{
			LPCWSTR costString = app.GetString(IDS_REPAIR_COST);
			wchar_t temp[256];
			swprintf(temp, 256, costString, m_repairMenu->cost);
			m_costString = temp;
			if(!m_repairMenu->getSlot(RepairMenu::RESULT_SLOT)->mayPickup(dynamic_pointer_cast<Player>(m_inventory->player->shared_from_this())))
			{
				canAfford = false;
			}
		}
	}
	setCostLabel(m_costString, canAfford);

	bool crossVisible = (m_repairMenu->getSlot(RepairMenu::INPUT_SLOT)->hasItem() || m_repairMenu->getSlot(RepairMenu::ADDITIONAL_SLOT)->hasItem()) && !m_repairMenu->getSlot(RepairMenu::RESULT_SLOT)->hasItem();
	showCross(crossVisible);
}

void IUIScene_AnvilMenu::updateItemName()
{
	Slot *slot = m_repairMenu->getSlot(RepairMenu::INPUT_SLOT);
	if (slot != NULL && slot->hasItem())
	{
		if (!slot->getItem()->hasCustomHoverName() && m_itemName.compare(slot->getItem()->getHoverName())==0)
		{
			m_itemName = L"";
		}
	}

	m_repairMenu->setItemName(m_itemName);

	// Convert to byteArray
	ByteArrayOutputStream baos;
	DataOutputStream dos(&baos);
	dos.writeUTF(m_itemName);
	Minecraft::GetInstance()->localplayers[getPad()]->connection->send(shared_ptr<CustomPayloadPacket>(new CustomPayloadPacket(CustomPayloadPacket::SET_ITEM_NAME_PACKET, baos.toByteArray())));
}

void IUIScene_AnvilMenu::refreshContainer(AbstractContainerMenu *container, vector<shared_ptr<ItemInstance> > *items)
{
	slotChanged(container, RepairMenu::INPUT_SLOT, container->getSlot(0)->getItem());
}

void IUIScene_AnvilMenu::slotChanged(AbstractContainerMenu *container, int slotIndex, shared_ptr<ItemInstance> item)
{
	if (slotIndex == RepairMenu::INPUT_SLOT)
	{
		m_itemName = item == NULL ? L"" : item->getHoverName();
		setEditNameValue(m_itemName);
		setEditNameEditable(item != NULL);
		if (item != NULL)
		{
			updateItemName();
		}
	}
}

void IUIScene_AnvilMenu::setContainerData(AbstractContainerMenu *container, int id, int value)
{
}