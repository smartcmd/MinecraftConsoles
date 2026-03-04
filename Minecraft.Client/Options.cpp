#include "stdafx.h"
#include "toml11.hpp"
#include "Options.h"
#include "KeyMapping.h"
#include "LevelRenderer.h"
#include "Textures.h"
#include "..\Minecraft.World\net.minecraft.locale.h"
#include "..\Minecraft.World\Language.h"
#include "..\Minecraft.World\File.h"
#include "..\Minecraft.World\FileInputStream.h"
#include "..\Minecraft.World\FileOutputStream.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "Common\App_enums.h"

// 4J - the Option sub-class used to be an java enumerated type, trying to emulate that functionality here
const Options::Option Options::Option::options[17] =
{
	Options::Option(L"options.music", true, false),
	Options::Option(L"options.sound", true, false),
	Options::Option(L"options.invertMouse", false, true),
	Options::Option(L"options.sensitivity", true, false),
	Options::Option(L"options.renderDistance", false, false),
	Options::Option(L"options.viewBobbing", false, true),
	Options::Option(L"options.anaglyph", false, true),
	Options::Option(L"options.advancedOpengl", false, true),
	Options::Option(L"options.framerateLimit", false, false),
	Options::Option(L"options.difficulty", false, false),
	Options::Option(L"options.graphics", false, false),
	Options::Option(L"options.ao", false, true),
	Options::Option(L"options.guiScale", false, false),
	Options::Option(L"options.fov", true, false),
	Options::Option(L"options.gamma", true, false),
	Options::Option(L"options.renderClouds",false, true),
	Options::Option(L"options.particles", false, false),
};

const Options::Option *Options::Option::MUSIC = &Options::Option::options[0];
const Options::Option *Options::Option::SOUND = &Options::Option::options[1];
const Options::Option *Options::Option::INVERT_MOUSE = &Options::Option::options[2];
const Options::Option *Options::Option::SENSITIVITY = &Options::Option::options[3];
const Options::Option *Options::Option::RENDER_DISTANCE = &Options::Option::options[4];
const Options::Option *Options::Option::VIEW_BOBBING = &Options::Option::options[5];
const Options::Option *Options::Option::ANAGLYPH = &Options::Option::options[6];
const Options::Option *Options::Option::ADVANCED_OPENGL = &Options::Option::options[7];
const Options::Option *Options::Option::FRAMERATE_LIMIT = &Options::Option::options[8];
const Options::Option *Options::Option::DIFFICULTY = &Options::Option::options[9];
const Options::Option *Options::Option::GRAPHICS = &Options::Option::options[10];
const Options::Option *Options::Option::AMBIENT_OCCLUSION = &Options::Option::options[11];
const Options::Option *Options::Option::GUI_SCALE = &Options::Option::options[12];
const Options::Option *Options::Option::FOV = &Options::Option::options[13];
const Options::Option *Options::Option::GAMMA = &Options::Option::options[14];
const Options::Option *Options::Option::RENDER_CLOUDS = &Options::Option::options[15];
const Options::Option *Options::Option::PARTICLES = &Options::Option::options[16];


const Options::Option *Options::Option::getItem(int id)
{
	return &options[id];
}

Options::Option::Option(const wstring& captionId, bool hasProgress, bool isBoolean) : _isProgress(hasProgress), _isBoolean(isBoolean), captionId(captionId)
{
}

bool Options::Option::isProgress() const
{
	return _isProgress;
}

bool Options::Option::isBoolean() const
{
	return _isBoolean;
}

int	Options::Option::getId() const
{
	return (int)(this-options);
}

wstring Options::Option::getCaptionId() const
{
	return captionId;
}

const wstring Options::RENDER_DISTANCE_NAMES[] =
{
        L"options.renderDistance.far", L"options.renderDistance.normal", L"options.renderDistance.short", L"options.renderDistance.tiny"
};
const wstring Options::DIFFICULTY_NAMES[] =
{
        L"options.difficulty.peaceful", L"options.difficulty.easy", L"options.difficulty.normal", L"options.difficulty.hard"
};
const wstring Options::GUI_SCALE[] =
{
        L"options.guiScale.auto", L"options.guiScale.small", L"options.guiScale.normal", L"options.guiScale.large"
};
const wstring Options::FRAMERATE_LIMITS[] =
{
        L"performance.max", L"performance.balanced", L"performance.powersaver"
};

const wstring Options::PARTICLES[] = {
	L"options.particles.all", L"options.particles.decreased", L"options.particles.minimal"
};

