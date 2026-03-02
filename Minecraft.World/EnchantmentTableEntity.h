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
	wstring name;

public:
	EnchantmentTableEntity();
	~EnchantmentTableEntity();

	virtual void save(CompoundTag *base);
	virtual void load(CompoundTag *base);
	virtual void tick();
	virtual wstring getName();
	virtual wstring getCustomName();
	virtual bool hasCustomName();
	virtual void setCustomName(const wstring &name);

	// 4J Added
	virtual shared_ptr<TileEntity> clone();
};