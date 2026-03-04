#include "stdafx.h"
#include "StatsScreen.h"
#include "StatsCounter.h"
#include "ItemRenderer.h"
#include "Button.h"
#include "Font.h"
#include "Lighting.h"
#include "..\Minecraft.World\net.minecraft.locale.h"
#include "..\Minecraft.World\net.minecraft.stats.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"

const float StatsScreen::SLOT_TEX_SIZE = 128.0f;
ItemRenderer *StatsScreen::itemRenderer = NULL;

StatsScreen::StatsScreen(Screen *lastScreen, StatsCounter *stats)
{
	// 4J - added initialisers
	itemRenderer = new ItemRenderer();
	statsList = NULL;
	itemStatsList = NULL;
	blockStatsList = NULL;
    this->lastScreen = lastScreen;
    this->stats = stats;
}

void StatsScreen::init()
{
    title = I18n::get(L"gui.stats");

    statsList = new GeneralStatisticsList(this);
    statsList->init(&buttons, 1, 1);

    itemStatsList = new ItemStatisticsList(this);
    itemStatsList->init(&buttons, 1, 1);

    blockStatsList = new BlockStatisticsList(this);
    blockStatsList->init(&buttons, 1, 1);

    activeList = statsList;

    postInit();
}

void StatsScreen::postInit()
{
    Language *language = Language::getInstance();
    buttons.push_back(new Button(BUTTON_CANCEL_ID, width / 2 + 4, height - 28, 150, 20, language->getElement(L"gui.done")));

    Button *blockButton, *itemButton;

    buttons.push_back(new Button(BUTTON_STATS_ID, width / 2 - 154, height - 52, 100, 20, language->getElement(L"stat.generalButton")));
    buttons.push_back(blockButton = new Button(BUTTON_BLOCKITEMSTATS_ID, width / 2 - 46, height - 52, 100, 20, language->getElement(L"stat.blocksButton")));
    buttons.push_back(itemButton = new Button(BUTTON_ITEMSTATS_ID, width / 2 + 62, height - 52, 100, 20, language->getElement(L"stat.itemsButton")));

    if (blockStatsList->getNumberOfItems() == 0)
	{
        blockButton->active = false;
    }
    if (itemStatsList->getNumberOfItems() == 0)
	{
        itemButton->active = false;
    }

}

void StatsScreen::buttonClicked(Button *button)
{
    if (!button->active) return;
    if (button->id == BUTTON_CANCEL_ID)
	{
        minecraft->setScreen(lastScreen);
    }
	else if (button->id == BUTTON_STATS_ID)
	{
        activeList = statsList;
    }
	else if (button->id == BUTTON_ITEMSTATS_ID)
	{
        activeList = itemStatsList;
    }
	else if (button->id == BUTTON_BLOCKITEMSTATS_ID)
	{
        activeList = blockStatsList;
    }
	else
	{
        activeList->buttonClicked(button);
    }
}

void StatsScreen::render(int xm, int ym, float a)
{
    activeList->render(xm, ym, a);

    drawCenteredString(font, title, width / 2, 20, 0xffffff);

    Screen::render(xm, ym, a);

}


StatsScreen::GeneralStatisticsList::GeneralStatisticsList(StatsScreen *ss) : ScrolledSelectionList(ss->minecraft, ss->width, ss->height, 32, ss->height - 64, 10)
{
	parent = ss;
	setRenderSelection(false);
}

int StatsScreen::GeneralStatisticsList::getNumberOfItems()
{
	return (int)Stats::generalStats->size();
}

void StatsScreen::GeneralStatisticsList::selectItem(int item, bool doubleClick)
{
}

bool StatsScreen::GeneralStatisticsList::isSelectedItem(int item)
{
	return false;
}

int StatsScreen::GeneralStatisticsList::getMaxPosition()
{
	return getNumberOfItems() * 10;
}

void StatsScreen::GeneralStatisticsList::renderBackground()
{
	parent->renderBackground();	// 4J - was StatsScreen.this.renderBackground();
}

