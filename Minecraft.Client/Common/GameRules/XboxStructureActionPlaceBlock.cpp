#include "stdafx.h"
#include "XboxStructureActionPlaceBlock.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.levelgen.structure.h"

XboxStructureActionPlaceBlock::XboxStructureActionPlaceBlock()
{
	m_x = m_y = m_z = m_tile = m_data = 0;
}

void XboxStructureActionPlaceBlock::writeAttributes(DataOutputStream *dos, UINT numAttrs)
{
	ConsoleGenerateStructureAction::writeAttributes(dos, numAttrs + 5);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_x);
	dos->writeUTF(std::to_wstring(m_x));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_y);
	dos->writeUTF(std::to_wstring(m_y));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_z);
	dos->writeUTF(std::to_wstring(m_z));

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_data);
	dos->writeUTF(std::to_wstring(m_data));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_block);
	dos->writeUTF(std::to_wstring(m_tile));
}


void XboxStructureActionPlaceBlock::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"x") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_x = value;
		app.DebugPrintf("XboxStructureActionPlaceBlock: Adding parameter x=%d\n",m_x);
	}
	else if(attributeName.compare(L"y") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_y = value;
		app.DebugPrintf("XboxStructureActionPlaceBlock: Adding parameter y=%d\n",m_y);
	}
	else if(attributeName.compare(L"z") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_z = value;
		app.DebugPrintf("XboxStructureActionPlaceBlock: Adding parameter z=%d\n",m_z);
	}
	else if(attributeName.compare(L"block") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_tile = value;
		app.DebugPrintf("XboxStructureActionPlaceBlock: Adding parameter block=%d\n",m_tile);
	}
	else if(attributeName.compare(L"data") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_data = value;
		app.DebugPrintf("XboxStructureActionPlaceBlock: Adding parameter data=%d\n",m_data);
	}
	else
	{
		GameRuleDefinition::addAttribute(attributeName, attributeValue);
	}
}

bool XboxStructureActionPlaceBlock::placeBlockInLevel(StructurePiece *structure, Level *level, BoundingBox *chunkBB)
{
	app.DebugPrintf("XboxStructureActionPlaceBlock - placing a block\n");
	structure->placeBlock(level,m_tile,m_data,m_x,m_y,m_z,chunkBB);
	return true;
}