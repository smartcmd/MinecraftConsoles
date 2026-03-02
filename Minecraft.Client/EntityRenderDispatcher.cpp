#include "stdafx.h"
#include "EntityRenderDispatcher.h"
#include "..\Minecraft.World\net.minecraft.world.entity.projectile.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"
#include "..\Minecraft.World\net.minecraft.world.entity.monster.h"
#include "..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\Minecraft.World\net.minecraft.world.entity.global.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.entity.boss.enderdragon.h"
#include "..\Minecraft.World\net.minecraft.world.entity.npc.h"
#include "..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.item.alchemy.h"
#include "SpiderRenderer.h"
#include "PigRenderer.h"
#include "SheepRenderer.h"
#include "CowRenderer.h"
#include "WolfRenderer.h"
#include "ChickenRenderer.h"
#include "CreeperRenderer.h"
#include "SlimeRenderer.h"
#include "PlayerRenderer.h"
#include "GhastRenderer.h"
#include "SquidRenderer.h"
#include "MobRenderer.h"
#include "GiantMobRenderer.h"
#include "EntityRenderer.h"
#include "PaintingRenderer.h"
#include "ArrowRenderer.h"
#include "FireballRenderer.h"
#include "ItemRenderer.h"
#include "ItemSpriteRenderer.h"
#include "TntRenderer.h"
#include "FallingTileRenderer.h"
#include "MinecartRenderer.h"
#include "BoatRenderer.h"
#include "FishingHookRenderer.h"
#include "LightningBoltRenderer.h"
#include "HumanoidMobRenderer.h"
#include "DefaultRenderer.h"
#include "EndermanRenderer.h"
#include "ExperienceOrbRenderer.h"
#include "SilverfishRenderer.h"
#include "MushroomCowRenderer.h"
#include "SnowmanRenderer.h"
#include "LavaSlimeRenderer.h"
#include "VillagerRenderer.h"
#include "EnderDragonRenderer.h"
#include "EnderCrystalRenderer.h"
#include "BlazeRenderer.h"
#include "SpiderModel.h"
#include "PigModel.h"
#include "SheepModel.h"
#include "CowModel.h"
#include "WolfModel.h"
#include "ChickenModel.h"
#include "CreeperModel.h"
#include "SlimeModel.h"
#include "GhastModel.h"
#include "SquidModel.h"
#include "MinecartModel.h"
#include "BoatModel.h"
#include "HumanoidModel.h"
#include "SheepFurModel.h"
#include "SkeletonModel.h"
#include "Options.h"
#include "ItemFrameRenderer.h"
#include "OzelotRenderer.h"
#include "VillagerGolemRenderer.h"
#include "OzelotModel.h"
#include "ZombieRenderer.h"

double EntityRenderDispatcher::xOff = 0.0;
double EntityRenderDispatcher::yOff = 0.0;
double EntityRenderDispatcher::zOff = 0.0;

EntityRenderDispatcher *EntityRenderDispatcher::instance = NULL;

void EntityRenderDispatcher::staticCtor()
{
	instance = new EntityRenderDispatcher();
}

