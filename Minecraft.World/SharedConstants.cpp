#include "stdafx.h"
#include "File.h"
#include "InputOutputStream.h"
#include "SharedConstants.h"

const wstring SharedConstants::VERSION_STRING = L"1.6.4";
const bool SharedConstants::TEXTURE_LIGHTING = true;

wstring SharedConstants::readAcceptableChars()
{
	// 4J-PB - I've added ã in (for Portuguese in bed string) and added the character at the same place in the default.png font
	wstring result = L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'abcdefghijklmnopqrstuvwxyz{|}~ ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜø£Ø×ƒáíóúñÑªº¿®¬½¼¡«»ã";
#if 0 // 4J - do we actually really need to get this from a file?
	//try {
	//BufferedReader br = new BufferedReader(new InputStreamReader(SharedConstants.class.getResourceAsStream("/font.txt"), "UTF-8"));
		BufferedReader *br = new BufferedReader(new InputStreamReader( new FileInputStream( new File( wstring(L"/font.txt") ) ) ) );
		wstring line;
		while ( !(line = br->readLine()).empty())
		{
			if (!( wcscmp( &line[0], L"#") == 0 ) )
			{
				result.append( line );
			}
		}
		br->close();
	//} catch (Exception e) {
		// TODO 4J Stu - Exception handling?
	//}
#endif
	return result;
}

bool SharedConstants::isAllowedChatCharacter(char ch)
{
	//return ch != '§' && (acceptableLetters.indexOf(ch) >= 0 || (int) ch > 32);
	// 4J Unused
	return true;
}

wstring SharedConstants::acceptableLetters;

void SharedConstants::staticCtor()
{
	acceptableLetters = readAcceptableChars();
}

const wchar_t SharedConstants::ILLEGAL_FILE_CHARACTERS[ILLEGAL_FILE_CHARACTERS_LENGTH] =
{
	// 4J-PB  - corrected
	L'/', L'\n', L'\r', L'\t', L'\0', L'\f', L'`', L'?', L'*', L'\\', L'<', L'>', L'|', L'\"', L':'
};