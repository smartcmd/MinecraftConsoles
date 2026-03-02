#include "stdafx.h"
#include "net.minecraft.world.level.redstone.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.h"
#include "net.minecraft.world.h"
#include "JukeboxTile.h"
#include "LevelEvent.h"

JukeboxTile::Entity::Entity() : TileEntity()
{
	record = nullptr;
}

void JukeboxTile::Entity::load(CompoundTag *tag)
{
	TileEntity::load(tag);

	if (tag->contains(L"RecordItem"))
	{
		setRecord(ItemInstance::fromTag(tag->getCompound(L"RecordItem")));
	}
	else if (tag->getInt(L"Record") > 0)
	{
		setRecord(shared_ptr<ItemInstance>( new ItemInstance(tag->getInt(L"Record"), 1, 0)));
	}
}

void JukeboxTile::Entity::save(CompoundTag *tag)
{
	TileEntity::save(tag);

	if (getRecord() != NULL)
	{
		tag->putCompound(L"RecordItem", getRecord()->save(new CompoundTag()));

		tag->putInt(L"Record", getRecord()->id);
	}
}

// 4J Added
shared_ptr<TileEntity> JukeboxTile::Entity::clone()
{
	shared_ptr<JukeboxTile::Entity> result = shared_ptr<JukeboxTile::Entity>( new JukeboxTile::Entity() );
	TileEntity::clone(result);

	result->record = record;

	return result;
}

shared_ptr<ItemInstance> JukeboxTile::Entity::getRecord()
{
	return record;
}

void JukeboxTile::Entity::setRecord(shared_ptr<ItemInstance> record)
{
	this->record = record;
	setChanged();
}

JukeboxTile::JukeboxTile(int id) : BaseEntityTile(id, Material::wood)
{
	iconTop = NULL;
}

Icon *JukeboxTile::getTexture(int face, int data)
{
	if (face == Facing::UP)
	{
		return iconTop;
	}
	return icon;
}

// 4J-PB - Adding a TestUse for tooltip display
bool JukeboxTile::TestUse(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	// if the jukebox is empty, return true
	if (level->getData(x, y, z) == 0) return false;
	return true;
}

bool JukeboxTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if (soundOnly) return false;
	if (level->getData(x, y, z) == 0) return false;
	dropRecording(level, x, y, z);
	return true;
}

void JukeboxTile::setRecord(Level *level, int x, int y, int z, shared_ptr<ItemInstance> record)
{
	if (level->isClientSide) return;

	shared_ptr<JukeboxTile::Entity> rte = dynamic_pointer_cast<JukeboxTile::Entity>( level->getTileEntity(x, y, z) );
	rte->setRecord(record->copy());
	rte->setChanged();

	level->setData(x, y, z, 1, Tile::UPDATE_CLIENTS);
}

void JukeboxTile::dropRecording(Level *level, int x, int y, int z)
{
	if (level->isClientSide) return;

	shared_ptr<JukeboxTile::Entity> rte = dynamic_pointer_cast<JukeboxTile::Entity>( level->getTileEntity(x, y, z) );
	if( rte == NULL ) return;

	shared_ptr<ItemInstance> oldRecord = rte->getRecord();
	if (oldRecord == NULL) return;


	level->levelEvent(LevelEvent::SOUND_PLAY_RECORDING, x, y, z, 0);
	// 4J-PB- the level event will play the music
	//level->playStreamingMusic(L"", x, y, z);
	rte->setRecord(nullptr);
	rte->setChanged();
	level->setData(x, y, z, 0, Tile::UPDATE_CLIENTS);

	float s = 0.7f;
	double xo = level->random->nextFloat() * s + (1 - s) * 0.5;
	double yo = level->random->nextFloat() * s + (1 - s) * 0.2 + 0.6;
	double zo = level->random->nextFloat() * s + (1 - s) * 0.5;

	shared_ptr<ItemInstance> itemInstance = oldRecord->copy();

	shared_ptr<ItemEntity> item = shared_ptr<ItemEntity>( new ItemEntity(level, x + xo, y + yo, z + zo, itemInstance ) );
	item->throwTime = 10;
	level->addEntity(item);
}

void JukeboxTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	dropRecording(level, x, y, z);
	Tile::onRemove(level, x, y, z, id, data);
}

void JukeboxTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus)
{
	if (level->isClientSide) return;
	Tile::spawnResources(level, x, y, z, data, odds, 0);
}

shared_ptr<TileEntity> JukeboxTile::newTileEntity(Level *level)
{
	return shared_ptr<JukeboxTile::Entity>( new JukeboxTile::Entity() );
}

void JukeboxTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(getIconName() + L"_side");
	iconTop = iconRegister->registerIcon(getIconName() + L"_top");
}

bool JukeboxTile::hasAnalogOutputSignal()
{
	return true;
}

int JukeboxTile::getAnalogOutputSignal(Level *level, int x, int y, int z, int dir)
{
	shared_ptr<ItemInstance> record = dynamic_pointer_cast<JukeboxTile::Entity>( level->getTileEntity(x, y, z))->getRecord();
	return record == NULL ? Redstone::SIGNAL_NONE : record->id + 1 - Item::record_01_Id;
}