EntityRenderDispatcher::EntityRenderDispatcher()
{
	glEnable(GL_LIGHTING);
	renderers[eTYPE_SPIDER] = new SpiderRenderer();
	renderers[eTYPE_CAVESPIDER] = new SpiderRenderer();
	renderers[eTYPE_PIG] = new PigRenderer(new PigModel(), new PigModel(0.5f), 0.7f);
	renderers[eTYPE_SHEEP] = new SheepRenderer(new SheepModel(), new SheepFurModel(), 0.7f);
	renderers[eTYPE_COW] = new CowRenderer(new CowModel(), 0.7f);
	renderers[eTYPE_MUSHROOMCOW] = new MushroomCowRenderer(new CowModel(), 0.7f);
	renderers[eTYPE_WOLF] = new WolfRenderer(new WolfModel(), new WolfModel(), 0.5f);
	renderers[eTYPE_CHICKEN] = new ChickenRenderer(new ChickenModel(), 0.3f);
	renderers[eTYPE_OZELOT] = new OzelotRenderer(new OzelotModel(), 0.4f);
	renderers[eTYPE_SILVERFISH] = new SilverfishRenderer();
	renderers[eTYPE_CREEPER] = new CreeperRenderer();
	renderers[eTYPE_ENDERMAN] = new EndermanRenderer();
	renderers[eTYPE_SNOWMAN] = new SnowManRenderer();
	renderers[eTYPE_SKELETON] = new HumanoidMobRenderer(new SkeletonModel(), 0.5f);
	renderers[eTYPE_BLAZE] = new BlazeRenderer();
	renderers[eTYPE_ZOMBIE] = new ZombieRenderer();
	renderers[eTYPE_PIGZOMBIE] = new HumanoidMobRenderer(new ZombieModel(), 0.5f);
	renderers[eTYPE_SLIME] = new SlimeRenderer(new SlimeModel(16), new SlimeModel(0), 0.25f);
	renderers[eTYPE_LAVASLIME] = new LavaSlimeRenderer();
	renderers[eTYPE_PLAYER] = new PlayerRenderer();
	renderers[eTYPE_GIANT] = new GiantMobRenderer(new ZombieModel(), 0.5f, 6);
	renderers[eTYPE_GHAST] = new GhastRenderer();
	renderers[eTYPE_SQUID] = new SquidRenderer(new SquidModel(), 0.7f);
	renderers[eTYPE_VILLAGER] = new VillagerRenderer();
	renderers[eTYPE_VILLAGERGOLEM] = new VillagerGolemRenderer();
	renderers[eTYPE_MOB] = new MobRenderer(new HumanoidModel(), 0.5f);
	renderers[eTYPE_ENDERDRAGON] = new EnderDragonRenderer();
	renderers[eTYPE_ENDER_CRYSTAL] = new EnderCrystalRenderer();
	renderers[eTYPE_ENTITY] = new DefaultRenderer();
	renderers[eTYPE_PAINTING] = new PaintingRenderer();
	renderers[eTYPE_ITEM_FRAME] = new ItemFrameRenderer();
	renderers[eTYPE_ARROW] = new ArrowRenderer();
	renderers[eTYPE_SNOWBALL] = new ItemSpriteRenderer(Item::snowBall);
	renderers[eTYPE_THROWNENDERPEARL] = new ItemSpriteRenderer(Item::enderPearl);
	renderers[eTYPE_EYEOFENDERSIGNAL] = new ItemSpriteRenderer(Item::eyeOfEnder);
	renderers[eTYPE_THROWNEGG] = new ItemSpriteRenderer(Item::egg);
	renderers[eTYPE_THROWNPOTION] = new ItemSpriteRenderer(Item::potion, PotionBrewing::THROWABLE_MASK);
	renderers[eTYPE_THROWNEXPBOTTLE] = new ItemSpriteRenderer(Item::expBottle);
	renderers[eTYPE_FIREBALL] = new FireballRenderer(2.0f);
	renderers[eTYPE_SMALL_FIREBALL] = new FireballRenderer(0.5f);
	renderers[eTYPE_DRAGON_FIREBALL] = new FireballRenderer(2.0f); // 4J Added TU9
	renderers[eTYPE_ITEMENTITY] = new ItemRenderer();
	renderers[eTYPE_EXPERIENCEORB] = new ExperienceOrbRenderer();
	renderers[eTYPE_PRIMEDTNT] = new TntRenderer();
	renderers[eTYPE_FALLINGTILE] = new FallingTileRenderer();
	renderers[eTYPE_MINECART] = new MinecartRenderer();
	renderers[eTYPE_BOAT] = new BoatRenderer();
	renderers[eTYPE_FISHINGHOOK] = new FishingHookRenderer();
	renderers[eTYPE_LIGHTNINGBOLT] = new LightningBoltRenderer();
	renderers[eTYPE_ARROW] = new ArrowRenderer();
	glDisable(GL_LIGHTING);

	AUTO_VAR(itEnd, renderers.end());
	for( classToRendererMap::iterator it = renderers.begin(); it != itEnd; it++ )
	{
		it->second->init(this);
	}

	isGuiRender = false;	// 4J added
}

