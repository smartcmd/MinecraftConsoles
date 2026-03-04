#include "stdafx.h"
#ifdef __PS3__
#ifndef SN_TARGET_PS3_SPU
// 	#include "..\..\..\stdafx.h"
#endif
#endif 

#include "ChunkRebuildData.h"
#include "Tesselator_SPU.h"


#ifndef SN_TARGET_PS3_SPU
#include "..\..\..\..\Minecraft.World\Tile.h"
#include "..\..\..\..\Minecraft.World\Level.h"
#include "..\..\..\..\Minecraft.World\Dimension.h"
// 
// #include "..\..\..\Chunk.h"
// #include "..\..\..\TileRenderer.h"
// #include "..\..\..\TileEntityRenderDispatcher.h"
// #include "..\..\..\LevelRenderer.h"
#include "..\..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "..\..\..\..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\..\..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\..\..\..\Minecraft.World\Icon.h"
#include "..\..\..\..\Minecraft.World\BiomeSource.h"

#else
 
#include "..\Common\spu_assert.h"

#endif //SN_TARGET_PS3_SPU

static const int Level_maxBuildHeight = 256;
static const int Level_MAX_LEVEL_SIZE = 30000000; 
static const int Level_MAX_BRIGHTNESS = 15;


#include "TileRenderer_SPU.h"
#include "Tile_SPU.h"
#include "ChunkRebuildData.h"
TileData_SPU g_tileSPUData;

static const int MAX_LEVEL_SIZE = 30000000;
static const int MAX_BRIGHTNESS = 15;

#if 0 //def SN_TARGET_PS3_SPU
int ChunkRebuildData::getTile( int x, int y, int z )			{	return m_tileIds[getTileIdx(x,y,z)]; }
int ChunkRebuildData::getBrightnessSky(int x, int y, int z)		{	return m_brightness[getTileIdx(x,y,z)] & 0x0f; }
int ChunkRebuildData::getBrightnessBlock(int x, int y, int z)	{	return m_brightness[getTileIdx(x,y,z)] >> 4; }
int ChunkRebuildData::getData(int x, int y, int z)				{	return m_data_flags[getTileIdx(x,y,z)] & 0x0f; }
int ChunkRebuildData::getGrassColor( int x, int z )				{	return m_grassColor[getTileIdx(x,z)]; }
int ChunkRebuildData::getFoliageColor( int x, int z )			{	return m_foliageColor[getTileIdx(x,z)]; }
int ChunkRebuildData::getFlags(int x, int y, int z)				{	return m_data_flags[getTileIdx(x,y,z)] >> 4; }
void ChunkRebuildData::setFlag(int x, int y, int z, int flag)	{	m_data_flags[getTileIdx(x,y,z)] |= (flag<<4);}
#endif 



void ChunkRebuildData::disableUnseenTiles()
{
	// We now go through the vertical section of this level chunk that we are interested in and try and establish
	// (1) if it is completely empty
	// (2) if any of the tiles can be quickly determined to not need rendering because they are in the middle of other tiles and
	//     so can't be seen. A large amount (> 60% in tests) of tiles that call tesselateInWorld in the unoptimised version
	//     of this function fall into this category. By far the largest category of these are tiles in solid regions of rock.
	int startX = m_x0+2;	// beginning of the chunk
	int startY = m_y0+2;	// beginning of the chunk
	int startZ = m_z0+2;	// beginning of the chunk

	for(int iX=startX; iX<(startX+16); iX++)
	{
		for(int iY=startY; iY<(startY+16); iY++)
		{
			for(int iZ=startZ; iZ<(startZ+16); iZ++)
			{
				int tileID = getTile(iX,iY,iZ);
				if( tileID == 0 )  continue;
				m_flags &= ~e_flag_EmptyChunk;
				

				// Don't bother trying to work out neighbours for this tile if we are at the edge of the chunk - apart from the very
				// bottom of the world where we shouldn't ever be able to see
				if( iY == 127 ) continue;
				if(( iX-startX == 0 ) || ( iX-startX == 15 )) continue;
				if(( iZ-startZ == 0 ) || ( iZ-startZ == 15 )) continue;

				int flags = getFlags(iX, iY, iZ);

				// Establish whether this tile and its neighbours are all made of rock, dirt, unbreakable tiles, or have already
				// been determined to meet this criteria themselves and have a tile of 255 set.
				if( !( ( tileID == Tile_SPU::rock_Id ) || ( tileID == Tile_SPU::dirt_Id ) || ( tileID == Tile_SPU::unbreakable_Id ) || ( flags & e_flag_NoRender) ) ) continue;
				tileID = getTile(iX-1, iY, iZ);
				flags = getFlags(iX-1, iY, iZ);
				if( !( ( tileID == Tile_SPU::rock_Id ) || ( tileID == Tile_SPU::dirt_Id ) || ( tileID == Tile_SPU::unbreakable_Id ) || ( flags & e_flag_NoRender) ) ) continue;
				tileID = getTile(iX+1, iY, iZ);
				flags = getFlags(iX+1, iY, iZ);
				if( !( ( tileID == Tile_SPU::rock_Id ) || ( tileID == Tile_SPU::dirt_Id ) || ( tileID == Tile_SPU::unbreakable_Id ) || ( flags & e_flag_NoRender) ) ) continue;
				tileID = getTile(iX, iY, iZ-1);
				flags = getFlags(iX, iY, iZ-1);
				if( !( ( tileID == Tile_SPU::rock_Id ) || ( tileID == Tile_SPU::dirt_Id ) || ( tileID == Tile_SPU::unbreakable_Id ) || ( flags & e_flag_NoRender) ) ) continue;
				tileID = getTile(iX, iY, iZ+1);
				flags = getFlags(iX, iY, iZ+1);
				if( !( ( tileID == Tile_SPU::rock_Id ) || ( tileID == Tile_SPU::dirt_Id ) || ( tileID == Tile_SPU::unbreakable_Id ) || ( flags & e_flag_NoRender) ) ) continue;
				// Treat the bottom of the world differently - we shouldn't ever be able to look up at this, so consider tiles as invisible
				// if they are surrounded on sides other than the bottom
				if( iY > 0 )
				{
					tileID = getTile(iX, iY-1, iZ);
					flags = getFlags(iX, iY-1, iZ);
					if( !( ( tileID == Tile_SPU::rock_Id ) || ( tileID == Tile_SPU::dirt_Id ) || ( tileID == Tile_SPU::unbreakable_Id ) || ( flags & e_flag_NoRender) ) ) continue;
				}
				tileID = getTile(iX, iY+1, iZ);
				flags = getFlags(iX, iY+1, iZ);
				if( !( ( tileID == Tile_SPU::rock_Id ) || ( tileID == Tile_SPU::dirt_Id ) || ( tileID == Tile_SPU::unbreakable_Id ) || ( flags & e_flag_NoRender) ) ) continue;

				// This tile is surrounded. Flag it as not requiring to be rendered by setting its id to 255.
				setFlag(iX, iY, iZ, e_flag_NoRender);
			}
		}
	}
}
 
