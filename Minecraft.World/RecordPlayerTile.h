#pragma once

#include "EntityTile.h"
#include "CompoundTag.h"
#include "TileEntity.h"

class CompoundTag;
class ChunkRebuildData;

class RecordPlayerTile : public EntityTile
{
	friend class Tile;
	friend class ChunkRebuildData;
public:
	class Entity : public TileEntity
	{
public:
	eINSTANCEOF GetType() { return eTYPE_RECORDPLAYERTILE; }
	static TileEntity *create() { return new RecordPlayerTile::Entity(); }

	public:
		int record;
		Entity() : TileEntity(), record( 0 ) {}

        virtual void load(CompoundTag *tag)
		{
            TileEntity::load(tag);
            record = tag->getInt(L"Record");
        }

         virtual void save(CompoundTag *tag)
		 {
            TileEntity::save(tag);
            if (record > 0) tag->putInt(L"Record", record);
			// return true;	// 4J - TODO, in java there is no return type here but we can't change our derived class member function to not have one - investigate
			// 4J Jev, took out the return statement, TileEntity::save is now virtual. Think this fixes above.
        }

		 // 4J Added
		 shared_ptr<TileEntity> clone()
		 {
			 shared_ptr<RecordPlayerTile::Entity> result = shared_ptr<RecordPlayerTile::Entity>( new RecordPlayerTile::Entity() );
			 TileEntity::clone(result);

			 result->record = record;

			 return result;
		 }
    };

private:
	Icon *iconTop;

protected:
	RecordPlayerTile(int id);

public:
	virtual Icon *getTexture(int face, int data);
	virtual bool TestUse(Level *level, int x, int y, int z, shared_ptr<Player> player);
    virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
    void setRecord(Level *level, int x, int y, int z, int record);
    void dropRecording(Level *level, int x, int y, int z);
    virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
    virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus);

	virtual shared_ptr<TileEntity> newTileEntity(Level *level);
	void registerIcons(IconRegister *iconRegister);
};
