#include "stdafx.h"
#include "HorseRenderer.h"
#include "MobRenderer.h"
#include "EntityRenderDispatcher.h"
#include "../Minecraft.World/net.minecraft.world.entity.animal.h"

ResourceLocation HorseRenderer::HORSE_LOCATION = ResourceLocation(TN_MOB_HORSE_WHITE);
ResourceLocation HorseRenderer::HORSE_MULE_LOCATION = ResourceLocation(TN_MOB_MULE);
ResourceLocation HorseRenderer::HORSE_DONKEY_LOCATION = ResourceLocation(TN_MOB_DONKEY);
ResourceLocation HorseRenderer::HORSE_ZOMBIE_LOCATION = ResourceLocation(TN_MOB_HORSE_ZOMBIE);
ResourceLocation HorseRenderer::HORSE_SKELETON_LOCATION = ResourceLocation(TN_MOB_HORSE_SKELETON);

std::map<wstring, ResourceLocation *> HorseRenderer::LAYERED_LOCATION_CACHE;

HorseRenderer::HorseRenderer(Model *model, float f) : MobRenderer(model, f)
{
}

void HorseRenderer::adjustHeight(shared_ptr<PathfinderMob> mob, float FHeight)
{
    glTranslatef(0.0F, FHeight, 0.0F);
}

void HorseRenderer::scale(shared_ptr<LivingEntity> entityliving, float f)
{
    float sizeFactor = 1.0f;

	int type = dynamic_pointer_cast<EntityHorse>(entityliving)->getType();
    if (type == EntityHorse::TYPE_DONKEY)
	{
        sizeFactor *= 0.87F;
    }
	else if (type == EntityHorse::TYPE_MULE)
	{
        sizeFactor *= 0.92F;
    }
    glScalef(sizeFactor, sizeFactor, sizeFactor);
    MobRenderer::scale(entityliving, f);
}

void HorseRenderer::renderModel(shared_ptr<LivingEntity> mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale)
{
    if (mob->isInvisible())
	{
        model->setupAnim(wp, ws, bob, headRotMinusBodyRot, headRotx, scale, mob);
    }
	else
	{
        EntityRenderer::bindTexture(mob);
        model->render(mob, wp, ws, bob, headRotMinusBodyRot, headRotx, scale, true);
		// Ensure that any extra layers of texturing are disabled after rendering this horse
		RenderManager.TextureBind(-1);
    }
}

void HorseRenderer::bindTexture(ResourceLocation *location)
{
	// Set up (potentially) multiple texture layers for the horse
	entityRenderDispatcher->textures->bindTextureLayers(location);
}

ResourceLocation *HorseRenderer::getTextureLocation(shared_ptr<Entity> entity)
{
	shared_ptr<EntityHorse> horse = dynamic_pointer_cast<EntityHorse>(entity);

    if (!horse->hasLayeredTextures())
	{
        switch (horse->getType())
		{
		default:
		case EntityHorse::TYPE_HORSE:		return &HORSE_LOCATION;
		case EntityHorse::TYPE_MULE:		return &HORSE_MULE_LOCATION;
		case EntityHorse::TYPE_DONKEY:		return &HORSE_DONKEY_LOCATION;
		case EntityHorse::TYPE_UNDEAD:		return &HORSE_ZOMBIE_LOCATION;
		case EntityHorse::TYPE_SKELETON:	return &HORSE_SKELETON_LOCATION;
		}
    }

    return getOrCreateLayeredTextureLocation(horse);
}

ResourceLocation *HorseRenderer::getOrCreateLayeredTextureLocation(shared_ptr<EntityHorse> horse)
{
    wstring textureName = horse->getLayeredTextureHashName();

    auto it = LAYERED_LOCATION_CACHE.find(textureName);

    ResourceLocation *location;
	if (it != LAYERED_LOCATION_CACHE.end())
	{
		location = it->second;
	}
	else
	{
		LAYERED_LOCATION_CACHE[textureName] = new ResourceLocation(horse->getLayeredTextureLayers());

		it = LAYERED_LOCATION_CACHE.find(textureName);
		location = it->second;
	}

    return location;
}