#ifndef SN_TARGET_PS3_SPU

void setIconSPUFromIcon(Icon_SPU* iconSpu, Icon* icon)
{
	iconSpu->set(icon->getX(), icon->getY(), icon->getWidth(), icon->getHeight(), icon->getSourceWidth(), icon->getSourceHeight());
}

void ChunkRebuildData::buildMaterial(int matSPUIndex, Material* mat)
{
	Material_SPU* matSPU = &m_tileData.materials[matSPUIndex];
	matSPU->id = matSPUIndex;
	matSPU->color = mat->color->col;
	matSPU->flags =0;
	matSPU->flags |= mat->isFlammable() ? Material_SPU::e_flammable : 0;
	matSPU->flags |= mat->isReplaceable() ? Material_SPU::e_replaceable : 0;
	matSPU->flags |= mat->_neverBuildable ? Material_SPU::e_neverBuildable : 0;
	matSPU->flags |= mat->isSolid() ? Material_SPU::e_isSolid : 0;
	matSPU->flags |= mat->isLiquid() ? Material_SPU::e_isLiquid : 0;
	matSPU->flags |= mat->blocksLight() ? Material_SPU::e_blocksLight : 0;
	matSPU->flags |= mat->blocksMotion() ? Material_SPU::e_blocksMotion : 0;
}

void ChunkRebuildData::buildMaterials()
{
	buildMaterial(Material_SPU::air_Id,					Material::air);					
	buildMaterial(Material_SPU::grass_Id,				Material::grass);				
	buildMaterial(Material_SPU::dirt_Id,				Material::dirt);				
	buildMaterial(Material_SPU::wood_Id,				Material::wood);				
	buildMaterial(Material_SPU::stone_Id,				Material::stone);				
	buildMaterial(Material_SPU::metal_Id,				Material::metal);				
	buildMaterial(Material_SPU::water_Id,				Material::water);				
	buildMaterial(Material_SPU::lava_Id,				Material::lava);				
	buildMaterial(Material_SPU::leaves_Id,				Material::leaves);				
	buildMaterial(Material_SPU::plant_Id,				Material::plant);				
	buildMaterial(Material_SPU::replaceable_plant_Id,	Material::replaceable_plant);	
	buildMaterial(Material_SPU::sponge_Id,				Material::sponge);				
	buildMaterial(Material_SPU::cloth_Id,				Material::cloth);				
	buildMaterial(Material_SPU::fire_Id,				Material::fire);				
	buildMaterial(Material_SPU::sand_Id,				Material::sand);				
	buildMaterial(Material_SPU::decoration_Id,			Material::decoration);		
	buildMaterial(Material_SPU::clothDecoration_Id,		Material::clothDecoration);		
	buildMaterial(Material_SPU::glass_Id,				Material::glass);				
	buildMaterial(Material_SPU::explosive_Id,			Material::explosive);			
	buildMaterial(Material_SPU::coral_Id,				Material::coral);				
	buildMaterial(Material_SPU::ice_Id,					Material::ice);					
	buildMaterial(Material_SPU::topSnow_Id,				Material::topSnow);				
	buildMaterial(Material_SPU::snow_Id,				Material::snow);				
	buildMaterial(Material_SPU::cactus_Id,				Material::cactus);				
	buildMaterial(Material_SPU::clay_Id,				Material::clay);				
	buildMaterial(Material_SPU::vegetable_Id,			Material::vegetable);			
	buildMaterial(Material_SPU::egg_Id,					Material::egg);					
	buildMaterial(Material_SPU::portal_Id,				Material::portal);				
	buildMaterial(Material_SPU::cake_Id,				Material::cake);				
	buildMaterial(Material_SPU::web_Id,					Material::web);					
	buildMaterial(Material_SPU::piston_Id,				Material::piston);				
	buildMaterial(Material_SPU::buildable_glass_Id,		Material::buildable_glass);				
	buildMaterial(Material_SPU::heavyMetal_Id,			Material::heavyMetal);				
}

