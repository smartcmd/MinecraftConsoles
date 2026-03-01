#pragma once
using namespace std;
#include "Class.h"

class SharedConstants
{
	public:
		static void staticCtor();
		static const wstring VERSION_STRING;
		static const int NETWORK_PROTOCOL_VERSION = 78;
		static const bool INGAME_DEBUG_OUTPUT = false;
		
		// NOT texture resolution. How many sub-blocks each block face is made up of.
		// 4J Added for texture packs
		static const int WORLD_RESOLUTION = 16;

		static bool isAllowedChatCharacter(char ch);

	private:
		static wstring readAcceptableChars();

	public:
		static const int maxChatLength = 100;
		static wstring acceptableLetters;

		static const int ILLEGAL_FILE_CHARACTERS_LENGTH = 15;
		static const wchar_t ILLEGAL_FILE_CHARACTERS[ILLEGAL_FILE_CHARACTERS_LENGTH];

		static const bool TEXTURE_LIGHTING;		// 4J - change brought forward from 1.8.2
		static const int TICKS_PER_SECOND = 20;

		static const int FULLBRIGHT_LIGHTVALUE = 15 << 20 | 15 << 4;
};