// 4J added
void Options::init()
{
    music = 1;
    sound = 1;
    sensitivity = 0.5f;
    invertYMouse = false;
    viewDistance = 0;
    bobView = true;
    anaglyph3d = false;
    advancedOpengl = false;
    framerateLimit = 0;
    fancyGraphics = true;
    ambientOcclusion = true;
	renderClouds = true;
    skin = L"Default";

    keyUp = new KeyMapping(L"key.forward", Keyboard::KEY_W);
    keyLeft = new KeyMapping(L"key.left", Keyboard::KEY_A);
    keyDown = new KeyMapping(L"key.back", Keyboard::KEY_S);
    keyRight = new KeyMapping(L"key.right", Keyboard::KEY_D);
    keyJump = new KeyMapping(L"key.jump", Keyboard::KEY_SPACE);
    keyBuild = new KeyMapping(L"key.inventory", Keyboard::KEY_E);
    keyDrop = new KeyMapping(L"key.drop", Keyboard::KEY_Q);
    keyChat = new KeyMapping(L"key.chat", Keyboard::KEY_T);
    keySneak = new KeyMapping(L"key.sneak", Keyboard::KEY_LSHIFT);
	keyAttack = new KeyMapping(L"key.attack", -100 + 0);
    keyUse = new KeyMapping(L"key.use", -100 + 1);
    keyPlayerList = new KeyMapping(L"key.playerlist", Keyboard::KEY_TAB);
    keyPickItem = new KeyMapping(L"key.pickItem", -100 + 2);
    keyToggleFog = new KeyMapping(L"key.fog", Keyboard::KEY_F);

	keyMappings[0] = keyAttack;
	keyMappings[1] = keyUse;
    keyMappings[2] = keyUp;
	keyMappings[3] = keyLeft;
	keyMappings[4] = keyDown;
	keyMappings[5] = keyRight;
	keyMappings[6] = keyJump;
	keyMappings[7] = keySneak;
	keyMappings[8] = keyDrop;
	keyMappings[9] = keyBuild;
	keyMappings[10] = keyChat;
	keyMappings[11] = keyPlayerList;
	keyMappings[12] = keyPickItem;
	keyMappings[13] = keyToggleFog;

	// Controller action defaults (maps action index to gamepad button bitmask)
	// These correspond to the default MAP_STYLE_0 layout
	controllerDefaults[0]  = _360_JOY_BUTTON_RT;     // Attack/Mine
	controllerDefaults[1]  = _360_JOY_BUTTON_LT;     // Use/Place
	controllerDefaults[2]  = _360_JOY_BUTTON_A;      // Jump
	controllerDefaults[3]  = _360_JOY_BUTTON_RTHUMB;  // Sneak
	controllerDefaults[4]  = _360_JOY_BUTTON_B;      // Drop
	controllerDefaults[5]  = _360_JOY_BUTTON_Y;      // Inventory
	controllerDefaults[6]  = _360_JOY_BUTTON_X;      // Crafting
	controllerDefaults[7]  = _360_JOY_BUTTON_LB;     // Scroll Left
	controllerDefaults[8]  = _360_JOY_BUTTON_RB;     // Scroll Right
	controllerDefaults[9]  = _360_JOY_BUTTON_BACK;   // Game Info
	controllerDefaults[10] = _360_JOY_BUTTON_START;   // Pause
	controllerDefaults[11] = _360_JOY_BUTTON_DPAD_UP; // Third Person
	controllerDefaults[12] = _360_JOY_BUTTON_LTHUMB;  // Sprint

	for (int i = 0; i < CONTROLLER_ACTIONS; i++)
		controllerMappings[i] = controllerDefaults[i];

	minecraft = NULL;
	//optionsFile = NULL;

	difficulty = 2;
	hideGui = false;
	thirdPersonView = false;
	renderDebug = false;
	lastMpIp = L"";

	isFlying = false;
	smoothCamera = false;
	fixedCamera = false;
	flySpeed = 1;
	cameraSpeed = 1;
	guiScale = 0;
	particles = 0;
	fov = 0;
	gamma = 0;
}

Options::Options(Minecraft *minecraft, File workingDirectory)
{
	init();
	this->minecraft = minecraft;
	optionsFile = File(L"options.toml");
	load();
}

Options::Options()
{
	init();
}

wstring Options::getKeyDescription(int i)
{
    Language *language = Language::getInstance();
    return language->getElement(keyMappings[i]->name);
}

wstring Options::getKeyMessage(int i)
{
	int key = keyMappings[i]->key;
	if (key < 0) {
		return I18n::get(L"key.mouseButton", key + 101);
	} else {
		return Keyboard::getKeyName(keyMappings[i]->key);
	}
}

void Options::setKey(int i, int key)
{
    keyMappings[i]->key = key;
    save();
}

void Options::set(const Options::Option *item, float fVal)
{
    if (item == Option::MUSIC)
	{
        music = fVal;
#ifdef _XBOX
        minecraft->soundEngine->updateMusicVolume(fVal*2.0f);
#else
		minecraft->soundEngine->updateMusicVolume(fVal);
#endif
    }
    if (item == Option::SOUND)
	{
        sound = fVal;
#ifdef _XBOX
        minecraft->soundEngine->updateSoundEffectVolume(fVal*2.0f);
#else
		minecraft->soundEngine->updateSoundEffectVolume(fVal);
#endif
    }
    if (item == Option::SENSITIVITY)
	{
        sensitivity = fVal;
    }
	if (item == Option::FOV)
	{
		fov = fVal;
	}
	if (item == Option::GAMMA)
	{
		gamma = fVal;
	}
}

