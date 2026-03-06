#include "stdafx.h"
#include "LevelSummary.h"

LevelSummary::LevelSummary(const wstring& levelId, const wstring& levelName, int64_t lastPlayed, int64_t sizeOnDisk,  GameType *gameMode, bool requiresConversion, bool hardcore, bool hasCheats) :
	levelId( levelId ),
	levelName( levelName ),
	lastPlayed( lastPlayed ),
	sizeOnDisk( sizeOnDisk ),
	gameMode( gameMode ),
	requiresConversion( requiresConversion ),
	hardcore( hardcore ),
	_hasCheats( hasCheats )
{
}

wstring LevelSummary::getLevelId()
{
	return levelId;
}

wstring LevelSummary::getLevelName()
{
	return levelName;
}

int64_t LevelSummary::getSizeOnDisk()
{
	return sizeOnDisk;
}

bool LevelSummary::isRequiresConversion()
{
	return requiresConversion;
}

int64_t LevelSummary::getLastPlayed()
{
	return lastPlayed;
}

int LevelSummary::compareTo(LevelSummary *rhs)
{
	if (lastPlayed < rhs->lastPlayed)
	{
		return 1;
	}
	if (lastPlayed > rhs->lastPlayed)
	{
		return -1;
	}

	// TODO 4J Jev, used to be compareTo in java, is this right?
	return levelId.compare(rhs->levelId);
}

GameType *LevelSummary::getGameMode()
{
	return gameMode;
}

bool LevelSummary::isHardcore()
{
	return hardcore;
}

bool LevelSummary::hasCheats()
{
	return _hasCheats;
}