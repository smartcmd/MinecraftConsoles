#pragma once
using namespace std;

wstring toLower(const wstring& a);
wstring trimString(const wstring& a);
wstring replaceAll(const wstring& in, const wstring& replace, const wstring& with);

bool equalsIgnoreCase(const wstring& a, const wstring& b);

template <class T> T _fromString(const std::wstring& s)
{
	std::wistringstream stream (s);
	T t;
	stream >> t;
	return t;
}
template <class T> T _fromHEXString(const std::wstring& s)
{
	std::wistringstream stream (s);
	T t;
	stream >> std::hex >> t;
	return t;
}

wstring convStringToWstring(const string& converting);
const char *wstringtofilename(const wstring& name);
const char *wstringtochararray(const wstring& name);
wstring filenametowstring(const char *name);

std::vector<std::wstring> &stringSplit(const std::wstring &s, wchar_t delim, std::vector<std::wstring> &elems);
std::vector<std::wstring> stringSplit(const std::wstring &s, wchar_t delim);

void stripWhitespaceForHtml(wstring &string, bool bRemoveNewline=true);
wstring escapeXML(const wstring &in);
wstring parseXMLSpecials(const wstring &in);
