#include "stdafx.h"
#include "SpiderRenderer.h"
#include "SpiderModel.h"
#include "..\Minecraft.World\net.minecraft.world.entity.monster.h"

SpiderRenderer::SpiderRenderer() : MobRenderer(new SpiderModel(), 1.0f)
{
	this->setArmor(new SpiderModel());
}

float SpiderRenderer::getFlipDegrees(shared_ptr<Mob> spider)
{
	return 180;
}

int SpiderRenderer::prepareArmor(shared_ptr<Mob> _spider, int layer, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<Spider> spider = dynamic_pointer_cast<Spider>(_spider);

    if (layer!=0) return -1;
	MemSect(31);
    bindTexture(TN_MOB_SPIDER_EYES);	// 4J was L"/mob/spider_eyes.png"
	MemSect(0);
	// 4J - changes brought forward from 1.8.2
    float br = 1.0f; // was (1-spider->getBrightness(1))*0.5f;
    glEnable(GL_BLEND);
	// 4J Stu - We probably don't need to do this on 360 either (as we force it back on the renderer)
	// However we do want it off for other platforms that don't force it on in the render lib CBuff handling
	// Several texture packs have fully transparent bits that break if this is off
#ifdef _XBOX
    glDisable(GL_ALPHA_TEST);
#endif
	// 4J - changes brought forward from 1.8.2
	glBlendFunc(GL_ONE, GL_ONE);
	if (spider->isInvisible())	glDepthMask(false);
	else						glDepthMask(true);

    if (SharedConstants::TEXTURE_LIGHTING)
	{
		// 4J - was 0xf0f0 but that looks like it is a mistake - maybe meant to be 0xf000f0 to enable both sky & block lighting? choosing 0x00f0 here instead
		// as most likely replicates what the java game does, without breaking our lighting (which doesn't like UVs out of the 0 to 255 range)
        int col = 0x00f0;			
        int u = col % 65536;
        int v = col / 65536;

        glMultiTexCoord2f(GL_TEXTURE1, u / 1.0f, v / 1.0f);
        glColor4f(1, 1, 1, 1);
    }
	// 4J - this doesn't seem right - surely there should be an else in here?
    glColor4f(1, 1, 1, br);
    return 1;
}

void SpiderRenderer::scale(shared_ptr<Mob> _mob, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<Spider> mob = dynamic_pointer_cast<Spider>(_mob);
	float scale = mob->getModelScale();
	glScalef(scale, scale, scale);
}