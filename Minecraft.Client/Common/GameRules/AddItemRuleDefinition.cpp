#include "stdafx.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "../../../Minecraft.World/net.minecraft.world.item.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../../Minecraft.World/net.minecraft.world.entity.player.h"
#include "AddItemRuleDefinition.h"
#include "AddEnchantmentRuleDefinition.h"

AddItemRuleDefinition::AddItemRuleDefinition()
{
	m_itemId = m_quantity = m_auxValue = m_dataTag = 0;
	m_slot = -1;
}

void AddItemRuleDefinition::writeAttributes(DataOutputStream *dos, UINT numAttrs)
{
	GameRuleDefinition::writeAttributes(dos, numAttrs + 5);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_itemId);
	dos->writeUTF( std::to_wstring( m_itemId ) );

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_quantity);
	dos->writeUTF( std::to_wstring( m_quantity ) );

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_auxValue);
	dos->writeUTF( std::to_wstring( m_auxValue ) );

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_dataTag);
	dos->writeUTF( std::to_wstring( m_dataTag ) );

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_slot);
	dos->writeUTF( std::to_wstring( m_slot ) );
}

void AddItemRuleDefinition::getChildren(vector<GameRuleDefinition *> *children)
{
	GameRuleDefinition::getChildren( children );
	for ( const auto& it : m_enchantments )
		children->push_back( it );
}

GameRuleDefinition *AddItemRuleDefinition::addChild(ConsoleGameRules::EGameRuleType ruleType)
{
	GameRuleDefinition *rule = NULL;
	if(ruleType == ConsoleGameRules::eGameRuleType_AddEnchantment)
	{
		rule = new AddEnchantmentRuleDefinition();
		m_enchantments.push_back((AddEnchantmentRuleDefinition *)rule);
	}
	else
	{
#ifndef _CONTENT_PACKAGE
		//wprintf(L"AddItemRuleDefinition: Attempted to add invalid child rule - %d\n", ruleType );
#endif
	}
	return rule;
}

void AddItemRuleDefinition::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"itemId") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_itemId = value;
		//app.DebugPrintf(2,"AddItemRuleDefinition: Adding parameter itemId=%d\n",m_itemId);
	}
	else if(attributeName.compare(L"quantity") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_quantity = value;
		//app.DebugPrintf(2,"AddItemRuleDefinition: Adding parameter quantity=%d\n",m_quantity);
	}
	else if(attributeName.compare(L"auxValue") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_auxValue = value;
		//app.DebugPrintf(2,"AddItemRuleDefinition: Adding parameter auxValue=%d\n",m_auxValue);
	}
	else if(attributeName.compare(L"dataTag") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_dataTag = value;
		//app.DebugPrintf(2,"AddItemRuleDefinition: Adding parameter dataTag=%d\n",m_dataTag);
	}
	else if(attributeName.compare(L"slot") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_slot = value;
		//app.DebugPrintf(2,"AddItemRuleDefinition: Adding parameter slot=%d\n",m_slot);
	}
	else
	{
		GameRuleDefinition::addAttribute(attributeName, attributeValue);
	}
}

bool AddItemRuleDefinition::addItemToContainer(shared_ptr<Container> container, int slotId)
{
	bool added = false;
	if(Item::items[m_itemId] != NULL)
	{
		int quantity = std::min<int>(m_quantity, Item::items[m_itemId]->getMaxStackSize());
		shared_ptr<ItemInstance> newItem = shared_ptr<ItemInstance>(new ItemInstance(m_itemId,quantity,m_auxValue) );
		newItem->set4JData(m_dataTag);

		for( auto& it : m_enchantments )
		{
			it->enchantItem(newItem);
		}

		if(m_slot >= 0 && m_slot < container->getContainerSize() )
		{
			container->setItem( m_slot, newItem );
			added = true;
		}
		else if(slotId >= 0 && slotId < container->getContainerSize() )
		{
			container->setItem( slotId, newItem );
			added = true;
		}
		else if(dynamic_pointer_cast<Inventory>(container) != NULL)
		{
			added = dynamic_pointer_cast<Inventory>(container)->add(newItem);
		}
	}
	return added;
}