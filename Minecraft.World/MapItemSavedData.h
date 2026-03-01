#pragma once

#include "Player.h"
#include "SavedData.h"

class MapItemSavedData : public SavedData
{
private:
	static const int HEADER_COLOURS = 0;
	static const int HEADER_DECORATIONS = 1;
	static const int HEADER_METADATA = 2;

public:
	static const int MAP_SIZE = 64;
	static const int MAX_SCALE = 4;

#ifdef _LARGE_WORLDS
	static const int DEC_PACKET_BYTES = 8;
#else
	static const int DEC_PACKET_BYTES = 7;
#endif

	class MapDecoration
	{
	public:
		char img, x, y, rot;
		int entityId; // 4J Added
		bool visible;

		MapDecoration(char img, char x, char y, char rot, int entityId, bool visible); // 4J added entityId, visible param
	};

	class HoldingPlayer
	{
	public:
		const shared_ptr<Player> player;
		intArray rowsDirtyMin;
		intArray rowsDirtyMax;

	private:
		int tick;
		int sendPosTick;
		charArray lastSentDecorations;

	public:
		int step;

	private:
		bool hasSentInitial;

	protected:
		const MapItemSavedData *parent;

	public:
		// 4J Stu - Had to add a reference to the MapItemSavedData object that created us as we try to access it's member variables
		HoldingPlayer(shared_ptr<Player> player, const MapItemSavedData *parent);
		~HoldingPlayer();
		charArray nextUpdatePacket(shared_ptr<ItemInstance> itemInstance);
	};

public:
	int x, z;
	char dimension;
	byte scale;
	byteArray colors;
	vector<shared_ptr<HoldingPlayer> > carriedBy;

private:

	typedef unordered_map<shared_ptr<Player> , shared_ptr<HoldingPlayer> , PlayerKeyHash, PlayerKeyEq> playerHoldingPlayerMapType;
	playerHoldingPlayerMapType carriedByPlayers;

public:
	vector<MapDecoration *> decorations;

private:
	// 4J Stu added
	unordered_map<int, MapDecoration *> nonPlayerDecorations;
	static const int END_PORTAL_DECORATION_KEY;


public:
	MapItemSavedData(const wstring& id);
	~MapItemSavedData();

	virtual void load(CompoundTag *tag);
	virtual void save(CompoundTag *tag);

	void tickCarriedBy(shared_ptr<Player> player, shared_ptr<ItemInstance> item);

	charArray getUpdatePacket(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);

	using SavedData::setDirty;
	void setDirty(int x, int y0, int y1);
	void handleComplexItemData(charArray &data);
	shared_ptr<HoldingPlayer> getHoldingPlayer(shared_ptr<Player> player);

	// 4J Stu Added
	void mergeInMapData(shared_ptr<MapItemSavedData> dataToAdd);
	void removeItemFrameDecoration(shared_ptr<ItemInstance> item);
};
