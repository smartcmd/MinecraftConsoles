#include "stdafx.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.trading.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.network.packet.h"
#include "..\..\Minecraft.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "..\..\ClientConnection.h"
#include "IUIScene_TradingMenu.h"

#include "UI.h"

IUIScene_TradingMenu::IUIScene_TradingMenu()
{
	m_validOffersCount = 0;
	m_selectedSlot = 0;
	m_offersStartIndex = 0;
	m_menu = nullptr;
	m_bHasUpdatedOnce = false;
}

shared_ptr<Merchant> IUIScene_TradingMenu::getMerchant()
{
	return m_merchant;
}

bool IUIScene_TradingMenu::handleKeyDown(int iPad, int iAction, bool bRepeat)
{
	bool handled = false;
	//MerchantRecipeList *offers = m_merchant->getOffers(Minecraft::GetInstance()->localplayers[getPad()]);

	bool changed = false;

	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( pMinecraft->localgameModes[getPad()] != nullptr )
	{
		Tutorial *tutorial = pMinecraft->localgameModes[getPad()]->getTutorial();
		if(tutorial != nullptr)
		{
			tutorial->handleUIInput(iAction);
			if(ui.IsTutorialVisible(getPad()) && !tutorial->isInputAllowed(iAction))
			{
				return S_OK;
			}
		}
	}


	switch(iAction)
	{
	case ACTION_MENU_B:
		ui.ShowTooltip( iPad, eToolTipButtonX, false );
		ui.ShowTooltip( iPad, eToolTipButtonB, false );
		ui.ShowTooltip( iPad, eToolTipButtonA, false );
		ui.ShowTooltip( iPad, eToolTipButtonRB, false );
		// kill the crafting xui
		//ui.PlayUISFX(eSFX_Back);
		ui.CloseUIScenes(iPad);

		handled = true;
		break;
	case ACTION_MENU_A:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		if(!m_activeOffers.empty())
		{
			int selectedShopItem = (m_selectedSlot + m_offersStartIndex);
			if( selectedShopItem < m_activeOffers.size() )
			{
				MerchantRecipe *activeRecipe = m_activeOffers.at(selectedShopItem).first;
				if(!activeRecipe->isDeprecated())
				{
					// Do we have the ingredients?
					shared_ptr<ItemInstance> buyAItem = activeRecipe->getBuyAItem();
					shared_ptr<ItemInstance> buyBItem = activeRecipe->getBuyBItem();
					shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->localplayers[getPad()];
					int buyAMatches = player->inventory->countMatches(buyAItem);
					int buyBMatches = player->inventory->countMatches(buyBItem);
					if( (buyAItem != nullptr && buyAMatches >= buyAItem->count) && (buyBItem == nullptr || buyBMatches >= buyBItem->count) )
					{
						// 4J-JEV: Fix for PS4 #7111: [PATCH 1.12] Trading Librarian villagers for multiple �Enchanted Books� will cause the title to crash.
						int actualShopItem = m_activeOffers.at(selectedShopItem).second;

						m_merchant->notifyTrade(activeRecipe);

						// Remove the items we are purchasing with
						player->inventory->removeResources(buyAItem);
						player->inventory->removeResources(buyBItem);

						// Add the item we have purchased
						shared_ptr<ItemInstance> result = activeRecipe->getSellItem()->copy();
						if(!player->inventory->add( result ) )
						{
							player->drop(result);
						}

						// Send a packet to the server
						player->connection->send(std::make_shared<TradeItemPacket>(m_menu->containerId, actualShopItem));

						updateDisplay();
					}
				}
			}
		}
		handled = true;
		break;
	case ACTION_MENU_LEFT:
		handled = true;
		if(m_selectedSlot == 0)
		{
			if(m_offersStartIndex > 0)
			{
				--m_offersStartIndex;
				changed = true;
			}
		}
		else
		{
			--m_selectedSlot;
			changed = true;
			moveSelector(false);
		}
		break;
	case ACTION_MENU_RIGHT:
		handled = true;
		if(m_selectedSlot == (DISPLAY_TRADES_COUNT - 1))
		{
			if((m_offersStartIndex + DISPLAY_TRADES_COUNT) < m_activeOffers.size())
			{
				++m_offersStartIndex;
				changed = true;
			}
		}
		else
		{
			++m_selectedSlot;
			changed = true;
			moveSelector(true);
		}
		break;
	}
	if (changed)
	{
		updateDisplay();

		int selectedShopItem = (m_selectedSlot + m_offersStartIndex);
		if( selectedShopItem < m_activeOffers.size() )
		{
			int actualShopItem = m_activeOffers.at(selectedShopItem).second;
			m_menu->setSelectionHint(actualShopItem);

			ByteArrayOutputStream rawOutput;
			DataOutputStream output(&rawOutput);
			output.writeInt(actualShopItem);
			Minecraft::GetInstance()->getConnection(getPad())->send(std::make_shared<CustomPayloadPacket>(CustomPayloadPacket::TRADER_SELECTION_PACKET, rawOutput.toByteArray()));
		}
	}
	return handled;
}

