#pragma once

#include "StructurePiece.h"

class MineShaftPieces
{
private:
	static const int DEFAULT_SHAFT_WIDTH = 3;
	static const int DEFAULT_SHAFT_HEIGHT = 3;
	static const int DEFAULT_SHAFT_LENGTH = 5;

	static const int MAX_DEPTH = 8; // 1.2.3 change

public:
	static void loadStatic();

private:
	static StructurePiece *createRandomShaftPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
	static StructurePiece *generateAndAddPiece(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth);

	/**
	* 
	*
	*/
public:
	class MineShaftRoom : public StructurePiece
	{
	public:
		static StructurePiece *Create() { return new MineShaftRoom(); }
		virtual EStructurePiece GetType() { return eStructurePiece_MineShaftRoom; }

	private:
		list<BoundingBox *> childEntranceBoxes;

	public:
		MineShaftRoom();
		MineShaftRoom(int genDepth, Random *random, int west, int north);
		~MineShaftRoom();

		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);

	protected:
		void addAdditonalSaveData(CompoundTag *tag);
		void readAdditonalSaveData(CompoundTag *tag);
	};

	/**
	* 
	*
	*/
	class MineShaftCorridor : public StructurePiece
	{
	public:
		static StructurePiece *Create() { return new MineShaftCorridor(); }
		virtual EStructurePiece GetType() { return eStructurePiece_MineShaftCorridor; }

	private:
		bool hasRails; // was final
		bool spiderCorridor; // was final
		bool hasPlacedSpider;
		int numSections;

	public:
		MineShaftCorridor();

	protected:
		void addAdditonalSaveData(CompoundTag *tag);
		void readAdditonalSaveData(CompoundTag *tag);

	public:
		MineShaftCorridor(int genDepth, Random *random, BoundingBox *corridorBox, int direction);

		static BoundingBox *findCorridorSize(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction);
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);

	protected:
		virtual bool createChest(Level *level, BoundingBox *chunkBB, Random *random, int x, int y, int z, WeighedTreasureArray treasure, int numRolls);

	public:
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

	/**
	* 
	*
	*/
	class MineShaftCrossing : public StructurePiece
	{
	public:
		static StructurePiece *Create() { return new MineShaftCrossing(); }
		virtual EStructurePiece GetType() { return eStructurePiece_MineShaftCrossing; }

	private:
		int direction;
		bool isTwoFloored;

	public:
		MineShaftCrossing();

	protected:
		void addAdditonalSaveData(CompoundTag *tag);
		void readAdditonalSaveData(CompoundTag *tag);

	public:
		MineShaftCrossing(int genDepth, Random *random, BoundingBox *crossingBox, int direction);

		static BoundingBox *findCrossing(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction);
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

	/**
	* 
	*
	*/
	class MineShaftStairs : public StructurePiece
	{
	public:
		static StructurePiece *Create() { return new MineShaftStairs(); }
		virtual EStructurePiece GetType() { return eStructurePiece_MineShaftStairs; }

	public:
		MineShaftStairs();
		MineShaftStairs(int genDepth, Random *random, BoundingBox *stairsBox, int direction);

	protected:
		void addAdditonalSaveData(CompoundTag *tag);
		void readAdditonalSaveData(CompoundTag *tag);

	public:
		static BoundingBox *findStairs(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction);
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);

	};

	/* @formatter:off */
private:
	static WeighedTreasureArray smallTreasureItems;
	/* @formatter:on */

public:
	static void staticCtor();

};