int ChunkRebuildData::getMaterialID(Tile* pTile)
{
	Material* m = pTile->material;
	if(m == Material::air)  			return Material_SPU::air_Id;					
	if(m == Material::grass)  			return Material_SPU::grass_Id;					
	if(m == Material::dirt)  			return Material_SPU::dirt_Id;				
	if(m == Material::wood)  			return Material_SPU::wood_Id;				
	if(m == Material::stone)  			return Material_SPU::stone_Id;					
	if(m == Material::metal)  			return Material_SPU::metal_Id;					
	if(m == Material::water)  			return Material_SPU::water_Id;					
	if(m == Material::lava)  			return Material_SPU::lava_Id;				
	if(m == Material::leaves)  			return Material_SPU::leaves_Id;					
	if(m == Material::plant)  			return Material_SPU::plant_Id;					
	if(m == Material::replaceable_plant)return Material_SPU::replaceable_plant_Id;		
	if(m == Material::sponge)  			return Material_SPU::sponge_Id;					
	if(m == Material::cloth)  			return Material_SPU::cloth_Id;					
	if(m == Material::fire)  			return Material_SPU::fire_Id;				
	if(m == Material::sand)  			return Material_SPU::sand_Id;				
	if(m == Material::decoration)  		return Material_SPU::decoration_Id;			
	if(m == Material::clothDecoration)  return Material_SPU::clothDecoration_Id;			
	if(m == Material::glass)  			return Material_SPU::glass_Id;
	if(m == Material::explosive)  		return Material_SPU::explosive_Id;				
	if(m == Material::coral)  			return Material_SPU::coral_Id;					
	if(m == Material::ice)  			return Material_SPU::ice_Id;					
	if(m == Material::topSnow)  		return Material_SPU::topSnow_Id;				
	if(m == Material::snow)  			return Material_SPU::snow_Id;				
	if(m == Material::cactus)  			return Material_SPU::cactus_Id;					
	if(m == Material::clay)  			return Material_SPU::clay_Id;				
	if(m == Material::vegetable)  		return Material_SPU::vegetable_Id;				
	if(m == Material::egg)  			return Material_SPU::egg_Id;					
	if(m == Material::portal)  			return Material_SPU::portal_Id;					
	if(m == Material::cake)  			return Material_SPU::cake_Id;				
	if(m == Material::web)  			return Material_SPU::web_Id;					
	if(m == Material::piston)  			return Material_SPU::piston_Id;					
	if(m == Material::buildable_glass)	return Material_SPU::buildable_glass_Id;
	if(m == Material::heavyMetal)		return Material_SPU::heavyMetal_Id;
	assert(0);	
	return Material_SPU::air_Id;
}


