#pragma once
#include "ResourceLocation.h"
#include "GuiComponent.h"
#include "GuiMessage.h"
#include "ResourceLocation.h"

class Random;
class Minecraft;
class ItemRenderer;

class Gui : public GuiComponent
{
private:
	static ResourceLocation PUMPKIN_BLUR_LOCATION;
	// 4J-PB - this doesn't account for the safe zone, and the indent applied to messages
	//static const int MAX_MESSAGE_WIDTH = 320;
	static const int m_iMaxMessageWidth = 280;
	static ItemRenderer *itemRenderer;
	vector<GuiMessage> guiMessages[XUSER_MAX_COUNT];
	Random *random;

	Minecraft *minecraft;
public:
	wstring selectedName;
private:
	int tickCount;
	wstring overlayMessageString;
	int overlayMessageTime;
	bool animateOverlayMessageColor;

	// 4J Added
	float lastTickA;
	float fAlphaIncrementPerCent;
public:
	static float currentGuiBlendFactor;		// 4J added
	static float currentGuiScaleFactor;		// 4J added

	float progress;

	//    private DecimalFormat df = new DecimalFormat("##.00");

public:
	Gui(Minecraft *minecraft);

	void render(float a, bool mouseFree, int xMouse, int yMouse);
	float tbr;

private:
	//void renderBossHealth(void);
	void renderPumpkin(int w, int h);
	void renderVignette(float br, int w, int h);
	void renderTp(float br, int w, int h);
	void renderSlot(int slot, int x, int y, float a);
public:
	void tick();
	void clearMessages(int iPad=-1);
	void addMessage(const wstring& string, int iPad,bool bIsDeathMessage=false);
	void setNowPlaying(const wstring& string);
	void displayClientMessage(int messageId, int iPad);

	// 4J Added
	DWORD getMessagesCount(int iPad) { return static_cast<int>(guiMessages[iPad].size()); }
	wstring getMessage(int iPad, DWORD index) { return guiMessages[iPad].at(index).string; }
	float getOpacity(int iPad, DWORD index);

	wstring getJukeboxMessage(int iPad) { return overlayMessageString; }
	float getJukeboxOpacity(int iPad);

	// 4J Added
	void renderGraph(int dataLength, int dataPos, int64_t *dataA, float dataAScale, int dataAWarning, int64_t *dataB, float dataBScale, int dataBWarning);
	void renderStackedGraph(int dataPos, int dataLength, int dataSources, int64_t (*func)(unsigned int dataPos, unsigned int dataSource) );
};
