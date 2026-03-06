#include "stdafx.h"
#include "FileFilter.h"
#include "McRegionLevelStorageSource.h"
#include "File.h"

#ifdef __PS3__
#include <cell/cell_fs.h>
#endif
#ifdef __PSVITA__
#include <fios2.h>
#endif

const wchar_t File::pathSeparator = L'\\';
#ifdef _XBOX
const std::wstring File::pathRoot = L"GAME:"; // Path root after pathSeparator has been removed
#else
const std::wstring File::pathRoot = L""; // Path root after pathSeparator has been removed
#endif

//Creates a new File instance from a parent abstract pathname and a child pathname string.
File::File( const File &parent, const std::wstring& child )
{
	m_abstractPathName = parent.getPath() + pathSeparator + child;
}

//Creates a new File instance by converting the given pathname string into an abstract pathname.
File::File( const wstring& pathname ) //: parent( NULL )
{
	// #ifndef _CONTENT_PACKAGE
	// 	char buf[256];
	// 	wcstombs(buf, pathname.c_str(), 256);
	// 	printf("File::File - %s\n",buf);
	// #endif
	if( pathname.empty() )
		m_abstractPathName = wstring( L"" );
	else
		m_abstractPathName = pathname;

#ifdef _WINDOWS64
	string path = wstringtochararray(m_abstractPathName);
	string finalPath = StorageManager.GetMountedPath(path.c_str());
	if(finalPath.size() == 0) finalPath = path;
	m_abstractPathName = convStringToWstring(finalPath);
#elif defined(_DURANGO)
	wstring finalPath = StorageManager.GetMountedPath(m_abstractPathName.c_str());
	if(finalPath.size() == 0) finalPath = m_abstractPathName;
	m_abstractPathName = finalPath;
#endif
	/*
	vector<wstring> path = stringSplit( pathname, pathSeparator );

	if( path.back().compare( pathRoot ) != 0 )		
	m_abstractPathName = path.back();
	else
	m_abstractPathName = L"";

	path.pop_back();

	if( path.size() > 0 )
	{
	// If the last member of the vector is the root then just stop
	if( path.back().compare( pathRoot ) != 0 )
	this->parent = new File( &path );
	else
	this->parent = NULL;
	}
	*/
}

File::File( const std::wstring& parent, const std::wstring& child  ) //: m_abstractPathName( child  )
{
	m_abstractPathName = pathRoot + pathSeparator + parent + pathSeparator + child;
	//this->parent = new File( parent );
}

//Creates a new File instance by converting the given path vector into an abstract pathname.
/*
File::File( vector<wstring> *path ) : parent( NULL )
{
m_abstractPathName = path->back();
path->pop_back();

if( path->size() > 0 )
{
// If the last member of the vector is the root then just stop
if( path->back().compare( pathRoot ) != 0 )
this->parent = new File( path );
else
this->parent = NULL;
}
}
*/

//Deletes the file or directory denoted by this abstract pathname. If this pathname denotes a directory,
//then the directory must be empty in order to be deleted.
//Returns:
//true if and only if the file or directory is successfully deleted; false otherwise
bool File::_delete()
{
#ifdef _UNICODE
	BOOL result = DeleteFile( getPath().c_str() );
#else
	BOOL result = DeleteFile( wstringtofilename(getPath()) );
#endif
	if( result == 0 )
	{
		DWORD error = GetLastError();
#ifndef _CONTENT_PACKAGE
		printf( "File::_delete - Error code %d (%#0.8X)\n", error, error );
#endif
		return false;
	}
	else
		return true;
}

//Creates the directory named by this abstract pathname.
//Returns:
//true if and only if the directory was created; false otherwise
bool File::mkdir() const
{
#ifdef _UNICODE
	return CreateDirectory( getPath().c_str(),  NULL) != 0;

#else
	return CreateDirectory( wstringtofilename(getPath()),  NULL) != 0;
#endif
}


