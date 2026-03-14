#include "stdafx.h"
#include "Textures.h"
#include "AbstractTexturePack.h"
#include "..\Minecraft.World\InputOutputStream.h"
#include "..\Minecraft.World\StringHelpers.h"

const unordered_map<std::wstring, std::wstring> AbstractTexturePack::INDEXED_TO_JAVA_MAP = {
	{L"res/misc/pumpkinblur.png", L"misc/pumpkinblur.png"},
	//	L"%blur%/misc/vignette",		// Not currently used
	{L"res/misc/shadow", L""},
	//	L"/achievement/bg",				// Not currently used
	{L"art/kz", L""},
	{L"res/environment/clouds.png", L"environment/clouds.png"},
	{L"res/environment/rain.png", L"environment/rain.png"},
	{L"res/environment/snow.png", L"environment/snow.png"},
	{L"gui/gui", L""},
	{L"gui/icons", L""},
	//{L"item/arrows", L""},
	//{L"item/boat", L""},
	//{L"item/cart", L""},
	//{L"item/sign", L""},
	{L"res/misc/mapbg", L""},
	{L"res/misc/mapicons", L""},
	{L"res/misc/water", L""},
	{L"res/misc/footprint", L""},
	//{L"mob/saddle", L""},
	{L"res/res/mob/sheep_fur.png", L"entity/sheep/sheep_fur.png"},
	{L"res/mob/spider_eyes.png", L""},
	{L"res/particles", L""},
	{L"res/mob/chicken.png", L"entity/chicken.png"},
	{L"res/mob/cow.png", L"entity/cow/cow.png"},
	{L"res/mob/pig.png", L"entity/pig/pig.png"},
	{L"res/mob/sheep.png", L"entity/sheep/sheep.png"},
	{L"res/mob/squid.png", L"entity/squid/squid.png"},
	{L"res/mob/wolf.png", L"entity/wolf/wolf.png"},
	{L"res/mob/wolf_tame.png", L"entity/wolf/wolf_tame.png"},
	{L"res/mob/wolf_angry.png", L"entity/wolf/wolf_angry.png"},
	{L"res/mob/creeper.png", L"entity/creeper/creeper.png"},
	{L"res/mob/ghast.png", L"entity/ghast/ghast.png"},
	{L"res/mob/ghast_fire.png", L"entity/ghast/ghast_shooting.png"},
	//{L"res/mob/zombie", L""}, // zombie uses 64x texture
	//{L"res/mob/pigzombie", L""}, // rip
	{L"res/mob/skeleton.png", L"entity/skeleton/skeleton.png"},
	{L"res/mob/slime.png", L"entity/slime/slime.png"},
	{L"res/mob/spider.png", L"entity/spider/spider.png"},
	//{L"mob/char", L""},
	//{L"mob/char1", L""},
	//{L"mob/char2", L""},
	//{L"mob/char3", L""},
	//{L"mob/char4", L""},
	//{L"mob/char5", L""},
	//{L"mob/char6", L""},
	//{L"mob/char7", L""},
	{L"terrain/moon", L""},
	{L"terrain/sun", L""},
	{L"armor/power", L""},

	// 1.8.2
	{L"res/mob/cavespider.png", L"entity/spider/cave_spider.png"},
	{L"res/mob/enderman.png", L"entity/enderman/enderman.png"},
	{L"res/mob/silverfish.png", L"entity/silverfish.png"},
	{L"res/mob/enderman_eyes.png", L"entity/enderman/enderman_eyes.png"},
	//{L"res/misc/explosion", L""}, // not bothering to reatlas them rn
	{L"res/item/xporb.png", L"entity/experience_orb.png"},
	{L"res/item/chest.png", L"entity/chest/normal.png"},
	//{L"item/largechest", L"entity/chest/normal"}, // was split in half

	// 1.3.2 
	{L"res/item/enderchest.png", L"entity/chest/ender.png"},

	// 1.0.1 
	{L"res/mob/redcow.png", L"entity/cow/mooshroom.png"},
	//{L"mob/snowman", L""},
	//{L"mob/enderdragon/ender", L""},
	{L"res/mob/fire.png", L"entity/blaze.png"},
	{L"res/mob/lava.png", L"entity/slime/magmacube.png"},
	//{L"mob/villager/villager", L""},
	//{L"mob/villager/farmer", L""},
	//{L"mob/villager/librarian", L""},
	//{L"mob/villager/priest", L""},
	//{L"mob/villager/smith", L""},
	//{L"mob/villager/butcher", L""},
	{L"res/mob/enderdragon/crystal.png", L"entity/end_crystal/end_crystal.png"},
	//{L"mob/enderdragon/shuffle", L""},
	{L"res/mob/enderdragon/beam.png", L"entity/end_crystal/end_crystal_beam.png"},
	//{L"mob/enderdragon/ender_eyes", L""},
	{L"res/misc/glint.png", L"misc/enchanted_item_glint.png"},
	//{L"item/book", L""},
	{L"res/misc/tunnel.png", L"environment/end_sky.png"},
	{L"res/misc/particlefield.png", L"entity/end_portal.png"},
	{L"res/terrain/moon_phases.png", L"environment/moon_phases.png"},

	// 1.2.3
	{L"res/mob/ozelot.png", L"entity/cat/ocelot.png"},
	{L"res/mob/cat_black.png", L"entity/cat/black.png"},
	{L"res/mob/cat_red.png", L"entity/cat/red.png"},
	{L"res/mob/cat_siamese.png", L"entity/cat/siamese.png"},
	{L"res/mob/villager_golem.png", L"entity/iron_golem.png"},
	{L"res/mob/skeleton_wither.png", L"entity/wither_skeleton.png"},

	// TU 14
	{L"res/mob/wolf_collar.png", L"entity/wolf/wolf_collar.png"},
	//{L"mob/zombie_villager", L""},

	// 1.6.4
	{L"res/item/lead_knot.png", L"entity/lead_knot.png"},

	{L"res/misc/beacon_beam.png", L"entity/beacon_beam.png"},

	//{L"res/mob/bat.png", L"entity/bat.png"}, // new uvs is broken

	// incompatible horse armor
	//{L"mob/horse/donkey", L""},
	//{L"mob/horse/horse_black", L""},
	//{L"mob/horse/horse_brown", L""},
	//{L"mob/horse/horse_chestnut", L""},
	//{L"mob/horse/horse_creamy", L""},
	//{L"mob/horse/horse_darkbrown", L""},
	//{L"mob/horse/horse_gray", L""},
	//{L"mob/horse/horse_markings_blackdots", L""},
	//{L"mob/horse/horse_markings_white", L""},
	//{L"mob/horse/horse_markings_whitedots", L""},
	//{L"mob/horse/horse_markings_whitefield", L""},
	//{L"mob/horse/horse_skeleton", L""},
	//{L"mob/horse/horse_white", L""},
	//{L"mob/horse/horse_zombie", L""},
	//{L"mob/horse/mule", L""},

	//{L"mob/horse/armor/horse_armor_diamond", L""},
	//{L"mob/horse/armor/horse_armor_gold", L""},
	//{L"mob/horse/armor/horse_armor_iron", L""},

	{ L"res/mob/witch", L"entity/witch.png" },

	{ L"res/mob/wither/wither.png", L"entity/wither/wither.png" },
	{ L"res/mob/wither/wither_armor.png", L"entity/wither/wither_armor.png" },
	{ L"res/mob/wither/wither_invulnerable.png", L"entity/wither/wither_invulnerable.png" },

	{ L"res/item/trapped.png", L"entity/chest/trapped.png" },
	//{ L"item/trapped_double", L"" },
	//L"item/christmas",
	//L"item/christmas_double",

#ifdef _LARGE_WORLDS
	//{L"misc/additionalmapicons", L""},
#endif

	//{L"font/Default", L""},
	//{L"font/alternate", L""},

	// skin packs 
/*	{L"/SP1", L""},
	{L"/SP2", L""},
	{L"/SP3", L""},
	{L"/SPF", L""},
	{ L""},
	{// themes L""},
	{L"/ThSt", L""},
	{L"/ThIr", L""},
	{L"/ThGo", L""},
	{L"/ThDi", L""},
	{ L""},
	{// gamerpics L""},
	{L"/GPAn", L""},
	{L"/GPCo", L""},
	{L"/GPEn", L""},
	{L"/GPFo", L""},
	{L"/GPTo", L""},
	{L"/GPBA", L""},
	{L"/GPFa", L""},
	{L"/GPME", L""},
	{L"/GPMF", L""},
	{L"/GPMM", L""},
	{L"/GPSE", L""},
	{ L""},
	{// avatar items L""},
	{ L""},
	{L"/AH_0006", L""},
	{L"/AH_0003", L""},
	{L"/AH_0007", L""},
	{L"/AH_0005", L""},
	{L"/AH_0004", L""},
	{L"/AH_0001", L""},
	{L"/AH_0002", L""},
	{L"/AT_0001", L""},
	{L"/AT_0002", L""},
	{L"/AT_0003", L""},
	{L"/AT_0004", L""},
	{L"/AT_0005", L""},
	{L"/AT_0006", L""},
	{L"/AT_0007", L""},
	{L"/AT_0008", L""},
	{L"/AT_0009", L""},
	{L"/AT_0010", L""},
	{L"/AT_0011", L""},
	{L"/AT_0012", L""},
	{L"/AP_0001", L""},
	{L"/AP_0002", L""},
	{L"/AP_0003", L""},
	{L"/AP_0004", L""},
	{L"/AP_0005", L""},
	{L"/AP_0006", L""},
	{L"/AP_0007", L""},
	{L"/AP_0009", L""},
	{L"/AP_0010", L""},
	{L"/AP_0011", L""},
	{L"/AP_0012", L""},
	{L"/AP_0013", L""},
	{L"/AP_0014", L""},
	{L"/AP_0015", L""},
	{L"/AP_0016", L""},
	{L"/AP_0017", L""},
	{L"/AP_0018", L""},
	{L"/AA_0001", L""},
	{L"/AT_0013", L""},
	{L"/AT_0014", L""},
	{L"/AT_0015", L""},
	{L"/AT_0016", L""},
	{L"/AT_0017", L""},
	{L"/AT_0018", L""},
	{L"/AP_0019", L""},
	{L"/AP_0020", L""},
	{L"/AP_0021", L""},
	{L"/AP_0022", L""},
	{L"/AP_0023", L""},
	{L"/AH_0008", L""},
	{L"/AH_0009", L""},*/

	//{L"gui/items", L""},
	//{L"terrain", L""}
};

