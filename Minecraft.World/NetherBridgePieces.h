#pragma once
#include "StructurePiece.h"

class NetherBridgePieces
{
private:
	static const int MAX_DEPTH = 30;
	// the dungeon starts at 64 and traverses downwards to this point
	static const int LOWEST_Y_POSITION = 10;

	// 4J - added to replace use of Class<? extends NetherBridgePiece> within this class
	enum EPieceClass
	{
		EPieceClass_BridgeStraight,
		EPieceClass_BridgeEndFiller,
		EPieceClass_BridgeCrossing,
		EPieceClass_RoomCrossing,
		EPieceClass_StairsRoom,
		EPieceClass_MonsterThrone,
		EPieceClass_CastleEntrance,
		EPieceClass_CastleStalkRoom,
		EPieceClass_CastleSmallCorridorPiece,
		EPieceClass_CastleSmallCorridorCrossingPiece,
		EPieceClass_CastleSmallCorridorRightTurnPiece,
		EPieceClass_CastleSmallCorridorLeftTurnPiece,
		EPieceClass_CastleCorridorStairsPiece,
		EPieceClass_CastleCorridorTBalconyPiece
	};

public:
	static void loadStatic();

private:
	class PieceWeight
	{
	public:
		EPieceClass pieceClass;
		const int weight;
		int placeCount;
		int maxPlaceCount;
		bool allowInRow;

		PieceWeight(EPieceClass pieceClass, int weight, int maxPlaceCount, bool allowInRow);
		PieceWeight(EPieceClass pieceClass, int weight, int maxPlaceCount);
		bool doPlace(int depth);
		bool isValid();
	};

	static const int BRIDGE_PIECEWEIGHTS_COUNT = 6;
	static const int CASTLE_PIECEWEIGHTS_COUNT = 7;
	static NetherBridgePieces::PieceWeight *bridgePieceWeights[BRIDGE_PIECEWEIGHTS_COUNT];
	static NetherBridgePieces::PieceWeight *castlePieceWeights[CASTLE_PIECEWEIGHTS_COUNT];

private:
	class NetherBridgePiece;
	static NetherBridgePiece *findAndCreateBridgePieceFactory(NetherBridgePieces::PieceWeight *piece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth);

	/**
	* 
	*
	*/
public:
	class StartPiece;
private:

	class NetherBridgePiece : public StructurePiece
	{
	protected:
		static const int FORTRESS_TREASURE_ITEMS_COUNT = 11;
		static WeighedTreasure *fortressTreasureItems[FORTRESS_TREASURE_ITEMS_COUNT];

	public:
		NetherBridgePiece();

	protected:
		NetherBridgePiece(int genDepth);

		virtual void readAdditonalSaveData(CompoundTag *tag);
		virtual void addAdditonalSaveData(CompoundTag *tag);

	private:
		int updatePieceWeight(list<PieceWeight *> *currentPieces);

		NetherBridgePiece *generatePiece(StartPiece *startPiece, list<NetherBridgePieces::PieceWeight *> *currentPieces, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth);
		StructurePiece *generateAndAddPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth, bool isCastle);
	protected:
		StructurePiece *generateChildForward(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int xOff, int yOff, bool isCastle);
		StructurePiece *generateChildLeft(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int yOff, int zOff, bool isCastle);
		StructurePiece *generateChildRight(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int yOff, int zOff, bool isCastle);

		static bool isOkBox(BoundingBox *box, StartPiece *startRoom); // 4J added startRoom param
		void generateLightPost(Level *level, Random *random, BoundingBox *chunkBB, int x, int y, int z, int xOff, int zOff);

