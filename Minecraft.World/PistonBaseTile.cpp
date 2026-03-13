#include "stdafx.h"
#include "PistonBaseTile.h"
#include "PistonMovingPiece.h"
#include "PistonPieceEntity.h"
#include "PistonExtensionTile.h"
#include "Facing.h"
#include "net.minecraft.world.level.h"
#include "../Minecraft.Client/Minecraft.h"
#include "../Minecraft.Client/MultiPlayerLevel.h"
#include "net.minecraft.world.h"
#include "LevelChunk.h"
#include "Dimension.h"

const wstring PistonBaseTile::EDGE_TEX = L"piston_side";
const wstring PistonBaseTile::PLATFORM_TEX = L"piston_top";
const wstring PistonBaseTile::PLATFORM_STICKY_TEX = L"piston_top_sticky";
const wstring PistonBaseTile::BACK_TEX = L"piston_bottom";
const wstring PistonBaseTile::INSIDE_TEX = L"piston_inner_top";

const float PistonBaseTile::PLATFORM_THICKNESS = 4.0f;

DWORD PistonBaseTile::tlsIdx = TlsAlloc();

// 4J - NOTE - this ignoreUpdate stuff has been removed from the java version, but I'm not currently sure how the java version does without it... there must be
// some other mechanism that we don't have that stops the event from one piston being processed, from causing neighbours to have extra events created for them.
// For us, that means that if we create a piston next to another one, then one of them gets two events to createPush, the second of which fails, leaving the
// piston in a bad (simultaneously extended & not extended) state.
// 4J - ignoreUpdate is a static in java, implementing as TLS here to make thread safe
bool PistonBaseTile::ignoreUpdate()
{
	return (TlsGetValue(tlsIdx) != nullptr);
}

void PistonBaseTile::ignoreUpdate(bool set)
{
	TlsSetValue(tlsIdx,(LPVOID)(set?1:0));
}

PistonBaseTile::PistonBaseTile(int id, bool isSticky) : Tile(id, Material::piston, isSolidRender() )
{
	// 4J - added initialiser
	ignoreUpdate(false);

	this->isSticky = isSticky;
	setSoundType(SOUND_STONE);
	setDestroyTime(0.5f);

	iconInside = nullptr;
	iconBack = nullptr;
	iconPlatform = nullptr;
}

Icon *PistonBaseTile::getPlatformTexture()
{
	return iconPlatform;
}

void PistonBaseTile::updateShape(float x0, float y0, float z0, float x1, float y1, float z1)
{
	setShape(x0, y0, z0, x1, y1, z1);
}

Icon *PistonBaseTile::getTexture(int face, int data)
{
	int facing = getFacing(data);

	if (facing > 5)
	{
		return iconPlatform;
	}

	if (face == facing)
	{
		// sorry about this mess...
		// when the piston is extended, either normally
		// or because a piston arm animation, the top
		// texture is the furnace bottom
		ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
		if (isExtended(data) || tls->xx0 > 0 || tls->yy0 > 0 || tls->zz0 > 0 || tls->xx1 < 1 || tls->yy1 < 1 || tls->zz1 < 1)
		{
			return iconInside;
		}
		return iconPlatform;
	}
	if (face == Facing::OPPOSITE_FACING[facing])
	{
		return iconBack;
	}

	return icon;
}

Icon *PistonBaseTile::getTexture(const wstring &name)
{
	if (name.compare(EDGE_TEX) == 0) return Tile::pistonBase->icon;
	if (name.compare(PLATFORM_TEX) == 0) return Tile::pistonBase->iconPlatform;
	if (name.compare(PLATFORM_STICKY_TEX) == 0) return Tile::pistonStickyBase->iconPlatform;
	if (name.compare(INSIDE_TEX) == 0) return Tile::pistonBase->iconInside;

	return nullptr;
}

//@Override
void PistonBaseTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(EDGE_TEX);
	iconPlatform = iconRegister->registerIcon(isSticky ? PLATFORM_STICKY_TEX : PLATFORM_TEX);
	iconInside = iconRegister->registerIcon(INSIDE_TEX);
	iconBack = iconRegister->registerIcon(BACK_TEX);
}

int PistonBaseTile::getRenderShape()
{
	return SHAPE_PISTON_BASE;
}

bool PistonBaseTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool PistonBaseTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	return false;
}

void PistonBaseTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	int targetData = getNewFacing(level, x, y, z, dynamic_pointer_cast<Player>(by) );
	level->setData(x, y, z, targetData, Tile::UPDATE_CLIENTS);
	if (!level->isClientSide && !ignoreUpdate())
	{
		checkIfExtend(level, x, y, z);
	}
}

void PistonBaseTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (!level->isClientSide && !ignoreUpdate())
	{
		checkIfExtend(level, x, y, z);
	}
}

void PistonBaseTile::onPlace(Level *level, int x, int y, int z)
{
	if (!level->isClientSide && level->getTileEntity(x, y, z) == nullptr && !ignoreUpdate())
	{
		checkIfExtend(level, x, y, z);
	}
}

void PistonBaseTile::checkIfExtend(Level *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	int facing = getFacing(data);

	if (facing == UNDEFINED_FACING)
	{
		return;
	}
	bool extend = getNeighborSignal(level, x, y, z, facing);

	if (extend && !isExtended(data))
	{
		if (canPush(level, x, y, z, facing))
		{
			level->tileEvent(x, y, z, id, TRIGGER_EXTEND, facing);
		}
	}
	else if (!extend && isExtended(data))
	{
		level->setData(x, y, z, facing, UPDATE_CLIENTS);
		level->tileEvent(x, y, z, id, TRIGGER_CONTRACT, facing);
	}
}

/**
* This method checks neighbor signals for this block and the block above,
* and directly beneath. However, it avoids checking blocks that would be
* pushed by this block.
* 
* @param level
* @param x
* @param y
* @param z
* @return
*/
bool PistonBaseTile::getNeighborSignal(Level *level, int x, int y, int z, int facing)
{
	// check adjacent neighbors, but not in push direction
	if (facing != Facing::DOWN && level->hasSignal(x, y - 1, z, Facing::DOWN)) return true;
	if (facing != Facing::UP && level->hasSignal(x, y + 1, z, Facing::UP)) return true;
	if (facing != Facing::NORTH && level->hasSignal(x, y, z - 1, Facing::NORTH)) return true;
	if (facing != Facing::SOUTH && level->hasSignal(x, y, z + 1, Facing::SOUTH)) return true;
	if (facing != Facing::EAST && level->hasSignal(x + 1, y, z, Facing::EAST)) return true;
	if (facing != Facing::WEST && level->hasSignal(x - 1, y, z, Facing::WEST)) return true;

	// check signals above
	if (level->hasSignal(x, y, z, 0)) return true;
	if (level->hasSignal(x, y + 2, z, 1)) return true;
	if (level->hasSignal(x, y + 1, z - 1, 2)) return true;
	if (level->hasSignal(x, y + 1, z + 1, 3)) return true;
	if (level->hasSignal(x - 1, y + 1, z, 4)) return true;
	if (level->hasSignal(x + 1, y + 1, z, 5)) return true;

	return false;
}

