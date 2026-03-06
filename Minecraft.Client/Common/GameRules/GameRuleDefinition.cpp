#include "stdafx.h"
#include "..\..\WstringLookup.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "ConsoleGameRules.h"

GameRuleDefinition::GameRuleDefinition()
{
	m_descriptionId = L"";
	m_promptId = L"";
	m_4JDataValue = 0;
}

void GameRuleDefinition::write(DataOutputStream *dos)
{
	// Write EGameRuleType.
	ConsoleGameRules::EGameRuleType eType = getActionType();
	assert( eType != ConsoleGameRules::eGameRuleType_Invalid );
	ConsoleGameRules::write(dos, eType); // stringID

	writeAttributes(dos, 0);

	// 4J-JEV: Get children.
	vector<GameRuleDefinition *> *children = new vector<GameRuleDefinition *>();
	getChildren( children );

	// Write children.
	dos->writeInt( children->size() );
	for ( auto& it : *children )
		it->write(dos);
}

void GameRuleDefinition::writeAttributes(DataOutputStream *dos, UINT numAttributes)
{
	dos->writeInt(numAttributes + 3);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_descriptionName);
	dos->writeUTF(m_descriptionId);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_promptName);
	dos->writeUTF(m_promptId);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_dataTag);
	dos->writeUTF(std::to_wstring(m_4JDataValue));
}

void GameRuleDefinition::getChildren(vector<GameRuleDefinition *> *children) {}

GameRuleDefinition *GameRuleDefinition::addChild(ConsoleGameRules::EGameRuleType ruleType)
{
#ifndef _CONTENT_PACKAGE
		wprintf(L"GameRuleDefinition: Attempted to add invalid child rule - %d\n", ruleType );
#endif
	return NULL;
}

void GameRuleDefinition::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"descriptionName") == 0)
	{
		m_descriptionId = attributeValue;
#ifndef _CONTENT_PACKAGE
		wprintf(L"GameRuleDefinition: Adding parameter descriptionId=%ls\n",m_descriptionId.c_str());
#endif
	}
	else if(attributeName.compare(L"promptName") == 0)
	{
		m_promptId = attributeValue;
#ifndef _CONTENT_PACKAGE
		wprintf(L"GameRuleDefinition: Adding parameter m_promptId=%ls\n",m_promptId.c_str());
#endif
	}
	else if(attributeName.compare(L"dataTag") == 0)
	{
		m_4JDataValue = _fromString<int>(attributeValue);
		app.DebugPrintf("GameRuleDefinition: Adding parameter m_4JDataValue=%d\n",m_4JDataValue);
	}
	else
	{
#ifndef _CONTENT_PACKAGE
		wprintf(L"GameRuleDefinition: Attempted to add invalid attribute: %ls\n", attributeName.c_str());
#endif
	}
}

void GameRuleDefinition::populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule *rule)
{
	GameRule::ValueType value;
	value.b = false;
	rule->setParameter(L"bComplete",value);
}

bool GameRuleDefinition::getComplete(GameRule *rule)
{
	GameRule::ValueType value;
	value = rule->getParameter(L"bComplete");
	return value.b;
}

void GameRuleDefinition::setComplete(GameRule *rule, bool val)
{
	GameRule::ValueType value;
	value = rule->getParameter(L"bComplete");
	value.b = val;
	rule->setParameter(L"bComplete",value);
}

vector<GameRuleDefinition *> *GameRuleDefinition::enumerate()
{
	// Get Vector.
	vector<GameRuleDefinition *> *gRules;
	gRules = new vector<GameRuleDefinition *>();
	gRules->push_back(this);
	getChildren(gRules);
	return gRules;
}

unordered_map<GameRuleDefinition *, int> *GameRuleDefinition::enumerateMap()
{
	unordered_map<GameRuleDefinition *, int> *out
		= new unordered_map<GameRuleDefinition *, int>();

	int i = 0;
	vector<GameRuleDefinition *> *gRules = enumerate();
	for ( auto& it : *gRules )
		out->emplace(it, i++);

	return out;
}

GameRulesInstance *GameRuleDefinition::generateNewGameRulesInstance(GameRulesInstance::EGameRulesInstanceType type, LevelRuleset *rules, Connection *connection)
{
	GameRulesInstance *manager = new GameRulesInstance(rules, connection);

	rules->populateGameRule(type, manager);

	return manager;
}

wstring GameRuleDefinition::generateDescriptionString(ConsoleGameRules::EGameRuleType defType, const wstring &description, void *data, int dataLength)
{
	wstring formatted = description;
	switch(defType)
	{
	case ConsoleGameRules::eGameRuleType_CompleteAllRule:
		formatted = CompleteAllRuleDefinition::generateDescriptionString(description,data,dataLength);
		break;
	default:
		break;
	};
	return formatted;
}