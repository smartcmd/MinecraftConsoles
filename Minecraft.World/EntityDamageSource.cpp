#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.network.packet.h"

//EntityDamageSource::EntityDamageSource(const wstring &msgId, shared_ptr<Entity> entity) : DamageSource(msgId)
EntityDamageSource::EntityDamageSource(ChatPacket::EChatPacketMessage msgId, ChatPacket::EChatPacketMessage msgWithItemId, shared_ptr<Entity> entity) : DamageSource(msgId, msgWithItemId)
{
	this->entity = entity;
}

shared_ptr<Entity> EntityDamageSource::getEntity()
{
	return entity;
}

//wstring EntityDamageSource::getLocalizedDeathMessage(shared_ptr<Player> player)
//{
//	return L"death." + msgId + player->name + entity->getAName();
//	//return I18n.get("death." + msgId, player.name, entity.getAName());
//}

shared_ptr<ChatPacket> EntityDamageSource::getDeathMessagePacket(shared_ptr<LivingEntity> player)
{
	shared_ptr<ItemInstance> held = (entity != nullptr) && entity->instanceof(eTYPE_LIVINGENTITY) ? dynamic_pointer_cast<LivingEntity>(entity)->getCarriedItem() : nullptr;
	wstring additional = L"";

	if (entity->instanceof(eTYPE_SERVERPLAYER))
	{
		additional = dynamic_pointer_cast<Player>(entity)->name;
	}
	else if (entity->instanceof(eTYPE_MOB))
	{
		shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(entity);
		if (mob->hasCustomName())
		{
			additional = mob->getCustomName();
		}
	}

	if ( (held != nullptr) && held->hasCustomHoverName())
	{
		return std::make_shared<ChatPacket>(player->getNetworkName(), m_msgWithItemId, entity->GetType(), additional, held->getHoverName());
	}
	else
	{
		return std::make_shared<ChatPacket>(player->getNetworkName(), m_msgId, entity->GetType(), additional);
	}
}

bool EntityDamageSource::scalesWithDifficulty()
{
	return (entity != nullptr) && entity->instanceof(eTYPE_LIVINGENTITY) && !entity->instanceof(eTYPE_PLAYER);
}

// 4J: Copy function
DamageSource *EntityDamageSource::copy()
{
	return new EntityDamageSource(*this);
}