void StatsScreen::GeneralStatisticsList::renderItem(int i, int x, int y, int h, Tesselator *t)
{
    Stat *stat = Stats::generalStats->at(i);
    parent->drawString(parent->font, stat->name, x + 2, y + 1, i % 2 == 0 ? 0xffffff : 0x909090);
    wstring msg = stat->format(parent->stats->getTotalValue(stat));
    parent->drawString(parent->font, msg, x + 2 + 213 - parent->font->width(msg), y + 1, i % 2 == 0 ? 0xffffff : 0x909090);
}

void StatsScreen::blitSlot(int x, int y, int item)
{
// 4J Unused
#if 0
	blitSlotBg(x + SLOT_BG_X, y + SLOT_BG_Y);

	glEnable(GL_RESCALE_NORMAL);

	glPushMatrix();
	glRotatef(180, 1, 0, 0);
	Lighting::turnOn();
	glPopMatrix();

	itemRenderer->renderGuiItem(font, minecraft->textures, item, 0, Item::items[item]->getIcon(0), x + SLOT_FG_X, y + SLOT_FG_Y);
	Lighting::turnOff();

	glDisable(GL_RESCALE_NORMAL);
#endif
}

void StatsScreen::blitSlotBg(int x, int y)
{
	blitSlotIcon(x, y, 0, 0);
}

void StatsScreen::blitSlotIcon(int x, int y, int sx, int sy)
{
	// 4J Unused
#if 0
    int tex = minecraft->textures->loadTexture(L"/gui/slot.png");
    glColor4f(1, 1, 1, 1);
    minecraft->textures->bind(tex);

    const float us = 1 / SLOT_TEX_SIZE;
    const float vs = 1 / SLOT_TEX_SIZE;
    const int w = SLOT_BG_SIZE;
    const int h = SLOT_BG_SIZE;
    Tesselator *t = Tesselator::getInstance();
    t->begin();
    t->vertexUV((float)(x + 0), (float)( y + h), (float)( blitOffset), (float)( (sx + 0) * us), (float)( (sy + h) * vs));
    t->vertexUV((float)(x + w), (float)( y + h), (float)( blitOffset), (float)( (sx + w) * us), (float)( (sy + h) * vs));
    t->vertexUV((float)(x + w), (float)( y + 0), (float)( blitOffset), (float)( (sx + w) * us), (float)( (sy + 0) * vs));
    t->vertexUV((float)(x + 0), (float)( y + 0), (float)( blitOffset), (float)( (sx + 0) * us), (float)( (sy + 0) * vs));
    t->end();
#endif
}

// 4J - added parameter so we can access parent
StatsScreen::StatisticsList::StatisticsList(StatsScreen *ss) : ScrolledSelectionList(ss->minecraft, ss->width, ss->height, 32, ss->height - 64, SLOT_STAT_HEIGHT)
{
	// 4J - added initialisers
	parent = ss;
	headerPressed = -1;
	sortColumn = -1;
	sortOrder = SORT_NONE;

    setRenderSelection(false);
    setRenderHeader(true, SLOT_STAT_HEIGHT);
}

void StatsScreen::StatisticsList::selectItem(int item, bool doubleClick)
{
}

bool StatsScreen::StatisticsList::isSelectedItem(int item)
{
	return false;
}

void StatsScreen::StatisticsList::renderBackground()
{
	parent->renderBackground();	// 4J - was	StatsScreen.this.renderBackground();
}

