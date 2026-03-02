#pragma once
#include "TileEntity.h"
class Random;

class EnchantmentTableEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_ENCHANTMENTTABLEENTITY; }
	static TileEntity *create() { return new EnchantmentTableEntity(); }

public:
	int time;
    float flip, oFlip, flipT, flipA;
    float open, oOpen;
    float rot, oRot, tRot;
private:
	Random *random;
public:
	EnchantmentTableEntity();
	~EnchantmentTableEntity();
    virtual void tick();

	// 4J Added
	virtual shared_ptr<TileEntity> clone();
};