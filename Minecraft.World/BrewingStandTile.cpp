#include "stdafx.h"
#include "BrewingStandTile.h"
#include "BrewingStandTileEntity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.h"

const wstring BrewingStandTile::TEXTURE_BASE = L"brewingStand_base";

BrewingStandTile::BrewingStandTile(int id) : EntityTile(id, Material::metal, isSolidRender())
{
	random = new Random();
	iconBase = NULL;
}

BrewingStandTile::~BrewingStandTile()
{
	delete random;
}

bool BrewingStandTile::isSolidRender(bool isServerLevel)
{
	return false;
}

int BrewingStandTile::getRenderShape()
{
	return SHAPE_BREWING_STAND;
}

shared_ptr<TileEntity> BrewingStandTile::newTileEntity(Level *level)
{
	return shared_ptr<TileEntity>(new BrewingStandTileEntity());
}

bool BrewingStandTile::isCubeShaped()
{
	return false;
}

void BrewingStandTile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
    setShape(7.0f / 16.0f, 0, 7.0f / 16.0f, 9.0f / 16.0f, 14.0f / 16.0f, 9.0f / 16.0f);
    EntityTile::addAABBs(level, x, y, z, box, boxes, source);
    updateDefaultShape();
    EntityTile::addAABBs(level, x, y, z, box, boxes, source);
}

void BrewingStandTile::updateDefaultShape()
{
	setShape(0, 0, 0, 1, 2.0f / 16.0f, 1);
}

bool BrewingStandTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if(soundOnly) return false;

    if (level->isClientSide)
	{
        return true;
    }
    shared_ptr<BrewingStandTileEntity> brewingStand = dynamic_pointer_cast<BrewingStandTileEntity>(level->getTileEntity(x, y, z));
    if (brewingStand != NULL) player->openBrewingStand(brewingStand);

	return true;
}

void BrewingStandTile::animateTick(Level *level, int xt, int yt, int zt, Random *random)
{
    double x = xt + 0.4f + random->nextFloat() * 0.2f;
    double y = yt + 0.7f + random->nextFloat() * 0.3f;
    double z = zt + 0.4f + random->nextFloat() * 0.2f;


    level->addParticle(eParticleType_smoke, x, y, z, 0, 0, 0);
}

void BrewingStandTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
    shared_ptr<TileEntity> tileEntity = level->getTileEntity(x, y, z);
    if (tileEntity != NULL && ( dynamic_pointer_cast<BrewingStandTileEntity>(tileEntity) != NULL) )
	{
        shared_ptr<BrewingStandTileEntity> container = dynamic_pointer_cast<BrewingStandTileEntity>(tileEntity);
        for (int i = 0; i < container->getContainerSize(); i++)
		{
            shared_ptr<ItemInstance> item = container->getItem(i);
            if (item != NULL)
			{
                float xo = random->nextFloat() * 0.8f + 0.1f;
                float yo = random->nextFloat() * 0.8f + 0.1f;
                float zo = random->nextFloat() * 0.8f + 0.1f;

                while (item->count > 0)
				{
                    int count = random->nextInt(21) + 10;
                    if (count > item->count) count = item->count;
                    item->count -= count;

                    shared_ptr<ItemEntity> itemEntity = shared_ptr<ItemEntity>(new ItemEntity(level, x + xo, y + yo, z + zo, shared_ptr<ItemInstance>( new ItemInstance(item->id, count, item->getAuxValue()))));
                    float pow = 0.05f;
                    itemEntity->xd = (float) random->nextGaussian() * pow;
                    itemEntity->yd = (float) random->nextGaussian() * pow + 0.2f;
                    itemEntity->zd = (float) random->nextGaussian() * pow;
                    if (item->hasTag())
					{
                        itemEntity->getItem()->setTag((CompoundTag *) item->getTag()->copy());
                    }
                    level->addEntity(itemEntity);
                }
            }
        }
    }
    EntityTile::onRemove(level, x, y, z, id, data);
}

int BrewingStandTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::brewingStand_Id;
}

int BrewingStandTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::brewingStand_Id;
}

void BrewingStandTile::registerIcons(IconRegister *iconRegister)
{
	EntityTile::registerIcons(iconRegister);
	iconBase = iconRegister->registerIcon(TEXTURE_BASE);
}

Icon *BrewingStandTile::getBaseTexture()
{
	return iconBase;
}