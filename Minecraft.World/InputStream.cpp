#include "stdafx.h"
#include "File.h"
#include "InputOutputStream.h"
#include "InputStream.h"

InputStream *InputStream::getResourceAsStream(const wstring &fileName)
{
	File file( fileName );

	return file.exists() ? new FileInputStream( file ) : nullptr;
}