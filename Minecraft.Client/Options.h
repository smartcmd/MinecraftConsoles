#pragma once
using namespace std;
class Minecraft;
class KeyMapping;
#include "..\Minecraft.World\File.h"

class Options
{
public:
	static const int AO_OFF = 0;
	static const int AO_MIN = 1;
	static const int AO_MAX = 2;

	// 4J - this used to be an enum
    class Option
	{
	public:
		static const Option options[17];
		static const Option *MUSIC;
		static const Option *SOUND;
		static const Option *INVERT_MOUSE;
		static const Option *SENSITIVITY;
		static const Option *RENDER_DISTANCE;
		static const Option *VIEW_BOBBING;
		static const Option *ANAGLYPH;
		static const Option *ADVANCED_OPENGL;
		static const Option *FRAMERATE_LIMIT;
		static const Option *DIFFICULTY;
		static const Option *GRAPHICS;
		static const Option *AMBIENT_OCCLUSION;
		static const Option *GUI_SCALE;
		static const Option *FOV;
		static const Option *GAMMA;
		static const Option *RENDER_CLOUDS;
		static const Option *PARTICLES;

	private:
		const bool _isProgress;
        const bool _isBoolean;
        const wstring captionId;

	public:
		static const Option *getItem(int id);

        Option(const wstring& captionId, bool hasProgress, bool isBoolean);
        bool	isProgress() const;
		bool	isBoolean() const;
		int		getId() const;
		wstring getCaptionId() const;
    };

private:
	static const wstring RENDER_DISTANCE_NAMES[];
    static const wstring DIFFICULTY_NAMES[];
    static const wstring GUI_SCALE[];
    static const wstring FRAMERATE_LIMITS[];
	static const wstring PARTICLES[];

public:
	float music;
    float sound;
    float sensitivity;
    bool invertYMouse;
    int viewDistance;
    bool bobView;
    bool anaglyph3d;
    bool advancedOpengl;
    int framerateLimit;
    bool fancyGraphics;
    bool ambientOcclusion;
	bool renderClouds;
    wstring skin;

    KeyMapping *keyUp;
    KeyMapping *keyLeft;
    KeyMapping *keyDown;
    KeyMapping *keyRight;
    KeyMapping *keyJump;
    KeyMapping *keyBuild;
    KeyMapping *keyDrop;
    KeyMapping *keyChat;
    KeyMapping *keySneak;
	KeyMapping *keyAttack;
    KeyMapping *keyUse;
    KeyMapping *keyPlayerList;
    KeyMapping *keyPickItem;
    KeyMapping *keyToggleFog;

	static const int keyMappings_length = 14;
    KeyMapping *keyMappings[keyMappings_length];

protected:
	Minecraft *minecraft;
private:
	File optionsFile;

public:
	int difficulty;
    bool hideGui;
    bool thirdPersonView;
    bool renderDebug;
    wstring lastMpIp;

    bool isFlying;
    bool smoothCamera;
    bool fixedCamera;
    float flySpeed;
    float cameraSpeed;
    int guiScale;
	int particles; // 0 is all, 1 is decreased and 2 is minimal
	float fov;
	float gamma;

	void init();	// 4J added
    Options(Minecraft *minecraft, File workingDirectory);
    Options();
    wstring getKeyDescription(int i);
    wstring getKeyMessage(int i);
    void setKey(int i, int key);
    void set(const Options::Option *item, float value);
    void toggle(const Options::Option *option, int dir);
    float getProgressValue(const Options::Option *item);
    bool getBooleanValue(const Options::Option *item);
    wstring getMessage(const Options::Option *item);
    void load();
private:
	float readFloat(wstring string);
public:
	void save();

	bool isCloudsOn();
};
