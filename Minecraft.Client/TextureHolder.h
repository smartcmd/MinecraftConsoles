#pragma once

class Texture;

class TextureHolder //implements Comparable<TextureHolder> {
{
private:
	Texture *texture;
	int width;
	int height;
	bool rotated;
	float scale;

public:
	TextureHolder(Texture *texture);

	Texture *getTexture();
	int getWidth() const;
	int getHeight() const;
	void rotate();
	bool isRotated();

private:
	int smallestFittingMinTexel(int input) const;

public:
	void setForcedScale(int targetSize);
	//@Override
	wstring toString();
	int compareTo(const TextureHolder *other) const;
};

struct TextureHolderLessThan
{
	inline bool operator()(const TextureHolder *first, const TextureHolder *second) const 
	{ return first->compareTo(second) >= 0; }
};