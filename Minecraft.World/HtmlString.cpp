#include "stdafx.h"
#include "HtmlString.h"
#include <iomanip>

HtmlString::HtmlString(wstring text, eMinecraftColour hexColor, bool italics, bool indent)
{
	this->text = escapeXML(text);
	this->color = hexColor;
	this->italics = italics;
	this->indent = indent;
}

wstring HtmlString::ToString()
{
	std::wstringstream ss;

	if (indent)
	{
		ss << L"&nbsp;&nbsp;";
	}

	if (italics)
	{
		ss << "<i>";
	}

	eMinecraftColour color = this->color == eMinecraftColour_NOT_SET ? eHTMLColor_7 : this->color;

	ss << L"<font color=\"#" << std::setfill(L'0') << std::setw(6) << std::hex << app.GetHTMLColor(color) << L"\">" << text << "</font>";

	if (italics)
	{
		ss << "</i>";
	}

	return ss.str();
}

wstring HtmlString::Compose(vector<HtmlString> *strings)
{
	if (strings == nullptr) return L"";

	std::wstringstream ss;

	for(int i = 0; i < strings->size(); i++)
	{
		ss << strings->at(i).ToString();

		// Add a break if there's another line
		if (i + 1 < strings->size())
		{
			ss << L"<br>";
		}
	}

	return ss.str();
}