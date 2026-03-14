#include "stdafx.h"
#include "CreeperRenderer.h"
#include "CreeperModel.h"
#include "../Minecraft.World/net.minecraft.world.entity.monster.h"
#include "../Minecraft.World/Mth.h"

ResourceLocation CreeperRenderer::POWER_LOCATION = ResourceLocation(TN_POWERED_CREEPER);
ResourceLocation CreeperRenderer::CREEPER_LOCATION = ResourceLocation(TN_MOB_CREEPER);

CreeperRenderer::CreeperRenderer() : MobRenderer(new CreeperModel(), 0.5f)
{
	armorModel = new CreeperModel(2);
}

void CreeperRenderer::scale(shared_ptr<LivingEntity> mob, float a)
{
    shared_ptr<Creeper> creeper = dynamic_pointer_cast<Creeper>(mob);

    float g = creeper->getSwelling(a);

    float wobble = 1.0f + Mth::sin(g * 100) * g * 0.01f;
    if (g < 0) g = 0;
    if (g > 1) g = 1;
    g = g * g;
    g = g * g;
    float s = (1.0f + g * 0.4f) * wobble;
    float hs = (1.0f + g * 0.1f) / wobble;
    glScalef(s, hs, s);
}

int CreeperRenderer::getOverlayColor(shared_ptr<LivingEntity> mob, float br, float a)
{
	shared_ptr<Creeper> creeper = dynamic_pointer_cast<Creeper>(mob);

    float step = creeper->getSwelling(a);

    if (static_cast<int>(step * 10) % 2 == 0) return 0;

    int _a = static_cast<int>(step * 0.2f * 255) + 25;	// 4J - added 25 here as our entities are rendered with alpha test still enabled, and so anything less is invisible
    if (_a < 0) _a = 0;
    if (_a > 255) _a = 255;

    int r = 255;
    int g = 255;
    int b = 255;

    return (_a << 24) | (r << 16) | (g << 8) | b;
}

int CreeperRenderer::prepareArmor(shared_ptr<LivingEntity> _mob, int layer, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<Creeper> mob = dynamic_pointer_cast<Creeper>(_mob);
    if (mob->isPowered())
	{
		if (mob->isInvisible())	glDepthMask(false);
		else					glDepthMask(true);

        if (layer == 1)
		{
            float time = mob->tickCount + a;
            bindTexture(&POWER_LOCATION);
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            float uo = time * 0.01f;
            float vo = time * 0.01f;
            glTranslatef(uo, vo, 0);
            setArmor(armorModel);
            glMatrixMode(GL_MODELVIEW);
            glEnable(GL_BLEND);
            float br = 0.5f;
            glColor4f(br, br, br, 1);
            glDisable(GL_LIGHTING);
            glBlendFunc(GL_ONE, GL_ONE);
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

int CreeperRenderer::prepareArmorOverlay(shared_ptr<LivingEntity> mob, int layer, float a)
{
	return -1;
}

ResourceLocation *CreeperRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &CREEPER_LOCATION;
}