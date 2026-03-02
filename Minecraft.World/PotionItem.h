#pragma once
using namespace std;
#include "Item.h"

class MobEffectInstance;

class PotionItem : public Item
{
private:
	static const int DRINK_DURATION = (int) (20 * 1.6);

public:
	static const wstring DEFAULT_ICON;
	static const wstring THROWABLE_ICON;
	static const wstring CONTENTS_ICON;

private:
	unordered_map<int, vector<MobEffectInstance *> *> cachedMobEffects;

	Icon *iconThrowable;
	Icon *iconDrinkable;
	Icon *iconOverlay;

public:
	PotionItem(int id);

	virtual vector<MobEffectInstance *> *getMobEffects(shared_ptr<ItemInstance> potion);
	virtual vector<MobEffectInstance *> *getMobEffects(int auxValue);
	virtual shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance);
	virtual UseAnim getUseAnimation(shared_ptr<ItemInstance> itemInstance);
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual bool TestUse(Level *level, shared_ptr<Player> player);
	virtual bool useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
	virtual Icon *getIcon(int auxValue);
	virtual Icon *getLayerIcon(int auxValue, int spriteLayer);
	static bool isThrowable(int auxValue);
	int getColor(int data);
	virtual int getColor(shared_ptr<ItemInstance> item, int spriteLayer);
	virtual bool hasMultipleSpriteLayers();
	virtual bool hasInstantenousEffects(int itemAuxValue);
	virtual wstring getHoverName(shared_ptr<ItemInstance> itemInstance);
	virtual void appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<wstring> *lines, bool advanced, vector<wstring> &unformattedStrings);
	virtual bool isFoil(shared_ptr<ItemInstance> itemInstance);

	virtual unsigned int getUseDescriptionId(shared_ptr<ItemInstance> instance);

	//@Override
	void registerIcons(IconRegister *iconRegister);
	static Icon *getTexture(const wstring &name);

	// 4J Stu - Based loosely on a function that gets added in java much later on (1.3)
	static vector<pair<int, int> > *getUniquePotionValues();
private:
	// 4J Stu - Added to support function above, different from Java implementation
	static vector<pair<int, int> > s_uniquePotionValues;
};