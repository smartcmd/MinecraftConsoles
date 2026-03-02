#pragma once
using namespace std;

class Mob;
class Entity;
class Arrow;
class Fireball;
class Player;

#include "ChatPacket.h"

class DamageSource
{
public:
	static DamageSource *inFire;
	static DamageSource *onFire;
	static DamageSource *lava;
	static DamageSource *inWall;
	static DamageSource *drown;
	static DamageSource *starve;
	static DamageSource *cactus;
	static DamageSource *fall;
	static DamageSource *outOfWorld;
	static DamageSource *genericSource;
	static DamageSource *explosion;
	static DamageSource *controlledExplosion;
	static DamageSource *magic;
	static DamageSource *dragonbreath;
	static DamageSource *wither;
	static DamageSource *anvil;
	static DamageSource *fallingBlock;

	static DamageSource *mobAttack(shared_ptr<Mob> mob);
	static DamageSource *playerAttack(shared_ptr<Player> player);
	static DamageSource *arrow(shared_ptr<Arrow> arrow, shared_ptr<Entity> owner);
	static DamageSource *fireball(shared_ptr<Fireball> fireball, shared_ptr<Entity> owner);
	static DamageSource *thrown(shared_ptr<Entity> entity, shared_ptr<Entity> owner);
	static DamageSource *indirectMagic(shared_ptr<Entity> entity, shared_ptr<Entity> owner);
	static DamageSource *thorns(shared_ptr<Entity> source);

private:
	bool _bypassArmor;
	bool _bypassInvul;
	// food exhastion caused by being damaged by this source
	float exhaustion;
	bool isFireSource;
	bool _isProjectile;
	bool _scalesWithDifficulty;
	bool _isMagic;

public:
	bool isProjectile();
	DamageSource *setProjectile();

	bool isBypassArmor();
	float getFoodExhaustion();
	bool isBypassInvul();

	//wstring msgId;
	ChatPacket::EChatPacketMessage m_msgId; // 4J Made int so we can localise

protected:
	//DamageSource(const wstring &msgId);
	DamageSource(ChatPacket::EChatPacketMessage msgId);

public:
	virtual ~DamageSource() {}

	virtual shared_ptr<Entity> getDirectEntity();
	virtual shared_ptr<Entity> getEntity();

protected:
	DamageSource *bypassArmor();
	DamageSource *bypassInvul();
	DamageSource *setIsFire();
	DamageSource *setScalesWithDifficulty();

public:
	virtual bool scalesWithDifficulty();

	bool isMagic();
	DamageSource *setMagic();

	// 4J Stu - Made return a packet
	//virtual wstring getLocalizedDeathMessage(shared_ptr<Player> player);
	virtual shared_ptr<ChatPacket> getDeathMessagePacket(shared_ptr<Player> player);

	bool isFire();
	ChatPacket::EChatPacketMessage getMsgId(); // 4J Stu - Used to return String
};