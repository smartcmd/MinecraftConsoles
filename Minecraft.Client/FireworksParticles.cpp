#include "stdafx.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "FireworksParticles.h"
#include "Tesselator.h"
#include "../Minecraft.World/Level.h"

FireworksParticles::FireworksStarter::FireworksStarter(Level *level, double x, double y, double z, double xd, double yd, double zd, ParticleEngine *engine, CompoundTag *infoTag) : Particle(level, x, y, z, 0, 0, 0)
{
	life = 0;
	twinkleDelay = false;

	this->xd = xd;
	this->yd = yd;
	this->zd = zd;
	this->engine = engine;
	lifetime = 8;

	if (infoTag != nullptr)
	{
		explosions = static_cast<ListTag<CompoundTag> *>(infoTag->getList(FireworksItem::TAG_EXPLOSIONS)->copy());
		if (explosions->size() == 0)
		{
			explosions = nullptr;
		}
		else
		{
			lifetime = explosions->size() * 2 - 1;

			// check if any of the explosions has flickering
			for (int e = 0; e < explosions->size(); e++)
			{
				CompoundTag *compoundTag = explosions->get(e);
				if (compoundTag->getBoolean(FireworksItem::TAG_E_FLICKER))
				{
					twinkleDelay = true;
					lifetime += 15;
					break;
				}
			}
		}
	}
	else
	{
		// 4J:
		explosions = nullptr;
	}
}

void FireworksParticles::FireworksStarter::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
	// Do nothing
}

void FireworksParticles::FireworksStarter::tick()
{
	if (life == 0 && explosions != nullptr)
	{
		bool farEffect = isFarAwayFromCamera();

		bool largeExplosion = false;
		if (explosions->size() >= 3)
		{
			largeExplosion = true;
		}
		else
		{
			for (int e = 0; e < explosions->size(); e++)
			{
				CompoundTag *compoundTag = explosions->get(e);
				if (compoundTag->getByte(FireworksItem::TAG_E_TYPE) == FireworksItem::TYPE_BIG)
				{
					largeExplosion = true;
					break;
				}
			}
		}

		eSOUND_TYPE soundId;

		if (largeExplosion && farEffect)
		{
			soundId = eSoundType_FIREWORKS_LARGE_BLAST_FAR;
		}
		else if (largeExplosion && !farEffect)
		{
			soundId = eSoundType_FIREWORKS_LARGE_BLAST;
		}
		else if (!largeExplosion && farEffect)
		{
			soundId = eSoundType_FIREWORKS_BLAST_FAR;
		}
		else
		{
			soundId = eSoundType_FIREWORKS_BLAST;
		}

		level->playLocalSound(x, y, z, soundId, 20, .95f + random->nextFloat() * .1f, true, 100.0f);
	}

	if ((life % 2) == 0 && explosions != nullptr && (life / 2) < explosions->size())
	{
		int eIndex = life / 2;
		CompoundTag *compoundTag = explosions->get(eIndex);

		int type = compoundTag->getByte(FireworksItem::TAG_E_TYPE);
		bool trail = compoundTag->getBoolean(FireworksItem::TAG_E_TRAIL);
		bool flicker = compoundTag->getBoolean(FireworksItem::TAG_E_FLICKER);
		intArray colors = compoundTag->getIntArray(FireworksItem::TAG_E_COLORS);
		intArray fadeColors = compoundTag->getIntArray(FireworksItem::TAG_E_FADECOLORS);

		if (type == FireworksItem::TYPE_BIG)
		{
			// large ball
			createParticleBall(.5, 4, colors, fadeColors, trail, flicker);
		}
		else if (type == FireworksItem::TYPE_STAR)
		{
			double coords[6][2] = {
				0.0, 1.0,
				0.3455, 0.3090,
				0.9511, 0.3090, 
				93.0 / 245.0, -31.0 / 245.0, 
				150.0 / 245.0, -197.0 / 245.0,
				0.0, -88.0 / 245.0,
			};
			coords2DArray coordsArray(6, 2);
			for(unsigned int i = 0; i < coordsArray.length; ++i)
			{
				for(unsigned int j = 0; j < coordsArray[i]->length; ++j)
				{
					coordsArray[i]->data[j] = coords[i][j];
				}
			}

			// star-shape
			createParticleShape(.5, coordsArray, colors, fadeColors, trail, flicker, false);

			for(unsigned int i = 0; i < coordsArray.length; ++i)
			{
				delete [] coordsArray[i]->data;
			}
			delete [] coordsArray.data;
		}
		else if (type == FireworksItem::TYPE_CREEPER)
		{
			double coords[12][2] = {
				0.0, 0.2,
				0.2, 0.2,
				0.2, 0.6,
				0.6, 0.6,
				0.6, 0.2,
				0.2, 0.2,
				0.2, 0.0,
				0.4, 0.0,
				0.4, -0.6,
				0.2, -0.6,
				0.2, -0.4,
				0.0, -0.4,
			};
			coords2DArray coordsArray(12, 2);
			for(unsigned int i = 0; i < coordsArray.length; ++i)
			{
				for(unsigned int j = 0; j < coordsArray[i]->length; ++j)
				{
					coordsArray[i]->data[j] = coords[i][j];
				}
			}

			// creeper-shape
			createParticleShape(.5, coordsArray, colors, fadeColors, trail, flicker, true);

			for(unsigned int i = 0; i < coordsArray.length; ++i)
			{
				delete [] coordsArray[i]->data;
			}
			delete [] coordsArray.data;
		}
		else if (type == FireworksItem::TYPE_BURST)
		{
			createParticleBurst(colors, fadeColors, trail, flicker);
		}
		else
		{
			// small ball
			createParticleBall(.25, 2, colors, fadeColors, trail, flicker);
		}
		{
			int rgb = colors[0];
			float r = static_cast<float>((rgb & 0xff0000) >> 16) / 255.0f;
			float g = static_cast<float>((rgb & 0x00ff00) >> 8) / 255.0f;
			float b = static_cast<float>((rgb & 0x0000ff) >> 0) / 255.0f;
			shared_ptr<FireworksOverlayParticle> fireworksOverlayParticle = std::make_shared<FireworksOverlayParticle>(level, x, y, z);
			fireworksOverlayParticle->setColor(r, g, b);
			fireworksOverlayParticle->setAlpha(0.99f);		// 4J added
			engine->add(fireworksOverlayParticle);
		}
	}
	life++;
	if (life > lifetime)
	{
		if (twinkleDelay)
		{
			bool farEffect = isFarAwayFromCamera();
			eSOUND_TYPE soundId = farEffect ? eSoundType_FIREWORKS_TWINKLE_FAR : eSoundType_FIREWORKS_TWINKLE;
			level->playLocalSound(x, y, z, soundId, 20, .90f + random->nextFloat() * .15f, true, 100.0f);

		}
		remove();
	}
}

