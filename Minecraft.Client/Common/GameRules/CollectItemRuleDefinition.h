#pragma once

#include "GameRuleDefinition.h"

class Pos;
class UseTileRuleDefinition;
class ItemInstance;

class CollectItemRuleDefinition : public GameRuleDefinition
{
private:
	// These values should map directly to the xsd definition for this Rule
	int m_itemId;
	unsigned char m_auxValue;
	int m_quantity;

public:
	CollectItemRuleDefinition();
	~CollectItemRuleDefinition();

	ConsoleGameRules::EGameRuleType getActionType() { return ConsoleGameRules::eGameRuleType_CollectItemRule; }

	virtual void writeAttributes(DataOutputStream *, UINT numAttributes);
	virtual void addAttribute(const wstring &attributeName, const wstring &attributeValue);

	virtual int getGoal();
	virtual int getProgress(GameRule *rule);
	
	virtual int getIcon() { return m_itemId; }
	virtual int getAuxValue() { return m_auxValue; }

	void populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule *rule);

	bool onCollectItem(GameRule *rule, shared_ptr<ItemInstance> item);

	static wstring generateXml(shared_ptr<ItemInstance> item);

private:	
	//static wstring generateXml(CollectItemRuleDefinition *ruleDef);
};