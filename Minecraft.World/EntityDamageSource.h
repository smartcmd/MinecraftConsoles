#pragma once
using namespace std;

#include "DamageSource.h"

class Entity;
class Player;

class EntityDamageSource : public DamageSource
{
protected:
	shared_ptr<Entity> entity;

public:
	//EntityDamageSource(const wstring &msgId, shared_ptr<Entity> entity);
	EntityDamageSource(ChatPacket::EChatPacketMessage msgId, ChatPacket::EChatPacketMessage msgWithItemId, shared_ptr<Entity> entity);
	virtual ~EntityDamageSource() { }

	shared_ptr<Entity> getEntity();

	// 4J Stu - Made return a packet
	//virtual wstring getLocalizedDeathMessage(shared_ptr<Player> player);
	virtual shared_ptr<ChatPacket> getDeathMessagePacket(shared_ptr<LivingEntity> player);

	virtual bool scalesWithDifficulty();

	virtual DamageSource *copy();
};