		void generateLightPostFacingRight(Level *level, Random *random, BoundingBox *chunkBB, int x, int y, int z);
		void generateLightPostFacingLeft(Level *level, Random *random, BoundingBox *chunkBB, int x, int y, int z);
		void generateLightPostFacingUp(Level *level, Random *random, BoundingBox *chunkBB, int x, int y, int z);
		void generateLightPostFacingDown(Level *level, Random *random, BoundingBox *chunkBB, int x, int y, int z);
	};

	/**
	* 
	*
	*/
	class BridgeStraight : public NetherBridgePiece
	{
	public:
		static StructurePiece *Create() { return new BridgeStraight(); }
		virtual EStructurePiece GetType() { return eStructurePiece_BridgeStraight; }

	private:
		static const int width = 5;
		static const int height = 10;
		static const int depth = 19;

	public:
		BridgeStraight();
		BridgeStraight(int genDepth, Random *random, BoundingBox *stairsBox, int direction);
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		static BridgeStraight *createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

	class BridgeEndFiller : public NetherBridgePiece
	{
	public:
		static StructurePiece *Create() { return new BridgeEndFiller(); }
		virtual EStructurePiece GetType() { return eStructurePiece_BridgeEndFiller; }

	private:
		static const int width = 5;
		static const int height = 10;
		static const int depth = 8;

		int selfSeed;

	public:
		BridgeEndFiller();
		BridgeEndFiller(int genDepth, Random *random, BoundingBox *stairsBox, int direction);

		static BridgeEndFiller *createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);

	protected:
		void readAdditonalSaveData(CompoundTag *tag);
		void addAdditonalSaveData(CompoundTag *tag);
	};

	class BridgeCrossing : public NetherBridgePiece
	{
	public:
		static StructurePiece *Create() { return new BridgeCrossing(); }
		virtual EStructurePiece GetType() { return eStructurePiece_BridgeCrossing; }

	private:
		static const int width = 19;
		static const int height = 10;
		static const int depth = 19;

	public:
		BridgeCrossing();
		BridgeCrossing(int genDepth, Random *random, BoundingBox *stairsBox, int direction);
	protected:
		BridgeCrossing(Random *random, int west, int north);
	public:
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		static BridgeCrossing *createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

public:
	class StartPiece : public BridgeCrossing
	{
	public:
		virtual EStructurePiece GetType() { return eStructurePiece_NetherBridgeStartPiece; }

	public:
		PieceWeight *previousPiece;
		Level *m_level;

		list<PieceWeight *> availableBridgePieces;
		list<PieceWeight *> availableCastlePieces;

		// this queue is used so that the addChildren calls are
		// called in a random order
		vector<StructurePiece *> pendingChildren;

		StartPiece();
		StartPiece(Random *random, int west, int north, Level *level); // 4J Added level param

	protected:
		virtual void readAdditonalSaveData(CompoundTag *tag);
		virtual void addAdditonalSaveData(CompoundTag *tag);
	};

private:
	class RoomCrossing : public NetherBridgePiece
	{
	public:
		static StructurePiece *Create() { return new RoomCrossing(); }
		virtual EStructurePiece GetType() { return eStructurePiece_RoomCrossing; }

	private:
		static const int width = 7;
		static const int height = 9;
		static const int depth = 7;

	public:
		RoomCrossing();
		RoomCrossing(int genDepth, Random *random, BoundingBox *box, int direction);
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		static RoomCrossing *createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

	class StairsRoom : public NetherBridgePiece
	{
	public:
		static StructurePiece *Create() { return new StairsRoom(); }
		virtual EStructurePiece GetType() { return eStructurePiece_StairsRoom; }

	private:
		static const int width = 7;
		static const int height = 11;
		static const int depth = 7;

	public:
		StairsRoom();
		StairsRoom(int genDepth, Random *random, BoundingBox *box, int direction);
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		static StairsRoom *createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

	class MonsterThrone : public NetherBridgePiece
	{
	public:
		static StructurePiece *Create() { return new MonsterThrone(); }
		virtual EStructurePiece GetType() { return eStructurePiece_MonsterThrone; }

	private:
		static const int width = 7;
		static const int height = 8;
		static const int depth = 9;

		bool hasPlacedMobSpawner;

	public:
		MonsterThrone();
		MonsterThrone(int genDepth, Random *random, BoundingBox *box, int direction);

	protected:
		virtual void readAdditonalSaveData(CompoundTag *tag);
		virtual void addAdditonalSaveData(CompoundTag *tag);

	public:
		static MonsterThrone *createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

	/**
	* 
	*
	*/
	class CastleEntrance : public NetherBridgePiece
	{
	public:
		static StructurePiece *Create() { return new CastleEntrance(); }
		virtual EStructurePiece GetType() { return eStructurePiece_CastleEntrance; }

	private:
		static const int width = 13;
		static const int height = 14;
		static const int depth = 13;
	public:
		CastleEntrance();
		CastleEntrance(int genDepth, Random *random, BoundingBox *stairsBox, int direction);
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		static CastleEntrance *createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

	/**
	* 
	*
	*/
	class CastleStalkRoom : public NetherBridgePiece
	{
	public:
		static StructurePiece *Create() { return new CastleStalkRoom(); }
		virtual EStructurePiece GetType() { return eStructurePiece_CastleStalkRoom; }

	private:
		static const int width = 13;
		static const int height = 14;
		static const int depth = 13;

	public:
		CastleStalkRoom();
		CastleStalkRoom(int genDepth, Random *random, BoundingBox *stairsBox, int direction);
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		static CastleStalkRoom *createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

	/**
	* 
	*
	*/
	class CastleSmallCorridorPiece : public NetherBridgePiece
	{
	public:
		static StructurePiece *Create() { return new CastleSmallCorridorPiece(); }
		virtual EStructurePiece GetType() { return eStructurePiece_CastleSmallCorridorPiece; }

	private:
		static const int width = 5;
		static const int height = 7;
		static const int depth = 5;

	public:
		CastleSmallCorridorPiece();
		CastleSmallCorridorPiece(int genDepth, Random *random, BoundingBox *stairsBox, int direction);
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		static CastleSmallCorridorPiece *createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

	/**
	* 
	*
	*/
	class CastleSmallCorridorCrossingPiece : public NetherBridgePiece
	{
	public:
		static StructurePiece *Create() { return new CastleSmallCorridorCrossingPiece(); }
		virtual EStructurePiece GetType() { return eStructurePiece_CastleSmallCorridorCrossingPiece; }

	private:
		static const int width = 5;
		static const int height = 7;
		static const int depth = 5;

	public:
		CastleSmallCorridorCrossingPiece();
		CastleSmallCorridorCrossingPiece(int genDepth, Random *random, BoundingBox *stairsBox, int direction);
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		static CastleSmallCorridorCrossingPiece *createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

	/**
	* 
	*
	*/
	class CastleSmallCorridorRightTurnPiece : public NetherBridgePiece
	{
	public:
		static StructurePiece *Create() { return new CastleSmallCorridorRightTurnPiece(); }
		virtual EStructurePiece GetType() { return eStructurePiece_CastleSmallCorridorRightTurnPiece; }

	private:
		static const int width = 5;
		static const int height = 7;
		static const int depth = 5;

		bool isNeedingChest;

	public:
		CastleSmallCorridorRightTurnPiece();
		CastleSmallCorridorRightTurnPiece(int genDepth, Random *random, BoundingBox *stairsBox, int direction);

	protected:
		virtual void readAdditonalSaveData(CompoundTag *tag);
		virtual void addAdditonalSaveData(CompoundTag *tag);

	public:
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		static CastleSmallCorridorRightTurnPiece *createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);

	};

	/**
	* 
	*
	*/
	class CastleSmallCorridorLeftTurnPiece : public NetherBridgePiece
	{
	public:
		static StructurePiece *Create() { return new CastleSmallCorridorLeftTurnPiece(); }
		virtual EStructurePiece GetType() { return eStructurePiece_CastleSmallCorridorLeftTurnPiece; }

	private:
		static const int width = 5;
		static const int height = 7;
		static const int depth = 5;
		bool isNeedingChest;

	public:
		CastleSmallCorridorLeftTurnPiece();
		CastleSmallCorridorLeftTurnPiece(int genDepth, Random *random, BoundingBox *stairsBox, int direction);

	protected:
		virtual void readAdditonalSaveData(CompoundTag *tag);
		virtual void addAdditonalSaveData(CompoundTag *tag);

	public:
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		static CastleSmallCorridorLeftTurnPiece *createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

	/**
	* 
	*
	*/
	class CastleCorridorStairsPiece : public NetherBridgePiece
	{
	public:
		static StructurePiece *Create() { return new CastleCorridorStairsPiece(); }
		virtual EStructurePiece GetType() { return eStructurePiece_CastleCorridorStairsPiece; }

	private:
		static const int width = 5;
		static const int height = 14;
		static const int depth = 10;

	public:
		CastleCorridorStairsPiece();
		CastleCorridorStairsPiece(int genDepth, Random *random, BoundingBox *stairsBox, int direction);
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		static CastleCorridorStairsPiece *createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);

	};

	/**
	* 
	*
	*/
	class CastleCorridorTBalconyPiece : public NetherBridgePiece
	{
	public:
		static StructurePiece *Create() { return new CastleCorridorTBalconyPiece(); }
		virtual EStructurePiece GetType() { return eStructurePiece_CastleCorridorTBalconyPiece; }

	private:
		static const int width = 9;
		static const int height = 7;
		static const int depth = 9;

	public:
		CastleCorridorTBalconyPiece();
		CastleCorridorTBalconyPiece(int genDepth, Random *random, BoundingBox *stairsBox, int direction);
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		static CastleCorridorTBalconyPiece *createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};
};