AbstractTexturePack::AbstractTexturePack(DWORD id, File *file, const wstring &name, TexturePack *fallback) : id(id), name(name)
{
	// 4J init
	textureId = -1;
	m_colourTable = NULL;


	this->file = file;
	this->fallback = fallback;

	m_iconData = NULL;
	m_iconSize = 0;

	m_comparisonData = NULL;
	m_comparisonSize = 0;

	// 4J Stu - These calls need to be in the most derived version of the class
	//loadIcon();
	//loadDescription();
}

wstring AbstractTexturePack::trim(wstring line)
{
	if (!line.empty() && line.length() > 34)
	{
		line = line.substr(0, 34);
	}
	return line;
}

void AbstractTexturePack::loadIcon()
{
#ifdef _XBOX
	// 4J Stu - Temporary only	
	const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string 
	WCHAR szResourceLocator[ LOCATOR_SIZE ];

	const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(NULL);
	swprintf(szResourceLocator, LOCATOR_SIZE ,L"section://%X,%ls#%ls",c_ModuleHandle,L"media", L"media/Graphics/TexturePackIcon.png");

	UINT size = 0;
	HRESULT hr = XuiResourceLoadAllNoLoc(szResourceLocator, &m_iconData, &size);
	m_iconSize = size;
#endif
}

