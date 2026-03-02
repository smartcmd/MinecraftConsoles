#pragma once
using namespace std;

#include "GameRuleDefinition.h"

class AddItemRuleDefinition;
class Pos;

class UpdatePlayerRuleDefinition : public GameRuleDefinition
{
private:
	vector<AddItemRuleDefinition *> m_items;

	bool m_bUpdateHealth, m_bUpdateFood, m_bUpdateYRot, m_bUpdateInventory;
	int m_health;
	int m_food;	
	Pos *m_spawnPos;
	float m_yRot;

public:
	UpdatePlayerRuleDefinition();
	~UpdatePlayerRuleDefinition();

	virtual ConsoleGameRules::EGameRuleType getActionType() { return ConsoleGameRules::eGameRuleType_UpdatePlayerRule; }
	
	virtual void getChildren(vector<GameRuleDefinition *> *children);
	virtual GameRuleDefinition *addChild(ConsoleGameRules::EGameRuleType ruleType);

	virtual void writeAttributes(DataOutputStream *dos, UINT numAttributes);
	virtual void addAttribute(const wstring &attributeName, const wstring &attributeValue);

	virtual void postProcessPlayer(shared_ptr<Player> player);
};