void Options::toggle(const Options::Option *option, int dir)
{
    if (option == Option::INVERT_MOUSE) invertYMouse = !invertYMouse;
    if (option == Option::RENDER_DISTANCE) viewDistance = (viewDistance + dir) & 3;
    if (option == Option::GUI_SCALE) guiScale = (guiScale + dir) & 3;
	if (option == Option::PARTICLES) particles = (particles + dir) % 3;

	// 4J-PB - changing
	//if (option == Option::VIEW_BOBBING) bobView = !bobView;
	if (option == Option::VIEW_BOBBING) ((dir==0)?bobView=false: bobView=true);
	if (option == Option::RENDER_CLOUDS) renderClouds = !renderClouds;
    if (option == Option::ADVANCED_OPENGL)
	{
        advancedOpengl = !advancedOpengl;
        minecraft->levelRenderer->allChanged();
    }
    if (option ==  Option::ANAGLYPH)
	{
        anaglyph3d = !anaglyph3d;
        minecraft->textures->reloadAll();
    }
    if (option ==  Option::FRAMERATE_LIMIT) framerateLimit = (framerateLimit + dir + 3) % 3;

	// 4J-PB - Change for Xbox
	//if (option ==  Option::DIFFICULTY) difficulty = (difficulty + dir) & 3;
	if (option ==  Option::DIFFICULTY) difficulty = (dir) & 3;

	app.DebugPrintf("Option::DIFFICULTY = %d",difficulty);

    if (option ==  Option::GRAPHICS)
	{
        fancyGraphics = !fancyGraphics;
        minecraft->levelRenderer->allChanged();
    }
    if (option == Option::AMBIENT_OCCLUSION)
	{
        ambientOcclusion = !ambientOcclusion;
        minecraft->levelRenderer->allChanged();
    }

	// 4J-PB - don't do the file save on the xbox
    // save();

}

float Options::getProgressValue(const Options::Option *item)
{
	if (item == Option::FOV) return fov;
	if (item == Option::GAMMA) return gamma;
    if (item == Option::MUSIC) return music;
    if (item == Option::SOUND) return sound;
    if (item == Option::SENSITIVITY) return sensitivity;
    return 0;
}

bool Options::getBooleanValue(const Options::Option *item)
{
	// 4J - was a switch statement which we can't do with our Option:: pointer types
	if( item == Option::INVERT_MOUSE) return invertYMouse;
	if( item == Option::VIEW_BOBBING) return bobView;
	if( item == Option::ANAGLYPH) return anaglyph3d;
	if( item == Option::ADVANCED_OPENGL) return advancedOpengl;
	if( item == Option::AMBIENT_OCCLUSION) return ambientOcclusion;
    if( item == Option::RENDER_CLOUDS) return renderClouds;
	return false;
}

wstring Options::getMessage(const Options::Option *item)
{
	// 4J TODO, should these wstrings append rather than add?

    Language *language = Language::getInstance();
    wstring caption = language->getElement(item->getCaptionId()) + L": ";

    if (item->isProgress())
	{
        float progressValue = getProgressValue(item);

        if (item == Option::SENSITIVITY)
		{
            if (progressValue == 0)
			{
                return caption + language->getElement(L"options.sensitivity.min");
            }
            if (progressValue == 1)
			{
                return caption + language->getElement(L"options.sensitivity.max");
            }
			return caption + _toString<int>((int) (progressValue * 200)) + L"%";
		} else if (item == Option::FOV)
		{
			if (progressValue == 0)
			{
				return caption + language->getElement(L"options.fov.min");
			}
			if (progressValue == 1)
			{
				return caption + language->getElement(L"options.fov.max");
			}
			return caption + _toString<int>((int) (70 + progressValue * 40));
		} else if (item == Option::GAMMA)
		{
			if (progressValue == 0)
			{
				return caption + language->getElement(L"options.gamma.min");
			}
			if (progressValue == 1)
			{
				return caption + language->getElement(L"options.gamma.max");
			}
			return caption + L"+" + _toString<int>((int) (progressValue * 100)) + L"%";
        }
		else
		{
            if (progressValue == 0)
			{
                return caption + language->getElement(L"options.off");
            }
            return caption + _toString<int>((int) (progressValue * 100)) + L"%";
        }
    } else if (item->isBoolean())
	{

        bool booleanValue = getBooleanValue(item);
        if (booleanValue)
		{
            return caption + language->getElement(L"options.on");
        }
        return caption + language->getElement(L"options.off");
    }
	else if (item == Option::RENDER_DISTANCE)
	{
        return caption + language->getElement(RENDER_DISTANCE_NAMES[viewDistance]);
    }
	else if (item == Option::DIFFICULTY)
	{
        return caption + language->getElement(DIFFICULTY_NAMES[difficulty]);
    }
	else if (item == Option::GUI_SCALE)
	{
        return caption + language->getElement(GUI_SCALE[guiScale]);
	}
	else if (item == Option::PARTICLES)
	{
		return caption + language->getElement(PARTICLES[particles]);
    }
	else if (item == Option::FRAMERATE_LIMIT)
	{
        return caption + I18n::get(FRAMERATE_LIMITS[framerateLimit]);
    }
	else if (item == Option::GRAPHICS)
	{
        if (fancyGraphics)
		{
            return caption + language->getElement(L"options.graphics.fancy");
        }
        return caption + language->getElement(L"options.graphics.fast");
    }

    return caption;

}

