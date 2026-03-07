#include "stdafx.h"
#include "SkullTileRenderer.h"
#include "SkeletonHeadModel.h"
#include "PlayerRenderer.h"
#include "../Minecraft.World/SkullTileEntity.h"
#include "../Minecraft.World/net.minecraft.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"

SkullTileRenderer *SkullTileRenderer::instance = NULL;

ResourceLocation SkullTileRenderer::SKELETON_LOCATION = ResourceLocation(TN_MOB_SKELETON);
ResourceLocation SkullTileRenderer::WITHER_SKELETON_LOCATION = ResourceLocation(TN_MOB_WITHER_SKELETON);
ResourceLocation SkullTileRenderer::ZOMBIE_LOCATION = ResourceLocation(TN_MOB_ZOMBIE);
ResourceLocation SkullTileRenderer::CREEPER_LOCATION = ResourceLocation(TN_MOB_CREEPER);

SkullTileRenderer::SkullTileRenderer()
{
	skeletonModel = new SkeletonHeadModel(0, 0, 64, 32);
	zombieModel = new SkeletonHeadModel(0, 0, 64, 64);
}

SkullTileRenderer::~SkullTileRenderer()
{
	delete skeletonModel;
	delete zombieModel;
}

void SkullTileRenderer::render(shared_ptr<TileEntity> _skull, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	shared_ptr<SkullTileEntity> skull = dynamic_pointer_cast<SkullTileEntity>(_skull);
	renderSkull((float) x, (float) y, (float) z, skull->getData() & SkullTile::PLACEMENT_MASK, skull->getRotation() * 360 / 16.0f, skull->getSkullType(), skull->getExtraType());
}

void SkullTileRenderer::init(TileEntityRenderDispatcher *tileEntityRenderDispatcher)
{
	TileEntityRenderer::init(tileEntityRenderDispatcher);
	instance = this;
}

void SkullTileRenderer::renderSkull(float x, float y, float z, int face, float rot, int type, const wstring &extra)
{
	Model *model = skeletonModel;

	switch (type)
	{
	case SkullTileEntity::TYPE_WITHER:
		bindTexture(&WITHER_SKELETON_LOCATION);
		break;
	case SkullTileEntity::TYPE_ZOMBIE:
		bindTexture(&ZOMBIE_LOCATION);
		//model = zombieModel;
		break;
	case SkullTileEntity::TYPE_CHAR:
		//if (!extra.empty())
		//{
		//	wstring url = "http://skins.minecraft.net/MinecraftSkins/" + StringUtil.stripColor(extra) + ".png";

		//	if (!instance->tileEntityRenderDispatcher->textures->hasHttpTexture(url))
		//	{
		//		instance->tileEntityRenderDispatcher->textures->addHttpTexture(url, new MobSkinTextureProcessor());
		//	}

		//	bindTexture(url, "/mob/char.png");
		//}
		//else
		{
			bindTexture(&PlayerRenderer::DEFAULT_LOCATION);
		}
		break;
	case SkullTileEntity::TYPE_CREEPER:
		bindTexture(&CREEPER_LOCATION);
		break;
	case SkullTileEntity::TYPE_SKELETON:
	default:
		bindTexture(&SKELETON_LOCATION);
		break;
	}

	glPushMatrix();
	glDisable(GL_CULL_FACE);

	if (face != Facing::UP)
	{
		switch (face)
		{
		case Facing::NORTH:
			glTranslatef(x + 0.5f, y + .25f, z + 0.74f);
			break;
		case Facing::SOUTH:
			glTranslatef(x + 0.5f, y + .25f, z + 0.26f);
			rot = 180.0f;
			break;
		case Facing::WEST:
			glTranslatef(x + 0.74f, y + .25f, z + 0.5f);
			rot = 270.0f;
			break;
		case Facing::EAST:
		default:
			glTranslatef(x + 0.26f, y + .25f, z + 0.5f);
			rot = 90.0f;
			break;
		}
	}
	else
	{
		glTranslatef(x + 0.5f, y, z + 0.5f);
	}

	float scale = 1 / 16.0f;
	glEnable(GL_RESCALE_NORMAL);
	glScalef(-1, -1, 1);

	glEnable(GL_ALPHA_TEST);

	model->render(nullptr, 0, 0, 0, rot, 0, scale,true);

	glPopMatrix();
}
