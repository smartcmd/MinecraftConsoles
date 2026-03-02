#include "stdafx.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.newbiome.layer.h"

TemperatureMixerLayer::TemperatureMixerLayer(shared_ptr<Layer>temp, shared_ptr<Layer>parent, int layer) : Layer(0)
{
	this->parent = parent;
	this->temp = temp;
	this->layer = layer;
}

intArray TemperatureMixerLayer::getArea(int xo, int yo, int w, int h)
{
	intArray b = parent->getArea(xo, yo, w, h);
	intArray t = temp->getArea(xo, yo, w, h);

	intArray result = IntCache::allocate(w * h);
	for (int i = 0; i < w * h; i++) {
		result[i] = t[i] + (Biome::biomes[b[i]]->getTemperatureInt() - t[i]) / (layer * 2 + 1);
	}

	return result;
}