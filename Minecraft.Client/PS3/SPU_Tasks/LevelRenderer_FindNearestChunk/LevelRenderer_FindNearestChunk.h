#pragma once



class LevelRenderer_FindNearestChunk_DataIn
{
public:
	class PlayerData
	{
	public:
		bool bValid;
		double x,y,z;
	};

	class Chunk;
	class ClipChunk
	{
	public:
		Chunk *chunk;
		int globalIdx;
		bool visible;
		float aabb[6];
		int xm, ym, zm;
	};

	class AABB 
	{
		double x0, y0, z0;
		double x1, y1, z1;
	};
	class MultiplayerChunkCache
	{
	public:
		int XZSIZE;
		int XZOFFSET;
 		void** cache;

		bool getChunkEmpty(int lowerOffset, int upperOffset, int x, int y, int z);
	};

	class CompressedTileStorage
	{
	public:
		unsigned char	*indicesAndData;
		int				allocatedSize;

		bool isRenderChunkEmpty(int y);				// Determine if 16x16x16 render-sized chunk is actually empty
	private:
		static void getBlock(int *block, int x, int y, int z) {	*block = ( ( x  & 0x0c ) << 5 ) | ( ( z & 0x0c ) << 3 ) | ( y >> 2 );	}

	};

	class Chunk
	{
	public:
		void *level;
		int x, y, z;
		int xRender, yRender, zRender;
		int xRenderOffs, yRenderOffs, zRenderOffs;
		int xm, ym, zm;
		AABB *bb;
		ClipChunk *clipChunk;

		int id;
		int padding[1];
		//public:
		//	vector<shared_ptr<TileEntity> > renderableTileEntities;		// 4J - removed

	private:
		void	*globalRenderableTileEntities;
		void	*globalRenderableTileEntities_cs;
		bool	assigned;
	};


	static const int CHUNK_SIZE = 16;
	static const int CHUNK_Y_COUNT = 256 / CHUNK_SIZE;

	int numGlobalChunks;
	unsigned char* pGlobalChunkFlags;

	bool onlyRebuild;
	LevelRenderer_FindNearestChunk_DataIn::ClipChunk*	chunks[4];
	int				chunkLengths[4];
	void* level[4];
	MultiplayerChunkCache multiplayerChunkCache[4];

	int lowerOffset;		// offsets into the level class, we don't want to compile the entire class
	int upperOffset; 

	int xChunks, yChunks, zChunks;

	PlayerData playerData[4];
	LevelRenderer_FindNearestChunk_DataIn::ClipChunk* nearChunk;
	int veryNearCount;
	int padding[2];

#ifdef SN_TARGET_PS3_SPU
	void findNearestChunk();
#endif
};
