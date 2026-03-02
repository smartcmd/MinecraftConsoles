#pragma once

#include "Item.h"

class FireworksItem : public Item
{
public:
	static const wstring TAG_FIREWORKS;
	static const wstring TAG_EXPLOSION;
	static const wstring TAG_EXPLOSIONS;
	static const wstring TAG_FLIGHT;
	static const wstring TAG_E_TYPE;
	static const wstring TAG_E_TRAIL;
	static const wstring TAG_E_FLICKER;
	static const wstring TAG_E_COLORS;
	static const wstring TAG_E_FADECOLORS;

	static const byte TYPE_SMALL = 0;
	static const byte TYPE_BIG = 1;
	static const byte TYPE_STAR = 2;
	static const byte TYPE_CREEPER = 3;
	static const byte TYPE_BURST = 4;

	static const byte TYPE_MIN = TYPE_SMALL;
	static const byte TYPE_MAX = TYPE_BURST;

	FireworksItem(int id);

	bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
	void appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<HtmlString> *lines, bool advanced);
};