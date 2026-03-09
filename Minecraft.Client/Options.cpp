#include "stdafx.h"
#include "Options.h"
#include "KeyMapping.h"
#include "LevelRenderer.h"
#include "Textures.h"
#include "..\Minecraft.World\net.minecraft.locale.h"
#include "..\Minecraft.World\Language.h"
#include "..\Minecraft.World\File.h"
#include "..\Minecraft.World\BufferedReader.h"
#include "..\Minecraft.World\DataInputStream.h"
#include "..\Minecraft.World\InputStreamReader.h"
#include "..\Minecraft.World\FileInputStream.h"
#include "..\Minecraft.World\FileOutputStream.h"
#include "..\Minecraft.World\DataOutputStream.h"
#include "..\Minecraft.World\StringHelpers.h"

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
	return static_cast<int>(this - options);
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

	minecraft = nullptr;
	//optionsFile = nullptr;

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
	optionsFile = File(workingDirectory, L"options.txt");
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
	if (item == Option::RENDER_DISTANCE)
	{
		viewDistance = fVal;
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
	if (item == Option::RENDER_DISTANCE) return viewDistance;
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
			return caption + std::to_wstring(static_cast<int>(progressValue * 200)) + L"%";
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
			return caption + std::to_wstring(static_cast<int>(70.0f + progressValue * 40.0f));
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
			return caption + L"+" + std::to_wstring( static_cast<int>(progressValue * 100.0f)) + L"%";
        }
		else
		{
            if (progressValue == 0)
			{
                return caption + language->getElement(L"options.off");
            }
            return caption + std::to_wstring(static_cast<int>(progressValue * 100.0f)) + L"%";
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

void Options::load()
{
	// 4J - removed try/catch
//    try {
        if (!optionsFile.exists()) return;
		// 4J - was new BufferedReader(new FileReader(optionsFile));
        BufferedReader *br = new BufferedReader(new InputStreamReader( new FileInputStream( optionsFile ) ) );

        wstring line = L"";
        while ((line = br->readLine()) != L"")	// 4J - was check against nullptr - do we need to distinguish between empty lines and a fail here?
		{
			// 4J - removed try/catch
//            try {
                wstring cmds[2];
				size_t splitpos = line.find(L":");
				if( splitpos == wstring::npos )
				{
					cmds[0] = line;
					cmds[1] = L"";
				}
				else
				{
					cmds[0] = line.substr(0,splitpos);
					cmds[1] = line.substr(splitpos,line.length()-splitpos);
				}

                if (cmds[0] == L"music") music = readFloat(cmds[1]);
                if (cmds[0] == L"sound") sound = readFloat(cmds[1]);
                if (cmds[0] == L"mouseSensitivity") sensitivity = readFloat(cmds[1]);
				if (cmds[0] == L"fov") fov = readFloat(cmds[1]);
				if (cmds[0] == L"gamma") gamma = readFloat(cmds[1]);
                if (cmds[0] == L"invertYMouse") invertYMouse = cmds[1]==L"true";
                if (cmds[0] == L"viewDistance") viewDistance = _fromString<int>(cmds[1]);
                if (cmds[0] == L"guiScale") guiScale =_fromString<int>(cmds[1]);
				if (cmds[0] == L"particles") particles = _fromString<int>(cmds[1]);
                if (cmds[0] == L"bobView") bobView = cmds[1]==L"true";
                if (cmds[0] == L"anaglyph3d") anaglyph3d = cmds[1]==L"true";
                if (cmds[0] == L"advancedOpengl") advancedOpengl = cmds[1]==L"true";
                if (cmds[0] == L"fpsLimit") framerateLimit = _fromString<int>(cmds[1]);
                if (cmds[0] == L"difficulty") difficulty = _fromString<int>(cmds[1]);
                if (cmds[0] == L"fancyGraphics") fancyGraphics = cmds[1]==L"true";
                if (cmds[0] == L"ao") ambientOcclusion = cmds[1]==L"true";
				if (cmds[0] == L"clouds") renderClouds = cmds[1]==L"true";
                if (cmds[0] == L"skin") skin = cmds[1];
                if (cmds[0] == L"lastServer") lastMpIp = cmds[1];

                for (int i = 0; i < keyMappings_length; i++)
				{
                    if (cmds[0] == (L"key_" + keyMappings[i]->name))
					{
                        keyMappings[i]->key = _fromString<int>(cmds[1]);
                    }
                }
//            } catch (Exception e) {
//                System.out.println("Skipping bad option: " + line);
//            }
        }
		//KeyMapping.resetMapping(); // 4J Not implemented
        br->close();
//    } catch (Exception e) {
//        System.out.println("Failed to load options");
//        e.printStackTrace();
//    }

}

float Options::readFloat(wstring string)
{
    if (string == L"true") return 1;
    if (string == L"false") return 0;
    return _fromString<float>(string);
}

void Options::save()
{
	// 4J - try/catch removed
//    try {

	// 4J - original used a PrintWriter & FileWriter, but seems a bit much implementing these just to do this
		FileOutputStream fos = FileOutputStream(optionsFile);
		DataOutputStream dos = DataOutputStream(&fos);
//        PrintWriter pw = new PrintWriter(new FileWriter(optionsFile));

		dos.writeChars(L"music:" + std::to_wstring(music) + L"\n");
        dos.writeChars(L"sound:" + std::to_wstring(sound) + L"\n");
        dos.writeChars(L"invertYMouse:" + wstring(invertYMouse ? L"true" : L"false") + L"\n");
        dos.writeChars(L"mouseSensitivity:" + std::to_wstring(sensitivity));
		dos.writeChars(L"fov:" + std::to_wstring(fov));
		dos.writeChars(L"gamma:" + std::to_wstring(gamma));
        dos.writeChars(L"viewDistance:" + std::to_wstring(viewDistance));
        dos.writeChars(L"guiScale:" + std::to_wstring(guiScale));
		dos.writeChars(L"particles:" + std::to_wstring(particles));
        dos.writeChars(L"bobView:" + wstring(bobView ? L"true" : L"false"));
        dos.writeChars(L"anaglyph3d:" + wstring(anaglyph3d ? L"true" : L"false"));
        dos.writeChars(L"advancedOpengl:" + wstring(advancedOpengl ? L"true" : L"false"));
        dos.writeChars(L"fpsLimit:" + std::to_wstring(framerateLimit));
        dos.writeChars(L"difficulty:" + std::to_wstring(difficulty));
        dos.writeChars(L"fancyGraphics:" + wstring(fancyGraphics ? L"true" : L"false"));
        dos.writeChars(ambientOcclusion ? L"ao:true" : L"ao:false");
		dos.writeChars(renderClouds ? L"clouds:true" : L"clouds:false");
        dos.writeChars(L"skin:" + skin);
        dos.writeChars(L"lastServer:" + lastMpIp);

        for (int i = 0; i < keyMappings_length; i++)
		{
            dos.writeChars(L"key_" + keyMappings[i]->name + L":" + std::to_wstring(keyMappings[i]->key));
        }

        dos.close();
//    } catch (Exception e) {
//        System.out.println("Failed to save options");
//        e.printStackTrace();
//    }

}

bool Options::isCloudsOn()
{
	return viewDistance < 2 && renderClouds;
}
