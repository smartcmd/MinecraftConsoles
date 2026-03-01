#pragma once

class StructurePiece;
class StructureStart;

typedef StructurePiece *(*structurePieceCreateFn)();
typedef StructureStart *(*structureStartCreateFn)();

enum EStructureStart
{
	eStructureStart_MineShaftStart,
	eStructureStart_VillageStart,
	eStructureStart_NetherBridgeStart,
	eStructureStart_StrongholdStart,
	eStructureStart_ScatteredFeatureStart,
};

enum EStructurePiece
{
	eStructurePiece_MineShaftRoom,
	eStructurePiece_MineShaftCorridor,
	eStructurePiece_MineShaftCrossing,
	eStructurePiece_MineShaftStairs,

	eStructurePiece_BridgeStraight,
	eStructurePiece_BridgeEndFiller,
	eStructurePiece_BridgeCrossing,
	eStructurePiece_RoomCrossing,
	eStructurePiece_StairsRoom,
	eStructurePiece_MonsterThrone,
	eStructurePiece_CastleEntrance,
	eStructurePiece_CastleStalkRoom,
	eStructurePiece_CastleSmallCorridorPiece,
	eStructurePiece_CastleSmallCorridorCrossingPiece,
	eStructurePiece_CastleSmallCorridorRightTurnPiece,
	eStructurePiece_CastleSmallCorridorLeftTurnPiece,
	eStructurePiece_CastleCorridorStairsPiece,
	eStructurePiece_CastleCorridorTBalconyPiece,
	eStructurePiece_NetherBridgeStartPiece,

	eStructurePiece_DesertPyramidPiece,
	eStructurePiece_JunglePyramidPiece,
	eStructurePiece_SwamplandHut,

	eStructurePiece_FillerCorridor,
	eStructurePiece_StairsDown,
	eStructurePiece_Straight,
	eStructurePiece_ChestCorridor,
	eStructurePiece_StraightStairsDown,
	eStructurePiece_LeftTurn,
	eStructurePiece_RightTurn,
	eStructurePiece_StrongholdRoomCrossing,
	eStructurePiece_PrisonHall,
	eStructurePiece_Library,
	eStructurePiece_FiveCrossing,
	eStructurePiece_PortalRoom,
	eStructurePiece_StrongholdStartPiece,

	eStructurePiece_Well,
	eStructurePiece_StraightRoad,
	eStructurePiece_SimpleHouse,
	eStructurePiece_SmallTemple,
	eStructurePiece_BookHouse,
	eStructurePiece_SmallHut,
	eStructurePiece_PigHouse,
	eStructurePiece_TwoRoomHouse,
	eStructurePiece_Smithy,
	eStructurePiece_Farmland,
	eStructurePiece_DoubleFarmland,
	eStructurePiece_LightPost,
	eStructurePiece_VillageStartPiece,
};

class StructureFeatureIO
{
private:
	static unordered_map<wstring, structureStartCreateFn> startIdClassMap;
	static unordered_map<unsigned int, wstring> startClassIdMap;

	static unordered_map<wstring, structurePieceCreateFn> pieceIdClassMap;
	static unordered_map<unsigned int, wstring> pieceClassIdMap;

public:
	static void setStartId(EStructureStart clas, structureStartCreateFn createFn, const wstring &id);
	static void setPieceId(EStructurePiece clas, structurePieceCreateFn createFn, const wstring &id);

public:
	static void staticCtor();
	static wstring getEncodeId(StructureStart *start);
	static wstring getEncodeId(StructurePiece *piece);
	static StructureStart *loadStaticStart(CompoundTag *tag, Level *level);
	static StructurePiece *loadStaticPiece(CompoundTag *tag, Level *level);
};