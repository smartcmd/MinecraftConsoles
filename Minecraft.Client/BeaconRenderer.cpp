#include "stdafx.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "BeaconRenderer.h"
#include "Tesselator.h"

ResourceLocation BeaconRenderer::BEAM_LOCATION = ResourceLocation(TN_MISC_BEACON_BEAM);

void BeaconRenderer::render(shared_ptr<TileEntity> _beacon, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	shared_ptr<BeaconTileEntity> beacon = dynamic_pointer_cast<BeaconTileEntity>(_beacon);

	float scale = beacon->getAndUpdateClientSideScale();

	if (scale > 0)
	{
		Tesselator *t = Tesselator::getInstance();

		bindTexture(&BEAM_LOCATION);

		// TODO: 4J: Put this back in
		//assert(0);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDepthMask(true);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		float tt = beacon->getLevel()->getGameTime() + a;
		float texVOff = -tt * .20f - floor(-tt * .10f);

		{
			int r = 1;

			double rot = tt * .025 * (1 - (r & 1) * 2.5);

			t->begin();
			t->color(255, 255, 255, 32);

			double rr1 = r * 0.2;

			double wnx = .5 + cos(rot + PI * .75) * rr1;
			double wnz = .5 + sin(rot + PI * .75) * rr1;
			double enx = .5 + cos(rot + PI * .25) * rr1;
			double enz = .5 + sin(rot + PI * .25) * rr1;

			double wsx = .5 + cos(rot + PI * 1.25) * rr1;
			double wsz = .5 + sin(rot + PI * 1.25) * rr1;
			double esx = .5 + cos(rot + PI * 1.75) * rr1;
			double esz = .5 + sin(rot + PI * 1.75) * rr1;

			double top = 256 * scale;

			double uu1 = 0;
			double uu2 = 1;
			double vv2 = -1 + texVOff;
			double vv1 = 256 * scale * (.5 / rr1) + vv2;

			t->vertexUV(x + wnx, y + top, z + wnz, uu2, vv1);
			t->vertexUV(x + wnx, y, z + wnz, uu2, vv2);
			t->vertexUV(x + enx, y, z + enz, uu1, vv2);
			t->vertexUV(x + enx, y + top, z + enz, uu1, vv1);

			t->vertexUV(x + esx, y + top, z + esz, uu2, vv1);
			t->vertexUV(x + esx, y, z + esz, uu2, vv2);
			t->vertexUV(x + wsx, y, z + wsz, uu1, vv2);
			t->vertexUV(x + wsx, y + top, z + wsz, uu1, vv1);

			t->vertexUV(x + enx, y + top, z + enz, uu2, vv1);
			t->vertexUV(x + enx, y, z + enz, uu2, vv2);
			t->vertexUV(x + esx, y, z + esz, uu1, vv2);
			t->vertexUV(x + esx, y + top, z + esz, uu1, vv1);
				
			t->vertexUV(x + wsx, y + top, z + wsz, uu2, vv1);
			t->vertexUV(x + wsx, y, z + wsz, uu2, vv2);
			t->vertexUV(x + wnx, y, z + wnz, uu1, vv2);
			t->vertexUV(x + wnx, y + top, z + wnz, uu1, vv1);
				
			t->end();
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(false);

		{
			t->begin();
			t->color(255, 255, 255, 32);

			double wnx = .2;
			double wnz = .2;
			double enx = .8;
			double enz = .2;

			double wsx = .2;
			double wsz = .8;
			double esx = .8;
			double esz = .8;

			double top = 256 * scale;

			double uu1 = 0;
			double uu2 = 1;
			double vv2 = -1 + texVOff;
			double vv1 = 256 * scale + vv2;

			t->vertexUV(x + wnx, y + top, z + wnz, uu2, vv1);
			t->vertexUV(x + wnx, y, z + wnz, uu2, vv2);
			t->vertexUV(x + enx, y, z + enz, uu1, vv2);
			t->vertexUV(x + enx, y + top, z + enz, uu1, vv1);
				
			t->vertexUV(x + esx, y + top, z + esz, uu2, vv1);
			t->vertexUV(x + esx, y, z + esz, uu2, vv2);
			t->vertexUV(x + wsx, y, z + wsz, uu1, vv2);
			t->vertexUV(x + wsx, y + top, z + wsz, uu1, vv1);
				
			t->vertexUV(x + enx, y + top, z + enz, uu2, vv1);
			t->vertexUV(x + enx, y, z + enz, uu2, vv2);
			t->vertexUV(x + esx, y, z + esz, uu1, vv2);
			t->vertexUV(x + esx, y + top, z + esz, uu1, vv1);
				
			t->vertexUV(x + wsx, y + top, z + wsz, uu2, vv1);
			t->vertexUV(x + wsx, y, z + wsz, uu2, vv2);
			t->vertexUV(x + wnx, y, z + wnz, uu1, vv2);
			t->vertexUV(x + wnx, y + top, z + wnz, uu1, vv1);
				
			t->end();
		}

		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);

		glDepthMask(true);
	}
}