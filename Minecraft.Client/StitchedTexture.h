#pragma once
using namespace std;

#include "../Minecraft.World/Icon.h"
class Texture;

class StitchedTexture : public Icon
{
private:
	const wstring name;

public:
	wstring m_fileName;

protected:
	Texture *source;
	vector<Texture *> *frames;

private:
	typedef vector<pair<int, int> > intPairVector;
	intPairVector *frameOverride;
	int flags;

protected:
	bool rotated;

	int x;
	int y;

protected:
	int width;
	int height;

	float u0;
	float u1;
	float v0;
	float v1;

	float widthTranslation;
	float heightTranslation;

protected:
	int frame;
	int subFrame;

public:
	static StitchedTexture *create(const wstring &name);
	
	~StitchedTexture();
protected:
	StitchedTexture(const wstring &name, const wstring &filename);

public:
	void initUVs(float U0, float V0, float U1, float V1);
	void init(Texture *source, vector<Texture *> *frames, int x, int y, int width, int height, bool rotated);
	void replaceWith(StitchedTexture *texture);
	int getX() const;
	int getY() const;
	int getWidth() const;
	int getHeight() const;
	float getU0(bool adjust = false) const;
	float getU1(bool adjust = false) const;
	float getU(double offset, bool adjust = false) const;
	float getV0(bool adjust = false) const;
	float getV1(bool adjust = false) const;
	float getV(double offset, bool adjust = false) const;
	wstring getName() const;
	virtual int getSourceWidth() const;
	virtual int getSourceHeight() const;
	virtual void cycleFrames();
	Texture *getSource();
	Texture *getFrame(int i);
	virtual int getFrames();

	/**
	* Loads animation frames from a file with the syntax, <code>
	* 0,1,2,3,
	* 4*10,5*10,
	* 4*10,3,2,1,
	* 0
	* </code> or similar
	* 
	* @param bufferedReader
	*/
	void loadAnimationFrames(BufferedReader *bufferedReader);
	void loadAnimationFrames(const wstring &string); // 4J Added

	int getFlags() const ;			// 4J added
	void setFlags(int flags);		// 4J added
	virtual void freeFrameTextures();		// 4J added
	virtual bool hasOwnData();		// 4J Added
};