#pragma once
#include "../Minecraft.World/MapItem.h"
class Options;
class Font;
class Textures;
class Player;
class MapItemSavedData;

class Minimap
{
private:
	static const int w = MapItem::IMAGE_WIDTH;
    static const int h = MapItem::IMAGE_HEIGHT;
#ifdef __ORBIS__
	static short LUT[256];	// 4J added
#else
	static int LUT[256];	// 4J added
#endif
	static bool genLUT;		// 4J added
	int renderCount;		// 4J added
	bool m_optimised;		// 4J Added
#ifdef __ORBIS__
	shortArray pixels;
#else
    intArray pixels;
#endif
    int mapTexture;
    Options *options;
    Font *font;

public:
	Minimap(Font *font, Options *options, Textures *textures, bool optimised = true); // 4J Added optimised param
	static void reloadColours();
    void render(shared_ptr<Player> player, Textures *textures, shared_ptr<MapItemSavedData> data, int entityId); // 4J added entityId param
};
