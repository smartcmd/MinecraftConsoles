#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "NoteBlockTile.h"
#include "SoundTypes.h"

NoteBlockTile::NoteBlockTile(int id) : BaseEntityTile(id, Material::wood)
{
}

void NoteBlockTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	app.DebugPrintf("-------- Neighbour changed type %d\n", type);
	bool signal = level->hasNeighborSignal(x, y, z);
	shared_ptr<MusicTileEntity> mte = dynamic_pointer_cast<MusicTileEntity>( level->getTileEntity(x, y, z) );
	app.DebugPrintf("-------- Signal is %s, tile is currently %s\n",signal?"TRUE":"FALSE", mte->on?"ON":"OFF");
	if (mte != NULL && mte->on != signal)
	{
		if (signal)
		{
			mte->playNote(level, x, y, z);
		}
		mte->on = signal;
	}
}

// 4J-PB - Adding a TestUse for tooltip display
bool NoteBlockTile::TestUse()
{
	return true;
}

bool NoteBlockTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if (soundOnly) return false;
	if (level->isClientSide) return true;
	shared_ptr<MusicTileEntity> mte = dynamic_pointer_cast<MusicTileEntity>( level->getTileEntity(x, y, z) );
	if (mte != NULL )
	{
		mte->tune();
		mte->playNote(level, x, y, z);
	}
	return true;
}

void NoteBlockTile::attack(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	if (level->isClientSide) return;
	shared_ptr<MusicTileEntity> mte = dynamic_pointer_cast<MusicTileEntity>( level->getTileEntity(x, y, z) );
	if( mte != NULL ) mte->playNote(level, x, y, z);
}

shared_ptr<TileEntity> NoteBlockTile::newTileEntity(Level *level)
{
	return shared_ptr<MusicTileEntity>( new MusicTileEntity() );
}

bool NoteBlockTile::triggerEvent(Level *level, int x, int y, int z, int i, int note)
{
	float pitch = (float) pow(2, (note - 12) / 12.0);

	int iSound;
	switch(i)
	{
	case 1:
		iSound=eSoundType_NOTE_BD;
		break;
	case 2:
		iSound=eSoundType_NOTE_SNARE;
		break;
	case 3:
		iSound=eSoundType_NOTE_HAT;
		break;
	case 4:
		iSound=eSoundType_NOTE_BASSATTACK;
		break;
	default:
		iSound=eSoundType_NOTE_HARP;
		break;
	}
	app.DebugPrintf("NoteBlockTile::triggerEvent - playSound - pitch = %f\n",pitch);
	level->playSound(x + 0.5, y + 0.5, z + 0.5, iSound, 3, pitch);
	level->addParticle(eParticleType_note, x + 0.5, y + 1.2, z + 0.5, note / 24.0, 0, 0);

	return true;
}
