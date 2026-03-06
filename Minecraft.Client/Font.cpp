#include "stdafx.h"
#include "Textures.h"
#include "Font.h"
#include "Options.h"
#include "Tesselator.h"
#include "ResourceLocation.h"
#include "..\Minecraft.World\IntBuffer.h"
#include "..\Minecraft.World\net.minecraft.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "..\Minecraft.World\Random.h"

Font::Font(Options *options, const wstring& name, Textures* textures, bool enforceUnicode, ResourceLocation *textureLocation, int cols, int rows, int charWidth, int charHeight, unsigned short charMap[]/* = nullptr */) : textures(textures)
{
	int charC = cols * rows; // Number of characters in the font

	charWidths = new int[charC];

	// 4J - added initialisers
	memset(charWidths, 0, charC);

	enforceUnicodeSheet = false;
	bidirectional = false;
	xPos = yPos = 0.0f;
	m_bold = false;
	m_italic = false;
	m_underline = false;
	m_strikethrough = false;

	// Set up member variables
	m_cols = cols;
	m_rows = rows;
	m_charWidth = charWidth;
	m_charHeight = charHeight;
	m_textureLocation = textureLocation;

	// Build character map
	if (charMap != NULL)
	{
		for(int i = 0; i < charC; i++)
		{
			m_charMap.insert(std::make_pair(charMap[i], i));
		}
	}

	random = new Random();

	// Load the image
    BufferedImage *img = textures->readImage(textureLocation->getTexture(), name);

	/* - 4J - TODO
	try {
        img = ImageIO.read(Textures.class.getResourceAsStream(name));
    } catch (IOException e) {
        throw new RuntimeException(e);
    }
	*/

    int w = img->getWidth();
    int h = img->getHeight();
    intArray rawPixels(w * h);
    img->getRGB(0, 0, w, h, rawPixels, 0, w);

    for (int i = 0; i < charC; i++)
	{
        int xt = i % m_cols;
        int yt = i / m_cols;

        int x = 7;
        for (; x >= 0; x--)
		{
            int xPixel = xt * 8 + x;
            bool emptyColumn = true;
            for (int y = 0; y < 8 && emptyColumn; y++)
			{
                int yPixel = (yt * 8 + y) * w;
				bool emptyPixel = (rawPixels[xPixel + yPixel] >> 24) == 0; // Check the alpha value
                if (!emptyPixel) emptyColumn = false;
            }
            if (!emptyColumn)
			{
                break;
            }
        }

        if (i == ' ') x = 4 - 2;
        charWidths[i] = x + 2;
    }

	delete img;

	// calculate colors
	for (int colorN = 0; colorN < 32; ++colorN)
	{
		int var10 = (colorN >> 3 & 1) * 85;
		int red = (colorN >> 2 & 1) * 170 + var10;
		int green = (colorN >> 1 & 1) * 170 + var10;
		int blue = (colorN >> 0 & 1) * 170 + var10;

		if (colorN == 6)
		{
			red += 85;
		}

		if (options->anaglyph3d)
		{
			int tmpRed = (red * 30 + green * 59 + blue * 11) / 100;
			int tmpGreen = (red * 30 + green * 70) / 100;
			int tmpBlue = (red * 30 + blue * 70) / 100;
			red = tmpRed;
			green = tmpGreen;
			blue = tmpBlue;
		}

		if (colorN >= 16)
		{
			red /= 4;
			green /= 4;
			blue /= 4;
		}

		colors[colorN] = (red & 255) << 16 | (green & 255) << 8 | (blue & 255);
	}
}

#ifndef _XBOX
// 4J Stu - This dtor clashes with one in xui! We never delete these anyway so take it out for now. Can go back when we have got rid of XUI
Font::~Font()
{
	delete[] charWidths;
}
#endif

void Font::renderStyleLine(float x0, float y0, float x1, float y1)
{
	Tesselator* t = Tesselator::getInstance();
	float u = 0.0f, v = 0.0f;
	t->begin();
	t->tex(u, v);
	t->vertex(x0, y1, 0.0f);
	t->tex(u, v);
	t->vertex(x1, y1, 0.0f);
	t->tex(u, v);
	t->vertex(x1, y0, 0.0f);
	t->tex(u, v);
	t->vertex(x0, y0, 0.0f);
	t->end();
}

