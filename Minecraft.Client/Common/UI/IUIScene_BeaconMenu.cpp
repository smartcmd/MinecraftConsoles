#include "stdafx.h"
#include "../Minecraft.World/CustomPayloadPacket.h"
#include "../Minecraft.World/net.minecraft.world.inventory.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "../Minecraft.World/net.minecraft.world.effect.h"
#include "../Minecraft.World/HtmlString.h"
#include "IUIScene_BeaconMenu.h"
#include "Minecraft.h"
#include "MultiPlayerLocalPlayer.h"
#include "ClientConnection.h"

IUIScene_BeaconMenu::IUIScene_BeaconMenu()
{
	m_beacon = nullptr;
	m_initPowerButtons = true;
}

IUIScene_AbstractContainerMenu::ESceneSection IUIScene_BeaconMenu::GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY )
{
	ESceneSection newSection = eSection;

	int xOffset = 0;

	// Find the new section if there is one
	switch( eSection )
	{
	case eSectionBeaconInventory:
		if(eTapDirection == eTapStateDown) newSection = eSectionBeaconUsing;
		else if(eTapDirection == eTapStateUp)
		{
			if( *piTargetX < 4 )
			{
				newSection = eSectionBeaconPrimaryTierThree;
			}
			else if ( *piTargetX < 7)
			{
				newSection = eSectionBeaconItem;
			}
			else
			{
				newSection = eSectionBeaconConfirm;
			}
		}
		break;
	case eSectionBeaconUsing:
		if(eTapDirection == eTapStateDown)
		{
			if( *piTargetX < 2)
			{
				newSection = eSectionBeaconPrimaryTierOneOne;
			}
			else if( *piTargetX < 5)
			{
				newSection = eSectionBeaconPrimaryTierOneTwo;
			}
			else if( *piTargetX > 8 && GetPowerButtonId(eSectionBeaconSecondaryTwo) > 0)
			{
				newSection = eSectionBeaconSecondaryTwo;
			}
			else
			{
				newSection = eSectionBeaconSecondaryOne;
			}
		}
		else if(eTapDirection == eTapStateUp) newSection = eSectionBeaconInventory;
		break;
	case eSectionBeaconItem:
		if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionBeaconInventory;
			xOffset = -5;
		}
		else if(eTapDirection == eTapStateUp) newSection = eSectionBeaconSecondaryOne;
		else if(eTapDirection == eTapStateLeft) newSection = eSectionBeaconConfirm;
		else if(eTapDirection == eTapStateRight) newSection = eSectionBeaconConfirm;
		break;
	case eSectionBeaconPrimaryTierOneOne:
		if(eTapDirection == eTapStateDown) newSection = eSectionBeaconPrimaryTierTwoOne;
		else if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionBeaconUsing;
			xOffset = -1;
		}
		else if(eTapDirection == eTapStateLeft) newSection = eSectionBeaconPrimaryTierOneTwo;
		else if(eTapDirection == eTapStateRight) newSection = eSectionBeaconPrimaryTierOneTwo;
		break;
	case eSectionBeaconPrimaryTierOneTwo:
		if(eTapDirection == eTapStateDown) newSection = eSectionBeaconPrimaryTierTwoTwo;
		else if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionBeaconUsing;
			xOffset = -3;
		}
		else if(eTapDirection == eTapStateLeft) newSection = eSectionBeaconPrimaryTierOneOne;
		else if(eTapDirection == eTapStateRight) newSection = eSectionBeaconPrimaryTierOneOne;
		break;
	case eSectionBeaconPrimaryTierTwoOne:
		if(eTapDirection == eTapStateDown) newSection = eSectionBeaconPrimaryTierThree;
		else if(eTapDirection == eTapStateUp) newSection = eSectionBeaconPrimaryTierOneOne;
		else if(eTapDirection == eTapStateLeft)
		{
			if(GetPowerButtonId(eSectionBeaconSecondaryTwo) > 0)
			{
				newSection = eSectionBeaconSecondaryTwo;
			}
			else
			{
				newSection = eSectionBeaconSecondaryOne;
			}
		}
		else if(eTapDirection == eTapStateRight) newSection = eSectionBeaconPrimaryTierTwoTwo;
		break;
	case eSectionBeaconPrimaryTierTwoTwo:
		if(eTapDirection == eTapStateDown) newSection = eSectionBeaconPrimaryTierThree;
		else if(eTapDirection == eTapStateUp) newSection = eSectionBeaconPrimaryTierOneTwo;
		else if(eTapDirection == eTapStateLeft) newSection = eSectionBeaconPrimaryTierTwoOne;
		else if(eTapDirection == eTapStateRight) newSection = eSectionBeaconSecondaryOne;
		break;
	case eSectionBeaconPrimaryTierThree:
		if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionBeaconInventory;
			xOffset = -3;
		}
		else if(eTapDirection == eTapStateUp) newSection = eSectionBeaconPrimaryTierTwoOne;
		break;
	case eSectionBeaconSecondaryOne:
		if(eTapDirection == eTapStateDown) newSection = eSectionBeaconItem;
		else if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionBeaconUsing;
			xOffset = -7;
		}
		else if(eTapDirection == eTapStateLeft) newSection = eSectionBeaconPrimaryTierTwoTwo;
		else if(eTapDirection == eTapStateRight)
		{
			if(GetPowerButtonId(eSectionBeaconSecondaryTwo) > 0)
			{
				newSection = eSectionBeaconSecondaryTwo;
			}
			else
			{
				newSection = eSectionBeaconPrimaryTierTwoOne;
			}
		}
		break;
	case eSectionBeaconSecondaryTwo:
		if(eTapDirection == eTapStateDown) newSection = eSectionBeaconItem;
		else if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionBeaconUsing;
			xOffset = -8;
		}
		else if(eTapDirection == eTapStateLeft) newSection = eSectionBeaconSecondaryOne;
		else if(eTapDirection == eTapStateRight) newSection = eSectionBeaconPrimaryTierTwoOne;
		break;
	case eSectionBeaconConfirm:
		if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionBeaconInventory;
			xOffset = -8;
		}
		else if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionBeaconSecondaryOne;
		}
		else if(eTapDirection == eTapStateLeft) newSection = eSectionBeaconItem;
		else if(eTapDirection == eTapStateRight) newSection = eSectionBeaconItem;
		break;
	default:
		assert(false);
		break;
	}

	updateSlotPosition(eSection, newSection, eTapDirection, piTargetX, piTargetY, xOffset);

	return newSection;
}

