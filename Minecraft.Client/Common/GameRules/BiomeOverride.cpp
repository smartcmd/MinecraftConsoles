#include "stdafx.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "BiomeOverride.h"

BiomeOverride::BiomeOverride()
{
	m_tile = 0;
	m_topTile = 0;
	m_biomeId = 0;
}

void BiomeOverride::writeAttributes(DataOutputStream *dos, UINT numAttrs)
{
	GameRuleDefinition::writeAttributes(dos, numAttrs + 3);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_biomeId);
	dos->writeUTF(std::to_wstring(m_biomeId));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_tileId);
	dos->writeUTF(std::to_wstring(m_tile));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_topTileId);
	dos->writeUTF(std::to_wstring(m_topTile));
}

void BiomeOverride::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"tileId") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_tile = value;
		app.DebugPrintf("BiomeOverride: Adding parameter tileId=%d\n",m_tile);
	}
	else if(attributeName.compare(L"topTileId") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_topTile = value;
		app.DebugPrintf("BiomeOverride: Adding parameter topTileId=%d\n",m_topTile);
	}
	else if(attributeName.compare(L"biomeId") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_biomeId = value;
		app.DebugPrintf("BiomeOverride: Adding parameter biomeId=%d\n",m_biomeId);
	}
	else
	{
		GameRuleDefinition::addAttribute(attributeName, attributeValue);
	}
}

bool BiomeOverride::isBiome(int id)
{
	return m_biomeId == id;
}

void BiomeOverride::getTileValues(BYTE &tile, BYTE &topTile)
{
	if(m_tile != 0) tile = (BYTE)m_tile;
	if(m_topTile != 0) topTile = (BYTE)m_topTile;
}