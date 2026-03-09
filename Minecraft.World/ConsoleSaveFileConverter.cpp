#include "stdafx.h"
#include "net.minecraft.world.level.chunk.storage.h"
#include "net.minecraft.world.level.storage.h"
#include "ConsoleSaveFileIO.h"
#include "ConsoleSaveFileConverter.h"
#include "ProgressListener.h"

void ConsoleSaveFileConverter::ProcessSimpleFile(ConsoleSaveFile *sourceSave, FileEntry *sourceFileEntry, ConsoleSaveFile *targetSave, FileEntry *targetFileEntry)
{
	DWORD numberOfBytesRead = 0;
	DWORD numberOfBytesWritten = 0;

	byte *data = new byte[sourceFileEntry->getFileSize()];

	// Read from source
	sourceSave->readFile(sourceFileEntry, data, sourceFileEntry->getFileSize(), &numberOfBytesRead);

	// Write back to target
	targetSave->writeFile(targetFileEntry, data, numberOfBytesRead, &numberOfBytesWritten);

	delete [] data;
}

void ConsoleSaveFileConverter::ProcessStandardRegionFile(ConsoleSaveFile *sourceSave, File sourceFile, ConsoleSaveFile *targetSave, File targetFile)
{
	DWORD numberOfBytesWritten = 0;
	DWORD numberOfBytesRead = 0;

	RegionFile sourceRegionFile(sourceSave, &sourceFile);
	RegionFile targetRegionFile(targetSave, &targetFile);

	for(unsigned int x = 0; x < 32; ++x)
	{
		for(unsigned int z = 0; z < 32; ++z)
		{
			DataInputStream *dis = sourceRegionFile.getChunkDataInputStream(x,z);

			if(dis)
			{
				int read = dis->read();
				DataOutputStream *dos = targetRegionFile.getChunkDataOutputStream(x,z);
				while(read != -1)
				{

					dos->write( read & 0xff );

					read = dis->read();
				}
				dos->close();
				dos->deleteChildStream();
				delete dos;
			}

			delete dis;
		}
	}
}