int IUIScene_BeaconMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection eSection)
{
	int offset = 0;
	switch( eSection )
	{
	case eSectionBeaconItem:
		offset = BeaconMenu::PAYMENT_SLOT;
		break;
	case eSectionBeaconInventory:
		offset = BeaconMenu::INV_SLOT_START;
		break;
	case eSectionBeaconUsing:
		offset = BeaconMenu::USE_ROW_SLOT_START;
		break;
	default:
		assert( false );
		break;
	}
	return offset;
}

bool IUIScene_BeaconMenu::IsSectionSlotList( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionBeaconItem:
		case eSectionBeaconInventory:
		case eSectionBeaconUsing:
			return true;
	}
	return false;
}

void IUIScene_BeaconMenu::handleOtherClicked(int iPad, ESceneSection eSection, int buttonNum, bool quickKey)
{
	switch(eSection)
	{
	case eSectionBeaconConfirm:
		{
			if( (m_beacon->getItem(0) == NULL) || (m_beacon->getPrimaryPower() <= 0) ) return;
			ByteArrayOutputStream baos;
			DataOutputStream dos(&baos);
			dos.writeInt(m_beacon->getPrimaryPower());
			dos.writeInt(m_beacon->getSecondaryPower());

			Minecraft::GetInstance()->localplayers[getPad()]->connection->send(shared_ptr<CustomPayloadPacket>(new CustomPayloadPacket(CustomPayloadPacket::SET_BEACON_PACKET, baos.toByteArray())));

			if (m_beacon->getPrimaryPower() > 0)
			{
				int effectId = m_beacon->getPrimaryPower();

				bool active = true;
				bool selected = false;

				int tier = 3;
				if (tier >= m_beacon->getLevels())
				{
					active = false;
				}
				else if (effectId == m_beacon->getSecondaryPower())
				{
					selected = true;
				}

				AddPowerButton(GetId(tier, m_beacon->getPrimaryPower()), MobEffect::effects[m_beacon->getPrimaryPower()]->getIcon(), tier, 1, active, selected);
			}
		}
		break;
	case eSectionBeaconPrimaryTierOneOne:
	case eSectionBeaconPrimaryTierOneTwo:
	case eSectionBeaconPrimaryTierTwoOne:
	case eSectionBeaconPrimaryTierTwoTwo:
	case eSectionBeaconPrimaryTierThree:
	case eSectionBeaconSecondaryOne:
	case eSectionBeaconSecondaryTwo:
		if(IsPowerButtonSelected(eSection))
		{
			return;
		}

		int id = GetPowerButtonId(eSection);
		int effectId = (id & 0xff);
		int tier = (id >> 8);

		if (tier < 3)
		{
			m_beacon->setPrimaryPower(effectId);
		}
		else
		{
			m_beacon->setSecondaryPower(effectId);
		}
		SetPowerButtonSelected(eSection);
		break;
	};
}

