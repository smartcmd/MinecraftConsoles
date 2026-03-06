#include "stdafx.h"
#include "StringHelpers.h"
#include "net.minecraft.world.level.levelgen.flat.h"
#include "net.minecraft.world.level.tile.h"
#include "FlatGeneratorInfo.h"

const wstring FlatGeneratorInfo::STRUCTURE_VILLAGE = L"village";
const wstring FlatGeneratorInfo::STRUCTURE_BIOME_SPECIFIC = L"biome_1";
const wstring FlatGeneratorInfo::STRUCTURE_STRONGHOLD = L"stronghold";
const wstring FlatGeneratorInfo::STRUCTURE_MINESHAFT = L"mineshaft";
const wstring FlatGeneratorInfo::STRUCTURE_BIOME_DECORATION = L"decoration";
const wstring FlatGeneratorInfo::STRUCTURE_LAKE = L"lake";
const wstring FlatGeneratorInfo::STRUCTURE_LAVA_LAKE = L"lava_lake";
const wstring FlatGeneratorInfo::STRUCTURE_DUNGEON = L"dungeon";

FlatGeneratorInfo::FlatGeneratorInfo()
{
	biome = 0;
}

FlatGeneratorInfo::~FlatGeneratorInfo()
{
	for(auto& layer : layers)
	{
		delete layer;
	}
}

int FlatGeneratorInfo::getBiome()
{
	return biome;
}

void FlatGeneratorInfo::setBiome(int biome)
{
	this->biome = biome;
}

unordered_map<wstring, unordered_map<wstring, wstring> > *FlatGeneratorInfo::getStructures()
{
	return &structures;
}

vector<FlatLayerInfo *> *FlatGeneratorInfo::getLayers()
{
	return &layers;
}

void FlatGeneratorInfo::updateLayers()
{
	int y = 0;

	for(auto& layer : layers)
	{
		layer->setStart(y);
		y += layer->getHeight();
	}
}

wstring FlatGeneratorInfo::toString()
{
	return L"";
#if 0
	StringBuilder builder = new StringBuilder();

	builder.append(SERIALIZATION_VERSION);
	builder.append(";");

	for (int i = 0; i < layers.size(); i++)
	{
		if (i > 0) builder.append(",");
		builder.append(layers.get(i).toString());
	}

	builder.append(";");
	builder.append(biome);

	if (!structures.isEmpty())
	{
		builder.append(";");
		int structCount = 0;

		for (Map.Entry<String, Map<String, String>> structure : structures.entrySet())
		{
			if (structCount++ > 0) builder.append(",");
			builder.append(structure.getKey().toLowerCase());

			Map<String, String> options = structure.getValue();
			if (!options.isEmpty())
			{
				builder.append("(");
				int optionCount = 0;

				for (Map.Entry<String, String> option : options.entrySet())
				{
					if (optionCount++ > 0) builder.append(" ");
					builder.append(option.getKey());
					builder.append("=");
					builder.append(option.getValue());
				}

				builder.append(")");
			}
		}
	}
	else
	{
		builder.append(";");
	}

	return builder.toString();
#endif
}

FlatLayerInfo *FlatGeneratorInfo::getLayerFromString(const wstring &input, int yOffset)
{
	return NULL;
#if 0
	std::vector<std::wstring> parts = stringSplit(input, L'x');

	int height = 1;
	int id;
	int data = 0;

	if (parts.size() == 2)
	{
		height = _fromString<int>(parts[0]);
		if (yOffset + height >= Level::maxBuildHeight) height = Level::maxBuildHeight - yOffset;
		if (height < 0) height = 0;
	}

	wstring identity = parts[parts.size() - 1];
	parts = stringSplit(identity, L':');

	id = _fromString<int>(parts[0]);
	if (parts.size() > 1) data = _from_String<int>(parts[1]);

	if (Tile::tiles[id] == NULL)
	{
		id = 0;
		data = 0;
	}

	if (data < 0 || data > 15) data = 0;

	FlatLayerInfo *result = new FlatLayerInfo(height, id, data);
	result->setStart(yOffset);
	return result;
#endif
}

vector<FlatLayerInfo *> *FlatGeneratorInfo::getLayersFromString(const wstring &input)
{
	if (input.empty()) return NULL;

	vector<FlatLayerInfo *> *result = new vector<FlatLayerInfo *>();
	std::vector<std::wstring> depths = stringSplit(input, L',');

	int yOffset = 0;

	for(auto& depth : depths)
	{
		FlatLayerInfo *layer = getLayerFromString(depth, yOffset);
		if (layer == NULL) return NULL;
		result->push_back(layer);
		yOffset += layer->getHeight();
	}

	return result;
}

FlatGeneratorInfo *FlatGeneratorInfo::fromValue(const wstring &input)
{
	return getDefault();

#if 0
	if (input.empty()) return getDefault();
	std::vector<std::wstring> parts = stringSplit(input, L';');

	int version = parts.size() == 1 ? 0 : Mth::getInt(parts[0], 0);
	if (version < 0 || version > SERIALIZATION_VERSION) return getDefault();

	FlatGeneratorInfo *result = new FlatGeneratorInfo();
	int index = parts.size() == 1 ? 0 : 1;
	vector<FlatLayerInfo *> *layers = getLayersFromString(parts[index++]);

	if (layers == NULL || layers->isEmpty())
	{
		delete layers;
		return getDefault();
	}

	result->getLayers()->addAll(layers);
	delete layers;
	result->updateLayers();

	int biome = Biome::plains_Id;
	if (version > 0 && parts.size() > index) biome = Mth::getInt(parts[index++], biome);
	result->setBiome(biome);

	if (version > 0 && parts.size() > index)
	{
		std::vector<std::wstring> structures = stringSplit(parts[index++], L',');

        for (auto it = structures.begin(); it != structures.end(); ++it)
        {
			std::vector<std::wstring> separated = stringSplit(parts[index++], L"\\(");

			unordered_map<wstring, wstring> structureOptions;

			if (separated[0].length() > 0)
			{
				(*result->getStructures())[separated[0]] = structureOptions;

				if (separated.size() > 1 && separated[1].endsWith(L")") && separated[1].length() > 1)
				{
					String[] options = separated[1].substring(0, separated[1].length() - 1).split(" ");

					for (int option = 0; option < options.length; option++)
					{
						String[] split = options[option].split("=", 2);
						if (split.length == 2) structureOptions[split[0]] = split[1];
					}
				}
			}
		}
	}
	else
	{
		(* (result->getStructures()) )[STRUCTURE_VILLAGE] = unordered_map<wstring, wstring>();
	}

	return result;
#endif
}

FlatGeneratorInfo *FlatGeneratorInfo::getDefault()
{
	FlatGeneratorInfo *result = new FlatGeneratorInfo();

	result->setBiome(Biome::plains->id);
	result->getLayers()->push_back(new FlatLayerInfo(1, Tile::unbreakable_Id));
	result->getLayers()->push_back(new FlatLayerInfo(2, Tile::dirt_Id));
	result->getLayers()->push_back(new FlatLayerInfo(1, Tile::grass_Id));
	result->updateLayers();
	(* (result->getStructures()) )[STRUCTURE_VILLAGE] = unordered_map<wstring, wstring>();

	return result;
}