EntityRenderer *EntityRenderDispatcher::getRenderer(eINSTANCEOF e)
{
	//EntityRenderer * r = renderers[e];
	AUTO_VAR(it, renderers.find( e )); // 4J Stu - The .at and [] accessors insert elements if they don't exist

	if( it == renderers.end() )
	{
		// New renderer mapping required in above table
		__debugbreak();
	}
	/* 4J - not doing this hierarchical search anymore. We need to explicitly add renderers for any eINSTANCEOF type that we want to be able to render
    if (it == renderers.end() && e != Entity::_class)
	{
        EntityRenderer *r = getRenderer(dynamic_cast<Entity::Class *>( e->getSuperclass() ));
        renderers.insert( classToRendererMap::value_type( e, r ) );
		return r;
		//assert(false);
    }*/
	return it->second;
}

EntityRenderer *EntityRenderDispatcher::getRenderer(shared_ptr<Entity> e)
{
	return getRenderer(e->GetType());
}

void EntityRenderDispatcher::prepare(Level *level, Textures *textures, Font *font, shared_ptr<Mob> player, Options *options, float a)
{
    this->level = level;
    this->textures = textures;
    this->options = options;
    this->cameraEntity = player;
    this->font = font;

    if (player->isSleeping())
	{
        int t = level->getTile(Mth::floor(player->x), Mth::floor(player->y), Mth::floor(player->z));
        if (t == Tile::bed_Id)
		{
            int data = level->getData(Mth::floor(player->x), Mth::floor(player->y), Mth::floor(player->z));

            int direction = data & 3;
            playerRotY = (float)(direction * 90 + 180);
            playerRotX = 0;
        }
    } else {
        playerRotY = player->yRotO + (player->yRot - player->yRotO) * a;
        playerRotX = player->xRotO + (player->xRot - player->xRotO) * a;
    }

	shared_ptr<Player> pl = dynamic_pointer_cast<Player>(player);
	if (pl->ThirdPersonView() == 2)
	{
		playerRotY += 180;
	}

    xPlayer = player->xOld + (player->x - player->xOld) * a;
    yPlayer = player->yOld + (player->y - player->yOld) * a;
    zPlayer = player->zOld + (player->z - player->zOld) * a;

}

void EntityRenderDispatcher::render(shared_ptr<Entity> entity, float a)
{
    double x = entity->xOld + (entity->x - entity->xOld) * a;
    double y = entity->yOld + (entity->y - entity->yOld) * a;
    double z = entity->zOld + (entity->z - entity->zOld) * a;

	// Fix for #61057 - TU7: Gameplay: Boat is glitching when player float forward and turning.
	// Fix to handle the case that yRot and yRotO wrap over the 0/360 line
	float rotDiff = entity->yRot - entity->yRotO;
	if( rotDiff > 180 || rotDiff < -180)
	{
		if(entity->yRot > entity->yRotO)
		{
			rotDiff = (entity->yRot - 360) - entity->yRotO;
		}
		else
		{
			rotDiff = entity->yRot - (entity->yRotO - 360);
		}
	}
    float r = entity->yRotO + (rotDiff) * a;
 
    int col = entity->getLightColor(a);
	if (entity->isOnFire())
	{
		col = SharedConstants::FULLBRIGHT_LIGHTVALUE;
	}
    int u = col % 65536;
    int v = col / 65536;
    glMultiTexCoord2f(GL_TEXTURE1, u / 1.0f, v / 1.0f);
    glColor4f(1, 1, 1, 1);

    render(entity, x - xOff, y - yOff, z - zOff, r, a);
}

void EntityRenderDispatcher::render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a, bool bItemFrame, bool bRenderPlayerShadow)
{
	EntityRenderer *renderer = getRenderer(entity);
    if (renderer != NULL)
	{	
		renderer->SetItemFrame(bItemFrame);
		
		renderer->render(entity, x, y, z, rot, a);
        renderer->postRender(entity, x, y, z, rot, a, bRenderPlayerShadow);
    }
}

double EntityRenderDispatcher::distanceToSqr(double x, double y, double z)
{
    double xd = x - xPlayer;
    double yd = y - yPlayer;
    double zd = z - zPlayer;
    return xd * xd + yd * yd + zd * zd;
}

Font *EntityRenderDispatcher::getFont()
{
	return font;
}

void EntityRenderDispatcher::registerTerrainTextures(IconRegister *iconRegister)
{
	//for (EntityRenderer<? extends Entity> renderer : renderers.values())
	for(AUTO_VAR(it, renderers.begin()); it != renderers.end(); ++it)
	{
		EntityRenderer *renderer = it->second;
		renderer->registerTerrainTextures(iconRegister);
	}
}