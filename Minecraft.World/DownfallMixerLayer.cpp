#include "stdafx.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.newbiome.layer.h"

DownfallMixerLayer::DownfallMixerLayer(shared_ptr<Layer>downfall, shared_ptr<Layer>parent, int layer) : Layer(0)
{
	this->parent = parent;
	this->downfall = downfall;
	this->layer = layer;
}

intArray DownfallMixerLayer::getArea(int xo, int yo, int w, int h)
{
	intArray b = parent->getArea(xo, yo, w, h);
	intArray d = downfall->getArea(xo, yo, w, h);

	intArray result = IntCache::allocate(w * h);
	for (int i = 0; i < w * h; i++)
	{
		result[i] = d[i] + (Biome::biomes[b[i]]->getDownfallInt() - d[i]) / (layer + 1);
	}

	return result;
}