#include "stdafx.h"
#include "DLCManager.h"
#include "DLCAudioFile.h"
#if defined _XBOX || defined _WINDOWS64
#include "..\..\Xbox\XML\ATGXmlParser.h"
#include "..\..\Xbox\XML\xmlFilesCallback.h"
#endif

DLCAudioFile::DLCAudioFile(const wstring &path) : DLCFile(DLCManager::e_DLCType_Audio,path)
{	
	m_pbData = NULL;
	m_dwBytes = 0;
}

void DLCAudioFile::addData(PBYTE pbData, DWORD dwBytes)
{
	m_pbData = pbData;
	m_dwBytes = dwBytes;

	processDLCDataFile(pbData,dwBytes);
}

PBYTE DLCAudioFile::getData(DWORD &dwBytes)
{
	dwBytes = m_dwBytes;
	return m_pbData;
}

WCHAR *DLCAudioFile::wchTypeNamesA[]=
{
	L"CUENAME",
	L"CREDIT",
};

DLCAudioFile::EAudioParameterType DLCAudioFile::getParameterType(const wstring &paramName)
{
	EAudioParameterType type = e_AudioParamType_Invalid;

	for(DWORD i = 0; i < e_AudioParamType_Max; ++i)
	{
		if(paramName.compare(wchTypeNamesA[i]) == 0)
		{
			type = (EAudioParameterType)i;
			break;
		}
	}

	return type;
}

void DLCAudioFile::addParameter(EAudioType type, EAudioParameterType ptype, const wstring &value)
{
	switch(ptype)
	{

		case e_AudioParamType_Credit: // If this parameter exists, then mark this as free
			//add it to the DLC credits list

			// we'll need to justify this text since we don't have a lot of room for lines of credits
			{
				// don't look for duplicate in the music credits

				//if(app.AlreadySeenCreditText(value)) break;

				int maximumChars = 55;

				bool bIsSDMode=!RenderManager.IsHiDef() && !RenderManager.IsWidescreen();

				if(bIsSDMode)
				{
					maximumChars = 45;
				}

				switch(XGetLanguage())
				{
				case XC_LANGUAGE_JAPANESE:
				case XC_LANGUAGE_TCHINESE:
				case XC_LANGUAGE_KOREAN:
					maximumChars = 35;
					break;
				}
				wstring creditValue = value;
				while (creditValue.length() > maximumChars)
				{
					unsigned int i = 1;
					while (i < creditValue.length() && (i + 1) <= maximumChars)
					{
						i++;
					}
					int iLast=(int)creditValue.find_last_of(L" ",i);
					switch(XGetLanguage())
					{
					case XC_LANGUAGE_JAPANESE:
					case XC_LANGUAGE_TCHINESE:
					case XC_LANGUAGE_KOREAN:
						iLast = maximumChars;
						break;
					default:
						iLast=(int)creditValue.find_last_of(L" ",i);
						break;
					}

					// if a space was found, include the space on this line
					if(iLast!=i)
					{
						iLast++;
					}

					app.AddCreditText((creditValue.substr(0, iLast)).c_str());
					creditValue = creditValue.substr(iLast);
				}
				app.AddCreditText(creditValue.c_str());

			}
			break;
		case e_AudioParamType_Cuename:
			m_parameters[type].push_back(value);
			//m_parameters[(int)type] = value;
			break;
	}
}

bool DLCAudioFile::processDLCDataFile(PBYTE pbData, DWORD dwLength)
{
	unordered_map<int, EAudioParameterType> parameterMapping;
	unsigned int uiCurrentByte=0;

	// File format defined in the AudioPacker
	// File format: Version 1

	unsigned int uiVersion=*(unsigned int *)pbData;
	uiCurrentByte+=sizeof(int);

	if(uiVersion < CURRENT_AUDIO_VERSION_NUM)
	{
		if(pbData!=NULL) delete [] pbData;
		app.DebugPrintf("DLC version of %d is too old to be read\n", uiVersion);
		return false;
	}
	
	unsigned int uiParameterTypeCount=*(unsigned int *)&pbData[uiCurrentByte];
	uiCurrentByte+=sizeof(int);
	C4JStorage::DLC_FILE_PARAM *pParams = (C4JStorage::DLC_FILE_PARAM *)&pbData[uiCurrentByte];
	
	for(unsigned int i=0;i<uiParameterTypeCount;i++)
	{
		// Map DLC strings to application strings, then store the DLC index mapping to application index
		wstring parameterName((WCHAR *)pParams->wchData);
		EAudioParameterType type = getParameterType(parameterName);
		if( type != e_AudioParamType_Invalid )
		{
			parameterMapping[pParams->dwType] = type;
		}
		uiCurrentByte+= sizeof(C4JStorage::DLC_FILE_PARAM)+(pParams->dwWchCount*sizeof(WCHAR));
		pParams = (C4JStorage::DLC_FILE_PARAM *)&pbData[uiCurrentByte];
	}
	unsigned int uiFileCount=*(unsigned int *)&pbData[uiCurrentByte];
	uiCurrentByte+=sizeof(int);
	C4JStorage::DLC_FILE_DETAILS *pFile = (C4JStorage::DLC_FILE_DETAILS *)&pbData[uiCurrentByte];

	DWORD dwTemp=uiCurrentByte;
	for(unsigned int i=0;i<uiFileCount;i++)
	{
		dwTemp+=sizeof(C4JStorage::DLC_FILE_DETAILS)+pFile->dwWchCount*sizeof(WCHAR);
		pFile = (C4JStorage::DLC_FILE_DETAILS *)&pbData[dwTemp];
	}
	PBYTE pbTemp=((PBYTE )pFile);
	pFile = (C4JStorage::DLC_FILE_DETAILS *)&pbData[uiCurrentByte];

	for(unsigned int i=0;i<uiFileCount;i++)
	{
		EAudioType type = (EAudioType)pFile->dwType;
		// Params
		unsigned int uiParameterCount=*(unsigned int *)pbTemp;
		pbTemp+=sizeof(int);
		pParams = (C4JStorage::DLC_FILE_PARAM *)pbTemp;
		for(unsigned int j=0;j<uiParameterCount;j++)
		{
			//EAudioParameterType paramType = e_AudioParamType_Invalid;

			AUTO_VAR(it, parameterMapping.find( pParams->dwType ));

			if(it != parameterMapping.end() )
			{
 				addParameter(type,(EAudioParameterType)pParams->dwType,(WCHAR *)pParams->wchData);
			}
			pbTemp+=sizeof(C4JStorage::DLC_FILE_PARAM)+(sizeof(WCHAR)*pParams->dwWchCount);
			pParams = (C4JStorage::DLC_FILE_PARAM *)pbTemp;
		}
		// Move the pointer to the start of the next files data;
		pbTemp+=pFile->uiFileSize;
		uiCurrentByte+=sizeof(C4JStorage::DLC_FILE_DETAILS)+pFile->dwWchCount*sizeof(WCHAR);

		pFile=(C4JStorage::DLC_FILE_DETAILS *)&pbData[uiCurrentByte];

	}

	return true;
}

int DLCAudioFile::GetCountofType(DLCAudioFile::EAudioType eType)
{
	return m_parameters[eType].size();
}


wstring &DLCAudioFile::GetSoundName(int iIndex)
{
	int iWorldType=e_AudioType_Overworld;
	while(iIndex>=m_parameters[iWorldType].size())
	{
		iIndex-=m_parameters[iWorldType].size();
		iWorldType++;
	}
	return m_parameters[iWorldType].at(iIndex);
}