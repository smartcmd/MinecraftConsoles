#pragma once

#include "GameRuleDefinition.h"

class ItemInstance;

class AddEnchantmentRuleDefinition : public GameRuleDefinition
{
private:
	int m_enchantmentId;
	int m_enchantmentLevel;

public:
	AddEnchantmentRuleDefinition();

	virtual ConsoleGameRules::EGameRuleType getActionType() { return ConsoleGameRules::eGameRuleType_AddEnchantment; }

	virtual void writeAttributes(DataOutputStream *, UINT numAttrs);

	virtual void addAttribute(const wstring &attributeName, const wstring &attributeValue);

	bool enchantItem(shared_ptr<ItemInstance> item);
};