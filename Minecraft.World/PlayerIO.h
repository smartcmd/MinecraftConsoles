#pragma once
using namespace std;

// If we have more than MAX_PLAYER_DATA_SAVES player.dat's then we delete the oldest ones
// This value can be no higher than MAXIMUM_MAP_SAVE_DATA/3 (3 being the number of dimensions in future versions)
#define MAX_PLAYER_DATA_SAVES 0x7FFFFFFF

class Player;

class PlayerIO 
{
public:
	virtual void save(shared_ptr<Player> player) = 0;
	virtual CompoundTag *load(shared_ptr<Player> player) = 0;
	virtual CompoundTag *loadPlayerDataTag(PlayerUID xuid) = 0; // 4J Changed from string name to xuid

	// 4J Added
	virtual void clearOldPlayerFiles() = 0;
	virtual void saveMapIdLookup() = 0;
	virtual void deleteMapFilesForPlayer(shared_ptr<Player> player) = 0;
	virtual void saveAllCachedData() = 0;
};