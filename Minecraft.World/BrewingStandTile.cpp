#include "stdafx.h"
#include "BrewingStandTile.h"
#include "BrewingStandTileEntity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.inventory.h"

BrewingStandTile::BrewingStandTile(int id) : BaseEntityTile(id, Material::metal, isSolidRender())
{
	random = new Random();
	iconBase = nullptr;
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
	return std::make_shared<BrewingStandTileEntity>();
}

bool BrewingStandTile::isCubeShaped()
{
	return false;
}

void BrewingStandTile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
	setShape(7.0f / 16.0f, 0, 7.0f / 16.0f, 9.0f / 16.0f, 14.0f / 16.0f, 9.0f / 16.0f);
	BaseEntityTile::addAABBs(level, x, y, z, box, boxes, source);
	updateDefaultShape();
	BaseEntityTile::addAABBs(level, x, y, z, box, boxes, source);
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
	if (brewingStand != nullptr) player->openBrewingStand(brewingStand);

	return true;
}

void BrewingStandTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	if (itemInstance->hasCustomHoverName())
	{
		dynamic_pointer_cast<BrewingStandTileEntity>( level->getTileEntity(x, y, z))->setCustomName(itemInstance->getHoverName());
	}
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
	if (tileEntity != nullptr && ( dynamic_pointer_cast<BrewingStandTileEntity>(tileEntity) != nullptr) )
	{
		shared_ptr<BrewingStandTileEntity> container = dynamic_pointer_cast<BrewingStandTileEntity>(tileEntity);
		for (int i = 0; i < container->getContainerSize(); i++)
		{
			shared_ptr<ItemInstance> item = container->getItem(i);
			if (item != nullptr)
			{
				float xo = random->nextFloat() * 0.8f + 0.1f;
				float yo = random->nextFloat() * 0.8f + 0.1f;
				float zo = random->nextFloat() * 0.8f + 0.1f;

				while (item->count > 0)
				{
					int count = random->nextInt(21) + 10;
					if (count > item->count) count = item->count;
					item->count -= count;

					shared_ptr<ItemEntity> itemEntity = std::make_shared<ItemEntity>(level, x + xo, y + yo, z + zo, shared_ptr<ItemInstance>(new ItemInstance(item->id, count, item->getAuxValue())));
					float pow = 0.05f;
					itemEntity->xd = static_cast<float>(random->nextGaussian()) * pow;
					itemEntity->yd = static_cast<float>(random->nextGaussian()) * pow + 0.2f;
					itemEntity->zd = static_cast<float>(random->nextGaussian()) * pow;
					if (item->hasTag())
					{
						itemEntity->getItem()->setTag(static_cast<CompoundTag *>(item->getTag()->copy()));
					}
					level->addEntity(itemEntity);
				}
			}
		}
	}
	BaseEntityTile::onRemove(level, x, y, z, id, data);
}

int BrewingStandTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::brewingStand_Id;
}

int BrewingStandTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::brewingStand_Id;
}

bool BrewingStandTile::hasAnalogOutputSignal()
{
	return true;
}

int BrewingStandTile::getAnalogOutputSignal(Level *level, int x, int y, int z, int dir)
{
	return AbstractContainerMenu::getRedstoneSignalFromContainer(dynamic_pointer_cast<Container>(level->getTileEntity(x, y, z)));
}

void BrewingStandTile::registerIcons(IconRegister *iconRegister)
{
	BaseEntityTile::registerIcons(iconRegister);
	iconBase = iconRegister->registerIcon(getIconName() + L"_base");
}

Icon *BrewingStandTile::getBaseTexture()
{
	return iconBase;
}