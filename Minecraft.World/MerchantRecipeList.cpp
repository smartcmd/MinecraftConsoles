#include "stdafx.h"
#include "net.minecraft.world.item.trading.h"
#include "MerchantRecipeList.h"

MerchantRecipeList::MerchantRecipeList()
{
}

MerchantRecipeList::MerchantRecipeList(CompoundTag *tag)
{
	load(tag);
}

MerchantRecipeList::~MerchantRecipeList()
{
	for(auto& it : m_recipes)
	{
		delete it;
	}
}

MerchantRecipe *MerchantRecipeList::getRecipeFor(shared_ptr<ItemInstance> buyA, shared_ptr<ItemInstance> buyB, int selectionHint)
{
	if (buyA == nullptr)
	{
		return nullptr;
	}

	if (selectionHint > 0 && selectionHint < static_cast<int>(m_recipes.size()))
	{
		// attempt to match vs the hint
		MerchantRecipe *r = m_recipes.at(selectionHint);
		if (buyA->id == r->getBuyAItem()->id && ((buyB == nullptr && !r->hasSecondaryBuyItem()) || (r->hasSecondaryBuyItem() && buyB != nullptr && r->getBuyBItem()->id == buyB->id)))
		{
			if (buyA->count >= r->getBuyAItem()->count && (!r->hasSecondaryBuyItem() || buyB->count >= r->getBuyBItem()->count))
			{
				return r;
			}
		}
		return nullptr;
	}
	for (size_t i = 0; i < m_recipes.size(); i++)
	{
		MerchantRecipe *r = m_recipes.at(i);
		if (buyA->id == r->getBuyAItem()->id && buyA->count >= r->getBuyAItem()->count
			&& ((!r->hasSecondaryBuyItem() && buyB == nullptr) || (r->hasSecondaryBuyItem() && buyB != nullptr && r->getBuyBItem()->id == buyB->id && buyB->count >= r->getBuyBItem()->count)))
		{
			return r;
		}
	}
	return nullptr;
}

bool MerchantRecipeList::addIfNewOrBetter(MerchantRecipe *recipe)
{
	bool added = false;
	for (size_t i = 0; i < m_recipes.size(); i++)
	{
		MerchantRecipe *r = m_recipes.at(i);
		if (recipe->isSame(r))
		{
			if (recipe->isSameSameButBetter(r))
			{
				delete m_recipes[i];
				m_recipes[i] = recipe;
				added = true;
			}
			return added;
		}
	}
	m_recipes.push_back(recipe);
	return true;
}

MerchantRecipe *MerchantRecipeList::getMatchingRecipeFor(shared_ptr<ItemInstance> buy, shared_ptr<ItemInstance> buyB, shared_ptr<ItemInstance> sell)
{
	for (size_t i = 0; i < m_recipes.size(); i++)
	{
		MerchantRecipe *r = m_recipes.at(i);
		if (buy->id == r->getBuyAItem()->id && buy->count >= r->getBuyAItem()->count && sell->id == r->getSellItem()->id)
		{
			if (!r->hasSecondaryBuyItem() || (buyB != nullptr && buyB->id == r->getBuyBItem()->id && buyB->count >= r->getBuyBItem()->count))
			{
				return r;
			}
		}
	}
	return nullptr;
}

void MerchantRecipeList::writeToStream(DataOutputStream *stream)
{
	stream->writeByte(static_cast<byte>(m_recipes.size() & 0xff));
	for (size_t i = 0; i < m_recipes.size(); i++)
	{
		MerchantRecipe *r = m_recipes.at(i);
		Packet::writeItem(r->getBuyAItem(), stream);
		Packet::writeItem(r->getSellItem(), stream);

		shared_ptr<ItemInstance> buyBItem = r->getBuyBItem();
		stream->writeBoolean(buyBItem != nullptr);
		if (buyBItem != nullptr)
		{
			Packet::writeItem(buyBItem, stream);
		}
		stream->writeBoolean(r->isDeprecated());
		stream->writeInt(r->getUses());
		stream->writeInt(r->getMaxUses());
	}
}

MerchantRecipeList *MerchantRecipeList::createFromStream(DataInputStream *stream)
{
	MerchantRecipeList *list = new MerchantRecipeList();

	int count = (int) (stream->readByte() & 0xff);
	for (int i = 0; i < count; i++)
	{
		shared_ptr<ItemInstance> buy = Packet::readItem(stream);
		shared_ptr<ItemInstance> sell = Packet::readItem(stream);

		shared_ptr<ItemInstance> buyB = nullptr;
		if (stream->readBoolean())
		{
			buyB = Packet::readItem(stream);
		}
		bool isDeprecated = stream->readBoolean();
		int uses = stream->readInt();
		int maxUses = stream->readInt();

		MerchantRecipe *recipe = new MerchantRecipe(buy, buyB, sell, uses, maxUses);
		if (isDeprecated)
		{
			recipe->enforceDeprecated();
		}
		list->push_back(recipe);
	}
	return list;
}

void MerchantRecipeList::load(CompoundTag *tag)
{
	ListTag<CompoundTag> *list = (ListTag<CompoundTag> *) tag->getList(L"Recipes");

	for (int i = 0; i < list->size(); i++)
	{
		CompoundTag *recipeTag = list->get(i);
		m_recipes.push_back(new MerchantRecipe(recipeTag));
	}
}

CompoundTag *MerchantRecipeList::createTag()
{
	CompoundTag *tag = new CompoundTag();

	ListTag<CompoundTag> *list = new ListTag<CompoundTag>(L"Recipes");
	for (size_t i = 0; i < m_recipes.size(); i++)
	{
		MerchantRecipe *merchantRecipe = m_recipes.at(i);
		list->add(merchantRecipe->createTag());
	}
	tag->put(L"Recipes", list);

	return tag;
}

void MerchantRecipeList::push_back(MerchantRecipe *recipe)
{
	m_recipes.push_back(recipe);
}

MerchantRecipe *MerchantRecipeList::at(size_t index)
{
	if (index >= m_recipes.size())
	{
		return nullptr;
	}
	return m_recipes.at(index);
}

std::vector<MerchantRecipe *>::iterator MerchantRecipeList::begin()
{
	return m_recipes.begin();
}

std::vector<MerchantRecipe *>::iterator MerchantRecipeList::end()
{
	return m_recipes.end();
}

std::vector<MerchantRecipe *>::iterator MerchantRecipeList::erase(std::vector<MerchantRecipe *>::iterator it)
{
	return m_recipes.erase(it);
}

size_t MerchantRecipeList::size()
{
	return m_recipes.size();
}

bool MerchantRecipeList::empty()
{
	return m_recipes.empty();
}