void IUIScene_TradingMenu::handleTick()
{
	int offerCount = 0;
	MerchantRecipeList *offers = m_merchant->getOffers(Minecraft::GetInstance()->localplayers[getPad()]);
	if (offers != nullptr)
	{
		offerCount = offers->size();

		if(!m_bHasUpdatedOnce)
		{
			updateDisplay();
		}
	}

	showScrollRightArrow( (m_offersStartIndex + DISPLAY_TRADES_COUNT) < m_activeOffers.size());
	showScrollLeftArrow(m_offersStartIndex > 0);
}

void IUIScene_TradingMenu::updateDisplay()
{
	int iA = -1;

	MerchantRecipeList *unfilteredOffers = m_merchant->getOffers(Minecraft::GetInstance()->localplayers[getPad()]);
	if (unfilteredOffers != nullptr)
	{
		m_activeOffers.clear();
		int unfilteredIndex = 0;
		int firstValidTrade = INT_MAX;
		for(auto& recipe : *unfilteredOffers)
		{
			if(!recipe->isDeprecated())
			{
				m_activeOffers.emplace_back(recipe,unfilteredIndex);
				firstValidTrade = std::min<int>(firstValidTrade, unfilteredIndex);
			}
			++unfilteredIndex;
		}

		if(!m_bHasUpdatedOnce)
		{
			if(firstValidTrade != 0 && firstValidTrade < unfilteredOffers->size())
			{
				m_menu->setSelectionHint(firstValidTrade);

				ByteArrayOutputStream rawOutput;
				DataOutputStream output(&rawOutput);
				output.writeInt(firstValidTrade);
				Minecraft::GetInstance()->getConnection(getPad())->send(std::make_shared<CustomPayloadPacket>(CustomPayloadPacket::TRADER_SELECTION_PACKET, rawOutput.toByteArray()));
			}
		}

		if( (m_offersStartIndex + DISPLAY_TRADES_COUNT) > m_activeOffers.size())
		{
			m_offersStartIndex = m_activeOffers.size() - DISPLAY_TRADES_COUNT;
			if(m_offersStartIndex < 0) m_offersStartIndex = 0;
		}

		for(unsigned int i = 0; i < DISPLAY_TRADES_COUNT; ++i)
		{
			int offerIndex = i + m_offersStartIndex;
			bool showRedBox = false;
			if(offerIndex < m_activeOffers.size())
			{
				showRedBox = !canMake(m_activeOffers.at(offerIndex).first);
				setTradeItem(i, m_activeOffers.at(offerIndex).first->getSellItem() );
			}
			else
			{
				setTradeItem(i, nullptr);
			}
			setTradeRedBox( i, showRedBox);
		}

		int selectedShopItem = (m_selectedSlot + m_offersStartIndex);
		if( selectedShopItem < m_activeOffers.size() )
		{
			MerchantRecipe *activeRecipe = m_activeOffers.at(selectedShopItem).first;

			wstring wsTemp;

			// 4J-PB - need to get the villager type here
			wsTemp = app.GetString(IDS_VILLAGER_OFFERS_ITEM);
			wsTemp = replaceAll(wsTemp,L"{*VILLAGER_TYPE*}",m_merchant->getDisplayName());
			size_t iPos=wsTemp.find(L"%s");
			wsTemp.replace(iPos,2,activeRecipe->getSellItem()->getHoverName());

			setTitle(wsTemp.c_str());

			vector<HtmlString> *offerDescription = GetItemDescription(activeRecipe->getSellItem());
			setOfferDescription(offerDescription);

			shared_ptr<ItemInstance> buyAItem = activeRecipe->getBuyAItem();
			shared_ptr<ItemInstance> buyBItem = activeRecipe->getBuyBItem();

			setRequest1Item(buyAItem);
			setRequest2Item(buyBItem);

			if(buyAItem != nullptr) setRequest1Name(buyAItem->getHoverName());
			else setRequest1Name(L"");

			if(buyBItem != nullptr) setRequest2Name(buyBItem->getHoverName());
			else setRequest2Name(L"");

			bool canMake = true;

			shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->localplayers[getPad()];
			int buyAMatches = player->inventory->countMatches(buyAItem);
			if(buyAMatches > 0)
			{
				setRequest1RedBox(buyAMatches < buyAItem->count);
				canMake = buyAMatches > buyAItem->count;
			}
			else
			{
				setRequest1RedBox(true);
				canMake = false;
			}

			int buyBMatches = player->inventory->countMatches(buyBItem);
			if(buyBMatches > 0)
			{
				setRequest2RedBox(buyBMatches < buyBItem->count);
				canMake = canMake && buyBMatches > buyBItem->count;
			}
			else
			{
				if(buyBItem!=nullptr)
				{
					setRequest2RedBox(true);
					canMake = false;
				}
				else
				{
					setRequest2RedBox(buyBItem != nullptr);
					canMake = canMake && buyBItem == nullptr;
				}
			}

			if(canMake) iA = IDS_TOOLTIPS_TRADE;
		}
		else
		{
			setTitle(m_merchant->getDisplayName());
			setRequest1Name(L"");
			setRequest2Name(L"");
			setRequest1RedBox(false);
			setRequest2RedBox(false);
			setRequest1Item(nullptr);
			setRequest2Item(nullptr);
			vector<HtmlString> offerDescription;
			setOfferDescription(&offerDescription);
		}

		m_bHasUpdatedOnce = true;
	}

	ui.SetTooltips(getPad(), iA, IDS_TOOLTIPS_EXIT);
}