void ConsoleSaveFileConverter::ConvertSave(ConsoleSaveFile *sourceSave, ConsoleSaveFile *targetSave, ProgressListener *progress)
{
	// Process level.dat
	ConsoleSavePath ldatPath( wstring(L"level.dat") );
	FileEntry *sourceLdatFe = sourceSave->createFile( ldatPath );
	FileEntry *targetLdatFe = targetSave->createFile( ldatPath );
	app.DebugPrintf("Processing level.dat\n");
	ProcessSimpleFile(sourceSave, sourceLdatFe, targetSave, targetLdatFe);

	// Process game rules
	{
		ConsoleSavePath gameRulesPath( GAME_RULE_SAVENAME );
		if(sourceSave->doesFileExist(gameRulesPath) )
		{
			FileEntry *sourceFe = sourceSave->createFile( gameRulesPath );
			FileEntry *targetFe = targetSave->createFile( gameRulesPath );
			app.DebugPrintf("Processing game rules\n");
			ProcessSimpleFile(sourceSave, sourceFe, targetSave, targetFe);
		}
	}

	// MGH added - find any player data files and copy them across
#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	vector<FileEntry *>* playerFiles = sourceSave->getValidPlayerDatFiles();
#else
	vector<FileEntry *> *playerFiles = sourceSave->getFilesWithPrefix( DirectoryLevelStorage::getPlayerDir() );
#endif

	if(playerFiles != nullptr)
	{
		for(int fileIdx = 0; fileIdx < playerFiles->size();fileIdx++)
		{
			ConsoleSavePath sourcePlayerDatPath( playerFiles->at(fileIdx)->data.filename );
#ifdef _XBOX_ONE
			// 4J Stu - As the XUIDs on X360 and X1 are different, we don't want to transfer these over. However as the first player
			// file should be the owner of the save, we can move their data over to the current players XUID
			if(fileIdx > 0) break;
			PlayerUID xuid;
			ProfileManager.GetXUID(ProfileManager.GetPrimaryPad(), &xuid, false);
			ConsoleSavePath targetPlayerDatPath( L"players/" + xuid.toString() + L".dat" );
#else
			ConsoleSavePath targetPlayerDatPath( playerFiles->at(fileIdx)->data.filename );
#endif
			{
				FileEntry *sourceFe = sourceSave->createFile( sourcePlayerDatPath );
				FileEntry *targetFe = targetSave->createFile( targetPlayerDatPath );
				app.DebugPrintf("Processing player dat file %s\n", playerFiles->at(fileIdx)->data.filename);
				ProcessSimpleFile(sourceSave, sourceFe, targetSave, targetFe);

				targetFe->data.lastModifiedTime = sourceFe->data.lastModifiedTime;
			}
		}
		delete playerFiles;
	}


#ifdef SPLIT_SAVES
	int xzSize = LEVEL_LEGACY_WIDTH;
	int hellScale = HELL_LEVEL_LEGACY_SCALE;
	if ( sourceSave->doesFileExist( ldatPath ) ) 
	{
		ConsoleSaveFileInputStream fis = ConsoleSaveFileInputStream(sourceSave, ldatPath);
		CompoundTag *root = NbtIo::readCompressed(&fis);
		CompoundTag *tag = root->getCompound(L"Data");
		LevelData ret(tag);

		xzSize = ret.getXZSize();
		hellScale = ret.getHellScale();

		delete root;
	}

	RegionFileCache sourceCache;
	RegionFileCache targetCache;

	if(progress)
	{
#ifndef _WINDOWS64
		progress->progressStage(IDS_SAVETRANSFER_STAGE_CONVERTING);
#endif
	}

	// Overworld
	{
		app.DebugPrintf("Processing the overworld\n");
		int halfXZSize = xzSize / 2;

		int progressTarget = (xzSize) * (xzSize);
		int currentProgress = 0;
		if(progress) progress->progressStagePercentage((currentProgress*100)/progressTarget);

		for(int x = -halfXZSize; x < halfXZSize; ++x)
		{
			for(int z = -halfXZSize; z < halfXZSize; ++z)
			{
				//app.DebugPrintf("Processing overworld chunk %d,%d\n",x,z);
				DataInputStream *dis = sourceCache._getChunkDataInputStream(sourceSave,L"",x,z);

				if(dis)
				{
					int read = dis->read();
					DataOutputStream *dos = targetCache._getChunkDataOutputStream(targetSave,L"",x,z);
					BufferedOutputStream bos(dos, 1024 * 1024);
					while(read != -1)
					{

						bos.write( read & 0xff );

						read = dis->read();
					}
					bos.flush();
					dos->close();
					dos->deleteChildStream();
					delete dos;			
					dis->deleteChildStream();
					delete dis;
				}


				++currentProgress;
				if(progress) progress->progressStagePercentage( (currentProgress*100)/progressTarget);

			}
		}
	}

	// Nether
	{
		app.DebugPrintf("Processing the nether\n");
		int hellSize = xzSize / hellScale;
		int halfXZSize = hellSize / 2;

		int progressTarget = (hellSize) * (hellSize);
		int currentProgress = 0;
		if(progress) progress->progressStagePercentage((currentProgress*100)/progressTarget);

		for(int x = -halfXZSize; x < halfXZSize; ++x)
		{
			for(int z = -halfXZSize; z < halfXZSize; ++z)
			{
				//app.DebugPrintf("Processing nether chunk %d,%d\n",x,z);
				DataInputStream *dis = sourceCache._getChunkDataInputStream(sourceSave,L"DIM-1",x,z);

				if(dis)
				{
					int read = dis->read();
					DataOutputStream *dos = targetCache._getChunkDataOutputStream(targetSave,L"DIM-1",x,z);
					BufferedOutputStream bos(dos, 1024 * 1024);
					while(read != -1)
					{

						bos.write( read & 0xff );

						read = dis->read();
					}
					bos.flush();
					dos->close();
					dos->deleteChildStream();
					delete dos;
					dis->deleteChildStream();
					delete dis;
				}


				++currentProgress;
				if(progress) progress->progressStagePercentage((currentProgress*100)/progressTarget);
			}
		}
	}

	// End
	{
		app.DebugPrintf("Processing the end\n");
		int halfXZSize = END_LEVEL_MAX_WIDTH / 2;

		int progressTarget = (END_LEVEL_MAX_WIDTH) * (END_LEVEL_MAX_WIDTH);
		int currentProgress = 0;
		if(progress) progress->progressStagePercentage((currentProgress*100)/progressTarget);

		for(int x = -halfXZSize; x < halfXZSize; ++x)
		{
			for(int z = -halfXZSize; z < halfXZSize; ++z)
			{
				//app.DebugPrintf("Processing end chunk %d,%d\n",x,z);
				DataInputStream *dis = sourceCache._getChunkDataInputStream(sourceSave,L"DIM1/",x,z);

				if(dis)
				{
					int read = dis->read();
					DataOutputStream *dos = targetCache._getChunkDataOutputStream(targetSave,L"DIM1/",x,z);
					BufferedOutputStream bos(dos, 1024 * 1024);
					while(read != -1)
					{

						bos.write( read & 0xff );

						read = dis->read();
					}
					bos.flush();
					dos->close();
					dos->deleteChildStream();
					delete dos;
					dis->deleteChildStream();
					delete dis;
				}


				++currentProgress;
				if(progress) progress->progressStagePercentage((currentProgress*100)/progressTarget);
			}
		}
	}

#else
	// 4J Stu - Old version that just changes the compression of chunks, not usable for XboxOne style split saves or compressed tile formats
	// Process region files
	vector<FileEntry *> *allFilesInSave = sourceSave->getFilesWithPrefix(wstring(L""));
	if ( allFilesInSave )
	{
		for ( FileEntry* fe : *allFilesInSave )
		{
			if (fe != sourceLdatFe)
			{
				wstring fName(fe->data.filename);
				wstring suffix(L".mcr");
				if (fName.compare(fName.length() - suffix.length(), suffix.length(), suffix) == 0)
				{
#ifndef _CONTENT_PACKAGE
					wprintf(L"Processing a region file: %s\n", fe->data.filename);
#endif
					ProcessStandardRegionFile(sourceSave, File(fe->data.filename), targetSave, File(fe->data.filename));
				}
				else
				{
#ifndef _CONTENT_PACKAGE
					wprintf(L"%s is not a region file, ignoring\n", fe->data.filename);
#endif
				}
			}
		}
#endif
	}
}