bool PistonBaseTile::triggerEvent(Level *level, int x, int y, int z, int param1, int facing)
{
	ignoreUpdate(true);

	if (!level->isClientSide)
	{
		bool extend = getNeighborSignal(level, x, y, z, facing);

		if (extend && param1 == TRIGGER_CONTRACT)
		{
			level->setData(x, y, z, facing | EXTENDED_BIT, UPDATE_CLIENTS);
			return false;
		}
		else if (!extend && param1 == TRIGGER_EXTEND)
		{
			return false;
		}
	}

	if (param1 == TRIGGER_EXTEND)
	{
		PIXBeginNamedEvent(0,"Create push\n");
		if (createPush(level, x, y, z, facing))
		{
			// 4J - it is (currently) critical that this setData sends data to the client, so have added a bool to the method so that it sends data even if the data was already set to the same value
			// as before, which was actually its behaviour until a change in 1.0.1 meant that setData only conditionally sent updates to listeners. If the data update Isn't sent, then what
			// can happen is:
			// (1) the host sends the tile event to the client
			// (2) the client gets the tile event, and sets the tile/data value locally.
			// (3) just before setting the tile/data locally, the client will put the old value in the vector of things to be restored should an update not be received back from the host
			// (4) we don't get any update of the tile from the host, and so the old value gets restored on the client
			// (5) the piston base ends up being restored to its retracted state whilst the piston arm is extended
			// We really need to spend some time investigating a better way for pistons to work as it all seems a bit scary how the host/client interact, but forcing this to send should at least
			// restore the behaviour of the pistons to something closer to what they were before the 1.0.1 update. By sending this data update, then (4) in the list above doesn't happen
			// because the client does actually receive an update for this tile from the host after the event has been processed on the cient.
			level->setData(x, y, z, facing | EXTENDED_BIT, Tile::UPDATE_CLIENTS, true);
			level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_TILE_PISTON_OUT, 0.5f, level->random->nextFloat() * 0.25f + 0.6f);
		}
		else
		{
			return false;
		}
		PIXEndNamedEvent();
	}
	else if (param1 == TRIGGER_CONTRACT)
	{
		PIXBeginNamedEvent(0,"Contract phase A\n");
		shared_ptr<TileEntity> prevTileEntity = level->getTileEntity(x + Facing::STEP_X[facing], y + Facing::STEP_Y[facing], z + Facing::STEP_Z[facing]);
		if (prevTileEntity != nullptr && dynamic_pointer_cast<PistonPieceEntity>(prevTileEntity) != nullptr)
		{
			dynamic_pointer_cast<PistonPieceEntity>(prevTileEntity)->finalTick();
		}

		stopSharingIfServer(level, x, y, z);	// 4J added
		level->setTileAndData(x, y, z, Tile::pistonMovingPiece_Id, facing, Tile::UPDATE_ALL);
		level->setTileEntity(x, y, z, PistonMovingPiece::newMovingPieceEntity(id, facing, facing, false, true));

		PIXEndNamedEvent();

		// sticky movement
		if (isSticky)
		{
			PIXBeginNamedEvent(0,"Contract sticky phase A\n");
			int twoX = x + Facing::STEP_X[facing] * 2;
			int twoY = y + Facing::STEP_Y[facing] * 2;
			int twoZ = z + Facing::STEP_Z[facing] * 2;
			int block = level->getTile(twoX, twoY, twoZ);
			int blockData = level->getData(twoX, twoY, twoZ);
			bool pistonPiece = false;

			PIXEndNamedEvent();

			if (block == Tile::pistonMovingPiece_Id)
			{
				PIXBeginNamedEvent(0,"Contract sticky phase B\n");
				// the block two steps away is a moving piston block piece, so replace it with the real data,
				// since it's probably this piston which is changing too fast
				shared_ptr<TileEntity> tileEntity = level->getTileEntity(twoX, twoY, twoZ);
				if (tileEntity != nullptr && dynamic_pointer_cast<PistonPieceEntity>(tileEntity) != nullptr )
				{
					shared_ptr<PistonPieceEntity> ppe = dynamic_pointer_cast<PistonPieceEntity>(tileEntity);

					if (ppe->getFacing() == facing && ppe->isExtending())
					{
						// force the tile to air before pushing
						ppe->finalTick();
						block = ppe->getId();
						blockData = ppe->getData();
						pistonPiece = true;
					}
				}
				PIXEndNamedEvent();
			}

			PIXBeginNamedEvent(0,"Contract sticky phase C\n");
			if (!pistonPiece && block > 0 && (isPushable(block, level, twoX, twoY, twoZ, false))
				&& (Tile::tiles[block]->getPistonPushReaction() == Material::PUSH_NORMAL || block == Tile::pistonBase_Id || block == Tile::pistonStickyBase_Id))
			{
				stopSharingIfServer(level, twoX, twoY, twoZ);	// 4J added

				x += Facing::STEP_X[facing];
				y += Facing::STEP_Y[facing];
				z += Facing::STEP_Z[facing];

				level->setTileAndData(x, y, z, Tile::pistonMovingPiece_Id, blockData, Tile::UPDATE_ALL);
				level->setTileEntity(x, y, z, PistonMovingPiece::newMovingPieceEntity(block, blockData, facing, false, false));

				ignoreUpdate(false);
				level->removeTile(twoX, twoY, twoZ);
				ignoreUpdate(true);
			}
			else if (!pistonPiece)
			{
				stopSharingIfServer(level, x + Facing::STEP_X[facing], y + Facing::STEP_Y[facing], z + Facing::STEP_Z[facing]);	// 4J added
				ignoreUpdate(false);
				level->removeTile(x + Facing::STEP_X[facing], y + Facing::STEP_Y[facing], z + Facing::STEP_Z[facing]);
				ignoreUpdate(true);
			}
			PIXEndNamedEvent();
		}
		else
		{
			stopSharingIfServer(level, x + Facing::STEP_X[facing], y + Facing::STEP_Y[facing], z + Facing::STEP_Z[facing]);	// 4J added
			ignoreUpdate(false);
			level->removeTile(x + Facing::STEP_X[facing], y + Facing::STEP_Y[facing], z + Facing::STEP_Z[facing]);
			ignoreUpdate(true);
		}

		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_TILE_PISTON_IN, 0.5f, level->random->nextFloat() * 0.15f + 0.6f);
	}

	ignoreUpdate(false);

	return true;
}

void PistonBaseTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	int data = (forceData == -1 ) ? level->getData(x, y, z) : forceData;

	if (isExtended(data))
	{
		const float thickness = PLATFORM_THICKNESS / 16.0f;
		switch (getFacing(data))
		{
		case Facing::DOWN:
			setShape(0, thickness, 0, 1, 1, 1);
			break;
		case Facing::UP:
			setShape(0, 0, 0, 1, 1 - thickness, 1);
			break;
		case Facing::NORTH:
			setShape(0, 0, thickness, 1, 1, 1);
			break;
		case Facing::SOUTH:
			setShape(0, 0, 0, 1, 1, 1 - thickness);
			break;
		case Facing::WEST:
			setShape(thickness, 0, 0, 1, 1, 1);
			break;
		case Facing::EAST:
			setShape(0, 0, 0, 1 - thickness, 1, 1);
			break;
		}
	}
	else
	{
		setShape(0, 0, 0, 1, 1, 1);
	}
}

void PistonBaseTile::updateDefaultShape()
{
	setShape(0, 0, 0, 1, 1, 1);
}

void PistonBaseTile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
	setShape(0, 0, 0, 1, 1, 1);
	Tile::addAABBs(level, x, y, z, box, boxes, source);
}

AABB *PistonBaseTile::getAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return Tile::getAABB(level, x, y, z);
}

bool PistonBaseTile::isCubeShaped()
{
	return false;
}

int PistonBaseTile::getFacing(int data)
{
	return data & 0x7;
}

bool PistonBaseTile::isExtended(int data)
{
	return (data & EXTENDED_BIT) != 0;
}

int PistonBaseTile::getNewFacing(Level *level, int x, int y, int z, shared_ptr<LivingEntity> player)
{
	if (Mth::abs(static_cast<float>(player->x) - x) < 2 && Mth::abs(static_cast<float>(player->z) - z) < 2) 
	{
		// If the player is above the block, the slot is on the top
		double py = player->y + 1.82 - player->heightOffset;
		if (py - y > 2)
		{
			return Facing::UP;
		}
		// If the player is below the block, the slot is on the bottom
		if (y - py > 0)
		{
			return Facing::DOWN;
		}
	}
	// The slot is on the side
	int i = Mth::floor(player->yRot * 4.0f / 360.0f + 0.5) & 0x3;
	if (i == 0) return Facing::NORTH;
	if (i == 1) return Facing::EAST;
	if (i == 2) return Facing::SOUTH;
	if (i == 3) return Facing::WEST;
	return 0;
}

bool PistonBaseTile::isPushable(int block, Level *level, int cx, int cy, int cz, bool allowDestroyable)
{
	// special case for obsidian
	if (block == Tile::obsidian_Id)
	{
		return false;
	}

	if (block == Tile::pistonBase_Id || block == Tile::pistonStickyBase_Id)
	{
		// special case for piston bases
		if (isExtended(level->getData(cx, cy, cz)))
		{
			return false;
		}
	}
	else
	{
		if (Tile::tiles[block]->getDestroySpeed(level, cx, cy, cz) == Tile::INDESTRUCTIBLE_DESTROY_TIME)
		{
			return false;
		}

		if (Tile::tiles[block]->getPistonPushReaction() == Material::PUSH_BLOCK)
		{
			return false;
		}

		if (Tile::tiles[block]->getPistonPushReaction() == Material::PUSH_DESTROY)
		{
			if(!allowDestroyable)
			{
				return false;
			}
			return true;
		}
	}

	if( Tile::tiles[block]->isEntityTile() )	// 4J - java uses instanceof EntityTile here
	{
		// may not push tile entities
		return false;
	}

	return true;
}

bool PistonBaseTile::canPush(Level *level, int sx, int sy, int sz, int facing)
{
	int cx = sx + Facing::STEP_X[facing];
	int cy = sy + Facing::STEP_Y[facing];
	int cz = sz + Facing::STEP_Z[facing];

	for (int i = 0; i < MAX_PUSH_DEPTH + 1; i++)
	{

		if (cy <= 0 || cy >= (Level::maxBuildHeight - 1))
		{
			// out of bounds
			return false;
		}

		// 4J - added to also check for out of bounds in x/z for our finite world
		int minXZ = - (level->dimension->getXZSize() * 16 ) / 2;
		int maxXZ = (level->dimension->getXZSize() * 16 ) / 2 - 1;
		if( ( cx <= minXZ ) || ( cx >= maxXZ ) || ( cz <= minXZ ) || ( cz >= maxXZ ) )
		{
			return false;
		}
		int block = level->getTile(cx, cy, cz);
		if (block == 0)
		{
			break;
		}

		if (!isPushable(block, level, cx, cy, cz, true))
		{
			return false;
		}

		if (Tile::tiles[block]->getPistonPushReaction() == Material::PUSH_DESTROY)
		{
			break;
		}

		if (i == MAX_PUSH_DEPTH)
		{
			// we've reached the maximum push depth
			// without finding air or a breakable block
			return false;
		}

		cx += Facing::STEP_X[facing];
		cy += Facing::STEP_Y[facing];
		cz += Facing::STEP_Z[facing];
	}

	return true;

}

