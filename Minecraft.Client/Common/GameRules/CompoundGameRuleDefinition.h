#pragma once

#include "GameRuleDefinition.h"

class CompoundGameRuleDefinition : public GameRuleDefinition
{
protected:
	vector<GameRuleDefinition *> m_children;
protected:
	GameRuleDefinition *m_lastRuleStatusChanged;
public:
	CompoundGameRuleDefinition();
	virtual ~CompoundGameRuleDefinition();

	virtual void getChildren(vector<GameRuleDefinition *> *children);
	virtual GameRuleDefinition *addChild(ConsoleGameRules::EGameRuleType ruleType);

	virtual void populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule *rule);

	virtual bool onUseTile(GameRule *rule, int tileId, int x, int y, int z);
	virtual bool onCollectItem(GameRule *rule, shared_ptr<ItemInstance> item);
	virtual void postProcessPlayer(shared_ptr<Player> player);
};