#pragma once
using namespace std;

#include "Item.h"

class RecordingItem : public Item
{
private:
	static unordered_map<wstring, RecordingItem *> BY_NAME;

public:
	const std::wstring recording;

public: // 4J Stu - Was protected in Java, but the can't access it where we need
	RecordingItem(int id, const wstring& recording);

	Icon *getIcon(int auxValue);
	virtual bool useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);

	virtual void appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<HtmlString> *lines, bool advanced);
	virtual const Rarity *getRarity(shared_ptr<ItemInstance> itemInstance);

	void registerIcons(IconRegister *iconRegister);
	static RecordingItem *getByName(const wstring &name);
};