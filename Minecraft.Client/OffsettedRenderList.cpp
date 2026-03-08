#include "stdafx.h"
#include "../Minecraft.World/IntBuffer.h"
#include "OffsettedRenderList.h"

// 4J added
OffsettedRenderList::OffsettedRenderList()
{
	x = y = z = 0;
	xOff = yOff = zOff = 0;
	lists = MemoryTracker::createIntBuffer(1024 * 64);
	inited = false;
	rendered = false;
}

void OffsettedRenderList::init(int x, int y, int z, double xOff, double yOff, double zOff)
{
    inited = true;
    lists->clear();
    this->x = x;
    this->y = y;
    this->z = z;

    this->xOff = (float) xOff;
    this->yOff = (float) yOff;
    this->zOff = (float) zOff;
}

bool OffsettedRenderList::isAt(int x, int y, int z)
{
    if (!inited) return false;
    return x == this->x && y == this->y && z == this->z;
}

void OffsettedRenderList::add(int list)
{
	// 4J - added - chunkList::getList returns -1 when chunks aren't visible, we really don't want to end up sending that to glCallLists
	if( list >= 0 )
	{
	    lists->put(list);
	}
    if (lists->remaining() == 0) render();
}

void OffsettedRenderList::render()
{
    if (!inited) return;
    if (!rendered)
	{
        lists->flip();
        rendered = true;
    }
    if (lists->remaining() > 0)
	{
        glPushMatrix();
        glTranslatef(x - xOff, y - yOff, z - zOff);
        glCallLists(lists);
        glPopMatrix();
    }
}

void OffsettedRenderList::clear()
{
    inited = false;
    rendered = false;
}