void Font::renderCharacter(wchar_t c)
{
	float xOff = c % m_cols * m_charWidth;
	float yOff = c / m_cols * m_charWidth;

	float width = charWidths[c] - .01f;
	float height = m_charHeight - .01f;

	float fontWidth = m_cols * m_charWidth;
	float fontHeight = m_rows * m_charHeight;

	const float shear = m_italic ? (height * 0.25f) : 0.0f;
	float x0 = xPos, x1 = xPos + width + shear;
	float y0 = yPos, y1 = yPos + height;

	Tesselator *t = Tesselator::getInstance();
	t->begin();
	t->tex(xOff / fontWidth, (yOff + 7.99f) / fontHeight);
	t->vertex(x0, y1, 0.0f);
	t->tex((xOff + width) / fontWidth, (yOff + 7.99f) / fontHeight);
	t->vertex(x1, y1, 0.0f);
	t->tex((xOff + width) / fontWidth, yOff / fontHeight);
	t->vertex(x1, y0, 0.0f);
	t->tex(xOff / fontWidth, yOff / fontHeight);
	t->vertex(x0, y0, 0.0f);
	t->end();

	if (m_bold)
	{
		float dx = 1.0f;
		t->begin();
		t->tex(xOff / fontWidth, (yOff + 7.99f) / fontHeight);
		t->vertex(x0 + dx, y1, 0.0f);
		t->tex((xOff + width) / fontWidth, (yOff + 7.99f) / fontHeight);
		t->vertex(x1 + dx, y1, 0.0f);
		t->tex((xOff + width) / fontWidth, yOff / fontHeight);
		t->vertex(x1 + dx, y0, 0.0f);
		t->tex(xOff / fontWidth, yOff / fontHeight);
		t->vertex(x0 + dx, y0, 0.0f);
		t->end();
	}

	if (m_underline)
		renderStyleLine(x0, y1 - 1.0f, xPos + static_cast<float>(charWidths[c]), y1);

	if (m_strikethrough)
	{
		float mid = y0 + height * 0.5f;
		renderStyleLine(x0, mid - 0.5f, xPos + static_cast<float>(charWidths[c]), mid + 0.5f);
	}

	xPos += static_cast<float>(charWidths[c]);
}

void Font::drawShadow(const wstring& str, int x, int y, int color)
{
    draw(str, x + 1, y + 1, color, true);
    draw(str, x, y, color, false);
}

void Font::drawShadowLiteral(const wstring& str, int x, int y, int color)
{
	int shadowColor = (color & 0xFCFCFC) >> 2 | (color & 0xFF000000);
	drawLiteral(str, x + 1, y + 1, shadowColor);
	drawLiteral(str, x, y, color);
}

void Font::drawLiteral(const wstring& str, int x, int y, int color)
{
	if (str.empty()) return;
	if ((color & 0xFC000000) == 0) color |= 0xFF000000;
	textures->bindTexture(m_textureLocation);
	glColor4f((color >> 16 & 255) / 255.0F, (color >> 8 & 255) / 255.0F, (color & 255) / 255.0F, (color >> 24 & 255) / 255.0F);
	xPos = static_cast<float>(x);
	yPos = static_cast<float>(y);
	wstring cleanStr = sanitize(str);
	for (size_t i = 0; i < cleanStr.length(); ++i)
		renderCharacter(cleanStr.at(i));
}

void Font::drawShadowWordWrap(const wstring &str, int x, int y, int w, int color, int h)
{
	drawWordWrapInternal(str, x + 1, y + 1, w, color, true, h);
	drawWordWrapInternal(str, x, y, w, color, h);
}

void Font::draw(const wstring& str, int x, int y, int color)
{
	draw(str, x, y, color, false);
}

wstring Font::reorderBidi(const wstring &str)
{
	// 4J Not implemented
	return str;
}

static bool isSectionFormatCode(wchar_t ca)
{
	if ((ca >= L'0' && ca <= L'9') || (ca >= L'a' && ca <= L'f') || (ca >= L'A' && ca <= L'F'))
		return true;
	wchar_t l = static_cast<wchar_t>(ca | 32);
	return l == L'l' || l == L'o' || l == L'n' || l == L'm' || l == L'r' || l == L'k';
}