void PistonBaseTile::stopSharingIfServer(Level *level, int x, int y, int z)
{
	if( !level->isClientSide )
	{ 
		MultiPlayerLevel *clientLevel = Minecraft::GetInstance()->getLevel(level->dimension->id);
		if( clientLevel )
		{
			LevelChunk *lc = clientLevel->getChunkAt( x, z );
			lc->stopSharingTilesAndData();
		}
	}
}

bool PistonBaseTile::createPush(Level *level, int sx, int sy, int sz, int facing)
{
	int cx = sx + Facing::STEP_X[facing];
	int cy = sy + Facing::STEP_Y[facing];
	int cz = sz + Facing::STEP_Z[facing];

	for (int i = 0; i < MAX_PUSH_DEPTH + 1; i++)
	{
		if (cy <= 0 || cy >= (Level::maxBuildHeight - 1))
		{
			// out of bounds
			return false;
		}

		// 4J - added to also check for out of bounds in x/z for our finite world
		int minXZ = - (level->dimension->getXZSize() * 16 ) / 2;
		int maxXZ = (level->dimension->getXZSize() * 16 ) / 2 - 1;
		if( ( cx <= minXZ ) || ( cx >= maxXZ ) || ( cz <= minXZ ) || ( cz >= maxXZ ) )
		{
			return false;
		}

		int block = level->getTile(cx, cy, cz);
		if (block == 0)
		{
			break;
		}

		if (!isPushable(block, level, cx, cy, cz, true))
		{
			return false;
		}

		if (Tile::tiles[block]->getPistonPushReaction() == Material::PUSH_DESTROY)
		{
			// this block is destroyed when pushed
			Tile::tiles[block]->spawnResources(level, cx, cy, cz, level->getData(cx, cy, cz), 0);
			// setting the tile to air is actually superflous, but helps vs multiplayer problems
			stopSharingIfServer(level, cx, cy, cz);	// 4J added
			level->removeTile(cx, cy, cz);
			break;
		}

		if (i == MAX_PUSH_DEPTH)
		{
			// we've reached the maximum push depth without finding air or a breakable block
			return false;
		}

		cx += Facing::STEP_X[facing];
		cy += Facing::STEP_Y[facing];
		cz += Facing::STEP_Z[facing];
	}

	int ex = cx;
	int ey = cy;
	int ez = cz;
	int count = 0;
	int tiles[MAX_PUSH_DEPTH + 1];

	while (cx != sx || cy != sy || cz != sz)
	{

		int nx = cx - Facing::STEP_X[facing];
		int ny = cy - Facing::STEP_Y[facing];
		int nz = cz - Facing::STEP_Z[facing];

		int block = level->getTile(nx, ny, nz);
		int data = level->getData(nx, ny, nz);

		stopSharingIfServer(level, cx, cy, cz);	// 4J added

		if (block == id && nx == sx && ny == sy && nz == sz)
		{
			level->setTileAndData(cx, cy, cz, Tile::pistonMovingPiece_Id, facing | (isSticky ? PistonExtensionTile::STICKY_BIT : 0), Tile::UPDATE_NONE);
			level->setTileEntity(cx, cy, cz, PistonMovingPiece::newMovingPieceEntity(Tile::pistonExtensionPiece_Id, facing | (isSticky ? PistonExtensionTile::STICKY_BIT : 0), facing, true, false));
		}
		else
		{
			level->setTileAndData(cx, cy, cz, Tile::pistonMovingPiece_Id, data, Tile::UPDATE_NONE);
			level->setTileEntity(cx, cy, cz, PistonMovingPiece::newMovingPieceEntity(block, data, facing, true, false));
		}
		tiles[count++] = block;

		cx = nx;
		cy = ny;
		cz = nz;
	}

	cx = ex;
	cy = ey;
	cz = ez;
	count = 0;

	while (cx != sx || cy != sy || cz != sz)
	{
		int nx = cx - Facing::STEP_X[facing];
		int ny = cy - Facing::STEP_Y[facing];
		int nz = cz - Facing::STEP_Z[facing];

		level->updateNeighborsAt(nx, ny, nz, tiles[count++]);

		cx = nx;
		cy = ny;
		cz = nz;
	}

	return true;

}
