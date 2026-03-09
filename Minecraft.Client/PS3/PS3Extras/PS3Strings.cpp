#include "stdafx.h"
#include "PS3Strings.h"

uint8_t *mallocAndCreateUTF8ArrayFromString(int iID)
{
	int result;
	l10n_conv_t cd;
	LPCWSTR wchString=app.GetString(iID);
	size_t src_len,dst_len;
	int iLen=wcslen(wchString);
	src_len=sizeof(WCHAR)*(iLen);

	if( (cd = l10n_get_converter( L10N_UTF16, L10N_UTF8 )) == -1 ) 
	{
		app.DebugPrintf("l10n_get_converter: no such converter\n");
		return nullptr;
	}

	l10n_convert_str( cd, wchString, &src_len, nullptr, &dst_len );
	uint8_t *strUtf8=static_cast<uint8_t *>(malloc(dst_len));
	memset(strUtf8,0,dst_len);

	result = l10n_convert_str( cd, wchString, &src_len, strUtf8, &dst_len );
	if( result != ConversionOK ) 
	{
		app.DebugPrintf("l10n_convert: conversion error : 0x%x\n", result);
		return nullptr;
	}

	return strUtf8;
}