void Font::draw(const wstring &str, bool dropShadow)
{
	// Bind the texture
	textures->bindTexture(m_textureLocation);

	bool noise = false;
	m_bold = m_italic = m_underline = m_strikethrough = false;
	wstring cleanStr = sanitize(str);

	for (int i = 0; i < static_cast<int>(cleanStr.length()); ++i)
	{
		// Map character
		wchar_t c = cleanStr.at(i);

		if (c == 167 && i + 1 < cleanStr.length())
		{
			wchar_t ca = cleanStr[i+1];
			if (!isSectionFormatCode(ca))
			{
				renderCharacter(167);
				renderCharacter(ca);
				i += 1;
				continue;
			}
			int colorN = 16;
			if(( ca >= L'0' ) && (ca <= L'9')) colorN = ca - L'0';
			else if(( ca >= L'a' ) && (ca <= L'f')) colorN = (ca - L'a') + 10;
			else if(( ca >= L'A' ) && (ca <= L'F')) colorN = (ca - L'A') + 10;

			if (colorN == 16)
			{
				wchar_t l = static_cast<wchar_t>(ca | 32);
				if (l == L'l') m_bold = true;
				else if (l == L'o') m_italic = true;
				else if (l == L'n') m_underline = true;
				else if (l == L'm') m_strikethrough = true;
				else if (l == L'r') m_bold = m_italic = m_underline = m_strikethrough = noise = false;
				else if (l == L'k') noise = true;
			}
			else
			{
				noise = false;
				if (colorN < 0 || colorN > 15) colorN = 15;

				if (dropShadow) colorN += 16;

				int color = colors[colorN];
				glColor3f((color >> 16) / 255.0F, ((color >> 8) & 255) / 255.0F, (color & 255) / 255.0F);
			}


			i += 1;
			continue;
		}

		// "noise" for crazy splash screen message
		if (noise)
		{
			int newc;
			do
			{
				newc = random->nextInt(SharedConstants::acceptableLetters.length());
			} while (charWidths[c + 32] != charWidths[newc + 32]);
			c = newc;
		}

		renderCharacter(c);
	}
}

void Font::draw(const wstring& str, int x, int y, int color, bool dropShadow)
{
	if (!str.empty())
	{
		if ((color & 0xFC000000) == 0) color |= 0xFF000000; // force alpha
		// if not set

		if (dropShadow) // divide RGB by 4, preserve alpha
			color = (color & 0xfcfcfc) >> 2 | (color & (-1 << 24));

		glColor4f((color >> 16 & 255) / 255.0F, (color >> 8 & 255) / 255.0F, (color & 255) / 255.0F, (color >> 24 & 255) / 255.0F);

		xPos = x;
		yPos = y;
		draw(str, dropShadow);
	}
}

int Font::width(const wstring& str)
{
	wstring cleanStr = sanitize(str);

	if (cleanStr == L"") return 0;	// 4J - was NULL comparison
	int len = 0;

	for (int i = 0; i < cleanStr.length(); ++i)
	{
		wchar_t c = cleanStr.at(i);

		if (c == 167)
		{
			if (i + 1 < cleanStr.length() && isSectionFormatCode(cleanStr[i+1]))
				++i;
			else
			{
				len += charWidths[167];
				if (i + 1 < cleanStr.length())
					len += charWidths[static_cast<unsigned>(cleanStr[++i])];
			}
		}
		else
			len += charWidths[c];
	}

	return len;
}

int Font::widthLiteral(const wstring& str)
{
	wstring cleanStr = sanitize(str);
	if (cleanStr == L"") return 0;
	int len = 0;
	for (size_t i = 0; i < cleanStr.length(); ++i)
		len += charWidths[static_cast<unsigned>(cleanStr.at(i))];
	return len;
}

wstring Font::sanitize(const wstring& str)
{
	wstring sb = str;

    for (unsigned int i = 0; i < sb.length(); i++)
	{
        if (CharacterExists(sb[i]))
		{
			sb[i] = MapCharacter(sb[i]);
		}
		else
		{
			// If this character isn't supported, just show the first character (empty square box character)
			sb[i] = 0;
		}
    }
    return sb;
}

int Font::MapCharacter(wchar_t c)
{
	if (!m_charMap.empty())
	{
		// Don't map space character
		return c == ' ' ? c : m_charMap[c];
	}
	else
	{
		return c;
	}
}

bool Font::CharacterExists(wchar_t c)
{
	if (!m_charMap.empty())
	{
		return m_charMap.find(c) != m_charMap.end();
	}
	else
	{
		return c >= 0 && c <= m_rows*m_cols;
	}
}

