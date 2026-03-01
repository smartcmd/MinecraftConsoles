#pragma once
using namespace std;
#include "net.minecraft.world.level.h"

class LevelType 
{
public:
	static LevelType *levelTypes[16];// = new LevelType[16];
	static LevelType *lvl_normal;// = new LevelType(0, "default", 1).setHasReplacement();
	static LevelType *lvl_flat;// = new LevelType(1, "flat");
	static LevelType *lvl_largeBiomes;// = new LevelType(2, "largeBiomes");
	static LevelType *lvl_normal_1_1;// = new LevelType(8, "default_1_1", 0).setSelectableByUser(false);

	static void staticCtor();

private:
	int id;
	wstring m_generatorName;
	int m_version;
	bool m_selectable;
	bool m_replacement;

	LevelType(int id, wstring generatorName);
	LevelType(int id, wstring generatorName, int version); 
	void init(int id, wstring generatorName, int version);
public:
	wstring getGeneratorName();
	wstring getDescriptionId();
	int getVersion();
	LevelType *getReplacementForVersion(int oldVersion);
private:
	LevelType *setSelectableByUser(bool selectable);
public:
	bool isSelectable();
private:
	LevelType *setHasReplacement();
public:
	bool hasReplacement();
	static LevelType *getLevelType(wstring name);
	int getId();
};
