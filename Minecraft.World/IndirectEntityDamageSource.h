#pragma once
using namespace std;

#include "EntityDamageSource.h"

class Entity;
class Player;

class IndirectEntityDamageSource : public EntityDamageSource
{
private:
	shared_ptr<Entity> owner;

public:
	//IndirectEntityDamageSource(const wstring &msgId, shared_ptr<Entity> entity, shared_ptr<Entity> owner);
	IndirectEntityDamageSource(ChatPacket::EChatPacketMessage msgId, ChatPacket::EChatPacketMessage msgWithItemId, shared_ptr<Entity> entity, shared_ptr<Entity> owner);
	virtual ~IndirectEntityDamageSource() { 	}

	virtual shared_ptr<Entity> getDirectEntity(); // 4J Stu - Brought forward from 1.2.3 to fix #46422
	virtual shared_ptr<Entity> getEntity();

	// 4J Stu - Made return a packet
	//virtual wstring getLocalizedDeathMessage(shared_ptr<Player> player);
	virtual shared_ptr<ChatPacket> getDeathMessagePacket(shared_ptr<LivingEntity> player);

	virtual DamageSource *copy();
};