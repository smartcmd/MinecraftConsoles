#pragma once
using namespace std;

class GameType;

class LevelSummary
{
	const wstring levelId;
    const wstring levelName;
    const int64_t lastPlayed;
    const int64_t sizeOnDisk;
    const bool requiresConversion;
	GameType *gameMode;
	const bool hardcore;
	const bool _hasCheats;

public:
	LevelSummary(const wstring& levelId, const wstring& levelName, int64_t lastPlayed, int64_t sizeOnDisk,  GameType *gameMode, bool requiresConversion, bool hardcore, bool hasCheats);
    wstring getLevelId();
    wstring getLevelName();
    int64_t getSizeOnDisk();
    bool isRequiresConversion();
    int64_t getLastPlayed();
    int compareTo(LevelSummary *rhs);
	GameType *getGameMode();
	bool isHardcore();
	bool hasCheats();
};