void StatsScreen::StatisticsList::renderHeader(int x, int y, Tesselator *t)
{
        if (!Mouse::isButtonDown(0))
		{
            headerPressed = -1;
        }

        if (headerPressed == 0)
		{
            parent->blitSlotIcon(x + ROW_COL_1 - SLOT_BG_SIZE, y + SLOT_BG_Y, SLOT_BG_SIZE * 0, SLOT_BG_SIZE * 0);
        }
		else
		{
            parent->blitSlotIcon(x + ROW_COL_1 - SLOT_BG_SIZE, y + SLOT_BG_Y, SLOT_BG_SIZE * 0, SLOT_BG_SIZE * 1);
        }

        if (headerPressed == 1)
		{
            parent->blitSlotIcon(x + ROW_COL_2 - SLOT_BG_SIZE, y + SLOT_BG_Y, SLOT_BG_SIZE * 0, SLOT_BG_SIZE * 0);
		}
		else
		{
            parent->blitSlotIcon(x + ROW_COL_2 - SLOT_BG_SIZE, y + SLOT_BG_Y, SLOT_BG_SIZE * 0, SLOT_BG_SIZE * 1);
        }

        if (headerPressed == 2)
		{
            parent->blitSlotIcon(x + ROW_COL_3 - SLOT_BG_SIZE, y + SLOT_BG_Y, SLOT_BG_SIZE * 0, SLOT_BG_SIZE * 0);
        }
		else
		{
            parent->blitSlotIcon(x + ROW_COL_3 - SLOT_BG_SIZE, y + SLOT_BG_Y, SLOT_BG_SIZE * 0, SLOT_BG_SIZE * 1);
        }

        if (sortColumn != -1)
		{
            int offset = ROW_COL_1 - SLOT_BG_SIZE * 2;
            int image = SLOT_BG_SIZE;

            if (sortColumn == 1)
			{
                offset = ROW_COL_2 - SLOT_BG_SIZE * 2;
            }
			else if (sortColumn == 2)
			{
                offset = ROW_COL_3 - SLOT_BG_SIZE * 2;
            }

            if (sortOrder == SORT_UP)
			{
                image = SLOT_BG_SIZE * 2;
            }
            parent->blitSlotIcon(x + offset, y + SLOT_BG_Y, image, SLOT_BG_SIZE * 0);
        }

}

void StatsScreen::StatisticsList::clickedHeader(int headerMouseX, int headerMouseY)
{
    headerPressed = -1;
    if (headerMouseX >= (ROW_COL_1 - SLOT_BG_SIZE * 2) && headerMouseX < ROW_COL_1)
	{
        headerPressed = 0;
    }
	else if (headerMouseX >= (ROW_COL_2 - SLOT_BG_SIZE * 2) && headerMouseX < ROW_COL_2)
	{
        headerPressed = 1;
    }
	else if (headerMouseX >= (ROW_COL_3 - SLOT_BG_SIZE * 2) && headerMouseX < ROW_COL_3)
	{
        headerPressed = 2;
    }

    if (headerPressed >= 0)
	{
        sortByColumn(headerPressed);
        parent->minecraft->soundEngine->playUI(eSoundType_RANDOM_CLICK, 1, 1);
    }
}

int StatsScreen::StatisticsList::getNumberOfItems()
{
	return (int)statItemList.size();
}

ItemStat *StatsScreen::StatisticsList::getSlotStat(int slot)
{
	return statItemList.at(slot);
}

void StatsScreen::StatisticsList::renderStat(ItemStat *stat, int x, int y, bool shaded)
{
    if (stat != NULL)
	{
        wstring msg = stat->format(parent->stats->getTotalValue(stat));
        parent->drawString(parent->font, msg, x - parent->font->width(msg), y + SLOT_TEXT_OFFSET, shaded ? 0xffffff : 0x909090);
    }
	else
	{
        wstring msg = L"-";
        parent->drawString(parent->font, msg, x - parent->font->width(msg), y + SLOT_TEXT_OFFSET, shaded ? 0xffffff : 0x909090);
    }
}

