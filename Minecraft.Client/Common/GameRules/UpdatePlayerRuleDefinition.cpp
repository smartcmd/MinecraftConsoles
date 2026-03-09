#include "stdafx.h"
#include "UpdatePlayerRuleDefinition.h"
#include "ConsoleGameRules.h"
#include "..\..\..\Minecraft.World\Pos.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.food.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"

UpdatePlayerRuleDefinition::UpdatePlayerRuleDefinition()
{
	m_bUpdateHealth = m_bUpdateFood = m_bUpdateYRot = false;;
	m_health = 0;
	m_food = 0;
	m_spawnPos = nullptr;
	m_yRot = 0.0f;
}

UpdatePlayerRuleDefinition::~UpdatePlayerRuleDefinition()
{
	for(auto& item : m_items)
	{
		delete item;
	}
}

void UpdatePlayerRuleDefinition::writeAttributes(DataOutputStream *dos, UINT numAttributes)
{
	int attrCount = 3;
	if(m_bUpdateHealth) ++attrCount;
	if(m_bUpdateFood) ++attrCount;
	if(m_bUpdateYRot) ++attrCount;
	GameRuleDefinition::writeAttributes(dos, numAttributes + attrCount );

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_spawnX);
	dos->writeUTF(std::to_wstring(m_spawnPos->x));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_spawnY);
	dos->writeUTF(std::to_wstring(m_spawnPos->y));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_spawnZ);
	dos->writeUTF(std::to_wstring(m_spawnPos->z));

	if(m_bUpdateYRot)
	{
		ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_yRot);
		dos->writeUTF(std::to_wstring(m_yRot));
	}
	if(m_bUpdateHealth)
	{
		ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_food);
		dos->writeUTF(std::to_wstring(m_health));
	}
	if(m_bUpdateFood)
	{
		ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_health);
		dos->writeUTF(std::to_wstring(m_food));
	}
}

void UpdatePlayerRuleDefinition::getChildren(vector<GameRuleDefinition *> *children)
{
	GameRuleDefinition::getChildren(children);
	for(auto& item : m_items)
		children->push_back(item);
}

GameRuleDefinition *UpdatePlayerRuleDefinition::addChild(ConsoleGameRules::EGameRuleType ruleType)
{
	GameRuleDefinition *rule = nullptr;
	if(ruleType == ConsoleGameRules::eGameRuleType_AddItem)
	{
		rule = new AddItemRuleDefinition();
		m_items.push_back(static_cast<AddItemRuleDefinition *>(rule));
	}
	else
	{
#ifndef _CONTENT_PACKAGE
		wprintf(L"UpdatePlayerRuleDefinition: Attempted to add invalid child rule - %d\n", ruleType );
#endif
	}
	return rule;
}

void UpdatePlayerRuleDefinition::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"spawnX") == 0)
	{
		if(m_spawnPos == nullptr) m_spawnPos = new Pos();
		int value = _fromString<int>(attributeValue);
		m_spawnPos->x = value;
		app.DebugPrintf("UpdatePlayerRuleDefinition: Adding parameter spawnX=%d\n",value);
	}
	else if(attributeName.compare(L"spawnY") == 0)
	{
		if(m_spawnPos == nullptr) m_spawnPos = new Pos();
		int value = _fromString<int>(attributeValue);
		m_spawnPos->y = value;
		app.DebugPrintf("UpdatePlayerRuleDefinition: Adding parameter spawnY=%d\n",value);
	}
	else if(attributeName.compare(L"spawnZ") == 0)
	{
		if(m_spawnPos == nullptr) m_spawnPos = new Pos();
		int value = _fromString<int>(attributeValue);
		m_spawnPos->z = value;
		app.DebugPrintf("UpdatePlayerRuleDefinition: Adding parameter spawnZ=%d\n",value);
	}
	else if(attributeName.compare(L"health") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_health = value;
		m_bUpdateHealth = true;
		app.DebugPrintf("UpdatePlayerRuleDefinition: Adding parameter health=%d\n",value);
	}
	else if(attributeName.compare(L"food") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_food = value;
		m_bUpdateFood = true;
		app.DebugPrintf("UpdatePlayerRuleDefinition: Adding parameter health=%d\n",value);
	}
	else if(attributeName.compare(L"yRot") == 0)
	{
		float value = _fromString<float>(attributeValue);
		m_yRot = value;
		m_bUpdateYRot = true;
		app.DebugPrintf("UpdatePlayerRuleDefinition: Adding parameter yRot=%f\n",value);
	}
	else
	{
		GameRuleDefinition::addAttribute(attributeName, attributeValue);
	}
}

void UpdatePlayerRuleDefinition::postProcessPlayer(shared_ptr<Player> player)
{
	if(m_bUpdateHealth)
	{
		player->lastHealth = m_health;
		player->setHealth(m_health);
	}

	if(m_bUpdateFood)
	{
		player->getFoodData()->setFoodLevel(m_food);
	}

	double x = player->x;
	double y = player->y;
	double z = player->z;
	float yRot = player->yRot;
	float xRot = player->xRot;
	if(m_spawnPos != nullptr)
	{
		x = m_spawnPos->x;
		y = m_spawnPos->y;
		z = m_spawnPos->z;
	}

	if(m_bUpdateYRot)
	{
		yRot = m_yRot;
	}

	if(m_spawnPos != nullptr || m_bUpdateYRot) player->absMoveTo(x,y,z,yRot,xRot);

	for(auto& addItem : m_items)
	{
		addItem->addItemToContainer(player->inventory, -1);
	}
}