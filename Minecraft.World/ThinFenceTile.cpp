#include "stdafx.h"
#include "ThinFenceTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"

ThinFenceTile::ThinFenceTile(int id, const wstring &tex, const wstring &edgeTex, Material *material, bool dropsResources) : Tile(id, material,isSolidRender())
{
	iconSide = NULL;
	edgeTexture = edgeTex;
	this->dropsResources = dropsResources;
	this->texture = tex;
}

int ThinFenceTile::getResource(int data, Random *random, int playerBonusLevel)
{
    if (!dropsResources)
	{
        return 0;
    }
    return Tile::getResource(data, random, playerBonusLevel);
}

bool ThinFenceTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool ThinFenceTile::isCubeShaped()
{
	return false;
}

int ThinFenceTile::getRenderShape()
{
	return material == Material::glass ? Tile::SHAPE_THIN_PANE : Tile::SHAPE_IRON_FENCE;
}

bool ThinFenceTile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face)
{
    int id = level->getTile(x, y, z);
    if (id == this->id) return false;
    return Tile::shouldRenderFace(level, x, y, z, face);
}

void ThinFenceTile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
    bool n = attachsTo(level->getTile(x, y, z - 1));
    bool s = attachsTo(level->getTile(x, y, z + 1));
    bool w = attachsTo(level->getTile(x - 1, y, z));
    bool e = attachsTo(level->getTile(x + 1, y, z));

    if ((w && e) || (!w && !e && !n && !s))
	{
        setShape(0, 0, 7.0f / 16.0f, 1, 1, 9.0f / 16.0f);
        Tile::addAABBs(level, x, y, z, box, boxes, source);
    }
	else if (w && !e)
	{
        setShape(0, 0, 7.0f / 16.0f, .5f, 1, 9.0f / 16.0f);
        Tile::addAABBs(level, x, y, z, box, boxes, source);
    }
	else if (!w && e)
	{
        setShape(.5f, 0, 7.0f / 16.0f, 1, 1, 9.0f / 16.0f);
        Tile::addAABBs(level, x, y, z, box, boxes, source);
    }
    if ((n && s) || (!w && !e && !n && !s))
	{
        setShape(7.0f / 16.0f, 0, 0, 9.0f / 16.0f, 1, 1);
        Tile::addAABBs(level, x, y, z, box, boxes, source);
    }
	else if (n && !s)
	{
        setShape(7.0f / 16.0f, 0, 0, 9.0f / 16.0f, 1, .5f);
        Tile::addAABBs(level, x, y, z, box, boxes, source);
    }
	else if (!n && s)
	{
        setShape(7.0f / 16.0f, 0, .5f, 9.0f / 16.0f, 1, 1);
        Tile::addAABBs(level, x, y, z, box, boxes, source);
    }
}

void ThinFenceTile::updateDefaultShape()
{
	setShape(0, 0, 0, 1, 1, 1);
}

void ThinFenceTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
    float minX = 7.0f / 16.0f;
    float maxX = 9.0f / 16.0f;
    float minZ = 7.0f / 16.0f;
    float maxZ = 9.0f / 16.0f;

    bool n = attachsTo(level->getTile(x, y, z - 1));
    bool s = attachsTo(level->getTile(x, y, z + 1));
    bool w = attachsTo(level->getTile(x - 1, y, z));
    bool e = attachsTo(level->getTile(x + 1, y, z));

    if ((w && e) || (!w && !e && !n && !s))
	{
        minX = 0;
        maxX = 1;
    }
	else if (w && !e)
	{
        minX = 0;
    }
	else if (!w && e)
	{
        maxX = 1;
    }
    if ((n && s) || (!w && !e && !n && !s))
	{
        minZ = 0;
        maxZ = 1;
    }
	else if (n && !s)
	{
        minZ = 0;
    }
	else if (!n && s)
	{
        maxZ = 1;
    }
	setShape(minX, 0, minZ, maxX, 1, maxZ);
}

Icon *ThinFenceTile::getEdgeTexture()
{
	return iconSide;
}

bool ThinFenceTile::attachsTo(int tile)
{
	return Tile::solid[tile] || tile == id || tile == Tile::glass_Id || tile == Tile::stained_glass_Id || tile == Tile::stained_glass_pane_Id;
}

bool ThinFenceTile::isSilkTouchable()
{
	return true;
}

shared_ptr<ItemInstance> ThinFenceTile::getSilkTouchItemInstance(int data)
{
	return shared_ptr<ItemInstance>(new ItemInstance(id, 1, data));
}

void ThinFenceTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(texture);
	iconSide = iconRegister->registerIcon(edgeTexture);
}