// String conversion helpers for toml++ (which uses std::string) and the game (which uses wstring)
static std::string narrowStr(const wstring &ws)
{
	std::string s;
	for (unsigned int i = 0; i < ws.size(); i++)
		s += (char)ws[i];
	return s;
}

static wstring widenStr(const std::string &s)
{
	wstring ws;
	for (unsigned int i = 0; i < s.size(); i++)
		ws += (wchar_t)s[i];
	return ws;
}

static void writeFileBytes(FileOutputStream &fos, const std::string &content)
{
	for (unsigned int i = 0; i < content.size(); i++)
		fos.write((unsigned int)(unsigned char)content[i]);
}

// Reverse lookup: key display name -> Keyboard key code (inverse of Keyboard::getKeyName)
static int getKeyByName(const std::string &name)
{
	if (name.size() == 1 && name[0] >= 'A' && name[0] <= 'Z')
		return Keyboard::KEY_A + (name[0] - 'A');
	if (name.size() == 1 && name[0] >= '1' && name[0] <= '9')
		return Keyboard::KEY_1 + (name[0] - '1');
	if (name == "0") return Keyboard::KEY_0;
	if (name == "SPACE") return Keyboard::KEY_SPACE;
	if (name == "L.SHIFT") return Keyboard::KEY_LSHIFT;
	if (name == "R.SHIFT") return Keyboard::KEY_RSHIFT;
	if (name == "ESCAPE") return Keyboard::KEY_ESCAPE;
	if (name == "BACKSPACE") return Keyboard::KEY_BACK;
	if (name == "ENTER") return Keyboard::KEY_RETURN;
	if (name == "UP") return Keyboard::KEY_UP;
	if (name == "DOWN") return Keyboard::KEY_DOWN;
	if (name == "LEFT") return Keyboard::KEY_LEFT;
	if (name == "RIGHT") return Keyboard::KEY_RIGHT;
	if (name == "TAB") return Keyboard::KEY_TAB;
	if (name == "L.CTRL") return Keyboard::KEY_LCONTROL;
	if (name == "R.CTRL") return Keyboard::KEY_RCONTROL;
	if (name == "L.ALT") return Keyboard::KEY_LALT;
	if (name == "R.ALT") return Keyboard::KEY_RALT;
	if (name == "L.MOUSE") return -100;
	if (name == "R.MOUSE") return -99;
	if (name == "M.MOUSE") return -98;
	return -1000; // Not found
}

// Reverse lookup: controller button display name -> bitmask (inverse of getControllerButtonName)
static unsigned int getControllerButtonByName(const std::string &name)
{
	if (name == "A") return _360_JOY_BUTTON_A;
	if (name == "B") return _360_JOY_BUTTON_B;
	if (name == "X") return _360_JOY_BUTTON_X;
	if (name == "Y") return _360_JOY_BUTTON_Y;
	if (name == "START") return _360_JOY_BUTTON_START;
	if (name == "BACK") return _360_JOY_BUTTON_BACK;
	if (name == "RB") return _360_JOY_BUTTON_RB;
	if (name == "LB") return _360_JOY_BUTTON_LB;
	if (name == "RT") return _360_JOY_BUTTON_RT;
	if (name == "LT") return _360_JOY_BUTTON_LT;
	if (name == "R.STICK") return _360_JOY_BUTTON_RTHUMB;
	if (name == "L.STICK") return _360_JOY_BUTTON_LTHUMB;
	if (name == "D-UP") return _360_JOY_BUTTON_DPAD_UP;
	if (name == "D-DOWN") return _360_JOY_BUTTON_DPAD_DOWN;
	if (name == "D-LEFT") return _360_JOY_BUTTON_DPAD_LEFT;
	if (name == "D-RIGHT") return _360_JOY_BUTTON_DPAD_RIGHT;
	return 0;
}

// Reverse lookup: controller action display name -> action index (inverse of getControllerActionName)
static int getControllerActionByName(const std::string &name)
{
	if (name == "Attack/Mine") return 0;
	if (name == "Use/Place") return 1;
	if (name == "Jump") return 2;
	if (name == "Sneak") return 3;
	if (name == "Drop") return 4;
	if (name == "Inventory") return 5;
	if (name == "Crafting") return 6;
	if (name == "Scroll Left") return 7;
	if (name == "Scroll Right") return 8;
	if (name == "Game Info") return 9;
	if (name == "Pause") return 10;
	if (name == "Third Person") return 11;
	if (name == "Sprint") return 12;
	return -1;
}