void AbstractTexturePack::loadComparison()
{
#ifdef _XBOX
	// 4J Stu - Temporary only	
	const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string 
	WCHAR szResourceLocator[ LOCATOR_SIZE ];

	const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(NULL);
	swprintf(szResourceLocator, LOCATOR_SIZE ,L"section://%X,%ls#%ls",c_ModuleHandle,L"media", L"media/Graphics/DefaultPack_Comparison.png");

	UINT size = 0;
	HRESULT hr = XuiResourceLoadAllNoLoc(szResourceLocator, &m_comparisonData, &size);
	m_comparisonSize = size;
#endif
}

void AbstractTexturePack::loadDescription()
{
	// 4J Unused currently
#if 0
	InputStream *inputStream = NULL;
	BufferedReader *br = NULL;
	//try {
	inputStream = getResourceImplementation(L"/pack.txt");
	br = new BufferedReader(new InputStreamReader(inputStream));
	desc1 = trim(br->readLine());
	desc2 = trim(br->readLine());
	//} catch (IOException ignored) {
	//} finally {
	// TODO [EB]: use IOUtils.closeSilently()
	//	try {
	if (br != NULL)
	{
		br->close();
		delete br;
	}
	if (inputStream != NULL)
	{
		inputStream->close();
		delete inputStream;
	}
	//	} catch (IOException ignored) {
	//	}
	//}
#endif
}

