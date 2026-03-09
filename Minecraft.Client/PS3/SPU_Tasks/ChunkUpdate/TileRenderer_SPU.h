#pragma once

#include <stddef.h>
//#include <string>
#include "Tile_SPU.h"
#include "Icon_SPU.h"
#include "ChunkRebuildData.h"

class Level;
class LevelSource;
class Tile_SPU;
class RailTile_SPU;
class Material;
class TileEntity;
class ThinFenceTile;
class FenceTile_SPU;
class FenceGateTile_SPU;
class BrewingStandTile_SPU;
class CauldronTile_SPU;
class EggTile_SPU;
class TheEndPortalFrameTile;
class DiodeTile_SPU;
class FireTile_SPU;
class StemTile_SPU;
class WaterlilyTile_SPU;
class StairTile_SPU;
class CocoaTile_SPU;
class AnvilTile_SPU;
class FlowerPotTile_SPU;
class WallTile_SPU;

class Icon;
class Minecraft;

class TileRenderer_SPU
{
	friend class FallingTileRenderer_SPU;
	private:
	ChunkRebuildData* level;
	Icon_SPU *fixedTexture;
	bool xFlipTexture;
	bool noCulling;
	public :
	static bool fancy;
	bool setColor;

	float tileShapeX0;
	float tileShapeX1;
	float tileShapeY0;
	float tileShapeY1;
	float tileShapeZ0;
	float tileShapeZ1;
	bool fixedShape;
	bool smoothShapeLighting;
// 	Minecraft *minecraft;

	void _init();

public:
	TileRenderer_SPU( ChunkRebuildData* level );
	TileRenderer_SPU();
	Tesselator_SPU* getTesselator();

	bool hasRenderer(Tile_SPU* tt);
	void setFixedTexture( Icon_SPU *fixedTexture );
	void clearFixedTexture();
	bool hasFixedTexture();
	void setShape(float x0, float y0, float z0, float x1, float y1, float z1);
	void setShape(Tile_SPU *tt);
	void setFixedShape(float x0, float y0, float z0, float x1, float y1, float z1);
	void clearFixedShape();

	void tesselateInWorldFixedTexture( Tile_SPU* tile, int x, int y, int z, Icon_SPU *fixedTexture );	// 4J renamed to differentiate from tesselateInWorld
	void tesselateInWorldNoCulling( Tile_SPU* tile, int x, int y, int z, int forceData = -1,
									TileEntity* forceEntity = nullptr );	// 4J added forceData, forceEntity param
	bool tesselateInWorld( Tile_SPU* tt, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr );	// 4J added forceData, forceEntity param

	private:
	bool tesselateAirPortalFrameInWorld(TheEndPortalFrameTile *tt, int x, int y, int z);
	bool tesselateBedInWorld( Tile_SPU* tt, int x, int y, int z );
	bool tesselateBrewingStandInWorld(BrewingStandTile_SPU *tt, int x, int y, int z);
	bool tesselateCauldronInWorld(CauldronTile_SPU *tt, int x, int y, int z);
	bool tesselateFlowerPotInWorld(FlowerPotTile_SPU *tt, int x, int y, int z);
	bool tesselateAnvilInWorld(AnvilTile_SPU *tt, int x, int y, int z);

public:
	bool tesselateAnvilInWorld(AnvilTile_SPU *tt, int x, int y, int z, int data);

private:
	bool tesselateAnvilInWorld(AnvilTile_SPU *tt, int x, int y, int z, int data, bool render);
	float tesselateAnvilPiece(AnvilTile_SPU *tt, int x, int y, int z, int part, float bottom, float width, float height, float length, bool rotate, bool render, int data);


	public:
	bool tesselateTorchInWorld( Tile_SPU* tt, int x, int y, int z );
	private:
	bool tesselateDiodeInWorld(DiodeTile_SPU *tt, int x, int y, int z);
	void tesselateDiodeInWorld( DiodeTile_SPU* tt, int x, int y, int z, int dir );
	static const int FLIP_NONE = 0, FLIP_CW = 1, FLIP_CCW = 2, FLIP_180 = 3;

