#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.redstone.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "CommandBlock.h"

CommandBlock::CommandBlock(int id) : BaseEntityTile(id, Material::metal, isSolidRender() )
{
}

shared_ptr<TileEntity> CommandBlock::newTileEntity(Level *level)
{
	return std::make_shared<CommandBlockEntity>();
}

void CommandBlock::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (!level->isClientSide)
	{

		bool signal = level->hasNeighborSignal(x, y, z);
		int data = level->getData(x, y, z);
		bool isTriggered = (data & TRIGGER_BIT) != 0;

		if (signal && !isTriggered)
		{
			level->setData(x, y, z, data | TRIGGER_BIT, Tile::UPDATE_NONE);
			level->addToTickNextTick(x, y, z, id, getTickDelay(level));
		}
		else if (!signal && isTriggered)
		{
			level->setData(x, y, z, data & ~TRIGGER_BIT, Tile::UPDATE_NONE);
		}
	}
}

void CommandBlock::tick(Level *level, int x, int y, int z, Random *random)
{
	shared_ptr<TileEntity> tileEntity = level->getTileEntity(x, y, z);

	if (tileEntity != nullptr && dynamic_pointer_cast<CommandBlockEntity>( tileEntity ) != nullptr)
	{
		shared_ptr<CommandBlockEntity> commandBlock = dynamic_pointer_cast<CommandBlockEntity>( tileEntity );
		commandBlock->setSuccessCount(commandBlock->performCommand(level));
		level->updateNeighbourForOutputSignal(x, y, z, id);
	}
}

int CommandBlock::getTickDelay(Level *level)
{
	return 1;
}

bool CommandBlock::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly)
{
	shared_ptr<CommandBlockEntity> amce = dynamic_pointer_cast<CommandBlockEntity>( level->getTileEntity(x, y, z) );

	if (amce != nullptr)
	{
		player->openTextEdit(amce);
	}

	return true;
}

bool CommandBlock::hasAnalogOutputSignal()
{
	return true;
}

int CommandBlock::getAnalogOutputSignal(Level *level, int x, int y, int z, int dir)
{
	shared_ptr<TileEntity> tileEntity = level->getTileEntity(x, y, z);

	if (tileEntity != nullptr && dynamic_pointer_cast<CommandBlockEntity>( tileEntity ) != nullptr)
	{
		return dynamic_pointer_cast<CommandBlockEntity>( tileEntity )->getSuccessCount();
	}

	return Redstone::SIGNAL_NONE;
}

void CommandBlock::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	shared_ptr<CommandBlockEntity> cblock = dynamic_pointer_cast<CommandBlockEntity>( level->getTileEntity(x, y, z) );

	if (itemInstance->hasCustomHoverName())
	{
		cblock->setName(itemInstance->getHoverName());
	}
}

int CommandBlock::getResourceCount(Random *random)
{
	return 0;
}