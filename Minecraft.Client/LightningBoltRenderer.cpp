#include "stdafx.h"
#include "LightningBoltRenderer.h"
#include "Tesselator.h"
#include "..\Minecraft.World\net.minecraft.world.entity.global.h"

void LightningBoltRenderer::render(shared_ptr<Entity> _bolt, double x, double y, double z, float rot, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<LightningBolt> bolt = dynamic_pointer_cast<LightningBolt>(_bolt);

    Tesselator *t = Tesselator::getInstance();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);


    double xOffs[8];
    double zOffs[8];
    double xOff = 0;
    double zOff = 0;
    {
        Random *random = new Random(bolt->seed);
        for (int h = 7; h >= 0; h--)
		{
            xOffs[h] = xOff;
            zOffs[h] = zOff;
            xOff += random->nextInt(11) - 5;
            zOff += random->nextInt(11) - 5;
        }
    }

    for (int r = 0; r < 4; r++)
	{
        Random *random = new Random(bolt->seed);
        for (int p = 0; p < 3; p++)
		{
            int hs = 7;
            int ht = 0;
            if (p > 0) hs = 7 - p;
            if (p > 0) ht = hs - 2;
            double xo0 = xOffs[hs] - xOff;
            double zo0 = zOffs[hs] - zOff;
            for (int h = hs; h >= ht; h--)
			{
                double xo1 = xo0;
                double zo1 = zo0;
                if (p == 0)
				{
                    xo0 += random->nextInt(11) - 5;
                    zo0 += random->nextInt(11) - 5;
                }
				else
				{
                    xo0 += random->nextInt(31) - 15;
                    zo0 += random->nextInt(31) - 15;
                }

                t->begin(GL_TRIANGLE_STRIP);
                float br = 0.5f;
                t->color(0.9f * br, 0.9f * br, 1 * br, 0.3f);

                double rr1 = (0.1 + r * 0.2);
                if (p == 0) rr1 *= (h * 0.1 + 1);

                double rr2 = (0.1 + r * 0.2);
                if (p == 0) rr2 *= ((h-1) * 0.1 + 1);

                for (int i = 0; i < 5; i++)
				{
                    double xx1 = x + 0.5 - rr1;
                    double zz1 = z + 0.5 - rr1;
                    if (i == 1 || i == 2) xx1 += rr1 * 2;
                    if (i == 2 || i == 3) zz1 += rr1 * 2;

                    double xx2 = x + 0.5 - rr2;
                    double zz2 = z + 0.5 - rr2;
                    if (i == 1 || i == 2) xx2 += rr2 * 2;
                    if (i == 2 || i == 3) zz2 += rr2 * 2;

                    t->vertex((float)(xx2 + xo0), (float)( y + (h) * 16), (float)( zz2 + zo0));
                    t->vertex((float)(xx1 + xo1), (float)( y + (h + 1) * 16), (float)( zz1 + zo1));

                }

                t->end();
            }
        }
    }


    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

}