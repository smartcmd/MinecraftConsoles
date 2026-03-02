#include "stdafx.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.newbiome.layer.h"

DownfallLayer::DownfallLayer(shared_ptr<Layer>parent) : Layer(0)
{
	this->parent = parent;
}

intArray DownfallLayer::getArea(int xo, int yo, int w, int h)
{
	intArray b = parent->getArea(xo, yo, w, h);

	intArray result = IntCache::allocate(w * h);
	for (int i = 0; i < w * h; i++)
	{
		result[i] = Biome::biomes[b[i]]->getDownfallInt();
	}

	return result;
}