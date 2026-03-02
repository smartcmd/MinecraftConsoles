#include "stdafx.h"
#include "JavaMath.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.h"
#include "LiquidTile.h"
#include "Facing.h"
#include "SoundTypes.h"

const wstring LiquidTile::TEXTURE_LAVA_STILL = L"lava";
const wstring LiquidTile::TEXTURE_WATER_STILL = L"water";
const wstring LiquidTile::TEXTURE_WATER_FLOW = L"water_flow";
const wstring LiquidTile::TEXTURE_LAVA_FLOW = L"lava_flow";

LiquidTile::LiquidTile(int id, Material *material) : Tile(id, material,isSolidRender())
{
    float yo = 0;
    float e = 0;

    setShape(0 + e, 0 + yo, 0 + e, 1 + e, 1 + yo, 1 + e);
    setTicking(true);
}

bool LiquidTile::isPathfindable(LevelSource *level, int x, int y, int z)
{
	return material != Material::lava;
}

int LiquidTile::getColor() const
{
	return 0xffffff;
}

int LiquidTile::getColor(LevelSource *level, int x, int y, int z)
{

	return getColor(level, x, y, z, 0);
}

int LiquidTile::getColor(LevelSource *level, int x, int y, int z, int d)
{
	if (material == Material::water)
	{
		int totalRed = 0;
		int totalGreen = 0;
		int totalBlue = 0;

		for (int oz = -1; oz <= 1; oz++)
		{
			for (int ox = -1; ox <= 1; ox++)
			{
				int waterColor = level->getBiome(x + ox, z + oz)->getWaterColor();

				totalRed += (waterColor & 0xff0000) >> 16;
				totalGreen += (waterColor & 0xff00) >> 8;
				totalBlue += (waterColor & 0xff);
			}
		}

		return (((totalRed / 9) & 0xFF) << 16) | (((totalGreen / 9) & 0xFF) << 8) | (((totalBlue / 9) & 0xFF));
	}
	return 0xffffff;
}

float LiquidTile::getHeight(int d)
{
    if (d >= 8) d = 0;
    return (d + 1) / 9.0f;
}

Icon *LiquidTile::getTexture(int face, int data)
{
	if (face == Facing::DOWN || face == Facing::UP)
	{
        return icons[0];
	}
	else
	{
        return icons[1];
    }
}

int LiquidTile::getDepth(Level *level, int x, int y, int z)
{
    if (level->getMaterial(x, y, z) == material) return level->getData(x, y, z);
	else return -1;
}

int LiquidTile::getRenderedDepth(LevelSource *level, int x, int y, int z)
{
    if (level->getMaterial(x, y, z) != material) return -1;
    int d = level->getData(x, y, z);
    if (d >= 8) d = 0;
    return d;
}

bool LiquidTile::isCubeShaped()
{
	return false;
}

bool LiquidTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool LiquidTile::mayPick(int data, bool liquid)
{
	return liquid && data == 0;
}

bool LiquidTile::isSolidFace(LevelSource *level, int x, int y, int z, int face)
{
    Material *m = level->getMaterial(x, y, z);
    if (m == this->material) return false;
	if (face == Facing::UP) return true;
    if (m == Material::ice) return false;
    
    return Tile::isSolidFace(level, x, y, z, face);
}

bool LiquidTile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face)
{
    Material *m = level->getMaterial(x, y, z);
    if (m == this->material) return false;
	if (face == Facing::UP) return true;
    if (m == Material::ice) return false;
    return Tile::shouldRenderFace(level, x, y, z, face);
}

AABB *LiquidTile::getAABB(Level *level, int x, int y, int z)
{
	return NULL;
}

int LiquidTile::getRenderShape()
{
	return Tile::SHAPE_WATER;
}

int LiquidTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return 0;
}

int LiquidTile::getResourceCount(Random *random)
{
	return 0;
}