// Simple JSON value extraction for migration from options.json
static std::string jsonExtractValue(const std::string &json, const std::string &key)
{
	std::string search = "\"" + key + "\"";
	size_t pos = json.find(search);
	if (pos == std::string::npos) return "";
	pos += search.size();
	while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r'))
		pos++;
	if (pos >= json.size() || json[pos] != ':') return "";
	pos++;
	while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r'))
		pos++;
	if (pos >= json.size()) return "";

	if (json[pos] == '"')
	{
		pos++;
		size_t end = json.find('"', pos);
		if (end == std::string::npos) return "";
		return json.substr(pos, end - pos);
	}
	else if (json[pos] == '{')
	{
		int depth = 1;
		size_t start = pos + 1;
		pos++;
		while (pos < json.size() && depth > 0)
		{
			if (json[pos] == '{') depth++;
			else if (json[pos] == '}') depth--;
			pos++;
		}
		return json.substr(start, pos - 1 - start);
	}
	else
	{
		size_t end = pos;
		while (end < json.size() && json[end] != ',' && json[end] != '}' && json[end] != '\n' && json[end] != '\r')
			end++;
		std::string val = json.substr(pos, end - pos);
		while (!val.empty() && (val.back() == ' ' || val.back() == '\t'))
			val.pop_back();
		return val;
	}
}

static double jsonDouble(const std::string &val, double def)
{
	if (val.empty()) return def;
	return atof(val.c_str());
}

static int jsonInt(const std::string &val, int def)
{
	if (val.empty()) return def;
	return atoi(val.c_str());
}

static bool jsonBool(const std::string &val, bool def)
{
	if (val == "true") return true;
	if (val == "false") return false;
	return def;
}

