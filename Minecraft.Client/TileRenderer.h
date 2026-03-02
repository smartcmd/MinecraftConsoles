#pragma once

class Level;
class LevelSource;
class Tile;
class RailTile;
class Material;
class TileEntity;
class ThinFenceTile;
class FenceTile;
class FenceGateTile;
class BrewingStandTile;
class CauldronTile;
class EggTile;
class TheEndPortalFrameTile;
class DiodeTile;
class FireTile;
class StemTile;
class StairTile;
class CocoaTile;
class AnvilTile;
class FlowerPotTile;
class WallTile;
class Icon;
class Minecraft;

class TileRenderer
{
	friend class FallingTileRenderer;
	private:
	LevelSource* level;
	Icon *fixedTexture;
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
	Minecraft *minecraft;
	///////////////////////////////////////// Data caching
	unordered_map<int,int> getLightColorCount;
	int xMin, yMin, zMin;
	int xMin2, yMin2, zMin2;
	int getLightColor( Tile *tt, LevelSource *level, int x, int y, int z);
	float getShadeBrightness( Tile *tt, LevelSource *level, int x, int y, int z);
	bool isTranslucentAt( LevelSource *level, int x, int y, int z);
	unsigned int *cache;
	unsigned char *tileIds;
	static const unsigned int cache_getLightColor_valid			= 0x80000000;
	static const unsigned int cache_isTranslucentAt_valid		= 0x40000000;
	static const unsigned int cache_isSolidBlockingTile_valid	= 0x20000000;
	static const unsigned int cache_getLightColor_mask			= 0x00f000f0;
	static const unsigned int cache_isTranslucentAt_flag		= 0x00000001;
	static const unsigned int cache_isSolidBlockingTile_flag	= 0x00000002;
	/////////////////////////////////////////

	void _init();

public:
	TileRenderer( LevelSource* level, int xMin, int yMin, int zMin, unsigned char *tileIds );
	TileRenderer( LevelSource* level );
	TileRenderer();
	~TileRenderer();
	void setFixedTexture( Icon *fixedTexture );
	void clearFixedTexture();
	bool hasFixedTexture();
	void setShape(float x0, float y0, float z0, float x1, float y1, float z1);
	void setShape(Tile *tt);
	void setFixedShape(float x0, float y0, float z0, float x1, float y1, float z1);
	void clearFixedShape();

	void tesselateInWorldFixedTexture( Tile* tile, int x, int y, int z, Icon *fixedTexture );	// 4J renamed to differentiate from tesselateInWorld
	void tesselateInWorldNoCulling( Tile* tile, int x, int y, int z, int forceData = -1,
									shared_ptr< TileEntity > forceEntity = shared_ptr< TileEntity >() );	// 4J added forceData, forceEntity param
	bool tesselateInWorld( Tile* tt, int x, int y, int z, int forceData = -1, shared_ptr< TileEntity > forceEntity =
						   shared_ptr< TileEntity >() );	// 4J added forceData, forceEntity param

	private:
	bool tesselateAirPortalFrameInWorld(TheEndPortalFrameTile *tt, int x, int y, int z);
	bool tesselateBedInWorld( Tile* tt, int x, int y, int z );
	bool tesselateBrewingStandInWorld(BrewingStandTile *tt, int x, int y, int z);
	bool tesselateCauldronInWorld(CauldronTile *tt, int x, int y, int z);
	bool tesselateFlowerPotInWorld(FlowerPotTile *tt, int x, int y, int z);
	bool tesselateAnvilInWorld(AnvilTile *tt, int x, int y, int z);

public:
	bool tesselateAnvilInWorld(AnvilTile *tt, int x, int y, int z, int data);

private:
	bool tesselateAnvilInWorld(AnvilTile *tt, int x, int y, int z, int data, bool render);
	float tesselateAnvilPiece(AnvilTile *tt, int x, int y, int z, int part, float bottom, float width, float height, float length, bool rotate, bool render, int data);

public:
	bool tesselateTorchInWorld( Tile* tt, int x, int y, int z );
private:
	bool tesselateDiodeInWorld(DiodeTile *tt, int x, int y, int z);
	void tesselateDiodeInWorld( DiodeTile* tt, int x, int y, int z, int dir );
	static const int FLIP_NONE = 0, FLIP_CW = 1, FLIP_CCW = 2, FLIP_180 = 3;

