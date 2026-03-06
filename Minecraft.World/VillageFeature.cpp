#include "stdafx.h"
#include "VillageFeature.h"
#include "VillagePieces.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.dimension.h"

const wstring VillageFeature::OPTION_SIZE_MODIFIER = L"size";
const wstring VillageFeature::OPTION_SPACING = L"distance";

vector<Biome *> VillageFeature::allowedBiomes;

void VillageFeature::staticCtor()
{
	allowedBiomes.push_back( Biome::plains );
	allowedBiomes.push_back( Biome::desert );
}

void VillageFeature::_init(int iXZSize)
{
	villageSizeModifier = 0;
	townSpacing = 32;
	minTownSeparation = 8;

	m_iXZSize=iXZSize;
}

VillageFeature::VillageFeature(int iXZSize)
{
	_init(iXZSize);
}

VillageFeature::VillageFeature(unordered_map<wstring, wstring> options, int iXZSize)
{
	_init(iXZSize);

	for (auto& option : options)
	{
		if (option.first.compare(OPTION_SIZE_MODIFIER) == 0)
		{
			villageSizeModifier = Mth::getInt(option.second, villageSizeModifier, 0);
		}
		else if (option.first.compare(OPTION_SPACING) == 0)
		{
			townSpacing = Mth::getInt(option.second, townSpacing, minTownSeparation + 1);
		}
	}
}

wstring VillageFeature::getFeatureName()
{
	return L"Village";
}

bool VillageFeature::isFeatureChunk(int x, int z,bool bIsSuperflat)
{
	int townSpacing = this->townSpacing;

	if(!bIsSuperflat
#ifdef _LARGE_WORLDS
		&& level->dimension->getXZSize() < 128
#endif
		)
		{
			townSpacing= 16;// 4J change 32;
		}

		int xx = x;
		int zz = z;
		if (x < 0) x -= townSpacing - 1;
		if (z < 0) z -= townSpacing - 1;

		int xCenterTownChunk = x / townSpacing;
		int zCenterTownChunk = z / townSpacing;
		Random *r = level->getRandomFor(xCenterTownChunk, zCenterTownChunk, 10387312);
		xCenterTownChunk *= townSpacing;
		zCenterTownChunk *= townSpacing;
		xCenterTownChunk += r->nextInt(townSpacing - minTownSeparation);
		zCenterTownChunk += r->nextInt(townSpacing - minTownSeparation);
		x = xx;
		z = zz;

		bool forcePlacement = false;
		LevelGenerationOptions *levelGenOptions = app.getLevelGenerationOptions();
		if( levelGenOptions != NULL )
		{
			forcePlacement = levelGenOptions->isFeatureChunk(x,z,eFeature_Village);
		}

		if (forcePlacement || (x == xCenterTownChunk && z == zCenterTownChunk) )
		{
			bool biomeOk = level->getBiomeSource()->containsOnly(x * 16 + 8, z * 16 + 8, 0, allowedBiomes);
			if (biomeOk)
			{
				//app.DebugPrintf("Biome ok for Village at %d, %d\n",(x * 16 + 8),(z * 16 + 8));
				return true;
			}
		}

		return false;
}

StructureStart *VillageFeature::createStructureStart(int x, int z)
{
	// 4J added
	app.AddTerrainFeaturePosition(eTerrainFeature_Village,x,z);

	return new VillageStart(level, random, x, z, villageSizeModifier, m_iXZSize);
}

VillageFeature::VillageStart::VillageStart()
{
	valid = false;	// 4J added initialiser
	m_iXZSize = 0;
	// for reflection
}

VillageFeature::VillageStart::VillageStart(Level *level, Random *random, int chunkX, int chunkZ, int villageSizeModifier, int iXZSize)
{
	valid = false;	// 4J added initialiser
	m_iXZSize=iXZSize;

	list<VillagePieces::PieceWeight *> *pieceSet = VillagePieces::createPieceSet(random, villageSizeModifier);

	VillagePieces::StartPiece *startRoom = new VillagePieces::StartPiece(level->getBiomeSource(), 0, random, (chunkX << 4) + 2, (chunkZ << 4) + 2, pieceSet, villageSizeModifier, level);
	pieces.push_back(startRoom);
	startRoom->addChildren(startRoom, &pieces, random);

	vector<StructurePiece *> *pendingRoads = &startRoom->pendingRoads;
	vector<StructurePiece *> *pendingHouses = &startRoom->pendingHouses;
	while (!pendingRoads->empty() || !pendingHouses->empty())
	{

		// prioritize roads
		if (pendingRoads->empty())
		{
			int pos = random->nextInt((int)pendingHouses->size());
            auto it = pendingHouses->begin() + pos;
            StructurePiece *structurePiece = *it;
			pendingHouses->erase(it);
			structurePiece->addChildren(startRoom, &pieces, random);
		}
		else
		{
			int pos = random->nextInt((int)pendingRoads->size());
            auto it = pendingRoads->begin() + pos;
            StructurePiece *structurePiece = *it;
			pendingRoads->erase(it);
			structurePiece->addChildren(startRoom, &pieces, random);
		}
	}

	calculateBoundingBox();

	size_t count = 0;
	for(auto& piece : pieces)
	{
		if ( piece && dynamic_cast<VillagePieces::VillageRoadPiece *>(piece) )
		{
			++count;
		}
	}
	valid = count > 2;
}

bool VillageFeature::VillageStart::isValid()
{
	// 4J-PB - Adding a bounds check to ensure a village isn't over the edge of our world - we end up with half houses in that case
	if((boundingBox->x0<(-m_iXZSize/2)) || (boundingBox->x1>(m_iXZSize/2)) || (boundingBox->z0<(-m_iXZSize/2)) || (boundingBox->z1>(m_iXZSize/2)))
	{
		valid=false;
	}
	return valid;
}

void VillageFeature::VillageStart::addAdditonalSaveData(CompoundTag *tag)
{
	StructureStart::addAdditonalSaveData(tag);

	tag->putBoolean(L"Valid", valid);
}

void VillageFeature::VillageStart::readAdditonalSaveData(CompoundTag *tag)
{
	StructureStart::readAdditonalSaveData(tag);
	valid = tag->getBoolean(L"Valid");
}