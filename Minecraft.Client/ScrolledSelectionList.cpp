#include "stdafx.h"
#include "ScrolledSelectionList.h"
#include "Button.h"
#include "Tesselator.h"
#include "Textures.h"
#include "..\Minecraft.World\System.h"

ScrolledSelectionList::ScrolledSelectionList(Minecraft *minecraft, int width, int height, int y0, int y1, int itemHeight)
{
	this->minecraft = minecraft;
	this->width = width;
	this->height = height;
	this->y0 = y0;
	this->y1 = y1;
	this->itemHeight = itemHeight;
	this->x0 = 0;
	this->x1 = width;


	// 4J Stu - Smoe default initialisers
	upId = 0;
    downId = 0;

    yDrag = 0.0f;
    yDragScale = 0.0f;
    yo = 0.0f;

    lastSelection = 0;
    lastSelectionTime = 0;

    renderSelection = false;
    _renderHeader = false;
    headerHeight = 0;
	//End
}

void ScrolledSelectionList::setRenderSelection(bool renderSelection)
{
	this->renderSelection = renderSelection;
}

void ScrolledSelectionList::setRenderHeader(bool renderHeader, int headerHeight)
{
    this->_renderHeader = renderHeader;
    this->headerHeight = headerHeight;

    if (!_renderHeader)
	{
        this->headerHeight = 0;
    }
}

int ScrolledSelectionList::getMaxPosition()
{
	return getNumberOfItems() * itemHeight + headerHeight;
}

void ScrolledSelectionList::renderHeader(int x, int y, Tesselator *t)
{
}

void ScrolledSelectionList::clickedHeader(int headerMouseX, int headerMouseY)
{
}

void ScrolledSelectionList::renderDecorations(int mouseX, int mouseY)
{
}

 int ScrolledSelectionList::getItemAtPosition(int x, int y)
{
    int x0 = width / 2 - (92 + 16 + 2);
    int x1 = width / 2 + (92 + 16 + 2);

    int clickSlotPos = (y - y0 - headerHeight + static_cast<int>(yo) - 4);
    int slot = clickSlotPos / itemHeight;
    if (x >= x0 && x <= x1 && slot >= 0 && clickSlotPos >= 0 && slot < getNumberOfItems())
	{
        return slot;
    }
    return -1;
}

void ScrolledSelectionList::init(vector<Button *> *buttons, int upButtonId, int downButtonId)
{
    this->upId = upButtonId;
    this->downId = downButtonId;
}

void ScrolledSelectionList::capYPosition()
{
    int max = getMaxPosition() - (y1 - y0 - 4);
    if (max < 0) max /= 2;
    if (yo < 0) yo = 0;
    if (yo > max) yo = static_cast<float>(max);
}

void ScrolledSelectionList::buttonClicked(Button *button)
{
    if (!button->active) return;

    if (button->id == upId)
	{
        yo -= (itemHeight * 2) / 3;
        yDrag = DRAG_OUTSIDE;
        capYPosition();
    } else if (button->id == downId)
	{
        yo += (itemHeight * 2) / 3;
        yDrag = DRAG_OUTSIDE;
        capYPosition();
    }
}

