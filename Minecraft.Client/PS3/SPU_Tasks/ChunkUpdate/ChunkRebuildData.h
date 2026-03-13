
#pragma once
#ifndef SN_TARGET_PS3_SPU
#include "../../../stdafx.h"
#include "../../../../Minecraft.World/Region.h"
#include "../../../Tesselator.h"
#include "../../../../Minecraft.World/LightLayer.h"
#endif // SN_TARGET_PS3_SPU

#include "../../../../Minecraft.World/LightLayer.h"
#include "Tile_SPU.h"
#include "Tesselator_SPU.h"

class TileRenderer_SPU;



class ChunkRebuildData
{
public:
	enum tileFlags
	{
		e_flag_NoRender = 0x01,
		e_flag_SPURenderCodeMissing = 0x02
	};

	enum chunkFlags
	{
		e_flag_EmptyChunk = 0x01,
		e_flag_HasCeiling = 0x02,
		e_flag_TouchedSky = 0x04,
		e_flag_Rendered = 0x08
	};

// private:
	static const int sc_size = 20;
	unsigned char	m_tileIds[sc_size*sc_size*sc_size];			// byte
	unsigned char	m_brightness[sc_size*sc_size*sc_size];		// 2x 4bit
	unsigned char	m_data_flags[sc_size*sc_size*sc_size];		// 2x 4bit
	int				m_grassColor[sc_size*sc_size];
	int				m_foliageColor[sc_size*sc_size];
	int				m_waterColor[sc_size*sc_size];

	TileData_SPU		m_tileData;
	Tesselator_SPU		m_tesselator;

#ifdef SN_TARGET_PS3_SPU
	void*			m_pRegion;
#else
	Region*			m_pRegion;
#endif // SN_TARGET_PS3_SPU
	int				m_x0;
	int				m_y0;
	int				m_z0;
	float			m_brightnessRamp[16];
	int				m_levelSkyDarken;
	unsigned int	m_flags;
	unsigned int	m_currentLayer;
	int m_lastHitBlockX;
	int m_lastHitBlockY;
	int m_lastHitBlockZ;
 	unsigned int	m_pad[3];// padding to 16 byte alignment


	int getTileIdx(int x, int y, int z) 	{	return (( x - m_x0 )*sc_size*sc_size) + (( y - m_y0 )*sc_size) + ( z - m_z0 );	}
	int getTileIdx(int x, int z)			{	return (( x - m_x0 )*sc_size) + ( z - m_z0 );	}

 	void buildTile(int x, int y, int z);
	void disableUnseenTiles();
	void createTileData();
public:

#if 0 //def SN_TARGET_PS3_SPU
	int getTile(int x, int y, int z);
	int getBrightnessSky(int x, int y, int z);
	int getBrightnessBlock(int x, int y, int z);
	int getData(int x, int y, int z);
	int getFlags(int x, int y, int z);

	void setFlag(int x, int y, int z, int flag);
	int getGrassColor(int x, int z);
	int getFoliageColor(int x, int z);

#else
	int getTile( int x, int y, int z )				{	return m_tileIds[getTileIdx(x,y,z)]; }
	int getBrightnessSky(int x, int y, int z)		{	return m_brightness[getTileIdx(x,y,z)] & 0x0f; }
	int getBrightnessBlock(int x, int y, int z)		{	return m_brightness[getTileIdx(x,y,z)] >> 4; }
	int getData(int x, int y, int z)				{	return m_data_flags[getTileIdx(x,y,z)] & 0x0f; }
	int getFlags(int x, int y, int z)				{	return m_data_flags[getTileIdx(x,y,z)] >> 4; }

	void setFlag(int x, int y, int z, int flag)		{	m_data_flags[getTileIdx(x,y,z)] |= (flag<<4); }
	int getGrassColor( int x, int z )				{	return m_grassColor[getTileIdx(x,z)]; }
	int getFoliageColor( int x, int z )				{	return m_foliageColor[getTileIdx(x,z)]; }
	int getWaterColor( int x, int z )				{	return m_waterColor[getTileIdx(x,z)]; }
#endif 


#ifndef SN_TARGET_PS3_SPU
	void buildMaterials();
	void buildMaterial(int matSPUIndex, Material* mat);
	int getMaterialID(Tile* pTile);
	void buildForChunk(Region* region, Level* level, int x0, int y0, int z0);
	void copyFromTesselator();
	void storeInTesselator();
#endif 
	bool isEmptyTile(int x, int y, int z);
	bool isEmptyChunk() { return m_flags & e_flag_EmptyChunk;}
	bool isSolidRenderTile(int x, int y, int z);
	bool isSolidBlockingTile(int x, int y, int z);
	float getBrightness(int x, int y, int z, int emitt);
	float getBrightness(int x, int y, int z);

	// 4J - changes brought forward from 1.8.2
	int getBrightness(LightLayer::variety layer, int x, int y, int z);
	int getBrightnessPropagate(LightLayer::variety layer, int x, int y, int z);

	int getLightColor(int x, int y, int z, int emitt);	// 4J - change brought forward from 1.8.2
	int getRawBrightness(int x, int y, int z);
	int getRawBrightness(int x, int y, int z, bool propagate);

	int LevelChunk_getRawBrightness(int x, int y, int z, int skyDampen);
	Material_SPU* getMaterial(int x, int y, int z);


	void tesselateAllTiles(TileRenderer_SPU* pTileRenderer);

};


