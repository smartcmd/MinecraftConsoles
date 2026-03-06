#include "stdafx.h"
#include "Filesystem.h"

#ifdef _WINDOWS64
#include <windows.h>
#endif // TODO: More os' filesystem handling for when the project moves away from only Windows

#include <stdio.h>

bool FileOrDirectoryExists(const char* path)
{
#ifdef _WINDOWS64
    DWORD attribs = GetFileAttributesA(path);
    return (attribs != INVALID_FILE_ATTRIBUTES);
#else
    #error "FileOrDirectoryExists not implemented for this platform"
    return false;
#endif
}

bool FileExists(const char* path)
{
#ifdef _WINDOWS64
    DWORD attribs = GetFileAttributesA(path);
    return (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY));
#else
    #error "FileExists not implemented for this platform"
    return false;
#endif
}

bool DirectoryExists(const char* path)
{
#ifdef _WINDOWS64
    DWORD attribs = GetFileAttributesA(path);
    return (attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY));
#else
    #error "DirectoryExists not implemented for this platform"
    return false;
#endif
}

bool GetFirstFileInDirectory(const char* directory, char* outFilePath, size_t outFilePathSize)
{
#ifdef _WINDOWS64
    char searchPath[MAX_PATH];
    snprintf(searchPath, MAX_PATH, "%s\\*", directory);

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath, &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    do
    {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            // Found a file, copy its path to the output buffer
            snprintf(outFilePath, outFilePathSize, "%s\\%s", directory, findData.cFileName);
            FindClose(hFind);
            return true;
        }
    } while (FindNextFileA(hFind, &findData) != 0);

    FindClose(hFind);
    return false; // No files found in the directory
#else
    #error "GetFirstFileInDirectory not implemented for this platform"
    return false;
#endif
}
