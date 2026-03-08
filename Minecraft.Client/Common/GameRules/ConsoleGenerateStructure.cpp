#include "stdafx.h"
#include "ConsoleGenerateStructure.h"
#include "ConsoleGameRules.h"
#include "../../../Minecraft.World/net.minecraft.world.level.h"
#include "../../../Minecraft.World/net.minecraft.world.level.dimension.h"
#include "../../../Minecraft.World/net.minecraft.world.level.levelgen.structure.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "../../../Minecraft.World/net.minecraft.h"

ConsoleGenerateStructure::ConsoleGenerateStructure() : StructurePiece(0)
{
	m_x = m_y = m_z = 0;
	boundingBox = NULL;
	orientation = Direction::NORTH;
	m_dimension = 0;
}

void ConsoleGenerateStructure::getChildren(vector<GameRuleDefinition *> *children)
{
	GameRuleDefinition::getChildren(children);

	for ( auto& action : m_actions )
		children->push_back( action );
}

GameRuleDefinition *ConsoleGenerateStructure::addChild(ConsoleGameRules::EGameRuleType ruleType)
{
	GameRuleDefinition *rule = NULL;
	if(ruleType == ConsoleGameRules::eGameRuleType_GenerateBox)
	{
		rule = new XboxStructureActionGenerateBox();
		m_actions.push_back((XboxStructureActionGenerateBox *)rule);
	}
	else if(ruleType == ConsoleGameRules::eGameRuleType_PlaceBlock)
	{
		rule = new XboxStructureActionPlaceBlock();
		m_actions.push_back((XboxStructureActionPlaceBlock *)rule);
	}
	else if(ruleType == ConsoleGameRules::eGameRuleType_PlaceContainer)
	{
		rule = new XboxStructureActionPlaceContainer();
		m_actions.push_back((XboxStructureActionPlaceContainer *)rule);
	}
	else if(ruleType == ConsoleGameRules::eGameRuleType_PlaceSpawner)
	{
		rule = new XboxStructureActionPlaceSpawner();
		m_actions.push_back((XboxStructureActionPlaceSpawner *)rule);
	}
	else
	{
#ifndef _CONTENT_PACKAGE
		wprintf(L"ConsoleGenerateStructure: Attempted to add invalid child rule - %d\n", ruleType );
#endif
	}
	return rule;
}

void ConsoleGenerateStructure::writeAttributes(DataOutputStream *dos, UINT numAttrs)
{
	GameRuleDefinition::writeAttributes(dos, numAttrs + 5);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_x);
	dos->writeUTF(std::to_wstring(m_x));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_y);
	dos->writeUTF(std::to_wstring(m_y));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_z);
	dos->writeUTF(std::to_wstring(m_z));

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_orientation);
	dos->writeUTF(std::to_wstring(orientation));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_dimension);
	dos->writeUTF(std::to_wstring(m_dimension));
}

void ConsoleGenerateStructure::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"x") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_x = value;
		app.DebugPrintf("ConsoleGenerateStructure: Adding parameter x=%d\n",m_x);
	}
	else if(attributeName.compare(L"y") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_y = value;
		app.DebugPrintf("ConsoleGenerateStructure: Adding parameter y=%d\n",m_y);
	}
	else if(attributeName.compare(L"z") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_z = value;
		app.DebugPrintf("ConsoleGenerateStructure: Adding parameter z=%d\n",m_z);
	}
	else if(attributeName.compare(L"orientation") == 0)
	{
		int value = _fromString<int>(attributeValue);
		orientation = value;
		app.DebugPrintf("ConsoleGenerateStructure: Adding parameter orientation=%d\n",orientation);
	}
	else if(attributeName.compare(L"dim") == 0)
	{
		m_dimension = _fromString<int>(attributeValue);
		if(m_dimension > 1 || m_dimension < -1) m_dimension = 0;
		app.DebugPrintf("ApplySchematicRuleDefinition: Adding parameter dimension=%d\n",m_dimension);
	}
	else
	{
		GameRuleDefinition::addAttribute(attributeName, attributeValue);
	}
}

BoundingBox* ConsoleGenerateStructure::getBoundingBox()
{
	if(boundingBox == NULL)
	{
		// Find the max bounds
		int maxX, maxY, maxZ;
		maxX = maxY = maxZ = 1;
		for( ConsoleGenerateStructureAction *action : m_actions )
		{
			maxX = std::max<int>(maxX,action->getEndX());
			maxY = std::max<int>(maxY,action->getEndY());
			maxZ = std::max<int>(maxZ,action->getEndZ());
		}

		boundingBox = new BoundingBox(m_x, m_y, m_z, m_x + maxX, m_y + maxY, m_z + maxZ);
	}
	return boundingBox;
}

bool ConsoleGenerateStructure::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if(level->dimension->id != m_dimension) return false;

	for( ConsoleGenerateStructureAction *action : m_actions )
	{
		switch(action->getActionType())
		{
		case ConsoleGameRules::eGameRuleType_GenerateBox:
			{
				XboxStructureActionGenerateBox *genBox = (XboxStructureActionGenerateBox *)action;
				genBox->generateBoxInLevel(this,level,chunkBB);
			}
			break;
		case ConsoleGameRules::eGameRuleType_PlaceBlock:
			{
				XboxStructureActionPlaceBlock *pPlaceBlock = (XboxStructureActionPlaceBlock *)action;
				pPlaceBlock->placeBlockInLevel(this,level,chunkBB);
			}
			break;
		case ConsoleGameRules::eGameRuleType_PlaceContainer:
			{
				XboxStructureActionPlaceContainer *pPlaceContainer = (XboxStructureActionPlaceContainer *)action;
				pPlaceContainer->placeContainerInLevel(this,level,chunkBB);
			}
			break;
		case ConsoleGameRules::eGameRuleType_PlaceSpawner:
			{
				XboxStructureActionPlaceSpawner *pPlaceSpawner = (XboxStructureActionPlaceSpawner *)action;
				pPlaceSpawner->placeSpawnerInLevel(this,level,chunkBB);
			}
			break;
		};
	}

	return false;
}

bool ConsoleGenerateStructure::checkIntersects(int x0, int y0, int z0, int x1, int y1, int z1)
{
	return getBoundingBox()->intersects(x0,y0,z0,x1,y1,z1);
}

int ConsoleGenerateStructure::getMinY()
{
	return getBoundingBox()->y0;
}