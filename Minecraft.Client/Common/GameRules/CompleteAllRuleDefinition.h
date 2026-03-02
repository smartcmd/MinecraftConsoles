#pragma once

#include "CompoundGameRuleDefinition.h"

class CompleteAllRuleDefinition : public CompoundGameRuleDefinition
{
private:
	typedef struct _packetData
	{
		int goal;
		int progress;
	} PacketData;

public:
	ConsoleGameRules::EGameRuleType getActionType() { return ConsoleGameRules::eGameRuleType_CompleteAllRule; }

	virtual void getChildren(vector<GameRuleDefinition *> *children);

	virtual bool onUseTile(GameRule *rule, int tileId, int x, int y, int z);
	virtual bool onCollectItem(GameRule *rule, shared_ptr<ItemInstance> item);

	static wstring generateDescriptionString(const wstring &description, void *data, int dataLength);

private:
	void updateStatus(GameRule *rule);
};