void ChunkRebuildData::createTileData()
{
	// build the material data
	buildMaterials();

	// build the tile data
	for(int i=0;i<256;i++)
	{
		m_tileData.mipmapEnable.set(i, Tile::mipmapEnable[i]);
		m_tileData.solid.set(i, Tile::solid[i]);
		m_tileData.transculent.set(i, Tile::transculent[i]);
//		m_tileData._sendTileData.set(i, Tile::_sendTileData[i]);
		m_tileData.propagate.set(i, Tile::propagate[i]);
		m_tileData.lightBlock[i] = Tile::lightBlock[i];
		m_tileData.lightEmission[i] = Tile::lightEmission[i];
		if(Tile::tiles[i])
		{
			m_tileData.signalSource.set(i, Tile::tiles[i]->isSignalSource());
			m_tileData.cubeShaped.set(i, Tile::tiles[i]->isCubeShaped());

			m_tileData.xx0[i] = (float)Tile::tiles[i]->getShapeX0();
			m_tileData.yy0[i] = (float)Tile::tiles[i]->getShapeY0();
			m_tileData.zz0[i] = (float)Tile::tiles[i]->getShapeZ0();
			m_tileData.xx1[i] = (float)Tile::tiles[i]->getShapeX1();
			m_tileData.yy1[i] = (float)Tile::tiles[i]->getShapeY1();
			m_tileData.zz1[i] = (float)Tile::tiles[i]->getShapeZ1();
			Icon* pTex = Tile::tiles[i]->icon;
			if(pTex)
			{
				setIconSPUFromIcon(&m_tileData.iconData[i], pTex);
			}
			m_tileData.materialIDs[i] = getMaterialID(Tile::tiles[i]);
		}
	}
	m_tileData.leafTile_allowSame = Tile::leaves->allowSame;
	m_tileData.leafTile_fancyTextureSet = Tile::leaves->fancyTextureSet;


	// Custom tile textures
	// get the width and height of any texture, since everything uses the same texture
	m_tileData.iconTexWidth = Tile::grass->iconTop->getSourceWidth();
	m_tileData.iconTexHeight = Tile::grass->iconTop->getSourceHeight();
	// Grass tile
	setIconSPUFromIcon(&m_tileData.grass_iconTop, Tile::grass->iconTop);
	setIconSPUFromIcon(&m_tileData.grass_iconSnowSide, Tile::grass->iconSnowSide);
	setIconSPUFromIcon(&m_tileData.grass_iconSideOverlay, Tile::grass->iconSideOverlay);

	// ThinFence
	setIconSPUFromIcon(&m_tileData.ironFence_EdgeTexture, ((ThinFenceTile*)Tile::ironFence)->getEdgeTexture());
	setIconSPUFromIcon(&m_tileData.thinGlass_EdgeTexture, ((ThinFenceTile*)Tile::thinGlass)->getEdgeTexture());

	//FarmTile
	setIconSPUFromIcon(&m_tileData.farmTile_Dry, ((FarmTile*)Tile::farmland)->iconDry);
	setIconSPUFromIcon(&m_tileData.farmTile_Wet, ((FarmTile*)Tile::farmland)->iconWet);

	// DoorTile
	for(int i=0;i<8; i++)
	{
		setIconSPUFromIcon(&m_tileData.doorTile_Icons[i], ((DoorTile*)Tile::door_wood)->icons[i]);
		// we're not supporting flipped icons, so manually flip here
		if(i>=4)
			m_tileData.doorTile_Icons[i].flipHorizontal();
	}

	// TallGrass
	for(int i=0;i<3; i++)
		setIconSPUFromIcon(&m_tileData.tallGrass_Icons[i], Tile::tallgrass->icons[i]);

	// SandStoneTile
	for(int i=0;i<3; i++)
		setIconSPUFromIcon(&m_tileData.sandStone_icons[i], ((SandStoneTile*)Tile::sandStone)->icons[i]);
	setIconSPUFromIcon(&m_tileData.sandStone_iconTop, ((SandStoneTile*)Tile::sandStone)->iconTop);
	setIconSPUFromIcon(&m_tileData.sandStone_iconBottom, ((SandStoneTile*)Tile::sandStone)->iconBottom);

	// WoodTile
// 	assert(WoodTile_SPU::WOOD_NAMES_LENGTH == 4);
	for(int i=0;i<4; i++)
		setIconSPUFromIcon(&m_tileData.woodTile_icons[i], ((WoodTile*)Tile::wood)->icons[i]);

	// TreeTile
// 	assert(TreeTile_SPU::TREE_NAMES_LENGTH == 4);
	for(int i=0;i<4; i++)
		setIconSPUFromIcon(&m_tileData.treeTile_icons[i], ((TreeTile*)Tile::treeTrunk)->icons[i]);
	setIconSPUFromIcon(&m_tileData.treeTile_iconTop, ((TreeTile*)Tile::treeTrunk)->iconTop);

	// LeafTile
	for(int i=0;i<2; i++)
		for(int j=0;j<4;j++)
			setIconSPUFromIcon(&m_tileData.leafTile_icons[i][j], ((LeafTile*)Tile::leaves)->icons[i][j]);

	// CropTile
	for(int i=0;i<8; i++)
		setIconSPUFromIcon(&m_tileData.cropTile_icons[i], ((CropTile*)Tile::crops)->icons[i]);

	// FurnaceTile
	setIconSPUFromIcon(&m_tileData.furnaceTile_iconTop, ((FurnaceTile*)Tile::furnace)->iconTop);
	setIconSPUFromIcon(&m_tileData.furnaceTile_iconFront, ((FurnaceTile*)Tile::furnace)->iconFront);
	setIconSPUFromIcon(&m_tileData.furnaceTile_iconFront_lit, ((FurnaceTile*)Tile::furnace_lit)->iconFront);

	//LiquidTile
	setIconSPUFromIcon(&m_tileData.liquidTile_iconWaterStill, (Tile::water)->icons[0]);
	setIconSPUFromIcon(&m_tileData.liquidTile_iconWaterFlow, (Tile::water)->icons[1]);
	setIconSPUFromIcon(&m_tileData.liquidTile_iconLavaStill, (Tile::lava)->icons[0]);
	setIconSPUFromIcon(&m_tileData.liquidTile_iconLavaFlow, (Tile::lava)->icons[1]);

	//FireTile
	for(int i=0;i<2;i++)
		setIconSPUFromIcon(&m_tileData.fireTile_icons[i], (Tile::fire)->icons[i]);

	// Sapling
	for(int i=0;i<4;i++)
		setIconSPUFromIcon(&m_tileData.sapling_icons[i], ((Sapling*)Tile::sapling)->icons[i]);

	m_tileData.glassTile_allowSame = ((GlassTile*)Tile::glass)->allowSame;
	m_tileData.iceTile_allowSame = ((IceTile*)Tile::ice)->allowSame;

	// DispenserTile
	setIconSPUFromIcon(&m_tileData.dispenserTile_iconTop, ((DispenserTile*)Tile::dispenser)->iconTop);
	setIconSPUFromIcon(&m_tileData.dispenserTile_iconFront, ((DispenserTile*)Tile::dispenser)->iconFront);
	setIconSPUFromIcon(&m_tileData.dispenserTile_iconFrontVertical, ((DispenserTile*)Tile::dispenser)->iconFrontVertical);

	// RailTile
	setIconSPUFromIcon(&m_tileData.railTile_iconTurn, ((RailTile*)Tile::rail)->iconTurn);
	setIconSPUFromIcon(&m_tileData.railTile_iconTurnGolden, ((RailTile*)Tile::goldenRail)->iconTurn);

	for(int i=0;i<2;i++)
		setIconSPUFromIcon(&m_tileData.detectorRailTile_icons[i], ((DetectorRailTile*)Tile::detectorRail)->icons[i]);

	// tntTile
	setIconSPUFromIcon(&m_tileData.tntTile_iconBottom, ((TntTile*)Tile::tnt)->iconBottom);
	setIconSPUFromIcon(&m_tileData.tntTile_iconTop, ((TntTile*)Tile::tnt)->iconTop);

	// workbenchTile
	setIconSPUFromIcon(&m_tileData.workBench_iconFront, ((WorkbenchTile*)Tile::workBench)->iconFront);
	setIconSPUFromIcon(&m_tileData.workBench_iconTop, ((WorkbenchTile*)Tile::workBench)->iconTop);

	// cactusTile
	setIconSPUFromIcon(&m_tileData.cactusTile_iconTop, ((CactusTile*)Tile::cactus)->iconTop);
	setIconSPUFromIcon(&m_tileData.cactusTile_iconBottom, ((CactusTile*)Tile::cactus)->iconBottom);

	// recordPlayer
	setIconSPUFromIcon(&m_tileData.recordPlayer_iconTop, ((RecordPlayerTile*)Tile::recordPlayer)->iconTop);

	// pumpkin 
	setIconSPUFromIcon(&m_tileData.pumpkinTile_iconTop, ((PumpkinTile*)Tile::pumpkin)->iconTop);
	setIconSPUFromIcon(&m_tileData.pumpkinTile_iconFace, ((PumpkinTile*)Tile::pumpkin)->iconFace);
	setIconSPUFromIcon(&m_tileData.pumpkinTile_iconFaceLit, ((PumpkinTile*)Tile::litPumpkin)->iconFace);

	// cakeTile 
	setIconSPUFromIcon(&m_tileData.cakeTile_iconTop, ((CakeTile*)Tile::cake)->iconTop);
	setIconSPUFromIcon(&m_tileData.cakeTile_iconBottom, ((CakeTile*)Tile::cake)->iconBottom);
	setIconSPUFromIcon(&m_tileData.cakeTile_iconInner, ((CakeTile*)Tile::cake)->iconInner);

	// SmoothStoneBrickTile
	for(int i=0;i<4;i++)
		setIconSPUFromIcon(&m_tileData.smoothStoneBrick_icons[i], ((SmoothStoneBrickTile*)Tile::stoneBrickSmooth)->icons[i]);

	// HugeMushroomTile
	for(int i=0;i<2;i++)
		setIconSPUFromIcon(&m_tileData.hugeMushroom_icons[i], ((HugeMushroomTile*)Tile::hugeMushroom1)->icons[i]);
	setIconSPUFromIcon(&m_tileData.hugeMushroom_iconStem, ((HugeMushroomTile*)Tile::hugeMushroom1)->iconStem);
	setIconSPUFromIcon(&m_tileData.hugeMushroom_iconInside, ((HugeMushroomTile*)Tile::hugeMushroom1)->iconInside);


	// MelonTile
	setIconSPUFromIcon(&m_tileData.melonTile_iconTop, ((MelonTile*)Tile::melon)->iconTop);

	// StemTile
	setIconSPUFromIcon(&m_tileData.stemTile_iconAngled, ((StemTile*)Tile::melonStem)->iconAngled);

	// MycelTile
	setIconSPUFromIcon(&m_tileData.mycelTile_iconTop, (Tile::mycel)->iconTop);
	setIconSPUFromIcon(&m_tileData.mycelTile_iconSnowSide, (Tile::mycel)->iconSnowSide);

	// NetherStalkTile
	for(int i=0;i<3;i++)
		setIconSPUFromIcon(&m_tileData.netherStalk_icons[i], ((NetherStalkTile*)Tile::netherStalk)->icons[i]);

	// EnchantmentTableTile
	setIconSPUFromIcon(&m_tileData.enchantmentTable_iconTop, ((EnchantmentTableTile*)Tile::enchantTable)->iconTop);
	setIconSPUFromIcon(&m_tileData.enchantmentTable_iconBottom, ((EnchantmentTableTile*)Tile::enchantTable)->iconBottom);

	//BrewingStandTile
	setIconSPUFromIcon(&m_tileData.brewingStand_iconBase, ((BrewingStandTile*)Tile::brewingStand)->iconBase);

	//RedStoneDust
	setIconSPUFromIcon(&m_tileData.redStoneDust_iconCross, (Tile::redStoneDust)->iconCross);
	setIconSPUFromIcon(&m_tileData.redStoneDust_iconCross, (Tile::redStoneDust)->iconLine);
	setIconSPUFromIcon(&m_tileData.redStoneDust_iconCrossOver, (Tile::redStoneDust)->iconCrossOver);
	setIconSPUFromIcon(&m_tileData.redStoneDust_iconLineOver, (Tile::redStoneDust)->iconLineOver);

	setIconSPUFromIcon(&m_tileData.stoneSlab_iconSide, ((StoneSlabTile*)(Tile::stoneSlab))->iconSide);

	for(int i=0;i<16;i++)
		setIconSPUFromIcon(&m_tileData.clothTile_icons[i], ((ClothTile*)Tile::cloth)->icons[i]);

	// CarrotTile
	for(int i=0;i<4;i++)
		setIconSPUFromIcon(&m_tileData.carrot_icons[i], ((CarrotTile*)Tile::carrots)->icons[i]);

	// PotatoTile
	for(int i=0;i<4;i++)
		setIconSPUFromIcon(&m_tileData.potato_icons[i], ((PotatoTile*)Tile::potatoes)->icons[i]);

	// AnvilTile
	for(int i=0;i<3;i++)
		setIconSPUFromIcon(&m_tileData.anvil_icons[i], ((AnvilTile*)Tile::anvil)->icons[i]);


	// QuartzBlockTile
	for(int i=0;i<5;i++)
		setIconSPUFromIcon(&m_tileData.quartzBlock_icons[i], ((QuartzBlockTile*)Tile::quartzBlock)->icons[i]);

	setIconSPUFromIcon(&m_tileData.quartzBlock_iconChiseledTop, ((QuartzBlockTile*)Tile::quartzBlock)->iconChiseledTop);
	setIconSPUFromIcon(&m_tileData.quartzBlock_iconLinesTop, ((QuartzBlockTile*)Tile::quartzBlock)->iconLinesTop);
	setIconSPUFromIcon(&m_tileData.quartzBlock_iconTop, ((QuartzBlockTile*)Tile::quartzBlock)->iconTop);
	setIconSPUFromIcon(&m_tileData.quartzBlock_iconBottom, ((QuartzBlockTile*)Tile::quartzBlock)->iconBottom);
}

