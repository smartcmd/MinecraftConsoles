#include "stdafx.h"
#include "Enemy.h"

EntitySelector *Enemy::ENEMY_SELECTOR = new Enemy::EnemyEntitySelector();

bool Enemy::EnemyEntitySelector::matches(shared_ptr<Entity> entity) const
{
	return (entity != NULL) && entity->instanceof(eTYPE_ENEMY);
}