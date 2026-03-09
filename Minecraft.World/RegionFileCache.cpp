#include "stdafx.h"
#include "File.h"
#include "RegionFileCache.h"
#include "ConsoleSaveFileIO.h"

RegionFileCache RegionFileCache::s_defaultCache;

bool RegionFileCache::useSplitSaves(ESavePlatform platform)
{
	switch(platform)
	{
	case SAVE_FILE_PLATFORM_XBONE:
	case SAVE_FILE_PLATFORM_PS4:
		return true;
	default:
		return false;
	};
}

RegionFile *RegionFileCache::_getRegionFile(ConsoleSaveFile *saveFile, const wstring &prefix, int chunkX, int chunkZ)		// 4J - TODO was synchronized
{
	// 4J Jev - changed back to use of the File class.
	//char file[MAX_PATH_SIZE];
	//sprintf(file,"%s\\region\\r.%d.%d.mcr",basePath,chunkX >> 5,chunkZ >> 5);

	//File regionDir(basePath, L"region");

	//File file(regionDir, wstring(L"r.") + std::to_wstring(chunkX>>5) + L"." + std::to_wstring(chunkZ>>5) + L".mcr" );
	MemSect(31);
	File file;
	if(useSplitSaves(saveFile->getSavePlatform()))
	{
		file = File( prefix + wstring(L"r.") + std::to_wstring(chunkX>>4) + L"." + std::to_wstring(chunkZ>>4) + L".mcr" );
	}
	else
	{
		file = File( prefix + wstring(L"r.") + std::to_wstring(chunkX>>5) + L"." + std::to_wstring(chunkZ>>5) + L".mcr" );
	}
	MemSect(0);

	RegionFile *ref = nullptr;
    auto it = cache.find(file);
    if( it != cache.end() )
		ref = it->second;

	// 4J Jev, put back in.
    if (ref != nullptr)
	{
		return ref;
    }

	// 4J Stu - Remove for new save files
	/*
    if (!regionDir.exists())
	{
        regionDir.mkdirs();
    }
	*/
    if (cache.size() >= MAX_CACHE_SIZE)
	{
        _clear();
    }

	RegionFile *reg = new RegionFile(saveFile, &file);
    cache[file] = reg;	   // 4J - this was originally a softReferenc
    return reg;

}

void RegionFileCache::_clear()															// 4J - TODO was synchronized
{
	for(auto& it : cache)
	{
        RegionFile *regionFile = it.second;
        if (regionFile != nullptr)
		{
            regionFile->close();
        }
		delete regionFile;
	}
	cache.clear();
}

int RegionFileCache::_getSizeDelta(ConsoleSaveFile *saveFile, const wstring &prefix, int chunkX, int chunkZ)
{
    RegionFile *r = _getRegionFile(saveFile, prefix, chunkX, chunkZ);
    return r->getSizeDelta();
}

DataInputStream *RegionFileCache::_getChunkDataInputStream(ConsoleSaveFile *saveFile, const wstring &prefix, int chunkX, int chunkZ)
{
	RegionFile* r = _getRegionFile(saveFile, prefix, chunkX, chunkZ);
	if(useSplitSaves(saveFile->getSavePlatform()))
	{
		return r->getChunkDataInputStream(chunkX & 15, chunkZ & 15);
	}
	else
	{

		return r->getChunkDataInputStream(chunkX & 31, chunkZ & 31);
	}
}

DataOutputStream *RegionFileCache::_getChunkDataOutputStream(ConsoleSaveFile *saveFile, const wstring &prefix, int chunkX, int chunkZ)
{
	RegionFile* r = _getRegionFile(saveFile, prefix, chunkX, chunkZ);
	if(useSplitSaves(saveFile->getSavePlatform()))
	{
		return r->getChunkDataOutputStream(chunkX & 15, chunkZ & 15);
	}
	else
	{

		return r->getChunkDataOutputStream(chunkX & 31, chunkZ & 31);
	}
}


RegionFileCache::~RegionFileCache()
{
	_clear();
}