Vec3 *LiquidTile::getFlow(LevelSource *level, int x, int y, int z)
{
    Vec3 *flow = Vec3::newTemp(0,0,0);
    int mid = getRenderedDepth(level, x, y, z);
    for (int d = 0; d < 4; d++)
	{

        int xt = x;
        int yt = y;
        int zt = z;

        if (d == 0) xt--;
        if (d == 1) zt--;
        if (d == 2) xt++;
        if (d == 3) zt++;

        int t = getRenderedDepth(level, xt, yt, zt);
        if (t < 0)
		{
            if (!level->getMaterial(xt, yt, zt)->blocksMotion())
			{
                t = getRenderedDepth(level, xt, yt - 1, zt);
                if (t >= 0)
				{
                    int dir = t - (mid - 8);
                    flow = flow->add((xt - x) * dir, (yt - y) * dir, (zt - z) * dir);
                }
            }
        } else
		{
            if (t >= 0)
			{
                int dir = t - mid;
                flow = flow->add((xt - x) * dir, (yt - y) * dir, (zt - z) * dir);
            }
        }

    }
    if (level->getData(x, y, z) >= 8)
	{
        bool ok = false;
        if (ok || isSolidFace(level, x, y, z - 1, 2)) ok = true;
        if (ok || isSolidFace(level, x, y, z + 1, 3)) ok = true;
        if (ok || isSolidFace(level, x - 1, y, z, 4)) ok = true;
        if (ok || isSolidFace(level, x + 1, y, z, 5)) ok = true;
        if (ok || isSolidFace(level, x, y + 1, z - 1, 2)) ok = true;
        if (ok || isSolidFace(level, x, y + 1, z + 1, 3)) ok = true;
        if (ok || isSolidFace(level, x - 1, y + 1, z, 4)) ok = true;
        if (ok || isSolidFace(level, x + 1, y + 1, z, 5)) ok = true;
        if (ok) flow = flow->normalize()->add(0, -6, 0);
    }
    flow = flow->normalize();
    return flow;
}

void LiquidTile::handleEntityInside(Level *level, int x, int y, int z, shared_ptr<Entity> e, Vec3 *current)
{
    Vec3 *flow = getFlow(level, x, y, z);
    current->x += flow->x;
    current->y += flow->y;
    current->z += flow->z;
}

int LiquidTile::getTickDelay()
{
    if (material == Material::water) return 5;
    if (material == Material::lava) return 30;
    return 0;
}

// 4J - change brought forward from 1.8.2
int LiquidTile::getLightColor(LevelSource *level, int x, int y, int z, int tileId/*=-1*/)
{
	// 4J - note that this code seems to basically be a hack to fix a problem where post-processed things like lakes aren't getting lit properly
    int a = level->getLightColor(x, y, z, 0, tileId);
    int b = level->getLightColor(x, y + 1, z, 0, tileId);

    int aa = a & 0xff;
    int ba = b & 0xff;
    int ab = (a >> 16) & 0xff;
    int bb = (b >> 16) & 0xff;

    return (aa > ba ? aa : ba) | ((ab > bb ? ab : bb) << 16);
}

float LiquidTile::getBrightness(LevelSource *level, int x, int y, int z)
{
    float a = level->getBrightness(x, y, z);
    float b = level->getBrightness(x, y + 1, z);
    return a > b ? a : b;
}

int LiquidTile::getRenderLayer()
{
	return material == Material::water ? 1 : 0;
}

void LiquidTile::animateTick(Level *level, int x, int y, int z, Random *random)
{
    if (material == Material::water)
	{
        if (random->nextInt(10) == 0)
		{
            int d = level->getData(x, y, z);
            if (d <= 0 || d >= 8)
			{
                level->addParticle(eParticleType_suspended, x + random->nextFloat(), y + random->nextFloat(), z + random->nextFloat(), 0, 0, 0);
            }
        }
		// 4J-PB - this loop won't run!
        for (int i = 0; i < 0; i++)
		{	// This was an attempt to add foam to
			// the bottoms of waterfalls. It
			// didn't went ok.
            int dir = random->nextInt(4);
            int xt = x;
            int zt = z;
            if (dir == 0) xt--;
            if (dir == 1) xt++;
            if (dir == 2) zt--;
            if (dir == 3) zt++;
            if (level->getMaterial(xt, y, zt) == Material::air && (level->getMaterial(xt, y - 1, zt)->blocksMotion() || level->getMaterial(xt, y - 1, zt)->isLiquid()))
			{
                float r = 1 / 16.0f;
                double xx = x + random->nextFloat();
                double yy = y + random->nextFloat();
                double zz = z + random->nextFloat();
                if (dir == 0) xx = x - r;
                if (dir == 1) xx = x + 1 + r;
                if (dir == 2) zz = z - r;
                if (dir == 3) zz = z + 1 + r;

                double xd = 0;
                double zd = 0;

                if (dir == 0) xd = -r;
                if (dir == 1) xd = +r;
                if (dir == 2) zd = -r;
                if (dir == 3) zd = +r;

                level->addParticle(eParticleType_splash, xx, yy, zz, xd, 0, zd);
            }
        }
    }
    if (material == Material::water && random->nextInt(64) == 0)
	{
        int d = level->getData(x, y, z);
        if (d > 0 && d < 8)
		{
            level->playLocalSound(x + 0.5f, y + 0.5f, z + 0.5f, eSoundType_LIQUID_WATER, random->nextFloat() * 0.25f + 0.75f, random->nextFloat() * 1.0f + 0.5f);
        }
    }
    if (material == Material::lava)
	{
        if (level->getMaterial(x, y + 1, z) == Material::air && !level->isSolidRenderTile(x, y + 1, z))
		{
            if (random->nextInt(100) == 0)
			{
				ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
                double xx = x + random->nextFloat();
                double yy = y + tls->yy1;
                double zz = z + random->nextFloat();
                level->addParticle(eParticleType_lava, xx, yy, zz, 0, 0, 0);
				// 4J - new sound brought forward from 1.2.3
				level->playLocalSound(xx, yy, zz, eSoundType_LIQUID_LAVA_POP, 0.2f + random->nextFloat() * 0.2f, 0.9f + random->nextFloat() * 0.15f);
            }
			// 4J - new sound brought forward from 1.2.3
            if (random->nextInt(200) == 0)
			{
                level->playLocalSound(x, y, z, eSoundType_LIQUID_LAVA, 0.2f + random->nextFloat() * 0.2f, 0.9f + random->nextFloat() * 0.15f);
            }
        }
    }

    if (random->nextInt(10) == 0)
	{
        if (level->isTopSolidBlocking(x, y - 1, z) && !level->getMaterial(x, y - 2, z)->blocksMotion())
		{
            double xx = x + random->nextFloat();
            double yy = y - 1.05;
            double zz = z + random->nextFloat();

			if (material == Material::water) level->addParticle(eParticleType_dripWater, xx, yy, zz, 0, 0, 0);
            else level->addParticle(eParticleType_dripLava, xx, yy, zz, 0, 0, 0);
        }
    }
}

