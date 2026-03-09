#include "stdafx.h"
#include "PSVitaStrings.h"
#include <ces.h>

uint8_t *mallocAndCreateUTF8ArrayFromString(int iID)
{
	LPCWSTR wchString=app.GetString(iID);
	size_t src_len,dst_len;
	int iLen=wcslen(wchString);
	src_len=sizeof(WCHAR)*(iLen);

	SceCesUcsContext context;
	int result = sceCesUcsContextInit( &context );

	if( result != S_OK ) 
	{
		app.DebugPrintf("sceCesUcsContextInit failed\n");
		return nullptr;
	}

	uint32_t utf16Len;
	uint32_t utf8Len;
	result = sceCesUtf16StrGetUtf8Len( &context,
		(uint16_t *)wchString,
		iLen,
		&utf16Len,
		&utf8Len
		);

	utf8Len += 1;
	uint8_t *strUtf8=(uint8_t *)malloc(utf8Len);
	memset(strUtf8,0,utf8Len);


	result = sceCesUtf16StrToUtf8Str(
		&context,
		(uint16_t *)wchString,
		iLen,
		&utf16Len,
		strUtf8,
		utf8Len,
		&utf8Len
		);
	if( result != SCE_OK ) 
	{
		app.DebugPrintf("sceCesUtf16StrToUtf8Str: conversion error : 0x%x\n", result);
		return nullptr;
	}

	return strUtf8;
}