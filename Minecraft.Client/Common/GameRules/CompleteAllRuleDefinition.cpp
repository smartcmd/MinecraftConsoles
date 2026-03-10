#include "stdafx.h"
#include "CompleteAllRuleDefinition.h"
#include "ConsoleGameRules.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\Connection.h"
#include "..\..\..\Minecraft.World\net.minecraft.network.packet.h"

void CompleteAllRuleDefinition::getChildren(vector<GameRuleDefinition *> *children)
{
	CompoundGameRuleDefinition::getChildren(children);
}

bool CompleteAllRuleDefinition::onUseTile(GameRule *rule, int tileId, int x, int y, int z)
{
	bool statusChanged = CompoundGameRuleDefinition::onUseTile(rule,tileId,x,y,z);
	if(statusChanged) updateStatus(rule);
	return statusChanged;
}

bool CompleteAllRuleDefinition::onCollectItem(GameRule *rule, shared_ptr<ItemInstance> item)
{
	bool statusChanged = CompoundGameRuleDefinition::onCollectItem(rule,item);
	if(statusChanged) updateStatus(rule);
	return statusChanged;
}

void CompleteAllRuleDefinition::updateStatus(GameRule *rule)
{
	int goal = 0;
	int progress = 0;
	for (auto& it : rule->m_parameters )
	{
		if(it.second.isPointer)
		{
			goal += it.second.gr->getGameRuleDefinition()->getGoal();
			progress += it.second.gr->getGameRuleDefinition()->getProgress(it.second.gr);
		}
	}
	if(rule->getConnection() != nullptr)
	{
		PacketData data;
		data.goal = goal;
		data.progress = progress;

		int icon = -1;
		int auxValue = 0;

		if(m_lastRuleStatusChanged != nullptr)
		{
			icon = m_lastRuleStatusChanged->getIcon();
			auxValue = m_lastRuleStatusChanged->getAuxValue();
			m_lastRuleStatusChanged = nullptr;
		}
		rule->getConnection()->send(std::make_shared<UpdateGameRuleProgressPacket>(getActionType(), this->m_descriptionId, icon, auxValue, 0, &data, sizeof(PacketData)));
	}
	app.DebugPrintf("Updated CompleteAllRule - Completed %d of %d\n", progress, goal);
}

wstring CompleteAllRuleDefinition::generateDescriptionString(const wstring &description, void *data, int dataLength)
{
	PacketData *values = static_cast<PacketData *>(data);
	wstring newDesc = description;
	newDesc = replaceAll(newDesc,L"{*progress*}",std::to_wstring(values->progress));
	newDesc = replaceAll(newDesc,L"{*goal*}",std::to_wstring(values->goal));
	return newDesc;
}