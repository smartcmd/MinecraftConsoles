#pragma once
#include "Screen.h"
#include <vector>
using namespace std;

class ChatScreen : public Screen
{
protected:
	wstring message;
	int cursorIndex;
	void applyHistoryMessage();

private:
	int frame;
	static const size_t CHAT_HISTORY_MAX = 100;
	static std::vector<wstring> s_chatHistory;
	static int s_historyIndex;
	static wstring s_historyDraft;
	static const wstring allowedChars;
	static bool isAllowedChatChar(wchar_t c);

public:
	ChatScreen();
	virtual void init();
	virtual void removed();
	virtual void tick();
	virtual void handlePasteRequest();
	virtual void handleHistoryUp();
	virtual void handleHistoryDown();

protected:
	void keyPressed(wchar_t ch, int eventKey);
public:
	void render(int xm, int ym, float a);
protected:
	void mouseClicked(int x, int y, int buttonNum);
};