// extern int g_lastHitBlockX;
// extern int g_lastHitBlockY;
// extern int g_lastHitBlockZ;

void ChunkRebuildData::buildForChunk( Region* region, Level* level, int x0, int y0, int z0 )
{
	static  bool bCreatedTileData = false;
	if(!bCreatedTileData)
	{
		createTileData();
		bCreatedTileData = true;
	}

	m_tileData.stemTile_minColour = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Tile_StemMin );
	m_tileData.stemTile_maxColour = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Tile_StemMax );
	m_tileData.waterLily_colour = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Tile_WaterLily );

	m_tileData.foliageColor_evergreenColor =  FoliageColor::getEvergreenColor();
	m_tileData.foliageColor_birchColor = FoliageColor::getBirchColor();


// 	m_lastHitBlockX = g_lastHitBlockX;
// 	m_lastHitBlockY = g_lastHitBlockY;
// 	m_lastHitBlockZ = g_lastHitBlockZ;

	m_x0 = x0-2;
	m_y0 = y0-2;
	m_z0 = z0-2;
	m_pRegion = region;
	m_flags = 0;//e_flag_EmptyChunk;

// 	for(int iX=m_x0; iX<(m_x0+sc_size); iX++)
// 	{
// 		for(int iY=m_y0; iY<(m_y0+sc_size); iY++)
// 		{
// 			for(int iZ=m_z0; iZ<(m_z0+sc_size); iZ++)
// 			{
// 				buildTile(iX, iY, iZ);
// 			}
// 		}
// 	}

	BiomeCache::Block* cacheBlocks[9];
	cacheBlocks[0] = region->getBiomeSource()->getBlockAt(m_x0, m_z0);
	cacheBlocks[1] = region->getBiomeSource()->getBlockAt(m_x0+2, m_z0);
	cacheBlocks[2] = region->getBiomeSource()->getBlockAt(m_x0+18, m_z0);

	cacheBlocks[3] = region->getBiomeSource()->getBlockAt(m_x0, m_z0+2);
	cacheBlocks[4] = region->getBiomeSource()->getBlockAt(m_x0+2, m_z0+2);
	cacheBlocks[5] = region->getBiomeSource()->getBlockAt(m_x0+18, m_z0+2);

	cacheBlocks[6] = region->getBiomeSource()->getBlockAt(m_x0, m_z0+18);
	cacheBlocks[7] = region->getBiomeSource()->getBlockAt(m_x0+2, m_z0+18);
	cacheBlocks[8] = region->getBiomeSource()->getBlockAt(m_x0+18, m_z0+18);

	int cacheMap[20] = { 0,0,  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  2,2};

	ColourTable* pColourTable = Minecraft::GetInstance()->getColourTable();

	for(int iX=m_x0; iX<(m_x0+sc_size); iX++)
	{
		for(int iZ=m_z0; iZ<(m_z0+sc_size); iZ++)
		{
			int newX = ( iX - m_x0 );
			int newZ = ( iZ - m_z0 );
			int index = (newX*sc_size) + newZ;
//			assert(index < 400);
			int cacheBlockIndex = (cacheMap[iZ-m_z0]*3) + cacheMap[iX-m_x0];
			BiomeCache::Block* pCacheBlock = cacheBlocks[cacheBlockIndex];
//			assert(region->getBiomeSource()->getBlockAt(iX, iZ) == pCacheBlock);		
			Biome* pBiome = pCacheBlock->getBiome(iX, iZ);
			m_grassColor[index] = pColourTable->getColor(pBiome->m_grassColor);
			m_foliageColor[index] =  pColourTable->getColor(pBiome->m_foliageColor);
			m_waterColor[index] =  pColourTable->getColor(pBiome->m_waterColor);
		}
	}

	if(isEmptyChunk())
		return;


	if(level->dimension->hasCeiling)
		m_flags |= e_flag_HasCeiling;
	for(int i=0;i<16;i++)
		m_brightnessRamp[i] = level->dimension->brightnessRamp[i];
	m_levelSkyDarken = level->skyDarken;
}