bool IUIScene_TradingMenu::canMake(MerchantRecipe *recipe)
{
	bool canMake = false;
	if (recipe != nullptr)
	{
		if(recipe->isDeprecated()) return false;

		shared_ptr<ItemInstance> buyAItem = recipe->getBuyAItem();
		shared_ptr<ItemInstance> buyBItem = recipe->getBuyBItem();

		shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->localplayers[getPad()];
		int buyAMatches = player->inventory->countMatches(buyAItem);
		if(buyAMatches > 0)
		{
			canMake = buyAMatches >= buyAItem->count;
		}
		else
		{
			canMake = buyAItem == nullptr;
		}

		int buyBMatches = player->inventory->countMatches(buyBItem);
		if(buyBMatches > 0)
		{
			canMake = canMake && buyBMatches >= buyBItem->count;
		}
		else
		{
			canMake = canMake && buyBItem == nullptr;
		}
	}
	return canMake;
}


void IUIScene_TradingMenu::setRequest1Item(shared_ptr<ItemInstance> item)
{
}

void IUIScene_TradingMenu::setRequest2Item(shared_ptr<ItemInstance> item)
{
}

void IUIScene_TradingMenu::setTradeItem(int index, shared_ptr<ItemInstance> item)
{
}

vector<HtmlString> *IUIScene_TradingMenu::GetItemDescription(shared_ptr<ItemInstance> item)
{
	vector<HtmlString> *lines = item->getHoverText(nullptr, false);

	// Add rarity to first line
	if (lines->size() > 0)
	{
		lines->at(0).color = item->getRarity()->color;
	}

	return lines;
}

void IUIScene_TradingMenu::HandleInventoryUpdated()
{
	updateDisplay();
}