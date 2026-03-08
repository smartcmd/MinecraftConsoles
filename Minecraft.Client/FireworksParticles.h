#pragma once
#include "Particle.h"
#include "../Minecraft.World/CompoundTag.h"

class ParticleEngine;

class FireworksParticles
{
public:

	class FireworksStarter : public Particle
	{
	public:
		virtual eINSTANCEOF GetType() { return eType_FIREWORKSSTARTERPARTICLE; }

	private:
		int life;
		ParticleEngine *engine;
		ListTag<CompoundTag> *explosions;
		bool twinkleDelay;

	public:
		FireworksStarter(Level *level, double x, double y, double z, double xd, double yd, double zd, ParticleEngine *engine, CompoundTag *infoTag);
		virtual void render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2);
		virtual void tick();
		bool isFarAwayFromCamera();
		void createParticle(double x, double y, double z, double xa, double ya, double za, intArray rgbColors, intArray fadeColors, bool trail, bool flicker);
		void createParticleBall(double baseSpeed, int steps, intArray rgbColors, intArray fadeColors, bool trail, bool flicker);
		void createParticleShape(double baseSpeed, coords2DArray coords, intArray rgbColors, intArray fadeColors, bool trail, bool flicker, bool flat);
		void createParticleBurst(intArray rgbColors, intArray fadeColors, bool trail, bool flicker);

	public:
		int getParticleTexture();
	};

	class FireworksSparkParticle : public Particle
	{
	public:
		virtual eINSTANCEOF GetType() { return eType_FIREWORKSSPARKPARTICLE; }

	private:
		int baseTex;
		bool trail;
		bool flicker;
		ParticleEngine *engine;

		float fadeR;
		float fadeG;
		float fadeB;
		bool hasFade;

	public:
		FireworksSparkParticle(Level *level, double x, double y, double z, double xa, double ya, double za, ParticleEngine *engine);
		void setTrail(bool trail);
		void setFlicker(bool flicker);
		using Particle::setColor;
		void setColor(int rgb);
		void setFadeColor(int rgb);
		virtual AABB *getCollideBox();
		virtual bool isPushable();
		virtual void render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2);
		virtual void tick();
		virtual void setBaseTex(int baseTex);
		virtual int getLightColor(float a);
		virtual float getBrightness(float a);
	};

	class FireworksOverlayParticle : public Particle
	{
	public:
		virtual eINSTANCEOF GetType() { return eType_FIREWORKSOVERLAYPARTICLE; }
	
		FireworksOverlayParticle(Level *level, double x, double y, double z);

		void render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2);
	};
};