void ChunkRebuildData::copyFromTesselator()
{
	Tesselator* t = Tesselator::getInstance(); // 4J - added - static initialiser being set at the wrong time

	m_tesselator.m_PPUArray = t->_array->data;
	assert(t->p == 0);
	m_tesselator.m_PPUOffset = 0;

	// copy tesselator vars over
	m_tesselator.vertices = t->vertices;																																																																																																																					
	m_tesselator.u = t->u;
	m_tesselator.v = t->v;
	m_tesselator._tex2 = t->_tex2;
	m_tesselator.col = t->col;
	m_tesselator.hasColor = t->hasColor;
	m_tesselator.hasTexture = t->hasTexture;
	m_tesselator.hasTexture2 = t->hasTexture2;
	m_tesselator.hasNormal = t->hasNormal;
	m_tesselator.p = t->p;
	m_tesselator.useCompactFormat360 = t->useCompactFormat360;
	m_tesselator.useProjectedTexturePixelShader = t->useProjectedTexturePixelShader;
	m_tesselator.count = t->count;
	m_tesselator._noColor = t->_noColor;
	m_tesselator.mode = t->mode;
	m_tesselator.xo = t->xo;
	m_tesselator.yo = t->yo;
	m_tesselator.zo = t->zo;
	m_tesselator.count = t->count	;
	m_tesselator._normal = t->_normal;
	m_tesselator.tesselating = t->tesselating;
	m_tesselator.mipmapEnable = t->mipmapEnable;	// 4J added
	m_tesselator.vboId = t->vboId;
	m_tesselator.vboCounts = t->vboCounts;
	m_tesselator.size = t->size;
	for(int i=0;i<6;i++)
		m_tesselator.bounds.boundingBox[i] = t->bounds.boundingBox[i];

}


void ChunkRebuildData::buildTile(int x, int y, int z)
{
	int newX = ( x - m_x0 );
	int newY = ( y - m_y0 );
	int newZ = ( z - m_z0 );

	int index = (newX*sc_size*sc_size) + (newY*sc_size) + newZ;

 	m_tileIds[index] = m_pRegion->getTile(x, y, z);
	m_brightness[index] = m_pRegion->getBrightness(LightLayer::Sky, x, y, z);
	m_brightness[index] |= (m_pRegion->getBrightness(LightLayer::Block, x, y, z)) << 4;
	m_data_flags[index] = m_pRegion->getData(x,y,z);
}


