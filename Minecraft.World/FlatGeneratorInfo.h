#pragma once

class FlatLayerInfo;

class FlatGeneratorInfo
{
public:
	static const int SERIALIZATION_VERSION = 2;
	static const wstring STRUCTURE_VILLAGE;
	static const wstring STRUCTURE_BIOME_SPECIFIC;
	static const wstring STRUCTURE_STRONGHOLD;
	static const wstring STRUCTURE_MINESHAFT;
	static const wstring STRUCTURE_BIOME_DECORATION;
	static const wstring STRUCTURE_LAKE;
	static const wstring STRUCTURE_LAVA_LAKE;
	static const wstring STRUCTURE_DUNGEON;

private:
	vector<FlatLayerInfo *> layers;
	unordered_map<wstring, unordered_map<wstring, wstring> > structures;
	int biome;

public:
	FlatGeneratorInfo();
	~FlatGeneratorInfo();

	int getBiome();
	void setBiome(int biome);
	unordered_map<wstring, unordered_map<wstring, wstring> > *getStructures();
	vector<FlatLayerInfo *> *getLayers();
	void updateLayers();
	wstring toString();

private:
	static FlatLayerInfo *getLayerFromString(const wstring &input, int yOffset);
	static vector<FlatLayerInfo *> *getLayersFromString(const wstring &input);

public:
	static FlatGeneratorInfo *fromValue(const wstring &input);
	static FlatGeneratorInfo *getDefault();
};