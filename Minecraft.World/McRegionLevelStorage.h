#pragma once
using namespace std;

#include "DirectoryLevelStorage.h"

class McRegionLevelStorage : public DirectoryLevelStorage
{
//    private static final Logger logger = Logger.getLogger("Minecraft");

	friend class McRegionLevelStorageSource; // 4J Jev, needs access to protected members.

protected:
	static const int MCREGION_VERSION_ID = 0x4abc;

public:
	McRegionLevelStorage(ConsoleSaveFile *saveFile, File dir, const wstring& levelName, bool createPlayerDir);
	~McRegionLevelStorage();

    virtual ChunkStorage *createChunkStorage(Dimension *dimension);
    virtual void saveLevelData(LevelData *levelData, vector<shared_ptr<Player> > *players);
    virtual void closeAll();
};