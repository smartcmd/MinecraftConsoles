#include "stdafx.h"
#include "TheEndPortalFrameTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"
#include "Facing.h"

const wstring TheEndPortalFrameTile::TEXTURE_EYE = L"endframe_eye";

TheEndPortalFrameTile::TheEndPortalFrameTile(int id) : Tile(id, Material::glass, isSolidRender() )
{
	iconTop = NULL;
	iconEye = NULL;
}

Icon *TheEndPortalFrameTile::getTexture(int face, int data)
{
    if (face == Facing::UP)
	{
        return iconTop;
    }
    if (face == Facing::DOWN)
	{
        return Tile::whiteStone->getTexture(face);
    }
    return icon;
}

void TheEndPortalFrameTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"endframe_side");
	iconTop = iconRegister->registerIcon(L"endframe_top");
	iconEye = iconRegister->registerIcon(L"endframe_eye");
}

Icon *TheEndPortalFrameTile::getEye()
{
	return iconEye;
}

bool TheEndPortalFrameTile::isSolidRender(bool isServerLevel)
{
	return false;
}

int TheEndPortalFrameTile::getRenderShape()
{
	return SHAPE_PORTAL_FRAME;
}

void TheEndPortalFrameTile::updateDefaultShape()
{
	setShape(0, 0, 0, 1, 13.0f / 16.0f, 1);
}

void TheEndPortalFrameTile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
    setShape(0, 0, 0, 1, 13.0f / 16.0f, 1);
    Tile::addAABBs(level, x, y, z, box, boxes, source);

    int data = level->getData(x, y, z);
    if (hasEye(data))
	{
        setShape(5.0f / 16.0f, 13.0f / 16.0f, 5.0f / 16.0f, 11.0f / 16.0f, 1, 11.0f / 16.0f);
        Tile::addAABBs(level, x, y, z, box, boxes, source);
    }
    updateDefaultShape();
}

bool TheEndPortalFrameTile::hasEye(int data)
{
	return (data & EYE_BIT) != 0;
}

int TheEndPortalFrameTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return 0;
}

void TheEndPortalFrameTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by)
{
    int dir = (((Mth::floor(by->yRot * 4 / (360) + 0.5)) & 3) + 2) % 4;
    level->setData(x, y, z, dir);
}
