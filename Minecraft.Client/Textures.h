#pragma once
#include "..\Minecraft.World\ArrayWithLength.h"

class DynamicTexture;
class BufferedImage;
class HttpTexture;
class MemTexture;
class TexturePackRepository;
class HttpTextureProcessor;
class MemTextureProcessor;
class Options;
using namespace std;
class IntBuffer;
class PreStitchedTextureMap;
class ResourceLocation;


typedef enum _TEXTURE_NAME
{
	TN__BLUR__MISC_PUMPKINBLUR,
//	TN__BLUR__MISC_VIGNETTE,		// Not currently used
	TN__CLAMP__MISC_SHADOW,
//	TN_ACHIEVEMENT_BG,				// Not currently used
	TN_ART_KZ,
	TN_ENVIRONMENT_CLOUDS,
	TN_ENVIRONMENT_RAIN,
	TN_ENVIRONMENT_SNOW,
	TN_GUI_GUI,
	TN_GUI_ICONS,
	TN_ITEM_ARROWS,
	TN_ITEM_BOAT,
	TN_ITEM_CART,
	TN_ITEM_SIGN,
	TN_MISC_MAPBG,
	TN_MISC_MAPICONS,
	TN_MISC_WATER,
	TN_MISC_FOOTSTEP,
	TN_MOB_SADDLE,
	TN_MOB_SHEEP_FUR,
	TN_MOB_SPIDER_EYES,
	TN_PARTICLES,
	TN_MOB_CHICKEN,
	TN_MOB_COW,
	TN_MOB_PIG,
	TN_MOB_SHEEP,
	TN_MOB_SQUID,
	TN_MOB_WOLF,
	TN_MOB_WOLF_TAME,
	TN_MOB_WOLF_ANGRY,
	TN_MOB_CREEPER,
	TN_MOB_GHAST,
	TN_MOB_GHAST_FIRE,
	TN_MOB_ZOMBIE,
	TN_MOB_PIGZOMBIE,
	TN_MOB_SKELETON,
	TN_MOB_SLIME,
	TN_MOB_SPIDER,
	TN_MOB_CHAR,
	TN_MOB_CHAR1,
	TN_MOB_CHAR2,
	TN_MOB_CHAR3,
	TN_MOB_CHAR4,
	TN_MOB_CHAR5,
	TN_MOB_CHAR6,
	TN_MOB_CHAR7,
	TN_TERRAIN_MOON,
	TN_TERRAIN_SUN,
	TN_POWERED_CREEPER,

	// 1.8.2
	TN_MOB_CAVE_SPIDER,
	TN_MOB_ENDERMAN,
	TN_MOB_SILVERFISH,
	TN_MOB_ENDERMAN_EYES, 
	TN_MISC_EXPLOSION,
	TN_ITEM_EXPERIENCE_ORB,
	TN_TILE_CHEST,
	TN_TILE_LARGE_CHEST,

	// 1.3.2
	TN_TILE_ENDER_CHEST,

	// 1.0.1
	TN_MOB_RED_COW,
	TN_MOB_SNOWMAN,
	TN_MOB_ENDERDRAGON,
	TN_MOB_BLAZE,
	TN_MOB_LAVA,
	TN_MOB_VILLAGER_VILLAGER,
	TN_MOB_VILLAGER_FARMER,
	TN_MOB_VILLAGER_LIBRARIAN,
	TN_MOB_VILLAGER_PRIEST,
	TN_MOB_VILLAGER_SMITH,
	TN_MOB_VILLAGER_BUTCHER,
	TN_MOB_ENDERDRAGON_ENDERCRYSTAL,
	TN_MOB_ENDERDRAGON_SHUFFLE,
	TN_MOB_ENDERDRAGON_BEAM,
	TN_MOB_ENDERDRAGON_ENDEREYES,
	TN__BLUR__MISC_GLINT,
	TN_ITEM_BOOK,
	TN_MISC_TUNNEL,
	TN_MISC_PARTICLEFIELD,
	TN_TERRAIN_MOON_PHASES,

	// 1.2.3
	TN_MOB_OCELOT,
	TN_MOB_CAT_BLACK,
	TN_MOB_CAT_RED,
	TN_MOB_CAT_SIAMESE,
	TN_MOB_VILLAGER_GOLEM,
	TN_MOB_WITHER_SKELETON,

	// TU14
	TN_MOB_WOLF_COLLAR,
	TN_MOB_ZOMBIE_VILLAGER,

	// 1.6.4
	TN_ITEM_LEASHKNOT,

	TN_MISC_BEACON_BEAM,

	TN_MOB_BAT,

	TN_MOB_DONKEY,
	TN_MOB_HORSE_BLACK,
	TN_MOB_HORSE_BROWN,
	TN_MOB_HORSE_CHESTNUT,
	TN_MOB_HORSE_CREAMY,
	TN_MOB_HORSE_DARKBROWN,
	TN_MOB_HORSE_GRAY,
	TN_MOB_HORSE_MARKINGS_BLACKDOTS,
	TN_MOB_HORSE_MARKINGS_WHITE,
	TN_MOB_HORSE_MARKINGS_WHITEDOTS,
	TN_MOB_HORSE_MARKINGS_WHITEFIELD,
	TN_MOB_HORSE_SKELETON,
	TN_MOB_HORSE_WHITE,
	TN_MOB_HORSE_ZOMBIE,
	TN_MOB_MULE,
	TN_MOB_HORSE_ARMOR_DIAMOND,
	TN_MOB_HORSE_ARMOR_GOLD,
	TN_MOB_HORSE_ARMOR_IRON,

	TN_MOB_WITCH,

	TN_MOB_WITHER,
	TN_MOB_WITHER_ARMOR,
	TN_MOB_WITHER_INVULNERABLE,

	TN_TILE_TRAP_CHEST,
	TN_TILE_LARGE_TRAP_CHEST,
	//TN_TILE_XMAS_CHEST,	
	//TN_TILE_LARGE_XMAS_CHEST,

#ifdef _LARGE_WORLDS
	TN_MISC_ADDITIONALMAPICONS,
#endif

	TN_DEFAULT_FONT,
	TN_ALT_FONT,

/*	TN_SP1,
	TN_SP2,
	TN_SP3,
	TN_SPF,
	TN_THST,
	TN_THIR,
	TN_THGO,
	TN_THDI,
	TN_GPAN,
	TN_GPCO,
	TN_GPEN,
	TN_GPFO,
	TN_GPTO,
	TN_GPBA,
	TN_GPFA,
	TN_GPME,
	TN_GPMF,
	TN_GPMM,
	TN_GPSE,
	TN_AH_0006,
	TN_AH_0003,
	TN_AH_0007,
	TN_AH_0005,
	TN_AH_0004,
	TN_AH_0001,
	TN_AH_0002,
	TN_AT_0001,
	TN_AT_0002,
	TN_AT_0003,
	TN_AT_0004,
	TN_AT_0005,
	TN_AT_0006,
	TN_AT_0007,
	TN_AT_0008,
	TN_AT_0009,
	TN_AT_0010,
	TN_AT_0011,
	TN_AT_0012,
	TN_AP_0001,
	TN_AP_0002,
	TN_AP_0003,
	TN_AP_0004,
	TN_AP_0005,
	TN_AP_0006,
	TN_AP_0007,
	TN_AP_0009,
	TN_AP_0010,
	TN_AP_0011,
	TN_AP_0012,
	TN_AP_0013,
	TN_AP_0014,
	TN_AP_0015,
	TN_AP_0016,
	TN_AP_0017,
	TN_AP_0018,
	TN_AA_0001,
	TN_AT_0013,
	TN_AT_0014,
	TN_AT_0015,
	TN_AT_0016,
	TN_AT_0017,
	TN_AT_0018,
	TN_AP_0019,
	TN_AP_0020,
	TN_AP_0021,
	TN_AP_0022,
	TN_AP_0023,
	TN_AH_0008,
	TN_AH_0009,*/

	TN_GUI_ITEMS,
	TN_TERRAIN,

	TN_COUNT,
	
} 
TEXTURE_NAME;

