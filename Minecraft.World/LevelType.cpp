#include "stdafx.h"
#include "net.minecraft.world.level.h"


// 	public static final LevelType[] levelTypes = new LevelType[16];
// 
// 

// 
// 
// 	private final String generatorName;
// 	private final int version;
// 	private boolean selectable;
// 	private boolean replacement;

LevelType *LevelType::levelTypes[16];// = new LevelType[16];
LevelType *LevelType::lvl_normal=nullptr;// = new LevelType(0, "default", 1).setHasReplacement();
LevelType *LevelType::lvl_flat=nullptr;// = new LevelType(1, "flat");
LevelType *LevelType::lvl_largeBiomes = nullptr;// = new LevelType(2, "largeBiomes");
LevelType *LevelType::lvl_normal_1_1=nullptr;// = new LevelType(8, "default_1_1", 0).setSelectableByUser(false);

void LevelType::staticCtor()
{
	for(int i=0;i<16;i++) levelTypes[i]=nullptr;
	lvl_normal = new LevelType(0, L"default", 1);
	lvl_normal->setHasReplacement();
	lvl_flat = new LevelType(1, L"flat");
	lvl_largeBiomes = new LevelType(2, L"largeBiomes");
	lvl_normal_1_1 = new LevelType(8, L"default_1_1", 0);
	lvl_normal_1_1->setSelectableByUser(false);	
}

LevelType::LevelType(int id, wstring generatorName) 
{
	init(id, generatorName, 0);
}

LevelType::LevelType(int id, wstring generatorName, int version) 
{
	m_generatorName = generatorName;
	m_version = version;
	m_selectable = true;
	levelTypes[id] = this;
}


void LevelType::init(int id, wstring generatorName, int version) 
{
	this->id = id;
	m_generatorName = generatorName;
	m_version = version;
	m_selectable = true;
	levelTypes[id] = this;
}

wstring LevelType::getGeneratorName() 
{
	return m_generatorName;
}

wstring LevelType::getDescriptionId() 
{
	return L"generator." + m_generatorName;
}

int LevelType::getVersion() 
{
	return m_version;
}

LevelType *LevelType::getReplacementForVersion(int oldVersion) 
{
	if (this == lvl_normal && oldVersion == 0) 
	{
		return lvl_normal_1_1;
	}
	return this;
}

LevelType *LevelType::setSelectableByUser(bool selectable) 
{
	m_selectable = selectable;
	return this;
}

bool LevelType::isSelectable() 
{
	return m_selectable;
}

LevelType *LevelType::setHasReplacement() 
{
	m_replacement = true;
	return this;
}

bool LevelType::hasReplacement() 
{
	return m_replacement;
}

LevelType *LevelType::getLevelType(wstring name) 
{
	if(name.length()>0)
	{	
		for (int i = 0; i < 16; i++) 
		{
			wstring genname=levelTypes[i]->m_generatorName;

			if (levelTypes[i] != nullptr && (genname.compare(name)==0)) 
			{
				return levelTypes[i];
			}
		}
	}
	return nullptr;
}

int LevelType::getId()
{
	return id;
}