void Options::load()
{
	std::string content;
	bool isJson = false;

	if (optionsFile.exists())
	{
		__int64 fileLen = optionsFile.length();
		if (fileLen <= 0 || fileLen > 65536)
		{
			app.DebugPrintf("FATAL: options.toml has invalid size (%lld bytes)\n", fileLen);
			exit(1);
		}
		FileInputStream fis(optionsFile);
		byteArray buf((int)fileLen);
		int bytesRead = fis.read(buf);
		fis.close();
		if (bytesRead <= 0)
		{
			delete[] buf.data;
			app.DebugPrintf("FATAL: Failed to read options.toml\n");
			exit(1);
		}
		content = std::string((const char *)buf.data, bytesRead);
		delete[] buf.data;
	}
	else
	{
		// JSON fallback: try options.json in the same directory
		wstring tomlPath = optionsFile.getPath();
		size_t dotPos = tomlPath.rfind(L'.');
		if (dotPos != wstring::npos)
		{
			wstring jsonPath = tomlPath.substr(0, dotPos) + L".json";
			File jsonFile(jsonPath);
			if (jsonFile.exists())
			{
				__int64 fileLen = jsonFile.length();
				if (fileLen <= 0 || fileLen > 65536)
				{
					app.DebugPrintf("FATAL: options.json has invalid size (%lld bytes)\n", fileLen);
					exit(1);
				}
				FileInputStream fis(jsonFile);
				byteArray buf((int)fileLen);
				int bytesRead = fis.read(buf);
				fis.close();
				if (bytesRead <= 0)
				{
					delete[] buf.data;
					app.DebugPrintf("FATAL: Failed to read options.json\n");
					exit(1);
				}
				content = std::string((const char *)buf.data, bytesRead);
				delete[] buf.data;
				isJson = true;
			}
		}

		if (!isJson)
		{
			// No options file found - create defaults from init() state
			app.DebugPrintf("No options file found, creating default options.toml\n");
			save();
			return;
		}
	}

	if (isJson)
	{
		// Parse JSON (migration from old format)
		std::string val;

		val = jsonExtractValue(content, "music");
		if (!val.empty()) music = (float)jsonDouble(val, music);
		val = jsonExtractValue(content, "sound");
		if (!val.empty()) sound = (float)jsonDouble(val, sound);
		val = jsonExtractValue(content, "mouseSensitivity");
		if (!val.empty()) sensitivity = (float)jsonDouble(val, sensitivity);
		val = jsonExtractValue(content, "fov");
		if (!val.empty()) fov = (float)jsonDouble(val, fov);
		val = jsonExtractValue(content, "gamma");
		if (!val.empty()) gamma = (float)jsonDouble(val, gamma);
		val = jsonExtractValue(content, "invertYMouse");
		if (!val.empty()) invertYMouse = jsonBool(val, invertYMouse);
		val = jsonExtractValue(content, "viewDistance");
		if (!val.empty()) viewDistance = jsonInt(val, viewDistance);
		val = jsonExtractValue(content, "guiScale");
		if (!val.empty()) guiScale = jsonInt(val, guiScale);
		val = jsonExtractValue(content, "particles");
		if (!val.empty()) particles = jsonInt(val, particles);
		val = jsonExtractValue(content, "bobView");
		if (!val.empty()) bobView = jsonBool(val, bobView);
		val = jsonExtractValue(content, "anaglyph3d");
		if (!val.empty()) anaglyph3d = jsonBool(val, anaglyph3d);
		val = jsonExtractValue(content, "advancedOpengl");
		if (!val.empty()) advancedOpengl = jsonBool(val, advancedOpengl);
		val = jsonExtractValue(content, "fpsLimit");
		if (!val.empty()) framerateLimit = jsonInt(val, framerateLimit);
		val = jsonExtractValue(content, "difficulty");
		if (!val.empty()) difficulty = jsonInt(val, difficulty);
		val = jsonExtractValue(content, "fancyGraphics");
		if (!val.empty()) fancyGraphics = jsonBool(val, fancyGraphics);
		val = jsonExtractValue(content, "ao");
		if (!val.empty()) ambientOcclusion = jsonBool(val, ambientOcclusion);
		val = jsonExtractValue(content, "clouds");
		if (!val.empty()) renderClouds = jsonBool(val, renderClouds);
		val = jsonExtractValue(content, "skin");
		if (!val.empty()) skin = widenStr(val);
		val = jsonExtractValue(content, "lastServer");
		if (!val.empty()) lastMpIp = widenStr(val);

		std::string kbObj = jsonExtractValue(content, "keyboard");
		if (!kbObj.empty())
		{
			for (int i = 0; i < keyMappings_length; i++)
			{
				val = jsonExtractValue(kbObj, narrowStr(keyMappings[i]->name));
				if (!val.empty())
					keyMappings[i]->key = jsonInt(val, keyMappings[i]->key);
			}
		}

		std::string ctObj = jsonExtractValue(content, "controller");
		if (!ctObj.empty())
		{
			for (int i = 0; i < CONTROLLER_ACTIONS; i++)
			{
				val = jsonExtractValue(ctObj, narrowStr(_toString<int>(i)));
				if (!val.empty())
					controllerMappings[i] = (unsigned int)jsonInt(val, (int)controllerMappings[i]);
			}
		}

		applyControllerMappings();
		save(); // Migrate to TOML format
		return;
	}

	// Parse TOML
	toml::result<toml::value, std::vector<toml::error_info>> result = toml::try_parse_str(content);
	if (!result)
	{
		app.DebugPrintf("FATAL: Failed to parse options.toml\n");
		exit(1);
	}

	toml::value tbl = result.unwrap();

	music = (float)toml::find_or<double>(tbl, "music", (double)music);
	sound = (float)toml::find_or<double>(tbl, "sound", (double)sound);
	sensitivity = (float)toml::find_or<double>(tbl, "mouseSensitivity", (double)sensitivity);
	fov = (float)toml::find_or<double>(tbl, "fov", (double)fov);
	gamma = (float)toml::find_or<double>(tbl, "gamma", (double)gamma);
	invertYMouse = toml::find_or<bool>(tbl, "invertYMouse", invertYMouse);
	viewDistance = (int)toml::find_or<std::int64_t>(tbl, "viewDistance", (std::int64_t)viewDistance);
	guiScale = (int)toml::find_or<std::int64_t>(tbl, "guiScale", (std::int64_t)guiScale);
	particles = (int)toml::find_or<std::int64_t>(tbl, "particles", (std::int64_t)particles);
	bobView = toml::find_or<bool>(tbl, "bobView", bobView);
	anaglyph3d = toml::find_or<bool>(tbl, "anaglyph3d", anaglyph3d);
	advancedOpengl = toml::find_or<bool>(tbl, "advancedOpengl", advancedOpengl);
	framerateLimit = (int)toml::find_or<std::int64_t>(tbl, "fpsLimit", (std::int64_t)framerateLimit);
	difficulty = (int)toml::find_or<std::int64_t>(tbl, "difficulty", (std::int64_t)difficulty);
	fancyGraphics = toml::find_or<bool>(tbl, "fancyGraphics", fancyGraphics);
	ambientOcclusion = toml::find_or<bool>(tbl, "ao", ambientOcclusion);
	renderClouds = toml::find_or<bool>(tbl, "clouds", renderClouds);
	std::string skinStr = toml::find_or<std::string>(tbl, "skin", narrowStr(skin));
	skin = widenStr(skinStr);
	std::string serverStr = toml::find_or<std::string>(tbl, "lastServer", narrowStr(lastMpIp));
	lastMpIp = widenStr(serverStr);

	if (tbl.contains("keyboard") && tbl.at("keyboard").is_table())
	{
		const toml::value &kb = tbl.at("keyboard");
		for (int i = 0; i < keyMappings_length; i++)
		{
			std::string keyName = narrowStr(keyMappings[i]->name);
			if (!kb.contains(keyName)) continue;
			const toml::value &entry = kb.at(keyName);
			// Try string value first (human-readable: "W", "SPACE", "L.MOUSE", etc.)
			if (entry.is_string())
			{
				int keyCode = getKeyByName(entry.as_string());
				if (keyCode != -1000)
					keyMappings[i]->key = keyCode;
			}
			// Fall back to integer value (old TOML format)
			else if (entry.is_integer())
			{
				keyMappings[i]->key = (int)entry.as_integer();
			}
		}
	}

	if (tbl.contains("controller") && tbl.at("controller").is_table())
	{
		const toml::value &ct = tbl.at("controller");
		// Try human-readable format first (action names as keys, button names as values)
		bool foundReadable = false;
		for (int i = 0; i < CONTROLLER_ACTIONS; i++)
		{
			std::string actionName = narrowStr(getControllerActionName(i));
			if (ct.contains(actionName) && ct.at(actionName).is_string())
			{
				controllerMappings[i] = getControllerButtonByName(ct.at(actionName).as_string());
				foundReadable = true;
			}
		}
		// Fall back to old numeric index format
		if (!foundReadable)
		{
			for (int i = 0; i < CONTROLLER_ACTIONS; i++)
			{
				std::string idx = narrowStr(_toString<int>(i));
				if (ct.contains(idx) && ct.at(idx).is_integer())
					controllerMappings[i] = (unsigned int)ct.at(idx).as_integer();
			}
		}
	}

	applyControllerMappings();
}

