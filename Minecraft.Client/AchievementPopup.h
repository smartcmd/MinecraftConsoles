#pragma once
#include "GuiComponent.h"
class Achievement;
class ItemRenderer;
using namespace std;

class AchievementPopup : public GuiComponent
{
private:
	Minecraft *mc;
    int width, height;

    wstring title;
    wstring desc;
    Achievement *ach;
    int64_t startTime;
    ItemRenderer *ir;
    bool isHelper;

public:
	AchievementPopup(Minecraft *mc);
    void popup(Achievement *ach);
    void permanent(Achievement *ach);
private:
	void prepareWindow();
public:
    void render();
};