	int northFlip;
	int southFlip;
	int eastFlip;
	int westFlip;
	int upFlip;
	int downFlip;
	public:
	void tesselatePistonBaseForceExtended( Tile* tile, int x, int y, int z, int forceData = -1 ); // 4J added data param
	private:
	bool tesselatePistonBaseInWorld( Tile* tt, int x, int y, int z, bool forceExtended, int forceData = -1 );	// 4J added data param
	void renderPistonArmUpDown( float x0, float x1, float y0, float y1, float z0, float z1, float br, float armLengthPixels );
	void renderPistonArmNorthSouth( float x0, float x1, float y0, float y1, float z0, float z1, float br, float armLengthPixels );
	void renderPistonArmEastWest( float x0, float x1, float y0, float y1, float z0, float z1, float br, float armLengthPixels );
	public:
	void tesselatePistonArmNoCulling( Tile* tile, int x, int y, int z, bool fullArm, int forceData = -1 ); // 4J added data param
	private:
	bool tesselatePistonExtensionInWorld( Tile* tt, int x, int y, int z, bool fullArm, int forceData = -1 ); // 4J added data param
	public:
	bool tesselateLeverInWorld( Tile* tt, int x, int y, int z );
	bool tesselateTripwireSourceInWorld(Tile *tt, int x, int y, int z);
	bool tesselateTripwireInWorld(Tile *tt, int x, int y, int z);
	bool tesselateFireInWorld( FireTile* tt, int x, int y, int z );
	bool tesselateDustInWorld( Tile* tt, int x, int y, int z );
	bool tesselateRailInWorld( RailTile* tt, int x, int y, int z );
	bool tesselateLadderInWorld( Tile* tt, int x, int y, int z );
	bool tesselateVineInWorld( Tile* tt, int x, int y, int z );
	bool tesselateThinFenceInWorld( ThinFenceTile* tt, int x, int y, int z );
	bool tesselateCrossInWorld( Tile* tt, int x, int y, int z );
	bool tesselateStemInWorld( Tile* _tt, int x, int y, int z );
	bool tesselateRowInWorld( Tile* tt, int x, int y, int z );
	void tesselateTorch( Tile* tt, float x, float y, float z, float xxa, float zza, int data );
	void tesselateCrossTexture( Tile* tt, int data, float x, float y, float z, float scale );
	void tesselateStemTexture( Tile* tt, int data, float h, float x, float y, float z );
	bool tesselateLilypadInWorld(Tile *tt, int x, int y, int z);
	void tesselateStemDirTexture( StemTile* tt, int data, int dir, float h, float x, float y, float z );

	void tesselateRowTexture( Tile* tt, int data, float x, float y, float z );
	bool tesselateWaterInWorld( Tile* tt, int x, int y, int z );
	private:
	float getWaterHeight( int x, int y, int z, Material* m );
	public:
	void renderBlock( Tile* tt, Level* level, int x, int y, int z );
	void renderBlock(Tile *tt, Level *level, int x, int y, int z, int data);
	bool tesselateBlockInWorld( Tile* tt, int x, int y, int z );
	// AP - added so we can cull earlier
	bool tesselateBlockInWorld( Tile* tt, int x, int y, int z, int faceFlags );
	bool tesselateTreeInWorld(Tile *tt, int x, int y, int z);
	bool tesselateQuartzInWorld(Tile *tt, int x, int y, int z);
	bool tesselateCocoaInWorld(CocoaTile *tt, int x, int y, int z);

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
	// AP - added faceFlags so we can cull earlier
	bool tesselateBlockInWorldWithAmbienceOcclusionTexLighting( Tile* tt, int pX, int pY, int pZ, float pBaseRed,
																float pBaseGreen, float pBaseBlue, int faceFlags );
	private:
	int blend( int a, int b, int c, int def );
	int blend(int a, int b, int c, int d, double fa, double fb, double fc, double fd);
	public:
	bool tesselateBlockInWorld( Tile* tt, int x, int y, int z, float r, float g, float b );
	bool tesselateCactusInWorld( Tile* tt, int x, int y, int z );
	bool tesselateCactusInWorld( Tile* tt, int x, int y, int z, float r, float g, float b );
	bool tesselateFenceInWorld( FenceTile* tt, int x, int y, int z );
	bool tesselateWallInWorld(WallTile *tt, int x, int y, int z);
	bool tesselateEggInWorld(EggTile *tt, int x, int y, int z);
	bool tesselateFenceGateInWorld(FenceGateTile *tt, int x, int y, int z);
	bool tesselateStairsInWorld( StairTile* tt, int x, int y, int z );
	bool tesselateDoorInWorld( Tile* tt, int x, int y, int z );
	void renderFaceUp( Tile* tt, double x, double y, double z, Icon *tex );
	void renderFaceDown( Tile* tt, double x, double y, double z, Icon *tex );
	void renderNorth( Tile* tt, double x, double y, double z, Icon *tex );
	void renderSouth( Tile* tt, double x, double y, double z, Icon *tex );
	void renderWest( Tile* tt, double x, double y, double z, Icon *tex );
	void renderEast( Tile* tt, double x, double y, double z, Icon *tex );
	void renderCube( Tile* tile, float alpha );
	void renderTile( Tile* tile, int data, float brightness, float fAlpha = 1.0f, bool useCompiled = true ); // 4J Added useCompiled
	static bool canRender( int renderShape );
	Icon *getTexture(Tile *tile, LevelSource *level, int x, int y, int z, int face);

	Icon *getTexture(Tile *tile, int face, int data);
	Icon *getTexture(Tile *tile, int face);
	Icon *getTexture(Tile *tile);
	Icon *getTextureOrMissing(Icon *icon);
};