bool FireworksParticles::FireworksStarter::isFarAwayFromCamera()
{
	Minecraft *instance = Minecraft::GetInstance();
	if (instance != nullptr && instance->cameraTargetPlayer != nullptr)
	{
		if (instance->cameraTargetPlayer->distanceToSqr(x, y, z) < 16 * 16)
		{
			return false;
		}
	}
	return true;
}

void FireworksParticles::FireworksStarter::createParticle(double x, double y, double z, double xa, double ya, double za, intArray rgbColors, intArray fadeColors, bool trail, bool flicker)
{
	shared_ptr<FireworksSparkParticle> fireworksSparkParticle = std::make_shared<FireworksSparkParticle>(level, x, y, z, xa, ya, za, engine);
	fireworksSparkParticle->setAlpha(0.99f);
	fireworksSparkParticle->setTrail(trail);
	fireworksSparkParticle->setFlicker(flicker);

	int color = random->nextInt(rgbColors.length);
	fireworksSparkParticle->setColor(rgbColors[color]);
	if (/*fadeColors != nullptr &&*/ fadeColors.length > 0)
	{
		fireworksSparkParticle->setFadeColor(fadeColors[random->nextInt(fadeColors.length)]);
	}
	engine->add(fireworksSparkParticle);
}

void FireworksParticles::FireworksStarter::createParticleBall(double baseSpeed, int steps, intArray rgbColors, intArray fadeColors, bool trail, bool flicker) {

	double xx = x;
	double yy = y;
	double zz = z;

	for (int yStep = -steps; yStep <= steps; yStep++) {
		for (int xStep = -steps; xStep <= steps; xStep++) {
			for (int zStep = -steps; zStep <= steps; zStep++) {
				double xa = xStep + (random->nextDouble() - random->nextDouble()) * .5;
				double ya = yStep + (random->nextDouble() - random->nextDouble()) * .5;
				double za = zStep + (random->nextDouble() - random->nextDouble()) * .5;
				double len = sqrt(xa * xa + ya * ya + za * za) / baseSpeed + random->nextGaussian() * .05;

				createParticle(xx, yy, zz, xa / len, ya / len, za / len, rgbColors, fadeColors, trail, flicker);

				if (yStep != -steps && yStep != steps && xStep != -steps && xStep != steps) {
					zStep += steps * 2 - 1;
				}
			}
		}
	}
}