void StatsScreen::StatisticsList::renderDecorations(int mouseX, int mouseY)
{
    if (mouseY < y0 || mouseY > y1 )
	{
        return;
    }

    int slot = getItemAtPosition(mouseX, mouseY);
    int rowX = parent->width / 2 - 92 - 16;
    if (slot >= 0)
	{
        if (mouseX < (rowX + SLOT_LEFT_INSERT) || mouseX > (rowX + SLOT_LEFT_INSERT + 20))
		{
            return;
        }

        ItemStat *stat = getSlotStat(slot);
        renderMousehoverTooltip(stat, mouseX, mouseY);
    }
	else
	{
        wstring elementName;
        if (mouseX >= (rowX + ROW_COL_1 - SLOT_BG_SIZE) && mouseX <= (rowX + ROW_COL_1))
		{
            elementName = getHeaderDescriptionId(0);
        }
		else if (mouseX >= (rowX + ROW_COL_2 - SLOT_BG_SIZE) && mouseX <= (rowX + ROW_COL_2))
		{
            elementName = getHeaderDescriptionId(1);
        }
		else if (mouseX >= (rowX + ROW_COL_3 - SLOT_BG_SIZE) && mouseX <= (rowX + ROW_COL_3))
		{
            elementName = getHeaderDescriptionId(2);
        }
		else
		{
            return;
        }

        elementName = trimString(L"" + Language::getInstance()->getElement(elementName));

        if (elementName.length() > 0)
		{
            int rx = mouseX + 12;
            int ry = mouseY - 12;
            int width = parent->font->width(elementName);
            parent->fillGradient(rx - 3, ry - 3, rx + width + 3, ry + 8 + 3, 0xc0000000, 0xc0000000);

            parent->font->drawShadow(elementName, rx, ry, 0xffffffff);
        }
    }

}

void StatsScreen::StatisticsList::renderMousehoverTooltip(ItemStat *stat, int x, int y)
{
	// 4J Stu - Unused
#if 0
    if (stat == NULL)
	{
        return;
    }

    Item *item = Item::items[stat->getItemId()];

    wstring elementName = trimString(L"" + Language::getInstance()->getElementName(item->getDescriptionId()));

    if (elementName.length() > 0)
	{
        int rx = x + 12;
        int ry = y - 12;
        int width = parent->font->width(elementName);
        parent->fillGradient(rx - 3, ry - 3, rx + width + 3, ry + 8 + 3, 0xc0000000, 0xc0000000);

        parent->font->drawShadow(elementName, rx, ry, 0xffffffff);
    }
#endif
}

void StatsScreen::StatisticsList::sortByColumn(int column)
{
    if (column != sortColumn)
	{
        sortColumn = column;
        sortOrder = SORT_DOWN;
    }
	else if (sortOrder == SORT_DOWN)
	{
        sortOrder = SORT_UP;
    }
	else
	{
        sortColumn = -1;
        sortOrder = SORT_NONE;
    }

//    Collections.sort(statItemList, itemStatSorter);		// 4J - TODO
}


StatsScreen::ItemStatisticsList::ItemStatisticsList(StatsScreen *ss) : StatsScreen::StatisticsList(ss)
{
	//4J Gordon: Removed, not used anyway
	/*for(vector<ItemStat *>::iterator it = Stats::itemStats->begin(); it != Stats::itemStats->end(); it++ )
	{
		ItemStat *stat = *it;

        bool addToList = false;
        int id = stat->getItemId();

        if (parent->stats->getTotalValue(stat) > 0)
		{
            addToList = true;
        }
		else if (Stats::itemBroke[id] != NULL && parent->stats->getTotalValue(Stats::itemBroke[id]) > 0)
		{
            addToList = true;
        }
		else if (Stats::itemCrafted[id] != NULL && parent->stats->getTotalValue(Stats::itemCrafted[id]) > 0)
		{
            addToList = true;
        }
        if (addToList)
		{
            statItemList.push_back(stat);
        }
    }*/

	/* 4J - TODO
    itemStatSorter = new Comparator<ItemStat>() {
        public int compare(ItemStat o1, ItemStat o2) {
            int id1 = o1.getItemId();
            int id2 = o2.getItemId();

            Stat stat1 = null;
            Stat stat2 = null;
            if (sortColumn == COLUMN_DEPLETED) {
                stat1 = Stats.itemBroke[id1];
                stat2 = Stats.itemBroke[id2];
            } else if (sortColumn == COLUMN_CRAFTED) {
                stat1 = Stats.itemCrafted[id1];
                stat2 = Stats.itemCrafted[id2];
            } else if (sortColumn == COLUMN_USED) {
                stat1 = Stats.itemUsed[id1];
                stat2 = Stats.itemUsed[id2];
            }

            if (stat1 != null || stat2 != null) {
                if (stat1 == null) {
                    return 1;
                } else if (stat2 == null) {
                    return -1;
                } else {
                    int value1 = stats.getValue(stat1);
                    int value2 = stats.getValue(stat2);
                    if (value1 != value2) {
                        return (value1 - value2) * sortOrder;
                    }
                }
            }

            return id1 - id2;
        }
    };
	*/
}