void Options::save()
{
	// Build keyboard sub-table
	toml::table keyboard_tbl;
	for (int i = 0; i < keyMappings_length; i++)
	{
		std::string keyName = narrowStr(keyMappings[i]->name);
		std::string keyValue = narrowStr(Keyboard::getKeyName(keyMappings[i]->key));
		keyboard_tbl[keyName] = keyValue;
	}

	// Build controller sub-table
	toml::table controller_tbl;
	for (int i = 0; i < CONTROLLER_ACTIONS; i++)
	{
		std::string actionName = narrowStr(getControllerActionName(i));
		std::string buttonName = narrowStr(getControllerButtonName(controllerMappings[i]));
		controller_tbl[actionName] = buttonName;
	}

	// Build TOML document
	toml::value tbl(toml::table{
		{"music", (double)music},
		{"sound", (double)sound},
		{"invertYMouse", invertYMouse},
		{"mouseSensitivity", (double)sensitivity},
		{"fov", (double)fov},
		{"gamma", (double)gamma},
		{"viewDistance", (std::int64_t)viewDistance},
		{"guiScale", (std::int64_t)guiScale},
		{"particles", (std::int64_t)particles},
		{"bobView", bobView},
		{"anaglyph3d", anaglyph3d},
		{"advancedOpengl", advancedOpengl},
		{"fpsLimit", (std::int64_t)framerateLimit},
		{"difficulty", (std::int64_t)difficulty},
		{"fancyGraphics", fancyGraphics},
		{"ao", ambientOcclusion},
		{"clouds", renderClouds},
		{"skin", narrowStr(skin)},
		{"lastServer", narrowStr(lastMpIp)},
		{"keyboard", toml::value(keyboard_tbl)},
		{"controller", toml::value(controller_tbl)}
	});

	// Serialize TOML to string
	std::string content = toml::format(tbl);

	// Delete existing file to avoid stale trailing data
	if (optionsFile.exists()) optionsFile._delete();

	FileOutputStream fos(optionsFile);
	writeFileBytes(fos, content);
	fos.close();

	// Also save options.json alongside options.toml
	wstring tomlPath = optionsFile.getPath();
	size_t dotPos = tomlPath.rfind(L'.');
	if (dotPos != wstring::npos)
	{
		wstring jsonPath = tomlPath.substr(0, dotPos) + L".json";
		File jsonFile(jsonPath);
		if (jsonFile.exists()) jsonFile._delete();

		std::string json = "{\n";
		json += "  \"music\": " + narrowStr(_toString<float>(music)) + ",\n";
		json += "  \"sound\": " + narrowStr(_toString<float>(sound)) + ",\n";
		json += "  \"invertYMouse\": " + std::string(invertYMouse ? "true" : "false") + ",\n";
		json += "  \"mouseSensitivity\": " + narrowStr(_toString<float>(sensitivity)) + ",\n";
		json += "  \"fov\": " + narrowStr(_toString<float>(fov)) + ",\n";
		json += "  \"gamma\": " + narrowStr(_toString<float>(gamma)) + ",\n";
		json += "  \"viewDistance\": " + narrowStr(_toString<int>(viewDistance)) + ",\n";
		json += "  \"guiScale\": " + narrowStr(_toString<int>(guiScale)) + ",\n";
		json += "  \"particles\": " + narrowStr(_toString<int>(particles)) + ",\n";
		json += "  \"bobView\": " + std::string(bobView ? "true" : "false") + ",\n";
		json += "  \"anaglyph3d\": " + std::string(anaglyph3d ? "true" : "false") + ",\n";
		json += "  \"advancedOpengl\": " + std::string(advancedOpengl ? "true" : "false") + ",\n";
		json += "  \"fpsLimit\": " + narrowStr(_toString<int>(framerateLimit)) + ",\n";
		json += "  \"difficulty\": " + narrowStr(_toString<int>(difficulty)) + ",\n";
		json += "  \"fancyGraphics\": " + std::string(fancyGraphics ? "true" : "false") + ",\n";
		json += "  \"ao\": " + std::string(ambientOcclusion ? "true" : "false") + ",\n";
		json += "  \"clouds\": " + std::string(renderClouds ? "true" : "false") + ",\n";
		json += "  \"skin\": \"" + narrowStr(skin) + "\",\n";
		json += "  \"lastServer\": \"" + narrowStr(lastMpIp) + "\",\n";

		json += "  \"keyboard\": {\n";
		for (int i = 0; i < keyMappings_length; i++)
		{
			std::string keyName = narrowStr(keyMappings[i]->name);
			std::string keyValue = narrowStr(Keyboard::getKeyName(keyMappings[i]->key));
			json += "    \"" + keyName + "\": \"" + keyValue + "\"";
			json += (i < keyMappings_length - 1) ? ",\n" : "\n";
		}
		json += "  },\n";

		json += "  \"controller\": {\n";
		for (int i = 0; i < CONTROLLER_ACTIONS; i++)
		{
			std::string actionName = narrowStr(getControllerActionName(i));
			std::string buttonName = narrowStr(getControllerButtonName(controllerMappings[i]));
			json += "    \"" + actionName + "\": \"" + buttonName + "\"";
			json += (i < CONTROLLER_ACTIONS - 1) ? ",\n" : "\n";
		}
		json += "  }\n";

		json += "}\n";

		FileOutputStream jsonFos(jsonFile);
		writeFileBytes(jsonFos, json);
		jsonFos.close();
	}
}

