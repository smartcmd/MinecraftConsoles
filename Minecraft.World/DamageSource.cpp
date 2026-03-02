#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.food.h"
#include "net.minecraft.network.packet.h"

DamageSource *DamageSource::inFire = (new DamageSource(ChatPacket::e_ChatDeathInFire))->setIsFire();
DamageSource *DamageSource::onFire = (new DamageSource(ChatPacket::e_ChatDeathOnFire))->bypassArmor()->setIsFire();
DamageSource *DamageSource::lava = (new DamageSource(ChatPacket::e_ChatDeathLava))->setIsFire();
DamageSource *DamageSource::inWall = (new DamageSource(ChatPacket::e_ChatDeathInWall))->bypassArmor();
DamageSource *DamageSource::drown = (new DamageSource(ChatPacket::e_ChatDeathDrown))->bypassArmor();
DamageSource *DamageSource::starve = (new DamageSource(ChatPacket::e_ChatDeathStarve))->bypassArmor();
DamageSource *DamageSource::cactus = new DamageSource(ChatPacket::e_ChatDeathCactus);
DamageSource *DamageSource::fall = (new DamageSource(ChatPacket::e_ChatDeathFall))->bypassArmor();
DamageSource *DamageSource::outOfWorld = (new DamageSource(ChatPacket::e_ChatDeathOutOfWorld))->bypassArmor()->bypassInvul();
DamageSource *DamageSource::genericSource = (new DamageSource(ChatPacket::e_ChatDeathGeneric))->bypassArmor();
DamageSource *DamageSource::explosion = (new DamageSource(ChatPacket::e_ChatDeathExplosion))->setScalesWithDifficulty();
DamageSource *DamageSource::controlledExplosion = (new DamageSource(ChatPacket::e_ChatDeathExplosion));
DamageSource *DamageSource::magic = (new DamageSource(ChatPacket::e_ChatDeathMagic))->bypassArmor()->setMagic();
DamageSource *DamageSource::dragonbreath = (new DamageSource(ChatPacket::e_ChatDeathDragonBreath))->bypassArmor();
DamageSource *DamageSource::wither = (new DamageSource(ChatPacket::e_ChatDeathWither))->bypassArmor();
DamageSource *DamageSource::anvil = (new DamageSource(ChatPacket::e_ChatDeathAnvil));
DamageSource *DamageSource::fallingBlock = (new DamageSource(ChatPacket::e_ChatDeathFallingBlock));

DamageSource *DamageSource::mobAttack(shared_ptr<Mob> mob)
{
	return new EntityDamageSource(ChatPacket::e_ChatDeathMob, mob);
}

DamageSource *DamageSource::playerAttack(shared_ptr<Player> player)
{
	return new EntityDamageSource(ChatPacket::e_ChatDeathPlayer, player);
}

DamageSource *DamageSource::arrow(shared_ptr<Arrow> arrow, shared_ptr<Entity> owner)
{
	return (new IndirectEntityDamageSource(ChatPacket::e_ChatDeathArrow, arrow, owner))->setProjectile();
}

DamageSource *DamageSource::fireball(shared_ptr<Fireball> fireball, shared_ptr<Entity> owner)
{
	if (owner == NULL)
	{
		return (new IndirectEntityDamageSource(ChatPacket::e_ChatDeathOnFire, fireball, fireball))->setIsFire()->setProjectile();
	}
	return (new IndirectEntityDamageSource(ChatPacket::e_ChatDeathFireball, fireball, owner))->setIsFire()->setProjectile();
}

DamageSource *DamageSource::thrown(shared_ptr<Entity> entity, shared_ptr<Entity> owner)
{
	return (new IndirectEntityDamageSource(ChatPacket::e_ChatDeathThrown, entity, owner))->setProjectile();
}

DamageSource *DamageSource::indirectMagic(shared_ptr<Entity> entity, shared_ptr<Entity> owner)
{
	return (new IndirectEntityDamageSource(ChatPacket::e_ChatDeathIndirectMagic, entity, owner) )->bypassArmor()->setMagic();;
}

DamageSource *DamageSource::thorns(shared_ptr<Entity> source)
{
	return (new EntityDamageSource(ChatPacket::e_ChatDeathThorns, source))->setMagic();
}

bool DamageSource::isProjectile()
{
	return _isProjectile;
}

DamageSource *DamageSource::setProjectile()
{
	this->_isProjectile = true;
	return this;
}

bool DamageSource::isBypassArmor()
{
	return _bypassArmor;
}

float DamageSource::getFoodExhaustion()
{
	return exhaustion;
}

bool DamageSource::isBypassInvul()
{
	return _bypassInvul;
}


//DamageSource::DamageSource(const wstring &msgId)
DamageSource::DamageSource(ChatPacket::EChatPacketMessage msgId)
{
	// 4J added initialisors
	_bypassArmor = false;
	_bypassInvul = false;
	// food exhastion caused by being damaged by this source
	exhaustion = FoodConstants::EXHAUSTION_ATTACK;
	isFireSource = false;
	_isProjectile = false;
	_isMagic = false;

	//this->msgId = msgId;
	m_msgId = msgId;
}

shared_ptr<Entity> DamageSource::getDirectEntity()
{
	return getEntity();
}

shared_ptr<Entity> DamageSource::getEntity()
{
	return shared_ptr<Entity>();
}

DamageSource *DamageSource::bypassArmor()
{
	_bypassArmor = true;
	// these kinds of damages don't cause the player to grow more hungry
	exhaustion = 0;
	return this;
}

DamageSource *DamageSource::bypassInvul()
{
	_bypassInvul = true;
	return this;
}

DamageSource *DamageSource::setIsFire()
{
	isFireSource = true;
	return this;
}

DamageSource *DamageSource::setScalesWithDifficulty()
{
	_scalesWithDifficulty = true;
	return this;
}

bool DamageSource::scalesWithDifficulty()
{
	return _scalesWithDifficulty;
}

bool DamageSource::isMagic()
{
	return _isMagic;
}

DamageSource *DamageSource::setMagic()
{
	_isMagic = true;
	return this;
}

//wstring DamageSource::getLocalizedDeathMessage(shared_ptr<Player> player)
//{
//	return L"death." + msgId + player->name;
//	//return I18n.get(L"death." + msgId, player.name);
//}

shared_ptr<ChatPacket> DamageSource::getDeathMessagePacket(shared_ptr<Player> player)
{
	return shared_ptr<ChatPacket>( new ChatPacket(player->name, m_msgId ) );
}

bool DamageSource::isFire()
{
	return isFireSource;
}

ChatPacket::EChatPacketMessage DamageSource::getMsgId()
{
	return m_msgId;
}