#include "stdafx.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "XboxStructureActionPlaceContainer.h"
#include "AddItemRuleDefinition.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.levelgen.structure.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"

XboxStructureActionPlaceContainer::XboxStructureActionPlaceContainer()
{
	m_tile = Tile::chest_Id;
}

XboxStructureActionPlaceContainer::~XboxStructureActionPlaceContainer()
{
	for(auto& item : m_items)
	{
		delete item;
	}
}

// 4J-JEV: Super class handles attr-facing fine.
//void XboxStructureActionPlaceContainer::writeAttributes(DataOutputStream *dos, UINT numAttrs)


void XboxStructureActionPlaceContainer::getChildren(vector<GameRuleDefinition *> *children)
{
	XboxStructureActionPlaceBlock::getChildren(children);
	for(auto & item : m_items)
		children->push_back( item );
}

GameRuleDefinition *XboxStructureActionPlaceContainer::addChild(ConsoleGameRules::EGameRuleType ruleType)
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
		wprintf(L"XboxStructureActionPlaceContainer: Attempted to add invalid child rule - %d\n", ruleType );
#endif
	}
	return rule;
}

void XboxStructureActionPlaceContainer::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"facing") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_data = value;
		app.DebugPrintf("XboxStructureActionPlaceContainer: Adding parameter facing=%d\n",m_data);
	}
	else
	{
		XboxStructureActionPlaceBlock::addAttribute(attributeName, attributeValue);
	}
}

bool XboxStructureActionPlaceContainer::placeContainerInLevel(StructurePiece *structure, Level *level, BoundingBox *chunkBB)
{
	int worldX = structure->getWorldX( m_x, m_z );
	int worldY = structure->getWorldY( m_y );
	int worldZ = structure->getWorldZ( m_x, m_z );

	if ( chunkBB->isInside( worldX, worldY, worldZ ) )
	{
		if ( level->getTileEntity( worldX, worldY, worldZ ) != nullptr )
		{
			// Remove the current tile entity
			level->removeTileEntity( worldX, worldY, worldZ );
			level->setTileAndData( worldX, worldY, worldZ, 0, 0, Tile::UPDATE_ALL );
		}

		level->setTileAndData( worldX, worldY, worldZ, m_tile, 0, Tile::UPDATE_ALL );
		shared_ptr<Container> container = dynamic_pointer_cast<Container>(level->getTileEntity( worldX, worldY, worldZ ));

		app.DebugPrintf("XboxStructureActionPlaceContainer - placing a container at (%d,%d,%d)\n", worldX, worldY, worldZ);
		if ( container != nullptr )
		{
			level->setData( worldX, worldY, worldZ, m_data, Tile::UPDATE_CLIENTS);
			// Add items
			int slotId = 0;
            for (auto it = m_items.begin(); it != m_items.end() && (slotId < container->getContainerSize()); ++it, ++slotId)
            {
				AddItemRuleDefinition *addItem = *it;

				addItem->addItemToContainer(container,slotId);
			}
		}
		return true;
	}
	return false;
}