void AbstractTexturePack::loadName()
{
}

void AbstractTexturePack::checkTexSize() {
	BufferedImage* img = getImageResource(L"dirt.png", true);
	if (img != NULL) {
		int width = img->getWidth();
		int height = img->getHeight();
		if (width != height) {
			app.DebugPrintf("Warning: Texture pack contains texture with bad size: %d x %d\n", width, height);
		} else
			texSize = width;
		delete img;
	} else {
		texSize = 16;
	}
}

InputStream *AbstractTexturePack::getResource(const wstring &name, bool allowFallback) //throws IOException
{
	app.DebugPrintf("texture - %ls\n",name.c_str());
	InputStream *is = getResourceImplementation(name);
	if (is == NULL && fallback != NULL && allowFallback)
	{
		is = fallback->getResource(name, true);
	}

	return is;
}

// 4J Currently removed due to override in TexturePack class
//InputStream *AbstractTexturePack::getResource(const wstring &name) //throws IOException
//{
//	return getResource(name, true);
//}

void AbstractTexturePack::unload(Textures *textures)
{
	if (iconImage != NULL && textureId != -1)
	{
		textures->releaseTexture(textureId);
	}
}

void AbstractTexturePack::load(Textures *textures)
{
	if (iconImage != NULL)
	{
		if (textureId == -1)
		{
			textureId = textures->getTexture(iconImage);
		}
		glBindTexture(GL_TEXTURE_2D, textureId);
		textures->clearLastBoundId();
	}
	else
	{
		// 4J Stu - Don't do this
		//textures->bindTexture(L"/gui/unknown_pack.png");
	}
}

bool AbstractTexturePack::hasFile(const wstring &name, bool allowFallback)
{
	if (name == L"res/terrain.png" && terrainAtlas != nullptr)
		return true;

	if (name == L"res/items.png" && itemAtlas != nullptr)
		return true;

	bool hasFile = this->hasFile(name);

	auto it = INDEXED_TO_JAVA_MAP.find(name);
	if (it != INDEXED_TO_JAVA_MAP.end()) {
		hasFile = this->hasFile(L"assets/minecraft/textures/" + it->second);
	}

	return !hasFile && (allowFallback && fallback != NULL) ? fallback->hasFile(name, allowFallback) : hasFile;
}