//Creates the directory named by this abstract pathname, including any
//necessary but nonexistent parent directories.  Note that if this
//operation fails it may have succeeded in creating some of the necessary
//parent directories.
//
//@return  <code>true</code> if and only if the directory was created,
//         along with all necessary parent directories; <code>false</code>
//         otherwise
//
//@throws  SecurityException
//         If a security manager exists and its <code>{@link
//         java.lang.SecurityManager#checkRead(java.lang.String)}</code>
//         method does not permit verification of the existence of the 
//         named directory and all necessary parent directories; or if
//         the <code>{@link 
//         java.lang.SecurityManager#checkWrite(java.lang.String)}</code>
//         method does not permit the named directory and all necessary
//         parent directories to be created
//
bool File::mkdirs() const
{
	std::vector<std::wstring> path = stringSplit( m_abstractPathName, pathSeparator );

	std::wstring pathToHere = L"";
	for( auto& it : path )
	{
		// If this member of the vector is the root then just skip to the next
		if( pathRoot.compare(it) == 0 )
		{			
			pathToHere = it;
			continue;
		}

		pathToHere = pathToHere + pathSeparator + it;

		// if not exists
#ifdef _UNICODE
		if( GetFileAttributes(  pathToHere.c_str() ) == -1 )
		{
			DWORD result = CreateDirectory( pathToHere.c_str(),  NULL);
			if( result == 0 )
			{
				// Failed to create
				return false;
			}
		}
#else
		if( GetFileAttributes(  wstringtofilename(pathToHere) ) == -1 )
		{
			DWORD result = CreateDirectory( wstringtofilename(pathToHere),  NULL);
			if( result == 0 )
			{
				// Failed to create
				return false;
			}
		}
#endif
	}

	// We should now exist
	assert( exists() );

	return true;
}

/*
File *File::getParent() const 
{
return (File *) parent;
}
*/

//Tests whether the file or directory denoted by this abstract pathname exists.
//Returns:
//true if and only if the file or directory denoted by this abstract pathname exists; false otherwise
bool File::exists() const
{
	// TODO 4J Stu - Possible we could get an error result from something other than the file not existing?
#ifdef _UNICODE
	return GetFileAttributes(  getPath().c_str() ) != -1;

#else
	return GetFileAttributes(  wstringtofilename(getPath()) ) != -1;
#endif
}

//Tests whether the file denoted by this abstract pathname is a normal file. A file is normal if it is not a directory and,
//in addition, satisfies other system-dependent criteria. Any non-directory file created by a Java application is guaranteed to be a normal file.
//Returns:
//true if and only if the file denoted by this abstract pathname exists and is a normal file; false otherwise
bool File::isFile() const
{
	return exists() && !isDirectory();
}

//Renames the file denoted by this abstract pathname.
//Whether or not this method can move a file from one filesystem to another is platform-dependent.
//The return value should always be checked to make sure that the rename operation was successful.
//
//Parameters:
//dest - The new abstract pathname for the named file
//Returns:
//true if and only if the renaming succeeded; false otherwise
bool File::renameTo(File dest)
{
	// 4J Stu - The wstringtofilename function returns a pointer to the same location in memory every time it is
	// called, therefore we were getting sourcePath and destPath having the same value. The solution here is to
	// make a copy of the sourcePath by storing it in a std::string
	std::string sourcePath = wstringtofilename(getPath());
	const char *destPath = wstringtofilename(dest.getPath());
#ifdef _DURANGO
	__debugbreak();	// TODO
	BOOL result = false;
#else
	BOOL result = MoveFile(sourcePath.c_str(), destPath);
#endif

	if( result == 0 )
	{
		DWORD error = GetLastError();
#ifndef _CONTENT_PACKAGE
		printf( "File::renameTo - Error code %d (%#0.8X)\n", error, error );
#endif
		return false;
	}
	else
		return true;
}

