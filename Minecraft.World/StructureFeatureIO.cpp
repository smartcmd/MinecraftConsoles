#include "stdafx.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "StructureFeatureIO.h"

unordered_map<wstring, structureStartCreateFn> StructureFeatureIO::startIdClassMap;
unordered_map<unsigned int, wstring> StructureFeatureIO::startClassIdMap;

unordered_map<wstring, structurePieceCreateFn> StructureFeatureIO::pieceIdClassMap;
unordered_map<unsigned int, wstring> StructureFeatureIO::pieceClassIdMap;

void StructureFeatureIO::setStartId(EStructureStart clas, structureStartCreateFn createFn, const wstring &id)
{
	startIdClassMap[id] = createFn;
	startClassIdMap[clas] = id;
}

void StructureFeatureIO::setPieceId(EStructurePiece clas, structurePieceCreateFn createFn, const wstring &id)
{
	pieceIdClassMap[id] = createFn;
	pieceClassIdMap[clas] = id;
}

void StructureFeatureIO::staticCtor()
{
	setStartId(eStructureStart_MineShaftStart, MineShaftStart::Create, L"Mineshaft");
	setStartId(eStructureStart_VillageStart, VillageFeature::VillageStart::Create, L"Village");
	setStartId(eStructureStart_NetherBridgeStart, NetherBridgeFeature::NetherBridgeStart::Create, L"Fortress");
	setStartId(eStructureStart_StrongholdStart, StrongholdFeature::StrongholdStart::Create, L"Stronghold");
	setStartId(eStructureStart_ScatteredFeatureStart, RandomScatteredLargeFeature::ScatteredFeatureStart::Create, L"Temple");

	MineShaftPieces::loadStatic();
	VillagePieces::loadStatic();
	NetherBridgePieces::loadStatic();
	StrongholdPieces::loadStatic();
	ScatteredFeaturePieces::loadStatic();
}

wstring StructureFeatureIO::getEncodeId(StructureStart *start)
{
    auto it = startClassIdMap.find(start->GetType());
    if(it != startClassIdMap.end())
	{
		return it->second;
	}
	else
	{
		return L"";
	}
}

wstring StructureFeatureIO::getEncodeId(StructurePiece *piece)
{
    auto it = pieceClassIdMap.find(piece->GetType());
    if(it != pieceClassIdMap.end())
	{
		return it->second;
	}
	else
	{
		return L"";
	}
}

StructureStart *StructureFeatureIO::loadStaticStart(CompoundTag *tag, Level *level)
{
	StructureStart *start = nullptr;

    auto it = startIdClassMap.find(tag->getString(L"id"));
    if(it != startIdClassMap.end())
	{
		start = (it->second)();
	}

	if (start != nullptr)
	{
		start->load(level, tag);
	}
	else
	{
		app.DebugPrintf( "Skipping Structure with id %ls", tag->getString(L"id").c_str() );
	}
	return start;
}

StructurePiece *StructureFeatureIO::loadStaticPiece(CompoundTag *tag, Level *level)
{
	StructurePiece *piece = nullptr;

    auto it = pieceIdClassMap.find(tag->getString(L"id"));
    if(it != pieceIdClassMap.end())
	{
		piece = (it->second)();
	}

	if (piece != nullptr)
	{
		piece->load(level, tag);
	}
	else
	{
		app.DebugPrintf( "Skipping Piece with id %ls", tag->getString(L"id").c_str() );
	}
	return piece;
}