DWORD AbstractTexturePack::getId()
{
	return id;
}

wstring AbstractTexturePack::getName()
{
	return texname;
}

wstring AbstractTexturePack::getWorldName()
{
	return m_wsWorldName;
}

wstring AbstractTexturePack::getDesc1()
{
	return desc1;
}

wstring AbstractTexturePack::getDesc2()
{
	return desc2;
}

wstring AbstractTexturePack::getAnimationString(const wstring &textureName, const wstring &path, bool allowFallback)
{
	return getAnimationString(textureName, path);
}

wstring AbstractTexturePack::getAnimationString(const wstring &textureName, const wstring &path)
{
	wstring animationDefinitionFile = textureName + L".txt";

	bool requiresFallback = !hasFile(L"\\" + textureName + L".png", false);
	
	wstring result = L"";

	InputStream *fileStream = getResource(L"\\" + path + animationDefinitionFile, requiresFallback);

	if(fileStream)
	{
		//Minecraft::getInstance()->getLogger().info("Found animation info for: " + animationDefinitionFile);
#ifndef _CONTENT_PACKAGE
		app.DebugPrintf("Found animation info for: %ls\n", animationDefinitionFile.c_str() );
#endif
		InputStreamReader isr(fileStream);
		BufferedReader br(&isr);


		wstring line = br.readLine();
		while (!line.empty())
		{
			line = trimString(line);
			if (line.length() > 0)
			{
				result.append(L",");
				result.append(line);
			}
			line = br.readLine();
		}
		delete fileStream;
	}

	return result;
}

BufferedImage *AbstractTexturePack::getImageResource(const wstring& File, bool filenameHasExtension /*= false*/, bool bTitleUpdateTexture /*=false*/, const wstring &drive /*=L""*/)
{
	const char *pchTexture=wstringtofilename(File);
	app.DebugPrintf("AbstractTexturePack::getImageResource - %s, drive is %s\n",pchTexture, wstringtofilename(drive));

	return new BufferedImage(TexturePack::getResource(L"/" + File),filenameHasExtension,bTitleUpdateTexture,drive);
}

void AbstractTexturePack::loadDefaultUI()
{
#ifdef _XBOX
	// load from the .xzp file
	const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(NULL);

	// Load new skin
	const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string 
	WCHAR szResourceLocator[ LOCATOR_SIZE ];

	swprintf(szResourceLocator, LOCATOR_SIZE,L"section://%X,%ls#%ls",c_ModuleHandle,L"media", L"media/skin_Minecraft.xur");
	
	XuiFreeVisuals(L"");
	app.LoadSkin(szResourceLocator,NULL);//L"TexturePack");
	//CXuiSceneBase::GetInstance()->SetVisualPrefix(L"TexturePack");
	CXuiSceneBase::GetInstance()->SkinChanged(CXuiSceneBase::GetInstance()->m_hObj);
#else
	ui.ReloadSkin();
#endif
}

void AbstractTexturePack::loadColourTable()
{
	loadDefaultColourTable();
	loadDefaultHTMLColourTable();
}

void AbstractTexturePack::loadDefaultColourTable()
{
	// Load the file
#ifdef __PS3__
	// need to check if it's a BD build, so pass in the name
	File coloursFile(AbstractTexturePack::getPath(true,app.GetBootedFromDiscPatch()?"colours.col":NULL).append(L"res/colours.col"));

#else
	File coloursFile(AbstractTexturePack::getPath(true).append(L"res/colours.col"));
#endif


	if(coloursFile.exists())
	{
		DWORD dwLength = coloursFile.length();
		byteArray data(dwLength);

		FileInputStream fis(coloursFile);
		fis.read(data,0,dwLength);
		fis.close();
		if(m_colourTable != NULL) delete m_colourTable;
		m_colourTable = new ColourTable(data.data, dwLength);

		delete [] data.data;
	}
	else
	{
		app.DebugPrintf("Failed to load the default colours table\n");
		app.FatalLoadError();
	}
}