//Returns an array of abstract pathnames denoting the files in the directory denoted by this abstract pathname.
//If this abstract pathname does not denote a directory, then this method returns null. Otherwise an array of File objects is returned,
//one for each file or directory in the directory. Pathnames denoting the directory itself and the directory's parent directory
//are not included in the result. Each resulting abstract pathname is constructed from this abstract pathname using 
//the File(File, String) constructor. Therefore if this pathname is absolute then each resulting pathname is absolute;
//if this pathname is relative then each resulting pathname will be relative to the same directory.
//
//There is no guarantee that the name strings in the resulting array will appear in any specific order; they are not,
//in particular, guaranteed to appear in alphabetical order.
//
//Returns:
//An array of abstract pathnames denoting the files and directories in the directory denoted by this abstract pathname.
//The array will be empty if the directory is empty. Returns null if this abstract pathname does not denote a directory,
//or if an I/O error occurs.
std::vector<File *> *File::listFiles() const
{
	std::vector<File *> *vOutput = new vector<File *>();

	// TODO 4J Stu - Also need to check for I/O errors?
	if( !isDirectory() )
		return vOutput;

#ifdef __PS3__
	const char *lpFileName=wstringtofilename(getPath());
	char filePath[256];

	std::string mountedPath = StorageManager.GetMountedPath(lpFileName);
	if(mountedPath.length() > 0)
	{
		strcpy(filePath, mountedPath.c_str());
	}
	else if(lpFileName[0] == '/') // already fully qualified path
		strcpy(filePath, lpFileName );
	else
		sprintf(filePath,"%s/%s",getUsrDirPath(), lpFileName );
	int fd;
	CellFsErrno err = cellFsOpendir(filePath , &fd);

	CellFsDirectoryEntry de;
	uint32_t count = 0;
	err = cellFsGetDirectoryEntries(fd, &de, sizeof(CellFsDirectoryEntry), &count);
	if(count != 0)
	{
		do
		{
			if(de.attribute.st_mode & CELL_FS_S_IFREG) vOutput->push_back( new File( *this, filenametowstring( de.entry_name.d_name ) ) );
			err = cellFsGetDirectoryEntries(fd, &de, sizeof(CellFsDirectoryEntry), &count);
		} while( count );
	}
	err = cellFsClose(fd);
#elif defined __ORBIS__ || defined __PSVITA__
	const char *lpFileName=wstringtofilename(getPath());
	char filePath[256];

	std::string mountedPath = StorageManager.GetMountedPath(lpFileName);
	if(mountedPath.length() > 0)
	{
		strcpy(filePath, mountedPath.c_str());
	}
	else if(lpFileName[0] == '/') // already fully qualified path
		strcpy(filePath, lpFileName );
	else
		sprintf(filePath,"%s/%s",getUsrDirPath(), lpFileName );

	bool exists = sceFiosDirectoryExistsSync( NULL, filePath );
	if( !exists  )
	{
		app.DebugPrintf("\nsceFiosDirectoryExistsSync - Directory doesn't exist\n");
	}

	//CD - Vita note: sceFiosDHOpenSync returns SCE_FIOS_ERROR_UNIMPLEMENTED
	//CD - The Handle also returns as 0 [dh], sceFiosDHOpen could also be failing
	//CD - Hence, this fails stating 0 files in directory

	SceFiosDH dh = SCE_FIOS_DH_INVALID;
	SceFiosBuffer buf;
	buf.length = 0;
	SceFiosOp op = sceFiosDHOpen(NULL, &dh, filePath, buf);

	int err = sceFiosOpWait(op);
	if( err != SCE_FIOS_OK  )
	{
		app.DebugPrintf("\nsceFiosOpWait = 0x%x\n",err);
	}
	SceFiosSize size = sceFiosOpGetActualCount(op);
	char *pBuf = new char[size];
	buf.set(pBuf, (size_t)size);

	sceFiosOpDelete( op );
	sceFiosDHClose(NULL, dh);

	err = sceFiosDHOpenSync(NULL, &dh, filePath, buf);
	if( err != SCE_FIOS_OK  )
	{
		app.DebugPrintf("\nsceFiosDHOpenSync = 0x%x\n",err);
	}
	SceFiosDirEntry entry;
	ZeroMemory(&entry, sizeof(SceFiosDirEntry));
	err = sceFiosDHReadSync(NULL, dh, &entry);
	while( err == SCE_FIOS_OK)
	{
		vOutput->push_back( new File( *this, filenametowstring( entry.fullPath + entry.offsetToName ) ) );
		ZeroMemory(&entry, sizeof(SceFiosDirEntry));
		err = sceFiosDHReadSync(NULL, dh, &entry);
	};

	sceFiosDHClose(NULL, dh);
	delete pBuf;
#else

	WIN32_FIND_DATA wfd;

#ifdef _UNICODE
	WCHAR path[MAX_PATH];
	swprintf( path, L"%ls\\*", getPath().c_str() );
	HANDLE hFind = FindFirstFile( path, &wfd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		int count = 0;
		do
		{
			//if( !(wfd.dwFileAttributes & dwAttr) )
			vOutput->push_back( new File( *this, wfd.cFileName  ) );
		} 
		while( FindNextFile( hFind, &wfd) );
		FindClose( hFind);
	}
#else
	char path[MAX_PATH] {};
	snprintf( path, MAX_PATH, "%s\\*", wstringtofilename( getPath() ) );
	HANDLE hFind = FindFirstFile( path, &wfd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		//int count = 0;
		do
		{
			//if( !(wfd.dwFileAttributes & dwAttr) )
			vOutput->push_back( new File( *this, filenametowstring( wfd.cFileName ) ) );
		} 
		while( FindNextFile( hFind, &wfd) );
		FindClose( hFind);
	}
#endif
#endif
	return vOutput;
}