class Textures
{
public:
	static bool MIPMAP;
	static C4JRender::eTextureFormat TEXTURE_FORMAT;

private:
	static const wchar_t *preLoaded[TN_COUNT];
	static int preLoadedIdx[TN_COUNT];

	unordered_map<wstring, int> idMap;
    unordered_map<wstring, intArray> pixelsMap;
    unordered_map<int, BufferedImage *> loadedImages;
    //IntBuffer *pixels;	// 4J - removed so we don't have a permanent buffer kicking round using up 1MB

	unordered_map<wstring, HttpTexture *> httpTextures;
	// 4J-PB - Added for GTS textures
	unordered_map<wstring,MemTexture *> memTextures;
    Options *options;

private:
    TexturePackRepository *skins;
    BufferedImage *missingNo;
	PreStitchedTextureMap *terrain;
	PreStitchedTextureMap *items;

	int lastBoundId;
public:
	Textures(TexturePackRepository *skins, Options *options);

private:
	void loadIndexedTextures(); // 4J Added

public:
	intArray loadTexturePixels(TEXTURE_NAME texId, const wstring& resourceName);
private:
	intArray loadTexturePixels(BufferedImage *img);
    intArray loadTexturePixels(BufferedImage *img, intArray pixels);
	void setTextureFormat(const wstring& resourceName);	// 4J added

public:
	void bindTexture(const wstring &resourceName);
	void bindTexture(ResourceLocation *resource); // 4J Added
	void bindTextureLayers(ResourceLocation *resource);	// 4J added

