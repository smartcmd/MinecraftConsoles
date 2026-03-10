#include "stdafx.h"

#include "StructureFeatureSavedData.h"

wstring StructureFeatureSavedData::TAG_FEATURES = L"Features";

StructureFeatureSavedData::StructureFeatureSavedData(const wstring &idName) : SavedData(idName)
{
	this->pieceTags = new CompoundTag(TAG_FEATURES);
}

StructureFeatureSavedData::~StructureFeatureSavedData()
{
	delete pieceTags;
}

void StructureFeatureSavedData::load(CompoundTag *tag)
{
	this->pieceTags = tag->getCompound(TAG_FEATURES);
}

void StructureFeatureSavedData::save(CompoundTag *tag)
{
	tag->put(TAG_FEATURES, pieceTags->copy() );
}

CompoundTag *StructureFeatureSavedData::getFeatureTag(int chunkX, int chunkZ)
{
	return pieceTags->getCompound(createFeatureTagId(chunkX, chunkZ));
}

void StructureFeatureSavedData::putFeatureTag(CompoundTag *tag, int chunkX, int chunkZ)
{
	wstring name = createFeatureTagId(chunkX, chunkZ);
	tag->setName(name);
	pieceTags->put(name, tag);
}

wstring StructureFeatureSavedData::createFeatureTagId(int chunkX, int chunkZ)
{
	return L"[" + std::to_wstring(chunkX) + L"," + std::to_wstring(chunkZ) + L"]";
}

CompoundTag *StructureFeatureSavedData::getFullTag()
{
	return pieceTags;
}