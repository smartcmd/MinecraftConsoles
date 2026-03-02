#pragma once

#include "GameRuleDefinition.h"

class Container;
class AddEnchantmentRuleDefinition;

class AddItemRuleDefinition : public GameRuleDefinition
{
private:
	int m_itemId;
	int m_quantity;
	int m_auxValue;
	int m_dataTag;
	int m_slot;
	vector<AddEnchantmentRuleDefinition *> m_enchantments;

public:
	AddItemRuleDefinition();

	virtual void writeAttributes(DataOutputStream *, UINT numAttributes);
	virtual void getChildren(vector<GameRuleDefinition *> *children);

	virtual ConsoleGameRules::EGameRuleType getActionType() { return ConsoleGameRules::eGameRuleType_AddItem; }

	virtual GameRuleDefinition *addChild(ConsoleGameRules::EGameRuleType ruleType);
	virtual void addAttribute(const wstring &attributeName, const wstring &attributeValue);

	bool addItemToContainer(shared_ptr<Container> container, int slotId);
};