	int northFlip;
	int southFlip;
	int eastFlip;
	int westFlip;
	int upFlip;
	int downFlip;
	public:
	void tesselatePistonBaseForceExtended( Tile_SPU* tile, int x, int y, int z, int forceData = -1 ); // 4J added data param
	private:
	bool tesselatePistonBaseInWorld( Tile_SPU* tt, int x, int y, int z, bool forceExtended, int forceData = -1 );	// 4J added data param
	void renderPistonArmUpDown( float x0, float x1, float y0, float y1, float z0, float z1, float br, float armLengthPixels );
	void renderPistonArmNorthSouth( float x0, float x1, float y0, float y1, float z0, float z1, float br, float armLengthPixels );
	void renderPistonArmEastWest( float x0, float x1, float y0, float y1, float z0, float z1, float br, float armLengthPixels );
	public:
	void tesselatePistonArmNoCulling( Tile_SPU* tile, int x, int y, int z, bool fullArm, int forceData = -1 ); // 4J added data param
	private:
	bool tesselatePistonExtensionInWorld( Tile_SPU* tt, int x, int y, int z, bool fullArm, int forceData = -1 ); // 4J added data param
	public:
	bool tesselateLeverInWorld( Tile_SPU* tt, int x, int y, int z );
	bool tesselateTripwireSourceInWorld(Tile_SPU *tt, int x, int y, int z);
	bool tesselateTripwireInWorld(Tile_SPU *tt, int x, int y, int z);
	bool tesselateFireInWorld( FireTile_SPU* tt, int x, int y, int z );
	bool tesselateDustInWorld( Tile_SPU* tt, int x, int y, int z );
	bool tesselateRailInWorld( RailTile_SPU* tt, int x, int y, int z );
	bool tesselateLadderInWorld( Tile_SPU* tt, int x, int y, int z );
	bool tesselateVineInWorld( Tile_SPU* tt, int x, int y, int z );
	bool tesselateThinFenceInWorld( ThinFenceTile* tt, int x, int y, int z );
	bool tesselateCrossInWorld( Tile_SPU* tt, int x, int y, int z );
	bool tesselateStemInWorld( Tile_SPU* _tt, int x, int y, int z );
	bool tesselateRowInWorld( Tile_SPU* tt, int x, int y, int z );
	void tesselateTorch( Tile_SPU* tt, float x, float y, float z, float xxa, float zza, int data );
	void tesselateCrossTexture( Tile_SPU* tt, int data, float x, float y, float z, float scale );
	void tesselateStemTexture( Tile_SPU* tt, int data, float h, float x, float y, float z );
	bool tesselateLilypadInWorld(WaterlilyTile_SPU *tt, int x, int y, int z);
	void tesselateStemDirTexture( StemTile_SPU* tt, int data, int dir, float h, float x, float y, float z );

	void tesselateRowTexture( Tile_SPU* tt, int data, float x, float y, float z );
	bool tesselateWaterInWorld( Tile_SPU* tt, int x, int y, int z );
	private:
	float getWaterHeight( int x, int y, int z, Material_SPU* m );
	public:
	void renderBlock( Tile_SPU* tt, ChunkRebuildData* level, int x, int y, int z );
	void renderBlock(Tile_SPU *tt, ChunkRebuildData *level, int x, int y, int z, int data);
	bool tesselateBlockInWorld( Tile_SPU* tt, int x, int y, int z );
	bool tesselateTreeInWorld(Tile_SPU *tt, int x, int y, int z);
	bool tesselateQuartzInWorld(Tile_SPU *tt, int x, int y, int z);
	bool tesselateCocoaInWorld(CocoaTile_SPU *tt, int x, int y, int z);

