#include "stdafx.h"
#include "SlimeRenderer.h"	
#include "../Minecraft.World/net.minecraft.world.entity.monster.h"

ResourceLocation SlimeRenderer::SLIME_LOCATION = ResourceLocation(TN_MOB_SLIME);

SlimeRenderer::SlimeRenderer(Model *model, Model *armor, float shadow) : MobRenderer(model, shadow)
{
	this->armor = armor;
}

int SlimeRenderer::prepareArmor(shared_ptr<LivingEntity> _slime, int layer, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<Slime> slime = dynamic_pointer_cast<Slime>(_slime);

	if (slime->isInvisible()) 
	{
		return 0;
	}
    if (layer == 0)
	{
        setArmor(armor);

        glEnable(GL_NORMALIZE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        return 1;
    }
    if (layer == 1)
	{
        glDisable(GL_BLEND);
        glColor4f(1, 1, 1, 1);
    }
    return -1;
}

void SlimeRenderer::scale(shared_ptr<LivingEntity> _slime, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<Slime> slime = dynamic_pointer_cast<Slime>(_slime);

    float size = static_cast<float>(slime->getSize());
    float ss = (slime->oSquish + (slime->squish - slime->oSquish) * a) / (size * 0.5f + 1);
    float w = 1 / (ss + 1);
    glScalef(w * size, 1 / w * size, w * size);
}

ResourceLocation *SlimeRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &SLIME_LOCATION;
}