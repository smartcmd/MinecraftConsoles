#pragma once

class IntBuffer;
class Options;
class Textures;
class ResourceLocation;

class Font
{
private:
	int *charWidths;
public:
	int fontTexture;
	Random *random;

private:
	int colors[32]; // RGB colors for formatting

	Textures *textures;

	float xPos;
	float yPos;

	// § format state (bold, italic, underline, strikethrough); set during draw(), reset by §r
	bool m_bold;
	bool m_italic;
	bool m_underline;
	bool m_strikethrough;

	bool enforceUnicodeSheet; // use unicode sheet for ascii
	bool bidirectional; // use bidi to flip strings

	int m_cols; // Number of columns in font sheet
	int m_rows; // Number of rows in font sheet
	int m_charWidth; // Maximum character width
	int m_charHeight; // Maximum character height
	ResourceLocation *m_textureLocation; // Texture
	std::map<int, int> m_charMap;

public:
    Font(Options *options, const wstring& name, Textures* textures, bool enforceUnicode, ResourceLocation *textureLocation, int cols, int rows, int charWidth, int charHeight, unsigned short charMap[] = NULL);
#ifndef _XBOX
	// 4J Stu - This dtor clashes with one in xui! We never delete these anyway so take it out for now. Can go back when we have got rid of XUI
	~Font();
#endif
	void renderFakeCB(IntBuffer *cb);	// 4J added

private:
	void renderCharacter(wchar_t c); // 4J added
	void addCharacterQuad(wchar_t c);
	void renderStyleLine(float x0, float y0, float x1, float y1); // solid line for underline/strikethrough

public:
    void drawShadow(const wstring& str, int x, int y, int color);
	void drawShadowLiteral(const wstring& str, int x, int y, int color); // draw without interpreting § codes
	void drawShadowWordWrap(const wstring &str, int x, int y, int w, int color, int h); // 4J Added h param
    void draw(const wstring &str, int x, int y, int color);
	/**
	* Reorders the string according to bidirectional levels. A bit expensive at
	* the moment.
	* 
	* @param str
	* @return
	*/
private:
	wstring reorderBidi(const wstring &str);

	void draw(const wstring &str, bool dropShadow);
    void draw(const wstring& str, int x, int y, int color, bool dropShadow);
	void drawLiteral(const wstring& str, int x, int y, int color); // no § parsing
	int MapCharacter(wchar_t c); // 4J added
	bool CharacterExists(wchar_t c); // 4J added

public:
    int width(const wstring& str);
	int widthLiteral(const wstring& str); // width without skipping § codes (for chat input)
    wstring sanitize(const wstring& str);
	void drawWordWrap(const wstring &string, int x, int y, int w, int col, int h); // 4J Added h param

private:
	void drawWordWrapInternal(const wstring &string, int x, int y, int w, int col, int h); // 4J Added h param

public:
	void drawWordWrap(const wstring &string, int x, int y, int w, int col, bool darken, int h); // 4J Added h param

private:
    void drawWordWrapInternal(const wstring& string, int x, int y, int w, int col, bool darken, int h); // 4J Added h param

public:
    int wordWrapHeight(const wstring& string, int w);
	void setEnforceUnicodeSheet(bool enforceUnicodeSheet);
	void setBidirectional(bool bidirectional);

	// 4J-PB - check for invalid player name - Japanese local name
	bool AllCharactersValid(const wstring &str);
};
