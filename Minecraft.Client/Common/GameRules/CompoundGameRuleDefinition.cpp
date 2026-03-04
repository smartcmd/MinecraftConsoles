#include "stdafx.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "CompoundGameRuleDefinition.h"
#include "ConsoleGameRules.h"

CompoundGameRuleDefinition::CompoundGameRuleDefinition()
{
	m_lastRuleStatusChanged = NULL;
}

CompoundGameRuleDefinition::~CompoundGameRuleDefinition()
{
	for(AUTO_VAR(it, m_children.begin()); it != m_children.end(); ++it)
	{
		delete (*it);
	}
}

void CompoundGameRuleDefinition::getChildren(vector<GameRuleDefinition *> *children)
{
	GameRuleDefinition::getChildren(children);
	for (AUTO_VAR(it, m_children.begin()); it != m_children.end(); it++)
		children->push_back(*it);
}

GameRuleDefinition *CompoundGameRuleDefinition::addChild(ConsoleGameRules::EGameRuleType ruleType)
{
	GameRuleDefinition *rule = NULL;
	if(ruleType == ConsoleGameRules::eGameRuleType_CompleteAllRule)
	{
		rule = new CompleteAllRuleDefinition();
	}
	else if(ruleType == ConsoleGameRules::eGameRuleType_CollectItemRule)
	{
		rule = new CollectItemRuleDefinition();
	}
	else if(ruleType == ConsoleGameRules::eGameRuleType_UseTileRule)
	{
		rule = new UseTileRuleDefinition();
	}
	else if(ruleType == ConsoleGameRules::eGameRuleType_UpdatePlayerRule)
	{		
		rule = new UpdatePlayerRuleDefinition();
	}
	else
	{
#ifndef _CONTENT_PACKAGE
		wprintf(L"CompoundGameRuleDefinition: Attempted to add invalid child rule - %d\n", ruleType );
#endif
	}
	if(rule != NULL) m_children.push_back(rule);
	return rule;
}

void CompoundGameRuleDefinition::populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule *rule)
{
	GameRule *newRule = NULL;
	int i = 0;
	for(AUTO_VAR(it, m_children.begin()); it != m_children.end(); ++it)
	{
		newRule = new GameRule(*it, rule->getConnection() );
		(*it)->populateGameRule(type,newRule);

		GameRule::ValueType value;
		value.gr = newRule;
		value.isPointer = true;

		// Somehow add the newRule to the current rule
		rule->setParameter(L"rule" + _toString<int>(i),value);
		++i;
	}
	GameRuleDefinition::populateGameRule(type, rule);
}

bool CompoundGameRuleDefinition::onUseTile(GameRule *rule, int tileId, int x, int y, int z)
{
	bool statusChanged = false;
	for(AUTO_VAR(it, rule->m_parameters.begin()); it != rule->m_parameters.end(); ++it)
	{
		if(it->second.isPointer)
		{
			bool changed = it->second.gr->getGameRuleDefinition()->onUseTile(it->second.gr,tileId,x,y,z);
			if(!statusChanged && changed)
			{
				m_lastRuleStatusChanged = it->second.gr->getGameRuleDefinition();
				statusChanged = true;
			}
		}
	}
	return statusChanged;
}

bool CompoundGameRuleDefinition::onCollectItem(GameRule *rule, shared_ptr<ItemInstance> item)
{
	bool statusChanged = false;
	for(AUTO_VAR(it, rule->m_parameters.begin()); it != rule->m_parameters.end(); ++it)
	{
		if(it->second.isPointer)
		{
			bool changed = it->second.gr->getGameRuleDefinition()->onCollectItem(it->second.gr,item);
			if(!statusChanged && changed)
			{				
				m_lastRuleStatusChanged = it->second.gr->getGameRuleDefinition();
				statusChanged = true;
			}
		}
	}
	return statusChanged;
}

void CompoundGameRuleDefinition::postProcessPlayer(shared_ptr<Player> player)
{
	for(AUTO_VAR(it, m_children.begin()); it != m_children.end(); ++it)
	{
		(*it)->postProcessPlayer(player);
	}
}