#include "stdafx.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "UseTileRuleDefinition.h"

UseTileRuleDefinition::UseTileRuleDefinition()
{
	m_tileId = -1;
	m_useCoords = false;
}

void UseTileRuleDefinition::writeAttributes(DataOutputStream *dos, UINT numAttributes)
{
	GameRuleDefinition::writeAttributes(dos, numAttributes + 5);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_tileId);
	dos->writeUTF(std::to_wstring(m_tileId));

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_useCoords);
	dos->writeUTF(std::to_wstring(m_useCoords));

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_x);
	dos->writeUTF(std::to_wstring(m_coordinates.x));

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_y);
	dos->writeUTF(std::to_wstring(m_coordinates.y));

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_z);
	dos->writeUTF(std::to_wstring(m_coordinates.z));
}

void UseTileRuleDefinition::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"tileId") == 0)
	{
		m_tileId = _fromString<int>(attributeValue);
		app.DebugPrintf("UseTileRule: Adding parameter tileId=%d\n",m_tileId);
	}
	else if(attributeName.compare(L"useCoords") == 0)
	{
		m_useCoords = _fromString<bool>(attributeValue);
		app.DebugPrintf("UseTileRule: Adding parameter useCoords=%s\n",m_useCoords?"TRUE":"FALSE");
	}
	else if(attributeName.compare(L"x") == 0)
	{
		m_coordinates.x = _fromString<int>(attributeValue);
		app.DebugPrintf("UseTileRule: Adding parameter x=%d\n",m_coordinates.x);
	}
	else if(attributeName.compare(L"y") == 0)
	{
		m_coordinates.y = _fromString<int>(attributeValue);
		app.DebugPrintf("UseTileRule: Adding parameter y=%d\n",m_coordinates.y);
	}
	else if(attributeName.compare(L"z") == 0)
	{
		m_coordinates.z = _fromString<int>(attributeValue);
		app.DebugPrintf("UseTileRule: Adding parameter z=%d\n",m_coordinates.z);
	}
	else
	{
		GameRuleDefinition::addAttribute(attributeName, attributeValue);
	}
}

bool UseTileRuleDefinition::onUseTile(GameRule *rule, int tileId, int x, int y, int z)
{
	bool statusChanged = false;
	if( m_tileId == tileId )
	{
		if( !m_useCoords || (m_coordinates.x == x && m_coordinates.y == y && m_coordinates.z == z) )
		{
			if(!getComplete(rule))
			{
				statusChanged = true;
				setComplete(rule,true);
				app.DebugPrintf("Completed UseTileRule with info - t:%d, coords:%s, x:%d, y:%d, z:%d\n", m_tileId,m_useCoords?"TRUE":"FALSE",m_coordinates.x,m_coordinates.y,m_coordinates.z);

				// Send a packet or some other announcement here
			}
		}
	}
	return statusChanged;
}