void FireworksParticles::FireworksStarter::createParticleShape(double baseSpeed, coords2DArray coords, intArray rgbColors, intArray fadeColors, bool trail, bool flicker, bool flat)
{
	double sx = coords[0]->data[0];
	double sy = coords[0]->data[1];

	{
		createParticle(x, y, z, sx * baseSpeed, sy * baseSpeed, 0, rgbColors, fadeColors, trail, flicker);
	}

	float baseAngle = random->nextFloat() * PI;
	double angleMod = (flat ? .034 : .34);
	for (int angleStep = 0; angleStep < 3; angleStep++)
	{
		double angle = baseAngle + angleStep * PI * angleMod;

		double ox = sx;
		double oy = sy;

		for (int c = 1; c < coords.length; c++)
		{
			double tx = coords[c]->data[0];
			double ty = coords[c]->data[1];

			for (double subStep = .25; subStep <= 1.0; subStep += .25)
			{
				double xa = (ox + (tx - ox) * subStep) * baseSpeed;
				double ya = (oy + (ty - oy) * subStep) * baseSpeed;

				double za = xa * sin(angle);
				xa = xa * cos(angle);

				for (double flip = -1; flip <= 1; flip += 2)
				{
					createParticle(x, y, z, xa * flip, ya, za * flip, rgbColors, fadeColors, trail, flicker);
				}
			}
			ox = tx;
			oy = ty;
		}

	}
}

void FireworksParticles::FireworksStarter::createParticleBurst(intArray rgbColors, intArray fadeColors, bool trail, bool flicker)
{
	double baseOffX = random->nextGaussian() * .05;
	double baseOffZ = random->nextGaussian() * .05;

	for (int i = 0; i < 70; i++) {

		double xa = xd * .5 + random->nextGaussian() * .15 + baseOffX;
		double za = zd * .5 + random->nextGaussian() * .15 + baseOffZ;
		double ya = yd * .5 + random->nextDouble() * .5;

		createParticle(x, y, z, xa, ya, za, rgbColors, fadeColors, trail, flicker);
	}
}

int FireworksParticles::FireworksStarter::getParticleTexture()
{
	return ParticleEngine::MISC_TEXTURE;
}

FireworksParticles::FireworksSparkParticle::FireworksSparkParticle(Level *level, double x, double y, double z, double xa, double ya, double za, ParticleEngine *engine) : Particle(level, x, y, z)
{
	baseTex = 10 * 16;

	xd = xa;
	yd = ya;
	zd = za;
	this->engine = engine;

	size *= 0.75f;

	lifetime = 48 + random->nextInt(12);
#ifdef __PSVITA__
	noPhysics = true;			// 4J - optimisation, these are just too slow on Vita to be running with physics on
#else
	noPhysics = false;
#endif

	
	trail = false;
	flicker = false;

	fadeR = 0.0f;
	fadeG = 0.0f;
	fadeB = 0.0f;
	hasFade = false;
}

void FireworksParticles::FireworksSparkParticle::setTrail(bool trail)
{
	this->trail = trail;
}

void FireworksParticles::FireworksSparkParticle::setFlicker(bool flicker)
{
	this->flicker = flicker;
}

void FireworksParticles::FireworksSparkParticle::setColor(int rgb) 
{
	float r = static_cast<float>((rgb & 0xff0000) >> 16) / 255.0f;
	float g = static_cast<float>((rgb & 0x00ff00) >> 8) / 255.0f;
	float b = static_cast<float>((rgb & 0x0000ff) >> 0) / 255.0f;
	float scale = 1.0f;
	Particle::setColor(r * scale, g * scale, b * scale);
}

