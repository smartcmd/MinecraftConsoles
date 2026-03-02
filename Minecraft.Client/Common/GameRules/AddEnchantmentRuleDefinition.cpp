#include "stdafx.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.enchantment.h"
#include "AddEnchantmentRuleDefinition.h"

AddEnchantmentRuleDefinition::AddEnchantmentRuleDefinition()
{
	m_enchantmentId = m_enchantmentLevel = 0;
}

void AddEnchantmentRuleDefinition::writeAttributes(DataOutputStream *dos, UINT numAttributes)
{
	GameRuleDefinition::writeAttributes(dos, numAttributes + 2);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_enchantmentId);
	dos->writeUTF( _toString( m_enchantmentId ) );

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_enchantmentLevel);
	dos->writeUTF( _toString( m_enchantmentLevel ) );
}

void AddEnchantmentRuleDefinition::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"enchantmentId") == 0)
	{
		int value = _fromString<int>(attributeValue);
		if(value < 0) value = 0;
		if(value >= 256) value = 255;
		m_enchantmentId = value;
		app.DebugPrintf("AddEnchantmentRuleDefinition: Adding parameter enchantmentId=%d\n",m_enchantmentId);
	}
	else if(attributeName.compare(L"enchantmentLevel") == 0)
	{
		int value = _fromString<int>(attributeValue);
		if(value < 0) value = 0;
		m_enchantmentLevel = value;
		app.DebugPrintf("AddEnchantmentRuleDefinition: Adding parameter enchantmentLevel=%d\n",m_enchantmentLevel);
	}
	else
	{
		GameRuleDefinition::addAttribute(attributeName, attributeValue);
	}
}

bool AddEnchantmentRuleDefinition::enchantItem(shared_ptr<ItemInstance> item)
{
	bool enchanted = false;
	if (item != NULL)
	{
		// 4J-JEV: Ripped code from enchantmenthelpers
		// Maybe we want to add an addEnchantment method to EnchantmentHelpers
		if (item->id == Item::enchantedBook_Id) 
		{
			Item::enchantedBook->addEnchantment( item, new EnchantmentInstance(m_enchantmentId, m_enchantmentLevel) );
		}
		else if (item->isEnchantable())
		{
			Enchantment *e = Enchantment::enchantments[m_enchantmentId];

			if(e != NULL && e->category->canEnchant(item->getItem()))
			{
				int level = min(e->getMaxLevel(), m_enchantmentLevel);
				item->enchant(e, m_enchantmentLevel);
				enchanted = true;
			}
		}
	}
	return enchanted;
}