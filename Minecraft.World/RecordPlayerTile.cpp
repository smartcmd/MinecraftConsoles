#include "stdafx.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.h"
#include "net.minecraft.world.h"
#include "RecordPlayerTile.h"
#include "LevelEvent.h"


RecordPlayerTile::RecordPlayerTile(int id) : EntityTile(id, Material::wood)
{
	iconTop = NULL;
}

Icon *RecordPlayerTile::getTexture(int face, int data)
{
	if (face == Facing::UP)
	{
		return iconTop;
	}
	return icon;
}

// 4J-PB - Adding a TestUse for tooltip display
bool RecordPlayerTile::TestUse(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	// if the jukebox is empty, return true
	if (level->getData(x, y, z) == 0) return false;
	return true;
}

bool RecordPlayerTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if (soundOnly) return false;
    if (level->getData(x, y, z) == 0) return false;
    dropRecording(level, x, y, z);
    return true;
}

void RecordPlayerTile::setRecord(Level *level, int x, int y, int z, int record)
{
    if (level->isClientSide) return;

    shared_ptr<RecordPlayerTile::Entity> rte = dynamic_pointer_cast<RecordPlayerTile::Entity>( level->getTileEntity(x, y, z) );
    rte->record = record;
    rte->setChanged();

    level->setData(x, y, z, 1);
}

void RecordPlayerTile::dropRecording(Level *level, int x, int y, int z)
{
    if (level->isClientSide) return;

    shared_ptr<RecordPlayerTile::Entity> rte = dynamic_pointer_cast<RecordPlayerTile::Entity>( level->getTileEntity(x, y, z) );
	if( rte == NULL ) return;

    int oldRecord = rte->record;
    if (oldRecord == 0) return;


    level->levelEvent(LevelEvent::SOUND_PLAY_RECORDING, x, y, z, 0);
	// 4J-PB- the level event will play the music
    //level->playStreamingMusic(L"", x, y, z);
    rte->record = 0;
    rte->setChanged();
    level->setData(x, y, z, 0);

    float s = 0.7f;
    double xo = level->random->nextFloat() * s + (1 - s) * 0.5;
    double yo = level->random->nextFloat() * s + (1 - s) * 0.2 + 0.6;
    double zo = level->random->nextFloat() * s + (1 - s) * 0.5;
    shared_ptr<ItemEntity> item = shared_ptr<ItemEntity>( new ItemEntity(level, x + xo, y + yo, z + zo, shared_ptr<ItemInstance>( new ItemInstance(oldRecord, 1, 0) ) ) );
    item->throwTime = 10;
    level->addEntity(item);
}

void RecordPlayerTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
    dropRecording(level, x, y, z);
    Tile::onRemove(level, x, y, z, id, data);
}

void RecordPlayerTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus)
{
    if (level->isClientSide) return;
    Tile::spawnResources(level, x, y, z, data, odds, 0);
}

shared_ptr<TileEntity> RecordPlayerTile::newTileEntity(Level *level)
{
	return shared_ptr<RecordPlayerTile::Entity>( new RecordPlayerTile::Entity() );
}

void RecordPlayerTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"musicBlock");
	iconTop = iconRegister->registerIcon(L"jukebox_top");
}