void FireworksParticles::FireworksSparkParticle::setFadeColor(int rgb)
{
	fadeR = static_cast<float>((rgb & 0xff0000) >> 16) / 255.0f;
	fadeG = static_cast<float>((rgb & 0x00ff00) >> 8) / 255.0f;
	fadeB = static_cast<float>((rgb & 0x0000ff) >> 0) / 255.0f;
	hasFade = true;
}

AABB *FireworksParticles::FireworksSparkParticle::getCollideBox()
{
	return nullptr;
}

bool FireworksParticles::FireworksSparkParticle::isPushable()
{
	return false;
}

void FireworksParticles::FireworksSparkParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
	if (!flicker || age < (lifetime / 3) || (((age + lifetime) / 3) % 2) == 0) 
	{
		Particle::render(t, a, xa, ya, za, xa2, za2);
	}
}

void FireworksParticles::FireworksSparkParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;

	if (age++ >= lifetime) remove();
	if (age > lifetime / 2)
	{
		setAlpha(1.0f - ((static_cast<float>(age) - lifetime / 2) / static_cast<float>(lifetime)));

		if (hasFade)
		{
			rCol = rCol + (fadeR - rCol) * .2f;
			gCol = gCol + (fadeG - gCol) * .2f;
			bCol = bCol + (fadeB - bCol) * .2f;
		}
	}

	setMiscTex(baseTex + (7 - age * 8 / lifetime));

	yd -= 0.004;
	move(xd, yd, zd, true);			// 4J - changed so these don't attempt to collide with entities
	xd *= 0.91f;
	yd *= 0.91f;
	zd *= 0.91f;

	if (onGround)
	{
		xd *= 0.7f;
		zd *= 0.7f;
	}

	if (trail && (age < lifetime / 2) && ((age + lifetime) % 2) == 0)
	{
		shared_ptr<FireworksSparkParticle> fireworksSparkParticle = std::make_shared<FireworksSparkParticle>(level, x, y, z, 0, 0, 0, engine);
		fireworksSparkParticle->setAlpha(0.99f);
		fireworksSparkParticle->setColor(rCol, gCol, bCol);
		fireworksSparkParticle->age = fireworksSparkParticle->lifetime / 2;
		if (hasFade)
		{
			fireworksSparkParticle->hasFade = true;
			fireworksSparkParticle->fadeR = fadeR;
			fireworksSparkParticle->fadeG = fadeG;
			fireworksSparkParticle->fadeB = fadeB;
		}
		fireworksSparkParticle->flicker = flicker;
		engine->add(fireworksSparkParticle);
	}
}

void FireworksParticles::FireworksSparkParticle::setBaseTex(int baseTex)
{
	this->baseTex = baseTex;
}

int FireworksParticles::FireworksSparkParticle::getLightColor(float a)
{
	return SharedConstants::FULLBRIGHT_LIGHTVALUE;
}

float FireworksParticles::FireworksSparkParticle::getBrightness(float a)
{
	return 1;
}

FireworksParticles::FireworksOverlayParticle::FireworksOverlayParticle(Level *level, double x, double y, double z) : Particle(level, x, y, z)
{
	lifetime = 4;
}

void FireworksParticles::FireworksOverlayParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
	float u0 = 32.0f / 128.0f;
	float u1 = u0 + 32.0f / 128.0f;
	float v0 = 16.0f / 128.0f;
	float v1 = v0 + 32.0f / 128.0f;
	float r = 7.1f * sin((static_cast<float>(age) + a - 1.0f) * .25f * PI);
	alpha = 0.6f - (static_cast<float>(age) + a - 1.0f) * .25f * .5f;

	float x = static_cast<float>(xo + (this->x - xo) * a - xOff);
	float y = static_cast<float>(yo + (this->y - yo) * a - yOff);
	float z = static_cast<float>(zo + (this->z - zo) * a - zOff);

	t->color(rCol, gCol, bCol, alpha);

	t->vertexUV(x - xa * r - xa2 * r, y - ya * r, z - za * r - za2 * r, u1, v1);
	t->vertexUV(x - xa * r + xa2 * r, y + ya * r, z - za * r + za2 * r, u1, v0);
	t->vertexUV(x + xa * r + xa2 * r, y + ya * r, z + za * r + za2 * r, u0, v0);
	t->vertexUV(x + xa * r - xa2 * r, y - ya * r, z + za * r - za2 * r, u0, v1);
}