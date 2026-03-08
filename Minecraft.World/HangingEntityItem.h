#pragma once

#include "Item.h"

class HangingEntity;

class HangingEntityItem : public Item 
{
private:
	//final Class<? extends HangingEntity> clazz;
	eINSTANCEOF eType;

public:
	HangingEntityItem(int id, eINSTANCEOF eClassType);

    virtual bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int xt, int yt, int zt, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly);//, float clickX, float clickY, float clickZ); 
	
private:
	shared_ptr<HangingEntity> createEntity(Level *level, int x, int y, int z, int dir, int auxValue); // 4J Stu added auxValue param

public:
	virtual void appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<HtmlString> *lines, bool advanced);
};
