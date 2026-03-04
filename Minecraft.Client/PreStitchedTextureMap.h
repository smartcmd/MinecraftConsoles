#pragma once
using namespace std;

#include "..\Minecraft.World\IconRegister.h"

class Icon;
class StitchedTexture;
class Texture;
class BufferedImage;

// 4J Added this class to stop having to do texture stitching at runtime
class PreStitchedTextureMap : public IconRegister
{
public:
	static const wstring NAME_MISSING_TEXTURE;

private:
	const int iconType;

	const wstring name;
	const wstring path;
	const wstring extension;

	bool m_mipMap;

	typedef unordered_map<wstring, Icon *> stringIconMap;
	stringIconMap texturesByName; //  = new HashMap<String, StitchedTexture>();
	BufferedImage *missingTexture; // = new BufferedImage(64, 64, BufferedImage.TYPE_INT_ARGB);
	StitchedTexture *missingPosition;
	Texture *stitchResult;
	vector<StitchedTexture *> animatedTextures; // = new ArrayList<StitchedTexture>();

	void loadUVs();
public:
	PreStitchedTextureMap(int type, const wstring &name, const wstring &path, BufferedImage *missingTexture, bool mipMap = false);

	void stitch();

private:
	void makeTextureAnimated(TexturePack *texturePack, StitchedTexture *tex);

public:
	StitchedTexture *getTexture(const wstring &name);
	void cycleAnimationFrames();
	Texture *getStitchedTexture();

	// 4J Stu - register is a reserved keyword in C++
	Icon *registerIcon(const wstring &name);

	int getIconType();
	Icon *getMissingIcon();

	int getFlags() const;
};