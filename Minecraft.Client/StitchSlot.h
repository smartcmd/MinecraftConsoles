#pragma once
using namespace std;

class TextureHolder;

class StitchSlot
{
private:
	const int originX;
	const int originY;

	const int width;
	const int height;
	vector<StitchSlot *> *subSlots;
	TextureHolder *textureHolder;

public:
	StitchSlot(int originX, int originY, int width, int height);

	TextureHolder *getHolder();
	int getX();
	int getY();
	bool add(TextureHolder *textureHolder);
	void collectAssignments(vector<StitchSlot *> *result);
};