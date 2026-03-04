#include "stdafx.h"
#include "WitherBossRenderer.h"
#include "WitherBossModel.h"
#include "MobRenderer.h"
#include "../Minecraft.World/WitherBoss.h"
#include "../Minecraft.Client/BossMobGuiInfo.h"

ResourceLocation WitherBossRenderer::WITHER_ARMOR_LOCATION = ResourceLocation(TN_MOB_WITHER_ARMOR);
ResourceLocation WitherBossRenderer::WITHER_INVULERABLE_LOCATION = ResourceLocation(TN_MOB_WITHER_INVULNERABLE);
ResourceLocation WitherBossRenderer::WITHER_LOCATION = ResourceLocation(TN_MOB_WITHER);


WitherBossRenderer::WitherBossRenderer() : MobRenderer(new WitherBossModel(), 1.0f)
{
	modelVersion = dynamic_cast<WitherBossModel*>(model)->modelVersion();
}

void WitherBossRenderer::render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a)
{
	shared_ptr<WitherBoss> mob = dynamic_pointer_cast<WitherBoss>(entity);

	BossMobGuiInfo::setBossHealth(mob, true);

	int modelVersion = dynamic_cast<WitherBossModel*>(model)->modelVersion();
	if (modelVersion != this->modelVersion)
	{
		this->modelVersion = modelVersion;
		model = new WitherBossModel();
	}
	MobRenderer::render(entity, x, y, z, rot, a);
}

ResourceLocation *WitherBossRenderer::getTextureLocation(shared_ptr<Entity> entity)
{
	shared_ptr<WitherBoss> mob = dynamic_pointer_cast<WitherBoss>(entity);

	int invulnerableTicks = mob->getInvulnerableTicks();
	if (invulnerableTicks <= 0 || ( (invulnerableTicks <= (SharedConstants::TICKS_PER_SECOND * 4)) && (invulnerableTicks / 5) % 2 == 1) )
	{
		return &WITHER_LOCATION;
	}
	return &WITHER_INVULERABLE_LOCATION;
}

void WitherBossRenderer::scale(shared_ptr<LivingEntity> _mob, float a)
{
	shared_ptr<WitherBoss> mob = dynamic_pointer_cast<WitherBoss>(_mob);
	int inTicks = mob->getInvulnerableTicks();
	if (inTicks > 0)
	{
		float scale = 2.0f - (((float) inTicks - a) / (SharedConstants::TICKS_PER_SECOND * 11)) * .5f;
		glScalef(scale, scale, scale);
	}
	else
	{
		glScalef(2, 2, 2);
	}
}

int WitherBossRenderer::prepareArmor(shared_ptr<LivingEntity> entity, int layer, float a)
{
	shared_ptr<WitherBoss> mob = dynamic_pointer_cast<WitherBoss>(entity);

	if (mob->isPowered())
	{
		if (mob->isInvisible())
		{
			glDepthMask(false);
		}
		else
		{
			glDepthMask(true);
		}

		if (layer == 1)
		{
			float time = mob->tickCount + a;
			bindTexture(&WITHER_ARMOR_LOCATION);
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			float uo = cos(time * 0.02f) * 3;
			float vo = time * 0.01f;
			glTranslatef(uo, vo, 0);
			setArmor(model);
			glMatrixMode(GL_MODELVIEW);
			glEnable(GL_BLEND);
			float br = 0.5f;
			glColor4f(br, br, br, 1);
			glDisable(GL_LIGHTING);
			glBlendFunc(GL_ONE, GL_ONE);
			glTranslatef(0, -.01f, 0);
			glScalef(1.1f, 1.1f, 1.1f);
			return 1;
		}
		if (layer == 2)
		{
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glEnable(GL_LIGHTING);
			glDisable(GL_BLEND);
		}
	}
	return -1;
}

int WitherBossRenderer::prepareArmorOverlay(shared_ptr<LivingEntity> entity, int layer, float a)
{
	return -1;
}