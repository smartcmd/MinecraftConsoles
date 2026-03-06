#pragma once
class Button;
class Tesselator;

class ScrolledSelectionList
{
private:
	static const int NO_DRAG = -1;
    static const int DRAG_OUTSIDE = -2;

    Minecraft *minecraft;
    int width;
    int height;
protected:
	int y0;
    int y1;
private:
	int x1;
    int x0;
protected:
	int itemHeight;
private:
	int upId;
    int downId;

    float yDrag;
    float yDragScale;
    float yo;

    int lastSelection;
    int64_t lastSelectionTime ;

    bool renderSelection;
    bool _renderHeader;
    int headerHeight;

public:
	ScrolledSelectionList(Minecraft *minecraft, int width, int height, int y0, int y1, int itemHeight);
    void setRenderSelection(bool renderSelection);

protected:
	void setRenderHeader(bool renderHeader, int headerHeight);
    virtual int getNumberOfItems() = 0;
    virtual void selectItem(int item, bool doubleClick) = 0;
    virtual bool isSelectedItem(int item) = 0;
    virtual int getMaxPosition();
    virtual void renderBackground() = 0;
    virtual void renderItem(int i, int x, int y, int h, Tesselator *t) = 0;
    void renderHeader(int x, int y, Tesselator *t);
    void clickedHeader(int headerMouseX, int headerMouseY);
    void renderDecorations(int mouseX, int mouseY);
public:
	int getItemAtPosition(int x, int y);
    void init(vector<Button *> *buttons, int upButtonId, int downButtonId);
private:
	void capYPosition();
public:
	void buttonClicked(Button *button);
    void render(int xm, int ym, float a);
private:
	void renderHoleBackground(int y0, int y1, int a0, int a1);
};