void ChunkRebuildData::storeInTesselator()
{
	Tesselator* t = Tesselator::getInstance(); // 4J - added - static initialiser being set at the wrong time

	// copy tesselator vars over
	t->vertices = m_tesselator.vertices;																																																																																																																					
	t->u = m_tesselator.u;
	t->v = m_tesselator.v;
	t->_tex2 = m_tesselator._tex2;
	t->col = m_tesselator.col;
	t->hasColor = m_tesselator.hasColor;
	t->hasTexture = m_tesselator.hasTexture;
	t->hasTexture2 = m_tesselator.hasTexture2;
	t->hasNormal = m_tesselator.hasNormal;
	t->p = m_tesselator.m_PPUOffset/4;
	t->useCompactFormat360 = m_tesselator.useCompactFormat360;
	t->useProjectedTexturePixelShader = m_tesselator.useProjectedTexturePixelShader;
	t->count = m_tesselator.count;
	t->_noColor = m_tesselator._noColor;
	t->mode = m_tesselator.mode;
	t->xo = m_tesselator.xo;
	t->yo = m_tesselator.yo;
	t->zo = m_tesselator.zo;
	t->count = m_tesselator.count;
	t->_normal = m_tesselator._normal;
	t->tesselating = m_tesselator.tesselating;
	t->mipmapEnable = m_tesselator.mipmapEnable;	// 4J added
	t->vboId = m_tesselator.vboId;
	t->vboCounts = m_tesselator.vboCounts;
	t->size = m_tesselator.size;
	for(int i=0;i<6;i++)
		t->bounds.boundingBox[i] = m_tesselator.bounds.boundingBox[i];
}
#endif // SN_TARGET_PS3_SPU



void ChunkRebuildData::tesselateAllTiles(TileRenderer_SPU* pTileRenderer)
{
	Tile_SPU::ms_pTileData = &m_tileData;
	// 4J - changed loop order here to leave y as the innermost loop for better cache performance
	bool renderNextLayer = false;
	bool rendered = false;
	int numRenderedLayer0 = 0;
	int startX = m_x0+2;	// beginning of the chunk
	int startY = m_y0+2;	// beginning of the chunk
	int startZ = m_z0+2;	// beginning of the chunk

	m_tesselator.beginData();
	for (int z = startZ; z < (startZ+16); z++)
	{
		for (int x = startX; x < (startX+16); x++)
		{
			for (int y = startY; y < (startY+16); y++)
			{
				// 4J - get tile from those copied into our local array in earlier optimisation
				int flags = getFlags(x,y,z);
				if(flags & ChunkRebuildData::e_flag_NoRender)
					continue;
				unsigned char tileId = getTile(x,y,z);
				if (tileId > 0)
				{
// 					if (m_currentLayer == 0 && m_tileData.isEntityTile[tileId])
// 					{
// 						shared_ptr<TileEntity> et = region->getTileEntity(x, y, z);
// 						if (TileEntityRenderDispatcher::instance->hasRenderer(et))
// 						{
// 							renderableTileEntities.push_back(et);
// 						}
// 					}
					TileRef_SPU tile(tileId);
					int renderLayer = tile->getRenderLayer();

					if (renderLayer != m_currentLayer)
					{
						renderNextLayer = true;
					}
					else //if (renderLayer == m_currentLayer)
					{
						if(pTileRenderer->hasRenderer(tile.getPtr()))
						{
							numRenderedLayer0++;
							rendered |= pTileRenderer->tesselateInWorld(tile.getPtr(), x, y, z);
						}
						else
						{
							setFlag(x, y, z, e_flag_SPURenderCodeMissing);
// #ifdef SN_TARGET_PS3_SPU
// 							spu_print("Render code missing for tile ID %d : render shape %d\n", tile->id, tile->getRenderShape());
// #endif

						}
					}
				}
			}
		}
	}
	if(rendered)
		m_flags |= e_flag_Rendered;

// 	spu_print("(%d,%d,%d) SPU num rendered : %d\n", m_x0+2, m_y0+2, m_z0+2,  numRenderedLayer0);

	m_tesselator.endData();
}

bool ChunkRebuildData::isEmptyTile(int x, int y, int z)
{
	return getTile(x, y, z) == 0;
}

bool ChunkRebuildData::isSolidRenderTile(int x, int y, int z)
{
	TileRef_SPU tile(getTile(x,y,z));
	if (tile.getPtr() == NULL) return false;


	// 4J - addition here to make rendering big blocks of leaves more efficient. Normally leaves never consider themselves as solid, so
	// blocks of leaves will have all sides of each block completely visible. Changing to consider as solid if this block is surrounded by
	// other leaves. This is paired with another change in Tile::getTexture which makes such solid tiles actually visibly solid (these
	// textures exist already for non-fancy graphics). Note: this tile-specific code is here rather than making some new virtual method in the tiles,
	// for the sake of efficiency - I don't imagine we'll be doing much more of this sort of thing
	if( tile->id == Tile_SPU::leaves_Id )
	{
		int axo[6] = { 1,-1, 0, 0, 0, 0};
		int ayo[6] = { 0, 0, 1,-1, 0, 0};
		int azo[6] = { 0, 0, 0, 0, 1,-1};
		for( int i = 0; i < 6; i++ )
		{
			int t = getTile(x + axo[i], y + ayo[i] , z + azo[i]);
			if( ( t != Tile_SPU::leaves_Id ) && ( ( Tile_SPU::m_tiles[t].id == -1) || !Tile_SPU::m_tiles[t].isSolidRender() ) )
			{
				return false;
			}
		}
		return true;
	}

	return tile->isSolidRender();
}


bool ChunkRebuildData::isSolidBlockingTile(int x, int y, int z)
{
	TileRef_SPU tile(getTile(x, y, z));
	if (tile.getPtr() == NULL) return false;
	bool ret =  tile->getMaterial()->blocksMotion() && tile->isCubeShaped();
	return ret;
}

float ChunkRebuildData::getBrightness(int x, int y, int z, int emitt)
{
	int n = getRawBrightness(x, y, z);
	if (n < emitt) n = emitt;
	return m_brightnessRamp[n];
}

float ChunkRebuildData::getBrightness(int x, int y, int z)
{
	return m_brightnessRamp[getRawBrightness(x, y, z)];
}

