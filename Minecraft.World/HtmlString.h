#pragma once

// 4J: Simple string wrapper that includes basic formatting information
class HtmlString
{
public:
	wstring text; // Text content of string
	eMinecraftColour color; // Hex color
	bool italics; // Show text in italics
	bool indent; // Indent text

	HtmlString(wstring text, eMinecraftColour color = eMinecraftColour_NOT_SET, bool italics = false, bool indent = false);
	wstring ToString();

	static wstring Compose(vector<HtmlString> *strings);
};