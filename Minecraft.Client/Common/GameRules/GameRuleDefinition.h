#pragma once
using namespace std;
#include <unordered_map>
#include <string>

#include "..\..\..\Minecraft.World\ItemInstance.h"
#include "ConsoleGameRulesConstants.h"

#include "GameRulesInstance.h"

class GameRule;
class LevelRuleset;
class Player;
class WstringLookup;

class GameRuleDefinition
{
private:
	// Owner type defines who this rule applies to
	GameRulesInstance::EGameRulesInstanceType m_ownerType;

protected:
	// These attributes should map to those in the XSD GameRuleType
	wstring m_descriptionId;
	wstring m_promptId;
	int m_4JDataValue;

public:
	GameRuleDefinition();

	virtual ConsoleGameRules::EGameRuleType getActionType() = 0;

	void setOwnerType(GameRulesInstance::EGameRulesInstanceType ownerType) { m_ownerType = ownerType;}

	virtual void write(DataOutputStream *);

	virtual void writeAttributes(DataOutputStream *dos, UINT numAttributes);
	virtual void getChildren(vector<GameRuleDefinition *> *);

	virtual GameRuleDefinition *addChild(ConsoleGameRules::EGameRuleType ruleType);
	virtual void addAttribute(const wstring &attributeName, const wstring &attributeValue);
	
	virtual void populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule *rule);

	bool getComplete(GameRule *rule);
	void setComplete(GameRule *rule, bool val);

	virtual int getGoal() { return 0; }
	virtual int getProgress(GameRule *rule) { return 0; }

	virtual int getIcon() { return -1; }
	virtual int getAuxValue() { return 0; }

	// Here we should have functions for all the hooks, with a GameRule* as the first parameter
	virtual bool onUseTile(GameRule *rule, int tileId, int x, int y, int z) { return false; }
	virtual bool onCollectItem(GameRule *rule, shared_ptr<ItemInstance> item) { return false; }
	virtual void postProcessPlayer(shared_ptr<Player> player) { }

	vector<GameRuleDefinition *> *enumerate();
	unordered_map<GameRuleDefinition *, int> *enumerateMap();

	// Static functions
	static GameRulesInstance *generateNewGameRulesInstance(GameRulesInstance::EGameRulesInstanceType type, LevelRuleset *rules, Connection *connection);
	static wstring generateDescriptionString(ConsoleGameRules::EGameRuleType defType, const wstring &description, void *data = NULL, int dataLength = 0);

};