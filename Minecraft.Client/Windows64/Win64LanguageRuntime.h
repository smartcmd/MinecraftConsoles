#pragma once

#include <Windows.h>
#include <string>
#include <vector>

DWORD XGetLanguage();
DWORD XGetLocale();

void Win64SetLanguageLocale(DWORD language, DWORD locale);
void Win64ForceEnglishLanguage();

void Win64SetForceLocaleCode(const std::wstring& localeCode);
const std::wstring& Win64GetForceLocaleCode();

void Win64SetTryAllLanguagesAtStartup(bool enabled);
bool Win64GetTryAllLanguagesAtStartup();

void Win64SetLanguageValidationFailed(bool failed);
bool Win64GetLanguageValidationFailed();

void Win64SetExtraLanguageCodes(const std::vector<std::wstring>& localeCodes);
const std::vector<std::wstring>& Win64GetExtraLanguageCodes();