	private:
	bool applyAmbienceOcclusion;
	float ll000, llx00, ll0y0, ll00z, llX00, ll0Y0, ll00Z;
	float llxyz, llxy0, llxyZ, ll0yz, ll0yZ, llXyz, llXy0;
	float llXyZ, llxYz, llxY0, llxYZ, ll0Yz, llXYz, llXY0;
	float ll0YZ, llXYZ, llx0z, llX0z, llx0Z, llX0Z;
	// 4J - brought forward changes from 1.8.2
	int ccx00, cc00z, cc0Y0, cc00Z;
	int ccxyz, ccxy0, ccxyZ, cc0yz, cc0yZ, ccXyz, ccXy0;
	int ccXyZ, ccxYz, ccxY0, ccxYZ, cc0Yz, ccXYz, ccXY0;
	int cc0YZ, ccXYZ, ccx0z, ccX0z, ccx0Z, ccX0Z;
	int blsmooth;
	int tc1, tc2, tc3, tc4; // 4J - brought forward changes from 1.8.2
	float c1r, c2r, c3r, c4r;
	float c1g, c2g, c3g, c4g;
	float c1b, c2b, c3b, c4b;
	bool llTrans0Yz, llTransXY0, llTransxY0, llTrans0YZ;
	bool llTransx0z, llTransX0Z, llTransx0Z, llTransX0z;
	bool llTrans0yz, llTransXy0, llTransxy0, llTrans0yZ;

	public:
	// 4J - brought forward changes from 1.8.2
	bool tesselateBlockInWorldWithAmbienceOcclusionTexLighting( Tile_SPU* tt, int pX, int pY, int pZ, float pBaseRed,
																float pBaseGreen, float pBaseBlue );
	bool tesselateBlockInWorldWithAmbienceOcclusionOldLighting( Tile_SPU* tt, int pX, int pY, int pZ, float pBaseRed,
																float pBaseGreen, float pBaseBlue );
	private:
	int blend( int a, int b, int c, int def );
	int blend(int a, int b, int c, int d, float fa, float fb, float fc, float fd);
	public:
	bool tesselateBlockInWorld( Tile_SPU* tt, int x, int y, int z, float r, float g, float b );
	bool tesselateCactusInWorld( Tile_SPU* tt, int x, int y, int z );
	bool tesselateCactusInWorld( Tile_SPU* tt, int x, int y, int z, float r, float g, float b );
	bool tesselateFenceInWorld( FenceTile_SPU* tt, int x, int y, int z );
	bool tesselateWallInWorld(WallTile_SPU *tt, int x, int y, int z);
	bool tesselateEggInWorld(EggTile_SPU *tt, int x, int y, int z);
	bool tesselateFenceGateInWorld(FenceGateTile_SPU *tt, int x, int y, int z);
	bool tesselateStairsInWorld( StairTile_SPU* tt, int x, int y, int z );
	bool tesselateDoorInWorld( Tile_SPU* tt, int x, int y, int z );
	void renderFaceUp( Tile_SPU* tt, float x, float y, float z, Icon_SPU *tex );
	void renderFaceDown( Tile_SPU* tt, float x, float y, float z, Icon_SPU *tex );
	void renderNorth( Tile_SPU* tt, float x, float y, float z, Icon_SPU *tex );
	void renderSouth( Tile_SPU* tt, float x, float y, float z, Icon_SPU *tex );
	void renderWest( Tile_SPU* tt, float x, float y, float z, Icon_SPU *tex );
	void renderEast( Tile_SPU* tt, float x, float y, float z, Icon_SPU *tex );
// 	void renderCube( Tile_SPU* tile, float alpha );
// 	void renderTile( Tile_SPU* tile, int data, float brightness, float fAlpha = 1.0f );
	static bool canRender( int renderShape );
	Icon_SPU *getTexture(Tile_SPU *tile, ChunkRebuildData *level, int x, int y, int z, int face);

	Icon_SPU *getTexture(Tile_SPU *tile, int face, int data);
	Icon_SPU *getTexture(Tile_SPU *tile, int face);
	Icon_SPU *getTexture(Tile_SPU *tile);
	Icon_SPU *getTextureOrMissing(Icon_SPU *icon);

	bool isAnaglyph3d() { return false; } //GameRenderer::anaglyph3d 
};
