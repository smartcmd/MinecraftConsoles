#pragma once
using namespace std;

#include "../Minecraft.World/IconRegister.h"

class StitchedTexture;
class Texture;
class BufferedImage;

class TextureMap : public IconRegister
{
public:
	static const wstring NAME_MISSING_TEXTURE;

private:
	const int iconType;

	const wstring name;
	const wstring path;
	const wstring extension;

	bool m_mipMap;

	typedef unordered_map<wstring, StitchedTexture *> stringStitchedTextureMap;
	stringStitchedTextureMap texturesByName; //  = new HashMap<String, StitchedTexture>();
	BufferedImage *missingTexture; // = new BufferedImage(64, 64, BufferedImage.TYPE_INT_ARGB);
	StitchedTexture *missingPosition;
	Texture *stitchResult;
	vector<StitchedTexture *> animatedTextures; // = new ArrayList<StitchedTexture>();

	stringStitchedTextureMap texturesToRegister; // = new HashMap<String, StitchedTexture>();

public:
	TextureMap(int type, const wstring &name, const wstring &path, BufferedImage *missingTexture, bool mipMap = false);

	void stitch();
	StitchedTexture *getTexture(const wstring &name);
	void cycleAnimationFrames();
	Texture *getStitchedTexture();

	// 4J Stu - register is a reserved keyword in C++
	Icon *registerIcon(const wstring &name);

	int getIconType();
	Icon *getMissingIcon();
};