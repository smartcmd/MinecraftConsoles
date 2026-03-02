#pragma once

#include "Spider.h"

class CaveSpider : public Spider
{
	public:
		eINSTANCEOF GetType() { return eTYPE_CAVESPIDER; }
		static Entity *create(Level *level) { return new CaveSpider(level); }

	public:
		CaveSpider(Level *level);

		virtual int getMaxHealth();
		virtual float getModelScale();
		virtual bool doHurtTarget(shared_ptr<Entity> target);
		void finalizeMobSpawn();
};