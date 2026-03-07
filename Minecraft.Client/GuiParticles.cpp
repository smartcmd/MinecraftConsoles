#include "stdafx.h"
#include "GuiParticles.h"
#include "GuiParticle.h"
#include "Textures.h"

GuiParticles::GuiParticles(Minecraft *mc)
{
	this->mc = mc;
}

void GuiParticles::tick()
{
    for (unsigned int i = 0; i < particles.size(); i++)
	{
        GuiParticle *gp = particles[i];

        gp->preTick();
        gp->tick(this);

        if (gp->removed)
		{
            particles.erase(particles.begin()+i);
			i--;
        }
    }
}

void GuiParticles::add(GuiParticle *guiParticle)
{
    particles.push_back(guiParticle);
    guiParticle->preTick();
}

void GuiParticles::render(float a)
{
	// 4J Stu - Never used
#if 0
    mc->textures->bindTexture(L"/gui/particles.png");

	for ( GuiParticle *gp : particles )
	{
        int xx = (int) (gp->xo + (gp->x - gp->xo) * a - 4);
        int yy = (int) (gp->yo + (gp->y - gp->yo) * a - 4);

        float alpha = ((float) (gp->oA + (gp->a - gp->oA) * a));
        float r = ((float) (gp->oR + (gp->r - gp->oR) * a));
        float g = ((float) (gp->oG + (gp->g - gp->oG) * a));
        float b = ((float) (gp->oB + (gp->b - gp->oB) * a));

        glColor4f(r, g, b, alpha);
        blit(xx, yy, 8 * 5, 0, 8, 8);
    }
#endif
}
