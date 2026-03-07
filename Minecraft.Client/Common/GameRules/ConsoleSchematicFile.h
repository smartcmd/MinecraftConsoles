#pragma once
using namespace std;

#define XBOX_SCHEMATIC_ORIGINAL_VERSION 1
#define XBOX_SCHEMATIC_CURRENT_VERSION 2

#include "../../../Minecraft.World/ArrayWithLength.h"

class Level;
class DataOutputStream;
class DataInputStream;
class TileEntity;
class LevelChunk;
class AABB;
class Vec3;

class ConsoleSchematicFile
{
public:
	enum ESchematicRotation
	{
		eSchematicRot_0,
		eSchematicRot_90,
		eSchematicRot_180,
		eSchematicRot_270
	};
private:
	int m_refCount;

public:
	void incrementRefCount()	{ ++m_refCount; }
	void decrementRefCount()	{ --m_refCount; }
	bool shouldDelete()			{ return m_refCount <= 0; }

	typedef struct _XboxSchematicInitParam
	{
		wchar_t name[64];
		int startX;
		int startY;
		int startZ;
		int endX;
		int endY;
		int endZ;
		bool bSaveMobs;

		Compression::ECompressionTypes compressionType;

		_XboxSchematicInitParam()
		{
			ZeroMemory(name,64*(sizeof(wchar_t)));
			startX = startY = startZ = endX = endY = endZ = 0;
			bSaveMobs = false;
			compressionType = Compression::eCompressionType_None;
		}
	} XboxSchematicInitParam;
private:
	int m_xSize, m_ySize, m_zSize;
	vector<shared_ptr<TileEntity> > m_tileEntities;
	vector< pair<Vec3 *, CompoundTag *> > m_entities;

public:
	byteArray m_data;

public:
	ConsoleSchematicFile();
	~ConsoleSchematicFile();

	int getXSize() { return m_xSize; }
	int getYSize() { return m_ySize; }
	int getZSize() { return m_zSize; }

	void save(DataOutputStream *dos);
	void load(DataInputStream *dis);

	int64_t applyBlocksAndData(LevelChunk *chunk, AABB *chunkBox, AABB *destinationBox, ESchematicRotation rot);
	int64_t applyLighting(LevelChunk *chunk, AABB *chunkBox, AABB *destinationBox, ESchematicRotation rot);
	void applyTileEntities(LevelChunk *chunk, AABB *chunkBox, AABB *destinationBox, ESchematicRotation rot);

	static void generateSchematicFile(DataOutputStream *dos, Level *level, int xStart, int yStart, int zStart, int xEnd, int yEnd, int zEnd, bool bSaveMobs, Compression::ECompressionTypes);
	static void setBlocksAndData(LevelChunk *chunk, byteArray blockData, byteArray dataData, byteArray data, int x0, int y0, int z0, int x1, int y1, int z1, int &blocksP, int &dataP, int &blockLightP, int &skyLightP);
private:
	void save_tags(DataOutputStream *dos);
	void load_tags(DataInputStream *dis);

	static void getBlocksAndData(LevelChunk *chunk, byteArray *data, int x0, int y0, int z0, int x1, int y1, int z1, int &blocksP, int &dataP, int &blockLightP, int &skyLightP);
	static vector<shared_ptr<TileEntity> > *getTileEntitiesInRegion(LevelChunk *chunk, int x0, int y0, int z0, int x1, int y1, int z1);

	void chunkCoordToSchematicCoord(AABB *destinationBox, int chunkX, int chunkZ, ESchematicRotation rot, int &schematicX, int &schematicZ);
	void schematicCoordToChunkCoord(AABB *destinationBox, double schematicX, double schematicZ, ESchematicRotation rot, double &chunkX, double &chunkZ);
};