// 4J - brought forward from 1.8.2
int ChunkRebuildData::getBrightness(LightLayer::variety layer, int x, int y, int z)
{
#ifndef SN_TARGET_PS3_SPU
	assert(Level_MAX_LEVEL_SIZE == Level::MAX_LEVEL_SIZE);
	assert(Level_maxBuildHeight == Level::maxBuildHeight);
	assert(Level_MAX_BRIGHTNESS == Level::MAX_BRIGHTNESS);
#endif

	if (y < 0) y = 0;
	if (y >= Level_maxBuildHeight) y = Level_maxBuildHeight - 1;
	if (y < 0 || y >= Level_maxBuildHeight || x < -Level_MAX_LEVEL_SIZE || z < -Level_MAX_LEVEL_SIZE || x >= Level_MAX_LEVEL_SIZE || z > Level_MAX_LEVEL_SIZE)
	{
		// 4J Stu - The java LightLayer was an enum class type with a member "surrounding" which is what we
		// were returning here. Surrounding has the same value as the enum value in our C++ code, so just cast
		// it to an int
		return (int)layer;
	}

	if(layer == LightLayer::Sky)
		return getBrightnessSky(x, y, z);
	return getBrightnessBlock(x, y, z);
}

// 4J - brought forward from 1.8.2
int ChunkRebuildData::getBrightnessPropagate(LightLayer::variety layer, int x, int y, int z)
{
	if (y < 0) y = 0;
	if (y >= Level_maxBuildHeight) y = Level_maxBuildHeight - 1;
	if (y < 0 || y >= Level_maxBuildHeight || x < -Level_MAX_LEVEL_SIZE || z < -Level_MAX_LEVEL_SIZE || x >= Level_MAX_LEVEL_SIZE || z > Level_MAX_LEVEL_SIZE)
	{
		// 4J Stu - The java LightLayer was an enum class type with a member "surrounding" which is what we
		// were returning here. Surrounding has the same value as the enum value in our C++ code, so just cast
		// it to an int
		return (int)layer;
	}

	{
		int id = getTile(x, y, z);
		if (m_tileData.propagate[getTile(x, y, z)])
		{
			int br = getBrightness(layer, x, y + 1, z);
			int br1 = getBrightness(layer, x + 1, y, z);
			int br2 = getBrightness(layer, x - 1, y, z);
			int br3 = getBrightness(layer, x, y, z + 1);
			int br4 = getBrightness(layer, x, y, z - 1);
			if (br1 > br) br = br1;
			if (br2 > br) br = br2;
			if (br3 > br) br = br3;
			if (br4 > br) br = br4;
			return br;
		}
	}

	if(layer == LightLayer::Sky)
		return getBrightnessSky(x, y, z);
	return getBrightnessBlock(x, y, z);
	
}
int ChunkRebuildData::getLightColor(int x, int y, int z, int emitt)	// 4J - change brought forward from 1.8.2
{
	int s = getBrightnessPropagate(LightLayer::Sky, x, y, z);
	int b = getBrightnessPropagate(LightLayer::Block, x, y, z);
	if (b < emitt) b = emitt;
	return s << 20 | b << 4;
}


int ChunkRebuildData::getRawBrightness(int x, int y, int z)
{
	return getRawBrightness(x, y, z, true);
}


int ChunkRebuildData::getRawBrightness(int x, int y, int z, bool propagate)
{
	if (x < -Level_MAX_LEVEL_SIZE || z < -Level_MAX_LEVEL_SIZE || x >= Level_MAX_LEVEL_SIZE || z > Level_MAX_LEVEL_SIZE)
	{
		return Level_MAX_BRIGHTNESS;
	}

	if (propagate)
	{
		int id = getTile(x, y, z);
		switch(id)
		{
		case Tile_SPU::stoneSlabHalf_Id:
		case Tile_SPU::woodSlabHalf_Id:
		case Tile_SPU::farmland_Id:
		case Tile_SPU::stairs_stone_Id:
		case Tile_SPU::stairs_wood_Id:
			{
				int br = getRawBrightness(x, y + 1, z, false);
				int br1 = getRawBrightness(x + 1, y, z, false);
				int br2 = getRawBrightness(x - 1, y, z, false);
				int br3 = getRawBrightness(x, y, z + 1, false);
				int br4 = getRawBrightness(x, y, z - 1, false);
				if (br1 > br) br = br1;
				if (br2 > br) br = br2;
				if (br3 > br) br = br3;
				if (br4 > br) br = br4;
				return br;
			}
			break;
		}
	}

	if (y < 0) return 0;
	if (y >= Level_maxBuildHeight)
	{
		int br = Level_MAX_BRIGHTNESS - m_levelSkyDarken;
		if (br < 0) br = 0;
		return br;
	}

	return LevelChunk_getRawBrightness(x, y, z, m_levelSkyDarken);
}

int ChunkRebuildData::LevelChunk_getRawBrightness(int x, int y, int z, int skyDampen)
{
	   int light = (m_flags & e_flag_HasCeiling) ? 0 : getBrightnessSky(x, y, z);
    if (light > 0) 
		m_flags |= e_flag_TouchedSky;
    light -= skyDampen;
    int block = getBrightnessBlock(x, y, z);
    if (block > light) light = block;

    /*
        * int xd = (absFloor(level.player.x-(this->x*16+x))); int yd =
        * (absFloor(level.player.y-(y))); int zd =
        * (absFloor(level.player.z-(this->z*16+z))); int dd = xd+yd+zd; if
        * (dd<15){ int carried = 15-dd; if (carried<0) carried = 0; if
        * (carried>15) carried = 15; if (carried > light) light = carried; }
        */

    return light;

}


Material_SPU *ChunkRebuildData::getMaterial(int x, int y, int z)
{
	int t = getTile(x, y, z);
	int matID = Material_SPU::air_Id;
	if (t != 0)
		matID = m_tileData.materialIDs[t];
	return &m_tileData.materials[matID];
}
