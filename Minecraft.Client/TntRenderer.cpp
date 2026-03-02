#include "stdafx.h"
#include "TntRenderer.h"
#include "TileRenderer.h"
#include "..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"

TntRenderer::TntRenderer()
{
	tileRenderer = new TileRenderer();
	this->shadowRadius = 0.5f;
}

void TntRenderer::render(shared_ptr<Entity> _tnt, double x, double y, double z, float rot, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<PrimedTnt> tnt = dynamic_pointer_cast<PrimedTnt>(_tnt);

	glPushMatrix();
    glTranslatef((float) x, (float) y, (float) z);
    if (tnt->life - a + 1 < 10)
	{
        float g = 1 - ((tnt->life - a + 1) / 10.0f);
        if (g < 0) g = 0;
        if (g > 1) g = 1;
        g = g * g;
        g = g * g;
        float s = 1.0f + g * 0.3f;
        glScalef(s, s, s);
    }

    float br = (1 - ((tnt->life - a + 1) / 100.0f)) * 0.8f;
    bindTexture(TN_TERRAIN);	// 4J was L"/terrain.png"
	// 4J - change brought forward from 1.8.2
	float brightness = SharedConstants::TEXTURE_LIGHTING ? 1.0f : tnt->getBrightness(a);
    tileRenderer->renderTile(Tile::tnt, 0, brightness);
    if (tnt->life / 5 % 2 == 0)
	{
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
        glColor4f(1, 1, 1, br);
		tileRenderer->setColor = false;					// 4J added so that renderTile doesn't set its own colour here
        tileRenderer->renderTile(Tile::tnt, 0, 1);
		tileRenderer->setColor = true;					// 4J added so that renderTile doesn't set its own colour here
        glColor4f(1, 1, 1, 1);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
    }
    glPopMatrix();
}