#pragma once

#include "SavedData.h"

class StructureFeatureSavedData : public SavedData
{
private:
	static wstring TAG_FEATURES;
	CompoundTag *pieceTags;

public:
	StructureFeatureSavedData(const wstring &idName);
	~StructureFeatureSavedData();

	void load(CompoundTag *tag);
	void save(CompoundTag *tag);
	CompoundTag *getFeatureTag(int chunkX, int chunkZ);
	void putFeatureTag(CompoundTag *tag, int chunkX, int chunkZ);
	wstring createFeatureTagId(int chunkX, int chunkZ);
	CompoundTag *getFullTag();
};