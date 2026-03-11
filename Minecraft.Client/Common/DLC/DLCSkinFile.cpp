#include "stdafx.h"
#include "DLCManager.h"
#include "DLCSkinFile.h"
#include "..\..\ModelPart.h"
#include "..\..\EntityRenderer.h"
#include "..\..\EntityRenderDispatcher.h"
#include "..\..\..\Minecraft.World\Player.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"

DLCSkinFile::DLCSkinFile(const wstring &path) : DLCFile(DLCManager::e_DLCType_Skin,path)
{
	m_displayName = L"";
	m_themeName = L"";
	m_cape = L"";
	m_bIsFree = false;	
	m_uiAnimOverrideBitmask=0L;
}

void DLCSkinFile::addData(PBYTE pbData, DWORD dwBytes)
{
	app.AddMemoryTextureFile(m_path,pbData,dwBytes);
}

void DLCSkinFile::addParameter(DLCManager::EDLCParameterType type, const wstring &value)
{
	switch(type)
	{
	case DLCManager::e_DLCParamType_DisplayName:
		{
			// 4J Stu - In skin pack 2, the name for Zap is mis-spelt with two p's as Zapp
			// dlcskin00000109.png
			if( m_path.compare(L"dlcskin00000109.png") == 0)
			{
				m_displayName = L"Zap";
			}
			else
			{
				m_displayName = value;
			}
		}
		break;
	case DLCManager::e_DLCParamType_ThemeName:
		m_themeName = value;
		break;
	case DLCManager::e_DLCParamType_Free: // If this parameter exists, then mark this as free
		m_bIsFree = true;
		break;
	case DLCManager::e_DLCParamType_Credit: // If this parameter exists, then mark this as free
		 //add it to the DLC credits list

		// we'll need to justify this text since we don't have a lot of room for lines of credits
		{
			if(app.AlreadySeenCreditText(value)) break;
			// first add a blank string for spacing
			app.AddCreditText(L"");

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
				size_t iLast=creditValue.find_last_of(L" ", i);
				switch(XGetLanguage())
				{
				case XC_LANGUAGE_JAPANESE:
				case XC_LANGUAGE_TCHINESE:
				case XC_LANGUAGE_KOREAN:
					iLast = maximumChars;
					break;
				default:
					iLast=creditValue.find_last_of(L" ", i);
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
	case DLCManager::e_DLCParamType_Cape:
		m_cape = value;
		break;
	case DLCManager::e_DLCParamType_Box:
		{
			WCHAR wchBodyPart[10];
			SKIN_BOX *pSkinBox = new SKIN_BOX;
			ZeroMemory(pSkinBox,sizeof(SKIN_BOX));

#ifdef __PS3__
			// 4J Stu - The Xbox version used swscanf_s which isn't available in GCC.
			swscanf(value.c_str(), L"%10ls%f%f%f%f%f%f%f%f", wchBodyPart,
#else
			swscanf_s(value.c_str(), L"%9ls%f%f%f%f%f%f%f%f", wchBodyPart,10,
#endif
				&pSkinBox->fX,
				&pSkinBox->fY,
				&pSkinBox->fZ,
				&pSkinBox->fW,
				&pSkinBox->fH,
				&pSkinBox->fD,
				&pSkinBox->fU,
				&pSkinBox->fV);
 
			if(wcscmp(wchBodyPart,L"HEAD")==0)
			{
				pSkinBox->ePart=eBodyPart_Head;
			}
			else if(wcscmp(wchBodyPart,L"BODY")==0)
			{
				pSkinBox->ePart=eBodyPart_Body;
			}
			else if(wcscmp(wchBodyPart,L"ARM0")==0)
			{
				pSkinBox->ePart=eBodyPart_Arm0;
			}
			else if(wcscmp(wchBodyPart,L"ARM1")==0)
			{
				pSkinBox->ePart=eBodyPart_Arm1;
			}
			else if(wcscmp(wchBodyPart,L"LEG0")==0)
			{
				pSkinBox->ePart=eBodyPart_Leg0;
			}
			else if(wcscmp(wchBodyPart,L"LEG1")==0)
			{
				pSkinBox->ePart=eBodyPart_Leg1;
			}

			// add this to the skin's vector of parts
			m_AdditionalBoxes.push_back(pSkinBox);
		}
		break;
	case DLCManager::e_DLCParamType_Anim:
#ifdef __PS3__
		// 4J Stu - The Xbox version used swscanf_s which isn't available in GCC.
		swscanf(value.c_str(), L"%X", &m_uiAnimOverrideBitmask);
#else
		swscanf_s(value.c_str(), L"%X", &m_uiAnimOverrideBitmask,sizeof(unsigned int));
#endif
		DWORD skinId = app.getSkinIdFromPath(m_path);
		app.SetAnimOverrideBitmask(skinId, m_uiAnimOverrideBitmask);
		break;
	}
}

// vector<ModelPart *> *DLCSkinFile::getAdditionalModelParts()
// {
// 	return &m_AdditionalModelParts;
// }

int DLCSkinFile::getAdditionalBoxesCount()
{
	return static_cast<int>(m_AdditionalBoxes.size());
}
vector<SKIN_BOX *> *DLCSkinFile::getAdditionalBoxes()
{
	return &m_AdditionalBoxes;
}

wstring DLCSkinFile::getParameterAsString(DLCManager::EDLCParameterType type)
{
	switch(type)
	{
	case DLCManager::e_DLCParamType_DisplayName:
		return m_displayName;
	case DLCManager::e_DLCParamType_ThemeName:
		return m_themeName;
	case DLCManager::e_DLCParamType_Cape:
		return m_cape;
	default:
		return L"";
	}
}

bool DLCSkinFile::getParameterAsBool(DLCManager::EDLCParameterType type)
{
	switch(type)
	{
	case DLCManager::e_DLCParamType_Free:
		// Patch all DLC to be "paid"
		return false;
		// return m_bIsFree;
	default:
		return false;
	}
}
