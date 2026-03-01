#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.network.packet.h"

//IndirectEntityDamageSource::IndirectEntityDamageSource(const wstring &msgId, shared_ptr<Entity> entity, shared_ptr<Entity> owner) : EntityDamageSource(msgId, entity)
IndirectEntityDamageSource::IndirectEntityDamageSource(ChatPacket::EChatPacketMessage msgId, ChatPacket::EChatPacketMessage msgWithItemId, shared_ptr<Entity> entity, shared_ptr<Entity> owner) : EntityDamageSource(msgId, msgWithItemId, entity)
{
	this->owner = owner;
}

// 4J Stu - Brought forward from 1.2.3 to fix #46422
shared_ptr<Entity> IndirectEntityDamageSource::getDirectEntity()
{
	return entity;
}

shared_ptr<Entity> IndirectEntityDamageSource::getEntity()
{
	return owner;
}

//wstring IndirectEntityDamageSource::getLocalizedDeathMessage(shared_ptr<Player> player)
//{
//	return L"death." + msgId + player->name + owner->getAName();
//	//return I18n.get("death." + msgId, player.name, owner.getAName());
//}

shared_ptr<ChatPacket> IndirectEntityDamageSource::getDeathMessagePacket(shared_ptr<LivingEntity> player)
{
	shared_ptr<ItemInstance> held = entity->instanceof(eTYPE_LIVINGENTITY) ? dynamic_pointer_cast<LivingEntity>(entity)->getCarriedItem() : nullptr;
	wstring additional = L"";
	int type;
	if(owner != NULL)
	{
		type = owner->GetType();
		if(type == eTYPE_SERVERPLAYER)
		{
			shared_ptr<Player> sourcePlayer = dynamic_pointer_cast<Player>(owner);
			if(sourcePlayer != NULL) additional = sourcePlayer->name;
		}
	}
	else
	{
		type = entity->GetType();
	}
	if(held != NULL && held->hasCustomHoverName() )
	{
		return shared_ptr<ChatPacket>( new ChatPacket(player->getNetworkName(), m_msgWithItemId, type, additional, held->getHoverName() ) );
	}
	else
	{
		return shared_ptr<ChatPacket>( new ChatPacket(player->getNetworkName(), m_msgId, type, additional ) );
	}
}

// 4J: Copy function
DamageSource *IndirectEntityDamageSource::copy()
{
	return new IndirectEntityDamageSource(*this);
}