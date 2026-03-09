#include "stdafx.h"

#include "TileRenderer_SPU.h"
#include "LiquidTile_SPU.h"
#include "FireTile_SPU.h"
#include "RailTile_SPU.h"
#include "StemTile_SPU.h"
#include "VineTile_SPU.h"
#include "WaterLilyTile_SPU.h"
#include "BrewingStandTile_SPU.h"
#include "RedStoneDustTile_SPU.h"
#include "DiodeTile_SPU.h"
#include "FenceGateTile_SPU.h"
#include "StairTile_SPU.h"
#include "EggTile_SPU.h"
#include "DoorTile_SPU.h"
#include "FenceTile_SPU.h"
#include "GrassTile_SPU.h"
#include "TreeTile_SPU.h"
#include "WallTile_SPU.h"
#include "QuartzBlockTile_SPU.h"

// #include <string>
#include <math.h>
#include <assert.h>

#ifdef SN_TARGET_PS3_SPU
#include "..\Common\spu_assert.h"
#endif 

static const float MATH_PI = 3.141592654f;

// #include "GameRenderer.h"
// #include "Minecraft.h"
// #include "Textures.h"
// #include "..\Minecraft.World\net.minecraft.world.level.h"
//  #include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.h"
// #include "..\Minecraft.World\net.minecraft.world.level.material.h"
// #include "..\Minecraft.World\net.minecraft.h"
// #include "..\Minecraft.World\net.minecraft.world.h"
//  #include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.h"

#include "Tesselator_SPU.h"

#ifdef SN_TARGET_PS3_SPU
#include "Stubs_SPU.h"
#endif // SN_TARGET_PS3_SPU

// #include "..\..\Minecraft.World\SharedConstants.h"
#include "Facing_SPU.h"

// #include "EntityTileRenderer.h"
// #include "Options.h"

// #define DISABLE_TESS_FUNCS
#ifdef SN_TARGET_PS3_SPU
class SharedConstants
{
public:
	static const bool TEXTURE_LIGHTING = true;
	static const int WORLD_RESOLUTION = 16;
};
#endif

// #define SN_TARGET_PS3_SPU

bool TileRenderer_SPU::fancy = true;
bool g_ambientOcclusionMax = false; // minecraft->options->ambientOcclusion >= Options::AO_MAX
const float smallUV = ( 1.0f / 16.0f );


void TileRenderer_SPU::_init()
{
	fixedTexture = nullptr;
	xFlipTexture = false;
	noCulling = false;
	blsmooth = 1;
	applyAmbienceOcclusion = false;
	setColor = true;
	northFlip = FLIP_NONE;
	southFlip = FLIP_NONE;
	eastFlip = FLIP_NONE;
	westFlip = FLIP_NONE;
	upFlip = FLIP_NONE;
	downFlip = FLIP_NONE;

	tileShapeX0 = 0.0;
	tileShapeX1 = 0.0;
	tileShapeY0 = 0.0;
	tileShapeY1 = 0.0;
	tileShapeZ0 = 0.0;
	tileShapeZ1 = 0.0;
	fixedShape = false;
	smoothShapeLighting = false;
//	minecraft = Minecraft::GetInstance();
}

TileRenderer_SPU::TileRenderer_SPU( ChunkRebuildData* level )
{
	this->level = level;
	_init();
}

TileRenderer_SPU::TileRenderer_SPU()
{
	this->level = nullptr;
	_init();
}

Tesselator_SPU* TileRenderer_SPU::getTesselator()
{
	Tesselator_SPU* t = &level->m_tesselator;
	return t;
}



void TileRenderer_SPU::setFixedTexture( Icon_SPU *fixedTexture )
{
	this->fixedTexture = fixedTexture;
}

void TileRenderer_SPU::clearFixedTexture()
{
	this->fixedTexture = nullptr;
}

bool TileRenderer_SPU::hasFixedTexture()
{
	return fixedTexture != nullptr;
}

void TileRenderer_SPU::setShape(float x0, float y0, float z0, float x1, float y1, float z1)
{
	if (!fixedShape)
	{
		tileShapeX0 = x0;
		tileShapeX1 = x1;
		tileShapeY0 = y0;
		tileShapeY1 = y1;
		tileShapeZ0 = z0;
		tileShapeZ1 = z1;
		smoothShapeLighting = (tileShapeX0 > 0 || tileShapeX1 < 1 || tileShapeY0 > 0 || tileShapeY1 < 1 || tileShapeZ0 > 0 || tileShapeZ1 < 1);
	}
}

void TileRenderer_SPU::setShape(Tile_SPU *tt)
{
	if (!fixedShape)
	{
		tileShapeX0 = tt->getShapeX0();
		tileShapeX1 = tt->getShapeX1();
		tileShapeY0 = tt->getShapeY0();
		tileShapeY1 = tt->getShapeY1();
		tileShapeZ0 = tt->getShapeZ0();
		tileShapeZ1 = tt->getShapeZ1();
		smoothShapeLighting = (tileShapeX0 > 0 || tileShapeX1 < 1 || tileShapeY0 > 0 || tileShapeY1 < 1 || tileShapeZ0 > 0 || tileShapeZ1 < 1);
	}
}

void TileRenderer_SPU::setFixedShape(float x0, float y0, float z0, float x1, float y1, float z1)
{
	tileShapeX0 = x0;
	tileShapeX1 = x1;
	tileShapeY0 = y0;
	tileShapeY1 = y1;
	tileShapeZ0 = z0;
	tileShapeZ1 = z1;
	fixedShape = true;

	smoothShapeLighting = (tileShapeX0 > 0 || tileShapeX1 < 1 || tileShapeY0 > 0 || tileShapeY1 < 1 || tileShapeZ0 > 0 || tileShapeZ1 < 1);
}

void TileRenderer_SPU::clearFixedShape()
{
	fixedShape = false;
}

void TileRenderer_SPU::tesselateInWorldFixedTexture( Tile_SPU* tile, int x, int y, int z, Icon_SPU *fixedTexture )	// 4J renamed to differentiate from tesselateInWorld
{
	this->setFixedTexture(fixedTexture);
	tesselateInWorld( tile, x, y, z );
	this->clearFixedTexture();
}

void TileRenderer_SPU::tesselateInWorldNoCulling( Tile_SPU* tile, int x, int y, int z, int forceData,
											  TileEntity* forceEntity )	// 4J added forceData, forceEntity param
{
	noCulling = true;
	tesselateInWorld( tile, x, y, z, forceData );
	noCulling = false;
}

bool TileRenderer_SPU::hasRenderer(Tile_SPU* tt)
{
	int	shape = tt->getRenderShape();
	bool retVal = false;
	switch(shape)
	{
	case Tile_SPU::SHAPE_BLOCK:
	case Tile_SPU::SHAPE_WATER:
	case Tile_SPU::SHAPE_CACTUS:
	case Tile_SPU::SHAPE_STEM:
	case Tile_SPU::SHAPE_LILYPAD:
	case Tile_SPU::SHAPE_ROWS:
	case Tile_SPU::SHAPE_TORCH:
	case Tile_SPU::SHAPE_FIRE:
	case Tile_SPU::SHAPE_LADDER:
	case Tile_SPU::SHAPE_DOOR:
	case Tile_SPU::SHAPE_RAIL:
	case Tile_SPU::SHAPE_EGG:
	case Tile_SPU::SHAPE_VINE:
	case Tile_SPU::SHAPE_BREWING_STAND:
	case Tile_SPU::SHAPE_CROSS_TEXTURE:
	case Tile_SPU::SHAPE_FENCE:
		retVal = true;
		break;

	case Tile_SPU::SHAPE_FENCE_GATE:
	case Tile_SPU::SHAPE_RED_DUST:
	case Tile_SPU::SHAPE_STAIRS:
	case Tile_SPU::SHAPE_DIODE:
	case Tile_SPU::SHAPE_LEVER:
	case Tile_SPU::SHAPE_BED:
	case Tile_SPU::SHAPE_PISTON_BASE:
	case Tile_SPU::SHAPE_PISTON_EXTENSION:
	case Tile_SPU::SHAPE_IRON_FENCE:
	case Tile_SPU::SHAPE_CAULDRON:
	case Tile_SPU::SHAPE_PORTAL_FRAME:
		retVal = false;
		break;
	}

	return retVal;
}


bool TileRenderer_SPU::tesselateInWorld( Tile_SPU* tt, int x, int y, int z, int forceData,
									 TileEntity* forceEntity )	// 4J added forceData, forceEntity param
{
	Tesselator_SPU* t = getTesselator();

	int	shape = tt->getRenderShape();
	tt->updateShape( level, x, y, z, forceData, forceEntity );
	setShape(tt);
	t->setMipmapEnable( level->m_tileData.mipmapEnable[tt->id] );	// 4J added

	bool retVal = false;
	switch(shape)
	{
	case Tile_SPU::SHAPE_BLOCK:
		retVal = tesselateBlockInWorld( tt, x, y, z );
		break;
	case Tile_SPU::SHAPE_TREE:
		retVal = tesselateTreeInWorld(tt, x, y, z);
		break;
	case Tile_SPU::SHAPE_QUARTZ:
		retVal = tesselateQuartzInWorld(tt, x, y, z);
		break;
	case Tile_SPU::SHAPE_WATER:
		retVal = tesselateWaterInWorld( tt, x, y, z );
		break;
	case Tile_SPU::SHAPE_CACTUS:
		retVal = tesselateCactusInWorld( tt, x, y, z );
		break;
	case Tile_SPU::SHAPE_CROSS_TEXTURE:
		retVal = tesselateCrossInWorld( tt, x, y, z );
		break;
	case Tile_SPU::SHAPE_STEM:
		retVal = tesselateStemInWorld( tt, x, y, z );
		break;
	case Tile_SPU::SHAPE_LILYPAD:
		retVal = tesselateLilypadInWorld( static_cast<WaterlilyTile_SPU *>(tt), x, y, z );
		break;
	case Tile_SPU::SHAPE_ROWS:
		retVal = tesselateRowInWorld( tt, x, y, z );
		break;
	case Tile_SPU::SHAPE_TORCH:
		retVal = tesselateTorchInWorld( tt, x, y, z );
		break;
	case Tile_SPU::SHAPE_FIRE:
		retVal = tesselateFireInWorld( static_cast<FireTile_SPU *>(tt), x, y, z );
		break;
	case Tile_SPU::SHAPE_RED_DUST:
		retVal = tesselateDustInWorld( tt, x, y, z );
		break;
	case Tile_SPU::SHAPE_LADDER:
		retVal = tesselateLadderInWorld( tt, x, y, z );
		break;
	case Tile_SPU::SHAPE_DOOR:
		retVal = tesselateDoorInWorld( tt, x, y, z );
		break;
	case Tile_SPU::SHAPE_RAIL:
		retVal = tesselateRailInWorld( static_cast<RailTile_SPU *>(tt), x, y, z );
		break;
	case Tile_SPU::SHAPE_STAIRS:
		retVal = tesselateStairsInWorld( static_cast<StairTile_SPU *>(tt), x, y, z );
		break;
	case Tile_SPU::SHAPE_EGG:
		retVal = tesselateEggInWorld(static_cast<EggTile_SPU *>(tt), x, y, z);
		break;
	case Tile_SPU::SHAPE_FENCE:
		retVal = tesselateFenceInWorld( static_cast<FenceTile_SPU *>(tt), x, y, z );
		break;
	case Tile_SPU::SHAPE_WALL:
		retVal = tesselateWallInWorld( static_cast<WallTile_SPU *>(tt), x, y, z);
		break;
	case Tile_SPU::SHAPE_LEVER:
		retVal = tesselateLeverInWorld( tt, x, y, z );
		break;
	case Tile_SPU::SHAPE_TRIPWIRE_SOURCE:
		retVal = tesselateTripwireSourceInWorld(tt, x, y, z);
		break;
	case Tile_SPU::SHAPE_TRIPWIRE:
		retVal = tesselateTripwireInWorld(tt, x, y, z);
		break;
	case Tile_SPU::SHAPE_BED:
		retVal = tesselateBedInWorld( tt, x, y, z );
		break;
	case Tile_SPU::SHAPE_DIODE:
		retVal = tesselateDiodeInWorld( static_cast<DiodeTile_SPU *>(tt), x, y, z );
		break;
	case Tile_SPU::SHAPE_PISTON_BASE:
		retVal = tesselatePistonBaseInWorld( tt, x, y, z, false, forceData );
		break;
	case Tile_SPU::SHAPE_PISTON_EXTENSION:
		retVal = tesselatePistonExtensionInWorld( tt, x, y, z, true, forceData );
		break;
	case Tile_SPU::SHAPE_IRON_FENCE:
		retVal = tesselateThinFenceInWorld( ( ThinFenceTile* )tt, x, y, z );
		break;
	case Tile_SPU::SHAPE_VINE:
		retVal = tesselateVineInWorld( tt, x, y, z );
		break;
	case Tile_SPU::SHAPE_FENCE_GATE:
		retVal = tesselateFenceGateInWorld( static_cast<FenceGateTile_SPU *>(tt), x, y, z );
		break;
	case Tile_SPU::SHAPE_CAULDRON:
		retVal = tesselateCauldronInWorld((CauldronTile_SPU* ) tt, x, y, z);
		break;
	case Tile_SPU::SHAPE_FLOWER_POT:
		retVal = tesselateFlowerPotInWorld((FlowerPotTile_SPU *) tt, x, y, z);
		break;
	case Tile_SPU::SHAPE_ANVIL:
		retVal = tesselateAnvilInWorld((AnvilTile_SPU *) tt, x, y, z);
		break;
	case Tile_SPU::SHAPE_BREWING_STAND:
		retVal = tesselateBrewingStandInWorld(static_cast<BrewingStandTile_SPU *>(tt), x, y, z);
		break;
	case Tile_SPU::SHAPE_PORTAL_FRAME:
		retVal = tesselateAirPortalFrameInWorld((TheEndPortalFrameTile *)tt, x, y, z);
		break;
	case Tile_SPU::SHAPE_COCOA:
		retVal = tesselateCocoaInWorld((CocoaTile_SPU *) tt, x, y, z);
		break;

	};


	t->setMipmapEnable( true );	// 4J added
	return retVal;

}

bool TileRenderer_SPU::tesselateAirPortalFrameInWorld(TheEndPortalFrameTile *tt, int x, int y, int z)
{
#ifdef DISABLE_TESS_FUNCS
    int data = level->getData(x, y, z);

    int direction = data & 3;
    if (direction == Direction::SOUTH)
	{
        upFlip = FLIP_180;
    }
	else if (direction == Direction::EAST)
	{
        upFlip = FLIP_CW;
    }
	else if (direction == Direction::WEST)
	{
        upFlip = FLIP_CCW;
    }

    if (!TheEndPortalFrameTile::hasEye(data))
	{
//		EnterCriticalSection( &Tile_SPU::m_csShape );
        setShape(0, 0, 0, 1, 13.0f / 16.0f, 1);
        tesselateBlockInWorld(tt, x, y, z);
//		LeaveCriticalSection( &Tile_SPU::m_csShape );

        upFlip = FLIP_NONE;
        return true;
    }

//	EnterCriticalSection( &Tile_SPU::m_csShape );
	noCulling = true;
    setShape(0, 0, 0, 1, 13.0f / 16.0f, 1);
    tesselateBlockInWorld(tt, x, y, z);
    setFixedTexture(tt->getEye());
    setShape(4.0f / 16.0f, 13.0f / 16.0f, 4.0f / 16.0f, 12.0f / 16.0f, 1, 12.0f / 16.0f);
    tesselateBlockInWorld(tt, x, y, z);
	noCulling = false;
    clearFixedTexture();
//	LeaveCriticalSection( &Tile_SPU::m_csShape );

    upFlip = FLIP_NONE;
#endif // DISABLE_TESS_FUNCS
    return true;
}

bool TileRenderer_SPU::tesselateBedInWorld( Tile_SPU* tt, int x, int y, int z )
{
#ifdef DISABLE_TESS_FUNCS
	Tesselator_SPU* t = getTesselator();

	int			data = level->getData( x, y, z );
	int			direction = BedTile::getDirection( data );
	bool		isHead = BedTile::isHeadPiece( data );

	float		c10 = 0.5f;
	float		c11 = 1.0f;
	float		c2 = 0.8f;
	float		c3 = 0.6f;

	float		r11 = c11;
	float		g11 = c11;
	float		b11 = c11;

	float		r10 = c10;
	float		r2 = c2;
	float		r3 = c3;

	float		g10 = c10;
	float		g2 = c2;
	float		g3 = c3;

	float		b10 = c10;
	float		b2 = c2;
	float		b3 = c3;

	// 4J - change brought forward from 1.8.2
	int			centerColor;
	float		centerBrightness;
	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		centerColor = tt->getLightColor( level, x, y, z );
	}
	else
	{
		centerBrightness = tt->getBrightness( level, x, y, z );
	}

	// render wooden underside
	{
		// 4J - change brought forward from 1.8.2
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( centerColor );
			t->color( r10, g10, b10 );
		}
		else
		{
			t->color( r10 * centerBrightness, g10 * centerBrightness, b10 * centerBrightness );
		}

		Icon_SPU *tex = getTexture( tt,  level, x, y, z, Facing::DOWN );

		float u0 = tex->getU0();
		float u1 = tex->getU1();
		float v0 = tex->getV0();
		float v1 = tex->getV1();

		float x0 = x + tileShapeX0;
		float x1 = x + tileShapeX1;
		float y0 = y + tileShapeY0 + 3.0 / 16.0;
		float z0 = z + tileShapeZ0;
		float z1 = z + tileShapeZ1;

		t->vertexUV(  x0 , y0 , z1 ,  u0 , v1  );
		t->vertexUV(  x0 , y0 , z0 ,  u0 , v0  );
		t->vertexUV(  x1 , y0 , z0 ,  u1 , v0  );
		t->vertexUV(  x1 , y0 , z1 ,  u1 , v1  );
	}

	// render bed top
	// 4J - change brought forward from 1.8.2
	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tt->getLightColor( level, x, y + 1, z ) );
		t->color( r11, g11, b11 );
	}
	else
	{
		float brightness = tt->getBrightness( level, x, y + 1, z );
		t->color( r11 * brightness, g11 * brightness, b11 * brightness );
	}

	Icon_SPU *tex = getTexture( tt, level, x, y, z, Facing::UP );

	float u0 = tex->getU0();
	float u1 = tex->getU1();
	float v0 = tex->getV0();
	float v1 = tex->getV1();

	float	topLeftU = u0;
	float	topRightU = u1;
	float	topLeftV = v0;
	float	topRightV = v0;
	float	bottomLeftU = u0;
	float	bottomRightU = u1;
	float	bottomLeftV = v1;
	float	bottomRightV = v1;

	if ( direction == Direction::SOUTH )
	{
		// rotate 90 degrees clockwise
		topRightU = u0;
		topLeftV = v1;
		bottomLeftU = u1;
		bottomRightV = v0;
	}
	else if ( direction == Direction::NORTH )
	{
		// rotate 90 degrees counter-clockwise
		topLeftU = u1;
		topRightV = v1;
		bottomRightU = u0;
		bottomLeftV = v0;
	}
	else if ( direction == Direction::EAST )
	{
		// rotate 180 degrees
		topLeftU = u1;
		topRightV = v1;
		bottomRightU = u0;
		bottomLeftV = v0;
		topRightU = u0;
		topLeftV = v1;
		bottomLeftU = u1;
		bottomRightV = v0;
	}

	float x0 = x + tileShapeX0;
	float x1 = x + tileShapeX1;
	float y1 = y + tileShapeY1;
	float z0 = z + tileShapeZ0;
	float z1 = z + tileShapeZ1;

	t->vertexUV( x1 , y1 , z1 , bottomLeftU ,bottomLeftV  );
	t->vertexUV( x1 , y1 , z0 , topLeftU , topLeftV  );
	t->vertexUV( x0 , y1 , z0 , topRightU ,topRightV  );
	t->vertexUV( x0 , y1 , z1 , bottomRightU ,bottomRightV  );

	// determine which edge to skip (the one between foot and head piece)
	int			skipEdge = Direction::DIRECTION_FACING[direction];
	if ( isHead )
	{
		skipEdge = Direction::DIRECTION_FACING[Direction::DIRECTION_OPPOSITE[direction]];
	}
	// and which edge to x-flip
	int			flipEdge = Facing::WEST;
	switch ( direction )
	{
		case Direction::NORTH:
			break;
		case Direction::SOUTH:
			flipEdge = Facing::EAST;
			break;
		case Direction::EAST:
			flipEdge = Facing::NORTH;
			break;
		case Direction::WEST:
			flipEdge = Facing::SOUTH;
			break;
	}

	if ( ( skipEdge != Facing::NORTH ) && ( noCulling || tt->shouldRenderFace( level, x, y, z - 1, Facing::NORTH ) ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeZ0 > 0 ? centerColor : tt->getLightColor( level, x, y, z - 1 ) );
			t->color( r2, g2, b2 );
		}
		else
		{
			float br = tt->getBrightness( level, x, y, z - 1 );
			if ( tileShapeZ0 > 0 ) br = centerBrightness;
			t->color( r2 * br, g2 * br, b2 * br );
		}
		xFlipTexture = flipEdge == Facing::NORTH;
		renderNorth( tt, x, y, z, getTexture( tt, level, x, y, z, 2 ) );
	}

	if ( ( skipEdge != Facing::SOUTH ) && ( noCulling || tt->shouldRenderFace( level, x, y, z + 1, Facing::SOUTH ) ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeZ1 < 1 ? centerColor : tt->getLightColor( level, x, y, z + 1 ) );
			t->color( r2, g2, b2 );
		}
		else
		{
			float br = tt->getBrightness( level, x, y, z + 1 );
			if ( tileShapeZ1 < 1 ) br = centerBrightness;
			t->color( r2 * br, g2 * br, b2 * br );
		}

		xFlipTexture = flipEdge == Facing::SOUTH;
		renderSouth( tt, x, y, z, getTexture( tt, level, x, y, z, 3 ) );
	}

	if ( ( skipEdge != Facing::WEST ) && ( noCulling || tt->shouldRenderFace( level, x - 1, y, z, Facing::WEST ) ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeZ0 > 0 ? centerColor : tt->getLightColor( level, x - 1, y, z ) );
			t->color( r3, g3, b3 );
		}
		else
		{
			float br = tt->getBrightness( level, x - 1, y, z );
			if ( tileShapeX0 > 0 ) br = centerBrightness;
			t->color( r3 * br, g3 * br, b3 * br );
		}
		xFlipTexture = flipEdge == Facing::WEST;
		renderWest( tt, x, y, z, getTexture( tt, level, x, y, z, 4 ) );
	}

	if ( ( skipEdge != Facing::EAST ) && ( noCulling || tt->shouldRenderFace( level, x + 1, y, z, Facing::EAST ) ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeZ1 < 1 ? centerColor : tt->getLightColor( level, x + 1, y, z ) );
			t->color( r3, g3, b3 );
		}
		else
		{
			float br = tt->getBrightness( level, x + 1, y, z );
			if ( tileShapeX1 < 1 ) br = centerBrightness;
			t->color( r3 * br, g3 * br, b3 * br );
		}
		xFlipTexture = flipEdge == Facing::EAST;
		renderEast( tt, x, y, z, getTexture( tt, level, x, y, z, 5 ) );
	}
	xFlipTexture = false;
#endif // DISABLE_TESS_FUNCS

	return true;

}

bool TileRenderer_SPU::tesselateBrewingStandInWorld(BrewingStandTile_SPU *tt, int x, int y, int z)
{

//	EnterCriticalSection( &Tile_SPU::m_csShape );
    // bounding box first
    setShape(7.0f / 16.0f, 0.0f, 7.0f / 16.0f, 9.0f / 16.0f, 14.0f / 16.0f, 9.0f / 16.0f);
    tesselateBlockInWorld(tt, x, y, z);

    setFixedTexture(tt->getBaseTexture());
    setShape(9.0f / 16.0f, 0.0f, 5.0f / 16.0f, 15.0f / 16.0f, 2 / 16.0f, 11.0f / 16.0f);
    tesselateBlockInWorld(tt, x, y, z);
    setShape(2.0f / 16.0f, 0.0f, 1.0f / 16.0f, 8.0f / 16.0f, 2 / 16.0f, 7.0f / 16.0f);
    tesselateBlockInWorld(tt, x, y, z);
    setShape(2.0f / 16.0f, 0.0f, 9.0f / 16.0f, 8.0f / 16.0f, 2 / 16.0f, 15.0f / 16.0f);
    tesselateBlockInWorld(tt, x, y, z);

    clearFixedTexture();

	Tesselator_SPU* t = getTesselator();

    float br;
    if (SharedConstants::TEXTURE_LIGHTING)
	{
        t->tex2(tt->getLightColor(level, x, y, z));
        br = 1;
    }
	else
	{
        br = tt->getBrightness(level, x, y, z);
    }
    int col = tt->getColor(level, x, y, z);
    float r = ((col >> 16) & 0xff) / 255.0f;
    float g = ((col >> 8) & 0xff) / 255.0f;
    float b = ((col) & 0xff) / 255.0f;

    t->color(br * r, br * g, br * b);

	Icon_SPU *tex = getTexture(tt, 0, 0);

	if (hasFixedTexture()) tex = fixedTexture;
	float v0 = tex->getV0();
	float v1 = tex->getV1();

    int data = level->getData(x, y, z);

    for (int arm = 0; arm < 3; arm++)
	{

        float angle = arm * MATH_PI * 2.0f / 3.0f + MATH_PI * 0.5f;

		float u0 = tex->getU(8);
		float u1 = tex->getU1();
//            if (brewEntity != null && brewEntity.getItem(arm) != null) {
        if ((data & (1 << arm)) != 0)
		{
            u1 = tex->getU0();
        }

		float x0 = x + 8.0f / 16.0f;
		float x1 = x + 8.0f / 16.0f + sin(angle) * 8.0f / 16.0f;
		float z0 = z + 8.0f / 16.0f;
		float z1 = z + 8.0f / 16.0f + cos(angle) * 8.0f / 16.0f;

        t->vertexUV(x0, y + 1.0f, z0, u0, v0);
        t->vertexUV(x0, y + 0.0f, z0, u0, v1);
        t->vertexUV(x1, y + 0.0f, z1, u1, v1);
        t->vertexUV(x1, y + 1.0f, z1, u1, v0);

        t->vertexUV(x1, y + 1.0f, z1, u1, v0);
        t->vertexUV(x1, y + 0.0f, z1, u1, v1);
        t->vertexUV(x0, y + 0.0f, z0, u0, v1);
        t->vertexUV(x0, y + 1.0f, z0, u0, v0);
    }

    tt->updateDefaultShape();

//	LeaveCriticalSection( &Tile_SPU::m_csShape );

    return true;
}

bool TileRenderer_SPU::tesselateCauldronInWorld(CauldronTile_SPU *tt, int x, int y, int z)
{
#ifdef DISABLE_TESS_FUNCS
    // bounding box first
    tesselateBlockInWorld(tt, x, y, z);

	Tesselator_SPU* t = getTesselator();

    float br;
    if (SharedConstants::TEXTURE_LIGHTING)
	{
        t->tex2(tt->getLightColor(level, x, y, z));
        br = 1;
    }
	else
	{
        br = tt->getBrightness(level, x, y, z);
    }
    int col = tt->getColor(level, x, y, z);
    float r = ((col >> 16) & 0xff) / 255.0f;
    float g = ((col >> 8) & 0xff) / 255.0f;
    float b = ((col) & 0xff) / 255.0f;

    t->color(br * r, br * g, br * b);

    // render inside
    Icon_SPU *insideTex = tt->getTexture(Facing::NORTH);
    const float cWidth = ( 2.0f / 16.0f ) - ( 1.0f / 128.0f );		// 4J - Moved by 1/128th (smallest movement possible with our vertex storage) to remove gap at edge of cauldron
    renderEast(tt, x - 1.0f + cWidth, y, z, insideTex);
    renderWest(tt, x + 1.0f - cWidth, y, z, insideTex);
    renderSouth(tt, x, y, z - 1.0f + cWidth, insideTex);
    renderNorth(tt, x, y, z + 1.0f - cWidth, insideTex);

    Icon_SPU *bottomTex = CauldronTile::getTexture(CauldronTile::TEXTURE_INSIDE);
    renderFaceUp(tt, x, y - 1.0f + 4.0f / 16.0f, z, bottomTex);
    renderFaceDown(tt, x, y + 1.0f - 12.0f / 16.0f, z, bottomTex);

    int waterLevel = level->getData(x, y, z);
    if (waterLevel > 0)
	{
        Icon_SPU *liquidTex = LiquidTile_SPU::getTexture(LiquidTile_SPU::TEXTURE_WATER_STILL);

        if (waterLevel > 3)
		{
            waterLevel = 3;
        }

        renderFaceUp(tt, x, y - 1.0f + (6.0f + waterLevel * 3.0f) / 16.0f, z, liquidTex);
    }
#endif // DISABLE_TESS_FUNCS

    return true;

}

bool TileRenderer_SPU::tesselateFlowerPotInWorld(FlowerPotTile_SPU *tt, int x, int y, int z)
{
#ifdef DISABLE_TESS_FUNCS
	// bounding box first
	tesselateBlockInWorld(tt, x, y, z);

	Tesselator *t = Tesselator::getInstance();

	float br;
	if (SharedConstants::TEXTURE_LIGHTING)
	{
		t->tex2(tt->getLightColor(level, x, y, z));
		br = 1;
	}
	else
	{
		br = tt->getBrightness(level, x, y, z);
	}
	int col = tt->getColor(level, x, y, z);
	Icon *tex = getTexture(tt, 0);
	float r = ((col >> 16) & 0xff) / 255.0f;
	float g = ((col >> 8) & 0xff) / 255.0f;
	float b = ((col) & 0xff) / 255.0f;

	if (GameRenderer::anaglyph3d)
	{
		float cr = (r * 30 + g * 59 + b * 11) / 100;
		float cg = (r * 30 + g * 70) / (100);
		float cb = (r * 30 + b * 70) / (100);

		r = cr;
		g = cg;
		b = cb;
	}
	t->color(br * r, br * g, br * b);

	// render inside

	float halfWidth = (6.0f / 16.0f) / 2 - 0.001f;
	renderEast(tt, x - 0.5f + halfWidth, y, z, tex);
	renderWest(tt, x + 0.5f - halfWidth, y, z, tex);
	renderSouth(tt, x, y, z - 0.5f + halfWidth, tex);
	renderNorth(tt, x, y, z + 0.5f - halfWidth, tex);

	renderFaceUp(tt, x, y - 0.5f + halfWidth + 3.0f / 16.0f, z, getTexture(Tile::dirt));

	int type = level->getData(x, y, z);

	if (type != 0)
	{
		float xOff = 0;
		float yOff = 4;
		float zOff = 0;
		Tile *plant = nullptr;

		switch (type)
		{
		case FlowerPotTile::TYPE_FLOWER_RED:
			plant = Tile::rose;
			break;
		case FlowerPotTile::TYPE_FLOWER_YELLOW:
			plant = Tile::flower;
			break;
		case FlowerPotTile::TYPE_MUSHROOM_BROWN:
			plant = Tile::mushroom1;
			break;
		case FlowerPotTile::TYPE_MUSHROOM_RED:
			plant = Tile::mushroom2;
			break;
		}

		t->addOffset(xOff / 16.0f, yOff / 16.0f, zOff / 16.0f);

		if (plant != nullptr)
		{
			tesselateInWorld(plant, x, y, z);
		}
		else
		{
			if (type == FlowerPotTile::TYPE_CACTUS)
			{

				// Force drawing of all faces else the cactus misses faces
				// when a block is adjacent
				noCulling = true;

				float halfSize = 0.25f / 2;
				setShape(0.5f - halfSize, 0.0f, 0.5f - halfSize, 0.5f + halfSize, 0.25f, 0.5f + halfSize);
				tesselateBlockInWorld(Tile::cactus, x, y, z);
				setShape(0.5f - halfSize, 0.25f, 0.5f - halfSize, 0.5f + halfSize, 0.5f, 0.5f + halfSize);
				tesselateBlockInWorld(Tile::cactus, x, y, z);
				setShape(0.5f - halfSize, 0.5f, 0.5f - halfSize, 0.5f + halfSize, 0.75f, 0.5f + halfSize);
				tesselateBlockInWorld(Tile::cactus, x, y, z);

				noCulling = false;

				setShape(0, 0, 0, 1, 1, 1);
			}
			else if (type == FlowerPotTile::TYPE_SAPLING_DEFAULT)
			{
				tesselateCrossTexture(Tile::sapling, Sapling::TYPE_DEFAULT, x, y, z, 0.75f);
			}
			else if (type == FlowerPotTile::TYPE_SAPLING_BIRCH)
			{
				tesselateCrossTexture(Tile::sapling, Sapling::TYPE_BIRCH, x, y, z, 0.75f);
			}
			else if (type == FlowerPotTile::TYPE_SAPLING_EVERGREEN)
			{
				tesselateCrossTexture(Tile::sapling, Sapling::TYPE_EVERGREEN, x, y, z, 0.75f);
			}
			else if (type == FlowerPotTile::TYPE_SAPLING_JUNGLE)
			{
				tesselateCrossTexture(Tile::sapling, Sapling::TYPE_JUNGLE, x, y, z, 0.75f);
			}
			else if (type == FlowerPotTile::TYPE_FERN)
			{
				col = Tile::tallgrass->getColor(level, x, y, z);
				r = ((col >> 16) & 0xff) / 255.0f;
				g = ((col >> 8) & 0xff) / 255.0f;
				b = ((col) & 0xff) / 255.0f;
				t->color(br * r, br * g, br * b);
				tesselateCrossTexture(Tile::tallgrass, TallGrass::FERN, x, y, z, 0.75f);
			}
			else if (type == FlowerPotTile::TYPE_DEAD_BUSH)
			{
				tesselateCrossTexture(Tile::deadBush, TallGrass::FERN, x, y, z, 0.75f);
			}
		}

		t->addOffset(-xOff / 16.0f, -yOff / 16.0f, -zOff / 16.0f);
	}
#endif //DISABLE_TESS_FUNCS

	return true;
}

bool TileRenderer_SPU::tesselateAnvilInWorld(AnvilTile_SPU *tt, int x, int y, int z)
{
	return tesselateAnvilInWorld(tt, x, y, z, level->getData(x, y, z));

}

bool TileRenderer_SPU::tesselateAnvilInWorld(AnvilTile_SPU *tt, int x, int y, int z, int data)
{
#ifdef DISABLE_TESS_FUNCS
	Tesselator *t = Tesselator::getInstance();

	float br;
	if (SharedConstants::TEXTURE_LIGHTING)
	{
		t->tex2(tt->getLightColor(level, x, y, z));
		br = 1;
	}
	else
	{
		br = tt->getBrightness(level, x, y, z);
	}
	int col = tt->getColor(level, x, y, z);
	float r = ((col >> 16) & 0xff) / 255.0f;
	float g = ((col >> 8) & 0xff) / 255.0f;
	float b = ((col) & 0xff) / 255.0f;

	if (GameRenderer::anaglyph3d)
	{
		float cr = (r * 30 + g * 59 + b * 11) / 100;
		float cg = (r * 30 + g * 70) / (100);
		float cb = (r * 30 + b * 70) / (100);

		r = cr;
		g = cg;
		b = cb;
	}
	t->color(br * r, br * g, br * b);
#endif // DISABLE_TESS_FUNCS


	return tesselateAnvilInWorld(tt, x, y, z, data, false);
}

bool TileRenderer_SPU::tesselateAnvilInWorld(AnvilTile_SPU *tt, int x, int y, int z, int data, bool render)
{
#ifdef DISABLE_TESS_FUNCS

	int facing = render ? 0 : data & 3;
	boolean rotate = false;
	float bottom = 0;

	switch (facing)
	{
	case Direction::NORTH:
		eastFlip = FLIP_CW;
		westFlip = FLIP_CCW;
		break;
	case Direction::SOUTH:
		eastFlip = FLIP_CCW;
		westFlip = FLIP_CW;
		upFlip = FLIP_180;
		downFlip = FLIP_180;
		break;
	case Direction::WEST:
		northFlip = FLIP_CW;
		southFlip = FLIP_CCW;
		upFlip = FLIP_CCW;
		downFlip = FLIP_CW;
		rotate = true;
		break;
	case Direction::EAST:
		northFlip = FLIP_CCW;
		southFlip = FLIP_CW;
		upFlip = FLIP_CW;
		downFlip = FLIP_CCW;
		rotate = true;
		break;
	}

	bottom = tesselateAnvilPiece(tt, x, y, z, AnvilTile::PART_BASE, bottom, 12.0f / 16.0f, 4.0f / 16.0f, 12.0f / 16.0f, rotate, render, data);
	bottom = tesselateAnvilPiece(tt, x, y, z, AnvilTile::PART_JOINT, bottom, 8.0f / 16.0f, 1.0f / 16.0f, 10.0f / 16.0f, rotate, render, data);
	bottom = tesselateAnvilPiece(tt, x, y, z, AnvilTile::PART_COLUMN, bottom, 4.0f / 16.0f, 5.0f / 16.0f, 8.0f / 16.0f, rotate, render, data);
	bottom = tesselateAnvilPiece(tt, x, y, z, AnvilTile::PART_TOP, bottom, 10.0f / 16.0f, 6.0f / 16.0f, 16.0f / 16.0f, rotate, render, data);

	setShape(0, 0, 0, 1, 1, 1);
	northFlip = FLIP_NONE;
	southFlip = FLIP_NONE;
	eastFlip = FLIP_NONE;
	westFlip = FLIP_NONE;
	upFlip = FLIP_NONE;
	downFlip = FLIP_NONE;
#endif //  DISABLE_TESS_FUNCS

	return true;
}

float TileRenderer_SPU::tesselateAnvilPiece(AnvilTile_SPU *tt, int x, int y, int z, int part, float bottom, float width, float height, float length, bool rotate, bool render, int data)
{
#ifdef DISABLE_TESS_FUNCS
	if (rotate)
	{
		float swap = width;
		width = length;
		length = swap;
	}

	width /= 2;
	length /= 2;

	ms_pTileData->anvilPart = part;
	setShape(0.5f - width, bottom, 0.5f - length, 0.5f + width, bottom + height, 0.5f + length);

	if (render)
	{
		Tesselator *t = Tesselator::getInstance();
		t->begin();
		t->normal(0, -1, 0);
		renderFaceDown(tt, 0, 0, 0, getTexture(tt, 0, data));
		t->end();

		t->begin();
		t->normal(0, 1, 0);
		renderFaceUp(tt, 0, 0, 0, getTexture(tt, 1, data));
		t->end();

		t->begin();
		t->normal(0, 0, -1);
		renderNorth(tt, 0, 0, 0, getTexture(tt, 2, data));
		t->end();

		t->begin();
		t->normal(0, 0, 1);
		renderSouth(tt, 0, 0, 0, getTexture(tt, 3, data));
		t->end();

		t->begin();
		t->normal(-1, 0, 0);
		renderWest(tt, 0, 0, 0, getTexture(tt, 4, data));
		t->end();

		t->begin();
		t->normal(1, 0, 0);
		renderEast(tt, 0, 0, 0, getTexture(tt, 5, data));
		t->end();
	}
	else
	{
		tesselateBlockInWorld(tt, x, y, z);
	}
#endif //  DISABLE_TESS_FUNCS

	return bottom + height;
}


bool TileRenderer_SPU::tesselateTorchInWorld( Tile_SPU* tt, int x, int y, int z )
{
	int			dir = level->getData( x, y, z );

	Tesselator_SPU* t = getTesselator();

	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tt->getLightColor( level, x, y, z ) );
		t->color( 1.0f, 1.0f, 1.0f );
	}
	else
	{
		float br = tt->getBrightness( level, x, y, z );
		if ( level->m_tileData.lightEmission[tt->id] > 0 ) br = 1.0f;
		t->color( br, br, br );
	}

	float		r = 0.40f;
	float		r2 = 0.5f - r;
	float		h = 0.20f;
	if ( dir == 1 )
	{
		tesselateTorch( tt, static_cast<float>(x) - r2, static_cast<float>(y) + h, static_cast<float>(z), -r, 0.0f, 0 );
	}
	else if ( dir == 2 )
	{
		tesselateTorch( tt, static_cast<float>(x) + r2, static_cast<float>(y) + h, static_cast<float>(z), +r, 0.0f, 0 );
	}
	else if ( dir == 3 )
	{
		tesselateTorch( tt, static_cast<float>(x), static_cast<float>(y) + h, z - r2, 0.0f, -r, 0 );
	}
	else if ( dir == 4 )
	{
		tesselateTorch( tt, static_cast<float>(x), static_cast<float>(y) + h, static_cast<float>(z) + r2, 0.0f, +r, 0 );
	}
	else
	{
		tesselateTorch( tt, static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 0.0f, 0.0f, 0 );
	}
	return true;

}

bool TileRenderer_SPU::tesselateDiodeInWorld(DiodeTile_SPU *tt, int x, int y, int z)
{
#ifdef DISABLE_TESS_FUNCS
	Tesselator_SPU* t = getTesselator();

	tesselateDiodeInWorld(tt, x, y, z, level->getData(x, y, z) & DiodeTile_SPU::DIRECTION_MASK);
	return true;
#endif // #ifdef DISABLE_TESS_FUNCS
	return false;
}

void TileRenderer_SPU::tesselateDiodeInWorld( DiodeTile_SPU* tt, int x, int y, int z, int dir )
{
#ifdef DISABLE_TESS_FUNCS
	// render half-block edges
	tesselateBlockInWorld( tt, x, y, z );

	Tesselator_SPU* t = getTesselator();

	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tt->getLightColor( level, x, y, z ) );
		t->color( 1.0f, 1.0f, 1.0f );
	}
	else
	{
		float br = tt->getBrightness( level, x, y, z );
		if ( level->m_tileData.lightEmission[tt->id] > 0 ) br = 1.0f;
		t->color( br, br, br );
	}

	int data = level->getData(x, y, z);

	// 4J Stu - This block gets moved in a later version, but we don't need that yet
	// BEGIN TORCH SECTION
	{
		int			dir = data & DiodeTile_SPU::DIRECTION_MASK;
		int			delay = ( data & DiodeTile_SPU::DELAY_MASK ) >> DiodeTile_SPU::DELAY_SHIFT;
		float		h = -3.0f / 16.0f;
		float		transmitterX = 0.0f;
		float		transmitterZ = 0.0f;
		float		receiverX = 0.0f;
		float		receiverZ = 0.0f;

		switch ( dir )
		{
			case Direction::SOUTH:
				receiverZ = -5.0f / 16.0f;
				transmitterZ = DiodeTile_SPU::DELAY_RENDER_OFFSETS[delay];
				break;
			case Direction::NORTH:
				receiverZ = 5.0f / 16.0f;
				transmitterZ = -DiodeTile_SPU::DELAY_RENDER_OFFSETS[delay];
				break;
			case Direction::EAST:
				receiverX = -5.0f / 16.0f;
				transmitterX = DiodeTile_SPU::DELAY_RENDER_OFFSETS[delay];
				break;
			case Direction::WEST:
				receiverX = 5.0f / 16.0f;
				transmitterX = -DiodeTile_SPU::DELAY_RENDER_OFFSETS[delay];
				break;
		}

		// render transmitter
		tesselateTorch( tt, x + transmitterX, y + h, z + transmitterZ, 0.0f, 0.0f, 0 );
		// render receiver
		tesselateTorch( tt, x + receiverX, y + h, z + receiverZ, 0.0f, 0.0f, 0 );
	}
	// END TORCH SECTION

	Icon_SPU *tex = getTexture(tt, Facing::UP, data);
	float u0 = tex->getU0();
	float u1 = tex->getU1();
	float v0 = tex->getV0();
	float v1 = tex->getV1();

	float		r = 2.0f / 16.0f;

	float		x0 = ( float )( x + 1.0f );
	float		x1 = ( float )( x + 1.0f );
	float		x2 = ( float )( x + 0.0f );
	float		x3 = ( float )( x + 0.0f );

	float		z0 = ( float )( z + 0.0f );
	float		z1 = ( float )( z + 1.0f );
	float		z2 = ( float )( z + 1.0f );
	float		z3 = ( float )( z + 0.0f );

	float		y0 = ( float )( y + r );

	if ( dir == Direction::NORTH )
	{
		// rotate 180 degrees
		x0 = x1 = ( float )( x + 0.0f );
		x2 = x3 = ( float )( x + 1.0f );
		z0 = z3 = ( float )( z + 1.0f );
		z1 = z2 = ( float )( z + 0.0f );
	}
	else if ( dir == Direction::EAST )
	{
		// rotate 90 degrees counter-clockwise
		x0 = x3 = ( float )( x + 0.0f );
		x1 = x2 = ( float )( x + 1.0f );
		z0 = z1 = ( float )( z + 0.0f );
		z2 = z3 = ( float )( z + 1.0f );
	}
	else if ( dir == Direction::WEST )
	{
		// rotate 90 degrees clockwise
		x0 = x3 = ( float )( x + 1.0f );
		x1 = x2 = ( float )( x + 0.0f );
		z0 = z1 = ( float )( z + 1.0f );
		z2 = z3 = ( float )( z + 0.0f );
	}

	t->vertexUV( x3 , y0 , z3 , u0 , v0  );
	t->vertexUV( x2 , y0 , z2 , u0 , v1  );
	t->vertexUV( x1 , y0 , z1 , u1 , v1  );
	t->vertexUV( x0 , y0 , z0 , u1 , v0  );
#endif // #ifdef DISABLE_TESS_FUNCS
}

void TileRenderer_SPU::tesselatePistonBaseForceExtended( Tile_SPU* tile, int x, int y, int z, int forceData )	// 4J added forceData param
{
#ifdef DISABLE_TESS_FUNCS
	noCulling = true;
	tesselatePistonBaseInWorld( tile, x, y, z, true, forceData );
	noCulling = false;
#endif // DISABLE_TESS_FUNCS

}

bool TileRenderer_SPU::tesselatePistonBaseInWorld( Tile_SPU* tt, int x, int y, int z, bool forceExtended, int forceData ) // 4J added forceData param
{
#ifdef DISABLE_TESS_FUNCS
	int			data = ( forceData == -1 ) ? level->getData( x, y, z ) : forceData;
	bool		extended = forceExtended || ( data & PistonBaseTile::EXTENDED_BIT ) != 0;
	int			facing = PistonBaseTile::getFacing( data );

	const float thickness = PistonBaseTile::PLATFORM_THICKNESS / 16.0f;

//	EnterCriticalSection( &Tile_SPU::m_csShape );
	if ( extended )
	{
		switch ( facing )
		{
			case Facing::DOWN:
				northFlip = FLIP_180;
				southFlip = FLIP_180;
				eastFlip = FLIP_180;
				westFlip = FLIP_180;
				setShape( 0.0f, thickness, 0.0f, 1.0f, 1.0f, 1.0f );
				break;
			case Facing::UP:
				setShape( 0.0f, 0.0f, 0.0f, 1.0f, 1.0f - thickness, 1.0f );
				break;
			case Facing::NORTH:
				eastFlip = FLIP_CW;
				westFlip = FLIP_CCW;
				setShape( 0.0f, 0.0f, thickness, 1.0f, 1.0f, 1.0f );
				break;
			case Facing::SOUTH:
				eastFlip = FLIP_CCW;
				westFlip = FLIP_CW;
				upFlip = FLIP_180;
				downFlip = FLIP_180;
				setShape( 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f - thickness );
				break;
			case Facing::WEST:
				northFlip = FLIP_CW;
				southFlip = FLIP_CCW;
				upFlip = FLIP_CCW;
				downFlip = FLIP_CW;
				setShape( thickness, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f );
				break;
			case Facing::EAST:
				northFlip = FLIP_CCW;
				southFlip = FLIP_CW;
				upFlip = FLIP_CW;
				downFlip = FLIP_CCW;
				setShape( 0.0f, 0.0f, 0.0f, 1.0f - thickness, 1.0f, 1.0f );
				break;
		}
		// weird way of telling the piston to use the
		// "inside" texture for the forward-facing edge
		((PistonBaseTile *) tt)->updateShape((float) tileShapeX0, (float) tileShapeY0, (float) tileShapeZ0, (float) tileShapeX1, (float) tileShapeY1, (float) tileShapeZ1);
		tesselateBlockInWorld( tt, x, y, z );
		northFlip = FLIP_NONE;
		southFlip = FLIP_NONE;
		eastFlip = FLIP_NONE;
		westFlip = FLIP_NONE;
		upFlip = FLIP_NONE;
		downFlip = FLIP_NONE;
		((PistonBaseTile *) tt)->updateShape((float) tileShapeX0, (float) tileShapeY0, (float) tileShapeZ0, (float) tileShapeX1, (float) tileShapeY1, (float) tileShapeZ1);
	}
	else
	{
		switch ( facing )
		{
			case Facing::DOWN:
				northFlip = FLIP_180;
				southFlip = FLIP_180;
				eastFlip = FLIP_180;
				westFlip = FLIP_180;
				break;
			case Facing::UP:
				break;
			case Facing::NORTH:
				eastFlip = FLIP_CW;
				westFlip = FLIP_CCW;
				break;
			case Facing::SOUTH:
				eastFlip = FLIP_CCW;
				westFlip = FLIP_CW;
				upFlip = FLIP_180;
				downFlip = FLIP_180;
				break;
			case Facing::WEST:
				northFlip = FLIP_CW;
				southFlip = FLIP_CCW;
				upFlip = FLIP_CCW;
				downFlip = FLIP_CW;
				break;
			case Facing::EAST:
				northFlip = FLIP_CCW;
				southFlip = FLIP_CW;
				upFlip = FLIP_CW;
				downFlip = FLIP_CCW;
				break;
		}
		tesselateBlockInWorld( tt, x, y, z );
		northFlip = FLIP_NONE;
		southFlip = FLIP_NONE;
		eastFlip = FLIP_NONE;
		westFlip = FLIP_NONE;
		upFlip = FLIP_NONE;
		downFlip = FLIP_NONE;
	}

//	LeaveCriticalSection( &Tile_SPU::m_csShape );
#endif // DISABLE_TESS_FUNCS

	return true;

}

void TileRenderer_SPU::renderPistonArmUpDown( float x0, float x1, float y0, float y1, float z0, float z1, float br,
										  float armLengthPixels )
{
#ifdef DISABLE_TESS_FUNCS
	Icon_SPU *armTex = PistonBaseTile::getTexture(PistonBaseTile::EDGE_TEX);
	if (hasFixedTexture()) armTex = fixedTexture;

	Tesselator_SPU* t = getTesselator();

	// upwards arm
	float u00 = armTex->getU0();
	float v00 = armTex->getV0();
	float u11 = armTex->getU(armLengthPixels);
	float v11 = armTex->getV(PistonBaseTile::PLATFORM_THICKNESS);

	t->color( br, br, br );

	t->vertexUV( x0, y1, z0, u11, v00 );
	t->vertexUV( x0, y0, z0, u00, v00 );
	t->vertexUV( x1, y0, z1, u00, v11 );
	t->vertexUV( x1, y1, z1, u11, v11 );
#endif // DISABLE_TESS_FUNCS

}

void TileRenderer_SPU::renderPistonArmNorthSouth( float x0, float x1, float y0, float y1, float z0, float z1,
											  float br, float armLengthPixels )
{
#ifdef DISABLE_TESS_FUNCS
	Icon_SPU *armTex = PistonBaseTile::getTexture(PistonBaseTile::EDGE_TEX);
	if (hasFixedTexture()) armTex = fixedTexture;

	Tesselator_SPU* t = getTesselator();

	// upwards arm
	float u00 = armTex->getU0();
	float v00 = armTex->getV0();
	float u11 = armTex->getU(armLengthPixels);
	float v11 = armTex->getV(PistonBaseTile::PLATFORM_THICKNESS);

	t->color( br, br, br );

	t->vertexUV( x0, y0, z1, u11, v00 );
	t->vertexUV( x0, y0, z0, u00, v00 );
	t->vertexUV( x1, y1, z0, u00, v11 );
	t->vertexUV( x1, y1, z1, u11, v11 );
#endif // DISABLE_TESS_FUNCS
}

void TileRenderer_SPU::renderPistonArmEastWest( float x0, float x1, float y0, float y1, float z0, float z1, float br,
											float armLengthPixels )
{
#ifdef DISABLE_TESS_FUNCS
	Icon_SPU *armTex = PistonBaseTile::getTexture(PistonBaseTile::EDGE_TEX);
	if (hasFixedTexture()) armTex = fixedTexture;

	Tesselator_SPU* t = getTesselator();

	// upwards arm
	float u00 = armTex->getU0();
	float v00 = armTex->getV0();
	float u11 = armTex->getU(armLengthPixels);
	float v11 = armTex->getV(PistonBaseTile::PLATFORM_THICKNESS);

	t->color( br, br, br );

	t->vertexUV( x1, y0, z0, u11, v00 );
	t->vertexUV( x0, y0, z0, u00, v00 );
	t->vertexUV( x0, y1, z1, u00, v11 );
	t->vertexUV( x1, y1, z1, u11, v11 );
#endif // DISABLE_TESS_FUNCS
}

void TileRenderer_SPU::tesselatePistonArmNoCulling( Tile_SPU* tile, int x, int y, int z, bool fullArm, int forceData )		// 4J added forceData param
{
#ifdef DISABLE_TESS_FUNCS
	noCulling = true;
	tesselatePistonExtensionInWorld( tile, x, y, z, fullArm );
	noCulling = false;
#endif // DISABLE_TESS_FUNCS
}

bool TileRenderer_SPU::tesselatePistonExtensionInWorld( Tile_SPU* tt, int x, int y, int z, bool fullArm, int forceData )	// 4J added forceData param
{
#ifdef DISABLE_TESS_FUNCS
	int				data = ( forceData == -1 ) ? level->getData( x, y, z ) : forceData;
	int				facing = PistonExtensionTile::getFacing( data );

	const float		thickness = PistonBaseTile::PLATFORM_THICKNESS / 16.0f;
	const float		leftEdge = ( 8.0f - ( PistonBaseTile::PLATFORM_THICKNESS / 2.0f ) ) / 16.0f;
	const float		rightEdge = ( 8.0f + ( PistonBaseTile::PLATFORM_THICKNESS / 2.0f ) ) / 16.0f;
	const float		br = tt->getBrightness( level, x, y, z );
	const float		armLength = fullArm ? 1.0f : 0.5f;
	const float		armLengthPixels = fullArm ? 16.0f : 8.0f;

//	EnterCriticalSection( &Tile_SPU::m_csShape );
	Tesselator_SPU* t = getTesselator();

	switch ( facing )
	{
		case Facing::DOWN:
			northFlip = FLIP_180;
			southFlip = FLIP_180;
			eastFlip = FLIP_180;
			westFlip = FLIP_180;
			setShape( 0.0f, 0.0f, 0.0f, 1.0f, thickness, 1.0f );
			tesselateBlockInWorld( tt, x, y, z );

			t->tex2( getLightColor(tt,  level, x, y , z ) );		// 4J added - renderPistonArmDown doesn't set its own tex2 so just inherited from previous tesselateBlockInWorld
			renderPistonArmUpDown( x + leftEdge, x + rightEdge, y + thickness, y + thickness + armLength,
								   z + rightEdge, z + rightEdge, br * 0.8f, armLengthPixels );
			renderPistonArmUpDown( x + rightEdge, x + leftEdge, y + thickness, y + thickness + armLength, z + leftEdge,
								   z + leftEdge, br * 0.8f, armLengthPixels );
			renderPistonArmUpDown( x + leftEdge, x + leftEdge, y + thickness, y + thickness + armLength, z + leftEdge,
								   z + rightEdge, br * 0.6f, armLengthPixels );
			renderPistonArmUpDown( x + rightEdge, x + rightEdge, y + thickness, y + thickness + armLength,
								   z + rightEdge, z + leftEdge, br * 0.6f, armLengthPixels );

			break;
		case Facing::UP:
			setShape( 0.0f, 1.0f - thickness, 0.0f, 1.0f, 1.0f, 1.0f );
			tesselateBlockInWorld( tt, x, y, z );

			t->tex2( getLightColor(tt,  level, x, y , z ) );		// 4J added - renderPistonArmDown doesn't set its own tex2 so just inherited from previous tesselateBlockInWorld
			renderPistonArmUpDown( x + leftEdge, x + rightEdge, y - thickness + 1.0f - armLength, y - thickness + 1.0f,
								   z + rightEdge, z + rightEdge, br * 0.8f, armLengthPixels );
			renderPistonArmUpDown( x + rightEdge, x + leftEdge, y - thickness + 1.0f - armLength, y - thickness + 1.0f,
								   z + leftEdge, z + leftEdge, br * 0.8f, armLengthPixels );
			renderPistonArmUpDown( x + leftEdge, x + leftEdge, y - thickness + 1.0f - armLength, y - thickness + 1.0f,
								   z + leftEdge, z + rightEdge, br * 0.6f, armLengthPixels );
			renderPistonArmUpDown( x + rightEdge, x + rightEdge, y - thickness + 1.0f - armLength,
								   y - thickness + 1.0f, z + rightEdge, z + leftEdge, br * 0.6f, armLengthPixels );
			break;
		case Facing::NORTH:
			eastFlip = FLIP_CW;
			westFlip = FLIP_CCW;
			setShape( 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, thickness );
			tesselateBlockInWorld( tt, x, y, z );

			t->tex2( getLightColor(tt,  level, x, y , z ) );		// 4J added - renderPistonArmDown doesn't set its own tex2 so just inherited from previous tesselateBlockInWorld
			renderPistonArmNorthSouth( x + leftEdge, x + leftEdge, y + rightEdge, y + leftEdge, z + thickness,
									   z + thickness + armLength, br * 0.6f, armLengthPixels );
			renderPistonArmNorthSouth( x + rightEdge, x + rightEdge, y + leftEdge, y + rightEdge, z + thickness,
									   z + thickness + armLength, br * 0.6f, armLengthPixels );
			renderPistonArmNorthSouth( x + leftEdge, x + rightEdge, y + leftEdge, y + leftEdge, z + thickness,
									   z + thickness + armLength, br * 0.5f, armLengthPixels );
			renderPistonArmNorthSouth( x + rightEdge, x + leftEdge, y + rightEdge, y + rightEdge, z + thickness,
									   z + thickness + armLength, br, armLengthPixels );
			break;
		case Facing::SOUTH:
			eastFlip = FLIP_CCW;
			westFlip = FLIP_CW;
			upFlip = FLIP_180;
			downFlip = FLIP_180;
			setShape( 0.0f, 0.0f, 1.0f - thickness, 1.0f, 1.0f, 1.0f );
			tesselateBlockInWorld( tt, x, y, z );

			t->tex2( getLightColor(tt,  level, x, y , z ) );		// 4J added - renderPistonArmDown doesn't set its own tex2 so just inherited from previous tesselateBlockInWorld
			renderPistonArmNorthSouth( x + leftEdge, x + leftEdge, y + rightEdge, y + leftEdge,
									   z - thickness + 1.0f - armLength, z - thickness + 1.0f, br * 0.6f,
									   armLengthPixels );
			renderPistonArmNorthSouth( x + rightEdge, x + rightEdge, y + leftEdge, y + rightEdge,
									   z - thickness + 1.0f - armLength, z - thickness + 1.0f, br * 0.6f,
									   armLengthPixels );
			renderPistonArmNorthSouth( x + leftEdge, x + rightEdge, y + leftEdge, y + leftEdge,
									   z - thickness + 1.0f - armLength, z - thickness + 1.0f, br * 0.5f,
									   armLengthPixels );
			renderPistonArmNorthSouth( x + rightEdge, x + leftEdge, y + rightEdge, y + rightEdge,
									   z - thickness + 1.0f - armLength, z - thickness + 1.0f, br, armLengthPixels );
			break;
		case Facing::WEST:
			northFlip = FLIP_CW;
			southFlip = FLIP_CCW;
			upFlip = FLIP_CCW;
			downFlip = FLIP_CW;
			setShape( 0.0f, 0.0f, 0.0f, thickness, 1.0f, 1.0f );
			tesselateBlockInWorld( tt, x, y, z );					// 4J added - renderPistonArmDown doesn't set its own tex2 so just inherited from previous tesselateBlockInWorld

			t->tex2( getLightColor(tt,  level, x, y , z ) );
			renderPistonArmEastWest( x + thickness, x + thickness + armLength, y + leftEdge, y + leftEdge,
									 z + rightEdge, z + leftEdge, br * 0.5f, armLengthPixels );
			renderPistonArmEastWest( x + thickness, x + thickness + armLength, y + rightEdge, y + rightEdge,
									 z + leftEdge, z + rightEdge, br, armLengthPixels );
			renderPistonArmEastWest( x + thickness, x + thickness + armLength, y + leftEdge, y + rightEdge,
									 z + leftEdge, z + leftEdge, br * 0.6f, armLengthPixels );
			renderPistonArmEastWest( x + thickness, x + thickness + armLength, y + rightEdge, y + leftEdge,
									 z + rightEdge, z + rightEdge, br * 0.6f, armLengthPixels );
			break;
		case Facing::EAST:
			northFlip = FLIP_CCW;
			southFlip = FLIP_CW;
			upFlip = FLIP_CW;
			downFlip = FLIP_CCW;
			setShape( 1.0f - thickness, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f );
			tesselateBlockInWorld( tt, x, y, z );

			t->tex2( getLightColor(tt,  level, x, y , z ) );			// 4J added - renderPistonArmDown doesn't set its own tex2 so just inherited from previous tesselateBlockInWorld
			renderPistonArmEastWest( x - thickness + 1.0f - armLength, x - thickness + 1.0f, y + leftEdge,
									 y + leftEdge, z + rightEdge, z + leftEdge, br * 0.5f, armLengthPixels );
			renderPistonArmEastWest( x - thickness + 1.0f - armLength, x - thickness + 1.0f, y + rightEdge,
									 y + rightEdge, z + leftEdge, z + rightEdge, br, armLengthPixels );
			renderPistonArmEastWest( x - thickness + 1.0f - armLength, x - thickness + 1.0f, y + leftEdge,
									 y + rightEdge, z + leftEdge, z + leftEdge, br * 0.6f, armLengthPixels );
			renderPistonArmEastWest( x - thickness + 1.0f - armLength, x - thickness + 1.0f, y + rightEdge,
									 y + leftEdge, z + rightEdge, z + rightEdge, br * 0.6f, armLengthPixels );
			break;
	}
	northFlip = FLIP_NONE;
	southFlip = FLIP_NONE;
	eastFlip = FLIP_NONE;
	westFlip = FLIP_NONE;
	upFlip = FLIP_NONE;
	downFlip = FLIP_NONE;
	setShape( 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f );

//	LeaveCriticalSection( &Tile_SPU::m_csShape );
#endif // DISABLE_TESS_FUNCS

	return true;

}

bool TileRenderer_SPU::tesselateLeverInWorld( Tile_SPU* tt, int x, int y, int z )
{
#ifdef DISABLE_TESS_FUNCS
	int			data = level->getData( x, y, z );

	int			dir = data & 7;
	bool		flipped = ( data & 8 ) > 0;

	Tesselator_SPU* t = getTesselator();

	bool hadFixed = hasFixedTexture();
	if (!hadFixed) this->setFixedTexture(getTexture(Tile_SPU::stoneBrick));
	float		w1 = 4.0f / 16.0f;
	float		w2 = 3.0f / 16.0f;
	float		h = 3.0f / 16.0f;
//	EnterCriticalSection( &Tile_SPU::m_csShape );
	if ( dir == 5 )
	{
		setShape( 0.5f - w2, 0.0f, 0.5f - w1, 0.5f + w2, h, 0.5f + w1 );
	}
	else if ( dir == 6 )
	{
		setShape( 0.5f - w1, 0.0f, 0.5f - w2, 0.5f + w1, h, 0.5f + w2 );
	}
	else if ( dir == 4 )
	{
		setShape( 0.5f - w2, 0.5f - w1, 1.0f - h, 0.5f + w2, 0.5f + w1, 1.0f );
	}
	else if ( dir == 3 )
	{
		setShape( 0.5f - w2, 0.5f - w1, 0, 0.5f + w2, 0.5f + w1, h );
	}
	else if ( dir == 2 )
	{
		setShape( 1.0f - h, 0.5f - w1, 0.5f - w2, 1.0f, 0.5f + w1, 0.5f + w2 );
	}
	else if ( dir == 1 )
	{
		setShape( 0, 0.5f - w1, 0.5f - w2, h, 0.5f + w1, 0.5f + w2 );
	}
	else if (dir == 0)
	{
		setShape(0.5f - w1, 1 - h, 0.5f - w2, 0.5f + w1, 1, 0.5f + w2);
	}
	else if (dir == 7)
	{
		setShape(0.5f - w2, 1 - h, 0.5f - w1, 0.5f + w2, 1, 0.5f + w1);
	}
	this->tesselateBlockInWorld( tt, x, y, z );
//	LeaveCriticalSection( &Tile_SPU::m_csShape );
	if ( !hadFixed ) this->clearFixedTexture();

	float		br;
	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tt->getLightColor( level, x, y, z ) );
		br = 1;
	}
	else
	{
		br = tt->getBrightness( level, x, y, z );
	}
	if ( Tile_SPU::lightEmission[tt->id] > 0 ) br = 1.0f;
	t->color( br, br, br );
	Icon_SPU *tex = getTexture(tt, 0);

	if (hasFixedTexture()) tex = fixedTexture;
	float u0 = tex->getU0();
	float v0 = tex->getV0();
	float u1 = tex->getU1();
	float v1 = tex->getV1();

	Vec3*		corners[8];
	float		xv = 1.0f / 16.0f;
	float		zv = 1.0f / 16.0f;
	float		yv = 10.0f / 16.0f;
	corners[0] = Vec3::newTemp( -xv, -0, -zv );
	corners[1] = Vec3::newTemp( +xv, -0, -zv );
	corners[2] = Vec3::newTemp( +xv, -0, +zv );
	corners[3] = Vec3::newTemp( -xv, -0, +zv );
	corners[4] = Vec3::newTemp( -xv, +yv, -zv );
	corners[5] = Vec3::newTemp( +xv, +yv, -zv );
	corners[6] = Vec3::newTemp( +xv, +yv, +zv );
	corners[7] = Vec3::newTemp( -xv, +yv, +zv );

	for ( int i = 0; i < 8; i++ )
	{
		if ( flipped )
		{
			corners[i]->z -= 1 / 16.0f;
			corners[i]->xRot( 40 * PI / 180 );
		}
		else
		{
			corners[i]->z += 1 / 16.0f;
			corners[i]->xRot( -40 * PI / 180 );
		}
		if (dir == 0 || dir == 7)
		{
			corners[i]->zRot(180 * PI / 180);
		}
		if ( dir == 6 || dir == 0 )
		{
			corners[i]->yRot( 90 * PI / 180 );
		}

		if ( dir > 0 && dir < 5 )
		{
			corners[i]->y -= 6 / 16.0f;
			corners[i]->xRot( 90 * PI / 180 );

			if ( dir == 4 ) corners[i]->yRot( 0 * PI / 180 );
			if ( dir == 3 ) corners[i]->yRot( 180 * PI / 180 );
			if ( dir == 2 ) corners[i]->yRot( 90 * PI / 180 );
			if ( dir == 1 ) corners[i]->yRot( -90 * PI / 180 );

			corners[i]->x += x + 0.5;
			corners[i]->y += y + 8 / 16.0f;
			corners[i]->z += z + 0.5;
		}
		else if (dir == 0 || dir == 7)
		{
			corners[i]->x += x + 0.5;
			corners[i]->y += y + 14 / 16.0f;
			corners[i]->z += z + 0.5;
		}
		else
		{
			corners[i]->x += x + 0.5;
			corners[i]->y += y + 2 / 16.0f;
			corners[i]->z += z + 0.5;
		}
	}

	Vec3*		c0 = nullptr, *c1 = nullptr, *c2 = nullptr, *c3 = nullptr;
	for ( int i = 0; i < 6; i++ )
	{
		if ( i == 0 )
		{
			u0 = tex->getU(7);
			v0 = tex->getV(6);
			u1 = tex->getU(9);
			v1 = tex->getV(8);
		}
		else if ( i == 2 )
		{
			u0 = tex->getU(7);
			v0 = tex->getV(6);
			u1 = tex->getU(9);
			v1 = tex->getV1();
		}
		if ( i == 0 )
		{
			c0 = corners[0];
			c1 = corners[1];
			c2 = corners[2];
			c3 = corners[3];
		}
		else if ( i == 1 )
		{
			c0 = corners[7];
			c1 = corners[6];
			c2 = corners[5];
			c3 = corners[4];
		}
		else if ( i == 2 )
		{
			c0 = corners[1];
			c1 = corners[0];
			c2 = corners[4];
			c3 = corners[5];
		}
		else if ( i == 3 )
		{
			c0 = corners[2];
			c1 = corners[1];
			c2 = corners[5];
			c3 = corners[6];
		}
		else if ( i == 4 )
		{
			c0 = corners[3];
			c1 = corners[2];
			c2 = corners[6];
			c3 = corners[7];
		}
		else if ( i == 5 )
		{
			c0 = corners[0];
			c1 = corners[3];
			c2 = corners[7];
			c3 = corners[4];
		}
		t->vertexUV( ( float )( c0->x ), ( float )( c0->y ), ( float )( c0->z ), ( float )( u0 ), ( float )( v1 ) );
		t->vertexUV( ( float )( c1->x ), ( float )( c1->y ), ( float )( c1->z ), ( float )( u1 ), ( float )( v1 ) );
		t->vertexUV( ( float )( c2->x ), ( float )( c2->y ), ( float )( c2->z ), ( float )( u1 ), ( float )( v0 ) );
		t->vertexUV( ( float )( c3->x ), ( float )( c3->y ), ( float )( c3->z ), ( float )( u0 ), ( float )( v0 ) );
	}
#endif // DISABLE_TESS_FUNCS
	return true;

}

bool TileRenderer_SPU::tesselateTripwireSourceInWorld(Tile_SPU *tt, int x, int y, int z)
{
#ifdef DISABLE_TESS_FUNCS
	Tesselator *t = Tesselator::getInstance();
	int data = level->getData(x, y, z);
	int dir = data & TripWireSourceTile::MASK_DIR;
	bool attached = (data & TripWireSourceTile::MASK_ATTACHED) == TripWireSourceTile::MASK_ATTACHED;
	bool powered = (data & TripWireSourceTile::MASK_POWERED) == TripWireSourceTile::MASK_POWERED;
	bool suspended = !level->isTopSolidBlocking(x, y - 1, z);

	bool hadFixed = hasFixedTexture();
	if (!hadFixed) this->setFixedTexture(getTexture(Tile::wood));

	float boxHeight = 4 / 16.0f;
	float boxWidth = 2 / 16.0f;
	float boxDepth = 2 / 16.0f;

	float boxy0 = 0.3f - boxHeight;
	float boxy1 = 0.3f + boxHeight;
	if (dir == Direction::NORTH)
	{
		setShape(0.5f - boxWidth, boxy0, 1 - boxDepth, 0.5f + boxWidth, boxy1, 1);
	}
	else if (dir == Direction::SOUTH)
	{
		setShape(0.5f - boxWidth, boxy0, 0, 0.5f + boxWidth, boxy1, boxDepth);
	}
	else if (dir == Direction::WEST)
	{
		setShape(1 - boxDepth, boxy0, 0.5f - boxWidth, 1, boxy1, 0.5f + boxWidth);
	}
	else if (dir == Direction::EAST)
	{
		setShape(0, boxy0, 0.5f - boxWidth, boxDepth, boxy1, 0.5f + boxWidth);
	}

	this->tesselateBlockInWorld(tt, x, y, z);
	if (!hadFixed) this->clearFixedTexture();

	float brightness;
	if (SharedConstants::TEXTURE_LIGHTING)
	{
		t->tex2(tt->getLightColor(level, x, y, z));
		brightness = 1;
	}
	else
	{
		brightness = tt->getBrightness(level, x, y, z);
	}
	if (Tile::lightEmission[tt->id] > 0) brightness = 1.0f;
	t->color(brightness, brightness, brightness);
	Icon *tex = getTexture(tt, 0);

	if (hasFixedTexture()) tex = fixedTexture;
	double u0 = tex->getU0();
	double v0 = tex->getV0();
	double u1 = tex->getU1();
	double v1 = tex->getV1();

	Vec3 *corners[8];
	float stickWidth = 0.75f / 16.0f;
	float stickHeight = 0.75f / 16.0f;
	float stickLength = 5 / 16.0f;
	corners[0] = Vec3::newTemp(-stickWidth, -0, -stickHeight);
	corners[1] = Vec3::newTemp(+stickWidth, -0, -stickHeight);
	corners[2] = Vec3::newTemp(+stickWidth, -0, +stickHeight);
	corners[3] = Vec3::newTemp(-stickWidth, -0, +stickHeight);
	corners[4] = Vec3::newTemp(-stickWidth, +stickLength, -stickHeight);
	corners[5] = Vec3::newTemp(+stickWidth, +stickLength, -stickHeight);
	corners[6] = Vec3::newTemp(+stickWidth, +stickLength, +stickHeight);
	corners[7] = Vec3::newTemp(-stickWidth, +stickLength, +stickHeight);

	for (int i = 0; i < 8; i++)
	{
		corners[i]->z += 1 / 16.0f;

		if (powered)
		{
			corners[i]->xRot(30 * PI / 180);
			corners[i]->y -= 7 / 16.0f;
		}
		else if (attached)
		{
			corners[i]->xRot(5 * PI / 180);
			corners[i]->y -= 7 / 16.0f;
		}
		else
		{
			corners[i]->xRot(-40 * PI / 180);
			corners[i]->y -= 6 / 16.0f;
		}

		corners[i]->xRot(90 * PI / 180);

		if (dir == Direction::NORTH) corners[i]->yRot(0 * PI / 180);
		if (dir == Direction::SOUTH) corners[i]->yRot(180 * PI / 180);
		if (dir == Direction::WEST) corners[i]->yRot(90 * PI / 180);
		if (dir == Direction::EAST) corners[i]->yRot(-90 * PI / 180);

		corners[i]->x += x + 0.5;
		corners[i]->y += y + 5 / 16.0f;
		corners[i]->z += z + 0.5;
	}

	Vec3 *c0 = nullptr, *c1 = nullptr, *c2 = nullptr, *c3 = nullptr;
	int stickX0 = 7;
	int stickX1 = 9;
	int stickY0 = 9;
	int stickY1 = 16;

	for (int i = 0; i < 6; i++)
	{
		if (i == 0)
		{
			c0 = corners[0];
			c1 = corners[1];
			c2 = corners[2];
			c3 = corners[3];
			u0 = tex->getU(stickX0);
			v0 = tex->getV(stickY0);
			u1 = tex->getU(stickX1);
			v1 = tex->getV(stickY0 + 2);
		}
		else if (i == 1)
		{
			c0 = corners[7];
			c1 = corners[6];
			c2 = corners[5];
			c3 = corners[4];
		}
		else if (i == 2)
		{
			c0 = corners[1];
			c1 = corners[0];
			c2 = corners[4];
			c3 = corners[5];
			u0 = tex->getU(stickX0);
			v0 = tex->getV(stickY0);
			u1 = tex->getU(stickX1);
			v1 = tex->getV(stickY1);
		}
		else if (i == 3)
		{
			c0 = corners[2];
			c1 = corners[1];
			c2 = corners[5];
			c3 = corners[6];
		}
		else if (i == 4)
		{
			c0 = corners[3];
			c1 = corners[2];
			c2 = corners[6];
			c3 = corners[7];
		}
		else if (i == 5)
		{
			c0 = corners[0];
			c1 = corners[3];
			c2 = corners[7];
			c3 = corners[4];
		}
		t->vertexUV(c0->x, c0->y, c0->z, u0, v1);
		t->vertexUV(c1->x, c1->y, c1->z, u1, v1);
		t->vertexUV(c2->x, c2->y, c2->z, u1, v0);
		t->vertexUV(c3->x, c3->y, c3->z, u0, v0);
	}


	float hoopWidth = 1.5f / 16.0f;
	float hoopHeight = 1.5f / 16.0f;
	float hoopLength = 0.5f / 16.0f;
	corners[0] = Vec3::newTemp(-hoopWidth, -0, -hoopHeight);
	corners[1] = Vec3::newTemp(+hoopWidth, -0, -hoopHeight);
	corners[2] = Vec3::newTemp(+hoopWidth, -0, +hoopHeight);
	corners[3] = Vec3::newTemp(-hoopWidth, -0, +hoopHeight);
	corners[4] = Vec3::newTemp(-hoopWidth, +hoopLength, -hoopHeight);
	corners[5] = Vec3::newTemp(+hoopWidth, +hoopLength, -hoopHeight);
	corners[6] = Vec3::newTemp(+hoopWidth, +hoopLength, +hoopHeight);
	corners[7] = Vec3::newTemp(-hoopWidth, +hoopLength, +hoopHeight);

	for (int i = 0; i < 8; i++)
	{
		corners[i]->z += 3.5f / 16.0f;

		if (powered)
		{
			corners[i]->y -= 1.5 / 16.0f;
			corners[i]->z -= 2.6 / 16.0f;
			corners[i]->xRot(0 * PI / 180);
		}
		else if (attached)
		{
			corners[i]->y += 0.25 / 16.0f;
			corners[i]->z -= 2.75 / 16.0f;
			corners[i]->xRot(10 * PI / 180);
		}
		else
		{
			corners[i]->xRot(50 * PI / 180);
		}

		if (dir == Direction::NORTH) corners[i]->yRot(0 * PI / 180);
		if (dir == Direction::SOUTH) corners[i]->yRot(180 * PI / 180);
		if (dir == Direction::WEST) corners[i]->yRot(90 * PI / 180);
		if (dir == Direction::EAST) corners[i]->yRot(-90 * PI / 180);

		corners[i]->x += x + 0.5;
		corners[i]->y += y + 5 / 16.0f;
		corners[i]->z += z + 0.5;
	}

	int hoopX0 = 5;
	int hoopX1 = 11;
	int hoopY0 = 3;
	int hoopY1 = 9;

	for (int i = 0; i < 6; i++)
	{
		if (i == 0)
		{
			c0 = corners[0];
			c1 = corners[1];
			c2 = corners[2];
			c3 = corners[3];
			u0 = tex->getU(hoopX0);
			v0 = tex->getV(hoopY0);
			u1 = tex->getU(hoopX1);
			v1 = tex->getV(hoopY1);
		}
		else if (i == 1)
		{
			c0 = corners[7];
			c1 = corners[6];
			c2 = corners[5];
			c3 = corners[4];
		}
		else if (i == 2)
		{
			c0 = corners[1];
			c1 = corners[0];
			c2 = corners[4];
			c3 = corners[5];
			u0 = tex->getU(hoopX0);
			v0 = tex->getV(hoopY0);
			u1 = tex->getU(hoopX1);
			v1 = tex->getV(hoopY0 + 2);
		}
		else if (i == 3)
		{
			c0 = corners[2];
			c1 = corners[1];
			c2 = corners[5];
			c3 = corners[6];
		}
		else if (i == 4)
		{
			c0 = corners[3];
			c1 = corners[2];
			c2 = corners[6];
			c3 = corners[7];
		}
		else if (i == 5)
		{
			c0 = corners[0];
			c1 = corners[3];
			c2 = corners[7];
			c3 = corners[4];
		}
		t->vertexUV(c0->x, c0->y, c0->z, u0, v1);
		t->vertexUV(c1->x, c1->y, c1->z, u1, v1);
		t->vertexUV(c2->x, c2->y, c2->z, u1, v0);
		t->vertexUV(c3->x, c3->y, c3->z, u0, v0);
	}

	if (attached)
	{
		double hoopBottomY = corners[0]->y;
		float width = 0.5f / 16.0f;
		float top = 0.5f - (width / 2);
		float bottom = top + width;
		Icon *wireTex = getTexture(Tile::tripWire);
		double wireX0 = tex->getU0();
		double wireY0 = tex->getV(attached ? 2 : 0);
		double wireX1 = tex->getU1();
		double wireY1 = tex->getV(attached ? 4 : 2);
		double floating = (suspended ? 3.5f : 1.5f) / 16.0;

		brightness = tt->getBrightness(level, x, y, z) * 0.75f;
		t->color(brightness, brightness, brightness);

		if (dir == Direction::NORTH)
		{
			t->vertexUV(x + top, y + floating, z + 0.25, wireX0, wireY0);
			t->vertexUV(x + bottom, y + floating, z + 0.25, wireX0, wireY1);
			t->vertexUV(x + bottom, y + floating, z, wireX1, wireY1);
			t->vertexUV(x + top, y + floating, z, wireX1, wireY0);

			t->vertexUV(x + top, hoopBottomY, z + 0.5, wireX0, wireY0);
			t->vertexUV(x + bottom, hoopBottomY, z + 0.5, wireX0, wireY1);
			t->vertexUV(x + bottom, y + floating, z + 0.25, wireX1, wireY1);
			t->vertexUV(x + top, y + floating, z + 0.25, wireX1, wireY0);
		}
		else if (dir == Direction::SOUTH)
		{
			t->vertexUV(x + top, y + floating, z + 0.75, wireX0, wireY0);
			t->vertexUV(x + bottom, y + floating, z + 0.75, wireX0, wireY1);
			t->vertexUV(x + bottom, hoopBottomY, z + 0.5, wireX1, wireY1);
			t->vertexUV(x + top, hoopBottomY, z + 0.5, wireX1, wireY0);

			t->vertexUV(x + top, y + floating, z + 1, wireX0, wireY0);
			t->vertexUV(x + bottom, y + floating, z + 1, wireX0, wireY1);
			t->vertexUV(x + bottom, y + floating, z + 0.75, wireX1, wireY1);
			t->vertexUV(x + top, y + floating, z + 0.75, wireX1, wireY0);
		}
		else if (dir == Direction::WEST)
		{
			t->vertexUV(x, y + floating, z + bottom, wireX0, wireY1);
			t->vertexUV(x + 0.25, y + floating, z + bottom, wireX1, wireY1);
			t->vertexUV(x + 0.25, y + floating, z + top, wireX1, wireY0);
			t->vertexUV(x, y + floating, z + top, wireX0, wireY0);

			t->vertexUV(x + 0.25, y + floating, z + bottom, wireX0, wireY1);
			t->vertexUV(x + 0.5, hoopBottomY, z + bottom, wireX1, wireY1);
			t->vertexUV(x + 0.5, hoopBottomY, z + top, wireX1, wireY0);
			t->vertexUV(x + 0.25, y + floating, z + top, wireX0, wireY0);
		}
		else
		{
			t->vertexUV(x + 0.5, hoopBottomY, z + bottom, wireX0, wireY1);
			t->vertexUV(x + 0.75, y + floating, z + bottom, wireX1, wireY1);
			t->vertexUV(x + 0.75, y + floating, z + top, wireX1, wireY0);
			t->vertexUV(x + 0.5, hoopBottomY, z + top, wireX0, wireY0);

			t->vertexUV(x + 0.75, y + floating, z + bottom, wireX0, wireY1);
			t->vertexUV(x + 1, y + floating, z + bottom, wireX1, wireY1);
			t->vertexUV(x + 1, y + floating, z + top, wireX1, wireY0);
			t->vertexUV(x + 0.75, y + floating, z + top, wireX0, wireY0);
		}
	}
#endif // #ifdef DISABLE_TESS_FUNCS

	return true;
}

bool TileRenderer_SPU::tesselateTripwireInWorld(Tile_SPU *tt, int x, int y, int z)
{
#ifdef DISABLE_TESS_FUNCS

	Tesselator *t = Tesselator::getInstance();
	Icon *tex = getTexture(tt, 0);
	int data = level->getData(x, y, z);
	bool attached = (data & TripWireTile::MASK_ATTACHED) == TripWireTile::MASK_ATTACHED;
	bool suspended = (data & TripWireTile::MASK_SUSPENDED) == TripWireTile::MASK_SUSPENDED;

	if (hasFixedTexture()) tex = fixedTexture;

	float brightness;
	if (SharedConstants::TEXTURE_LIGHTING)
	{
		t->tex2(tt->getLightColor(level, x, y, z));
	}
	brightness = tt->getBrightness(level, x, y, z) * 0.75f;
	t->color(brightness, brightness, brightness);

	double wireX0 = tex->getU0();
	double wireY0 = tex->getV(attached ? 2 : 0);
	double wireX1 = tex->getU1();
	double wireY1 = tex->getV(attached ? 4 : 2);
	double floating = (suspended ? 3.5f : 1.5f) / 16.0;

	bool w = TripWireTile::shouldConnectTo(level, x, y, z, data, Direction::WEST);
	bool e = TripWireTile::shouldConnectTo(level, x, y, z, data, Direction::EAST);
	bool n = TripWireTile::shouldConnectTo(level, x, y, z, data, Direction::NORTH);
	bool s = TripWireTile::shouldConnectTo(level, x, y, z, data, Direction::SOUTH);

	float width = 0.5f / 16.0f;
	float top = 0.5f - (width / 2);
	float bottom = top + width;

	if (!n && !e && !s && !w)
	{
		n = true;
		s = true;
	}

	if (n)
	{
		t->vertexUV(x + top, y + floating, z + 0.25, wireX0, wireY0);
		t->vertexUV(x + bottom, y + floating, z + 0.25, wireX0, wireY1);
		t->vertexUV(x + bottom, y + floating, z, wireX1, wireY1);
		t->vertexUV(x + top, y + floating, z, wireX1, wireY0);

		t->vertexUV(x + top, y + floating, z, wireX1, wireY0);
		t->vertexUV(x + bottom, y + floating, z, wireX1, wireY1);
		t->vertexUV(x + bottom, y + floating, z + 0.25, wireX0, wireY1);
		t->vertexUV(x + top, y + floating, z + 0.25, wireX0, wireY0);
	}
	if (n || (s && !e && !w))
	{
		t->vertexUV(x + top, y + floating, z + 0.5, wireX0, wireY0);
		t->vertexUV(x + bottom, y + floating, z + 0.5, wireX0, wireY1);
		t->vertexUV(x + bottom, y + floating, z + 0.25, wireX1, wireY1);
		t->vertexUV(x + top, y + floating, z + 0.25, wireX1, wireY0);

		t->vertexUV(x + top, y + floating, z + 0.25, wireX1, wireY0);
		t->vertexUV(x + bottom, y + floating, z + 0.25, wireX1, wireY1);
		t->vertexUV(x + bottom, y + floating, z + 0.5, wireX0, wireY1);
		t->vertexUV(x + top, y + floating, z + 0.5, wireX0, wireY0);
	}
	if (s || (n && !e && !w))
	{
		t->vertexUV(x + top, y + floating, z + 0.75, wireX0, wireY0);
		t->vertexUV(x + bottom, y + floating, z + 0.75, wireX0, wireY1);
		t->vertexUV(x + bottom, y + floating, z + 0.5, wireX1, wireY1);
		t->vertexUV(x + top, y + floating, z + 0.5, wireX1, wireY0);

		t->vertexUV(x + top, y + floating, z + 0.5, wireX1, wireY0);
		t->vertexUV(x + bottom, y + floating, z + 0.5, wireX1, wireY1);
		t->vertexUV(x + bottom, y + floating, z + 0.75, wireX0, wireY1);
		t->vertexUV(x + top, y + floating, z + 0.75, wireX0, wireY0);
	}
	if (s)
	{
		t->vertexUV(x + top, y + floating, z + 1, wireX0, wireY0);
		t->vertexUV(x + bottom, y + floating, z + 1, wireX0, wireY1);
		t->vertexUV(x + bottom, y + floating, z + 0.75, wireX1, wireY1);
		t->vertexUV(x + top, y + floating, z + 0.75, wireX1, wireY0);

		t->vertexUV(x + top, y + floating, z + 0.75, wireX1, wireY0);
		t->vertexUV(x + bottom, y + floating, z + 0.75, wireX1, wireY1);
		t->vertexUV(x + bottom, y + floating, z + 1, wireX0, wireY1);
		t->vertexUV(x + top, y + floating, z + 1, wireX0, wireY0);
	}

	if (w)
	{
		t->vertexUV(x, y + floating, z + bottom, wireX0, wireY1);
		t->vertexUV(x + 0.25, y + floating, z + bottom, wireX1, wireY1);
		t->vertexUV(x + 0.25, y + floating, z + top, wireX1, wireY0);
		t->vertexUV(x, y + floating, z + top, wireX0, wireY0);

		t->vertexUV(x, y + floating, z + top, wireX0, wireY0);
		t->vertexUV(x + 0.25, y + floating, z + top, wireX1, wireY0);
		t->vertexUV(x + 0.25, y + floating, z + bottom, wireX1, wireY1);
		t->vertexUV(x, y + floating, z + bottom, wireX0, wireY1);
	}
	if (w || (e && !n && !s))
	{
		t->vertexUV(x + 0.25, y + floating, z + bottom, wireX0, wireY1);
		t->vertexUV(x + 0.5, y + floating, z + bottom, wireX1, wireY1);
		t->vertexUV(x + 0.5, y + floating, z + top, wireX1, wireY0);
		t->vertexUV(x + 0.25, y + floating, z + top, wireX0, wireY0);

		t->vertexUV(x + 0.25, y + floating, z + top, wireX0, wireY0);
		t->vertexUV(x + 0.5, y + floating, z + top, wireX1, wireY0);
		t->vertexUV(x + 0.5, y + floating, z + bottom, wireX1, wireY1);
		t->vertexUV(x + 0.25, y + floating, z + bottom, wireX0, wireY1);
	}
	if (e || (w && !n && !s))
	{
		t->vertexUV(x + 0.5, y + floating, z + bottom, wireX0, wireY1);
		t->vertexUV(x + 0.75, y + floating, z + bottom, wireX1, wireY1);
		t->vertexUV(x + 0.75, y + floating, z + top, wireX1, wireY0);
		t->vertexUV(x + 0.5, y + floating, z + top, wireX0, wireY0);

		t->vertexUV(x + 0.5, y + floating, z + top, wireX0, wireY0);
		t->vertexUV(x + 0.75, y + floating, z + top, wireX1, wireY0);
		t->vertexUV(x + 0.75, y + floating, z + bottom, wireX1, wireY1);
		t->vertexUV(x + 0.5, y + floating, z + bottom, wireX0, wireY1);
	}
	if (e)
	{
		t->vertexUV(x + 0.75, y + floating, z + bottom, wireX0, wireY1);
		t->vertexUV(x + 1, y + floating, z + bottom, wireX1, wireY1);
		t->vertexUV(x + 1, y + floating, z + top, wireX1, wireY0);
		t->vertexUV(x + 0.75, y + floating, z + top, wireX0, wireY0);

		t->vertexUV(x + 0.75, y + floating, z + top, wireX0, wireY0);
		t->vertexUV(x + 1, y + floating, z + top, wireX1, wireY0);
		t->vertexUV(x + 1, y + floating, z + bottom, wireX1, wireY1);
		t->vertexUV(x + 0.75, y + floating, z + bottom, wireX0, wireY1);
	}
#endif // DISABLE_TESS_FUNCS

	return true;
}



bool TileRenderer_SPU::tesselateFireInWorld( FireTile_SPU* tt, int x, int y, int z )
{
	Tesselator_SPU* t = getTesselator();

	Icon_SPU *firstTex = tt->getTextureLayer(0);
	Icon_SPU *secondTex = tt->getTextureLayer(1);
	Icon_SPU *tex = firstTex;

	if (hasFixedTexture()) tex = fixedTexture;

	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->color( 1.0f, 1.0f, 1.0f );
		t->tex2( tt->getLightColor( level, x, y, z ) );
	}
	else
	{
		float br = tt->getBrightness( level, x, y, z );
		t->color( br, br, br );
	}
	float u0 = tex->getU0();
	float v0 = tex->getV0();
	float u1 = tex->getU1();
	float v1 = tex->getV1();
	float		h = 1.4f;

	if ( level->isSolidBlockingTile( x, y - 1, z ) || FireTile_SPU::canBurn( level, x, y - 1, z ) )
	{
		float	x0 = x + 0.5f + 0.2f;
		float	x1 = x + 0.5f - 0.2f;
		float	z0 = z + 0.5f + 0.2f;
		float	z1 = z + 0.5f - 0.2f;

		float	x0_ = x + 0.5f - 0.3f;
		float	x1_ = x + 0.5f + 0.3f;
		float	z0_ = z + 0.5f - 0.3f;
		float	z1_ = z + 0.5f + 0.3f;

		t->vertexUV( ( float )( x0_ ), ( float )( y + h ), static_cast<float>(z + 1), ( float )( u1 ), ( float )( v0 ) );
		t->vertexUV( ( float )( x0 ), static_cast<float>(y + 0), static_cast<float>(z + 1), ( float )( u1 ), ( float )( v1 ) );
		t->vertexUV( ( float )( x0 ), static_cast<float>(y + 0), static_cast<float>(z + 0), ( float )( u0 ), ( float )( v1 ) );
		t->vertexUV( ( float )( x0_ ), ( float )( y + h ), static_cast<float>(z + 0), ( float )( u0 ), ( float )( v0 ) );

		t->vertexUV( ( float )( x1_ ), ( float )( y + h ), static_cast<float>(z + 0), ( float )( u1 ), ( float )( v0 ) );
		t->vertexUV( ( float )( x1 ), static_cast<float>(y + 0), static_cast<float>(z + 0), ( float )( u1 ), ( float )( v1 ) );
		t->vertexUV( ( float )( x1 ), static_cast<float>(y + 0), static_cast<float>(z + 1), ( float )( u0 ), ( float )( v1 ) );
		t->vertexUV( ( float )( x1_ ), ( float )( y + h ), static_cast<float>(z + 1), ( float )( u0 ), ( float )( v0 ) );

		tex = secondTex;
		u0 = tex->getU0();
		v0 = tex->getV0();
		u1 = tex->getU1();
		v1 = tex->getV1();

		t->vertexUV( static_cast<float>(x + 1), ( float )( y + h ), ( float )( z1_ ), ( float )( u1 ), ( float )( v0 ) );
		t->vertexUV( static_cast<float>(x + 1), static_cast<float>(y + 0), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
		t->vertexUV( static_cast<float>(x + 0), static_cast<float>(y + 0), ( float )( z1 ), ( float )( u0 ), ( float )( v1 ) );
		t->vertexUV( static_cast<float>(x + 0), ( float )( y + h ), ( float )( z1_ ), ( float )( u0 ), ( float )( v0 ) );

		t->vertexUV( static_cast<float>(x + 0), ( float )( y + h ), ( float )( z0_ ), ( float )( u1 ), ( float )( v0 ) );
		t->vertexUV( static_cast<float>(x + 0), static_cast<float>(y + 0), ( float )( z0 ), ( float )( u1 ), ( float )( v1 ) );
		t->vertexUV( static_cast<float>(x + 1), static_cast<float>(y + 0), ( float )( z0 ), ( float )( u0 ), ( float )( v1 ) );
		t->vertexUV( static_cast<float>(x + 1), ( float )( y + h ), ( float )( z0_ ), ( float )( u0 ), ( float )( v0 ) );

		x0 = x + 0.5f - 0.5f;
		x1 = x + 0.5f + 0.5f;
		z0 = z + 0.5f - 0.5f;
		z1 = z + 0.5f + 0.5f;

		x0_ = x + 0.5f - 0.4f;
		x1_ = x + 0.5f + 0.4f;
		z0_ = z + 0.5f - 0.4f;
		z1_ = z + 0.5f + 0.4f;

		t->vertexUV( ( float )( x0_ ), ( float )( y + h ), static_cast<float>(z + 0), ( float )( u0 ), ( float )( v0 ) );
		t->vertexUV( ( float )( x0 ), static_cast<float>(y + 0), static_cast<float>(z + 0), ( float )( u0 ), ( float )( v1 ) );
		t->vertexUV( ( float )( x0 ), static_cast<float>(y + 0), static_cast<float>(z + 1), ( float )( u1 ), ( float )( v1 ) );
		t->vertexUV( ( float )( x0_ ), ( float )( y + h ), static_cast<float>(z + 1), ( float )( u1 ), ( float )( v0 ) );

		t->vertexUV( ( float )( x1_ ), ( float )( y + h ), static_cast<float>(z + 1), ( float )( u0 ), ( float )( v0 ) );
		t->vertexUV( ( float )( x1 ), static_cast<float>(y + 0), static_cast<float>(z + 1), ( float )( u0 ), ( float )( v1 ) );
		t->vertexUV( ( float )( x1 ), static_cast<float>(y + 0), static_cast<float>(z + 0), ( float )( u1 ), ( float )( v1 ) );
		t->vertexUV( ( float )( x1_ ), ( float )( y + h ), static_cast<float>(z + 0), ( float )( u1 ), ( float )( v0 ) );

		tex = firstTex;
		u0 = tex->getU0();
		v0 = tex->getV0();
		u1 = tex->getU1();
		v1 = tex->getV1();

		t->vertexUV( static_cast<float>(x + 0), ( float )( y + h ), ( float )( z1_ ), ( float )( u0 ), ( float )( v0 ) );
		t->vertexUV( static_cast<float>(x + 0), static_cast<float>(y + 0), ( float )( z1 ), ( float )( u0 ), ( float )( v1 ) );
		t->vertexUV( static_cast<float>(x + 1), static_cast<float>(y + 0), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
		t->vertexUV( static_cast<float>(x + 1), ( float )( y + h ), ( float )( z1_ ), ( float )( u1 ), ( float )( v0 ) );

		t->vertexUV( static_cast<float>(x + 1), ( float )( y + h ), ( float )( z0_ ), ( float )( u0 ), ( float )( v0 ) );
		t->vertexUV( static_cast<float>(x + 1), static_cast<float>(y + 0), ( float )( z0 ), ( float )( u0 ), ( float )( v1 ) );
		t->vertexUV( static_cast<float>(x + 0), static_cast<float>(y + 0), ( float )( z0 ), ( float )( u1 ), ( float )( v1 ) );
		t->vertexUV( static_cast<float>(x + 0), ( float )( y + h ), ( float )( z0_ ), ( float )( u1 ), ( float )( v0 ) );
	}
	else
	{
		float	r = 0.2f;
		float	yo = 1 / 16.0f;
		if ( ( ( x + y + z ) & 1 ) == 1 )
		{
			tex = secondTex;
			u0 = tex->getU0();
			v0 = tex->getV0();
			u1 = tex->getU1();
			v1 = tex->getV1();
		}
		if ( ( ( x / 2 + y / 2 + z / 2 ) & 1 ) == 1 )
		{
			float tmp = u1;
			u1 = u0;
			u0 = tmp;
		}
		if ( FireTile_SPU::canBurn( level, x - 1, y, z ) )
		{
			t->vertexUV( ( float )( x + r ), ( float )( y + h + yo ), ( float )( z +
						 1.0f ), ( float )( u1 ), ( float )( v0 ) );
			t->vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
						 1.0f ), ( float )( u1 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
						 0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + r ), ( float )( y + h + yo ), ( float )( z +
						 0.0f ), ( float )( u0 ), ( float )( v0 ) );

			t->vertexUV( ( float )( x + r ), ( float )( y + h + yo ), ( float )( z +
						 0.0f ), ( float )( u0 ), ( float )( v0 ) );
			t->vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
						 0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
						 1.0f ), ( float )( u1 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + r ), ( float )( y + h + yo ), ( float )( z +
						 1.0f ), ( float )( u1 ), ( float )( v0 ) );
		}
		if ( FireTile_SPU::canBurn( level, x + 1, y, z ) )
		{
			t->vertexUV( ( float )( x + 1 - r ), ( float )( y + h + yo ), ( float )( z +
						 0.0f ), ( float )( u0 ), ( float )( v0 ) );
			t->vertexUV( static_cast<float>(x + 1 - 0), ( float )( y + 0 + yo ), ( float )( z +
                                                                                            0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t->vertexUV( static_cast<float>(x + 1 - 0), ( float )( y + 0 + yo ), ( float )( z +
                                                                                            1.0f ), ( float )( u1 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + 1 - r ), ( float )( y + h + yo ), ( float )( z +
						 1.0f ), ( float )( u1 ), ( float )( v0 ) );

			t->vertexUV( ( float )( x + 1.0f - r ), ( float )( y + h + yo ), ( float )( z +
						 1.0f ), ( float )( u1 ), ( float )( v0 ) );
			t->vertexUV( ( float )( x + 1.0f - 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
						 1.0f ), ( float )( u1 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + 1.0f - 0 ), ( float )( y + 0.0f + yo ), ( float )( z +
						 0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + 1.0f - r ), ( float )( y + h + yo ), ( float )( z +
						 0.0f ), ( float )( u0 ), ( float )( v0 ) );
		}
		if ( FireTile_SPU::canBurn( level, x, y, z - 1 ) )
		{
			t->vertexUV( ( float )( x + 0.0f ), ( float )( y + h + yo ), ( float )( z +
						 r ), ( float )( u1 ), ( float )( v0 ) );
			t->vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
						 0.0f ), ( float )( u1 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + 1.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
						 0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + 1.0f ), ( float )( y + h + yo ), ( float )( z +
						 r ), ( float )( u0 ), ( float )( v0 ) );

			t->vertexUV( ( float )( x + 1.0f ), ( float )( y + h + yo ), ( float )( z +
						 r ), ( float )( u0 ), ( float )( v0 ) );
			t->vertexUV( ( float )( x + 1.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
						 0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
						 0.0f ), ( float )( u1 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + 0.0f ), ( float )( y + h + yo ), ( float )( z +
						 r ), ( float )( u1 ), ( float )( v0 ) );
		}
		if ( FireTile_SPU::canBurn( level, x, y, z + 1 ) )
		{
			t->vertexUV( ( float )( x + 1.0f ), ( float )( y + h + yo ), ( float )( z + 1.0f -
						 r ), ( float )( u0 ), ( float )( v0 ) );
			t->vertexUV( ( float )( x + 1.0f ), ( float )( y + 0.0f + yo ), ( float )( z + 1.0f -
						 0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z + 1.0f -
						 0.0f ), ( float )( u1 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + 0.0f ), ( float )( y + h + yo ), ( float )( z + 1.0f -
						 r ), ( float )( u1 ), ( float )( v0 ) );

			t->vertexUV( ( float )( x + 0.0f ), ( float )( y + h + yo ), ( float )( z + 1.0f -
						 r ), ( float )( u1 ), ( float )( v0 ) );
			t->vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z + 1.0f -
						 0.0f ), ( float )( u1 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + 1.0f ), ( float )( y + 0.0f + yo ), ( float )( z + 1.0f -
						 0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x + 1.0f ), ( float )( y + h + yo ), ( float )( z + 1.0f -
						 r ), ( float )( u0 ), ( float )( v0 ) );
		}
		if ( FireTile_SPU::canBurn( level, x, y + 1.0f, z ) )
		{
			float	x0 = x + 0.5f + 0.5f;
			float	x1 = x + 0.5f - 0.5f;
			float	z0 = z + 0.5f + 0.5f;
			float	z1 = z + 0.5f - 0.5f;

			float	x0_ = x + 0.5f - 0.5f;
			float	x1_ = x + 0.5f + 0.5f;
			float	z0_ = z + 0.5f - 0.5f;
			float	z1_ = z + 0.5f + 0.5f;

			tex = firstTex;
			u0 = tex->getU0();
			v0 = tex->getV0();
			u1 = tex->getU1();
			v1 = tex->getV1();

			y += 1;
			h = -0.2f;

			if ( ( ( x + y + z ) & 1 ) == 0 )
			{
				t->vertexUV( ( float )( x0_ ), ( float )( y + h ), static_cast<float>(z +
                                                                                      0), ( float )( u1 ), ( float )( v0 ) );
				t->vertexUV( ( float )( x0 ), static_cast<float>(y + 0), static_cast<float>(z +
                                                                                            0), ( float )( u1 ), ( float )( v1 ) );
				t->vertexUV( ( float )( x0 ), static_cast<float>(y + 0), static_cast<float>(z +
                                                                                            1), ( float )( u0 ), ( float )( v1 ) );
				t->vertexUV( ( float )( x0_ ), ( float )( y + h ), static_cast<float>(z +
                                                                                      1), ( float )( u0 ), ( float )( v0 ) );

				tex = secondTex;
				u0 = tex->getU0();
				v0 = tex->getV0();
				u1 = tex->getU1();
				v1 = tex->getV1();

				t->vertexUV( ( float )( x1_ ), ( float )( y + h ), ( float )( z +
							 1.0f ), ( float )( u1 ), ( float )( v0 ) );
				t->vertexUV( ( float )( x1 ), ( float )( y + 0.0f ), ( float )( z +
							 1.0f ), ( float )( u1 ), ( float )( v1 ) );
				t->vertexUV( ( float )( x1 ), ( float )( y + 0.0f ), static_cast<float>(z +
                                                                                        0), ( float )( u0 ), ( float )( v1 ) );
				t->vertexUV( ( float )( x1_ ), ( float )( y + h ), static_cast<float>(z +
                                                                                      0), ( float )( u0 ), ( float )( v0 ) );
			}
			else
			{
				t->vertexUV( ( float )( x + 0.0f ), ( float )( y +
							 h ), ( float )( z1_ ), ( float )( u1 ), ( float )( v0 ) );
				t->vertexUV( ( float )( x + 0.0f ), ( float )( y +
							 0.0f ), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
				t->vertexUV( ( float )( x + 1.0f ), ( float )( y +
							 0.0f ), ( float )( z1 ), ( float )( u0 ), ( float )( v1 ) );
				t->vertexUV( ( float )( x + 1.0f ), ( float )( y +
							 h ), ( float )( z1_ ), ( float )( u0 ), ( float )( v0 ) );

				tex = secondTex;
				u0 = tex->getU0();
				v0 = tex->getV0();
				u1 = tex->getU1();
				v1 = tex->getV1();

				t->vertexUV( ( float )( x + 1.0f ), ( float )( y +
							 h ), ( float )( z0_ ), ( float )( u1 ), ( float )( v0 ) );
				t->vertexUV( ( float )( x + 1.0f ), ( float )( y +
							 0.0f ), ( float )( z0 ), ( float )( u1 ), ( float )( v1 ) );
				t->vertexUV( ( float )( x + 0.0f ), ( float )( y +
							 0.0f ), ( float )( z0 ), ( float )( u0 ), ( float )( v1 ) );
				t->vertexUV( ( float )( x + 0.0f ), ( float )( y +
							 h ), ( float )( z0_ ), ( float )( u0 ), ( float )( v0 ) );
			}
		}
	}
	return true;
}

bool TileRenderer_SPU::tesselateDustInWorld( Tile_SPU* tt, int x, int y, int z )
{
#ifdef DISABLE_TESS_FUNCS

	Tesselator_SPU* t = getTesselator();

	int			data = level->getData( x, y, z );
	Icon_SPU *crossTexture = RedStoneDustTile_SPU::getTextureByName(RedStoneDustTile_SPU::TEXTURE_CROSS);
	Icon_SPU *lineTexture = RedStoneDustTile_SPU::getTextureByName(RedStoneDustTile_SPU::TEXTURE_LINE);
	Icon_SPU *crossTextureOverlay = RedStoneDustTile_SPU::getTextureByName(RedStoneDustTile_SPU::TEXTURE_CROSS_OVERLAY);
	Icon_SPU *lineTextureOverlay = RedStoneDustTile_SPU::getTextureByName(RedStoneDustTile_SPU::TEXTURE_LINE_OVERLAY);

	float		br;
	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tt->getLightColor( level, x, y, z ) );
		br = 1;
	}
	else
	{
		br = tt->getBrightness( level, x, y, z );
	}
	float		pow = ( data / 15.0f );
	float		red = pow * 0.6f + 0.4f;
	if ( data == 0 ) red = 0.3f;

	float		green = pow * pow * 0.7f - 0.5f;
	float		blue = pow * pow * 0.6f - 0.7f;
	if ( green < 0 ) green = 0;
	if ( blue < 0 ) blue = 0;

	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->color( red, green, blue );
	}
	else
	{
		t->color( br * red, br * green, br * blue );
	}
	const float dustOffset = 0.25f / 16.0f;
	const float overlayOffset = 0.25f / 16.0f;

	bool		w = RedStoneDustTile_SPU::shouldConnectTo( level, x - 1, y, z, Direction::WEST )
		|| ( !level->isSolidBlockingTile( x - 1, y, z ) && RedStoneDustTile_SPU::shouldConnectTo( level, x - 1, y - 1, z,
																							  Direction::UNDEFINED ) );
	bool		e = RedStoneDustTile_SPU::shouldConnectTo( level, x + 1, y, z, Direction::EAST )
		|| ( !level->isSolidBlockingTile( x + 1, y, z ) && RedStoneDustTile_SPU::shouldConnectTo( level, x + 1, y - 1, z,
																							  Direction::UNDEFINED ) );
	bool		n = RedStoneDustTile_SPU::shouldConnectTo( level, x, y, z - 1, Direction::NORTH )
		|| ( !level->isSolidBlockingTile( x, y, z - 1 ) && RedStoneDustTile_SPU::shouldConnectTo( level, x, y - 1, z - 1,
																							  Direction::UNDEFINED ) );
	bool		s = RedStoneDustTile_SPU::shouldConnectTo( level, x, y, z + 1, Direction::SOUTH )
		|| ( !level->isSolidBlockingTile( x, y, z + 1 ) && RedStoneDustTile_SPU::shouldConnectTo( level, x, y - 1, z + 1,
																							  Direction::UNDEFINED ) );
	if ( !level->isSolidBlockingTile( x, y + 1, z ) )
	{
		if ( level->isSolidBlockingTile( x - 1, y, z ) && RedStoneDustTile_SPU::shouldConnectTo( level, x - 1, y + 1, z,
																							 Direction::UNDEFINED ) ) w
				= true;
		if ( level->isSolidBlockingTile( x + 1, y, z ) && RedStoneDustTile_SPU::shouldConnectTo( level, x + 1, y + 1, z,
																							 Direction::UNDEFINED ) ) e
				= true;
		if ( level->isSolidBlockingTile( x, y, z - 1 ) && RedStoneDustTile_SPU::shouldConnectTo( level, x, y + 1, z - 1,
																							 Direction::UNDEFINED ) ) n
				= true;
		if ( level->isSolidBlockingTile( x, y, z + 1 ) && RedStoneDustTile_SPU::shouldConnectTo( level, x, y + 1, z + 1,
																							 Direction::UNDEFINED ) ) s
				= true;
	}
	float		x0 = ( float )( x + 0.0f );
	float		x1 = ( float )( x + 1.0f );
	float		z0 = ( float )( z + 0.0f );
	float		z1 = ( float )( z + 1.0f );

	int			pic = 0;
	if ( ( w || e ) && ( !n && !s ) ) pic = 1;
	if ( ( n || s ) && ( !e && !w ) ) pic = 2;

	if ( pic == 0 )
	{
//		if ( e || n || s || w )
		int u0 = 0;
		int v0 = 0;
		int u1 = SharedConstants::WORLD_RESOLUTION;
		int v1 = SharedConstants::WORLD_RESOLUTION;

		int cutDistance = 5;
		if (!w) x0 += cutDistance / (float) SharedConstants::WORLD_RESOLUTION;
		if (!w) u0 += cutDistance;
		if (!e) x1 -= cutDistance / (float) SharedConstants::WORLD_RESOLUTION;
		if (!e) u1 -= cutDistance;
		if (!n) z0 += cutDistance / (float) SharedConstants::WORLD_RESOLUTION;
		if (!n) v0 += cutDistance;
		if (!s) z1 -= cutDistance / (float) SharedConstants::WORLD_RESOLUTION;
		if (!s) v1 -= cutDistance;
		t->vertexUV( ( float )( x1 ), ( float )( y + dustOffset ), ( float )( z1 ), crossTexture->getU(u1), crossTexture->getV(v1) );
		t->vertexUV( ( float )( x1 ), ( float )( y + dustOffset ), ( float )( z0 ), crossTexture->getU(u1), crossTexture->getV(v0) );
		t->vertexUV( ( float )( x0 ), ( float )( y + dustOffset ), ( float )( z0 ), crossTexture->getU(u0), crossTexture->getV(v0) );
		t->vertexUV( ( float )( x0 ), ( float )( y + dustOffset ), ( float )( z1 ), crossTexture->getU(u0), crossTexture->getV(v1) );

		t->color( br, br, br );
		t->vertexUV( ( float )( x1 ), ( float )( y + dustOffset ), ( float )( z1 ), crossTextureOverlay->getU(u1), crossTextureOverlay->getV(v1) );
		t->vertexUV( ( float )( x1 ), ( float )( y + dustOffset ), ( float )( z0 ), crossTextureOverlay->getU(u1), crossTextureOverlay->getV(v0) );
		t->vertexUV( ( float )( x0 ), ( float )( y + dustOffset ), ( float )( z0 ), crossTextureOverlay->getU(u0), crossTextureOverlay->getV(v0) );
		t->vertexUV( ( float )( x0 ), ( float )( y + dustOffset ), ( float )( z1 ), crossTextureOverlay->getU(u0), crossTextureOverlay->getV(v1) );
	}
	else if ( pic == 1 )
	{
		t->vertexUV( ( float )( x1 ), ( float )( y + dustOffset ), ( float )( z1 ), lineTexture->getU1(), lineTexture->getV1() );
		t->vertexUV( ( float )( x1 ), ( float )( y + dustOffset ), ( float )( z0 ), lineTexture->getU1(), lineTexture->getV0() );
		t->vertexUV( ( float )( x0 ), ( float )( y + dustOffset ), ( float )( z0 ), lineTexture->getU0(), lineTexture->getV0() );
		t->vertexUV( ( float )( x0 ), ( float )( y + dustOffset ), ( float )( z1 ), lineTexture->getU0(), lineTexture->getV1() );

		t->color( br, br, br );
		t->vertexUV( ( float )( x1 ), ( float )( y + overlayOffset ), ( float )( z1 ), lineTextureOverlay->getU1(), lineTextureOverlay->getV1() );
		t->vertexUV( ( float )( x1 ), ( float )( y + overlayOffset ), ( float )( z0 ), lineTextureOverlay->getU1(), lineTextureOverlay->getV0() );
		t->vertexUV( ( float )( x0 ), ( float )( y + overlayOffset ), ( float )( z0 ), lineTextureOverlay->getU0(), lineTextureOverlay->getV0() );
		t->vertexUV( ( float )( x0 ), ( float )( y + overlayOffset ), ( float )( z1 ), lineTextureOverlay->getU0(), lineTextureOverlay->getV1() );
	}
	else
	{
		t->vertexUV( ( float )( x1 ), ( float )( y + dustOffset ), ( float )( z1 ), lineTexture->getU1(), lineTexture->getV1() );
		t->vertexUV( ( float )( x1 ), ( float )( y + dustOffset ), ( float )( z0 ), lineTexture->getU0(), lineTexture->getV1() );
		t->vertexUV( ( float )( x0 ), ( float )( y + dustOffset ), ( float )( z0 ), lineTexture->getU0(), lineTexture->getV0() );
		t->vertexUV( ( float )( x0 ), ( float )( y + dustOffset ), ( float )( z1 ), lineTexture->getU1(), lineTexture->getV0() );

		t->color( br, br, br );
		t->vertexUV( ( float )( x1 ), ( float )( y + overlayOffset ), ( float )( z1 ), lineTextureOverlay->getU1(), lineTextureOverlay->getV1() );
		t->vertexUV( ( float )( x1 ), ( float )( y + overlayOffset ), ( float )( z0 ), lineTextureOverlay->getU0(), lineTextureOverlay->getV1() );
		t->vertexUV( ( float )( x0 ), ( float )( y + overlayOffset ), ( float )( z0 ), lineTextureOverlay->getU0(), lineTextureOverlay->getV0() );
		t->vertexUV( ( float )( x0 ), ( float )( y + overlayOffset ), ( float )( z1 ), lineTextureOverlay->getU1(), lineTextureOverlay->getV0() );
	}

	if ( !level->isSolidBlockingTile( x, y + 1, z ) )
	{
		const float yStretch = .35f / 16.0f;

		if ( level->isSolidBlockingTile( x - 1, y, z ) && level->getTile( x - 1, y + 1, z ) == Tile_SPU::redStoneDust_Id )
		{
			t->color( br * red, br * green, br * blue );
			t->vertexUV( ( float )( x + dustOffset ), ( float )( y + 1 + yStretch ), ( float )( z + 1 ), lineTexture->getU1(), lineTexture->getV0() );
			t->vertexUV( ( float )( x + dustOffset ), ( float )( y + 0 ), ( float )( z + 1 ), lineTexture->getU0(), lineTexture->getV0() );
			t->vertexUV( ( float )( x + dustOffset ), ( float )( y + 0 ), ( float )( z + 0 ), lineTexture->getU0(), lineTexture->getV1() );
			t->vertexUV( ( float )( x + dustOffset ), ( float )( y + 1 + yStretch ), ( float )( z +	 0 ), lineTexture->getU1(), lineTexture->getV1() );

			t->color( br, br, br );
			t->vertexUV( ( float )( x + overlayOffset ), ( float )( y + 1 + yStretch ), ( float )( z + 1 ), lineTextureOverlay->getU1(), lineTextureOverlay->getV0() );
			t->vertexUV( ( float )( x + overlayOffset ), ( float )( y + 0 ), ( float )( z + 1 ), lineTextureOverlay->getU0(), lineTextureOverlay->getV0() );
			t->vertexUV( ( float )( x + overlayOffset ), ( float )( y + 0 ), ( float )( z +	 0 ), lineTextureOverlay->getU0(), lineTextureOverlay->getV1() );
			t->vertexUV( ( float )( x + overlayOffset ), ( float )( y + 1 + yStretch ), ( float )( z + 0 ), lineTextureOverlay->getU1(), lineTextureOverlay->getV1() );
		}
		if ( level->isSolidBlockingTile( x + 1, y, z ) && level->getTile( x + 1, y + 1, z ) == Tile_SPU::redStoneDust_Id )
		{
			t->color( br * red, br * green, br * blue );
			t->vertexUV( ( float )( x + 1 - dustOffset ), ( float )( y + 0 ), ( float )( z + 1 ), lineTexture->getU0(), lineTexture->getV1() );
			t->vertexUV( ( float )( x + 1 - dustOffset ), ( float )( y + 1 + yStretch ), ( float )( z + 1 ), lineTexture->getU1(), lineTexture->getV1() );
			t->vertexUV( ( float )( x + 1 - dustOffset ), ( float )( y + 1 + yStretch ), ( float )( z + 0 ), lineTexture->getU1(), lineTexture->getV0() );
			t->vertexUV( ( float )( x + 1 - dustOffset ), ( float )( y + 0 ), ( float )( z + 0 ), lineTexture->getU0(), lineTexture->getV0() );

			t->color( br, br, br );
			t->vertexUV( ( float )( x + 1 - overlayOffset ), ( float )( y + 0 ), ( float )( z + 1 ), lineTextureOverlay->getU0(), lineTextureOverlay->getV1() );
			t->vertexUV( ( float )( x + 1 - overlayOffset ), ( float )( y + 1 + yStretch ), ( float )( z + 1 ), lineTextureOverlay->getU1(), lineTextureOverlay->getV1() );
			t->vertexUV( ( float )( x + 1 - overlayOffset ), ( float )( y + 1 + yStretch ), ( float )( z + 0 ), lineTextureOverlay->getU1(), lineTextureOverlay->getV0() );
			t->vertexUV( ( float )( x + 1 - overlayOffset ), ( float )( y + 0 ), ( float )( z + 0 ), lineTextureOverlay->getU0(), lineTextureOverlay->getV0() );
		}
		if ( level->isSolidBlockingTile( x, y, z - 1 ) && level->getTile( x, y + 1, z - 1 ) == Tile_SPU::redStoneDust_Id )
		{
			t->color( br * red, br * green, br * blue );
			t->vertexUV( ( float )( x + 1 ), ( float )( y + 0 ), ( float )( z + dustOffset ), lineTexture->getU0(), lineTexture->getV1() );
			t->vertexUV( ( float )( x + 1 ), ( float )( y + 1 + yStretch ), ( float )( z + dustOffset ), lineTexture->getU1(), lineTexture->getV1() );
			t->vertexUV( ( float )( x + 0 ), ( float )( y + 1 + yStretch ), ( float )( z + dustOffset ), lineTexture->getU1(), lineTexture->getV0() );
			t->vertexUV( ( float )( x + 0 ), ( float )( y + 0 ), ( float )( z + dustOffset ), lineTexture->getU0(), lineTexture->getV0() );

			t->color( br, br, br );
			t->vertexUV( ( float )( x + 1 ), ( float )( y + 0 ), ( float )( z + overlayOffset ), lineTextureOverlay->getU0(), lineTextureOverlay->getV1() );
			t->vertexUV( ( float )( x + 1 ), ( float )( y + 1 + yStretch ), ( float )( z + overlayOffset ), lineTextureOverlay->getU1(), lineTextureOverlay->getV1() );
			t->vertexUV( ( float )( x + 0 ), ( float )( y + 1 + yStretch ), ( float )( z + overlayOffset ), lineTextureOverlay->getU1(), lineTextureOverlay->getV0() );
			t->vertexUV( ( float )( x + 0 ), ( float )( y + 0 ), ( float )( z + overlayOffset ), lineTextureOverlay->getU0(), lineTextureOverlay->getV0() );
		}
		if ( level->isSolidBlockingTile( x, y, z + 1 ) && level->getTile( x, y + 1, z + 1 ) == Tile_SPU::redStoneDust_Id )
		{
			t->color( br * red, br * green, br * blue );
			t->vertexUV( ( float )( x + 1 ), ( float )( y + 1 + yStretch ), ( float )( z + 1 - dustOffset ), lineTexture->getU1(), lineTexture->getV0() );
			t->vertexUV( ( float )( x + 1 ), ( float )( y + 0 ), ( float )( z + 1 -	dustOffset ), lineTexture->getU0(), lineTexture->getV0() );
			t->vertexUV( ( float )( x + 0 ), ( float )( y + 0 ), ( float )( z + 1 -	dustOffset ), lineTexture->getU0(), lineTexture->getV1() );
			t->vertexUV( ( float )( x + 0 ), ( float )( y + 1 + yStretch ), ( float )( z + 1 - dustOffset ), lineTexture->getU1(), lineTexture->getV1() );

			t->color( br, br, br );
			t->vertexUV( ( float )( x + 1 ), ( float )( y + 1 + yStretch ), ( float )( z + 1 - overlayOffset ), lineTextureOverlay->getU1(), lineTextureOverlay->getV0() );
			t->vertexUV( ( float )( x + 1 ), ( float )( y + 0 ), ( float )( z + 1 - overlayOffset ), lineTextureOverlay->getU0(), lineTextureOverlay->getV0() );
			t->vertexUV( ( float )( x + 0 ), ( float )( y + 0 ), ( float )( z + 1 - overlayOffset ), lineTextureOverlay->getU0(), lineTextureOverlay->getV1() );
			t->vertexUV( ( float )( x + 0 ), ( float )( y + 1 + yStretch ), ( float )( z + 1 - overlayOffset ), lineTextureOverlay->getU1(), lineTextureOverlay->getV1() );
		}
	}
#endif // #ifdef DISABLE_TESS_FUNCS

	return true;
}

bool TileRenderer_SPU::tesselateRailInWorld( RailTile_SPU* tt, int x, int y, int z )
{
	Tesselator_SPU* t = getTesselator();
	int			data = level->getData( x, y, z );

	Icon_SPU *tex = getTexture(tt, 0, data);
	if (hasFixedTexture()) tex = fixedTexture;

	if ( tt->isUsesDataBit() )
	{
		data &= RailTile_SPU::RAIL_DIRECTION_MASK;
	}

	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tt->getLightColor( level, x, y, z ) );
		t->color( 1.0f, 1.0f, 1.0f );
	}
	else
	{
		float br = tt->getBrightness( level, x, y, z );
		t->color( br, br, br );
	}

	float u0 = tex->getU0();
	float v0 = tex->getV0();
	float u1 = tex->getU1();
	float v1 = tex->getV1();

	float		r = 1 / 16.0f;

	float		x0 = static_cast<float>(x + 1);
	float		x1 = static_cast<float>(x + 1);
	float		x2 = static_cast<float>(x + 0);
	float		x3 = static_cast<float>(x + 0);

	float		z0 = static_cast<float>(z + 0);
	float		z1 = static_cast<float>(z + 1);
	float		z2 = static_cast<float>(z + 1);
	float		z3 = static_cast<float>(z + 0);

	float		y0 = ( float )( y + r );
	float		y1 = ( float )( y + r );
	float		y2 = ( float )( y + r );
	float		y3 = ( float )( y + r );

	if ( data == 1 || data == 2 || data == 3 || data == 7 )
	{
		x0 = x3 = static_cast<float>(x + 1);
		x1 = x2 = static_cast<float>(x + 0);
		z0 = z1 = static_cast<float>(z + 1);
		z2 = z3 = static_cast<float>(z + 0);
	}
	else if ( data == 8 )
	{
		x0 = x1 = static_cast<float>(x + 0);
		x2 = x3 = static_cast<float>(x + 1);
		z0 = z3 = static_cast<float>(z + 1);
		z1 = z2 = static_cast<float>(z + 0);
	}
	else if ( data == 9 )
	{
		x0 = x3 = static_cast<float>(x + 0);
		x1 = x2 = static_cast<float>(x + 1);
		z0 = z1 = static_cast<float>(z + 0);
		z2 = z3 = static_cast<float>(z + 1);
	}

	if ( data == 2 || data == 4 )
	{
		y0 += 1;
		y3 += 1;
	}
	else if ( data == 3 || data == 5 )
	{
		y1 += 1;
		y2 += 1;
	}

	t->vertexUV( ( float )( x0 ), ( float )( y0 ), ( float )( z0 ), ( float )( u1 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y1 ), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x2 ), ( float )( y2 ), ( float )( z2 ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x3 ), ( float )( y3 ), ( float )( z3 ), ( float )( u0 ), ( float )( v0 ) );

	t->vertexUV( ( float )( x3 ), ( float )( y3 ), ( float )( z3 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x2 ), ( float )( y2 ), ( float )( z2 ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y1 ), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y0 ), ( float )( z0 ), ( float )( u1 ), ( float )( v0 ) );

	return true;

}

bool TileRenderer_SPU::tesselateLadderInWorld( Tile_SPU* tt, int x, int y, int z )
{
	Tesselator_SPU* t = getTesselator();

	Icon_SPU *tex = getTexture(tt, 0);

	if (hasFixedTexture()) tex = fixedTexture;

	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tt->getLightColor( level, x, y, z ) );
		float br = 1;
		t->color( br, br, br );
	}
	else
	{
		float br = tt->getBrightness( level, x, y, z );
		t->color( br, br, br );
	}
	float u0 = tex->getU0();
	float v0 = tex->getV0();
	float u1 = tex->getU1();
	float v1 = tex->getV1();

	int			face = level->getData( x, y, z );

	float		o = 0 / 16.0f;
	float		r = 0.05f;
	if ( face == 5 )
	{
		t->vertexUV( ( float )( x + r ), ( float )( y + 1 + o ), ( float )( z + 1 +
					 o ), ( float )( u0 ), ( float )( v0 ) );
		t->vertexUV( ( float )( x + r ), ( float )( y + 0 - o ), ( float )( z + 1 +
					 o ), ( float )( u0 ), ( float )( v1 ) );
		t->vertexUV( ( float )( x + r ), ( float )( y + 0 - o ), ( float )( z + 0 -
					 o ), ( float )( u1 ), ( float )( v1 ) );
		t->vertexUV( ( float )( x + r ), ( float )( y + 1 + o ), ( float )( z + 0 -
					 o ), ( float )( u1 ), ( float )( v0 ) );
	}
	if ( face == 4 )
	{
		t->vertexUV( ( float )( x + 1 - r ), ( float )( y + 0 - o ), ( float )( z + 1 +
					 o ), ( float )( u1 ), ( float )( v1 ) );
		t->vertexUV( ( float )( x + 1 - r ), ( float )( y + 1 + o ), ( float )( z + 1 +
					 o ), ( float )( u1 ), ( float )( v0 ) );
		t->vertexUV( ( float )( x + 1 - r ), ( float )( y + 1 + o ), ( float )( z + 0 -
					 o ), ( float )( u0 ), ( float )( v0 ) );
		t->vertexUV( ( float )( x + 1 - r ), ( float )( y + 0 - o ), ( float )( z + 0 -
					 o ), ( float )( u0 ), ( float )( v1 ) );
	}
	if ( face == 3 )
	{
		t->vertexUV( ( float )( x + 1 + o ), ( float )( y + 0 - o ), ( float )( z +
					 r ), ( float )( u1 ), ( float )( v1 ) );
		t->vertexUV( ( float )( x + 1 + o ), ( float )( y + 1 + o ), ( float )( z +
					 r ), ( float )( u1 ), ( float )( v0 ) );
		t->vertexUV( ( float )( x + 0 - o ), ( float )( y + 1 + o ), ( float )( z +
					 r ), ( float )( u0 ), ( float )( v0 ) );
		t->vertexUV( ( float )( x + 0 - o ), ( float )( y + 0 - o ), ( float )( z +
					 r ), ( float )( u0 ), ( float )( v1 ) );
	}
	if ( face == 2 )
	{
		t->vertexUV( ( float )( x + 1 + o ), ( float )( y + 1 + o ), ( float )( z + 1 -
					 r ), ( float )( u0 ), ( float )( v0 ) );
		t->vertexUV( ( float )( x + 1 + o ), ( float )( y + 0 - o ), ( float )( z + 1 -
					 r ), ( float )( u0 ), ( float )( v1 ) );
		t->vertexUV( ( float )( x + 0 - o ), ( float )( y + 0 - o ), ( float )( z + 1 -
					 r ), ( float )( u1 ), ( float )( v1 ) );
		t->vertexUV( ( float )( x + 0 - o ), ( float )( y + 1 + o ), ( float )( z + 1 -
					 r ), ( float )( u1 ), ( float )( v0 ) );
	}
	return true;
}

bool TileRenderer_SPU::tesselateVineInWorld( Tile_SPU* tt, int x, int y, int z )
{
	Tesselator_SPU* t = getTesselator();

	Icon_SPU *tex = getTexture(tt, 0);

	if (hasFixedTexture()) tex = fixedTexture;


	float		br = 1;
	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tt->getLightColor( level, x, y, z ) );
	}
	else
	{
		br = tt->getBrightness( level, x, y, z );
	}
	{
		int		col = tt->getColor( level, x, y, z );
		float	r = ( ( col >> 16 ) & 0xff ) / 255.0f;
		float	g = ( ( col >> 8 ) & 0xff ) / 255.0f;
		float	b = ( ( col )& 0xff ) / 255.0f;

		t->color( br * r, br * g, br * b );
	}

	float u0 = tex->getU0();
	float v0 = tex->getV0();
	float u1 = tex->getU1();
	float v1 = tex->getV1();

	float		r = 0.05f;
	int			facings = level->getData( x, y, z );

	if ( ( facings & VineTile_SPU::VINE_WEST ) != 0 )
	{
		t->vertexUV( x + r, y + 1, z + 1, u0, v0 );
		t->vertexUV( x + r, y + 0, z + 1, u0, v1 );
		t->vertexUV( x + r, y + 0, z + 0, u1, v1 );
		t->vertexUV( x + r, y + 1, z + 0, u1, v0 );

		t->vertexUV( x + r, y + 1, z + 0, u1, v0 );
		t->vertexUV( x + r, y + 0, z + 0, u1, v1 );
		t->vertexUV( x + r, y + 0, z + 1, u0, v1 );
		t->vertexUV( x + r, y + 1, z + 1, u0, v0 );
	}
	if ( ( facings & VineTile_SPU::VINE_EAST ) != 0 )
	{
		t->vertexUV( x + 1 - r, y + 0, z + 1, u1, v1 );
		t->vertexUV( x + 1 - r, y + 1, z + 1, u1, v0 );
		t->vertexUV( x + 1 - r, y + 1, z + 0, u0, v0 );
		t->vertexUV( x + 1 - r, y + 0, z + 0, u0, v1 );

		t->vertexUV( x + 1 - r, y + 0, z + 0, u0, v1 );
		t->vertexUV( x + 1 - r, y + 1, z + 0, u0, v0 );
		t->vertexUV( x + 1 - r, y + 1, z + 1, u1, v0 );
		t->vertexUV( x + 1 - r, y + 0, z + 1, u1, v1 );
	}
	if ( ( facings & VineTile_SPU::VINE_NORTH ) != 0 )
	{
		t->vertexUV( x + 1, y + 0, z + r, u1, v1 );
		t->vertexUV( x + 1, y + 1, z + r, u1, v0 );
		t->vertexUV( x + 0, y + 1, z + r, u0, v0 );
		t->vertexUV( x + 0, y + 0, z + r, u0, v1 );

		t->vertexUV( x + 0, y + 0, z + r, u0, v1 );
		t->vertexUV( x + 0, y + 1, z + r, u0, v0 );
		t->vertexUV( x + 1, y + 1, z + r, u1, v0 );
		t->vertexUV( x + 1, y + 0, z + r, u1, v1 );
	}
	if ( ( facings & VineTile_SPU::VINE_SOUTH ) != 0 )
	{
		t->vertexUV( x + 1, y + 1, z + 1 - r, u0, v0 );
		t->vertexUV( x + 1, y + 0, z + 1 - r, u0, v1 );
		t->vertexUV( x + 0, y + 0, z + 1 - r, u1, v1 );
		t->vertexUV( x + 0, y + 1, z + 1 - r, u1, v0 );

		t->vertexUV( x + 0, y + 1, z + 1 - r, u1, v0 );
		t->vertexUV( x + 0, y + 0, z + 1 - r, u1, v1 );
		t->vertexUV( x + 1, y + 0, z + 1 - r, u0, v1 );
		t->vertexUV( x + 1, y + 1, z + 1 - r, u0, v0 );
	}
	if ( level->isSolidBlockingTile( x, y + 1, z ) )
	{
		t->vertexUV( x + 1, y + 1 - r, z + 0, u0, v0 );
		t->vertexUV( x + 1, y + 1 - r, z + 1, u0, v1 );
		t->vertexUV( x + 0, y + 1 - r, z + 1, u1, v1 );
		t->vertexUV( x + 0, y + 1 - r, z + 0, u1, v0 );
	}
	return true;
}

bool TileRenderer_SPU::tesselateThinFenceInWorld( ThinFenceTile* tt, int x, int y, int z )
{
#ifdef DISABLE_TESS_FUNCS
	int				depth = level->getDepth();
	Tesselator_SPU* t = getTesselator();

	float			br;
	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tt->getLightColor( level, x, y, z ) );
		br = 1;
	}
	else
	{
		br = tt->getBrightness( level, x, y, z );
	}
	int				col = tt->getColor( level, x, y, z );
	float			r = ( ( col >> 16 ) & 0xff ) / 255.0f;
	float			g = ( ( col >> 8 ) & 0xff ) / 255.0f;
	float			b = ( ( col )& 0xff ) / 255.0f;

	if ( GameRenderer::anaglyph3d )
	{
		float	cr = ( r * 30 + g * 59 + b * 11 ) / 100;
		float	cg = ( r * 30 + g * 70 ) / ( 100 );
		float	cb = ( r * 30 + b * 70 ) / ( 100 );

		r = cr;
		g = cg;
		b = cb;
	}
	t->color( br * r, br * g, br * b );

	Icon_SPU *tex;
	Icon_SPU *edgeTex;

	if ( hasFixedTexture() )
	{
		tex = fixedTexture;
		edgeTex = fixedTexture;
	}
	else
	{
		int data = level->getData( x, y, z );
		tex = getTexture( tt, 0, data );
		edgeTex = tt->getEdgeTexture();
	}

	int xt = tex->getX();
	int yt = tex->getY();
	float u0 = tex->getU0();
	float u1 = tex->getU(8);
	float u2 = tex->getU1();
	float v0 = tex->getV0();
	float v2 = tex->getV1();

	int xet = edgeTex->getX();
	int yet = edgeTex->getY();

	float iu0 = edgeTex->getU(7);
	float iu1 = edgeTex->getU(9);
	float iv0 = edgeTex->getV0();
	float iv1 = edgeTex->getV(8);
	float iv2 = edgeTex->getV1();

	float			x0 = (float)x;
	float			x1 = x + 0.5f;
	float			x2 = x + 1.0f;
	float			z0 = (float)z;
	float			z1 = z + 0.5f;
	float			z2 = z + 1.0f;
	float			ix0 = x + 0.5f - 1.0f / 16.0f;
	float			ix1 = x + 0.5f + 1.0f / 16.0f;
	float			iz0 = z + 0.5f - 1.0f / 16.0f;
	float			iz1 = z + 0.5f + 1.0f / 16.0f;

	bool			n = tt->attachsTo( level->getTile( x, y, z - 1 ) );
	bool			s = tt->attachsTo( level->getTile( x, y, z + 1 ) );
	bool			w = tt->attachsTo( level->getTile( x - 1, y, z ) );
	bool			e = tt->attachsTo( level->getTile( x + 1, y, z ) );

	bool			up = tt->shouldRenderFace( level, x, y + 1, z, Facing::UP );
	bool			down = tt->shouldRenderFace( level, x, y - 1, z, Facing::DOWN );

	const float	noZFightingOffset = 0.01f;
	const float noZFightingOffsetB = 0.005;

	if ( ( w && e ) || ( !w && !e && !n && !s ) )
	{
		t->vertexUV( x0, y + 1, z1, u0, v0 );
		t->vertexUV( x0, y + 0, z1, u0, v2 );
		t->vertexUV( x2, y + 0, z1, u2, v2 );
		t->vertexUV( x2, y + 1, z1, u2, v0 );

		t->vertexUV( x2, y + 1, z1, u0, v0 );
		t->vertexUV( x2, y + 0, z1, u0, v2 );
		t->vertexUV( x0, y + 0, z1, u2, v2 );
		t->vertexUV( x0, y + 1, z1, u2, v0 );

		if ( up )
		{
			// small edge texture
			t->vertexUV( x0, y + 1 + noZFightingOffset, iz1, iu1, iv2 );
			t->vertexUV( x2, y + 1 + noZFightingOffset, iz1, iu1, iv0 );
			t->vertexUV( x2, y + 1 + noZFightingOffset, iz0, iu0, iv0 );
			t->vertexUV( x0, y + 1 + noZFightingOffset, iz0, iu0, iv2 );

			t->vertexUV( x2, y + 1 + noZFightingOffset, iz1, iu1, iv2 );
			t->vertexUV( x0, y + 1 + noZFightingOffset, iz1, iu1, iv0 );
			t->vertexUV( x0, y + 1 + noZFightingOffset, iz0, iu0, iv0 );
			t->vertexUV( x2, y + 1 + noZFightingOffset, iz0, iu0, iv2 );
		}
		else
		{
			if ( y < ( depth - 1 ) && level->isEmptyTile( x - 1, y + 1, z ) )
			{
				t->vertexUV( x0, y + 1 + noZFightingOffset, iz1, iu1, iv1 );
				t->vertexUV( x1, y + 1 + noZFightingOffset, iz1, iu1, iv2 );
				t->vertexUV( x1, y + 1 + noZFightingOffset, iz0, iu0, iv2 );
				t->vertexUV( x0, y + 1 + noZFightingOffset, iz0, iu0, iv1 );

				t->vertexUV( x1, y + 1 + noZFightingOffset, iz1, iu1, iv1 );
				t->vertexUV( x0, y + 1 + noZFightingOffset, iz1, iu1, iv2 );
				t->vertexUV( x0, y + 1 + noZFightingOffset, iz0, iu0, iv2 );
				t->vertexUV( x1, y + 1 + noZFightingOffset, iz0, iu0, iv1 );
			}
			if ( y < ( depth - 1 ) && level->isEmptyTile( x + 1, y + 1, z ) )
			{
				t->vertexUV( x1, y + 1 + noZFightingOffset, iz1, iu1, iv0 );
				t->vertexUV( x2, y + 1 + noZFightingOffset, iz1, iu1, iv1 );
				t->vertexUV( x2, y + 1 + noZFightingOffset, iz0, iu0, iv1 );
				t->vertexUV( x1, y + 1 + noZFightingOffset, iz0, iu0, iv0 );

				t->vertexUV( x2, y + 1 + noZFightingOffset, iz1, iu1, iv0 );
				t->vertexUV( x1, y + 1 + noZFightingOffset, iz1, iu1, iv1 );
				t->vertexUV( x1, y + 1 + noZFightingOffset, iz0, iu0, iv1 );
				t->vertexUV( x2, y + 1 + noZFightingOffset, iz0, iu0, iv0 );
			}
		}
		if ( down )
		{
			// small edge texture
			t->vertexUV( x0, y - noZFightingOffset, iz1, iu1, iv2 );
			t->vertexUV( x2, y - noZFightingOffset, iz1, iu1, iv0 );
			t->vertexUV( x2, y - noZFightingOffset, iz0, iu0, iv0 );
			t->vertexUV( x0, y - noZFightingOffset, iz0, iu0, iv2 );

			t->vertexUV( x2, y - noZFightingOffset, iz1, iu1, iv2 );
			t->vertexUV( x0, y - noZFightingOffset, iz1, iu1, iv0 );
			t->vertexUV( x0, y - noZFightingOffset, iz0, iu0, iv0 );
			t->vertexUV( x2, y - noZFightingOffset, iz0, iu0, iv2 );
		}
		else
		{
			if ( y > 1 && level->isEmptyTile( x - 1, y - 1, z ) )
			{
				t->vertexUV( x0, y - noZFightingOffset, iz1, iu1, iv1 );
				t->vertexUV( x1, y - noZFightingOffset, iz1, iu1, iv2 );
				t->vertexUV( x1, y - noZFightingOffset, iz0, iu0, iv2 );
				t->vertexUV( x0, y - noZFightingOffset, iz0, iu0, iv1 );

				t->vertexUV( x1, y - noZFightingOffset, iz1, iu1, iv1 );
				t->vertexUV( x0, y - noZFightingOffset, iz1, iu1, iv2 );
				t->vertexUV( x0, y - noZFightingOffset, iz0, iu0, iv2 );
				t->vertexUV( x1, y - noZFightingOffset, iz0, iu0, iv1 );
			}
			if ( y > 1 && level->isEmptyTile( x + 1, y - 1, z ) )
			{
				t->vertexUV( x1, y - noZFightingOffset, iz1, iu1, iv0 );
				t->vertexUV( x2, y - noZFightingOffset, iz1, iu1, iv1 );
				t->vertexUV( x2, y - noZFightingOffset, iz0, iu0, iv1 );
				t->vertexUV( x1, y - noZFightingOffset, iz0, iu0, iv0 );

				t->vertexUV( x2, y - noZFightingOffset, iz1, iu1, iv0 );
				t->vertexUV( x1, y - noZFightingOffset, iz1, iu1, iv1 );
				t->vertexUV( x1, y - noZFightingOffset, iz0, iu0, iv1 );
				t->vertexUV( x2, y - noZFightingOffset, iz0, iu0, iv0 );
			}
		}
	}
	else if ( w && !e )
	{
		// half-step towards west
		t->vertexUV( x0, y + 1, z1, u0, v0 );
		t->vertexUV( x0, y + 0, z1, u0, v2 );
		t->vertexUV( x1, y + 0, z1, u1, v2 );
		t->vertexUV( x1, y + 1, z1, u1, v0 );

		t->vertexUV( x1, y + 1, z1, u0, v0 );
		t->vertexUV( x1, y + 0, z1, u0, v2 );
		t->vertexUV( x0, y + 0, z1, u1, v2 );
		t->vertexUV( x0, y + 1, z1, u1, v0 );

		// small edge texture
		if ( !s && !n )
		{
			t->vertexUV( x1, y + 1, iz1, iu0, iv0 );
			t->vertexUV( x1, y + 0, iz1, iu0, iv2 );
			t->vertexUV( x1, y + 0, iz0, iu1, iv2 );
			t->vertexUV( x1, y + 1, iz0, iu1, iv0 );

			t->vertexUV( x1, y + 1, iz0, iu0, iv0 );
			t->vertexUV( x1, y + 0, iz0, iu0, iv2 );
			t->vertexUV( x1, y + 0, iz1, iu1, iv2 );
			t->vertexUV( x1, y + 1, iz1, iu1, iv0 );
		}

		if ( up || ( y < ( depth - 1 ) && level->isEmptyTile( x - 1, y + 1, z ) ) )
		{
			// small edge texture
			t->vertexUV( x0, y + 1 + noZFightingOffset, iz1, iu1, iv1 );
			t->vertexUV( x1, y + 1 + noZFightingOffset, iz1, iu1, iv2 );
			t->vertexUV( x1, y + 1 + noZFightingOffset, iz0, iu0, iv2 );
			t->vertexUV( x0, y + 1 + noZFightingOffset, iz0, iu0, iv1 );

			t->vertexUV( x1, y + 1 + noZFightingOffset, iz1, iu1, iv1 );
			t->vertexUV( x0, y + 1 + noZFightingOffset, iz1, iu1, iv2 );
			t->vertexUV( x0, y + 1 + noZFightingOffset, iz0, iu0, iv2 );
			t->vertexUV( x1, y + 1 + noZFightingOffset, iz0, iu0, iv1 );
		}
		if ( down || ( y > 1 && level->isEmptyTile( x - 1, y - 1, z ) ) )
		{
			// small edge texture
			t->vertexUV( x0, y - noZFightingOffset, iz1, iu1, iv1 );
			t->vertexUV( x1, y - noZFightingOffset, iz1, iu1, iv2 );
			t->vertexUV( x1, y - noZFightingOffset, iz0, iu0, iv2 );
			t->vertexUV( x0, y - noZFightingOffset, iz0, iu0, iv1 );

			t->vertexUV( x1, y - noZFightingOffset, iz1, iu1, iv1 );
			t->vertexUV( x0, y - noZFightingOffset, iz1, iu1, iv2 );
			t->vertexUV( x0, y - noZFightingOffset, iz0, iu0, iv2 );
			t->vertexUV( x1, y - noZFightingOffset, iz0, iu0, iv1 );
		}

	}
	else if ( !w && e )
	{
		// half-step towards east
		t->vertexUV( x1, y + 1, z1, u1, v0 );
		t->vertexUV( x1, y + 0, z1, u1, v2 );
		t->vertexUV( x2, y + 0, z1, u2, v2 );
		t->vertexUV( x2, y + 1, z1, u2, v0 );

		t->vertexUV( x2, y + 1, z1, u1, v0 );
		t->vertexUV( x2, y + 0, z1, u1, v2 );
		t->vertexUV( x1, y + 0, z1, u2, v2 );
		t->vertexUV( x1, y + 1, z1, u2, v0 );

		// small edge texture
		if ( !s && !n )
		{
			t->vertexUV( x1, y + 1, iz0, iu0, iv0 );
			t->vertexUV( x1, y + 0, iz0, iu0, iv2 );
			t->vertexUV( x1, y + 0, iz1, iu1, iv2 );
			t->vertexUV( x1, y + 1, iz1, iu1, iv0 );

			t->vertexUV( x1, y + 1, iz1, iu0, iv0 );
			t->vertexUV( x1, y + 0, iz1, iu0, iv2 );
			t->vertexUV( x1, y + 0, iz0, iu1, iv2 );
			t->vertexUV( x1, y + 1, iz0, iu1, iv0 );
		}

		if ( up || ( y < ( depth - 1 ) && level->isEmptyTile( x + 1, y + 1, z ) ) )
		{
			// small edge texture
			t->vertexUV( x1, y + 1 + noZFightingOffset, iz1, iu1, iv0 );
			t->vertexUV( x2, y + 1 + noZFightingOffset, iz1, iu1, iv1 );
			t->vertexUV( x2, y + 1 + noZFightingOffset, iz0, iu0, iv1 );
			t->vertexUV( x1, y + 1 + noZFightingOffset, iz0, iu0, iv0 );

			t->vertexUV( x2, y + 1 + noZFightingOffset, iz1, iu1, iv0 );
			t->vertexUV( x1, y + 1 + noZFightingOffset, iz1, iu1, iv1 );
			t->vertexUV( x1, y + 1 + noZFightingOffset, iz0, iu0, iv1 );
			t->vertexUV( x2, y + 1 + noZFightingOffset, iz0, iu0, iv0 );
		}
		if ( down || ( y > 1 && level->isEmptyTile( x + 1, y - 1, z ) ) )
		{
			// small edge texture
			t->vertexUV( x1, y - noZFightingOffset, iz1, iu1, iv0 );
			t->vertexUV( x2, y - noZFightingOffset, iz1, iu1, iv1 );
			t->vertexUV( x2, y - noZFightingOffset, iz0, iu0, iv1 );
			t->vertexUV( x1, y - noZFightingOffset, iz0, iu0, iv0 );

			t->vertexUV( x2, y - noZFightingOffset, iz1, iu1, iv0 );
			t->vertexUV( x1, y - noZFightingOffset, iz1, iu1, iv1 );
			t->vertexUV( x1, y - noZFightingOffset, iz0, iu0, iv1 );
			t->vertexUV( x2, y - noZFightingOffset, iz0, iu0, iv0 );
		}

	}

	if ( ( n && s ) || ( !w && !e && !n && !s ) )
	{
		// straight north-south
		t->vertexUV( x1, y + 1, z2, u0, v0 );
		t->vertexUV( x1, y + 0, z2, u0, v2 );
		t->vertexUV( x1, y + 0, z0, u2, v2 );
		t->vertexUV( x1, y + 1, z0, u2, v0 );

		t->vertexUV( x1, y + 1, z0, u0, v0 );
		t->vertexUV( x1, y + 0, z0, u0, v2 );
		t->vertexUV( x1, y + 0, z2, u2, v2 );
		t->vertexUV( x1, y + 1, z2, u2, v0 );

		if ( up )
		{
			// small edge texture
			t->vertexUV( ix1, y + 1 + noZFightingOffset, z2, iu1, iv2 );
			t->vertexUV( ix1, y + 1 + noZFightingOffset, z0, iu1, iv0 );
			t->vertexUV( ix0, y + 1 + noZFightingOffset, z0, iu0, iv0 );
			t->vertexUV( ix0, y + 1 + noZFightingOffset, z2, iu0, iv2 );

			t->vertexUV( ix1, y + 1 + noZFightingOffset, z0, iu1, iv2 );
			t->vertexUV( ix1, y + 1 + noZFightingOffset, z2, iu1, iv0 );
			t->vertexUV( ix0, y + 1 + noZFightingOffset, z2, iu0, iv0 );
			t->vertexUV( ix0, y + 1 + noZFightingOffset, z0, iu0, iv2 );
		}
		else
		{
			if ( y < ( depth - 1 ) && level->isEmptyTile( x, y + 1, z - 1 ) )
			{
				t->vertexUV( ix0, y + 1 + noZFightingOffset, z0, iu1, iv0 );
				t->vertexUV( ix0, y + 1 + noZFightingOffset, z1, iu1, iv1 );
				t->vertexUV( ix1, y + 1 + noZFightingOffset, z1, iu0, iv1 );
				t->vertexUV( ix1, y + 1 + noZFightingOffset, z0, iu0, iv0 );

				t->vertexUV( ix0, y + 1 + noZFightingOffset, z1, iu1, iv0 );
				t->vertexUV( ix0, y + 1 + noZFightingOffset, z0, iu1, iv1 );
				t->vertexUV( ix1, y + 1 + noZFightingOffset, z0, iu0, iv1 );
				t->vertexUV( ix1, y + 1 + noZFightingOffset, z1, iu0, iv0 );
			}
			if ( y < ( depth - 1 ) && level->isEmptyTile( x, y + 1, z + 1 ) )
			{
				t->vertexUV( ix0, y + 1 + noZFightingOffset, z1, iu0, iv1 );
				t->vertexUV( ix0, y + 1 + noZFightingOffset, z2, iu0, iv2 );
				t->vertexUV( ix1, y + 1 + noZFightingOffset, z2, iu1, iv2 );
				t->vertexUV( ix1, y + 1 + noZFightingOffset, z1, iu1, iv1 );

				t->vertexUV( ix0, y + 1 + noZFightingOffset, z2, iu0, iv1 );
				t->vertexUV( ix0, y + 1 + noZFightingOffset, z1, iu0, iv2 );
				t->vertexUV( ix1, y + 1 + noZFightingOffset, z1, iu1, iv2 );
				t->vertexUV( ix1, y + 1 + noZFightingOffset, z2, iu1, iv1 );
			}
		}
		if ( down )
		{
			// small edge texture
			t->vertexUV( ix1, y - noZFightingOffset, z2, iu1, iv2 );
			t->vertexUV( ix1, y - noZFightingOffset, z0, iu1, iv0 );
			t->vertexUV( ix0, y - noZFightingOffset, z0, iu0, iv0 );
			t->vertexUV( ix0, y - noZFightingOffset, z2, iu0, iv2 );

			t->vertexUV( ix1, y - noZFightingOffset, z0, iu1, iv2 );
			t->vertexUV( ix1, y - noZFightingOffset, z2, iu1, iv0 );
			t->vertexUV( ix0, y - noZFightingOffset, z2, iu0, iv0 );
			t->vertexUV( ix0, y - noZFightingOffset, z0, iu0, iv2 );
		}
		else
		{
			if ( y > 1 && level->isEmptyTile( x, y - 1, z - 1 ) )
			{
				// north half-step
				t->vertexUV( ix0, y - noZFightingOffset, z0, iu1, iv0 );
				t->vertexUV( ix0, y - noZFightingOffset, z1, iu1, iv1 );
				t->vertexUV( ix1, y - noZFightingOffset, z1, iu0, iv1 );
				t->vertexUV( ix1, y - noZFightingOffset, z0, iu0, iv0 );

				t->vertexUV( ix0, y - noZFightingOffset, z1, iu1, iv0 );
				t->vertexUV( ix0, y - noZFightingOffset, z0, iu1, iv1 );
				t->vertexUV( ix1, y - noZFightingOffset, z0, iu0, iv1 );
				t->vertexUV( ix1, y - noZFightingOffset, z1, iu0, iv0 );
			}
			if ( y > 1 && level->isEmptyTile( x, y - 1, z + 1 ) )
			{
				// south half-step
				t->vertexUV( ix0, y - noZFightingOffset, z1, iu0, iv1 );
				t->vertexUV( ix0, y - noZFightingOffset, z2, iu0, iv2 );
				t->vertexUV( ix1, y - noZFightingOffset, z2, iu1, iv2 );
				t->vertexUV( ix1, y - noZFightingOffset, z1, iu1, iv1 );

				t->vertexUV( ix0, y - noZFightingOffset, z2, iu0, iv1 );
				t->vertexUV( ix0, y - noZFightingOffset, z1, iu0, iv2 );
				t->vertexUV( ix1, y - noZFightingOffset, z1, iu1, iv2 );
				t->vertexUV( ix1, y - noZFightingOffset, z2, iu1, iv1 );
			}
		}

	}
	else if ( n && !s )
	{
		// half-step towards north
		t->vertexUV( x1, y + 1, z0, u0, v0 );
		t->vertexUV( x1, y + 0, z0, u0, v2 );
		t->vertexUV( x1, y + 0, z1, u1, v2 );
		t->vertexUV( x1, y + 1, z1, u1, v0 );

		t->vertexUV( x1, y + 1, z1, u0, v0 );
		t->vertexUV( x1, y + 0, z1, u0, v2 );
		t->vertexUV( x1, y + 0, z0, u1, v2 );
		t->vertexUV( x1, y + 1, z0, u1, v0 );

		// small edge texture
		if ( !e && !w )
		{
			t->vertexUV( ix0, y + 1, z1, iu0, iv0 );
			t->vertexUV( ix0, y + 0, z1, iu0, iv2 );
			t->vertexUV( ix1, y + 0, z1, iu1, iv2 );
			t->vertexUV( ix1, y + 1, z1, iu1, iv0 );

			t->vertexUV( ix1, y + 1, z1, iu0, iv0 );
			t->vertexUV( ix1, y + 0, z1, iu0, iv2 );
			t->vertexUV( ix0, y + 0, z1, iu1, iv2 );
			t->vertexUV( ix0, y + 1, z1, iu1, iv0 );
		}

		if ( up || ( y < ( depth - 1 ) && level->isEmptyTile( x, y + 1, z - 1 ) ) )
		{
			// small edge texture
			t->vertexUV( ix0, y + 1 + noZFightingOffset, z0, iu1, iv0 );
			t->vertexUV( ix0, y + 1 + noZFightingOffset, z1, iu1, iv1 );
			t->vertexUV( ix1, y + 1 + noZFightingOffset, z1, iu0, iv1 );
			t->vertexUV( ix1, y + 1 + noZFightingOffset, z0, iu0, iv0 );

			t->vertexUV( ix0, y + 1 + noZFightingOffset, z1, iu1, iv0 );
			t->vertexUV( ix0, y + 1 + noZFightingOffset, z0, iu1, iv1 );
			t->vertexUV( ix1, y + 1 + noZFightingOffset, z0, iu0, iv1 );
			t->vertexUV( ix1, y + 1 + noZFightingOffset, z1, iu0, iv0 );
		}

		if ( down || ( y > 1 && level->isEmptyTile( x, y - 1, z - 1 ) ) )
		{
			// small edge texture
			t->vertexUV( ix0, y - noZFightingOffset, z0, iu1, iv0 );
			t->vertexUV( ix0, y - noZFightingOffset, z1, iu1, iv1 );
			t->vertexUV( ix1, y - noZFightingOffset, z1, iu0, iv1 );
			t->vertexUV( ix1, y - noZFightingOffset, z0, iu0, iv0 );

			t->vertexUV( ix0, y - noZFightingOffset, z1, iu1, iv0 );
			t->vertexUV( ix0, y - noZFightingOffset, z0, iu1, iv1 );
			t->vertexUV( ix1, y - noZFightingOffset, z0, iu0, iv1 );
			t->vertexUV( ix1, y - noZFightingOffset, z1, iu0, iv0 );
		}

	}
	else if ( !n && s )
	{
		// half-step towards south
		t->vertexUV( x1, y + 1, z1, u1, v0 );
		t->vertexUV( x1, y + 0, z1, u1, v2 );
		t->vertexUV( x1, y + 0, z2, u2, v2 );
		t->vertexUV( x1, y + 1, z2, u2, v0 );

		t->vertexUV( x1, y + 1, z2, u1, v0 );
		t->vertexUV( x1, y + 0, z2, u1, v2 );
		t->vertexUV( x1, y + 0, z1, u2, v2 );
		t->vertexUV( x1, y + 1, z1, u2, v0 );

		// small edge texture
		if ( !e && !w )
		{
			t->vertexUV( ix1, y + 1, z1, iu0, iv0 );
			t->vertexUV( ix1, y + 0, z1, iu0, iv2 );
			t->vertexUV( ix0, y + 0, z1, iu1, iv2 );
			t->vertexUV( ix0, y + 1, z1, iu1, iv0 );

			t->vertexUV( ix0, y + 1, z1, iu0, iv0 );
			t->vertexUV( ix0, y + 0, z1, iu0, iv2 );
			t->vertexUV( ix1, y + 0, z1, iu1, iv2 );
			t->vertexUV( ix1, y + 1, z1, iu1, iv0 );
		}

		if ( up || ( y < ( depth - 1 ) && level->isEmptyTile( x, y + 1, z + 1 ) ) )
		{
			// small edge texture
			t->vertexUV( ix0, y + 1 + noZFightingOffset, z1, iu0, iv1 );
			t->vertexUV( ix0, y + 1 + noZFightingOffset, z2, iu0, iv2 );
			t->vertexUV( ix1, y + 1 + noZFightingOffset, z2, iu1, iv2 );
			t->vertexUV( ix1, y + 1 + noZFightingOffset, z1, iu1, iv1 );

			t->vertexUV( ix0, y + 1 + noZFightingOffset, z2, iu0, iv1 );
			t->vertexUV( ix0, y + 1 + noZFightingOffset, z1, iu0, iv2 );
			t->vertexUV( ix1, y + 1 + noZFightingOffset, z1, iu1, iv2 );
			t->vertexUV( ix1, y + 1 + noZFightingOffset, z2, iu1, iv1 );
		}
		if ( down || ( y > 1 && level->isEmptyTile( x, y - 1, z + 1 ) ) )
		{
			// small edge texture
			t->vertexUV( ix0, y - noZFightingOffset, z1, iu0, iv1 );
			t->vertexUV( ix0, y - noZFightingOffset, z2, iu0, iv2 );
			t->vertexUV( ix1, y - noZFightingOffset, z2, iu1, iv2 );
			t->vertexUV( ix1, y - noZFightingOffset, z1, iu1, iv1 );

			t->vertexUV( ix0, y - noZFightingOffset, z2, iu0, iv1 );
			t->vertexUV( ix0, y - noZFightingOffset, z1, iu0, iv2 );
			t->vertexUV( ix1, y - noZFightingOffset, z1, iu1, iv2 );
			t->vertexUV( ix1, y - noZFightingOffset, z2, iu1, iv1 );
		}

	}
#endif // DISABLE_TESS_FUNCS

	return true;
}

bool TileRenderer_SPU::tesselateCrossInWorld( Tile_SPU* tt, int x, int y, int z )
{
	Tesselator_SPU* t = getTesselator();

	float		br;
	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tt->getLightColor( level, x, y, z ) );
		br = 1;
	}
	else
	{
		br = tt->getBrightness( level, x, y, z );
	}

	int			col = tt->getColor( level, x, y, z );
	float		r = ( ( col >> 16 ) & 0xff ) / 255.0f;
	float		g = ( ( col >> 8 ) & 0xff ) / 255.0f;
	float		b = ( ( col )& 0xff ) / 255.0f;

	if ( isAnaglyph3d() )
	{
		float	cr = ( r * 30 + g * 59 + b * 11 ) / 100;
		float	cg = ( r * 30 + g * 70 ) / ( 100 );
		float	cb = ( r * 30 + b * 70 ) / ( 100 );

		r = cr;
		g = cg;
		b = cb;
	}
	t->color( br * r, br * g, br * b );

	float		xt = static_cast<float>(x);
	float		yt = static_cast<float>(y);
	float		zt = static_cast<float>(z);

	if (tt->id == Tile_SPU::tallgrass_Id)
	{
	    int64_t seed = (x * 3129871) ^ (z * 116129781l) ^ (y);
	    seed = seed * seed * 42317861 + seed * 11;
	
		xt += ((((seed >> 16) & 0xf) / 15.0f) - 0.5f) * 0.5f;
		yt += ((((seed >> 20) & 0xf) / 15.0f) - 1.0f) * 0.2f;
		zt += ((((seed >> 24) & 0xf) / 15.0f) - 0.5f) * 0.5f;
	}

	tesselateCrossTexture( tt, level->getData( x, y, z ), xt, yt, zt, 1 );
	return true;
}

bool TileRenderer_SPU::tesselateStemInWorld( Tile_SPU* _tt, int x, int y, int z )
{
	StemTile_SPU*	tt = static_cast<StemTile_SPU *>(_tt);
	Tesselator_SPU* t = getTesselator();

	float		br;
	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tt->getLightColor( level, x, y, z ) );
		br = 1;
	}
	else
	{
		br = tt->getBrightness( level, x, y, z );
	}
	int			col = tt->getColor( level, x, y, z );
	float		r = ( ( col >> 16 ) & 0xff ) / 255.0f;
	float		g = ( ( col >> 8 ) & 0xff ) / 255.0f;
	float		b = ( ( col )& 0xff ) / 255.0f;

	if ( isAnaglyph3d())
	{
		float	cr = ( r * 30.0f + g * 59.0f + b * 11.0f ) / 100.0f;
		float	cg = ( r * 30.0f + g * 70.0f ) / ( 100.0f );
		float	cb = ( r * 30.0f + b * 70.0f ) / ( 100.0f );

		r = cr;
		g = cg;
		b = cb;
	}
	t->color( br * r, br * g, br * b );

	tt->updateShape( level, x, y, z );
	int			dir = tt->getConnectDir( level, x, y, z );
	if ( dir < 0 )
	{
		tesselateStemTexture( tt, level->getData( x, y, z ), tileShapeY1, x, y - 1 / 16.0f, z );
	}
	else
	{
		tesselateStemTexture( tt, level->getData( x, y, z ), 0.5f, x, y - 1 / 16.0f, z );
		tesselateStemDirTexture( tt, level->getData( x, y, z ), dir, tileShapeY1, x, y - 1 / 16.0f, z );
	}
	return true;
}

bool TileRenderer_SPU::tesselateRowInWorld( Tile_SPU* tt, int x, int y, int z )
{
	Tesselator_SPU* t = getTesselator();

	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tt->getLightColor( level, x, y, z ) );
		t->color( 1.0f, 1.0f, 1.0f );
	}
	else
	{
		float br = tt->getBrightness( level, x, y, z );
		t->color( br, br, br );
	}

	tesselateRowTexture( tt, level->getData( x, y, z ), x, y - 1.0f / 16.0f, z );
	return true;
}

void TileRenderer_SPU::tesselateTorch( Tile_SPU* tt, float x, float y, float z, float xxa, float zza, int data )
{
	Tesselator_SPU* t = getTesselator();

	Icon_SPU *tex = getTexture(tt, Facing::DOWN, data);

	if (hasFixedTexture()) tex = fixedTexture;
	float u0 = tex->getU0();
	float v0 = tex->getV0();
	float u1 = tex->getU1();
	float v1 = tex->getV1();

	float ut0 = tex->getU(7);
	float vt0 = tex->getV(6);
	float ut1 = tex->getU(9);
	float vt1 = tex->getV(8);

	float ub0 = tex->getU(7);
	float vb0 = tex->getV(13);
	float ub1 = tex->getU(9);
	float vb1 = tex->getV(15);

	x += 0.5f;
	z += 0.5f;

	float		x0 = x - 0.5f;
	float		x1 = x + 0.5f;
	float		z0 = z - 0.5f;
	float		z1 = z + 0.5f;
	float		r = 1 / 16.0f;

	float		h = 10.0f / 16.0f;
	t->vertexUV( ( float )( x + xxa * ( 1 - h ) - r ), ( float )( y + h ), ( float )( z + zza * ( 1 - h ) - r ), ut0, vt0 );
	t->vertexUV( ( float )( x + xxa * ( 1 - h ) - r ), ( float )( y + h ), ( float )( z + zza * ( 1 - h ) + r ), ut0, vt1 );
	t->vertexUV( ( float )( x + xxa * ( 1 - h ) + r ), ( float )( y + h ), ( float )( z + zza * ( 1 - h ) + r ), ut1, vt1 );
	t->vertexUV( ( float )( x + xxa * ( 1 - h ) + r ), ( float )( y + h ), ( float )( z + zza * ( 1 - h ) - r ), ut1, vt0 );

	t->vertexUV( (float)(x + r + xxa), (float) y, (float)(z - r + zza), ub1, vb0);
	t->vertexUV( (float)(x + r + xxa), (float) y, (float)(z + r + zza), ub1, vb1);
	t->vertexUV( (float)(x - r + xxa), (float) y, (float)(z + r + zza), ub0, vb1);
	t->vertexUV( (float)(x - r + xxa), (float) y, (float)(z - r + zza), ub0, vb0);

	t->vertexUV( ( float )( x - r ), ( float )( y + 1 ), ( float )( z0 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x - r + xxa ), ( float )( y + 0 ), ( float )( z0 +
				 zza ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x - r + xxa ), ( float )( y + 0 ), ( float )( z1 +
				 zza ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x - r ), ( float )( y + 1 ), ( float )( z1 ), ( float )( u1 ), ( float )( v0 ) );

	t->vertexUV( ( float )( x + r ), ( float )( y + 1 ), ( float )( z1 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x + xxa + r ), ( float )( y + 0 ), ( float )( z1 +
				 zza ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x + xxa + r ), ( float )( y + 0 ), ( float )( z0 +
				 zza ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x + r ), ( float )( y + 1 ), ( float )( z0 ), ( float )( u1 ), ( float )( v0 ) );

	t->vertexUV( ( float )( x0 ), ( float )( y + 1 ), ( float )( z + r ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x0 + xxa ), ( float )( y + 0 ), ( float )( z + r +
				 zza ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 + xxa ), ( float )( y + 0 ), ( float )( z + r +
				 zza ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 1 ), ( float )( z + r ), ( float )( u1 ), ( float )( v0 ) );

	t->vertexUV( ( float )( x1 ), ( float )( y + 1 ), ( float )( z - r ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x1 + xxa ), ( float )( y + 0 ), ( float )( z - r +
				 zza ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 + xxa ), ( float )( y + 0 ), ( float )( z - r +
				 zza ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 1 ), ( float )( z - r ), ( float )( u1 ), ( float )( v0 ) );
}

void TileRenderer_SPU::tesselateCrossTexture( Tile_SPU* tt, int data, float x, float y, float z, float scale )
{
	Tesselator_SPU* t = getTesselator();

	Icon_SPU *tex = getTexture(tt, 0, data);

	if (hasFixedTexture()) tex = fixedTexture;
	float u0 = tex->getU0();
	float v0 = tex->getV0();
	float u1 = tex->getU1();
	float v1 = tex->getV1();

	float width = 0.45 * scale;
	float x0 = x + 0.5 - width;
	float x1 = x + 0.5 + width;
	float z0 = z + 0.5 - width;
	float z1 = z + 0.5 + width;

	t->vertexUV( ( float )( x0 ), ( float )( y + scale ), ( float )( z0 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + scale ), ( float )( z1 ), ( float )( u1 ), ( float )( v0 ) );

	t->vertexUV( ( float )( x1 ), ( float )( y + scale ), ( float )( z1 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + scale ), ( float )( z0 ), ( float )( u1 ), ( float )( v0 ) );

	t->vertexUV( ( float )( x0 ), ( float )( y + scale ), ( float )( z1 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + scale ), ( float )( z0 ), ( float )( u1 ), ( float )( v0 ) );

	t->vertexUV( ( float )( x1 ), ( float )( y + scale ), ( float )( z0 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + scale ), ( float )( z1 ), ( float )( u1 ), ( float )( v0 ) );
}

void TileRenderer_SPU::tesselateStemTexture( Tile_SPU* tt, int data, float h, float x, float y, float z )
{
// #ifdef DISABLE_TESS_FUNCS
	Tesselator_SPU* t = getTesselator();

	Icon_SPU *tex = getTexture(tt, 0, data);

	if (hasFixedTexture()) tex = fixedTexture;
	float u0 = tex->getU0();
	float v0 = tex->getV0();
	float u1 = tex->getU1();
	float v1 = tex->getV(h * SharedConstants::WORLD_RESOLUTION);

	float		x0 = x + 0.5f - 0.45f;
	float		x1 = x + 0.5f + 0.45f;
	float		z0 = z + 0.5f - 0.45f;
	float		z1 = z + 0.5f + 0.45f;

	t->vertexUV( x0, y + h, z0, u0, v0 );
	t->vertexUV( x0, y + 0, z0, u0, v1 );
	t->vertexUV( x1, y + 0, z1, u1, v1 );
	t->vertexUV( x1, y + h, z1, u1, v0 );

	t->vertexUV( x1, y + h, z1, u0, v0 );
	t->vertexUV( x1, y + 0, z1, u0, v1 );
	t->vertexUV( x0, y + 0, z0, u1, v1 );
	t->vertexUV( x0, y + h, z0, u1, v0 );

	t->vertexUV( x0, y + h, z1, u0, v0 );
	t->vertexUV( x0, y + 0, z1, u0, v1 );
	t->vertexUV( x1, y + 0, z0, u1, v1 );
	t->vertexUV( x1, y + h, z0, u1, v0 );

	t->vertexUV( x1, y + h, z0, u0, v0 );
	t->vertexUV( x1, y + 0, z0, u0, v1 );
	t->vertexUV( x0, y + 0, z1, u1, v1 );
	t->vertexUV( x0, y + h, z1, u1, v0 );
// #endif // DISABLE_TESS_FUNCS
}

bool TileRenderer_SPU::tesselateLilypadInWorld(WaterlilyTile_SPU *tt, int x, int y, int z)
{
	Tesselator_SPU* t = getTesselator();

	Icon_SPU *tex = getTexture(tt, Facing::UP);

	if (hasFixedTexture()) tex = fixedTexture;
    float h = 0.25f / 16.0f;

	float u0 = tex->getU0();
	float v0 = tex->getV0();
	float u1 = tex->getU1();
	float v1 = tex->getV1();

    int64_t seed = (x * 3129871) ^ (z * 116129781l) ^ (y);
    seed = seed * seed * 42317861 + seed * 11;

    int dir = static_cast<int>((seed >> 16) & 0x3);



    t->tex2(tt->getLightColor(level, x, y, z));

    float xx = x + 0.5f;
    float zz = z + 0.5f;
    float c = ((dir & 1) * 0.5f) * (1 - dir / 2 % 2 * 2);
    float s = (((dir + 1) & 1) * 0.5f) * (1 - (dir + 1) / 2 % 2 * 2);

    t->color(tt->getColor());
    t->vertexUV(xx + c - s, y + h, zz + c + s, u0, v0);
    t->vertexUV(xx + c + s, y + h, zz - c + s, u1, v0);
    t->vertexUV(xx - c + s, y + h, zz - c - s, u1, v1);
    t->vertexUV(xx - c - s, y + h, zz + c - s, u0, v1);

    t->color((tt->getColor() & 0xfefefe) >> 1);
    t->vertexUV(xx - c - s, y + h, zz + c - s, u0, v1);
    t->vertexUV(xx - c + s, y + h, zz - c - s, u1, v1);
    t->vertexUV(xx + c + s, y + h, zz - c + s, u1, v0);
    t->vertexUV(xx + c - s, y + h, zz + c + s, u0, v0);

    return true;
}

void TileRenderer_SPU::tesselateStemDirTexture( StemTile_SPU* tt, int data, int dir, float h, float x, float y, float z )
{
	Tesselator_SPU* t = getTesselator();

	Icon_SPU *tex = tt->getAngledTexture();

	if (hasFixedTexture()) tex = fixedTexture;
	float u0 = tex->getU0();
	float v0 = tex->getV0();
	float u1 = tex->getU1();
	float v1 = tex->getV1();

	float		x0 = x + 0.5f - 0.5f;
	float		x1 = x + 0.5f + 0.5f;
	float		z0 = z + 0.5f - 0.5f;
	float		z1 = z + 0.5f + 0.5f;

	float		xm = x + 0.5f;
	float		zm = z + 0.5f;

	if ( ( dir + 1 ) / 2 % 2 == 1 )
	{
		float tmp = u1;
		u1 = u0;
		u0 = tmp;
	}

	if ( dir < 2 )
	{
		t->vertexUV( x0, y + h, zm, u0, v0 );
		t->vertexUV( x0, y + 0, zm, u0, v1 );
		t->vertexUV( x1, y + 0, zm, u1, v1 );
		t->vertexUV( x1, y + h, zm, u1, v0 );

		t->vertexUV( x1, y + h, zm, u1, v0 );
		t->vertexUV( x1, y + 0, zm, u1, v1 );
		t->vertexUV( x0, y + 0, zm, u0, v1 );
		t->vertexUV( x0, y + h, zm, u0, v0 );
	}
	else
	{
		t->vertexUV( xm, y + h, z1, u0, v0 );
		t->vertexUV( xm, y + 0, z1, u0, v1 );
		t->vertexUV( xm, y + 0, z0, u1, v1 );
		t->vertexUV( xm, y + h, z0, u1, v0 );

		t->vertexUV( xm, y + h, z0, u1, v0 );
		t->vertexUV( xm, y + 0, z0, u1, v1 );
		t->vertexUV( xm, y + 0, z1, u0, v1 );
		t->vertexUV( xm, y + h, z1, u0, v0 );
	}
}


void TileRenderer_SPU::tesselateRowTexture( Tile_SPU* tt, int data, float x, float y, float z )
{
	Tesselator_SPU* t = getTesselator();

	Icon_SPU *tex = getTexture(tt, 0, data);

	if (hasFixedTexture()) tex = fixedTexture;
	float u0 = tex->getU0();
	float v0 = tex->getV0();
	float u1 = tex->getU1();
	float v1 = tex->getV1();

	float		x0 = x + 0.5f - 0.25f;
	float		x1 = x + 0.5f + 0.25f;
	float		z0 = z + 0.5f - 0.5f;
	float		z1 = z + 0.5f + 0.5f;

	t->vertexUV( ( float )( x0 ), ( float )( y + 1 ), ( float )( z0 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 1 ), ( float )( z1 ), ( float )( u1 ), ( float )( v0 ) );

	t->vertexUV( ( float )( x0 ), ( float )( y + 1 ), ( float )( z1 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 1 ), ( float )( z0 ), ( float )( u1 ), ( float )( v0 ) );

	t->vertexUV( ( float )( x1 ), ( float )( y + 1 ), ( float )( z1 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 1 ), ( float )( z0 ), ( float )( u1 ), ( float )( v0 ) );

	t->vertexUV( ( float )( x1 ), ( float )( y + 1 ), ( float )( z0 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 1 ), ( float )( z1 ), ( float )( u1 ), ( float )( v0 ) );

	x0 = x + 0.5f - 0.5f;
	x1 = x + 0.5f + 0.5f;
	z0 = z + 0.5f - 0.25f;
	z1 = z + 0.5f + 0.25f;

	t->vertexUV( ( float )( x0 ), ( float )( y + 1 ), ( float )( z0 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 1 ), ( float )( z0 ), ( float )( u1 ), ( float )( v0 ) );

	t->vertexUV( ( float )( x1 ), ( float )( y + 1 ), ( float )( z0 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 1 ), ( float )( z0 ), ( float )( u1 ), ( float )( v0 ) );

	t->vertexUV( ( float )( x1 ), ( float )( y + 1 ), ( float )( z1 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 1 ), ( float )( z1 ), ( float )( u1 ), ( float )( v0 ) );

	t->vertexUV( ( float )( x0 ), ( float )( y + 1 ), ( float )( z1 ), ( float )( u0 ), ( float )( v0 ) );
	t->vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u0 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
	t->vertexUV( ( float )( x1 ), ( float )( y + 1 ), ( float )( z1 ), ( float )( u1 ), ( float )( v0 ) );

}

bool TileRenderer_SPU::tesselateWaterInWorld( Tile_SPU* tt, int x, int y, int z )
{
	// 4J Java comment
	// TODO: This all needs to change. Somehow.
	Tesselator_SPU* t = getTesselator();

	int			col = tt->getColor( level, x, y, z );
	float		r = ( col >> 16 & 0xff ) / 255.0f;
	float		g = ( col >> 8 & 0xff ) / 255.0f;
	float		b = ( col & 0xff ) / 255.0f;
	bool		up = tt->shouldRenderFace( level, x, y + 1, z, 1 );
	bool		down = tt->shouldRenderFace( level, x, y - 1, z, 0 );
	bool		dirs[4];
	dirs[0] = tt->shouldRenderFace( level, x, y, z - 1, 2 );
	dirs[1] = tt->shouldRenderFace( level, x, y, z + 1, 3 );
	dirs[2] = tt->shouldRenderFace( level, x - 1, y, z, 4 );
	dirs[3] = tt->shouldRenderFace( level, x + 1, y, z, 5 );

	if ( !up && !down && !dirs[0] && !dirs[1] && !dirs[2] && !dirs[3] ) return false;

	bool		changed = false;
	float		c10 = 0.5f;
	float		c11 = 1;
	float		c2 = 0.8f;
	float		c3 = 0.6f;

	float		yo0 = 0;
	float		yo1 = 1;

	Material_SPU*	m = tt->getMaterial();
	int			data = level->getData( x, y, z );

	float		h0 = getWaterHeight( x, y, z, m );
	float		h1 = getWaterHeight( x, y, z + 1, m );
	float		h2 = getWaterHeight( x + 1, y, z + 1, m );
	float		h3 = getWaterHeight( x + 1, y, z, m );

    float offs = 0.001f;
	// 4J - added. Farm tiles often found beside water, but they consider themselves non-solid as they only extend up to 15.0f / 16.0f.
	// If the max height of this water is below that level, don't bother rendering sides bordering onto farmland.
	float		maxh = h0;
	if ( h1 > maxh ) maxh = h1;
	if ( h2 > maxh ) maxh = h2;
	if ( h3 > maxh ) maxh = h3;
	if ( maxh <= ( 15.0f / 16.0f ) )
	{
		if ( level->getTile( x, y, z - 1 ) == Tile_SPU::farmland_Id )
		{
			dirs[0] = false;
		}
		if ( level->getTile( x, y, z + 1 ) == Tile_SPU::farmland_Id )
		{
			dirs[1] = false;
		}
		if ( level->getTile( x - 1, y, z ) == Tile_SPU::farmland_Id )
		{
			dirs[2] = false;
		}
		if ( level->getTile( x + 1, y, z ) == Tile_SPU::farmland_Id )
		{
			dirs[3] = false;
		}
	}

	if ( noCulling || up )
	{
		changed = true;
		Icon_SPU *tex = getTexture( tt, 1, data );
		float	angle = static_cast<float>(LiquidTile_SPU::getSlopeAngle(level, x, y, z, m));
		if ( angle > -999 )
		{
			tex = getTexture( tt, 2, data );
		}

        h0 -= offs;
        h1 -= offs;
        h2 -= offs;
        h3 -= offs;

		float u00, u01, u10, u11;
		float v00, v01, v10, v11;
		if ( angle < -999 )
		{
			u00 = tex->getU(0);
			v00 = tex->getV(0);
			u01 = u00;
			v01 = tex->getV(SharedConstants::WORLD_RESOLUTION);
			u10 = tex->getU(SharedConstants::WORLD_RESOLUTION);
			v10 = v01;
			u11 = u10;
			v11 = v00;
		}
		else
		{
			float s = sinf(angle) * .25f;
			float c = cosf(angle) * .25f;
			float cc = SharedConstants::WORLD_RESOLUTION * .5f;
			u00 = tex->getU(cc + (-c - s) * SharedConstants::WORLD_RESOLUTION);
			v00 = tex->getV(cc + (-c + s) * SharedConstants::WORLD_RESOLUTION);
			u01 = tex->getU(cc + (-c + s) * SharedConstants::WORLD_RESOLUTION);
			v01 = tex->getV(cc + (+c + s) * SharedConstants::WORLD_RESOLUTION);
			u10 = tex->getU(cc + (+c + s) * SharedConstants::WORLD_RESOLUTION);
			v10 = tex->getV(cc + (+c - s) * SharedConstants::WORLD_RESOLUTION);
			u11 = tex->getU(cc + (+c - s) * SharedConstants::WORLD_RESOLUTION);
			v11 = tex->getV(cc + (-c - s) * SharedConstants::WORLD_RESOLUTION);
		}

		float	br;
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tt->getLightColor( level, x, y, z ) );
			br = 1;
		}
		else
		{
			br = tt->getBrightness( level, x, y, z );
		}
		t->color( c11 * br * r, c11 * br * g, c11 * br * b );
		t->vertexUV( ( float )( x + 0.0f ), ( float )( y + h0 ), ( float )( z + 0.0f ), u00, v00 );
		t->vertexUV( ( float )( x + 0.0f ), ( float )( y + h1 ), ( float )( z + 1.0f ), u01, v01 );
		t->vertexUV( ( float )( x + 1.0f ), ( float )( y + h2 ), ( float )( z + 1.0f ), u10, v10 );
		t->vertexUV( ( float )( x + 1.0f ), ( float )( y + h3 ), ( float )( z + 0.0f ), u11, v11 );
	}

	if ( noCulling || down )
	{
		float br;
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tt->getLightColor( level, x, y - 1, z ) );
			br = 1;
		}
		else
		{
			br = tt->getBrightness( level, x, y - 1, z );
		}
		t->color( c10 * br, c10 * br, c10 * br );
		renderFaceDown( tt, x, y + offs, z, getTexture( tt, 0 ) );
		changed = true;
	}

	for ( int face = 0; face < 4; face++ )
	{
		int xt = x;
		int yt = y;
		int zt = z;

		if ( face == 0 ) zt--;
		if ( face == 1 ) zt++;
		if ( face == 2 ) xt--;
		if ( face == 3 ) xt++;

		Icon_SPU *tex = getTexture(tt, face + 2, data);

		if ( noCulling || dirs[face] )
		{
			float	hh0;
			float	hh1;
			float	x0, z0, x1, z1;
			if ( face == 0 )
			{
				hh0 = ( float )( h0 );
				hh1 = ( float )( h3 );
				x0 = static_cast<float>(x);
				x1 = static_cast<float>(x + 1);
				z0 = ( float )( z + offs);
				z1 = ( float )( z + offs);
			}
			else if ( face == 1 )
			{
				hh0 = ( float )( h2 );
				hh1 = ( float )( h1 );
				x0 = static_cast<float>(x + 1);
				x1 = static_cast<float>(x);
				z0 = ( float )( z + 1 - offs);
				z1 = ( float )( z + 1 - offs);
			}
			else if ( face == 2 )
			{
				hh0 = ( float )( h1 );
				hh1 = ( float )( h0 );
				x0 = ( float )( x + offs);
				x1 = ( float )( x + offs);
				z0 = static_cast<float>(z + 1);
				z1 = static_cast<float>(z);
			}
			else
			{
				hh0 = ( float )( h3 );
				hh1 = ( float )( h2 );
				x0 = ( float )( x + 1 - offs);
				x1 = ( float )( x + 1 - offs);
				z0 = static_cast<float>(z);
				z1 = static_cast<float>(z + 1);
			}


			changed = true;
			float u0 = tex->getU(0);
			float u1 = tex->getU(SharedConstants::WORLD_RESOLUTION * .5f);

// 			int yTex = tex->getY();
			float v01 = tex->getV((1 - hh0) * SharedConstants::WORLD_RESOLUTION * .5f);
			float v02 = tex->getV((1 - hh1) * SharedConstants::WORLD_RESOLUTION * .5f);
			float v1 = tex->getV(SharedConstants::WORLD_RESOLUTION * .5f);

			float	br;
			if ( SharedConstants::TEXTURE_LIGHTING )
			{
				t->tex2( tt->getLightColor( level, xt, yt, zt ) );
				br = 1;
			}
			else
			{
				br = tt->getBrightness( level, xt, yt, zt );
			}
			if ( face < 2 ) br *= c2;
			else
				br *= c3;

			t->color( c11 * br * r, c11 * br * g, c11 * br * b );
			t->vertexUV( ( float )( x0 ), ( float )( y + hh0 ), ( float )( z0 ), ( float )( u0 ), ( float )( v01 ) );
			t->vertexUV( ( float )( x1 ), ( float )( y + hh1 ), ( float )( z1 ), ( float )( u1 ), ( float )( v02 ) );
			t->vertexUV( ( float )( x1 ), static_cast<float>(y + 0), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
			t->vertexUV( ( float )( x0 ), static_cast<float>(y + 0), ( float )( z0 ), ( float )( u0 ), ( float )( v1 ) );

		}

	}

	tileShapeY0 = yo0;
	tileShapeY1 = yo1;

	return changed;
}

float TileRenderer_SPU::getWaterHeight( int x, int y, int z, Material_SPU* m )
{
	int		count = 0;
	float	h = 0;
	for ( int i = 0; i < 4; i++ )
	{
		int			xx = x - ( i & 1 );
		int			yy = y;
		int			zz = z - ( ( i >> 1 ) & 1 );
		if ( level->getMaterial( xx, yy + 1, zz ) == m )
		{
			return 1;
		}
		Material_SPU*	tm = level->getMaterial( xx, yy, zz );
		if ( tm == m )
		{
			int d = level->getData( xx, yy, zz );
			if ( d >= 8 || d == 0 )
			{
				h += ( LiquidTile_SPU::getHeight( d ) )* 10;
				count += 10;
			}
			h += LiquidTile_SPU::getHeight( d );
			count++;
		}
		else if ( !tm->isSolid() )
		{
			h += 1;
			count++;
		}
	}
	return 1 - h / count;
}

void TileRenderer_SPU::renderBlock( Tile_SPU* tt, ChunkRebuildData* level, int x, int y, int z )
{
	renderBlock(tt, level, x, y, z, 0);
}

void TileRenderer_SPU::renderBlock(Tile_SPU *tt, ChunkRebuildData *level, int x, int y, int z, int data)
{
	float		c10 = 0.5f;
	float		c11 = 1;
	float		c2 = 0.8f;
	float		c3 = 0.6f;

	Tesselator_SPU* t = getTesselator();
	t->begin();
	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tt->getLightColor( level, x, y, z ) );
	}
	float		center = SharedConstants::TEXTURE_LIGHTING ? 1 : tt->getBrightness( level, x, y, z );
	float		br = SharedConstants::TEXTURE_LIGHTING ? 1 : tt->getBrightness( level, x, y - 1, z );

	if ( br < center ) br = center;
	t->color( c10 * br, c10 * br, c10 * br );
	renderFaceDown( tt, -0.5f, -0.5f, -0.5f, getTexture( tt, 0, data ) );

	br = SharedConstants::TEXTURE_LIGHTING ? 1 : tt->getBrightness( level, x, y + 1, z );
	if ( br < center ) br = center;
	t->color( c11 * br, c11 * br, c11 * br );
	renderFaceUp( tt, -0.5f, -0.5f, -0.5f, getTexture( tt, 1, data ) );

	br = SharedConstants::TEXTURE_LIGHTING ? 1 : tt->getBrightness( level, x, y, z - 1 );
	if ( br < center ) br = center;
	t->color( c2 * br, c2 * br, c2 * br );
	renderNorth( tt, -0.5f, -0.5f, -0.5f, getTexture( tt, 2, data ) );

	br = SharedConstants::TEXTURE_LIGHTING ? 1 : tt->getBrightness( level, x, y, z + 1 );
	if ( br < center ) br = center;
	t->color( c2 * br, c2 * br, c2 * br );
	renderSouth( tt, -0.5f, -0.5f, -0.5f, getTexture( tt, 3, data ) );

	br = SharedConstants::TEXTURE_LIGHTING ? 1 : tt->getBrightness( level, x - 1, y, z );
	if ( br < center ) br = center;
	t->color( c3 * br, c3 * br, c3 * br );
	renderWest( tt, -0.5f, -0.5f, -0.5f, getTexture( tt, 4, data ) );

	br = SharedConstants::TEXTURE_LIGHTING ? 1 : tt->getBrightness( level, x + 1, y, z );
	if ( br < center ) br = center;
	t->color( c3 * br, c3 * br, c3 * br );
	renderEast( tt, -0.5f, -0.5f, -0.5f, getTexture( tt, 5, data ) );
	t->end();

}

bool TileRenderer_SPU::tesselateBlockInWorld( Tile_SPU* tt, int x, int y, int z )
{
	int		col = tt->getColor( level, x, y, z );
	float	r = ( ( col >> 16 ) & 0xff ) / 255.0f;
	float	g = ( ( col >> 8 ) & 0xff ) / 255.0f;
	float	b = ( ( col )& 0xff ) / 255.0f;

	if ( isAnaglyph3d())
	{
		float	cr = ( r * 30 + g * 59 + b * 11 ) / 100;
		float	cg = ( r * 30 + g * 70 ) / ( 100 );
		float	cb = ( r * 30 + b * 70 ) / ( 100 );

		r = cr;
		g = cg;
		b = cb;
	}

	if ( level->m_tileData.lightEmission[tt->id] == 0 )//4J - TODO/remove (Minecraft::useAmbientOcclusion())
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			return tesselateBlockInWorldWithAmbienceOcclusionTexLighting( tt, x, y, z, r, g, b );
		}
		else
		{
			return tesselateBlockInWorldWithAmbienceOcclusionOldLighting( tt, x, y, z, r, g, b );
		}
	}
	else
	{
		return tesselateBlockInWorld( tt, x, y, z, r, g, b );
	}
}

bool TileRenderer_SPU::tesselateTreeInWorld(Tile_SPU *tt, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	int facing = data & TreeTile_SPU::MASK_FACING;

	if (facing == TreeTile_SPU::FACING_X)
	{
		northFlip = FLIP_CW;
		southFlip = FLIP_CW;
		upFlip = FLIP_CW;
		downFlip = FLIP_CW;
	}
	else if (facing == TreeTile_SPU::FACING_Z)
	{
		eastFlip = FLIP_CW;
		westFlip = FLIP_CW;
	}

	bool result = tesselateBlockInWorld(tt, x, y, z);

	eastFlip = 0;
	northFlip = 0;
	southFlip = 0;
	westFlip = 0;
	upFlip = 0;
	downFlip = 0;

	return result;
}

bool TileRenderer_SPU::tesselateQuartzInWorld(Tile_SPU *tt, int x, int y, int z)
{
	int data = level->getData(x, y, z);

	if (data == QuartzBlockTile_SPU::TYPE_LINES_X)
	{
		northFlip = FLIP_CW;
		southFlip = FLIP_CW;
		upFlip = FLIP_CW;
		downFlip = FLIP_CW;
	}
	else if (data == QuartzBlockTile_SPU::TYPE_LINES_Z)
	{
		eastFlip = FLIP_CW;
		westFlip = FLIP_CW;
	}

	bool result = tesselateBlockInWorld(tt, x, y, z);

	eastFlip = 0;
	northFlip = 0;
	southFlip = 0;
	westFlip = 0;
	upFlip = 0;
	downFlip = 0;

	return result;
}

bool TileRenderer_SPU::tesselateCocoaInWorld(CocoaTile_SPU *tt, int x, int y, int z)
{
#ifdef DISABLE_TESS_FUNCS
	Tesselator *t = Tesselator::getInstance();

	if (SharedConstants::TEXTURE_LIGHTING)
	{
		t->tex2(tt->getLightColor(level, x, y, z));
		t->color(1.0f, 1.0f, 1.0f);
	}
	else
	{
		float br = tt->getBrightness(level, x, y, z);
		if (Tile::lightEmission[tt->id] > 0) br = 1.0f;
		t->color(br, br, br);
	}

	int data = level->getData(x, y, z);
	int dir = DirectionalTile::getDirection(data);
	int age = CocoaTile::getAge(data);
	Icon *tex = tt->getTextureForAge(age);

	int cocoaWidth = 4 + age * 2;
	int cocoaHeight = 5 + age * 2;

	double us = 15.0 - cocoaWidth;
	double ue = 15.0;
	double vs = 4.0;
	double ve = 4.0 + cocoaHeight;
	double u0 = tex->getU(us, true);
	double u1 = tex->getU(ue, true);
	double v0 = tex->getV(vs, true);
	double v1 = tex->getV(ve, true);


	double offX = 0;
	double offZ = 0;

	switch (dir)
	{
	case Direction::NORTH:
		offX = 8.0 - cocoaWidth / 2;
		offZ = 1.0;
		break;
	case Direction::SOUTH:
		offX = 8.0 - cocoaWidth / 2;
		offZ = 15.0 - cocoaWidth;
		break;
	case Direction::EAST:
		offX = 15.0 - cocoaWidth;
		offZ = 8.0 - cocoaWidth / 2;
		break;
	case Direction::WEST:
		offX = 1.0;
		offZ = 8.0 - cocoaWidth / 2;
		break;
	}

	double x0 = x + offX / 16.0;
	double x1 = x + (offX + cocoaWidth) / 16.0;
	double y0 = y + (12.0 - cocoaHeight) / 16.0;
	double y1 = y + 12.0 / 16.0;
	double z0 = z + offZ / 16.0;
	double z1 = z + (offZ + cocoaWidth) / 16.0;

	// west
	{
		t->vertexUV(x0, y0, z0, u0, v1);
		t->vertexUV(x0, y0, z1, u1, v1);
		t->vertexUV(x0, y1, z1, u1, v0);
		t->vertexUV(x0, y1, z0, u0, v0);
	}
	// east
	{
		t->vertexUV(x1, y0, z1, u0, v1);
		t->vertexUV(x1, y0, z0, u1, v1);
		t->vertexUV(x1, y1, z0, u1, v0);
		t->vertexUV(x1, y1, z1, u0, v0);
	}
	// north
	{
		t->vertexUV(x1, y0, z0, u0, v1);
		t->vertexUV(x0, y0, z0, u1, v1);
		t->vertexUV(x0, y1, z0, u1, v0);
		t->vertexUV(x1, y1, z0, u0, v0);
	}
	// south
	{
		t->vertexUV(x0, y0, z1, u0, v1);
		t->vertexUV(x1, y0, z1, u1, v1);
		t->vertexUV(x1, y1, z1, u1, v0);
		t->vertexUV(x0, y1, z1, u0, v0);
	}

	int topWidth = cocoaWidth;
	if (age >= 2)
	{
		// special case because the top piece didn't fit
		topWidth--;
	}

	u0 = tex->getU0(true);
	u1 = tex->getU(topWidth, true);
	v0 = tex->getV0(true);
	v1 = tex->getV(topWidth, true);

	// top
	{
		t->vertexUV(x0, y1, z1, u0, v1);
		t->vertexUV(x1, y1, z1, u1, v1);
		t->vertexUV(x1, y1, z0, u1, v0);
		t->vertexUV(x0, y1, z0, u0, v0);
	}
	// bottom
	{
		t->vertexUV(x0, y0, z0, u0, v0);
		t->vertexUV(x1, y0, z0, u1, v0);
		t->vertexUV(x1, y0, z1, u1, v1);
		t->vertexUV(x0, y0, z1, u0, v1);
	}

	// stalk
	u0 = tex->getU(12, true);
	u1 = tex->getU1(true);
	v0 = tex->getV0(true);
	v1 = tex->getV(4, true);

	offX = 8;
	offZ = 0;

	switch (dir)
	{
	case Direction::NORTH:
		offX = 8.0;
		offZ = 0.0;
		break;
	case Direction::SOUTH:
		offX = 8;
		offZ = 12;
		{
			double temp = u0;
			u0 = u1;
			u1 = temp;
		}
		break;
	case Direction::EAST:
		offX = 12.0;
		offZ = 8.0;
		{
			double temp = u0;
			u0 = u1;
			u1 = temp;
		}
		break;
	case Direction::WEST:
		offX = 0.0;
		offZ = 8.0;
		break;
	}

	x0 = x + offX / 16.0;
	x1 = x + (offX + 4.0) / 16.0;
	y0 = y + 12.0 / 16.0;
	y1 = y + 16.0 / 16.0;
	z0 = z + offZ / 16.0;
	z1 = z + (offZ + 4.0) / 16.0;
	if (dir == Direction::NORTH || dir == Direction::SOUTH)
	{
		// west
		{
			t->vertexUV(x0, y0, z0, u1, v1);
			t->vertexUV(x0, y0, z1, u0, v1);
			t->vertexUV(x0, y1, z1, u0, v0);
			t->vertexUV(x0, y1, z0, u1, v0);
		}
		// east
		{
			t->vertexUV(x0, y0, z1, u0, v1);
			t->vertexUV(x0, y0, z0, u1, v1);
			t->vertexUV(x0, y1, z0, u1, v0);
			t->vertexUV(x0, y1, z1, u0, v0);
		}
	}
	else if (dir == Direction::WEST || dir == Direction::EAST)
	{
		// north
		{
			t->vertexUV(x1, y0, z0, u0, v1);
			t->vertexUV(x0, y0, z0, u1, v1);
			t->vertexUV(x0, y1, z0, u1, v0);
			t->vertexUV(x1, y1, z0, u0, v0);
		}
		// south
		{
			t->vertexUV(x0, y0, z0, u1, v1);
			t->vertexUV(x1, y0, z0, u0, v1);
			t->vertexUV(x1, y1, z0, u0, v0);
			t->vertexUV(x0, y1, z0, u1, v0);
		}
	}

#endif // DISABLE_TESS_FUNCS
	return true;
}

// 4J - brought changes forward from 1.8.2
bool TileRenderer_SPU::tesselateBlockInWorldWithAmbienceOcclusionTexLighting( Tile_SPU* tt, int pX, int pY, int pZ,
																		  float pBaseRed, float pBaseGreen,
																		  float pBaseBlue )
{
	// 4J - added these faceFlags so we can detect whether this block is going to have no visible faces and early out
	// the original code checked noCulling and shouldRenderFace directly where faceFlags is used now
	int			faceFlags = 0;
	if ( noCulling )
	{
		faceFlags = 0x3f;
	}
	else
	{
/*#ifdef _DEBUG
		if(dynamic_cast<StairTile *>(tt)!=nullptr)
		{
			// stair tile
			faceFlags |= tt->shouldRenderFace( level, pX, pY - 1, pZ, 0 ) ? 0x01 : 0;
			faceFlags |= tt->shouldRenderFace( level, pX, pY + 1, pZ, 1 ) ? 0x02 : 0;
			faceFlags |= tt->shouldRenderFace( level, pX, pY, pZ - 1, 2 ) ? 0x04 : 0;
			faceFlags |= tt->shouldRenderFace( level, pX, pY, pZ + 1, 3 ) ? 0x08 : 0;
			faceFlags |= tt->shouldRenderFace( level, pX - 1, pY, pZ, 4 ) ? 0x10 : 0;
			faceFlags |= tt->shouldRenderFace( level, pX + 1, pY, pZ, 5 ) ? 0x20 : 0;

			printf("Stair tile\n");
		}
		else
		{
			faceFlags |= tt->shouldRenderFace( level, pX, pY - 1, pZ, 0 ) ? 0x01 : 0;
			faceFlags |= tt->shouldRenderFace( level, pX, pY + 1, pZ, 1 ) ? 0x02 : 0;
			faceFlags |= tt->shouldRenderFace( level, pX, pY, pZ - 1, 2 ) ? 0x04 : 0;
			faceFlags |= tt->shouldRenderFace( level, pX, pY, pZ + 1, 3 ) ? 0x08 : 0;
			faceFlags |= tt->shouldRenderFace( level, pX - 1, pY, pZ, 4 ) ? 0x10 : 0;
			faceFlags |= tt->shouldRenderFace( level, pX + 1, pY, pZ, 5 ) ? 0x20 : 0;

		}
#else*/
		faceFlags |= tt->shouldRenderFace( level, pX, pY - 1, pZ, 0 ) ? 0x01 : 0;
		faceFlags |= tt->shouldRenderFace( level, pX, pY + 1, pZ, 1 ) ? 0x02 : 0;
		faceFlags |= tt->shouldRenderFace( level, pX, pY, pZ - 1, 2 ) ? 0x04 : 0;
		faceFlags |= tt->shouldRenderFace( level, pX, pY, pZ + 1, 3 ) ? 0x08 : 0;
		faceFlags |= tt->shouldRenderFace( level, pX - 1, pY, pZ, 4 ) ? 0x10 : 0;
		faceFlags |= tt->shouldRenderFace( level, pX + 1, pY, pZ, 5 ) ? 0x20 : 0;
//#endif
	}
	if ( faceFlags == 0 )
	{
		return false;
	}


	// If we are only rendering the bottom face and we're at the bottom of the world, we shouldn't be able to see this - don't render anything
	if( ( faceFlags == 1 ) && ( pY == 0 ) )
	{
		return false;
	}

	applyAmbienceOcclusion = true;
	float		ll1 = ll000;
	float		ll2 = ll000;
	float		ll3 = ll000;
	float		ll4 = ll000;
	bool		tint0 = true;
	bool		tint1 = true;
	bool		tint2 = true;
	bool		tint3 = true;
	bool		tint4 = true;
	bool		tint5 = true;


	ll000 = tt->getShadeBrightness( level, pX, pY, pZ );
// 	Tile* t2 = Tile::tiles[tt->id];
// 	if(t2->getShadeBrightness(level->m_pRegion, pX, pY, pZ) != ll000)
// 	{
// 		app.DebugPrintf("Failed\n");
// 		ll000 = tt->getShadeBrightness( level, pX, pY, pZ );
// 		ll000 = t2->getShadeBrightness(level->m_pRegion, pX, pY, pZ);
// 	}
	llx00 = tt->getShadeBrightness( level, pX - 1, pY, pZ );
	ll0y0 = tt->getShadeBrightness( level, pX, pY - 1, pZ );
	ll00z = tt->getShadeBrightness( level, pX, pY, pZ - 1 );
	llX00 = tt->getShadeBrightness( level, pX + 1, pY, pZ );
	ll0Y0 = tt->getShadeBrightness( level, pX, pY + 1, pZ );
	ll00Z = tt->getShadeBrightness( level, pX, pY, pZ + 1 );



	// 4J - these changes brought forward from 1.2.3
	int			centerColor = tt->getLightColor( level, pX, pY, pZ );
	int			ccx00 = centerColor;
	int			cc0y0 = centerColor;
	int			cc00z = centerColor;
	int			ccX00 = centerColor;
	int			cc0Y0 = centerColor;
	int			cc00Z = centerColor;


	if (tileShapeY0 <= 0 || !level->isSolidRenderTile(pX, pY - 1, pZ)) cc0y0 = tt->getLightColor(level, pX, pY - 1, pZ);
	if (tileShapeY1 >= 1 || !level->isSolidRenderTile(pX, pY + 1, pZ)) cc0Y0 = tt->getLightColor(level, pX, pY + 1, pZ);
	if (tileShapeX0 <= 0 || !level->isSolidRenderTile(pX - 1, pY, pZ)) ccx00 = tt->getLightColor(level, pX - 1, pY, pZ);
	if (tileShapeX1 >= 1 || !level->isSolidRenderTile(pX + 1, pY, pZ)) ccX00 = tt->getLightColor(level, pX + 1, pY, pZ);
	if (tileShapeZ0 <= 0 || !level->isSolidRenderTile(pX, pY, pZ - 1)) cc00z = tt->getLightColor(level, pX, pY, pZ - 1);
	if (tileShapeZ1 >= 1 || !level->isSolidRenderTile(pX, pY, pZ + 1)) cc00Z = tt->getLightColor(level, pX, pY, pZ + 1);


	Tesselator_SPU* t = getTesselator();
	t->tex2( 0xf000f );

	llTransXY0 = level->m_tileData.transculent[level->getTile( pX + 1, pY + 1, pZ )];
	llTransXy0 = level->m_tileData.transculent[level->getTile( pX + 1, pY - 1, pZ )];
	llTransX0Z = level->m_tileData.transculent[level->getTile( pX + 1, pY, pZ + 1 )];
	llTransX0z = level->m_tileData.transculent[level->getTile( pX + 1, pY, pZ - 1 )];
	llTransxY0 = level->m_tileData.transculent[level->getTile( pX - 1, pY + 1, pZ )];
	llTransxy0 = level->m_tileData.transculent[level->getTile( pX - 1, pY - 1, pZ )];
	llTransx0z = level->m_tileData.transculent[level->getTile( pX - 1, pY, pZ - 1 )];
	llTransx0Z = level->m_tileData.transculent[level->getTile( pX - 1, pY, pZ + 1 )];
	llTrans0YZ = level->m_tileData.transculent[level->getTile( pX, pY + 1, pZ + 1 )];
	llTrans0Yz = level->m_tileData.transculent[level->getTile( pX, pY + 1, pZ - 1 )];
	llTrans0yZ = level->m_tileData.transculent[level->getTile( pX, pY - 1, pZ + 1 )];
	llTrans0yz = level->m_tileData.transculent[level->getTile( pX, pY - 1, pZ - 1 )];

	if ( getTexture(tt)== &Tile_SPU::ms_pTileData->grass_iconTop ) 
		tint0 = tint2 = tint3 = tint4 = tint5 = false;
	if ( hasFixedTexture() ) tint0 = tint2 = tint3 = tint4 = tint5 = false;

	if ( faceFlags & 0x01 )
	{
		if ( blsmooth > 0 )
		{
			if ( tileShapeY0 <= 0 ) pY--;		// 4J - condition brought forwardEnterCriticalSection from 1.2.3

			ccxy0 = tt->getLightColor( level, pX - 1, pY, pZ );
			cc0yz = tt->getLightColor( level, pX, pY, pZ - 1 );
			cc0yZ = tt->getLightColor( level, pX, pY, pZ + 1 );
			ccXy0 = tt->getLightColor( level, pX + 1, pY, pZ );

			llxy0 = tt->getShadeBrightness( level, pX - 1, pY, pZ );
			ll0yz = tt->getShadeBrightness( level, pX, pY, pZ - 1 );
			ll0yZ = tt->getShadeBrightness( level, pX, pY, pZ + 1 );
			llXy0 = tt->getShadeBrightness( level, pX + 1, pY, pZ );

			if ( llTrans0yz || llTransxy0 )
			{
				llxyz = tt->getShadeBrightness( level, pX - 1, pY, pZ - 1 );
				ccxyz = tt->getLightColor( level, pX - 1, pY, pZ - 1 );
			}
			else
			{
				llxyz = llxy0;
				ccxyz = ccxy0;
			}
			if ( llTrans0yZ || llTransxy0 )
			{
				llxyZ = tt->getShadeBrightness( level, pX - 1, pY, pZ + 1 );
				ccxyZ = tt->getLightColor( level, pX - 1, pY, pZ + 1 );
			}
			else
			{
				llxyZ = llxy0;
				ccxyZ = ccxy0;
			}
			if ( llTrans0yz || llTransXy0 )
			{
				llXyz = tt->getShadeBrightness( level, pX + 1, pY, pZ - 1 );
				ccXyz = tt->getLightColor( level, pX + 1, pY, pZ - 1 );
			}
			else
			{
				llXyz = llXy0;
				ccXyz = ccXy0;
			}
			if ( llTrans0yZ || llTransXy0 )
			{
				llXyZ = tt->getShadeBrightness( level, pX + 1, pY, pZ + 1 );
				ccXyZ = tt->getLightColor( level, pX + 1, pY, pZ + 1 );
			}
			else
			{
				llXyZ = llXy0;
				ccXyZ = ccXy0;
			}

			if ( tileShapeY0 <= 0 ) pY++;		// 4J - condition brought forward from 1.2.3
			ll1 = ( llxyZ + llxy0 + ll0yZ + ll0y0 ) / 4.0f;
			ll4 = ( ll0yZ + ll0y0 + llXyZ + llXy0 ) / 4.0f;
			ll3 = ( ll0y0 + ll0yz + llXy0 + llXyz ) / 4.0f;
			ll2 = ( llxy0 + llxyz + ll0y0 + ll0yz ) / 4.0f;

			tc1 = blend( ccxyZ, ccxy0, cc0yZ, cc0y0 );
			tc4 = blend( cc0yZ, ccXyZ, ccXy0, cc0y0 );
			tc3 = blend( cc0yz, ccXy0, ccXyz, cc0y0 );
			tc2 = blend( ccxy0, ccxyz, cc0yz, cc0y0 );
		}
		else
		{
			ll1 = ll2 = ll3 = ll4 = ll0y0;
			tc1 = tc2 = tc3 = tc4 = ccxy0;
		}
		c1r = c2r = c3r = c4r = ( tint0 ? pBaseRed : 1.0f ) * 0.5f;
		c1g = c2g = c3g = c4g = ( tint0 ? pBaseGreen : 1.0f ) * 0.5f;
		c1b = c2b = c3b = c4b = ( tint0 ? pBaseBlue : 1.0f ) * 0.5f;
		c1r *= ll1;
		c1g *= ll1;
		c1b *= ll1;
		c2r *= ll2;
		c2g *= ll2;
		c2b *= ll2;
		c3r *= ll3;
		c3g *= ll3;
		c3b *= ll3;
		c4r *= ll4;
		c4g *= ll4;
		c4b *= ll4;

		renderFaceDown( tt, static_cast<float>(pX), static_cast<float>(pY), static_cast<float>(pZ), getTexture( tt, level, pX, pY, pZ, 0 ) );
	}
	if ( faceFlags & 0x02 )
	{
		if ( blsmooth > 0 )
		{
			if ( tileShapeY1 >= 1 ) pY++;		// 4J - condition brought forward from 1.2.3

			ccxY0 = tt->getLightColor( level, pX - 1, pY, pZ );
			ccXY0 = tt->getLightColor( level, pX + 1, pY, pZ );
			cc0Yz = tt->getLightColor( level, pX, pY, pZ - 1 );
			cc0YZ = tt->getLightColor( level, pX, pY, pZ + 1 );

			llxY0 = tt->getShadeBrightness( level, pX - 1, pY, pZ );
			llXY0 = tt->getShadeBrightness( level, pX + 1, pY, pZ );
			ll0Yz = tt->getShadeBrightness( level, pX, pY, pZ - 1 );
			ll0YZ = tt->getShadeBrightness( level, pX, pY, pZ + 1 );

			if ( llTrans0Yz || llTransxY0 )
			{
				llxYz = tt->getShadeBrightness( level, pX - 1, pY, pZ - 1 );
				ccxYz = tt->getLightColor( level, pX - 1, pY, pZ - 1 );
			}
			else
			{
				llxYz = llxY0;
				ccxYz = ccxY0;
			}
			if ( llTrans0Yz || llTransXY0 )
			{
				llXYz = tt->getShadeBrightness( level, pX + 1, pY, pZ - 1 );
				ccXYz = tt->getLightColor( level, pX + 1, pY, pZ - 1 );
			}
			else
			{
				llXYz = llXY0;
				ccXYz = ccXY0;
			}
			if ( llTrans0YZ || llTransxY0 )
			{
				llxYZ = tt->getShadeBrightness( level, pX - 1, pY, pZ + 1 );
				ccxYZ = tt->getLightColor( level, pX - 1, pY, pZ + 1 );
			}
			else
			{
				llxYZ = llxY0;
				ccxYZ = ccxY0;
			}
			if ( llTrans0YZ || llTransXY0 )
			{
				llXYZ = tt->getShadeBrightness( level, pX + 1, pY, pZ + 1 );
				ccXYZ = tt->getLightColor( level, pX + 1, pY, pZ + 1 );
			}
			else
			{
				llXYZ = llXY0;
				ccXYZ = ccXY0;
			}
			if ( tileShapeY1 >= 1 ) pY--;		// 4J - condition brought forward from 1.2.3

			ll4 = ( llxYZ + llxY0 + ll0YZ + ll0Y0 ) / 4.0f;
			ll1 = ( ll0YZ + ll0Y0 + llXYZ + llXY0 ) / 4.0f;
			ll2 = ( ll0Y0 + ll0Yz + llXY0 + llXYz ) / 4.0f;
			ll3 = ( llxY0 + llxYz + ll0Y0 + ll0Yz ) / 4.0f;

			tc4 = blend( ccxYZ, ccxY0, cc0YZ, cc0Y0 );
			tc1 = blend( cc0YZ, ccXYZ, ccXY0, cc0Y0 );
			tc2 = blend( cc0Yz, ccXY0, ccXYz, cc0Y0 );
			tc3 = blend( ccxY0, ccxYz, cc0Yz, cc0Y0 );
		}
		else
		{
			ll1 = ll2 = ll3 = ll4 = ll0Y0;
			tc1 = tc2 = tc3 = tc4 = cc0Y0;
		}
		c1r = c2r = c3r = c4r = ( tint1 ? pBaseRed : 1.0f );
		c1g = c2g = c3g = c4g = ( tint1 ? pBaseGreen : 1.0f );
		c1b = c2b = c3b = c4b = ( tint1 ? pBaseBlue : 1.0f );
		c1r *= ll1;
		c1g *= ll1;
		c1b *= ll1;
		c2r *= ll2;
		c2g *= ll2;
		c2b *= ll2;
		c3r *= ll3;
		c3g *= ll3;
		c3b *= ll3;
		c4r *= ll4;
		c4g *= ll4;
		c4b *= ll4;


		renderFaceUp( tt, static_cast<float>(pX), static_cast<float>(pY), static_cast<float>(pZ), getTexture( tt, level, pX, pY, pZ, 1 ) );
	}
	if ( faceFlags & 0x04 )
	{
		if ( blsmooth > 0 )
		{
			if ( tileShapeZ0 <= 0 ) pZ--;		// 4J - condition brought forward from 1.2.3
			llx0z = tt->getShadeBrightness( level, pX - 1, pY, pZ );
			ll0yz = tt->getShadeBrightness( level, pX, pY - 1, pZ );
			ll0Yz = tt->getShadeBrightness( level, pX, pY + 1, pZ );
			llX0z = tt->getShadeBrightness( level, pX + 1, pY, pZ );

			ccx0z = tt->getLightColor( level, pX - 1, pY, pZ );
			cc0yz = tt->getLightColor( level, pX, pY - 1, pZ );
			cc0Yz = tt->getLightColor( level, pX, pY + 1, pZ );
			ccX0z = tt->getLightColor( level, pX + 1, pY, pZ );

			if ( llTransx0z || llTrans0yz )
			{
				llxyz = tt->getShadeBrightness( level, pX - 1, pY - 1, pZ );
				ccxyz = tt->getLightColor( level, pX - 1, pY - 1, pZ );
			}
			else
			{
				llxyz = llx0z;
				ccxyz = ccx0z;
			}
			if ( llTransx0z || llTrans0Yz )
			{
				llxYz = tt->getShadeBrightness( level, pX - 1, pY + 1, pZ );
				ccxYz = tt->getLightColor( level, pX - 1, pY + 1, pZ );
			}
			else
			{
				llxYz = llx0z;
				ccxYz = ccx0z;
			}
			if ( llTransX0z || llTrans0yz )
			{
				llXyz = tt->getShadeBrightness( level, pX + 1, pY - 1, pZ );
				ccXyz = tt->getLightColor( level, pX + 1, pY - 1, pZ );
			}
			else
			{
				llXyz = llX0z;
				ccXyz = ccX0z;
			}
			if ( llTransX0z || llTrans0Yz )
			{
				llXYz = tt->getShadeBrightness( level, pX + 1, pY + 1, pZ );
				ccXYz = tt->getLightColor( level, pX + 1, pY + 1, pZ );
			}
			else
			{
				llXYz = llX0z;
				ccXYz = ccX0z;
			}
			if ( tileShapeZ0 <= 0 ) pZ++;		// 4J - condition brought forward from 1.2.3

#ifdef _XBOX
			#pragma message(__LOC__"ambientOcclusion NEEDS CHANGED FROM A BOOL ")
#endif
			if (smoothShapeLighting && g_ambientOcclusionMax)//minecraft->options->ambientOcclusion >= Options::AO_MAX)
			{
				float _ll1 = (llx0z + llxYz + ll00z + ll0Yz) / 4.0f;
				float _ll2 = (ll00z + ll0Yz + llX0z + llXYz) / 4.0f;
				float _ll3 = (ll0yz + ll00z + llXyz + llX0z) / 4.0f;
				float _ll4 = (llxyz + llx0z + ll0yz + ll00z) / 4.0f;
				ll1 = static_cast<float>(_ll1 * tileShapeY1 * (1.0 - tileShapeX0) + _ll2 * tileShapeY0 * tileShapeX0 + _ll3 * (1.0 - tileShapeY1) * tileShapeX0 + _ll4 * (1.0 - tileShapeY1)
                                         * (1.0 - tileShapeX0));
				ll2 = static_cast<float>(_ll1 * tileShapeY1 * (1.0 - tileShapeX1) + _ll2 * tileShapeY1 * tileShapeX1 + _ll3 * (1.0 - tileShapeY1) * tileShapeX1 + _ll4 * (1.0 - tileShapeY1)
                                         * (1.0 - tileShapeX1));
				ll3 = static_cast<float>(_ll1 * tileShapeY0 * (1.0 - tileShapeX1) + _ll2 * tileShapeY0 * tileShapeX1 + _ll3 * (1.0 - tileShapeY0) * tileShapeX1 + _ll4 * (1.0 - tileShapeY0)
                                         * (1.0 - tileShapeX1));
				ll4 = static_cast<float>(_ll1 * tileShapeY0 * (1.0 - tileShapeX0) + _ll2 * tileShapeY0 * tileShapeX0 + _ll3 * (1.0 - tileShapeY0) * tileShapeX0 + _ll4 * (1.0 - tileShapeY0)
                                         * (1.0 - tileShapeX0));

				int _tc1 = blend(ccx0z, ccxYz, cc0Yz, cc00z);
				int _tc2 = blend(cc0Yz, ccX0z, ccXYz, cc00z);
				int _tc3 = blend(cc0yz, ccXyz, ccX0z, cc00z);
				int _tc4 = blend(ccxyz, ccx0z, cc0yz, cc00z);
				tc1 = blend(_tc1, _tc2, _tc3, _tc4, tileShapeY1 * (1.0 - tileShapeX0), tileShapeY1 * tileShapeX0, (1.0 - tileShapeY1) * tileShapeX0, (1.0 - tileShapeY1) * (1.0 - tileShapeX0));
				tc2 = blend(_tc1, _tc2, _tc3, _tc4, tileShapeY1 * (1.0 - tileShapeX1), tileShapeY1 * tileShapeX1, (1.0 - tileShapeY1) * tileShapeX1, (1.0 - tileShapeY1) * (1.0 - tileShapeX1));
				tc3 = blend(_tc1, _tc2, _tc3, _tc4, tileShapeY0 * (1.0 - tileShapeX1), tileShapeY0 * tileShapeX1, (1.0 - tileShapeY0) * tileShapeX1, (1.0 - tileShapeY0) * (1.0 - tileShapeX1));
				tc4 = blend(_tc1, _tc2, _tc3, _tc4, tileShapeY0 * (1.0 - tileShapeX0), tileShapeY0 * tileShapeX0, (1.0 - tileShapeY0) * tileShapeX0, (1.0 - tileShapeY0) * (1.0 - tileShapeX0));
			} else {
				ll1 = ( llx0z + llxYz + ll00z + ll0Yz ) / 4.0f;
				ll2 = ( ll00z + ll0Yz + llX0z + llXYz ) / 4.0f;
				ll3 = ( ll0yz + ll00z + llXyz + llX0z ) / 4.0f;
				ll4 = ( llxyz + llx0z + ll0yz + ll00z ) / 4.0f;

				tc1 = blend( ccx0z, ccxYz, cc0Yz, cc00z );
				tc2 = blend( cc0Yz, ccX0z, ccXYz, cc00z );
				tc3 = blend( cc0yz, ccXyz, ccX0z, cc00z );
				tc4 = blend( ccxyz, ccx0z, cc0yz, cc00z );
			}
		}
		else
		{
			ll1 = ll2 = ll3 = ll4 = ll00z;
			tc1 = tc2 = tc3 = tc4 = cc00z;
		}
		c1r = c2r = c3r = c4r = ( tint2 ? pBaseRed : 1.0f ) * 0.8f;
		c1g = c2g = c3g = c4g = ( tint2 ? pBaseGreen : 1.0f ) * 0.8f;
		c1b = c2b = c3b = c4b = ( tint2 ? pBaseBlue : 1.0f ) * 0.8f;
		c1r *= ll1;
		c1g *= ll1;
		c1b *= ll1;
		c2r *= ll2;
		c2g *= ll2;
		c2b *= ll2;
		c3r *= ll3;
		c3g *= ll3;
		c3b *= ll3;
		c4r *= ll4;
		c4g *= ll4;
		c4b *= ll4;


		Icon_SPU *tex = getTexture(tt, level, pX, pY, pZ, 2);
		renderNorth( tt, static_cast<float>(pX), static_cast<float>(pY), static_cast<float>(pZ), tex );

		if ( fancy && (tex == &Tile_SPU::ms_pTileData->iconData[Tile_SPU::grass_Id] && !hasFixedTexture() ))
		{
			c1r *= pBaseRed;
			c2r *= pBaseRed;
			c3r *= pBaseRed;
			c4r *= pBaseRed;
			c1g *= pBaseGreen;
			c2g *= pBaseGreen;
			c3g *= pBaseGreen;
			c4g *= pBaseGreen;
			c1b *= pBaseBlue;
			c2b *= pBaseBlue;
			c3b *= pBaseBlue;
			c4b *= pBaseBlue;
			bool prev = t->setMipmapEnable( false );	// 4J added - this is rendering the little bit of grass at the top of the side of dirt, don't mipmap it
			renderNorth( tt, static_cast<float>(pX), static_cast<float>(pY), static_cast<float>(pZ), GrassTile_SPU::getSideTextureOverlay() );
			t->setMipmapEnable( prev );
		}
	}
	if ( faceFlags & 0x08 )
	{
		if ( blsmooth > 0 )
		{
			if ( tileShapeZ1 >= 1 ) pZ++;		// 4J - condition brought forward from 1.2.3

			llx0Z = tt->getShadeBrightness( level, pX - 1, pY, pZ );
			llX0Z = tt->getShadeBrightness( level, pX + 1, pY, pZ );
			ll0yZ = tt->getShadeBrightness( level, pX, pY - 1, pZ );
			ll0YZ = tt->getShadeBrightness( level, pX, pY + 1, pZ );

			ccx0Z = tt->getLightColor( level, pX - 1, pY, pZ );
			ccX0Z = tt->getLightColor( level, pX + 1, pY, pZ );
			cc0yZ = tt->getLightColor( level, pX, pY - 1, pZ );
			cc0YZ = tt->getLightColor( level, pX, pY + 1, pZ );

			if ( llTransx0Z || llTrans0yZ )
			{
				llxyZ = tt->getShadeBrightness( level, pX - 1, pY - 1, pZ );
				ccxyZ = tt->getLightColor( level, pX - 1, pY - 1, pZ );
			}
			else
			{
				llxyZ = llx0Z;
				ccxyZ = ccx0Z;
			}
			if ( llTransx0Z || llTrans0YZ )
			{
				llxYZ = tt->getShadeBrightness( level, pX - 1, pY + 1, pZ );
				ccxYZ = tt->getLightColor( level, pX - 1, pY + 1, pZ );
			}
			else
			{
				llxYZ = llx0Z;
				ccxYZ = ccx0Z;
			}
			if ( llTransX0Z || llTrans0yZ )
			{
				llXyZ = tt->getShadeBrightness( level, pX + 1, pY - 1, pZ );
				ccXyZ = tt->getLightColor( level, pX + 1, pY - 1, pZ );
			}
			else
			{
				llXyZ = llX0Z;
				ccXyZ = ccX0Z;
			}
			if ( llTransX0Z || llTrans0YZ )
			{
				llXYZ = tt->getShadeBrightness( level, pX + 1, pY + 1, pZ );
				ccXYZ = tt->getLightColor( level, pX + 1, pY + 1, pZ );
			}
			else
			{
				llXYZ = llX0Z;
				ccXYZ = ccX0Z;
			}
			if ( tileShapeZ1 >= 1 ) pZ--;		// 4J - condition brought forward from 1.2.3
			if (smoothShapeLighting && g_ambientOcclusionMax)//minecraft->options->ambientOcclusion >= Options::AO_MAX)
			{
				float _ll1 = (llx0Z + llxYZ + ll00Z + ll0YZ) / 4.0f;
				float _ll4 = (ll00Z + ll0YZ + llX0Z + llXYZ) / 4.0f;
				float _ll3 = (ll0yZ + ll00Z + llXyZ + llX0Z) / 4.0f;
				float _ll2 = (llxyZ + llx0Z + ll0yZ + ll00Z) / 4.0f;
				ll1 = static_cast<float>(_ll1 * tileShapeY1 * (1.0 - tileShapeX0) + _ll4 * tileShapeY1 * tileShapeX0 + _ll3 * (1.0 - tileShapeY1) * tileShapeX0 + _ll2 * (1.0 - tileShapeY1)
                                         * (1.0 - tileShapeX0));
				ll2 = static_cast<float>(_ll1 * tileShapeY0 * (1.0 - tileShapeX0) + _ll4 * tileShapeY0 * tileShapeX0 + _ll3 * (1.0 - tileShapeY0) * tileShapeX0 + _ll2 * (1.0 - tileShapeY0)
                                         * (1.0 - tileShapeX0));
				ll3 = static_cast<float>(_ll1 * tileShapeY0 * (1.0 - tileShapeX1) + _ll4 * tileShapeY0 * tileShapeX1 + _ll3 * (1.0 - tileShapeY0) * tileShapeX1 + _ll2 * (1.0 - tileShapeY0)
                                         * (1.0 - tileShapeX1));
				ll4 = static_cast<float>(_ll1 * tileShapeY1 * (1.0 - tileShapeX1) + _ll4 * tileShapeY1 * tileShapeX1 + _ll3 * (1.0 - tileShapeY1) * tileShapeX1 + _ll2 * (1.0 - tileShapeY1)
                                         * (1.0 - tileShapeX1));

				int _tc1 = blend(ccx0Z, ccxYZ, cc0YZ, cc00Z);
				int _tc4 = blend(cc0YZ, ccX0Z, ccXYZ, cc00Z);
				int _tc3 = blend(cc0yZ, ccXyZ, ccX0Z, cc00Z);
				int _tc2 = blend(ccxyZ, ccx0Z, cc0yZ, cc00Z);
				tc1 = blend(_tc1, _tc2, _tc3, _tc4, tileShapeY1 * (1.0 - tileShapeX0), (1.0 - tileShapeY1) * (1.0 - tileShapeX0), (1.0 - tileShapeY1) * tileShapeX0, tileShapeY1 * tileShapeX0);
				tc2 = blend(_tc1, _tc2, _tc3, _tc4, tileShapeY0 * (1.0 - tileShapeX0), (1.0 - tileShapeY0) * (1.0 - tileShapeX0), (1.0 - tileShapeY0) * tileShapeX0, tileShapeY0 * tileShapeX0);
				tc3 = blend(_tc1, _tc2, _tc3, _tc4, tileShapeY0 * (1.0 - tileShapeX1), (1.0 - tileShapeY0) * (1.0 - tileShapeX1), (1.0 - tileShapeY0) * tileShapeX1, tileShapeY0 * tileShapeX1);
				tc4 = blend(_tc1, _tc2, _tc3, _tc4, tileShapeY1 * (1.0 - tileShapeX1), (1.0 - tileShapeY1) * (1.0 - tileShapeX1), (1.0 - tileShapeY1) * tileShapeX1, tileShapeY1 * tileShapeX1);
			}
			else
			{
				ll1 = ( llx0Z + llxYZ + ll00Z + ll0YZ ) / 4.0f;
				ll4 = ( ll00Z + ll0YZ + llX0Z + llXYZ ) / 4.0f;
				ll3 = ( ll0yZ + ll00Z + llXyZ + llX0Z ) / 4.0f;
				ll2 = ( llxyZ + llx0Z + ll0yZ + ll00Z ) / 4.0f;

				tc1 = blend( ccx0Z, ccxYZ, cc0YZ, cc00Z );
				tc4 = blend( cc0YZ, ccX0Z, ccXYZ, cc00Z );
				tc3 = blend( cc0yZ, ccXyZ, ccX0Z, cc00Z );
				tc2 = blend( ccxyZ, ccx0Z, cc0yZ, cc00Z );
			}
		}
		else
		{
			ll1 = ll2 = ll3 = ll4 = ll00Z;
			tc1 = tc2 = tc3 = tc4 = cc00Z;
		}
		c1r = c2r = c3r = c4r = ( tint3 ? pBaseRed : 1.0f ) * 0.8f;
		c1g = c2g = c3g = c4g = ( tint3 ? pBaseGreen : 1.0f ) * 0.8f;
		c1b = c2b = c3b = c4b = ( tint3 ? pBaseBlue : 1.0f ) * 0.8f;
		c1r *= ll1;
		c1g *= ll1;
		c1b *= ll1;
		c2r *= ll2;
		c2g *= ll2;
		c2b *= ll2;
		c3r *= ll3;
		c3g *= ll3;
		c3b *= ll3;
		c4r *= ll4;
		c4g *= ll4;
		c4b *= ll4;
		Icon_SPU *tex = getTexture(tt, level, pX, pY, pZ, 3);
		renderSouth( tt, static_cast<float>(pX), static_cast<float>(pY), static_cast<float>(pZ), getTexture(tt, level, pX, pY, pZ, 3 ) );

		if ( fancy && (tex == &Tile_SPU::ms_pTileData->iconData[Tile_SPU::grass_Id] && !hasFixedTexture() ))
		{
			c1r *= pBaseRed;
			c2r *= pBaseRed;
			c3r *= pBaseRed;
			c4r *= pBaseRed;
			c1g *= pBaseGreen;
			c2g *= pBaseGreen;
			c3g *= pBaseGreen;
			c4g *= pBaseGreen;
			c1b *= pBaseBlue;
			c2b *= pBaseBlue;
			c3b *= pBaseBlue;
			c4b *= pBaseBlue;
			bool prev = t->setMipmapEnable( false );	// 4J added - this is rendering the little bit of grass at the top of the side of dirt, don't mipmap it
			renderSouth( tt, static_cast<float>(pX), static_cast<float>(pY), static_cast<float>(pZ), GrassTile_SPU::getSideTextureOverlay() );
			t->setMipmapEnable( prev );
		}
	}
	if ( faceFlags & 0x10 )
	{
		if ( blsmooth > 0 )
		{
			if ( tileShapeX0 <= 0 ) pX--;		// 4J - condition brought forward from 1.2.3
			llxy0 = tt->getShadeBrightness( level, pX, pY - 1, pZ );
			llx0z = tt->getShadeBrightness( level, pX, pY, pZ - 1 );
			llx0Z = tt->getShadeBrightness( level, pX, pY, pZ + 1 );
			llxY0 = tt->getShadeBrightness( level, pX, pY + 1, pZ );

			ccxy0 = tt->getLightColor( level, pX, pY - 1, pZ );
			ccx0z = tt->getLightColor( level, pX, pY, pZ - 1 );
			ccx0Z = tt->getLightColor( level, pX, pY, pZ + 1 );
			ccxY0 = tt->getLightColor( level, pX, pY + 1, pZ );

			if ( llTransx0z || llTransxy0 )
			{
				llxyz = tt->getShadeBrightness( level, pX, pY - 1, pZ - 1 );
				ccxyz = tt->getLightColor( level, pX, pY - 1, pZ - 1 );
			}
			else
			{
				llxyz = llx0z;
				ccxyz = ccx0z;
			}
			if ( llTransx0Z || llTransxy0 )
			{
				llxyZ = tt->getShadeBrightness( level, pX, pY - 1, pZ + 1 );
				ccxyZ = tt->getLightColor( level, pX, pY - 1, pZ + 1 );
			}
			else
			{
				llxyZ = llx0Z;
				ccxyZ = ccx0Z;
			}
			if ( llTransx0z || llTransxY0 )
			{
				llxYz = tt->getShadeBrightness( level, pX, pY + 1, pZ - 1 );
				ccxYz = tt->getLightColor( level, pX, pY + 1, pZ - 1 );
			}
			else
			{
				llxYz = llx0z;
				ccxYz = ccx0z;
			}
			if ( llTransx0Z || llTransxY0 )
			{
				llxYZ = tt->getShadeBrightness( level, pX, pY + 1, pZ + 1 );
				ccxYZ = tt->getLightColor( level, pX, pY + 1, pZ + 1 );
			}
			else
			{
				llxYZ = llx0Z;
				ccxYZ = ccx0Z;
			}
			if ( tileShapeX0 <= 0 ) pX++;		// 4J - condition brought forward from 1.2.3
			if (smoothShapeLighting && g_ambientOcclusionMax)//minecraft->options->ambientOcclusion >= Options::AO_MAX)
			{
				float _ll4 = (llxy0 + llxyZ + llx00 + llx0Z) / 4.0f;
				float _ll1 = (llx00 + llx0Z + llxY0 + llxYZ) / 4.0f;
				float _ll2 = (llx0z + llx00 + llxYz + llxY0) / 4.0f;
				float _ll3 = (llxyz + llxy0 + llx0z + llx00) / 4.0f;
				ll1 = static_cast<float>(_ll1 * tileShapeY1 * tileShapeZ1 + _ll2 * tileShapeY1 * (1.0 - tileShapeZ1) + _ll3 * (1.0 - tileShapeY1) * (1.0 - tileShapeZ1) + _ll4 * (1.0 - tileShapeY1)
                                         * tileShapeZ1);
				ll2 = static_cast<float>(_ll1 * tileShapeY1 * tileShapeZ0 + _ll2 * tileShapeY1 * (1.0 - tileShapeZ0) + _ll3 * (1.0 - tileShapeY1) * (1.0 - tileShapeZ0) + _ll4 * (1.0 - tileShapeY1)
                                         * tileShapeZ0);
				ll3 = static_cast<float>(_ll1 * tileShapeY0 * tileShapeZ0 + _ll2 * tileShapeY0 * (1.0 - tileShapeZ0) + _ll3 * (1.0 - tileShapeY0) * (1.0 - tileShapeZ0) + _ll4 * (1.0 - tileShapeY0)
                                         * tileShapeZ0);
				ll4 = static_cast<float>(_ll1 * tileShapeY0 * tileShapeZ1 + _ll2 * tileShapeY0 * (1.0 - tileShapeZ1) + _ll3 * (1.0 - tileShapeY0) * (1.0 - tileShapeZ1) + _ll4 * (1.0 - tileShapeY0)
                                         * tileShapeZ1);

				int _tc4 = blend(ccxy0, ccxyZ, ccx0Z, ccx00);
				int _tc1 = blend(ccx0Z, ccxY0, ccxYZ, ccx00);
				int _tc2 = blend(ccx0z, ccxYz, ccxY0, ccx00);
				int _tc3 = blend(ccxyz, ccxy0, ccx0z, ccx00);
				tc1 = blend(_tc1, _tc2, _tc3, _tc4, tileShapeY1 * tileShapeZ1, tileShapeY1 * (1.0 - tileShapeZ1), (1.0 - tileShapeY1) * (1.0 - tileShapeZ1), (1.0 - tileShapeY1) * tileShapeZ1);
				tc2 = blend(_tc1, _tc2, _tc3, _tc4, tileShapeY1 * tileShapeZ0, tileShapeY1 * (1.0 - tileShapeZ0), (1.0 - tileShapeY1) * (1.0 - tileShapeZ0), (1.0 - tileShapeY1) * tileShapeZ0);
				tc3 = blend(_tc1, _tc2, _tc3, _tc4, tileShapeY0 * tileShapeZ0, tileShapeY0 * (1.0 - tileShapeZ0), (1.0 - tileShapeY0) * (1.0 - tileShapeZ0), (1.0 - tileShapeY0) * tileShapeZ0);
				tc4 = blend(_tc1, _tc2, _tc3, _tc4, tileShapeY0 * tileShapeZ1, tileShapeY0 * (1.0 - tileShapeZ1), (1.0 - tileShapeY0) * (1.0 - tileShapeZ1), (1.0 - tileShapeY0) * tileShapeZ1);
			}
			else
			{
				ll4 = ( llxy0 + llxyZ + llx00 + llx0Z ) / 4.0f;
				ll1 = ( llx00 + llx0Z + llxY0 + llxYZ ) / 4.0f;
				ll2 = ( llx0z + llx00 + llxYz + llxY0 ) / 4.0f;
				ll3 = ( llxyz + llxy0 + llx0z + llx00 ) / 4.0f;

				tc4 = blend( ccxy0, ccxyZ, ccx0Z, ccx00 );
				tc1 = blend( ccx0Z, ccxY0, ccxYZ, ccx00 );
				tc2 = blend( ccx0z, ccxYz, ccxY0, ccx00 );
				tc3 = blend( ccxyz, ccxy0, ccx0z, ccx00 );
			}
		}
		else
		{
			ll1 = ll2 = ll3 = ll4 = llx00;
			tc1 = tc2 = tc3 = tc4 = ccx00;
		}
		c1r = c2r = c3r = c4r = ( tint4 ? pBaseRed : 1.0f ) * 0.6f;
		c1g = c2g = c3g = c4g = ( tint4 ? pBaseGreen : 1.0f ) * 0.6f;
		c1b = c2b = c3b = c4b = ( tint4 ? pBaseBlue : 1.0f ) * 0.6f;
		c1r *= ll1;
		c1g *= ll1;
		c1b *= ll1;
		c2r *= ll2;
		c2g *= ll2;
		c2b *= ll2;
		c3r *= ll3;
		c3g *= ll3;
		c3b *= ll3;
		c4r *= ll4;
		c4g *= ll4;
		c4b *= ll4;
		Icon_SPU *tex = getTexture(tt, level, pX, pY, pZ, 4);
		renderWest( tt, static_cast<float>(pX), static_cast<float>(pY), static_cast<float>(pZ), tex );

		if ( fancy && (tex == &Tile_SPU::ms_pTileData->iconData[Tile_SPU::grass_Id] && !hasFixedTexture() ))
		{
			c1r *= pBaseRed;
			c2r *= pBaseRed;
			c3r *= pBaseRed;
			c4r *= pBaseRed;
			c1g *= pBaseGreen;
			c2g *= pBaseGreen;
			c3g *= pBaseGreen;
			c4g *= pBaseGreen;
			c1b *= pBaseBlue;
			c2b *= pBaseBlue;
			c3b *= pBaseBlue;
			c4b *= pBaseBlue;
			bool prev = t->setMipmapEnable( false );	// 4J added - this is rendering the little bit of grass at the top of the side of dirt, don't mipmap it
			renderWest( tt, static_cast<float>(pX), static_cast<float>(pY), static_cast<float>(pZ), GrassTile_SPU::getSideTextureOverlay() );
			t->setMipmapEnable( prev );
		}
	}
	if ( faceFlags & 0x20 )
	{
		if ( blsmooth > 0 )
		{
			if ( tileShapeX1 >= 1 ) pX++;		// 4J - condition brought forward from 1.2.3
			llXy0 = tt->getShadeBrightness( level, pX, pY - 1, pZ );
			llX0z = tt->getShadeBrightness( level, pX, pY, pZ - 1 );
			llX0Z = tt->getShadeBrightness( level, pX, pY, pZ + 1 );
			llXY0 = tt->getShadeBrightness( level, pX, pY + 1, pZ );

			ccXy0 = tt->getLightColor( level, pX, pY - 1, pZ );
			ccX0z = tt->getLightColor( level, pX, pY, pZ - 1 );
			ccX0Z = tt->getLightColor( level, pX, pY, pZ + 1 );
			ccXY0 = tt->getLightColor( level, pX, pY + 1, pZ );

			if ( llTransXy0 || llTransX0z )
			{
				llXyz = tt->getShadeBrightness( level, pX, pY - 1, pZ - 1 );
				ccXyz = tt->getLightColor( level, pX, pY - 1, pZ - 1 );
			}
			else
			{
				llXyz = llX0z;
				ccXyz = ccX0z;
			}
			if ( llTransXy0 || llTransX0Z )
			{
				llXyZ = tt->getShadeBrightness( level, pX, pY - 1, pZ + 1 );
				ccXyZ = tt->getLightColor( level, pX, pY - 1, pZ + 1 );
			}
			else
			{
				llXyZ = llX0Z;
				ccXyZ = ccX0Z;
			}
			if ( llTransXY0 || llTransX0z )
			{
				llXYz = tt->getShadeBrightness( level, pX, pY + 1, pZ - 1 );
				ccXYz = tt->getLightColor( level, pX, pY + 1, pZ - 1 );
			}
			else
			{
				llXYz = llX0z;
				ccXYz = ccX0z;
			}
			if ( llTransXY0 || llTransX0Z )
			{
				llXYZ = tt->getShadeBrightness( level, pX, pY + 1, pZ + 1 );
				ccXYZ = tt->getLightColor( level, pX, pY + 1, pZ + 1 );
			}
			else
			{
				llXYZ = llX0Z;
				ccXYZ = ccX0Z;
			}
			if ( tileShapeX1 >= 1 ) pX--;		// 4J - condition brought forward from 1.2.3
			if (smoothShapeLighting && g_ambientOcclusionMax)//minecraft->options->ambientOcclusion >= Options::AO_MAX)
			{
				float _ll1 = (llXy0 + llXyZ + llX00 + llX0Z) / 4.0f;
				float _ll2 = (llXyz + llXy0 + llX0z + llX00) / 4.0f;
				float _ll3 = (llX0z + llX00 + llXYz + llXY0) / 4.0f;
				float _ll4 = (llX00 + llX0Z + llXY0 + llXYZ) / 4.0f;
				ll1 = static_cast<float>(_ll1 * (1.0 - tileShapeY0) * tileShapeZ1 + _ll2 * (1.0 - tileShapeY0) * (1.0 - tileShapeZ1) + _ll3 * tileShapeY0 * (1.0 - tileShapeZ1) + _ll4 * tileShapeY0
                                         * tileShapeZ1);
				ll2 = static_cast<float>(_ll1 * (1.0 - tileShapeY0) * tileShapeZ0 + _ll2 * (1.0 - tileShapeY0) * (1.0 - tileShapeZ0) + _ll3 * tileShapeY0 * (1.0 - tileShapeZ0) + _ll4 * tileShapeY0
                                         * tileShapeZ0);
				ll3 = static_cast<float>(_ll1 * (1.0 - tileShapeY1) * tileShapeZ0 + _ll2 * (1.0 - tileShapeY1) * (1.0 - tileShapeZ0) + _ll3 * tileShapeY1 * (1.0 - tileShapeZ0) + _ll4 * tileShapeY1
                                         * tileShapeZ0);
				ll4 = static_cast<float>(_ll1 * (1.0 - tileShapeY1) * tileShapeZ1 + _ll2 * (1.0 - tileShapeY1) * (1.0 - tileShapeZ1) + _ll3 * tileShapeY1 * (1.0 - tileShapeZ1) + _ll4 * tileShapeY1
                                         * tileShapeZ1);

				int _tc1 = blend(ccXy0, ccXyZ, ccX0Z, ccX00);
				int _tc4 = blend(ccX0Z, ccXY0, ccXYZ, ccX00);
				int _tc3 = blend(ccX0z, ccXYz, ccXY0, ccX00);
				int _tc2 = blend(ccXyz, ccXy0, ccX0z, ccX00);
				tc1 = blend(_tc1, _tc2, _tc3, _tc4, (1.0 - tileShapeY0) * tileShapeZ1, (1.0 - tileShapeY0) * (1.0 - tileShapeZ1), tileShapeY0 * (1.0 - tileShapeZ1), tileShapeY0 * tileShapeZ1);
				tc2 = blend(_tc1, _tc2, _tc3, _tc4, (1.0 - tileShapeY0) * tileShapeZ0, (1.0 - tileShapeY0) * (1.0 - tileShapeZ0), tileShapeY0 * (1.0 - tileShapeZ0), tileShapeY0 * tileShapeZ0);
				tc3 = blend(_tc1, _tc2, _tc3, _tc4, (1.0 - tileShapeY1) * tileShapeZ0, (1.0 - tileShapeY1) * (1.0 - tileShapeZ0), tileShapeY1 * (1.0 - tileShapeZ0), tileShapeY1 * tileShapeZ0);
				tc4 = blend(_tc1, _tc2, _tc3, _tc4, (1.0 - tileShapeY1) * tileShapeZ1, (1.0 - tileShapeY1) * (1.0 - tileShapeZ1), tileShapeY1 * (1.0 - tileShapeZ1), tileShapeY1 * tileShapeZ1);
			}
			else
			{
				ll1 = (llXy0 + llXyZ + llX00 + llX0Z) / 4.0f;
				ll2 = (llXyz + llXy0 + llX0z + llX00) / 4.0f;
				ll3 = (llX0z + llX00 + llXYz + llXY0) / 4.0f;
				ll4 = (llX00 + llX0Z + llXY0 + llXYZ) / 4.0f;

				tc1 = blend(ccXy0, ccXyZ, ccX0Z, ccX00);
				tc4 = blend(ccX0Z, ccXY0, ccXYZ, ccX00);
				tc3 = blend(ccX0z, ccXYz, ccXY0, ccX00);
				tc2 = blend(ccXyz, ccXy0, ccX0z, ccX00);
			}
		}
		else
		{
			ll1 = ll2 = ll3 = ll4 = llX00;
			tc1 = tc2 = tc3 = tc4 = ccX00;
		}
		c1r = c2r = c3r = c4r = ( tint5 ? pBaseRed : 1.0f ) * 0.6f;
		c1g = c2g = c3g = c4g = ( tint5 ? pBaseGreen : 1.0f ) * 0.6f;
		c1b = c2b = c3b = c4b = ( tint5 ? pBaseBlue : 1.0f ) * 0.6f;
		c1r *= ll1;
		c1g *= ll1;
		c1b *= ll1;
		c2r *= ll2;
		c2g *= ll2;
		c2b *= ll2;
		c3r *= ll3;
		c3g *= ll3;
		c3b *= ll3;
		c4r *= ll4;
		c4g *= ll4;
		c4b *= ll4;

		Icon_SPU *tex = getTexture(tt, level, pX, pY, pZ, 5);
		renderEast( tt, static_cast<float>(pX), static_cast<float>(pY), static_cast<float>(pZ), tex );
		if ( fancy && (tex == &Tile_SPU::ms_pTileData->iconData[Tile_SPU::grass_Id] && !hasFixedTexture() ))
		{
			c1r *= pBaseRed;
			c2r *= pBaseRed;
			c3r *= pBaseRed;
			c4r *= pBaseRed;
			c1g *= pBaseGreen;
			c2g *= pBaseGreen;
			c3g *= pBaseGreen;
			c4g *= pBaseGreen;
			c1b *= pBaseBlue;
			c2b *= pBaseBlue;
			c3b *= pBaseBlue;
			c4b *= pBaseBlue;

			bool prev = t->setMipmapEnable( false );	// 4J added - this is rendering the little bit of grass at the top of the side of dirt, don't mipmap it
			renderEast( tt, static_cast<float>(pX), static_cast<float>(pY), static_cast<float>(pZ), GrassTile_SPU::getSideTextureOverlay() );
			t->setMipmapEnable( prev );
		}
	}
	applyAmbienceOcclusion = false;

	return true;

}

bool TileRenderer_SPU::tesselateBlockInWorldWithAmbienceOcclusionOldLighting( Tile_SPU* tt, int pX, int pY, int pZ,
																		  float pBaseRed, float pBaseGreen,
																		  float pBaseBlue )
{
// 
// 	// 4J - added these faceFlags so we can detect whether this block is going to have no visible faces and early out
// 	// the original code checked noCulling and shouldRenderFace directly where faceFlags is used now
// 	int		faceFlags = 0;
// 	if ( noCulling )
// 	{
// 		faceFlags = 0x3f;
// 	}
// 	else
// 	{
// 		faceFlags |= tt->shouldRenderFace( level, pX, pY - 1, pZ, 0 ) ? 0x01 : 0;
// 		faceFlags |= tt->shouldRenderFace( level, pX, pY + 1, pZ, 1 ) ? 0x02 : 0;
// 		faceFlags |= tt->shouldRenderFace( level, pX, pY, pZ - 1, 2 ) ? 0x04 : 0;
// 		faceFlags |= tt->shouldRenderFace( level, pX, pY, pZ + 1, 3 ) ? 0x08 : 0;
// 		faceFlags |= tt->shouldRenderFace( level, pX - 1, pY, pZ, 4 ) ? 0x10 : 0;
// 		faceFlags |= tt->shouldRenderFace( level, pX + 1, pY, pZ, 5 ) ? 0x20 : 0;
// 	}
// 	if ( faceFlags == 0 )
// 	{
// 		return false;
// 	}
// 
// 	applyAmbienceOcclusion = true;
// 	float	ll1 = ll000;
// 	float	ll2 = ll000;
// 	float	ll3 = ll000;
// 	float	ll4 = ll000;
// 	bool	tint0 = true;
// 	bool	tint1 = true;
// 	bool	tint2 = true;
// 	bool	tint3 = true;
// 	bool	tint4 = true;
// 	bool	tint5 = true;
// 
// 
// 	ll000 = tt->getBrightness( level, pX, pY, pZ );
// 	llx00 = tt->getBrightness( level, pX - 1, pY, pZ );
// 	ll0y0 = tt->getBrightness( level, pX, pY - 1, pZ );
// 	ll00z = tt->getBrightness( level, pX, pY, pZ - 1 );
// 	llX00 = tt->getBrightness( level, pX + 1, pY, pZ );
// 	ll0Y0 = tt->getBrightness( level, pX, pY + 1, pZ );
// 	ll00Z = tt->getBrightness( level, pX, pY, pZ + 1 );
// 
// 	llTransXY0 = level->m_tileData.transculent[level->getTile( pX + 1, pY + 1, pZ )];
// 	llTransXy0 = level->m_tileData.transculent[level->getTile( pX + 1, pY - 1, pZ )];
// 	llTransX0Z = level->m_tileData.transculent[level->getTile( pX + 1, pY, pZ + 1 )];
// 	llTransX0z = level->m_tileData.transculent[level->getTile( pX + 1, pY, pZ - 1 )];
// 	llTransxY0 = level->m_tileData.transculent[level->getTile( pX - 1, pY + 1, pZ )];
// 	llTransxy0 = level->m_tileData.transculent[level->getTile( pX - 1, pY - 1, pZ )];
// 	llTransx0z = level->m_tileData.transculent[level->getTile( pX - 1, pY, pZ - 1 )];
// 	llTransx0Z = level->m_tileData.transculent[level->getTile( pX - 1, pY, pZ + 1 )];
// 	llTrans0YZ = level->m_tileData.transculent[level->getTile( pX, pY + 1, pZ + 1 )];
// 	llTrans0Yz = level->m_tileData.transculent[level->getTile( pX, pY + 1, pZ - 1 )];
// 	llTrans0yZ = level->m_tileData.transculent[level->getTile( pX, pY - 1, pZ + 1 )];
// 	llTrans0yz = level->m_tileData.transculent[level->getTile( pX, pY - 1, pZ - 1 )];
// 
// 	spu_print("Have to add texture name check here\n");
// 	if ( getTexture(tt)->getName().compare(L"grass_top") == 0 ) tint0 = tint2 = tint3 = tint4 = tint5 = false;
// 	if ( hasFixedTexture() ) tint0 = tint2 = tint3 = tint4 = tint5 = false;
// 
// 	if ( faceFlags & 0x01 )
// 	{
// 		if ( blsmooth > 0 )
// 		{
// 			pY--;
// 
// 			llxy0 = tt->getBrightness( level, pX - 1, pY, pZ );
// 			ll0yz = tt->getBrightness( level, pX, pY, pZ - 1 );
// 			ll0yZ = tt->getBrightness( level, pX, pY, pZ + 1 );
// 			llXy0 = tt->getBrightness( level, pX + 1, pY, pZ );
// 
// 			if ( llTrans0yz || llTransxy0 )
// 			{
// 				llxyz = tt->getBrightness( level, pX - 1, pY, pZ - 1 );
// 			}
// 			else
// 			{
// 				llxyz = llxy0;
// 			}
// 			if ( llTrans0yZ || llTransxy0 )
// 			{
// 				llxyZ = tt->getBrightness( level, pX - 1, pY, pZ + 1 );
// 			}
// 			else
// 			{
// 				llxyZ = llxy0;
// 			}
// 			if ( llTrans0yz || llTransXy0 )
// 			{
// 				llXyz = tt->getBrightness( level, pX + 1, pY, pZ - 1 );
// 			}
// 			else
// 			{
// 				llXyz = llXy0;
// 			}
// 			if ( llTrans0yZ || llTransXy0 )
// 			{
// 				llXyZ = tt->getBrightness( level, pX + 1, pY, pZ + 1 );
// 			}
// 			else
// 			{
// 				llXyZ = llXy0;
// 			}
// 
// 			pY++;
// 			ll1 = ( llxyZ + llxy0 + ll0yZ + ll0y0 ) / 4.0f;
// 			ll4 = ( ll0yZ + ll0y0 + llXyZ + llXy0 ) / 4.0f;
// 			ll3 = ( ll0y0 + ll0yz + llXy0 + llXyz ) / 4.0f;
// 			ll2 = ( llxy0 + llxyz + ll0y0 + ll0yz ) / 4.0f;
// 		}
// 		else
// 		{
// 			ll1 = ll2 = ll3 = ll4 = ll0y0;
// 		}
// 		c1r = c2r = c3r = c4r = ( tint0 ? pBaseRed : 1.0f ) * 0.5f;
// 		c1g = c2g = c3g = c4g = ( tint0 ? pBaseGreen : 1.0f ) * 0.5f;
// 		c1b = c2b = c3b = c4b = ( tint0 ? pBaseBlue : 1.0f ) * 0.5f;
// 		c1r *= ll1;
// 		c1g *= ll1;
// 		c1b *= ll1;
// 		c2r *= ll2;
// 		c2g *= ll2;
// 		c2b *= ll2;
// 		c3r *= ll3;
// 		c3g *= ll3;
// 		c3b *= ll3;
// 		c4r *= ll4;
// 		c4g *= ll4;
// 		c4b *= ll4;
// 
// 		renderFaceDown( tt, ( float )pX, ( float )pY, ( float )pZ, getTexture( tt, level, pX, pY, pZ, 0 ) );
// 	}
// 	if ( faceFlags & 0x02 )
// 	{
// 		if ( blsmooth > 0 )
// 		{
// 			pY++;
// 
// 			llxY0 = tt->getBrightness( level, pX - 1, pY, pZ );
// 			llXY0 = tt->getBrightness( level, pX + 1, pY, pZ );
// 			ll0Yz = tt->getBrightness( level, pX, pY, pZ - 1 );
// 			ll0YZ = tt->getBrightness( level, pX, pY, pZ + 1 );
// 
// 			if ( llTrans0Yz || llTransxY0 )
// 			{
// 				llxYz = tt->getBrightness( level, pX - 1, pY, pZ - 1 );
// 			}
// 			else
// 			{
// 				llxYz = llxY0;
// 			}
// 			if ( llTrans0Yz || llTransXY0 )
// 			{
// 				llXYz = tt->getBrightness( level, pX + 1, pY, pZ - 1 );
// 			}
// 			else
// 			{
// 				llXYz = llXY0;
// 			}
// 			if ( llTrans0YZ || llTransxY0 )
// 			{
// 				llxYZ = tt->getBrightness( level, pX - 1, pY, pZ + 1 );
// 			}
// 			else
// 			{
// 				llxYZ = llxY0;
// 			}
// 			if ( llTrans0YZ || llTransXY0 )
// 			{
// 				llXYZ = tt->getBrightness( level, pX + 1, pY, pZ + 1 );
// 			}
// 			else
// 			{
// 				llXYZ = llXY0;
// 			}
// 			pY--;
// 
// 			ll4 = ( llxYZ + llxY0 + ll0YZ + ll0Y0 ) / 4.0f;
// 			ll1 = ( ll0YZ + ll0Y0 + llXYZ + llXY0 ) / 4.0f;
// 			ll2 = ( ll0Y0 + ll0Yz + llXY0 + llXYz ) / 4.0f;
// 			ll3 = ( llxY0 + llxYz + ll0Y0 + ll0Yz ) / 4.0f;
// 		}
// 		else
// 		{
// 			ll1 = ll2 = ll3 = ll4 = ll0Y0;
// 		}
// 		c1r = c2r = c3r = c4r = ( tint1 ? pBaseRed : 1.0f );
// 		c1g = c2g = c3g = c4g = ( tint1 ? pBaseGreen : 1.0f );
// 		c1b = c2b = c3b = c4b = ( tint1 ? pBaseBlue : 1.0f );
// 		c1r *= ll1;
// 		c1g *= ll1;
// 		c1b *= ll1;
// 		c2r *= ll2;
// 		c2g *= ll2;
// 		c2b *= ll2;
// 		c3r *= ll3;
// 		c3g *= ll3;
// 		c3b *= ll3;
// 		c4r *= ll4;
// 		c4g *= ll4;
// 		c4b *= ll4;
// 		renderFaceUp( tt, ( float )pX, ( float )pY, ( float )pZ, getTexture( tt, level, pX, pY, pZ, 1 ) );
// 	}
// 	if ( faceFlags & 0x04 )
// 	{
// 		if ( blsmooth > 0 )
// 		{
// 			pZ--;
// 			llx0z = tt->getBrightness( level, pX - 1, pY, pZ );
// 			ll0yz = tt->getBrightness( level, pX, pY - 1, pZ );
// 			ll0Yz = tt->getBrightness( level, pX, pY + 1, pZ );
// 			llX0z = tt->getBrightness( level, pX + 1, pY, pZ );
// 
// 			if ( llTransx0z || llTrans0yz )
// 			{
// 				llxyz = tt->getBrightness( level, pX - 1, pY - 1, pZ );
// 			}
// 			else
// 			{
// 				llxyz = llx0z;
// 			}
// 			if ( llTransx0z || llTrans0Yz )
// 			{
// 				llxYz = tt->getBrightness( level, pX - 1, pY + 1, pZ );
// 			}
// 			else
// 			{
// 				llxYz = llx0z;
// 			}
// 			if ( llTransX0z || llTrans0yz )
// 			{
// 				llXyz = tt->getBrightness( level, pX + 1, pY - 1, pZ );
// 			}
// 			else
// 			{
// 				llXyz = llX0z;
// 			}
// 			if ( llTransX0z || llTrans0Yz )
// 			{
// 				llXYz = tt->getBrightness( level, pX + 1, pY + 1, pZ );
// 			}
// 			else
// 			{
// 				llXYz = llX0z;
// 			}
// 			pZ++;
// 			ll1 = ( llx0z + llxYz + ll00z + ll0Yz ) / 4.0f;
// 			ll2 = ( ll00z + ll0Yz + llX0z + llXYz ) / 4.0f;
// 			ll3 = ( ll0yz + ll00z + llXyz + llX0z ) / 4.0f;
// 			ll4 = ( llxyz + llx0z + ll0yz + ll00z ) / 4.0f;
// 		}
// 		else
// 		{
// 			ll1 = ll2 = ll3 = ll4 = ll00z;
// 		}
// 		c1r = c2r = c3r = c4r = ( tint2 ? pBaseRed : 1.0f ) * 0.8f;
// 		c1g = c2g = c3g = c4g = ( tint2 ? pBaseGreen : 1.0f ) * 0.8f;
// 		c1b = c2b = c3b = c4b = ( tint2 ? pBaseBlue : 1.0f ) * 0.8f;
// 		c1r *= ll1;
// 		c1g *= ll1;
// 		c1b *= ll1;
// 		c2r *= ll2;
// 		c2g *= ll2;
// 		c2b *= ll2;
// 		c3r *= ll3;
// 		c3g *= ll3;
// 		c3b *= ll3;
// 		c4r *= ll4;
// 		c4g *= ll4;
// 		c4b *= ll4;
// 
// 		Icon_SPU *tex = getTexture(tt, level, pX, pY, pZ, 2);
// 		renderNorth( tt, ( float )pX, ( float )pY, ( float )pZ, tex );
// 
// 		if ( fancy && (tex->getName().compare(L"grass_side") == 0) && !hasFixedTexture())
// 		{
// 			c1r *= pBaseRed;
// 			c2r *= pBaseRed;
// 			c3r *= pBaseRed;
// 			c4r *= pBaseRed;
// 			c1g *= pBaseGreen;
// 			c2g *= pBaseGreen;
// 			c3g *= pBaseGreen;
// 			c4g *= pBaseGreen;
// 			c1b *= pBaseBlue;
// 			c2b *= pBaseBlue;
// 			c3b *= pBaseBlue;
// 			c4b *= pBaseBlue;
// 			renderNorth( tt, ( float )pX, ( float )pY, ( float )pZ, getGrassSideTextureOverlay() );
// 		}
// 	}
// 	if ( faceFlags & 0x08 )
// 	{
// 		if ( blsmooth > 0 )
// 		{
// 			pZ++;
// 
// 			llx0Z = tt->getBrightness( level, pX - 1, pY, pZ );
// 			llX0Z = tt->getBrightness( level, pX + 1, pY, pZ );
// 			ll0yZ = tt->getBrightness( level, pX, pY - 1, pZ );
// 			ll0YZ = tt->getBrightness( level, pX, pY + 1, pZ );
// 
// 			if ( llTransx0Z || llTrans0yZ )
// 			{
// 				llxyZ = tt->getBrightness( level, pX - 1, pY - 1, pZ );
// 			}
// 			else
// 			{
// 				llxyZ = llx0Z;
// 			}
// 			if ( llTransx0Z || llTrans0YZ )
// 			{
// 				llxYZ = tt->getBrightness( level, pX - 1, pY + 1, pZ );
// 			}
// 			else
// 			{
// 				llxYZ = llx0Z;
// 			}
// 			if ( llTransX0Z || llTrans0yZ )
// 			{
// 				llXyZ = tt->getBrightness( level, pX + 1, pY - 1, pZ );
// 			}
// 			else
// 			{
// 				llXyZ = llX0Z;
// 			}
// 			if ( llTransX0Z || llTrans0YZ )
// 			{
// 				llXYZ = tt->getBrightness( level, pX + 1, pY + 1, pZ );
// 			}
// 			else
// 			{
// 				llXYZ = llX0Z;
// 			}
// 			pZ--;
// 			ll1 = ( llx0Z + llxYZ + ll00Z + ll0YZ ) / 4.0f;
// 			ll4 = ( ll00Z + ll0YZ + llX0Z + llXYZ ) / 4.0f;
// 			ll3 = ( ll0yZ + ll00Z + llXyZ + llX0Z ) / 4.0f;
// 			ll2 = ( llxyZ + llx0Z + ll0yZ + ll00Z ) / 4.0f;
// 		}
// 		else
// 		{
// 			ll1 = ll2 = ll3 = ll4 = ll00Z;
// 		}
// 		c1r = c2r = c3r = c4r = ( tint3 ? pBaseRed : 1.0f ) * 0.8f;
// 		c1g = c2g = c3g = c4g = ( tint3 ? pBaseGreen : 1.0f ) * 0.8f;
// 		c1b = c2b = c3b = c4b = ( tint3 ? pBaseBlue : 1.0f ) * 0.8f;
// 		c1r *= ll1;
// 		c1g *= ll1;
// 		c1b *= ll1;
// 		c2r *= ll2;
// 		c2g *= ll2;
// 		c2b *= ll2;
// 		c3r *= ll3;
// 		c3g *= ll3;
// 		c3b *= ll3;
// 		c4r *= ll4;
// 		c4g *= ll4;
// 		c4b *= ll4;
// 		Icon_SPU *tex = getTexture(tt, level, pX, pY, pZ, 3);
// 		renderSouth( tt, ( float )pX, ( float )pY, ( float )pZ, getTexture(tt, level, pX, pY, pZ, 3 ) );
// 		if ( fancy && (tex->getName().compare(L"grass_side") == 0) && !hasFixedTexture() )
// 		{
// 			c1r *= pBaseRed;
// 			c2r *= pBaseRed;
// 			c3r *= pBaseRed;
// 			c4r *= pBaseRed;
// 			c1g *= pBaseGreen;
// 			c2g *= pBaseGreen;
// 			c3g *= pBaseGreen;
// 			c4g *= pBaseGreen;
// 			c1b *= pBaseBlue;
// 			c2b *= pBaseBlue;
// 			c3b *= pBaseBlue;
// 			c4b *= pBaseBlue;
// 			renderSouth( tt, ( float )pX, ( float )pY, ( float )pZ, getGrassSideTextureOverlay() );
// 		}
// 	}
// 	if ( faceFlags & 0x10 )
// 	{
// 		if ( blsmooth > 0 )
// 		{
// 			pX--;
// 			llxy0 = tt->getBrightness( level, pX, pY - 1, pZ );
// 			llx0z = tt->getBrightness( level, pX, pY, pZ - 1 );
// 			llx0Z = tt->getBrightness( level, pX, pY, pZ + 1 );
// 			llxY0 = tt->getBrightness( level, pX, pY + 1, pZ );
// 
// 			if ( llTransx0z || llTransxy0 )
// 			{
// 				llxyz = tt->getBrightness( level, pX, pY - 1, pZ - 1 );
// 			}
// 			else
// 			{
// 				llxyz = llx0z;
// 			}
// 			if ( llTransx0Z || llTransxy0 )
// 			{
// 				llxyZ = tt->getBrightness( level, pX, pY - 1, pZ + 1 );
// 			}
// 			else
// 			{
// 				llxyZ = llx0Z;
// 			}
// 			if ( llTransx0z || llTransxY0 )
// 			{
// 				llxYz = tt->getBrightness( level, pX, pY + 1, pZ - 1 );
// 			}
// 			else
// 			{
// 				llxYz = llx0z;
// 			}
// 			if ( llTransx0Z || llTransxY0 )
// 			{
// 				llxYZ = tt->getBrightness( level, pX, pY + 1, pZ + 1 );
// 			}
// 			else
// 			{
// 				llxYZ = llx0Z;
// 			}
// 			pX++;
// 			ll4 = ( llxy0 + llxyZ + llx00 + llx0Z ) / 4.0f;
// 			ll1 = ( llx00 + llx0Z + llxY0 + llxYZ ) / 4.0f;
// 			ll2 = ( llx0z + llx00 + llxYz + llxY0 ) / 4.0f;
// 			ll3 = ( llxyz + llxy0 + llx0z + llx00 ) / 4.0f;
// 		}
// 		else
// 		{
// 			ll1 = ll2 = ll3 = ll4 = llx00;
// 		}
// 		c1r = c2r = c3r = c4r = ( tint4 ? pBaseRed : 1.0f ) * 0.6f;
// 		c1g = c2g = c3g = c4g = ( tint4 ? pBaseGreen : 1.0f ) * 0.6f;
// 		c1b = c2b = c3b = c4b = ( tint4 ? pBaseBlue : 1.0f ) * 0.6f;
// 		c1r *= ll1;
// 		c1g *= ll1;
// 		c1b *= ll1;
// 		c2r *= ll2;
// 		c2g *= ll2;
// 		c2b *= ll2;
// 		c3r *= ll3;
// 		c3g *= ll3;
// 		c3b *= ll3;
// 		c4r *= ll4;
// 		c4g *= ll4;
// 		c4b *= ll4;
// 		Icon_SPU *tex = getTexture(tt, level, pX, pY, pZ, 4);
// 		renderWest( tt, ( float )pX, ( float )pY, ( float )pZ, tex );
// 		if ( fancy &&(tex->getName().compare(L"grass_side") == 0) && !hasFixedTexture() )
// 		{
// 			c1r *= pBaseRed;
// 			c2r *= pBaseRed;
// 			c3r *= pBaseRed;
// 			c4r *= pBaseRed;
// 			c1g *= pBaseGreen;
// 			c2g *= pBaseGreen;
// 			c3g *= pBaseGreen;
// 			c4g *= pBaseGreen;
// 			c1b *= pBaseBlue;
// 			c2b *= pBaseBlue;
// 			c3b *= pBaseBlue;
// 			c4b *= pBaseBlue;
// 			renderWest( tt, ( float )pX, ( float )pY, ( float )pZ, getGrassSideTextureOverlay() );
// 		}
// 	}
// 	if ( faceFlags & 0x20 )
// 	{
// 		if ( blsmooth > 0 )
// 		{
// 			pX++;
// 			llXy0 = tt->getBrightness( level, pX, pY - 1, pZ );
// 			llX0z = tt->getBrightness( level, pX, pY, pZ - 1 );
// 			llX0Z = tt->getBrightness( level, pX, pY, pZ + 1 );
// 			llXY0 = tt->getBrightness( level, pX, pY + 1, pZ );
// 
// 			if ( llTransXy0 || llTransX0z )
// 			{
// 				llXyz = tt->getBrightness( level, pX, pY - 1, pZ - 1 );
// 			}
// 			else
// 			{
// 				llXyz = llX0z;
// 			}
// 			if ( llTransXy0 || llTransX0Z )
// 			{
// 				llXyZ = tt->getBrightness( level, pX, pY - 1, pZ + 1 );
// 			}
// 			else
// 			{
// 				llXyZ = llX0Z;
// 			}
// 			if ( llTransXY0 || llTransX0z )
// 			{
// 				llXYz = tt->getBrightness( level, pX, pY + 1, pZ - 1 );
// 			}
// 			else
// 			{
// 				llXYz = llX0z;
// 			}
// 			if ( llTransXY0 || llTransX0Z )
// 			{
// 				llXYZ = tt->getBrightness( level, pX, pY + 1, pZ + 1 );
// 			}
// 			else
// 			{
// 				llXYZ = llX0Z;
// 			}
// 			pX--;
// 			ll1 = ( llXy0 + llXyZ + llX00 + llX0Z ) / 4.0f;
// 			ll4 = ( llX00 + llX0Z + llXY0 + llXYZ ) / 4.0f;
// 			ll3 = ( llX0z + llX00 + llXYz + llXY0 ) / 4.0f;
// 			ll2 = ( llXyz + llXy0 + llX0z + llX00 ) / 4.0f;
// 		}
// 		else
// 		{
// 			ll1 = ll2 = ll3 = ll4 = llX00;
// 		}
// 		c1r = c2r = c3r = c4r = ( tint5 ? pBaseRed : 1.0f ) * 0.6f;
// 		c1g = c2g = c3g = c4g = ( tint5 ? pBaseGreen : 1.0f ) * 0.6f;
// 		c1b = c2b = c3b = c4b = ( tint5 ? pBaseBlue : 1.0f ) * 0.6f;
// 		c1r *= ll1;
// 		c1g *= ll1;
// 		c1b *= ll1;
// 		c2r *= ll2;
// 		c2g *= ll2;
// 		c2b *= ll2;
// 		c3r *= ll3;
// 		c3g *= ll3;
// 		c3b *= ll3;
// 		c4r *= ll4;
// 		c4g *= ll4;
// 		c4b *= ll4;
// 
// 		Icon_SPU *tex = getTexture(tt, level, pX, pY, pZ, 5);
// 		renderEast( tt, ( float )pX, ( float )pY, ( float )pZ, tex );
// 		if ( fancy && (tex->getName().compare(L"grass_side") == 0) && !hasFixedTexture() )
// 		{
// 			c1r *= pBaseRed;
// 			c2r *= pBaseRed;
// 			c3r *= pBaseRed;
// 			c4r *= pBaseRed;
// 			c1g *= pBaseGreen;
// 			c2g *= pBaseGreen;
// 			c3g *= pBaseGreen;
// 			c4g *= pBaseGreen;
// 			c1b *= pBaseBlue;
// 			c2b *= pBaseBlue;
// 			c3b *= pBaseBlue;
// 			c4b *= pBaseBlue;
// 			renderEast( tt, ( float )pX, ( float )pY, ( float )pZ, getGrassSideTextureOverlay() );
// 		}
// 	}
// 	applyAmbienceOcclusion = false;
// 
	return true;

}

// 4J - brought forward from 1.8.2
int TileRenderer_SPU::blend( int a, int b, int c, int def )
{
	if ( a == 0 ) a = def;
	if ( b == 0 ) b = def;
	if ( c == 0 ) c = def;
	return ( ( a + b + c + def ) >> 2 ) & 0xff00ff;
}

int TileRenderer_SPU::blend(int a, int b, int c, int d, float fa, float fb, float fc, float fd)
{

	int top = static_cast<int>((float)((a >> 16) & 0xff) * fa + (float)((b >> 16) & 0xff) * fb + (float)((c >> 16) & 0xff) * fc + (float)((d >> 16) & 0xff) * fd) & 0xff;
	int bottom = static_cast<int>((float)(a & 0xff) * fa + (float)(b & 0xff) * fb + (float)(c & 0xff) * fc + (float)(d & 0xff) * fd) & 0xff;
	return (top << 16) | bottom;
}

bool TileRenderer_SPU::tesselateBlockInWorld( Tile_SPU* tt, int x, int y, int z, float r, float g, float b )
{
	applyAmbienceOcclusion = false;

	Tesselator_SPU* t = getTesselator();

	bool		changed = false;
	float		c10 = 0.5f;
	float		c11 = 1;
	float		c2 = 0.8f;
	float		c3 = 0.6f;

	float		r11 = c11 * r;
	float		g11 = c11 * g;
	float		b11 = c11 * b;

	float		r10 = c10;
	float		r2 = c2;
	float		r3 = c3;

	float		g10 = c10;
	float		g2 = c2;
	float		g3 = c3;

	float		b10 = c10;
	float		b2 = c2;
	float		b3 = c3;

	if ( tt->id != Tile_SPU::grass_Id )
	{
		r10 *= r;
		r2 *= r;
		r3 *= r;

		g10 *= g;
		g2 *= g;
		g3 *= g;

		b10 *= b;
		b2 *= b;
		b3 *= b;
	}

	int			centerColor = 0;
	float		centerBrightness = 0.0f;
	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		centerColor = tt->getLightColor( level, x, y, z );
	}
	else
	{
		centerBrightness = tt->getBrightness( level, x, y, z );
	}

	if ( noCulling || tt->shouldRenderFace( level, x, y - 1, z, Facing::DOWN ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeY0 > 0 ? centerColor : tt->getLightColor( level, x, y - 1, z ) );
			t->color( r10, g10, b10 );
		}
		else
		{
			float br = tt->getBrightness( level, x, y - 1, z );
			t->color( r10 * br, g10 * br, b10 * br );
		}
		renderFaceDown( tt, x, y, z, getTexture(tt, level, x, y, z, 0 ) );
		changed = true;
	}

	if ( noCulling || tt->shouldRenderFace( level, x, y + 1, z, Facing::UP ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeY1 < 1 ? centerColor : tt->getLightColor( level, x, y + 1, z ) );
			t->color( r11, g11, b11 );
		}
		else
		{
			float br = tt->getBrightness( level, x, y + 1, z );
		//	spu_print("need to add material settings\n");
//			if ( tileShapeY1 != 1 && !tt->material->isLiquid() ) br = centerBrightness;
			t->color( r11 * br, g11 * br, b11 * br );
		}
		renderFaceUp( tt, x, y, z, getTexture(tt, level, x, y, z, 1 ) );
		changed = true;
	}

	if ( noCulling || tt->shouldRenderFace( level, x, y, z - 1, Facing::NORTH ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeZ0 > 0 ? centerColor : tt->getLightColor( level, x, y, z - 1 ) );
			t->color( r2, g2, b2 );
		}
		else
		{
			float br = tt->getBrightness( level, x, y, z - 1 );
			if ( tileShapeZ0 > 0 ) br = centerBrightness;
			t->color( r2 * br, g2 * br, b2 * br );
		}

		Icon_SPU *tex = getTexture(tt, level, x, y, z, 2);
		renderNorth( tt, x, y, z, tex );

		if ( fancy && (tex == &Tile_SPU::ms_pTileData->iconData[Tile_SPU::grass_Id] && !hasFixedTexture() ))
		{
			t->color( r2 * r, g2 * g, b2 * b );
			renderNorth( tt, x, y, z, GrassTile_SPU::getSideTextureOverlay() );
		}
		changed = true;
	}

	if ( noCulling || tt->shouldRenderFace( level, x, y, z + 1, Facing::SOUTH ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeZ1 < 1 ? centerColor : tt->getLightColor( level, x, y, z + 1 ) );
			t->color( r2, g2, b2 );
		}
		else
		{
			float br = tt->getBrightness( level, x, y, z + 1 );
			if ( tileShapeZ1 < 1 ) br = centerBrightness;
			t->color( r2 * br, g2 * br, b2 * br );
		}
		Icon_SPU *tex = getTexture(tt, level, x, y, z, 3);
		renderSouth( tt, x, y, z, tex );

		if ( fancy && (tex == &Tile_SPU::ms_pTileData->iconData[Tile_SPU::grass_Id] && !hasFixedTexture() ))
		{
			t->color( r2 * r, g2 * g, b2 * b );
			renderSouth( tt, x, y, z, GrassTile_SPU::getSideTextureOverlay() );
		}
		changed = true;
	}

	if ( noCulling || tt->shouldRenderFace( level, x - 1, y, z, Facing::WEST ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeX0 > 0 ? centerColor : tt->getLightColor( level, x - 1, y, z ) );
			t->color( r3, g3, b3 );
		}
		else
		{
			float br = tt->getBrightness( level, x - 1, y, z );
			if ( tileShapeX0 > 0 ) br = centerBrightness;
			t->color( r3 * br, g3 * br, b3 * br );
		}
		Icon_SPU *tex = getTexture(tt, level, x, y, z, 4);
		renderWest( tt, x, y, z, tex );

		if ( fancy && (tex == &Tile_SPU::ms_pTileData->iconData[Tile_SPU::grass_Id] && !hasFixedTexture() ))
		{
			t->color( r3 * r, g3 * g, b3 * b );
			renderWest( tt, x, y, z, GrassTile_SPU::getSideTextureOverlay() );
		}
		changed = true;
	}

	if ( noCulling || tt->shouldRenderFace( level, x + 1, y, z, Facing::EAST ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeX1 < 1 ? centerColor : tt->getLightColor( level, x + 1, y, z ) );
			t->color( r3, g3, b3 );
		}
		else
		{
			float br = tt->getBrightness( level, x + 1, y, z );
			if ( tileShapeX1 < 1 ) br = centerBrightness;
			t->color( r3 * br, g3 * br, b3 * br );
		}
		Icon_SPU *tex = getTexture(tt, level, x, y, z, 5);
		renderEast( tt, x, y, z, tex );

		if ( fancy && (tex == &Tile_SPU::ms_pTileData->iconData[Tile_SPU::grass_Id] && !hasFixedTexture() ))
		{
			t->color( r3 * r, g3 * g, b3 * b );
			renderEast( tt, x, y, z, GrassTile_SPU::getSideTextureOverlay() );
		}
		changed = true;
	}

	return changed;

}

bool TileRenderer_SPU::tesselateCactusInWorld( Tile_SPU* tt, int x, int y, int z )
{
	int		col = tt->getColor( level, x, y, z );
	float	r = ( ( col >> 16 ) & 0xff ) / 255.0f;
	float	g = ( ( col >> 8 ) & 0xff ) / 255.0f;
	float	b = ( ( col )& 0xff ) / 255.0f;

	if ( isAnaglyph3d() )
	{
		float	cr = ( r * 30 + g * 59 + b * 11 ) / 100;
		float	cg = ( r * 30 + g * 70 ) / ( 100 );
		float	cb = ( r * 30 + b * 70 ) / ( 100 );

		r = cr;
		g = cg;
		b = cb;
	}
	return tesselateCactusInWorld( tt, x, y, z, r, g, b );
}

bool TileRenderer_SPU::tesselateCactusInWorld( Tile_SPU* tt, int x, int y, int z, float r, float g, float b )
{
	Tesselator_SPU* t = getTesselator();

	bool		changed = false;
	float		c10 = 0.5f;
	float		c11 = 1;
	float		c2 = 0.8f;
	float		c3 = 0.6f;

	float		r10 = c10 * r;
	float		r11 = c11 * r;
	float		r2 = c2 * r;
	float		r3 = c3 * r;

	float		g10 = c10 * g;
	float		g11 = c11 * g;
	float		g2 = c2 * g;
	float		g3 = c3 * g;

	float		b10 = c10 * b;
	float		b11 = c11 * b;
	float		b2 = c2 * b;
	float		b3 = c3 * b;

	float		s = 1 / 16.0f;

	float		centerBrightness = 0.0f;
	int			centerColor = 0;
	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		centerColor = tt->getLightColor( level, x, y, z );
	}
	else
	{
		centerBrightness = tt->getBrightness( level, x, y, z );
	}

	if ( noCulling || tt->shouldRenderFace( level, x, y - 1, z, 0 ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeY0 > 0 ? centerColor : tt->getLightColor( level, x, y - 1, z ) );
			t->color( r10, g10, b10 );
		}
		else
		{
			float br = tt->getBrightness( level, x, y - 1, z );
			t->color( r10 * br, g10 * br, b10 * br );
		}
		renderFaceDown( tt, x, y, z, getTexture( tt, level, x, y, z, 0 ) );
		changed = true;
	}

	if ( noCulling || tt->shouldRenderFace( level, x, y + 1, z, 1 ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeY1 < 1 ? centerColor : tt->getLightColor( level, x, y + 1, z ) );
			t->color( r11, g11, b11 );
		}
		else
		{
			float br = tt->getBrightness( level, x, y + 1, z );
			if ( tileShapeY1 != 1 && !tt->getMaterial()->isLiquid() ) br = centerBrightness;
			t->color( r11 * br, g11 * br, b11 * br );
		}
		renderFaceUp( tt, x, y, z, getTexture( tt, level, x, y, z, 1 ) );
		changed = true;
	}

	if ( noCulling || tt->shouldRenderFace( level, x, y, z - 1, 2 ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeZ0 > 0 ? centerColor : tt->getLightColor( level, x, y, z - 1 ) );
			t->color( r2, g2, b2 );
		}
		else
		{
			float br = tt->getBrightness( level, x, y, z - 1 );
			if ( tileShapeZ0 > 0 ) br = centerBrightness;
			t->color( r2 * br, g2 * br, b2 * br );
		}
		t->addOffset( 0, 0, s );
		renderNorth( tt, x, y, z, getTexture(tt, level, x, y, z, 2 ) );
		t->addOffset( 0, 0, -s );
		changed = true;
	}

	if ( noCulling || tt->shouldRenderFace( level, x, y, z + 1, 3 ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeZ1 < 1 ? centerColor : tt->getLightColor( level, x, y, z + 1 ) );
			t->color( r2, g2, b2 );
		}
		else
		{
			float br = tt->getBrightness( level, x, y, z + 1 );
			if ( tileShapeZ1 < 1 ) br = centerBrightness;
			t->color( r2 * br, g2 * br, b2 * br );
		}

		t->addOffset( 0, 0, -s );
		renderSouth( tt, x, y, z, getTexture(tt, level, x, y, z, 3 ) );
		t->addOffset( 0, 0, s );
		changed = true;
	}

	if ( noCulling || tt->shouldRenderFace( level, x - 1, y, z, 4 ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeX0 > 0 ? centerColor : tt->getLightColor( level, x - 1, y, z ) );
			t->color( r3, g3, b3 );
		}
		else
		{
			float br = tt->getBrightness( level, x - 1, y, z );
			if ( tileShapeX0 > 0 ) br = centerBrightness;
			t->color( r3 * br, g3 * br, b3 * br );
		}
		t->addOffset( s, 0, 0 );
		renderWest( tt, x, y, z, getTexture(tt, level, x, y, z, 4 ) );
		t->addOffset( -s, 0, 0 );
		changed = true;
	}

	if ( noCulling || tt->shouldRenderFace( level, x + 1, y, z, 5 ) )
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeX1 < 1 ? centerColor : tt->getLightColor( level, x + 1, y, z ) );
			t->color( r3, g3, b3 );
		}
		else
		{
			float br = tt->getBrightness( level, x + 1, y, z );
			if ( tileShapeX1 < 1 ) br = centerBrightness;
			t->color( r3 * br, g3 * br, b3 * br );
		}
		t->addOffset( -s, 0, 0 );
		renderEast( tt, x, y, z, getTexture(tt, level, x, y, z, 5 ) );
		t->addOffset( s, 0, 0 );
		changed = true;
	}

	return changed;
}

bool TileRenderer_SPU::tesselateFenceInWorld( FenceTile_SPU* tt, int x, int y, int z )
{
// #ifdef DISABLE_TESS_FUNCS
	bool	changed = false;

	float	a = 6 / 16.0f;
	float	b = 10 / 16.0f;
//	EnterCriticalSection( &Tile_SPU::m_csShape );
	setShape( a, 0, a, b, 1, b );
	tesselateBlockInWorld( tt, x, y, z );
	changed = true;

	bool	vertical = false;
	bool	horizontal = false;

	if (tt->connectsTo(level, x - 1, y, z) || tt->connectsTo(level, x + 1, y, z)) vertical = true;
	if (tt->connectsTo(level, x, y, z - 1) || tt->connectsTo(level, x, y, z + 1)) horizontal = true;

	bool l = tt->connectsTo(level, x - 1, y, z);
	bool r = tt->connectsTo(level, x + 1, y, z);
	bool u = tt->connectsTo(level, x, y, z - 1);
	bool d = tt->connectsTo(level, x, y, z + 1);

	if ( !vertical && !horizontal ) vertical = true;

	a = 7 / 16.0f;
	b = 9 / 16.0f;
	float	h0 = 12 / 16.0f;
	float	h1 = 15 / 16.0f;

	float	x0 = l ? 0 : a;
	float	x1 = r ? 1 : b;
	float	z0 = u ? 0 : a;
	float	z1 = d ? 1 : b;
	if ( vertical )
	{
		setShape( x0, h0, a, x1, h1, b );
		tesselateBlockInWorld( tt, x, y, z );
		changed = true;
	}
	if ( horizontal )
	{
		setShape( a, h0, z0, b, h1, z1 );
		tesselateBlockInWorld( tt, x, y, z );
		changed = true;
	}

	h0 = 6 / 16.0f;
	h1 = 9 / 16.0f;
	if ( vertical )
	{
		setShape( x0, h0, a, x1, h1, b );
		tesselateBlockInWorld( tt, x, y, z );
		changed = true;
	}
	if ( horizontal )
	{
		setShape( a, h0, z0, b, h1, z1 );
		tesselateBlockInWorld( tt, x, y, z );
		changed = true;
	}

	tt->updateShape(level, x, y, z);
//	LeaveCriticalSection( &Tile_SPU::m_csShape );
	return changed;
// #endif // #ifdef DISABLE_TESS_FUNCS
	return false;
}

bool TileRenderer_SPU::tesselateWallInWorld(WallTile_SPU *tt, int x, int y, int z)
{
	bool w = tt->connectsTo(level, x - 1, y, z);
	bool e = tt->connectsTo(level, x + 1, y, z);
	bool n = tt->connectsTo(level, x, y, z - 1);
	bool s = tt->connectsTo(level, x, y, z + 1);

	bool vertical = (n && s && !w && !e);
	bool horizontal = (!n && !s && w && e);
	bool emptyAbove = level->isEmptyTile(x, y + 1, z);

	if ((!vertical && !horizontal) || !emptyAbove)
	{
		// center post
		setShape(.5f - WallTile_SPU::POST_WIDTH, 0, .5f - WallTile_SPU::POST_WIDTH, .5f + WallTile_SPU::POST_WIDTH, WallTile_SPU::POST_HEIGHT, .5f + WallTile_SPU::POST_WIDTH);
		tesselateBlockInWorld(tt, x, y, z);

		if (w)
		{
			setShape(0, 0, .5f - WallTile_SPU::WALL_WIDTH, .5f - WallTile_SPU::POST_WIDTH, WallTile_SPU::WALL_HEIGHT, .5f + WallTile_SPU::WALL_WIDTH);
			tesselateBlockInWorld(tt, x, y, z);
		}
		if (e)
		{
			setShape(.5f + WallTile_SPU::POST_WIDTH, 0, .5f - WallTile_SPU::WALL_WIDTH, 1, WallTile_SPU::WALL_HEIGHT, .5f + WallTile_SPU::WALL_WIDTH);
			tesselateBlockInWorld(tt, x, y, z);
		}
		if (n)
		{
			setShape(.5f - WallTile_SPU::WALL_WIDTH, 0, 0, .5f + WallTile_SPU::WALL_WIDTH, WallTile_SPU::WALL_HEIGHT, .5f - WallTile_SPU::POST_WIDTH);
			tesselateBlockInWorld(tt, x, y, z);
		}
		if (s)
		{
			setShape(.5f - WallTile_SPU::WALL_WIDTH, 0, .5f + WallTile_SPU::POST_WIDTH, .5f + WallTile_SPU::WALL_WIDTH, WallTile_SPU::WALL_HEIGHT, 1);
			tesselateBlockInWorld(tt, x, y, z);
		}
	}
	else if (vertical)
	{
		// north-south wall
		setShape(.5f - WallTile_SPU::WALL_WIDTH, 0, 0, .5f + WallTile_SPU::WALL_WIDTH, WallTile_SPU::WALL_HEIGHT, 1);
		tesselateBlockInWorld(tt, x, y, z);
	}
	else
	{
		// west-east wall
		setShape(0, 0, .5f - WallTile_SPU::WALL_WIDTH, 1, WallTile_SPU::WALL_HEIGHT, .5f + WallTile_SPU::WALL_WIDTH);
		tesselateBlockInWorld(tt, x, y, z);
	}

	tt->updateShape(level, x, y, z);
	return true;
}


bool TileRenderer_SPU::tesselateEggInWorld(EggTile_SPU *tt, int x, int y, int z)
{
	bool changed = false;
//	EnterCriticalSection( &Tile_SPU::m_csShape );

    int y0 = 0;
    for (int i = 0; i < 8; i++)
	{
        int ww = 0;
        int hh = 1;
        if (i == 0) ww = 2;
        if (i == 1) ww = 3;
        if (i == 2) ww = 4;
        if (i == 3)
		{
            ww = 5;
            hh = 2;
        }
        if (i == 4)
		{
            ww = 6;
            hh = 3;
        }
        if (i == 5)
		{
            ww = 7;
            hh = 5;
        }
        if (i == 6)
		{
            ww = 6;
            hh = 2;
        }
        if (i == 7) ww = 3;
        float w = ww / 16.0f;
        float yy1 = 1 - (y0 / 16.0f);
        float yy0 = 1 - ((y0 + hh) / 16.0f);
        y0 += hh;
        setShape(0.5f - w, yy0, 0.5f - w, 0.5f + w, yy1, 0.5f + w);
        tesselateBlockInWorld(tt, x, y, z);
    }
    changed = true;

    setShape(0, 0, 0, 1, 1, 1);
//	LeaveCriticalSection( &Tile_SPU::m_csShape );
    return changed;
}

bool TileRenderer_SPU::tesselateFenceGateInWorld(FenceGateTile_SPU *tt, int x, int y, int z)
{
    bool changed = true;
#ifdef DISABLE_TESS_FUNCS

    int data = level->getData(x, y, z);
    bool isOpen = FenceGateTile_SPU::isOpen(data);
    int direction = FenceGateTile_SPU::getDirection(data);

    float h00 = 6 / 16.0f;
    float h01 = 9 / 16.0f;
    float h10 = 12 / 16.0f;
    float h11 = 15 / 16.0f;
    float h20 = 5 / 16.0f;
    float h21 = 16 / 16.0f;

	//if (((direction == Direction::NORTH || direction == Direction::SOUTH) && level->getTile(x - 1, y, z) == Tile_SPU::cobbleWall_Id && level->getTile(x + 1, y, z) == Tile_SPU::cobbleWall_Id)
	//	|| ((direction == Direction::EAST || direction == Direction::WEST) && level->getTile(x, y, z - 1) == Tile_SPU::cobbleWall_Id && level->getTile(x, y, z + 1) == Tile_SPU::cobbleWall_Id))
	//{
	//		h00 -= 3.0f / 16.0f;
	//		h01 -= 3.0f / 16.0f;
	//		h10 -= 3.0f / 16.0f;
	//		h11 -= 3.0f / 16.0f;
	//		h20 -= 3.0f / 16.0f;
	//		h21 -= 3.0f / 16.0f;
	//}

	noCulling = true;

//	EnterCriticalSection( &Tile_SPU::m_csShape );

    // edge sticks
    if (direction == Direction::EAST || direction == Direction::WEST)
	{
		upFlip = FLIP_CW;
        float x0 = 7 / 16.0f;
        float x1 = 9 / 16.0f;
        float z0 = 0 / 16.0f;
        float z1 = 2 / 16.0f;
        setShape(x0, h20, z0, x1, h21, z1);
        tesselateBlockInWorld(tt, x, y, z);

        z0 = 14 / 16.0f;
        z1 = 16 / 16.0f;
        setShape(x0, h20, z0, x1, h21, z1);
        tesselateBlockInWorld(tt, x, y, z);
		upFlip = FLIP_NONE;
    }
	else
	{
        float x0 = 0 / 16.0f;
        float x1 = 2 / 16.0f;
        float z0 = 7 / 16.0f;
        float z1 = 9 / 16.0f;
        setShape(x0, h20, z0, x1, h21, z1);
        tesselateBlockInWorld(tt, x, y, z);

        x0 = 14 / 16.0f;
        x1 = 16 / 16.0f;
        setShape(x0, h20, z0, x1, h21, z1);
        tesselateBlockInWorld(tt, x, y, z);
    }
	if (isOpen)
	{
		if (direction == Direction::NORTH || direction == Direction::SOUTH)
		{
			upFlip = FLIP_CW;
		}
		if (direction == Direction::EAST)
		{

			const float z00 = 0 / 16.0f;
			const float z01 = 2 / 16.0f;
			const float z10 = 14 / 16.0f;
			const float z11 = 16 / 16.0f;

			const float x0 = 9 / 16.0f;
			const float x1 = 13 / 16.0f;
			const float x2 = 15 / 16.0f;

			setShape(x1, h00, z00, x2, h11, z01);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x1, h00, z10, x2, h11, z11);
			tesselateBlockInWorld(tt, x, y, z);

			setShape(x0, h00, z00, x1, h01, z01);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x0, h00, z10, x1, h01, z11);
			tesselateBlockInWorld(tt, x, y, z);

			setShape(x0, h10, z00, x1, h11, z01);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x0, h10, z10, x1, h11, z11);
			tesselateBlockInWorld(tt, x, y, z);
		}
		else if (direction == Direction::WEST)
		{
			const float z00 = 0 / 16.0f;
			const float z01 = 2 / 16.0f;
			const float z10 = 14 / 16.0f;
			const float z11 = 16 / 16.0f;

			const float x0 = 1 / 16.0f;
			const float x1 = 3 / 16.0f;
			const float x2 = 7 / 16.0f;

			setShape(x0, h00, z00, x1, h11, z01);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x0, h00, z10, x1, h11, z11);
			tesselateBlockInWorld(tt, x, y, z);

			setShape(x1, h00, z00, x2, h01, z01);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x1, h00, z10, x2, h01, z11);
			tesselateBlockInWorld(tt, x, y, z);

			setShape(x1, h10, z00, x2, h11, z01);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x1, h10, z10, x2, h11, z11);
			tesselateBlockInWorld(tt, x, y, z);
		}
		else if (direction == Direction::SOUTH)
		{

			const float x00 = 0 / 16.0f;
			const float x01 = 2 / 16.0f;
			const float x10 = 14 / 16.0f;
			const float x11 = 16 / 16.0f;

			const float z0 = 9 / 16.0f;
			const float z1 = 13 / 16.0f;
			const float z2 = 15 / 16.0f;

			setShape(x00, h00, z1, x01, h11, z2);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x10, h00, z1, x11, h11, z2);
			tesselateBlockInWorld(tt, x, y, z);

			setShape(x00, h00, z0, x01, h01, z1);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x10, h00, z0, x11, h01, z1);
			tesselateBlockInWorld(tt, x, y, z);

		setShape(x00, h10, z0, x01, h11, z1);
		tesselateBlockInWorld(tt, x, y, z);
		setShape(x10, h10, z0, x11, h11, z1);
		tesselateBlockInWorld(tt, x, y, z);
		}
		else if (direction == Direction::NORTH)
		{
			const float x00 = 0 / 16.0f;
			const float x01 = 2 / 16.0f;
			const float x10 = 14 / 16.0f;
			const float x11 = 16 / 16.0f;

			const float z0 = 1 / 16.0f;
			const float z1 = 3 / 16.0f;
			const float z2 = 7 / 16.0f;

			setShape(x00, h00, z0, x01, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x10, h00, z0, x11, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);

			setShape(x00, h00, z1, x01, h01, z2);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x10, h00, z1, x11, h01, z2);
			tesselateBlockInWorld(tt, x, y, z);

			setShape(x00, h10, z1, x01, h11, z2);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x10, h10, z1, x11, h11, z2);
			tesselateBlockInWorld(tt, x, y, z);
		}
	}
	else
	{
		if (direction == Direction::EAST || direction == Direction::WEST)
		{
			upFlip = FLIP_CW;
			float x0 = 7 / 16.0f;
			float x1 = 9 / 16.0f;
			float z0 = 6 / 16.0f;
			float z1 = 8 / 16.0f;
			setShape(x0, h00, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			z0 = 8 / 16.0f;
			z1 = 10 / 16.0f;
			setShape(x0, h00, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			z0 = 10 / 16.0f;
			z1 = 14 / 16.0f;
			setShape(x0, h00, z0, x1, h01, z1);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x0, h10, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			z0 = 2 / 16.0f;
			z1 = 6 / 16.0f;
			setShape(x0, h00, z0, x1, h01, z1);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x0, h10, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
		}
		else
		{
			float x0 = 6 / 16.0f;
			float x1 = 8 / 16.0f;
			float z0 = 7 / 16.0f;
			float z1 = 9 / 16.0f;
			setShape(x0, h00, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			x0 = 8 / 16.0f;
			x1 = 10 / 16.0f;
			setShape(x0, h00, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			x0 = 10 / 16.0f;
			x1 = 14 / 16.0f;
			setShape(x0, h00, z0, x1, h01, z1);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x0, h10, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			x0 = 2 / 16.0f;
			x1 = 6 / 16.0f;
			setShape(x0, h00, z0, x1, h01, z1);
			tesselateBlockInWorld(tt, x, y, z);
			setShape(x0, h10, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);

		}
	}
	noCulling = false;
        upFlip = FLIP_NONE;

    setShape(0, 0, 0, 1, 1, 1);

//	LeaveCriticalSection( &Tile_SPU::m_csShape );
#endif // #ifdef DISABLE_TESS_FUNCS

    return changed;
}

bool TileRenderer_SPU::tesselateStairsInWorld( StairTile_SPU* tt, int x, int y, int z )
{
#ifdef DISABLE_TESS_FUNCS
	//	EnterCriticalSection( &Tile_SPU::m_csShape );
	tt->setBaseShape(level, x, y, z);
	setShape(tt);
	tesselateBlockInWorld(tt, x, y, z);

	bool checkInnerPiece = tt->setStepShape(level, x, y, z);
	setShape(tt);
	tesselateBlockInWorld(tt, x, y, z);

	if (checkInnerPiece)
	{
		if (tt->setInnerPieceShape(level, x, y, z))
		{
			setShape(tt);
			tesselateBlockInWorld(tt, x, y, z);
		}
	}
//	LeaveCriticalSection( &Tile_SPU::m_csShape );
#endif // #ifdef DISABLE_TESS_FUNCS
	return true;
}

bool TileRenderer_SPU::tesselateDoorInWorld( Tile_SPU* tt, int x, int y, int z )
{
	Tesselator_SPU* t = getTesselator();

	// skip rendering if the other half of the door is missing,
	// to avoid rendering doors that are about to be removed
	int data = level->getData(x, y, z);
	if ((data & DoorTile_SPU::UPPER_BIT) != 0)
	{
		if (level->getTile(x, y - 1, z) != tt->id)
		{
			return false;
		}
	}
	else {
		if (level->getTile(x, y + 1, z) != tt->id)
		{
			return false;
		}
	}

	bool		changed = false;
	float		c10 = 0.5f;
	float		c11 = 1;
	float		c2 = 0.8f;
	float		c3 = 0.6f;

	int			centerColor = 0;
	float		centerBrightness = 0.0f;

	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		centerColor = tt->getLightColor( level, x, y, z );
	}
	else
	{
		centerBrightness = tt->getBrightness( level, x, y, z );
	}


	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tileShapeY0 > 0 ? centerColor : tt->getLightColor( level, x, y - 1, z ) );
		t->color( c10, c10, c10 );
	}
	else
	{
		float br = tt->getBrightness( level, x, y - 1, z );
		if ( tileShapeY0 > 0 ) br = centerBrightness;
		if (level->m_tileData.lightEmission[tt->id] > 0 ) br = 1.0f;
		t->color( c10 * br, c10 * br, c10 * br );
	}
	renderFaceDown( tt, x, y, z, getTexture(tt, level, x, y, z, 0 ) );
	changed = true;

	if ( SharedConstants::TEXTURE_LIGHTING )
	{
		t->tex2( tileShapeY1 < 1 ? centerColor : tt->getLightColor( level, x, y + 1, z ) );
		t->color( c11, c11, c11 );
	}
	else
	{
		float br = tt->getBrightness( level, x, y + 1, z );
		if ( tileShapeY1 < 1 ) br = centerBrightness;
		if ( level->m_tileData.lightEmission[tt->id] > 0 ) br = 1.0f;
		t->color( c11 * br, c11 * br, c11 * br );
	}
	renderFaceUp( tt, x, y, z, getTexture(tt,  level, x, y, z, 1 ) );
	changed = true;

	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeZ0 > 0 ? centerColor : tt->getLightColor( level, x, y, z - 1 ) );
			t->color( c2, c2, c2 );
		}
		else
		{
			float br = tt->getBrightness( level, x, y, z - 1 );
			if ( tileShapeZ0 > 0 ) br = centerBrightness;
			if ( level->m_tileData.lightEmission[tt->id] > 0 ) br = 1.0f;
			t->color( c2 * br, c2 * br, c2 * br );
		}
		Icon_SPU *tex = getTexture(tt, level, x, y, z, 2 );
		renderNorth( tt, x, y, z, tex );
		changed = true;
		xFlipTexture = false;
	}
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeZ1 < 1 ? centerColor : tt->getLightColor( level, x, y, z + 1 ) );
			t->color( c2, c2, c2 );
		}
		else
		{
			float br = tt->getBrightness( level, x, y, z + 1 );
			if ( tileShapeZ1 < 1 ) br = centerBrightness;
			if ( level->m_tileData.lightEmission[tt->id] > 0 ) br = 1.0f;
			t->color( c2 * br, c2 * br, c2 * br );
		}
		Icon_SPU *tex = getTexture( tt, level, x, y, z, 3 );
		renderSouth( tt, x, y, z, tex );
		changed = true;
		xFlipTexture = false;
	}
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeX0 > 0 ? centerColor : tt->getLightColor( level, x - 1, y, z ) );
			t->color( c3, c3, c3 );
		}
		else
		{
			float br = tt->getBrightness( level, x - 1, y, z );
			if ( tileShapeX0 > 0 ) br = centerBrightness;
			if ( level->m_tileData.lightEmission[tt->id] > 0 ) br = 1.0f;
			t->color( c3 * br, c3 * br, c3 * br );
		}
		Icon_SPU *tex = getTexture(tt, level, x, y, z, 4 );
		renderWest( tt, x, y, z, tex );
		changed = true;
		xFlipTexture = false;
	}
	{
		if ( SharedConstants::TEXTURE_LIGHTING )
		{
			t->tex2( tileShapeX1 < 1 ? centerColor : tt->getLightColor( level, x + 1, y, z ) );
			t->color( c3, c3, c3 );
		}
		else
		{
			float br = tt->getBrightness( level, x + 1, y, z );
			if ( tileShapeX1 < 1 ) br = centerBrightness;
			if ( level->m_tileData.lightEmission[tt->id] > 0 ) br = 1.0f;
			t->color( c3 * br, c3 * br, c3 * br );
		}
		Icon_SPU *tex = getTexture(tt, level, x, y, z, 5 );
		renderEast( tt, x, y, z, tex );
		changed = true;
		xFlipTexture = false;
	}
	return changed;
}

void TileRenderer_SPU::renderFaceDown( Tile_SPU* tt, float x, float y, float z, Icon_SPU *tex )
{
	Tesselator_SPU* t = getTesselator();

	if (hasFixedTexture()) tex = fixedTexture;
	float u00 = tex->getU(tileShapeX0 * 16.0f);
	float u11 = tex->getU(tileShapeX1 * 16.0f);
	float v00 = tex->getV(tileShapeZ0 * 16.0f);
	float v11 = tex->getV(tileShapeZ1 * 16.0f);

	if ( tileShapeX0 < 0 || tileShapeX1 > 1 )
	{
		u00 = tex->getU0();
		u11 = tex->getU1();
	}
	if ( tileShapeZ0 < 0 || tileShapeZ1 > 1 )
	{
		v00 = tex->getV0();
		v11 = tex->getV1();
	}

	float		u01 = u11, u10 = u00, v01 = v00, v10 = v11;
	if ( downFlip == FLIP_CCW )
	{
		u00 = tex->getU(tileShapeZ0 * 16.0f);
		v00 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeX1 * 16.0f);
		u11 = tex->getU(tileShapeZ1 * 16.0f);
		v11 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeX0 * 16.0f);

		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u01 = u00;
		u10 = u11;
		v00 = v11;
		v11 = v01;
	}
	else if ( downFlip == FLIP_CW )
	{
		// reshape
		u00 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeZ1 * 16.0f);
		v00 = tex->getV(tileShapeX0 * 16.0f);
		u11 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeZ0 * 16.0f);
		v11 = tex->getV(tileShapeX1 * 16.0f);

		// rotate
		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u00 = u01;
		u11 = u10;
		v01 = v11;
		v10 = v00;
	}
	else if ( downFlip == FLIP_180 )
	{
		u00 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeX0 * 16.0f);
		u11 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeX1 * 16.0f);
		v00 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeZ0 * 16.0f);
		v11 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeZ1 * 16.0f);

		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
	}

	float x0 = x + tileShapeX0;
	float x1 = x + tileShapeX1;
	float y0 = y + tileShapeY0;
	float z0 = z + tileShapeZ0;
	float z1 = z + tileShapeZ1;

	if ( applyAmbienceOcclusion )
	{
		t->color( c1r, c1g, c1b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc1 );
		t->vertexUV( ( float )( x0 ), ( float )( y0 ), ( float )( z1 ), ( float )( u10 ), ( float )( v10 ) );
		t->color( c2r, c2g, c2b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc2 );
		t->vertexUV( ( float )( x0 ), ( float )( y0 ), ( float )( z0 ), ( float )( u00 ), ( float )( v00 ) );
		t->color( c3r, c3g, c3b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc3 );
		t->vertexUV( ( float )( x1 ), ( float )( y0 ), ( float )( z0 ), ( float )( u01 ), ( float )( v01 ) );
		t->color( c4r, c4g, c4b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc4 );
		t->vertexUV( ( float )( x1 ), ( float )( y0 ), ( float )( z1 ), ( float )( u11 ), ( float )( v11 ) );
	}
	else
	{
		t->vertexUV( ( float )( x0 ), ( float )( y0 ), ( float )( z1 ), ( float )( u10 ), ( float )( v10 ) );
		t->vertexUV( ( float )( x0 ), ( float )( y0 ), ( float )( z0 ), ( float )( u00 ), ( float )( v00 ) );
		t->vertexUV( ( float )( x1 ), ( float )( y0 ), ( float )( z0 ), ( float )( u01 ), ( float )( v01 ) );
		t->vertexUV( ( float )( x1 ), ( float )( y0 ), ( float )( z1 ), ( float )( u11 ), ( float )( v11 ) );
	}
}

void TileRenderer_SPU::renderFaceUp( Tile_SPU* tt, float x, float y, float z, Icon_SPU *tex )
{
	Tesselator_SPU* t = getTesselator();

	if (hasFixedTexture()) tex = fixedTexture;
	float u00 = tex->getU(tileShapeX0 * 16.0f);
	float u11 = tex->getU(tileShapeX1 * 16.0f);
	float v00 = tex->getV(tileShapeZ0 * 16.0f);
	float v11 = tex->getV(tileShapeZ1 * 16.0f);

	if ( tileShapeX0 < 0 || tileShapeX1 > 1 )
	{
		u00 = tex->getU0();
		u11 = tex->getU1();
	}
	if ( tileShapeZ0 < 0 || tileShapeZ1 > 1 )
	{
		v00 = tex->getV0();
		v11 = tex->getV1();
	}

	float u01 = u11, u10 = u00, v01 = v00, v10 = v11;

	if ( upFlip == FLIP_CW )
	{
		u00 = tex->getU(tileShapeZ0 * 16.0f);
		v00 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeX1 * 16.0f);
		u11 = tex->getU(tileShapeZ1 * 16.0f);
		v11 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeX0 * 16.0f);

		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u01 = u00;
		u10 = u11;
		v00 = v11;
		v11 = v01;
	}
	else if ( upFlip == FLIP_CCW )
	{
		// reshape
		u00 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeZ1 * 16.0f);
		v00 = tex->getV(tileShapeX0 * 16.0f);
		u11 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeZ0 * 16.0f);
		v11 = tex->getV(tileShapeX1 * 16.0f);

		// rotate
		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u00 = u01;
		u11 = u10;
		v01 = v11;
		v10 = v00;
	}
	else if ( upFlip == FLIP_180 )
	{
		u00 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeX0 * 16.0f);
		u11 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeX1 * 16.0f);
		v00 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeZ0 * 16.0f);
		v11 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeZ1 * 16.0f);

		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
	}


	float x0 = x + tileShapeX0;
	float x1 = x + tileShapeX1;
	float y1 = y + tileShapeY1;
	float z0 = z + tileShapeZ0;
	float z1 = z + tileShapeZ1;

	if ( applyAmbienceOcclusion )
	{
		t->color( c1r, c1g, c1b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc1 );
		t->vertexUV( ( float )( x1 ), ( float )( y1 ), ( float )( z1 ), ( float )( u11 ), ( float )( v11 ) );
		t->color( c2r, c2g, c2b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc2 );
		t->vertexUV( ( float )( x1 ), ( float )( y1 ), ( float )( z0 ), ( float )( u01 ), ( float )( v01 ) );
		t->color( c3r, c3g, c3b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc3 );
		t->vertexUV( ( float )( x0 ), ( float )( y1 ), ( float )( z0 ), ( float )( u00 ), ( float )( v00 ) );
		t->color( c4r, c4g, c4b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc4 );
		t->vertexUV( ( float )( x0 ), ( float )( y1 ), ( float )( z1 ), ( float )( u10 ), ( float )( v10 ) );
	}
	else
	{
		t->vertexUV( ( float )( x1 ), ( float )( y1 ), ( float )( z1 ), ( float )( u11 ), ( float )( v11 ) );
		t->vertexUV( ( float )( x1 ), ( float )( y1 ), ( float )( z0 ), ( float )( u01 ), ( float )( v01 ) );
		t->vertexUV( ( float )( x0 ), ( float )( y1 ), ( float )( z0 ), ( float )( u00 ), ( float )( v00 ) );
		t->vertexUV( ( float )( x0 ), ( float )( y1 ), ( float )( z1 ), ( float )( u10 ), ( float )( v10 ) );
	}

}

void TileRenderer_SPU::renderNorth( Tile_SPU* tt, float x, float y, float z, Icon_SPU *tex )
{
	Tesselator_SPU* t = getTesselator();

	if (hasFixedTexture()) tex = fixedTexture;
	float u00 = tex->getU(tileShapeX0 * 16.0f);
	float u11 = tex->getU(tileShapeX1 * 16.0f);
	float v00 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeY1 * 16.0f);
	float v11 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeY0 * 16.0f);
	if ( xFlipTexture )
	{
		float tmp = u00;
		u00 = u11;
		u11 = tmp;
	}

	if ( tileShapeX0 < 0 || tileShapeX1 > 1 )
	{
		u00 = tex->getU0();
		u11 = tex->getU1();
	}
	if ( tileShapeY0 < 0 || tileShapeY1 > 1 )
	{
		v00 = tex->getV0();
		v11 = tex->getV1();
	}

	float		u01 = u11, u10 = u00, v01 = v00, v10 = v11;

	if ( northFlip == FLIP_CCW )
	{
		u00 = tex->getU(tileShapeY0 * 16.0f);
		v00 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeX0 * 16.0f);
		u11 = tex->getU(tileShapeY1 * 16.0f);
		v11 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeX1 * 16.0f);

		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u01 = u00;
		u10 = u11;
		v00 = v11;
		v11 = v01;
	}
	else if ( northFlip == FLIP_CW )
	{
		// reshape
		u00 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeY1 * 16.0f);
		v00 = tex->getV(tileShapeX1 * 16.0f);
		u11 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeY0 * 16.0f);
		v11 = tex->getV(tileShapeX0 * 16.0f);

		// rotate
		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u00 = u01;
		u11 = u10;
		v01 = v11;
		v10 = v00;
	}
	else if ( northFlip == FLIP_180 )
	{
		u00 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeX0 * 16.0f);
		u11 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeX1 * 16.0f);
		v00 = tex->getV(tileShapeY1 * 16.0f);
		v11 = tex->getV(tileShapeY0 * 16.0f);

		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
	}


	float x0 = x + tileShapeX0;
	float x1 = x + tileShapeX1;
	float y0 = y + tileShapeY0;
	float y1 = y + tileShapeY1;
	float z0 = z + tileShapeZ0;

	if ( applyAmbienceOcclusion )
	{
		t->color( c1r, c1g, c1b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc1 );
		t->vertexUV( ( float )( x0 ), ( float )( y1 ), ( float )( z0 ), ( float )( u01 ), ( float )( v01 ) );
		t->color( c2r, c2g, c2b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc2 );
		t->vertexUV( ( float )( x1 ), ( float )( y1 ), ( float )( z0 ), ( float )( u00 ), ( float )( v00 ) );
		t->color( c3r, c3g, c3b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc3 );
		t->vertexUV( ( float )( x1 ), ( float )( y0 ), ( float )( z0 ), ( float )( u10 ), ( float )( v10 ) );
		t->color( c4r, c4g, c4b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc4 );
		t->vertexUV( ( float )( x0 ), ( float )( y0 ), ( float )( z0 ), ( float )( u11 ), ( float )( v11 ) );
	}
	else
	{
		t->vertexUV( ( float )( x0 ), ( float )( y1 ), ( float )( z0 ), ( float )( u01 ), ( float )( v01 ) );
		t->vertexUV( ( float )( x1 ), ( float )( y1 ), ( float )( z0 ), ( float )( u00 ), ( float )( v00 ) );
		t->vertexUV( ( float )( x1 ), ( float )( y0 ), ( float )( z0 ), ( float )( u10 ), ( float )( v10 ) );
		t->vertexUV( ( float )( x0 ), ( float )( y0 ), ( float )( z0 ), ( float )( u11 ), ( float )( v11 ) );
	}

}

void TileRenderer_SPU::renderSouth( Tile_SPU* tt, float x, float y, float z, Icon_SPU *tex )
{
	Tesselator_SPU* t = getTesselator();

	if (hasFixedTexture()) tex = fixedTexture;
	float u00 = tex->getU(tileShapeX0 * 16.0f);
	float u11 = tex->getU(tileShapeX1 * 16.0f);
	float v00 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeY1 * 16.0f);
	float v11 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeY0 * 16.0f);
	if ( xFlipTexture )
	{
		float tmp = u00;
		u00 = u11;
		u11 = tmp;
	}

	if ( tileShapeX0 < 0 || tileShapeX1 > 1 )
	{
		u00 = tex->getU0();
		u11 = tex->getU1();
	}
	if ( tileShapeY0 < 0 || tileShapeY1 > 1 )
	{
		v00 = tex->getV0();
		v11 = tex->getV1();
	}

	float		u01 = u11, u10 = u00, v01 = v00, v10 = v11;

	if ( southFlip == FLIP_CW )
	{
		u00 = tex->getU(tileShapeY0 * 16.0f);
		v11 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeX0 * 16.0f);
		u11 = tex->getU(tileShapeY1 * 16.0f);
		v00 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeX1 * 16.0f);

		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u01 = u00;
		u10 = u11;
		v00 = v11;
		v11 = v01;
	}
	else if ( southFlip == FLIP_CCW )
	{
		// reshape
		u00 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeY1 * 16.0f);
		v00 = tex->getV(tileShapeX0 * 16.0f);
		u11 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeY0 * 16.0f);
		v11 = tex->getV(tileShapeX1 * 16.0f);

		// rotate
		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u00 = u01;
		u11 = u10;
		v01 = v11;
		v10 = v00;
	}
	else if ( southFlip == FLIP_180 )
	{
		u00 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeX0 * 16.0f);
		u11 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeX1 * 16.0f);
		v00 = tex->getV(tileShapeY1 * 16.0f);
		v11 = tex->getV(tileShapeY0 * 16.0f);

		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
	}


	float x0 = x + tileShapeX0;
	float x1 = x + tileShapeX1;
	float y0 = y + tileShapeY0;
	float y1 = y + tileShapeY1;
	float z1 = z + tileShapeZ1;

	if ( applyAmbienceOcclusion )
	{
		t->color( c1r, c1g, c1b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc1 );
		t->vertexUV( ( float )( x0 ), ( float )( y1 ), ( float )( z1 ), ( float )( u00 ), ( float )( v00 ) );
		t->color( c2r, c2g, c2b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc2 );
		t->vertexUV( ( float )( x0 ), ( float )( y0 ), ( float )( z1 ), ( float )( u10 ), ( float )( v10 ) );
		t->color( c3r, c3g, c3b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc3 );
		t->vertexUV( ( float )( x1 ), ( float )( y0 ), ( float )( z1 ), ( float )( u11 ), ( float )( v11 ) );
		t->color( c4r, c4g, c4b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc4 );
		t->vertexUV( ( float )( x1 ), ( float )( y1 ), ( float )( z1 ), ( float )( u01 ), ( float )( v01 ) );
	}
	else
	{
		t->vertexUV( ( float )( x0 ), ( float )( y1 ), ( float )( z1 ), ( float )( u00 ), ( float )( v00 ) );
		t->vertexUV( ( float )( x0 ), ( float )( y0 ), ( float )( z1 ), ( float )( u10 ), ( float )( v10 ) );
		t->vertexUV( ( float )( x1 ), ( float )( y0 ), ( float )( z1 ), ( float )( u11 ), ( float )( v11 ) );
		t->vertexUV( ( float )( x1 ), ( float )( y1 ), ( float )( z1 ), ( float )( u01 ), ( float )( v01 ) );
	}

}

void TileRenderer_SPU::renderWest( Tile_SPU* tt, float x, float y, float z, Icon_SPU *tex )
{
	Tesselator_SPU* t = getTesselator();

	if (hasFixedTexture()) tex = fixedTexture;
	float u00 = tex->getU(tileShapeZ0 * 16.0f);
	float u11 = tex->getU(tileShapeZ1 * 16.0f);
	float v00 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeY1 * 16.0f);
	float v11 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeY0 * 16.0f);
	if ( xFlipTexture )
	{
		float tmp = u00;
		u00 = u11;
		u11 = tmp;
	}

	if ( tileShapeZ0 < 0 || tileShapeZ1 > 1 )
	{
		u00 = tex->getU0();
		u11 = tex->getU1();
	}
	if ( tileShapeY0 < 0 || tileShapeY1 > 1 )
	{
		v00 = tex->getV0();
		v11 = tex->getV1();
	}

	float		u01 = u11, u10 = u00, v01 = v00, v10 = v11;

	if ( westFlip == FLIP_CW )
	{
		u00 = tex->getU(tileShapeY0 * 16.0f);
		v00 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeZ1 * 16.0f);
		u11 = tex->getU(tileShapeY1 * 16.0f);
		v11 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeZ0 * 16.0f);

		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u01 = u00;
		u10 = u11;
		v00 = v11;
		v11 = v01;
	}
	else if ( westFlip == FLIP_CCW )
	{
		// reshape
		u00 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeY1 * 16.0f);
		v00 = tex->getV(tileShapeZ0 * 16.0f);
		u11 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeY0 * 16.0f);
		v11 = tex->getV(tileShapeZ1 * 16.0f);

		// rotate
		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u00 = u01;
		u11 = u10;
		v01 = v11;
		v10 = v00;
	}
	else if ( westFlip == FLIP_180 )
	{
		u00 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeZ0 * 16.0f);
		u11 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeZ1 * 16.0f);
		v00 = tex->getV(tileShapeY1 * 16.0f);
		v11 = tex->getV(tileShapeY0 * 16.0f);


		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
	}

	float x0 = x + tileShapeX0;
	float y0 = y + tileShapeY0;
	float y1 = y + tileShapeY1;
	float z0 = z + tileShapeZ0;
	float z1 = z + tileShapeZ1;

	if ( applyAmbienceOcclusion )
	{
		t->color( c1r, c1g, c1b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc1 );
		t->vertexUV( ( float )( x0 ), ( float )( y1 ), ( float )( z1 ), ( float )( u01 ), ( float )( v01 ) );
		t->color( c2r, c2g, c2b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc2 );
		t->vertexUV( ( float )( x0 ), ( float )( y1 ), ( float )( z0 ), ( float )( u00 ), ( float )( v00 ) );
		t->color( c3r, c3g, c3b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc3 );
		t->vertexUV( ( float )( x0 ), ( float )( y0 ), ( float )( z0 ), ( float )( u10 ), ( float )( v10 ) );
		t->color( c4r, c4g, c4b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc4 );
		t->vertexUV( ( float )( x0 ), ( float )( y0 ), ( float )( z1 ), ( float )( u11 ), ( float )( v11 ) );
	}
	else
	{
		t->vertexUV( ( float )( x0 ), ( float )( y1 ), ( float )( z1 ), ( float )( u01 ), ( float )( v01 ) );
		t->vertexUV( ( float )( x0 ), ( float )( y1 ), ( float )( z0 ), ( float )( u00 ), ( float )( v00 ) );
		t->vertexUV( ( float )( x0 ), ( float )( y0 ), ( float )( z0 ), ( float )( u10 ), ( float )( v10 ) );
		t->vertexUV( ( float )( x0 ), ( float )( y0 ), ( float )( z1 ), ( float )( u11 ), ( float )( v11 ) );
	}

}

void TileRenderer_SPU::renderEast( Tile_SPU* tt, float x, float y, float z, Icon_SPU *tex )
{
	Tesselator_SPU* t = getTesselator();

	if (hasFixedTexture()) tex = fixedTexture;
	float u00 = tex->getU(tileShapeZ0 * 16.0f);
	float u11 = tex->getU(tileShapeZ1 * 16.0f);
	float v00 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeY1 * 16.0f);
	float v11 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeY0 * 16.0f);
	if ( xFlipTexture )
	{
		float tmp = u00;
		u00 = u11;
		u11 = tmp;
	}

	if ( tileShapeZ0 < 0 || tileShapeZ1 > 1 )
	{
		u00 = tex->getU0();
		u11 = tex->getU1();
	}
	if ( tileShapeY0 < 0 || tileShapeY1 > 1 )
	{
		v00 = tex->getV0();
		v11 = tex->getV1();
	}

	float		u01 = u11, u10 = u00, v01 = v00, v10 = v11;

	if ( eastFlip == FLIP_CCW )
	{
		u00 = tex->getU(tileShapeY0 * 16.0f);
		v00 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeZ0 * 16.0f);
		u11 = tex->getU(tileShapeY1 * 16.0f);
		v11 = tex->getV(SharedConstants::WORLD_RESOLUTION - tileShapeZ1 * 16.0f);

		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u01 = u00;
		u10 = u11;
		v00 = v11;
		v11 = v01;
	}
	else if ( eastFlip == FLIP_CW )
	{
		// reshape
		u00 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeY1 * 16.0f);
		v00 = tex->getV(tileShapeZ1 * 16.0f);
		u11 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeY0 * 16.0f);
		v11 = tex->getV(tileShapeZ0 * 16.0f);

		// rotate
		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u00 = u01;
		u11 = u10;
		v01 = v11;
		v10 = v00;
	}
	else if ( eastFlip == FLIP_180 )
	{
		u00 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeZ0 * 16.0f);
		u11 = tex->getU(SharedConstants::WORLD_RESOLUTION - tileShapeZ1 * 16.0f);
		v00 = tex->getV(tileShapeY1 * 16.0f);
		v11 = tex->getV(tileShapeY0 * 16.0f);

		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
	}

	float x1 = x + tileShapeX1;
	float y0 = y + tileShapeY0;
	float y1 = y + tileShapeY1;
	float z0 = z + tileShapeZ0;
	float z1 = z + tileShapeZ1;

	if ( applyAmbienceOcclusion )
	{
		t->color( c1r, c1g, c1b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc1 );
		t->vertexUV( ( float )( x1 ), ( float )( y0 ), ( float )( z1 ), ( float )( u10 ), ( float )( v10 ) );
		t->color( c2r, c2g, c2b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc2 );
		t->vertexUV( ( float )( x1 ), ( float )( y0 ), ( float )( z0 ), ( float )( u11 ), ( float )( v11 ) );
		t->color( c3r, c3g, c3b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc3 );
		t->vertexUV( ( float )( x1 ), ( float )( y1 ), ( float )( z0 ), ( float )( u01 ), ( float )( v01 ) );
		t->color( c4r, c4g, c4b );
		if ( SharedConstants::TEXTURE_LIGHTING ) t->tex2( tc4 );
		t->vertexUV( ( float )( x1 ), ( float )( y1 ), ( float )( z1 ), ( float )( u00 ), ( float )( v00 ) );
	}
	else
	{
		t->vertexUV( ( float )( x1 ), ( float )( y0 ), ( float )( z1 ), ( float )( u10 ), ( float )( v10 ) );
		t->vertexUV( ( float )( x1 ), ( float )( y0 ), ( float )( z0 ), ( float )( u11 ), ( float )( v11 ) );
		t->vertexUV( ( float )( x1 ), ( float )( y1 ), ( float )( z0 ), ( float )( u01 ), ( float )( v01 ) );
		t->vertexUV( ( float )( x1 ), ( float )( y1 ), ( float )( z1 ), ( float )( u00 ), ( float )( v00 ) );
	}

}

bool TileRenderer_SPU::canRender( int renderShape )
{
	if ( renderShape == Tile_SPU::SHAPE_BLOCK ) return true;
	if ( renderShape == Tile_SPU::SHAPE_TREE ) return true;
	if ( renderShape == Tile_SPU::SHAPE_QUARTZ) return true;
	if ( renderShape == Tile_SPU::SHAPE_CACTUS ) return true;
	if ( renderShape == Tile_SPU::SHAPE_STAIRS ) return true;
	if ( renderShape == Tile_SPU::SHAPE_FENCE ) return true;
	if ( renderShape == Tile_SPU::SHAPE_EGG) return true;
    if ( renderShape == Tile_SPU::SHAPE_ENTITYTILE_ANIMATED) return true;
    if ( renderShape == Tile_SPU::SHAPE_FENCE_GATE) return true;
	if ( renderShape == Tile_SPU::SHAPE_PISTON_BASE ) return true;
	if ( renderShape == Tile_SPU::SHAPE_PORTAL_FRAME ) return true;
	if ( renderShape == Tile_SPU::SHAPE_WALL) return true;
	if ( renderShape == Tile_SPU::SHAPE_ANVIL) return true;
	return false;
}

Icon_SPU *TileRenderer_SPU::getTexture(Tile_SPU *tile, ChunkRebuildData *level, int x, int y, int z, int face)
{
	return getTextureOrMissing(tile->getTexture(level, x, y, z, face));
}

Icon_SPU *TileRenderer_SPU::getTexture(Tile_SPU *tile, int face, int data)
{
	return getTextureOrMissing(tile->getTexture(face, data));
}

Icon_SPU *TileRenderer_SPU::getTexture(Tile_SPU *tile, int face)
{
	return getTextureOrMissing(tile->getTexture(face));
}

Icon_SPU *TileRenderer_SPU::getTexture(Tile_SPU *tile)
{
	return getTextureOrMissing(tile->getTexture(Facing::UP));
}

Icon_SPU *TileRenderer_SPU::getTextureOrMissing(Icon_SPU *Icon_SPU)
{
	if (Icon_SPU == nullptr) 
	{
		assert(0);
	//	return minecraft->textures->getMissingIcon_SPU(Icon_SPU::TYPE_TERRAIN);
	}
	return Icon_SPU;
}