void AbstractTexturePack::loadDefaultHTMLColourTable()
{
#ifdef _XBOX
	// load from the .xzp file
	const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(NULL);

	const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string 
	WCHAR szResourceLocator[ LOCATOR_SIZE ];

	// Try and load the HTMLColours.col based off the common XML first, before the deprecated xuiscene_colourtable	
	wsprintfW(szResourceLocator,L"section://%X,%s#%s",c_ModuleHandle,L"media", L"media/HTMLColours.col");
	BYTE *data;
	UINT dataLength;
	if(XuiResourceLoadAll(szResourceLocator, &data, &dataLength) == S_OK)
	{
		m_colourTable->loadColoursFromData(data,dataLength);

		XuiFree(data);
	}
	else
	{
		wsprintfW(szResourceLocator,L"section://%X,%s#%s",c_ModuleHandle,L"media", L"media/");
		HXUIOBJ hScene;
		HRESULT hr = XuiSceneCreate(szResourceLocator,L"xuiscene_colourtable.xur", NULL, &hScene);

		if(HRESULT_SUCCEEDED(hr))
		{
			loadHTMLColourTableFromXuiScene(hScene);
		}
	}
#else
	if(app.hasArchiveFile(L"HTMLColours.col"))
	{
		byteArray textColours = app.getArchiveFile(L"HTMLColours.col");
		m_colourTable->loadColoursFromData(textColours.data,textColours.length);

		delete [] textColours.data;
	}
#endif
}

#ifdef _XBOX
void AbstractTexturePack::loadHTMLColourTableFromXuiScene(HXUIOBJ hObj)
{
	HXUIOBJ child;
	HRESULT hr = XuiElementGetFirstChild(hObj, &child);

	while(HRESULT_SUCCEEDED(hr) && child != NULL)
	{
		LPCWSTR childName;
		XuiElementGetId(child,&childName);
		m_colourTable->setColour(childName,XuiTextElementGetText(child));

		//eMinecraftTextColours colourIndex = eTextColor_NONE;
		//for(int i = 0; i < (int)eTextColor_MAX; i++)
		//{
		//	if(wcscmp(HTMLColourTableElements[i],childName)==0)
		//	{
		//		colourIndex = (eMinecraftTextColours)i;
		//		break;
		//	}
		//}

		//LPCWSTR stringValue = XuiTextElementGetText(child);

		//m_htmlColourTable[colourIndex] = XuiTextElementGetText(child);

		hr = XuiElementGetNext(child, &child);
	}
}
#endif

void AbstractTexturePack::loadUI()
{
	loadColourTable();
	
#ifdef _XBOX
	CXuiSceneBase::GetInstance()->SkinChanged(CXuiSceneBase::GetInstance()->m_hObj);
#endif
}

void AbstractTexturePack::unloadUI()
{
	// Do nothing
}

wstring AbstractTexturePack::getXuiRootPath()
{
	const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(NULL);

	// Load new skin
	const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string 
	WCHAR szResourceLocator[ LOCATOR_SIZE ];

	swprintf(szResourceLocator, LOCATOR_SIZE,L"section://%X,%ls#%ls",c_ModuleHandle,L"media", L"media/");
	return szResourceLocator;
}

PBYTE AbstractTexturePack::getPackIcon(DWORD &dwImageBytes)
{
	if(m_iconSize == 0 || m_iconData == NULL) loadIcon();
	dwImageBytes = m_iconSize;
	return m_iconData;
}

PBYTE AbstractTexturePack::getPackComparison(DWORD &dwImageBytes)
{
	if(m_comparisonSize == 0 || m_comparisonData == NULL) loadComparison();

	dwImageBytes = m_comparisonSize;
	return m_comparisonData;
}

unsigned int AbstractTexturePack::getDLCParentPackId()
{
	return 0;
}

unsigned char AbstractTexturePack::getDLCSubPackId()
{
	return 0;
}