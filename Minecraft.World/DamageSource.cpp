#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.food.h"
#include "net.minecraft.network.packet.h"

DamageSource *DamageSource::inFire = (new DamageSource(ChatPacket::e_ChatDeathInFire, ChatPacket::e_ChatDeathInFirePlayer))->setIsFire();
DamageSource *DamageSource::onFire = (new DamageSource(ChatPacket::e_ChatDeathOnFire, ChatPacket::e_ChatDeathOnFirePlayer))->bypassArmor()->setIsFire();
DamageSource *DamageSource::lava = (new DamageSource(ChatPacket::e_ChatDeathLava, ChatPacket::e_ChatDeathLavaPlayer))->setIsFire();
DamageSource *DamageSource::inWall = (new DamageSource(ChatPacket::e_ChatDeathInWall))->bypassArmor();
DamageSource *DamageSource::drown = (new DamageSource(ChatPacket::e_ChatDeathDrown, ChatPacket::e_ChatDeathDrownPlayer))->bypassArmor();
DamageSource *DamageSource::starve = (new DamageSource(ChatPacket::e_ChatDeathStarve))->bypassArmor();
DamageSource *DamageSource::cactus = new DamageSource(ChatPacket::e_ChatDeathCactus, ChatPacket::e_ChatDeathCactusPlayer);
DamageSource *DamageSource::fall = (new DamageSource(ChatPacket::e_ChatDeathFall))->bypassArmor();
DamageSource *DamageSource::outOfWorld = (new DamageSource(ChatPacket::e_ChatDeathOutOfWorld))->bypassArmor()->bypassInvul();
DamageSource *DamageSource::genericSource = (new DamageSource(ChatPacket::e_ChatDeathGeneric))->bypassArmor();
DamageSource *DamageSource::magic = (new DamageSource(ChatPacket::e_ChatDeathMagic))->bypassArmor()->setMagic();
DamageSource *DamageSource::dragonbreath = (new DamageSource(ChatPacket::e_ChatDeathDragonBreath))->bypassArmor();
DamageSource *DamageSource::wither = (new DamageSource(ChatPacket::e_ChatDeathWither))->bypassArmor();
DamageSource *DamageSource::anvil = (new DamageSource(ChatPacket::e_ChatDeathAnvil));
DamageSource *DamageSource::fallingBlock = (new DamageSource(ChatPacket::e_ChatDeathFallingBlock));

DamageSource *DamageSource::mobAttack(shared_ptr<LivingEntity> mob)
{
	return new EntityDamageSource(ChatPacket::e_ChatDeathMob, ChatPacket::e_ChatDeathMob, mob);
}

DamageSource *DamageSource::playerAttack(shared_ptr<Player> player)
{
	return new EntityDamageSource(ChatPacket::e_ChatDeathPlayer, ChatPacket::e_ChatDeathPlayerItem, player);
}

DamageSource *DamageSource::arrow(shared_ptr<Arrow> arrow, shared_ptr<Entity> owner)
{
	return (new IndirectEntityDamageSource(ChatPacket::e_ChatDeathArrow, ChatPacket::e_ChatDeathArrowItem, arrow, owner))->setProjectile();
}

DamageSource *DamageSource::fireball(shared_ptr<Fireball> fireball, shared_ptr<Entity> owner)
{
	if (owner == nullptr)
	{
		return (new IndirectEntityDamageSource(ChatPacket::e_ChatDeathOnFire, ChatPacket::e_ChatDeathOnFire, fireball, fireball))->setIsFire()->setProjectile();
	}
	return (new IndirectEntityDamageSource(ChatPacket::e_ChatDeathFireball, ChatPacket::e_ChatDeathArrowItem, fireball, owner))->setIsFire()->setProjectile();
}

DamageSource *DamageSource::thrown(shared_ptr<Entity> entity, shared_ptr<Entity> owner)
{
	return (new IndirectEntityDamageSource(ChatPacket::e_ChatDeathThrown, ChatPacket::e_ChatDeathThrownItem, entity, owner))->setProjectile();
}

DamageSource *DamageSource::indirectMagic(shared_ptr<Entity> entity, shared_ptr<Entity> owner)
{
	return (new IndirectEntityDamageSource(ChatPacket::e_ChatDeathIndirectMagic, ChatPacket::e_ChatDeathIndirectMagicItem, entity, owner) )->bypassArmor()->setMagic();;
}

DamageSource *DamageSource::thorns(shared_ptr<Entity> source)
{
	return (new EntityDamageSource(ChatPacket::e_ChatDeathThorns, ChatPacket::e_ChatDeathThorns, source))->setMagic();
}

DamageSource *DamageSource::explosion(Explosion *explosion)
{
	if ( (explosion != nullptr) && (explosion->getSourceMob() != nullptr) )
	{
		return (new EntityDamageSource(ChatPacket::e_ChatDeathExplosionPlayer, ChatPacket::e_ChatDeathExplosionPlayer, explosion->getSourceMob()))->setScalesWithDifficulty()->setExplosion();
	}
	else
	{
		return (new DamageSource(ChatPacket::e_ChatDeathExplosion))->setScalesWithDifficulty()->setExplosion();
	}
}

bool DamageSource::isProjectile()
{
	return _isProjectile;
}

DamageSource *DamageSource::setProjectile()
{
	_isProjectile = true;
	return this;
}

bool DamageSource::isExplosion()
{
	return _isExplosion;
}

DamageSource *DamageSource::setExplosion()
{
	_isExplosion = true;
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
DamageSource::DamageSource(ChatPacket::EChatPacketMessage msgId, ChatPacket::EChatPacketMessage msgWithItemId)
{
	// 4J added initialisors
	_bypassArmor = false;
	_bypassInvul = false;
	// food exhastion caused by being damaged by this source
	exhaustion = FoodConstants::EXHAUSTION_ATTACK;
	isFireSource = false;
	_isProjectile = false;
	_isMagic = false;
	_isExplosion = false;

	//this->msgId = msgId;
	m_msgId = msgId;
	m_msgWithItemId = msgWithItemId;
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

shared_ptr<ChatPacket> DamageSource::getDeathMessagePacket(shared_ptr<LivingEntity> player)
{
	shared_ptr<LivingEntity> source = player->getKillCredit();
	if(source != nullptr)
	{
		return std::make_shared<ChatPacket>(player->getNetworkName(), m_msgWithItemId != ChatPacket::e_ChatCustom ? m_msgWithItemId : m_msgId, source->GetType(), source->getNetworkName());
	}
	else
	{
		return std::make_shared<ChatPacket>(player->getNetworkName(), m_msgId);
	}
}

bool DamageSource::isFire()
{
	return isFireSource;
}

ChatPacket::EChatPacketMessage DamageSource::getMsgId()
{
	return m_msgId;
}

// 4J: Very limited check for equality (used to detect fall damage, etc)
bool DamageSource::equals(DamageSource *source)
{
	return m_msgId == source->m_msgId && m_msgWithItemId == source->m_msgWithItemId;
}

// 4J: Copy function
DamageSource *DamageSource::copy()
{
	return new DamageSource(*this);
}