void Font::drawWordWrap(const wstring &string, int x, int y, int w, int col, int h)
{
	//if (bidirectional)
	//{
	//	string = reorderBidi(string);
	//}
	drawWordWrapInternal(string, x, y, w, col, h);
}

void Font::drawWordWrapInternal(const wstring &string, int x, int y, int w, int col, int h)
{
	drawWordWrapInternal(string, x, y, w, col, false, h);
}

void Font::drawWordWrap(const wstring &string, int x, int y, int w, int col, bool darken, int h)
{
	//if (bidirectional)
	//{
	//	string = reorderBidi(string);
	//}
	drawWordWrapInternal(string, x, y, w, col, darken, h);
}

void Font::drawWordWrapInternal(const wstring& string, int x, int y, int w, int col, bool darken, int h)
{
    vector<wstring>lines = stringSplit(string,L'\n');
    if (lines.size() > 1)
	{
        for ( auto& it : lines )
        {
			// 4J Stu - Don't draw text that will be partially cutoff/overlap something it shouldn't
			if( (y + this->wordWrapHeight(it, w)) > h) break;
            drawWordWrapInternal(it, x, y, w, col, h);
            y += this->wordWrapHeight(it, w);
        }
        return;
    }
    vector<wstring> words = stringSplit(string,L' ');
    unsigned int pos = 0;
    while (pos < words.size())
	{
        wstring line = words[pos++] + L" ";
        while (pos < words.size() && width(line + words[pos]) < w)
		{
            line += words[pos++] + L" ";
        }
        while (width(line) > w)
		{
            int l = 0;
            while (width(line.substr(0, l + 1)) <= w)
			{
                l++;
            }
            if (trimString(line.substr(0, l)).length() > 0)
			{
                draw(line.substr(0, l), x, y, col);
                y += 8;
            }
            line = line.substr(l);

			// 4J Stu - Don't draw text that will be partially cutoff/overlap something it shouldn't
			if( (y + 8) > h) break;
        }
		// 4J Stu - Don't draw text that will be partially cutoff/overlap something it shouldn't
        if (trimString(line).length() > 0 && !( (y + 8) > h) )
		{
            draw(line, x, y, col);
            y += 8;
        }
    }

}

int Font::wordWrapHeight(const wstring& string, int w)
{
    vector<wstring> lines = stringSplit(string,L'\n');
    if (lines.size() > 1)
	{
        int h = 0;
        for ( auto& it : lines )
        {
            h += this->wordWrapHeight(it, w);
        }
        return h;
    }
	vector<wstring> words = stringSplit(string,L' ');
    unsigned int pos = 0;
    int y = 0;
    while (pos < words.size())
	{
        wstring line = words[pos++] + L" ";
        while (pos < words.size() && width(line + words[pos]) < w)
		{
            line += words[pos++] + L" ";
        }
        while (width(line) > w)
		{
            int l = 0;
			while (width(line.substr(0, l + 1)) <= w)
			{
                l++;
            }
            if (trimString(line.substr(0, l)).length() > 0)
			{
                y += 8;
            }
            line = line.substr(l);
        }
        if (trimString(line).length() > 0) {
            y += 8;
        }
    }
    if (y < 8) y += 8;
    return y;

}

void Font::setEnforceUnicodeSheet(bool enforceUnicodeSheet)
{
	this->enforceUnicodeSheet = enforceUnicodeSheet;
}

void Font::setBidirectional(bool bidirectional)
{
	this->bidirectional = bidirectional;
}

bool Font::AllCharactersValid(const wstring &str)
{
	for (int i = 0; i < static_cast<int>(str.length()); ++i)
	{
		wchar_t c = str.at(i);

		if (c == 167 && i + 1 < str.length())
		{
			// skip special color setting
			i += 1;
			continue;
		}

		int index = SharedConstants::acceptableLetters.find(c);

		if ((c != ' ') && !(index > 0 && !enforceUnicodeSheet))
		{
			return false;
		}
	}
	return true;
}