	// 4J Made public for use in XUI controls
	void bind(int id);

	ResourceLocation *getTextureLocation(shared_ptr<Entity> entity);
	ResourceLocation *getTextureLocation(int iconType);

public:
	void clearLastBoundId();

private:
	int loadTexture(TEXTURE_NAME texId, const wstring& resourceName);
public:
	int loadTexture(int idx);			// 4J added
	int getTexture(BufferedImage *img, C4JRender::eTextureFormat format = C4JRender::TEXTURE_FORMAT_RxGyBzAw, bool mipmap = true);
    void loadTexture(BufferedImage *img, int id);
	void loadTexture(BufferedImage *img, int id, bool blur, bool clamp);
private:
	intArray anaglyph(intArray rawPixels);
public:
    void replaceTexture(intArray rawPixels, int w, int h, int id);
	void replaceTextureDirect(intArray rawPixels, int w, int h, int id);		// 4J added as optimisation
	void replaceTextureDirect(shortArray rawPixels, int w, int h, int id);		// 4J added as optimisation
	void releaseTexture(int id);
    int loadHttpTexture(const wstring& url, const wstring& backup);
	int loadHttpTexture(const wstring& url, int backup);	// 4J added
	bool hasHttpTexture(const wstring &url);
    HttpTexture *addHttpTexture(const wstring& url, HttpTextureProcessor *processor);
    void removeHttpTexture(const wstring& url);

	// 4J-PB - for the GTS textures
	int				loadMemTexture(const wstring& url, const wstring& backup);
	int				loadMemTexture(const wstring& url, int backup);
	MemTexture *	addMemTexture(const wstring& url, MemTextureProcessor *processor);
	//MemTexture *	getMemTexture(const wstring& url, MemTextureProcessor *processor);
	void			removeMemTexture(const wstring& url);

    void tick(bool updateTextures, bool tickDynamics = true);		// 4J added updateTextures parameter & tickDynamics
public:
	void reloadAll();
	void stitch();
	Icon *getMissingIcon(int type);
	BufferedImage *readImage(TEXTURE_NAME texId, const wstring& name); // Moved this to public for Font.cpp access
	// check list of title update textures to see if we need to use the UPDATE: drive
	static bool IsTUImage(TEXTURE_NAME texId, const wstring& name);
	static bool IsOriginalImage(TEXTURE_NAME texId, const wstring& name);
};
