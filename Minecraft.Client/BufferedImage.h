#pragma once
using namespace std;

class Graphics;
class DLCPack;

class BufferedImage
{
private:
	int *data[10];	// Arrays for mipmaps - nullptr if not used
	int width;
	int height;
	void ByteFlip4(unsigned int &data);	// 4J added
public:
	static const int TYPE_INT_ARGB = 0;
	static const int TYPE_INT_RGB = 1;
	BufferedImage(int width,int height,int type);
	BufferedImage(const wstring& File, bool filenameHasExtension = false, bool bTitleUpdateTexture=false, const wstring &drive =L"");	// 4J added
	BufferedImage(DLCPack *dlcPack, const wstring& File, bool filenameHasExtension = false ); // 4J Added
	BufferedImage(BYTE *pbData, DWORD dwBytes);	// 4J added
	~BufferedImage();

	int getWidth();	
	int getHeight();
	void getRGB(int startX, int startY, int w, int h, intArray out,int offset,int scansize, int level = 0); // 4J Added level param
	int *getData();	// 4J added
	int *getData(int level);	// 4J added
	Graphics *getGraphics();
	int getTransparency();
	BufferedImage *getSubimage(int x, int y, int w, int h);

	void preMultiplyAlpha();
};