bool Options::isCloudsOn()
{
	return viewDistance < 2 && renderClouds;
}

wstring Options::getControllerActionName(int action)
{
	switch (action)
	{
	case 0:  return L"Attack/Mine";
	case 1:  return L"Use/Place";
	case 2:  return L"Jump";
	case 3:  return L"Sneak";
	case 4:  return L"Drop";
	case 5:  return L"Inventory";
	case 6:  return L"Crafting";
	case 7:  return L"Scroll Left";
	case 8:  return L"Scroll Right";
	case 9:  return L"Game Info";
	case 10: return L"Pause";
	case 11: return L"Third Person";
	case 12: return L"Sprint";
	default: return L"Unknown";
	}
}

wstring Options::getControllerButtonName(unsigned int button)
{
	if (button & _360_JOY_BUTTON_A) return L"A";
	if (button & _360_JOY_BUTTON_B) return L"B";
	if (button & _360_JOY_BUTTON_X) return L"X";
	if (button & _360_JOY_BUTTON_Y) return L"Y";
	if (button & _360_JOY_BUTTON_START) return L"START";
	if (button & _360_JOY_BUTTON_BACK) return L"BACK";
	if (button & _360_JOY_BUTTON_RB) return L"RB";
	if (button & _360_JOY_BUTTON_LB) return L"LB";
	if (button & _360_JOY_BUTTON_RT) return L"RT";
	if (button & _360_JOY_BUTTON_LT) return L"LT";
	if (button & _360_JOY_BUTTON_RTHUMB) return L"R.STICK";
	if (button & _360_JOY_BUTTON_LTHUMB) return L"L.STICK";
	if (button & _360_JOY_BUTTON_DPAD_UP) return L"D-UP";
	if (button & _360_JOY_BUTTON_DPAD_DOWN) return L"D-DOWN";
	if (button & _360_JOY_BUTTON_DPAD_LEFT) return L"D-LEFT";
	if (button & _360_JOY_BUTTON_DPAD_RIGHT) return L"D-RIGHT";
	return L"NONE";
}

void Options::setControllerMapping(int action, unsigned int button)
{
	if (action >= 0 && action < CONTROLLER_ACTIONS)
	{
		controllerMappings[action] = button;
		applyControllerMappings();
		save();
	}
}

void Options::resetControllerDefaults()
{
	for (int i = 0; i < CONTROLLER_ACTIONS; i++)
		controllerMappings[i] = controllerDefaults[i];
	applyControllerMappings();
	save();
}

void Options::resetKeyboardDefaults()
{
	for (int i = 0; i < keyMappings_length; i++)
		keyMappings[i]->resetToDefault();
	save();
}

void Options::applyControllerMappings()
{
	// Map controller action indices to MINECRAFT_ACTION_* enums
	static const unsigned char actionMap[] = {
		MINECRAFT_ACTION_ACTION,                // 0: Attack/Mine
		MINECRAFT_ACTION_USE,                   // 1: Use/Place
		MINECRAFT_ACTION_JUMP,                  // 2: Jump
		MINECRAFT_ACTION_SNEAK_TOGGLE,          // 3: Sneak
		MINECRAFT_ACTION_DROP,                  // 4: Drop
		MINECRAFT_ACTION_INVENTORY,             // 5: Inventory
		MINECRAFT_ACTION_CRAFTING,              // 6: Crafting
		MINECRAFT_ACTION_LEFT_SCROLL,           // 7: Scroll Left
		MINECRAFT_ACTION_RIGHT_SCROLL,          // 8: Scroll Right
		MINECRAFT_ACTION_GAME_INFO,             // 9: Game Info
		MINECRAFT_ACTION_PAUSEMENU,             // 10: Pause
		MINECRAFT_ACTION_RENDER_THIRD_PERSON,   // 11: Third Person
	};

	int count = CONTROLLER_ACTIONS;
	if (count > 12) count = 12; // Only apply the 12 actions with known MINECRAFT_ACTION_* enums

	for (int i = 0; i < count; i++)
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_0, actionMap[i], controllerMappings[i]);
	}
}