double LiquidTile::getSlopeAngle(LevelSource *level, int x, int y, int z, Material *m)
{
    Vec3 *flow = NULL;
    if (m == Material::water) flow = ((LiquidTile *) Tile::water)->getFlow(level, x, y, z);
    if (m == Material::lava) flow = ((LiquidTile *) Tile::lava)->getFlow(level, x, y, z);
    if (flow->x == 0 && flow->z == 0) return -1000;
    return atan2(flow->z, flow->x) - PI / 2;
}

void LiquidTile::onPlace(Level *level, int x, int y, int z)
{
	updateLiquid(level, x, y, z);
}

void LiquidTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	updateLiquid(level, x, y, z);
}

void LiquidTile::updateLiquid(Level *level, int x, int y, int z)
{
    if (level->getTile(x, y, z) != id) return;
    if (material == Material::lava)
	{
        bool water = false;
        if (water || level->getMaterial(x, y, z - 1) == Material::water) water = true;
        if (water || level->getMaterial(x, y, z + 1) == Material::water) water = true;
        if (water || level->getMaterial(x - 1, y, z) == Material::water) water = true;
        if (water || level->getMaterial(x + 1, y, z) == Material::water) water = true;
        if (water || level->getMaterial(x, y + 1, z) == Material::water) water = true;
        if (water)
		{
            int data = level->getData(x, y, z);
            if (data == 0)
			{
                level->setTile(x, y, z, Tile::obsidian_Id);
            }
			else if (data <= 4)
			{
                level->setTile(x, y, z, Tile::stoneBrick_Id);
            }
            fizz(level, x, y, z);
        }
    }

}

void LiquidTile::fizz(Level *level, int x, int y, int z)
{
	MemSect(31);
    level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, eSoundType_RANDOM_FIZZ, 0.5f, 2.6f + (level->random->nextFloat() - level->random->nextFloat()) * 0.8f);
	MemSect(0);
    for (int i = 0; i < 8; i++)
	{
        level->addParticle(eParticleType_largesmoke, x +Math::random(), y + 1.2, z + Math::random(), 0, 0, 0);
    }
}

void LiquidTile::registerIcons(IconRegister *iconRegister)
{
	if (material == Material::lava)
	{
		icons[0] = iconRegister->registerIcon(TEXTURE_LAVA_STILL);
		icons[1] = iconRegister->registerIcon(TEXTURE_LAVA_FLOW);
	}
	else
	{
		icons[0] = iconRegister->registerIcon(TEXTURE_WATER_STILL);
		icons[1] = iconRegister->registerIcon(TEXTURE_WATER_FLOW);
	}
}

Icon *LiquidTile::getTexture(const wstring &name)
{
	if (name.compare(TEXTURE_WATER_STILL)==0) return Tile::water->icons[0];
	if (name.compare(TEXTURE_WATER_FLOW)==0) return Tile::water->icons[1];
	if (name.compare(TEXTURE_LAVA_STILL)==0) return Tile::lava->icons[0];
	if (name.compare(TEXTURE_LAVA_FLOW)==0) return Tile::lava->icons[1];
	return NULL;
}
