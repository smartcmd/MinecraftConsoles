#pragma once
#include "StructureFeature.h"
#include "StructureStart.h"
class Biome;

class VillageFeature : public StructureFeature
{
public:
	static const wstring OPTION_SIZE_MODIFIER;
	static const wstring OPTION_SPACING;

private:
	int villageSizeModifier;
	int townSpacing;
	int minTownSeparation;

	void _init(int iXZSize);
public:
	static void staticCtor();
	static vector<Biome *> allowedBiomes;
	VillageFeature(int iXZSize);
	VillageFeature(unordered_map<wstring, wstring> options, int iXZSize);
	wstring getFeatureName();

protected:
	virtual bool isFeatureChunk(int x, int z, bool bIsSuperflat=false);
	virtual StructureStart *createStructureStart(int x, int z);


public:
	class VillageStart : public StructureStart
	{
public:
	static StructureStart *Create() { return new VillageStart(); }
	virtual EStructureStart GetType() { return eStructureStart_VillageStart; }

	private:
		bool valid;
		int m_iXZSize;
	public:
		VillageStart();
		VillageStart(Level *level, Random *random, int chunkX, int chunkZ, int villageSizeModifier,int iXZSize);
		bool isValid();
		void addAdditonalSaveData(CompoundTag *tag);
		void readAdditonalSaveData(CompoundTag *tag);
	};

	int m_iXZSize;
};
