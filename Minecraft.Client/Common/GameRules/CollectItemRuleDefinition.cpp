#include "stdafx.h"
#include "..\..\WstringLookup.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "CollectItemRuleDefinition.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\Connection.h"
#include "..\..\..\Minecraft.World\net.minecraft.network.packet.h"

CollectItemRuleDefinition::CollectItemRuleDefinition()
{
	m_itemId = 0;
	m_auxValue = 0;
	m_quantity = 0;
}

CollectItemRuleDefinition::~CollectItemRuleDefinition()
{
}

void CollectItemRuleDefinition::writeAttributes(DataOutputStream *dos, UINT numAttributes)
{
	GameRuleDefinition::writeAttributes(dos, numAttributes + 3);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_itemId);
	dos->writeUTF( std::to_wstring( m_itemId ) );

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_auxValue);
	dos->writeUTF( std::to_wstring( m_auxValue ) );

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_quantity);
	dos->writeUTF( std::to_wstring( m_quantity ) );
}

void CollectItemRuleDefinition::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"itemId") == 0)
	{
		m_itemId = _fromString<int>(attributeValue);
		app.DebugPrintf("CollectItemRule: Adding parameter itemId=%d\n",m_itemId);
	}
	else if(attributeName.compare(L"auxValue") == 0)
	{
		m_auxValue = _fromString<int>(attributeValue);
		app.DebugPrintf("CollectItemRule: Adding parameter m_auxValue=%d\n",m_auxValue);
	}
	else if(attributeName.compare(L"quantity") == 0)
	{
		m_quantity = _fromString<int>(attributeValue);
		app.DebugPrintf("CollectItemRule: Adding parameter m_quantity=%d\n",m_quantity);
	}
	else
	{
		GameRuleDefinition::addAttribute(attributeName, attributeValue);
	}
}

int CollectItemRuleDefinition::getGoal()
{
	return m_quantity;
}

int CollectItemRuleDefinition::getProgress(GameRule *rule)
{
	GameRule::ValueType value = rule->getParameter(L"iQuantity");
	return value.i;
}

void CollectItemRuleDefinition::populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule *rule)
{
	GameRule::ValueType value;
	value.i = 0;
	rule->setParameter(L"iQuantity",value);

	GameRuleDefinition::populateGameRule(type, rule);
}

bool CollectItemRuleDefinition::onCollectItem(GameRule *rule, shared_ptr<ItemInstance> item)
{
	bool statusChanged = false;
	if(item != nullptr && item->id == m_itemId && item->getAuxValue() == m_auxValue && item->get4JData() == m_4JDataValue)
	{
		if(!getComplete(rule))
		{
			GameRule::ValueType value = rule->getParameter(L"iQuantity");
			int quantityCollected = (value.i += item->count);
			rule->setParameter(L"iQuantity",value);

			statusChanged = true;

			if(quantityCollected >= m_quantity)
			{
				setComplete(rule, true);
                app.DebugPrintf("Completed CollectItemRule with info - itemId:%d, auxValue:%d, quantity:%d, dataTag:%d\n", m_itemId, m_auxValue, m_quantity, m_4JDataValue);

                if (rule->getConnection() != nullptr)
                {
                    rule->getConnection()->send(std::make_shared<UpdateGameRuleProgressPacket>(
                        getActionType(),
                        this->m_descriptionId,
                        m_itemId,
                        m_auxValue,
                        this->m_4JDataValue,
                        nullptr,
                        static_cast<DWORD>(0)
                        ));
                }
            }
		}
	}
	return statusChanged;
}

wstring CollectItemRuleDefinition::generateXml(shared_ptr<ItemInstance> item)
{
	// 4J Stu - This should be kept in sync with the GameRulesDefinition.xsd
	wstring xml = L"";
	if(item != nullptr)
	{
		xml = L"<CollectItemRule itemId=\"" + std::to_wstring(item->id) + L"\" quantity=\"SET\" descriptionName=\"OPTIONAL\" promptName=\"OPTIONAL\"";
		if(item->getAuxValue() != 0) xml += L" auxValue=\"" + std::to_wstring(item->getAuxValue()) + L"\"";
		if(item->get4JData() != 0) xml += L" dataTag=\"" + std::to_wstring(item->get4JData()) + L"\"";
		xml += L"/>\n";
	}
	return xml;
}