#include "stdafx.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "../../StringTable.h"
#include "ConsoleGameRules.h"
#include "LevelRuleset.h"

LevelRuleset::LevelRuleset()
{
	m_stringTable = NULL;
}

LevelRuleset::~LevelRuleset()
{
	for (auto it = m_areas.begin(); it != m_areas.end(); ++it)
	{
		delete *it;
	}
}

void LevelRuleset::getChildren(vector<GameRuleDefinition *> *children)
{
	CompoundGameRuleDefinition::getChildren(children);
	for (const auto& area : m_areas)
		children->push_back(area);
}

GameRuleDefinition *LevelRuleset::addChild(ConsoleGameRules::EGameRuleType ruleType)
{
	GameRuleDefinition *rule = NULL;
	if(ruleType == ConsoleGameRules::eGameRuleType_NamedArea)
	{
		rule = new NamedAreaRuleDefinition();
		m_areas.push_back((NamedAreaRuleDefinition *)rule);
	}
	else
	{
		rule = CompoundGameRuleDefinition::addChild(ruleType);
	}
	return rule;
}

void LevelRuleset::loadStringTable(StringTable *table)
{
	m_stringTable = table;
}

LPCWSTR LevelRuleset::getString(const wstring &key)
{
	if(m_stringTable == NULL)
	{
		return L"";
	}
	else
	{
		return m_stringTable->getString(key);
	}
}

AABB *LevelRuleset::getNamedArea(const wstring &areaName)
{
	AABB *area = nullptr;
	for(auto& it : m_areas)
	{
		if( it->getName().compare(areaName) == 0 )
		{
			area = it->getArea();
			break;
		}
	}
	return area;
}