void ScrolledSelectionList::render(int xm, int ym, float a)
{
	// 4J Unused
#if 0
    renderBackground();

    int itemCount = getNumberOfItems();

    int xx0 = width / 2 + 124;
    int xx1 = xx0 + 6;

    if (Mouse::isButtonDown(0))
	{
        if (yDrag == NO_DRAG)
		{
            bool doDrag = true;
            if (ym >= y0 && ym <= y1)
			{
                int x0 = width / 2 - (92 + 16 + 2);
                int x1 = width / 2 + (92 + 16 + 2);

                int clickSlotPos = (ym - y0 - headerHeight + (int) yo - 4);
                int slot = clickSlotPos / itemHeight;
                if (xm >= x0 && xm <= x1 && slot >= 0 && clickSlotPos >= 0 && slot < itemCount)
				{

                    bool doubleClick = slot == lastSelection && (System::currentTimeMillis() - lastSelectionTime) < 250;

                    selectItem(slot, doubleClick);
                    lastSelection = slot;
                    lastSelectionTime = System::currentTimeMillis();
                }
				else if (xm >= x0 && xm <= x1 && clickSlotPos < 0)
				{
                    clickedHeader(xm - x0, ym - y0 + (int) yo - 4);
                    doDrag = false;
                }
                if (xm >= xx0 && xm <= xx1)
				{
                    yDragScale = -1;

                    int max = getMaxPosition() - (y1 - y0 - 4);
                    if (max < 1) max = 1;
                    int barHeight = (int) ((y1 - y0) * (y1 - y0) / (float) getMaxPosition());
                    if (barHeight < 32) barHeight = 32;
                    if (barHeight > (y1 - y0 - 8)) barHeight = (y1 - y0 - 8);

//                        int yp = yo * (y1 - y0 - barHeight) / max + y0;
//                        if (yp < y0) yp = y0;

                    yDragScale /= (y1 - y0 - barHeight) / (float) max;

                }
				else
				{
                    yDragScale = 1;
                }
                if (doDrag)
				{
                    yDrag = (float)ym;
                }
				else
				{
                    yDrag = DRAG_OUTSIDE;
                }
            }
			else
			{
                yDrag = DRAG_OUTSIDE;
            }
        }
		else if (yDrag >= 0)
		{
            yo -= (ym - yDrag) * yDragScale;
            yDrag = (float)ym;
        }

    }
	else
	{
        yDrag = NO_DRAG;
    }

    capYPosition();

    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    Tesselator *t = Tesselator::getInstance();

    glBindTexture(GL_TEXTURE_2D, minecraft->textures->loadTexture(L"/gui/background.png"));
    glColor4f(1.0f, 1, 1, 1);
    float s = 32;
    t->begin();
    t->color(0x202020);
    t->vertexUV((float)(x0), (float)( y1), (float)( 0), (float)( x0 / s), (float)( (y1 + (int) yo) / s));
    t->vertexUV((float)(x1), (float)( y1), (float)( 0), (float)( x1 / s), (float)( (y1 + (int) yo) / s));
    t->vertexUV((float)(x1), (float)( y0), (float)( 0), (float)( x1 / s), (float)( (y0 + (int) yo) / s));
    t->vertexUV((float)(x0), (float)( y0), (float)( 0), (float)( x0 / s), (float)( (y0 + (int) yo) / s));
    t->end();

    int rowX = width / 2 - 92 - 16;
    int rowBaseY = y0 + 4 - (int) yo;

    if (_renderHeader)
	{
        renderHeader(rowX, rowBaseY, t);
    }

    for (int i = 0; i < itemCount; i++)
	{

        int y = rowBaseY + (i) * itemHeight + headerHeight;
        int h = itemHeight - 4;

        if (y > y1 || (y + h) < y0)
		{
            continue;
        }

        if (renderSelection && isSelectedItem(i))
		{
            int x0 = width / 2 - (92 + 16 + 2);
            int x1 = width / 2 + (92 + 16 + 2);
            glColor4f(1, 1, 1, 1);
            glDisable(GL_TEXTURE_2D);
            t->begin();
            t->color(0x808080);
            t->vertexUV((float)(x0), (float)( y + h + 2), (float)( 0), (float)( 0), (float)( 1));
            t->vertexUV((float)(x1), (float)( y + h + 2), (float)( 0), (float)( 1), (float)( 1));
            t->vertexUV((float)(x1), (float)( y - 2), (float)( 0), (float)( 1), (float)( 0));
            t->vertexUV((float)(x0), (float)( y - 2), (float)( 0), (float)( 0), (float)( 0));

            t->color(0x000000);
            t->vertexUV((float)(x0 + 1), (float)( y + h + 1), (float)( 0), (float)( 0), (float)( 1));
            t->vertexUV((float)(x1 - 1), (float)( y + h + 1), (float)( 0), (float)( 1), (float)( 1));
            t->vertexUV((float)(x1 - 1), (float)( y - 1), (float)( 0), (float)( 1), (float)( 0));
            t->vertexUV((float)(x0 + 1), (float)( y - 1), (float)( 0), (float)( 0), (float)( 0));

            t->end();
            glEnable(GL_TEXTURE_2D);
        }

        renderItem(i, rowX, y, h, t);

    }

    glDisable(GL_DEPTH_TEST);


    int d = 4;

    renderHoleBackground(0, y0, 255, 255);
    renderHoleBackground(y1, height, 255, 255);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_ALPHA_TEST);
    glShadeModel(GL_SMOOTH);

    glDisable(GL_TEXTURE_2D);

    t->begin();
    t->color(0x000000, 0);
    t->vertexUV((float)(x0), (float)( y0 + d), (float)( 0), (float)( 0), (float)( 1));
    t->vertexUV((float)(x1), (float)( y0 + d), (float)( 0), (float)( 1), (float)( 1));
    t->color(0x000000, 255);
    t->vertexUV((float)(x1), (float)( y0), (float)( 0), (float)( 1), (float)( 0));
    t->vertexUV((float)(x0), (float)( y0), (float)( 0), (float)( 0), (float)( 0));
    t->end();

    t->begin();
    t->color(0x000000, 255);
    t->vertexUV((float)(x0), (float)( y1), (float)( 0), (float)( 0), (float)( 1));
    t->vertexUV((float)(x1), (float)( y1), (float)( 0), (float)( 1), (float)( 1));
    t->color(0x000000, 0);
    t->vertexUV((float)(x1), (float)( y1 - d), (float)( 0), (float)( 1), (float)( 0));
    t->vertexUV((float)(x0), (float)( y1 - d), (float)( 0), (float)( 0), (float)( 0));
    t->end();

    {
        int max = getMaxPosition() - (y1 - y0 - 4);
        if (max > 0)
		{
            int barHeight = (int) (y1 - y0) * (y1 - y0) / (getMaxPosition());
            if (barHeight < 32) barHeight = 32;
            if (barHeight > (y1 - y0 - 8)) barHeight = (y1 - y0 - 8);

            int yp = (int) yo * (y1 - y0 - barHeight) / max + y0;
            if (yp < y0) yp = y0;

            t->begin();
            t->color(0x000000, 255);
            t->vertexUV((float)(xx0), (float)( y1), (float)( 0), (float)( 0), (float)( 1));
            t->vertexUV((float)(xx1), (float)( y1), (float)( 0), (float)( 1), (float)( 1));
            t->vertexUV((float)(xx1), (float)( y0), (float)( 0), (float)( 1), (float)( 0));
            t->vertexUV((float)(xx0), (float)( y0), (float)( 0), (float)( 0), (float)( 0));
            t->end();

            t->begin();
            t->color(0x808080, 255);
            t->vertexUV((float)(xx0), (float)( yp + barHeight), (float)( 0), (float)( 0), (float)( 1));
            t->vertexUV((float)(xx1), (float)( yp + barHeight), (float)( 0), (float)( 1), (float)( 1));
            t->vertexUV((float)(xx1), (float)( yp), (float)( 0), (float)( 1), (float)( 0));
            t->vertexUV((float)(xx0), (float)( yp), (float)( 0), (float)( 0), (float)( 0));
            t->end();

            t->begin();
            t->color(0xc0c0c0, 255);
            t->vertexUV((float)(xx0), (float)( yp + barHeight - 1), (float)( 0), (float)( 0), (float)( 1));
            t->vertexUV((float)(xx1 - 1), (float)( yp + barHeight - 1), (float)( 0), (float)( 1), (float)( 1));
            t->vertexUV((float)(xx1 - 1), (float)( yp), (float)( 0), (float)( 1), (float)( 0));
            t->vertexUV((float)(xx0), (float)( yp), (float)( 0), (float)( 0), (float)( 0));
            t->end();
        }
    }

    renderDecorations(xm, ym);

    glEnable(GL_TEXTURE_2D);

    glShadeModel(GL_FLAT);
    glEnable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
#endif
}

void ScrolledSelectionList::renderHoleBackground(int y0, int y1, int a0, int a1)
{
	// 4J Unused
#if 0
    Tesselator *t = Tesselator::getInstance();
    glBindTexture(GL_TEXTURE_2D, minecraft->textures->loadTexture(L"/gui/background.png"));
    glColor4f(1.0f, 1, 1, 1);
    float s = 32;
    t->begin();
    t->color(0x404040, a1);
    t->vertexUV((float)(0), (float)( y1), (float)( 0), (float)( 0), (float)( y1 / s));
    t->vertexUV((float)(width), (float)( y1), (float)( 0), (float)( width / s), (float)( y1 / s));
    t->color(0x404040, a0);
    t->vertexUV((float)(width), (float)( y0), (float)( 0), (float)( width / s), (float)( y0 / s));
    t->vertexUV((float)(0), (float)( y0), (float)( 0), (float)( 0), (float)( y0 / s));
    t->end();
#endif
}
