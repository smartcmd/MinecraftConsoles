#include "stdafx.h"

wstring toLower(const wstring& a)
{
	wstring out = wstring(a);
	std::transform(out.begin(), out.end(), out.begin(), ::tolower);
	return out;
}

wstring trimString(const wstring& a)
{
	wstring b;
	int start = (int)a.find_first_not_of(L" \t\n\r");
	int end = (int)a.find_last_not_of(L" \t\n\r");
	if( start == wstring::npos ) start = 0;
	if( end == wstring::npos ) end = (int)a.size()-1;
	b = a.substr(start,(end-start)+1);
	return b;
}

wstring replaceAll(const wstring& in, const wstring& replace, const wstring& with)
{
	wstring out = in;
	size_t pos = 0;
	while( ( pos = out.find(replace, pos) ) != wstring::npos )
	{
		out.replace( pos, replace.length(), with );
		pos++;
	}
	return out;
}

bool equalsIgnoreCase(const wstring& a, const wstring& b)
{
	bool out;
	wstring c = toLower(a);
	wstring d = toLower(b);
	out = c.compare(d) == 0;
	return out;
}

wstring convStringToWstring(const string& converting)
{
	wstring converted(converting.length(), L' ');
	copy(converting.begin(), converting.end(), converted.begin());
	return converted;
}

// Convert for filename wstrings to a straight character pointer for Xbox APIs. The returned string is only valid until
// this function is called again, and it isn't thread-safe etc. as I'm just storing the returned name in a local static
// to save having to clear it up everywhere this is used.
const char *wstringtofilename(const wstring& name)
{
	static char buf[256];
	assert(name.length()<256);
	for(unsigned int i = 0; i < name.length(); i++ )
	{
		wchar_t c = name[i];
#if defined __PS3__ || defined __ORBIS__
		if(c=='\\') c='/';
#else
		if(c=='/') c='\\';
#endif
		assert(c<128);	// Will we have to do any conversion of non-ASCII characters in filenames?
		buf[i] = (char)c;
	}
	buf[name.length()] = 0;
	return buf;
}

const char *wstringtochararray(const wstring& name)
{
	static char buf[256];
	assert(name.length()<256);
	for(unsigned int i = 0; i < name.length(); i++ )
	{
		wchar_t c = name[i];
		assert(c<128);	// Will we have to do any conversion of non-ASCII characters in filenames?
		buf[i] = (char)c;
	}
	buf[name.length()] = 0;
	return buf;
}

wstring filenametowstring(const char *name)
{
	return convStringToWstring(name);
}

std::vector<std::wstring> &stringSplit(const std::wstring &s, wchar_t delim, std::vector<std::wstring> &elems)
{
    std::wstringstream ss(s);
    std::wstring item;
    while(std::getline(ss, item, delim))
	{
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::wstring> stringSplit(const std::wstring &s, wchar_t delim)
{
    std::vector<std::wstring> elems;
    return stringSplit(s, delim, elems);
}

bool BothAreSpaces(wchar_t lhs, wchar_t rhs) { return (lhs == rhs) && (lhs == L' '); }

void stripWhitespaceForHtml(wstring &string, bool bRemoveNewline)
{
	// Strip newline chars
	if(bRemoveNewline)
	{	
		string.erase(std::remove(string.begin(), string.end(), '\n'), string.end());
		string.erase(std::remove(string.begin(), string.end(), '\r'), string.end());
	}

	string.erase(std::remove(string.begin(), string.end(), '\t'), string.end());

	// Strip duplicate spaces
	string.erase(std::unique(string.begin(), string.end(), BothAreSpaces), string.end()); 

	string = trimString(string);
}

wstring escapeXML(const wstring &in)
{
	wstring out = in;
	out = replaceAll(out, L"&", L"&amp;");
	//out = replaceAll(out, L"\"", L"&quot;");
	//out = replaceAll(out, L"'", L"&apos;");
	out = replaceAll(out, L"<", L"&lt;");
	out = replaceAll(out, L">", L"&gt;");
	return out;
}

wstring parseXMLSpecials(const wstring &in)
{
	wstring out = in;
	out = replaceAll(out, L"&amp;", L"&");
	//out = replaceAll(out, L"\"", L"&quot;");
	//out = replaceAll(out, L"'", L"&apos;");
	out = replaceAll(out, L"&lt;", L"<");
	out = replaceAll(out, L"&gt;", L">");
	return out;
}