//Returns an array of abstract pathnames denoting the files and directories in the directory denoted by this abstract pathname that
//satisfy the specified filter. The behavior of this method is the same as that of the listFiles() method, except that the pathnames
//in the returned array must satisfy the filter. If the given filter is null then all pathnames are accepted. Otherwise, a pathname
//satisfies the filter if and only if the value true results when the FileFilter.accept(java.io.File) method of the filter is invoked
//on the pathname.
//Parameters:
//filter - A file filter
//Returns:
//An array of abstract pathnames denoting the files and directories in the directory denoted by this abstract pathname.
//The array will be empty if the directory is empty. Returns null if this abstract pathname does not denote a directory, or if an I/O error occurs.
std::vector<File *> *File::listFiles(FileFilter *filter) const
{
	// TODO 4J Stu - Also need to check for I/O errors?
	if( !isDirectory() )
		return NULL;

	std::vector<File *> *vOutput = new std::vector<File *>();

#ifdef __PS3__
	const char *lpFileName=wstringtofilename(getPath());
	char filePath[256];

	std::string mountedPath = StorageManager.GetMountedPath(lpFileName);
	if(mountedPath.length() > 0)
	{
		strcpy(filePath, mountedPath.c_str());
	}
	else if(lpFileName[0] == '/') // already fully qualified path
		strcpy(filePath, lpFileName );
	else
		sprintf(filePath,"%s/%s",getUsrDirPath(), lpFileName );
	int fd;
	CellFsErrno err = cellFsOpendir(filePath, &fd);

	CellFsDirectoryEntry de;
	uint32_t count = 0;
	err = cellFsGetDirectoryEntries(fd, &de, sizeof(CellFsDirectoryEntry), &count);
	if(count != 0)
	{
		do
		{
			File thisFile = File( *this, filenametowstring( de.entry_name.d_name ) );
			if( filter->accept( &thisFile ) )
			{
				File storageFile = thisFile;
				if(de.attribute.st_mode & CELL_FS_S_IFREG) vOutput->push_back( &storageFile );
			}
			err = cellFsGetDirectoryEntries(fd, &de, sizeof(CellFsDirectoryEntry), &count);
		} while( count );
	}
	err = cellFsClose(fd);
#else

#ifdef _UNICODE

	WCHAR path[MAX_PATH];
	WIN32_FIND_DATA wfd;
	DWORD dwAttr = FILE_ATTRIBUTE_DIRECTORY;

	swprintf( path, L"%ls\\*", getPath().c_str() );
	HANDLE hFind = FindFirstFile( path, &wfd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		int count = 0;
		do
		{
			File thisFile = File( *this, wfd.cFileName  );
			if( filter->accept( &thisFile ) )
			{
				File storageFile = thisFile;
				vOutput->push_back( &storageFile );
			}
		} while( FindNextFile( hFind, &wfd) );
		FindClose( hFind);
	}
#else
	char path[MAX_PATH];
	WIN32_FIND_DATA wfd;
	//DWORD dwAttr = FILE_ATTRIBUTE_DIRECTORY;

	sprintf( path, "%s\\*", wstringtofilename( getPath() ) );
	HANDLE hFind = FindFirstFile( path, &wfd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		//int count = 0;
		do
		{
			File thisFile = File( *this, filenametowstring( wfd.cFileName ) );
			if( filter->accept( &thisFile ) )
			{
				File storageFile = thisFile;
				vOutput->push_back( &storageFile );
			}
		} while( FindNextFile( hFind, &wfd) );
		FindClose( hFind);
	}
#endif
#endif
	return vOutput;
}