void IUIScene_BeaconMenu::handleTick()
{
	if (m_initPowerButtons && m_beacon->getLevels() >= 0)
	{
		m_initPowerButtons = false;
		for (int tier = 0; tier <= 2; tier++)
		{
			int count = BeaconTileEntity::BEACON_EFFECTS_EFFECTS;//BEACON_EFFECTS[tier].length;
			int totalWidth = count * 22 + (count - 1) * 2;

			for (int c = 0; c < count; c++)
			{
				if(BeaconTileEntity::BEACON_EFFECTS[tier][c] == NULL) continue;

				int effectId = BeaconTileEntity::BEACON_EFFECTS[tier][c]->id;
				int icon = BeaconTileEntity::BEACON_EFFECTS[tier][c]->getIcon();

				bool active = true;
				bool selected = false;

				if (tier >= m_beacon->getLevels())
				{
					active = false;
				}
				else if (effectId == m_beacon->getPrimaryPower())
				{
					selected = true;
				}

				AddPowerButton(GetId(tier, effectId), icon, tier, c, active, selected);
			}
		}

		{
			int tier = 3;

			int count = BeaconTileEntity::BEACON_EFFECTS_EFFECTS + 1;//BEACON_EFFECTS[tier].length + 1;
			int totalWidth = count * 22 + (count - 1) * 2;

			for (int c = 0; c < count - 1; c++)
			{
				if(BeaconTileEntity::BEACON_EFFECTS[tier][c] == NULL) continue;

				int effectId = BeaconTileEntity::BEACON_EFFECTS[tier][c]->id;
				int icon = BeaconTileEntity::BEACON_EFFECTS[tier][c]->getIcon();

				bool active = true;
				bool selected = false;

				if (tier >= m_beacon->getLevels())
				{
					active = false;
				}
				else if (effectId == m_beacon->getSecondaryPower())
				{
					selected = true;
				}

				AddPowerButton(GetId(tier, effectId), icon, tier, c, active, selected);
			}
			if (m_beacon->getPrimaryPower() > 0)
			{
				int effectId = m_beacon->getPrimaryPower();

				bool active = true;
				bool selected = false;

				if (tier >= m_beacon->getLevels())
				{
					active = false;
				}
				else if (effectId == m_beacon->getSecondaryPower())
				{
					selected = true;
				}

				AddPowerButton(GetId(tier, m_beacon->getPrimaryPower()), MobEffect::effects[m_beacon->getPrimaryPower()]->getIcon(), tier, 1, active, selected);
			}
		}
	}

	SetConfirmButtonEnabled( (m_beacon->getItem(0) != NULL) && (m_beacon->getPrimaryPower() > 0) );
}

int IUIScene_BeaconMenu::GetId(int tier, int effectId)
{
	return (tier << 8) | effectId;
}

vector<HtmlString> *IUIScene_BeaconMenu::GetSectionHoverText(ESceneSection eSection)
{
	vector<HtmlString> *desc = NULL;
	switch(eSection)
	{
	case eSectionBeaconSecondaryTwo:
		if(GetPowerButtonId(eSectionBeaconSecondaryTwo) == 0)
		{
			// This isn't visible
			break;
		}
		// Fall through otherwise
	case eSectionBeaconPrimaryTierOneOne:
	case eSectionBeaconPrimaryTierOneTwo:
	case eSectionBeaconPrimaryTierTwoOne:
	case eSectionBeaconPrimaryTierTwoTwo:
	case eSectionBeaconPrimaryTierThree:
	case eSectionBeaconSecondaryOne:
		{
			int id = GetPowerButtonId(eSection);
			int effectId = (id & 0xff);

			desc = new vector<HtmlString>();

			if (effectId < MobEffect::e_MobEffectIcon_COUNT && MobEffect::effects[effectId]) {
				HtmlString string( app.GetString(MobEffect::effects[effectId]->getDescriptionId()), eHTMLColor_White );
				desc->push_back( string );
			}
		}
		break;
	}
	return desc;
}

bool IUIScene_BeaconMenu::IsVisible( ESceneSection eSection )
{
	switch( eSection )
	{
	case eSectionBeaconSecondaryTwo:
		if(GetPowerButtonId(eSectionBeaconSecondaryTwo) == 0)
		{
			// This isn't visible
			return false;
		}
	}
	return true;
}