void StatsScreen::ItemStatisticsList::renderHeader(int x, int y, Tesselator *t)
{
    StatsScreen::StatisticsList::renderHeader(x, y, t);

    if (headerPressed == 0)
	{
        parent->blitSlotIcon(x + ROW_COL_1 - SLOT_BG_SIZE + 1, y + SLOT_BG_Y + 1, SLOT_BG_SIZE * 4, SLOT_BG_SIZE * 1);
    }
	else
	{
        parent->blitSlotIcon(x + ROW_COL_1 - SLOT_BG_SIZE, y + SLOT_BG_Y, SLOT_BG_SIZE * 4, SLOT_BG_SIZE * 1);
    }
    if (headerPressed == 1)
	{
        parent->blitSlotIcon(x + ROW_COL_2 - SLOT_BG_SIZE + 1, y + SLOT_BG_Y + 1, SLOT_BG_SIZE * 1, SLOT_BG_SIZE * 1);
    }
	else
	{
        parent->blitSlotIcon(x + ROW_COL_2 - SLOT_BG_SIZE, y + SLOT_BG_Y, SLOT_BG_SIZE * 1, SLOT_BG_SIZE * 1);
    }
    if (headerPressed == 2)
	{
        parent->blitSlotIcon(x + ROW_COL_3 - SLOT_BG_SIZE + 1, y + SLOT_BG_Y + 1, SLOT_BG_SIZE * 2, SLOT_BG_SIZE * 1);
    }
	else
	{
        parent->blitSlotIcon(x + ROW_COL_3 - SLOT_BG_SIZE, y + SLOT_BG_Y, SLOT_BG_SIZE * 2, SLOT_BG_SIZE * 1);
    }

}

void StatsScreen::ItemStatisticsList::renderItem(int i, int x, int y, int h, Tesselator *t)
{
	//4J Gordon: Removed, not used anyway
    /*ItemStat *stat = getSlotStat(i);
    int id = stat->getItemId();

    parent->blitSlot(x + SLOT_LEFT_INSERT, y, id);

    renderStat((ItemStat *) Stats::itemBroke[id], x + ROW_COL_1, y, i % 2 == 0);
    renderStat((ItemStat *) Stats::itemCrafted[id], x + ROW_COL_2, y, i % 2 == 0);
    renderStat((ItemStat *) stat, x + ROW_COL_3, y, i % 2 == 0);*/
}

wstring StatsScreen::ItemStatisticsList::getHeaderDescriptionId(int column)
{
	if (column == COLUMN_CRAFTED)
	{
		return L"stat.crafted";
	}
	else if (column == COLUMN_USED)
	{
		return L"stat.used";
	}
	else
	{
		return L"stat.depleted";
	}
}