//Tests whether the file denoted by this abstract pathname is a directory.
//Returns:
//true if and only if the file denoted by this abstract pathname exists and is a directory; false otherwise
bool File::isDirectory() const
{
#ifdef _UNICODE
	return exists() && ( GetFileAttributes( getPath().c_str() ) & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
#else
	return exists() && ( GetFileAttributes( wstringtofilename(getPath()) ) & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
#endif
}

//Returns the length of the file denoted by this abstract pathname. The return value is unspecified if this pathname denotes a directory.
//Returns:
//The length, in bytes, of the file denoted by this abstract pathname, or 0L if the file does not exist
int64_t File::length()
{
#ifdef __PS3__
	//extern const char* getPS3HomePath();
	CellFsErrno err=0;
	const char *lpFileName=wstringtofilename(getPath());
	char filePath[256];

	std::string mountedPath = StorageManager.GetMountedPath(lpFileName);
	if(mountedPath.length() > 0)
	{
		strcpy(filePath, mountedPath.c_str());
	}
	else if(lpFileName[0] == '/') // already fully qualified path
		strcpy(filePath, lpFileName );
	else
		sprintf(filePath,"%s/%s",getUsrDirPath(), lpFileName );

#ifndef _CONTENT_PACKAGE
	//printf("+++File::length - %s\n",filePath);
#endif
	// check if the file exists first
	CellFsStat statData;
	err=cellFsStat(filePath, &statData);

	if( err != CELL_FS_SUCCEEDED)
	{
		//printf("+++File::length FAILED with %d\n",err);
		return 0;
	}
	if(statData.st_mode == CELL_FS_S_IFDIR)
	{
		//printf("+++File::length FAILED with %d\n",err);
		return 0;
	}

	//printf("+++File::length - %ll\n",statData.st_size);

	return statData.st_size;
#elif defined __ORBIS__ || defined __PSVITA__

	char filePath[256];
	const char *lpFileName=wstringtofilename(getPath());

	std::string mountedPath = StorageManager.GetMountedPath(lpFileName);
	if(mountedPath.length() > 0)
	{
		strcpy(filePath, mountedPath.c_str());
	}
	else if(lpFileName[0] == '/') // already fully qualified path
		strcpy(filePath, lpFileName );
	else
		sprintf(filePath,"%s/%s",getUsrDirPath(), lpFileName );

	// check if the file exists first
	SceFiosStat  statData;
	if(sceFiosStatSync(NULL, filePath, &statData) != SCE_FIOS_OK)
	{
		return 0;
	}
	if(statData.statFlags & SCE_FIOS_STATUS_DIRECTORY  )
	{
		return 0;
	}
	return statData.fileSize;
#else
	WIN32_FILE_ATTRIBUTE_DATA fileInfoBuffer;

#ifdef _UNICODE
	BOOL result = GetFileAttributesEx(
		getPath().c_str(), // file or directory name
		GetFileExInfoStandard, // attribute 
		&fileInfoBuffer // attribute information 
		);
#else
	BOOL result = GetFileAttributesEx(
		wstringtofilename(getPath()), // file or directory name
		GetFileExInfoStandard, // attribute 
		&fileInfoBuffer // attribute information 
		);
#endif

	if( result != 0 && !( (fileInfoBuffer.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) )
	{
		// Success
		LARGE_INTEGER liFileSize;
		liFileSize.HighPart = fileInfoBuffer.nFileSizeHigh;
		liFileSize.LowPart = fileInfoBuffer.nFileSizeLow;

		return liFileSize.QuadPart;
	}
	else
	{
		//Fail or a Directory
		return 0l;
	}
#endif
}

//Returns the time that the file denoted by this abstract pathname was last modified.
//Returns:
//A long value representing the time the file was last modified, measured in milliseconds since the epoch (00:00:00 GMT, January 1, 1970),
//or 0L if the file does not exist or if an I/O error occurs
int64_t File::lastModified()
{
	WIN32_FILE_ATTRIBUTE_DATA fileInfoBuffer;
#ifdef _UNICODE
	BOOL result = GetFileAttributesEx(
		getPath().c_str(), // file or directory name
		GetFileExInfoStandard, // attribute 
		&fileInfoBuffer // attribute information 
		);
#else
	BOOL result = GetFileAttributesEx(
		wstringtofilename(getPath()), // file or directory name
		GetFileExInfoStandard, // attribute 
		&fileInfoBuffer // attribute information 
		);
#endif

	if( result != 0 && !( (fileInfoBuffer.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) )
	{
		// Success
		LARGE_INTEGER liLastModified;
		liLastModified.HighPart = fileInfoBuffer.ftLastWriteTime.dwHighDateTime;
		liLastModified.LowPart = fileInfoBuffer.ftLastWriteTime.dwLowDateTime;

		return liLastModified.QuadPart;
	}
	else
	{
		//Fail or a Directory
		return 0l;
	}
}

const std::wstring File::getPath() const
{
	/*
	wstring path;
	if ( parent != NULL)
	path = parent->getPath();
	else
	path = wstring(pathRoot);

	path.push_back( pathSeparator );
	path.append(m_abstractPathName);
	*/
	return m_abstractPathName;
}

std::wstring File::getName() const
{
	unsigned int sep = (unsigned int )(m_abstractPathName.find_last_of( this->pathSeparator ));
	return m_abstractPathName.substr( sep + 1, m_abstractPathName.length() );
}

bool File::eq_test(const File &x, const File &y)
{
	return x.getPath().compare( y.getPath() ) == 0;
}

// 4J TODO JEV, a better hash function may be nessesary.
int File::hash_fnct(const File &k)
{
	int hashCode = 0;

	//if (k->parent != NULL)
	//	hashCode = hash_fnct(k->getParent());

	wchar_t *ref = (wchar_t *) k.m_abstractPathName.c_str();

	for (unsigned int i = 0; i < k.m_abstractPathName.length(); i++)
	{
		hashCode += ((hashCode * 33) + ref[i]) % 149;
	}

	return (int) hashCode;
}

int FileKeyHash::operator() (const File &k) const 
{ return File::hash_fnct(k); }

bool FileKeyEq::operator() (const File &x, const File &y) const 
{ return File::eq_test(x,y); }