// Not in use
/*// 4J - this code is lifted from #if 0 section above, so that we can directly create what would have gone in each of our 256 + 32 command buffers
void Font::renderFakeCB(IntBuffer *ib)
{
    Tesselator *t = Tesselator::getInstance();

	int i;

	for(unsigned int j = 0; j < ib->limit(); j++)
	{
		int cb = ib->get(j);

		if( cb < 256 )
		{
			i = cb;
			t->begin();
			int ix = i % 16 * 8;
			int iy = i / 16 * 8;
			// float s = 7.99f;
			float s = 7.99f;

			float uo = (0.0f) / 128.0f;
			float vo = (0.0f) / 128.0f;

			t->vertexUV(static_cast<float>(0), static_cast<float>(0 + s), static_cast<float>(0), static_cast<float>(ix / 128.0f + uo), static_cast<float>((iy + s) / 128.0f + vo));
			t->vertexUV(static_cast<float>(0 + s), static_cast<float>(0 + s), static_cast<float>(0), static_cast<float>((ix + s) / 128.0f + uo), static_cast<float>((iy + s) / 128.0f + vo));
			t->vertexUV(static_cast<float>(0 + s), static_cast<float>(0), static_cast<float>(0), static_cast<float>((ix + s) / 128.0f + uo), static_cast<float>(iy / 128.0f + vo));
			t->vertexUV(static_cast<float>(0), static_cast<float>(0), static_cast<float>(0), static_cast<float>(ix / 128.0f + uo), static_cast<float>(iy / 128.0f + vo));
			// target.colorBlit(texture, x + xo, y, color, ix, iy,
		// charWidths[chars[i]], 8);
			t->end();

			glTranslatef(static_cast<float>(charWidths[i]), 0, 0);
		}
		else
		{
			i = cb - 256;

			int br = ((i >> 3) & 1) * 0x55;
			int r = ((i >> 2) & 1) * 0xaa + br;
			int g = ((i >> 1) & 1) * 0xaa + br;
			int b = ((i >> 0) & 1) * 0xaa + br;
			if (i == 6)
			{
				r += 0x55;
			}
			bool darken = i >= 16;

			// color = r << 16 | g << 8 | b;
			if (darken)
			{
				r /= 4;
				g /= 4;
				b /= 4;
			}
			glColor3f(r / 255.0f, g / 255.0f, b / 255.0f);
		}
	}
}

void Font::loadUnicodePage(int page)
{
	wchar_t fileName[25];
	//String fileName = String.format("/1_2_2/font/glyph_%02X.png", page);
	swprintf(fileName,25,L"/1_2_2/font/glyph_%02X.png",page);
	BufferedImage *image = new BufferedImage(fileName);
	//try
	//{
	//	image = ImageIO.read(Textures.class.getResourceAsStream(fileName.toString()));
	//}
	//catch (IOException e)
	//{
	//	throw new RuntimeException(e);
	//}

	unicodeTexID[page] = textures->getTexture(image);
	lastBoundTexture = unicodeTexID[page];
}

void Font::renderUnicodeCharacter(wchar_t c)
{
	if (unicodeWidth[c] == 0)
	{
		// System.out.println("no-width char " + c);
		return;
	}

	int page = c / 256;

	if (unicodeTexID[page] == 0) loadUnicodePage(page);

	if (lastBoundTexture != unicodeTexID[page])
	{
		glBindTexture(GL_TEXTURE_2D, unicodeTexID[page]);
		lastBoundTexture = unicodeTexID[page];
	}

	// first column with non-trans pixels
	int firstLeft = unicodeWidth[c] >> 4;
	// last column with non-trans pixels
	int firstRight = unicodeWidth[c] & 0xF;

	float left = firstLeft;
	float right = firstRight + 1;

	float xOff = c % 16 * 16 + left;
	float yOff = (c & 0xFF) / 16 * 16;
	float width = right - left - .02f;

    Tesselator *t = Tesselator::getInstance();
	t->begin(GL_TRIANGLE_STRIP);
	t->tex(xOff / 256.0F, yOff / 256.0F);
	t->vertex(xPos, yPos, 0.0f);
	t->tex(xOff / 256.0F, (yOff + 15.98f) / 256.0F);
	t->vertex(xPos, yPos + 7.99f, 0.0f);
	t->tex((xOff + width) / 256.0F, yOff / 256.0F);
	t->vertex(xPos + width / 2, yPos, 0.0f);
	t->tex((xOff + width) / 256.0F, (yOff + 15.98f) / 256.0F);
	t->vertex(xPos + width / 2, yPos + 7.99f, 0.0f);
	t->end();

	xPos += (right - left) / 2 + 1;
}
*/