StatsScreen::BlockStatisticsList::BlockStatisticsList(StatsScreen *ss) : StatisticsList(ss)
{
	//4J Gordon: Removed, not used anyway
	/*for(vector<ItemStat *>::iterator it = Stats::blockStats->begin(); it != Stats::blockStats->end(); it++ )
	{
		ItemStat *stat = *it;

        bool addToList = false;
        int id = stat->getItemId();

        if (parent->stats->getTotalValue(stat) > 0)
		{
            addToList = true;
        }
		else if (Stats::itemUsed[id] != NULL && parent->stats->getTotalValue(Stats::itemUsed[id]) > 0)
		{
            addToList = true;
        }
		else if (Stats::itemCrafted[id] != NULL && parent->stats->getTotalValue(Stats::itemCrafted[id]) > 0)
		{
            addToList = true;
        }
        if (addToList)
		{
            statItemList.push_back(stat);
        }
    }*/

	/* 4J - TODO
            itemStatSorter = new Comparator<ItemStat>() {
                public int compare(ItemStat o1, ItemStat o2) {
                    int id1 = o1.getItemId();
                    int id2 = o2.getItemId();

                    Stat stat1 = null;
                    Stat stat2 = null;
                    if (sortColumn == COLUMN_MINED) {
                        stat1 = Stats.blockMined[id1];
                        stat2 = Stats.blockMined[id2];
                    } else if (sortColumn == COLUMN_CRAFTED) {
                        stat1 = Stats.itemCrafted[id1];
                        stat2 = Stats.itemCrafted[id2];
                    } else if (sortColumn == COLUMN_USED) {
                        stat1 = Stats.itemUsed[id1];
                        stat2 = Stats.itemUsed[id2];
                    }

                    if (stat1 != null || stat2 != null) {
                        if (stat1 == null) {
                            return 1;
                        } else if (stat2 == null) {
                            return -1;
                        } else {
                            int value1 = stats.getValue(stat1);
                            int value2 = stats.getValue(stat2);
                            if (value1 != value2) {
                                return (value1 - value2) * sortOrder;
                            }
                        }
                    }

                    return id1 - id2;
                }
            };
	*/
}

void StatsScreen::BlockStatisticsList::renderHeader(int x, int y, Tesselator *t)
{
    StatisticsList::renderHeader(x, y, t);

    if (headerPressed == 0)
	{
        parent->blitSlotIcon(x + ROW_COL_1 - SLOT_BG_SIZE + 1, y + SLOT_BG_Y + 1, SLOT_BG_SIZE * 1, SLOT_BG_SIZE * 1);
    }
	else
	{
        parent->blitSlotIcon(x + ROW_COL_1 - SLOT_BG_SIZE, y + SLOT_BG_Y, SLOT_BG_SIZE * 1, SLOT_BG_SIZE * 1);
    }
    if (headerPressed == 1)
	{
        parent->blitSlotIcon(x + ROW_COL_2 - SLOT_BG_SIZE + 1, y + SLOT_BG_Y + 1, SLOT_BG_SIZE * 2, SLOT_BG_SIZE * 1);
    }
	else
	{
        parent->blitSlotIcon(x + ROW_COL_2 - SLOT_BG_SIZE, y + SLOT_BG_Y, SLOT_BG_SIZE * 2, SLOT_BG_SIZE * 1);
    }
    if (headerPressed == 2)
	{
        parent->blitSlotIcon(x + ROW_COL_3 - SLOT_BG_SIZE + 1, y + SLOT_BG_Y + 1, SLOT_BG_SIZE * 3, SLOT_BG_SIZE * 1);
    }
	else
	{
        parent->blitSlotIcon(x + ROW_COL_3 - SLOT_BG_SIZE, y + SLOT_BG_Y, SLOT_BG_SIZE * 3, SLOT_BG_SIZE * 1);
    }

}

void StatsScreen::BlockStatisticsList::renderItem(int i, int x, int y, int h, Tesselator *t)
{
	//4J Gordon: Removed, not used anyway
    /*ItemStat *mineCount = getSlotStat(i);
    int id = mineCount->getItemId();

    parent->blitSlot(x + SLOT_LEFT_INSERT, y, id);

    renderStat((ItemStat *) Stats::itemCrafted[id], x + ROW_COL_1, y, i % 2 == 0);
    renderStat((ItemStat *) Stats::itemUsed[id], x + ROW_COL_2, y, i % 2 == 0);
    renderStat((ItemStat *) mineCount, x + ROW_COL_3, y, i % 2 == 0);*/
}

wstring StatsScreen::BlockStatisticsList::getHeaderDescriptionId(int column)
{
	if (column == COLUMN_CRAFTED)
	{
		return L"stat.crafted";
	}
	else if (column == COLUMN_USED)
	{
		return L"stat.used";
	}
	else
	{
		return L"stat.mined";
	}
}