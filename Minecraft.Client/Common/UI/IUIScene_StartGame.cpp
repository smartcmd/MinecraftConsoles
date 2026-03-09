#include "stdafx.h"
#include "UI.h"
#include "TexturePack.h"
#include "TexturePackRepository.h"
#include "Minecraft.h"
#include "IUIScene_StartGame.h"

IUIScene_StartGame::IUIScene_StartGame(int iPad, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	m_bIgnoreInput = false;
	m_iTexturePacksNotInstalled=0;
	m_texturePackDescDisplayed = false;
	m_bShowTexturePackDescription = false;
	m_iSetTexturePackDescription = -1;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	m_currentTexturePackIndex = pMinecraft->skins->getTexturePackIndex(0);
}

void IUIScene_StartGame::HandleDLCMountingComplete()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	// clear out the current texture pack list
	m_texturePackList.clearSlots();

	int texturePacksCount = pMinecraft->skins->getTexturePackCount();

	for(unsigned int i = 0; i < texturePacksCount; ++i)
	{
		TexturePack *tp = pMinecraft->skins->getTexturePackByIndex(i);

		DWORD dwImageBytes;
		PBYTE pbImageData = tp->getPackIcon(dwImageBytes);

		if(dwImageBytes > 0 && pbImageData)
		{
			wchar_t imageName[64];
			swprintf(imageName,64,L"tpack%08x",tp->getId());
			registerSubstitutionTexture(imageName, pbImageData, dwImageBytes);
			m_texturePackList.addPack(i,imageName);
		}
	}

	m_iTexturePacksNotInstalled=0;

	// 4J-PB - there may be texture packs we don't have, so use the info from TMS for this
	// REMOVE UNTIL WORKING
	DLC_INFO *pDLCInfo=nullptr;

	// first pass - look to see if there are any that are not in the list
	bool bTexturePackAlreadyListed;
	bool bNeedToGetTPD=false;

	for(unsigned int i = 0; i < app.GetDLCInfoTexturesOffersCount(); ++i)
	{
		bTexturePackAlreadyListed=false;
#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
		char *pchName=app.GetDLCInfoTextures(i);
		pDLCInfo=app.GetDLCInfo(pchName);
#elif defined _XBOX_ONE
		pDLCInfo=app.GetDLCInfoForFullOfferID((WCHAR *)app.GetDLCInfoTexturesFullOffer(i).c_str());
#else
		ULONGLONG ull=app.GetDLCInfoTexturesFullOffer(i);
		pDLCInfo=app.GetDLCInfoForFullOfferID(ull);
#endif
		for(unsigned int i = 0; i < texturePacksCount; ++i)
		{
			TexturePack *tp = pMinecraft->skins->getTexturePackByIndex(i);
			if(pDLCInfo->iConfig==tp->getDLCParentPackId())
			{
				bTexturePackAlreadyListed=true;
			}
		}
		if(bTexturePackAlreadyListed==false)
		{
			// some missing
			bNeedToGetTPD=true;

			m_iTexturePacksNotInstalled++;
		}
	}

#if TO_BE_IMPLEMENTED
	if(bNeedToGetTPD==true)
	{
		// add a TMS request for them
		app.DebugPrintf("+++ Adding TMSPP request for texture pack data\n");
		app.AddTMSPPFileTypeRequest(e_DLC_TexturePackData);
		if(m_iConfigA!=nullptr)
		{
			delete m_iConfigA;
		}
		m_iConfigA= new int [m_iTexturePacksNotInstalled];
		m_iTexturePacksNotInstalled=0;

		for(unsigned int i = 0; i < app.GetDLCInfoTexturesOffersCount(); ++i)
		{
			bTexturePackAlreadyListed=false;
			ULONGLONG ull=app.GetDLCInfoTexturesFullOffer(i);
			pDLCInfo=app.GetDLCInfoForFullOfferID(ull);
			for(unsigned int i = 0; i < texturePacksCount; ++i)
			{
				TexturePack *tp = pMinecraft->skins->getTexturePackByIndex(i);
				if(pDLCInfo->iConfig==tp->getDLCParentPackId())
				{
					bTexturePackAlreadyListed=true;
				}
			}
			if(bTexturePackAlreadyListed==false)
			{
				m_iConfigA[m_iTexturePacksNotInstalled++]=pDLCInfo->iConfig;
			}
		}
	}
#endif
	m_currentTexturePackIndex = pMinecraft->skins->getTexturePackIndex(0);
	UpdateTexturePackDescription(m_currentTexturePackIndex);

	m_texturePackList.selectSlot(m_currentTexturePackIndex);
	m_bIgnoreInput=false;
	app.m_dlcManager.checkForCorruptDLCAndAlert();
}

void IUIScene_StartGame::handleSelectionChanged(F64 selectedId)
{
	m_iSetTexturePackDescription = static_cast<int>(selectedId);

	if(!m_texturePackDescDisplayed)
	{	
		m_bShowTexturePackDescription = true;
	}
}

void IUIScene_StartGame::UpdateTexturePackDescription(int index)
{
	TexturePack *tp = Minecraft::GetInstance()->skins->getTexturePackByIndex(index);

	if(tp==nullptr)
	{
#if TO_BE_IMPLEMENTED		
		// this is probably a texture pack icon added from TMS

		DWORD dwBytes=0,dwFileBytes=0;
		PBYTE pbData=nullptr,pbFileData=nullptr;

		CXuiCtrl4JList::LIST_ITEM_INFO ListItem;
		// get the current index of the list, and then get the data
		ListItem=m_pTexturePacksList->GetData(index);

		app.GetTPD(ListItem.iData,&pbData,&dwBytes);

		app.GetFileFromTPD(eTPDFileType_Loc,pbData,dwBytes,&pbFileData,&dwFileBytes );
		if(dwFileBytes > 0 && pbFileData)
		{
			StringTable *pStringTable = new StringTable(pbFileData, dwFileBytes);
			m_texturePackTitle.SetText(pStringTable->getString(L"IDS_DISPLAY_NAME"));
			m_texturePackDescription.SetText(pStringTable->getString(L"IDS_TP_DESCRIPTION"));
		}

		app.GetFileFromTPD(eTPDFileType_Icon,pbData,dwBytes,&pbFileData,&dwFileBytes );
		if(dwFileBytes >= 0 && pbFileData)
		{
			XuiCreateTextureBrushFromMemory(pbFileData,dwFileBytes,&m_hTexturePackIconBrush);
			m_texturePackIcon->UseBrush(m_hTexturePackIconBrush);
		}
		app.GetFileFromTPD(eTPDFileType_Comparison,pbData,dwBytes,&pbFileData,&dwFileBytes );
		if(dwFileBytes >= 0 && pbFileData)
		{
			XuiCreateTextureBrushFromMemory(pbFileData,dwFileBytes,&m_hTexturePackComparisonBrush);
			m_texturePackComparison->UseBrush(m_hTexturePackComparisonBrush);
		}
		else
		{
			m_texturePackComparison->UseBrush(nullptr);
		}
#endif
	}
	else
	{
		m_labelTexturePackName.setLabel(tp->getName());
		m_labelTexturePackDescription.setLabel(tp->getDesc1());

		DWORD dwImageBytes;
		PBYTE pbImageData = tp->getPackIcon(dwImageBytes);

		//if(dwImageBytes > 0 && pbImageData)
		//{
		//	registerSubstitutionTexture(L"texturePackIcon", pbImageData, dwImageBytes);
		//	m_bitmapTexturePackIcon.setTextureName(L"texturePackIcon");
		//}

		wchar_t imageName[64];
		swprintf(imageName,64,L"tpack%08x",tp->getId());
		m_bitmapTexturePackIcon.setTextureName(imageName);

		pbImageData = tp->getPackComparison(dwImageBytes);

		if(dwImageBytes > 0 && pbImageData)
		{
			swprintf(imageName,64,L"texturePackComparison%08x",tp->getId());
			registerSubstitutionTexture(imageName, pbImageData, dwImageBytes);
			m_bitmapComparison.setTextureName(imageName);
		}
		else
		{
			m_bitmapComparison.setTextureName(L"");
		}
	}
}

void IUIScene_StartGame::UpdateCurrentTexturePack(int iSlot)
{
	m_currentTexturePackIndex = iSlot;
	TexturePack *tp = Minecraft::GetInstance()->skins->getTexturePackByIndex(m_currentTexturePackIndex);

	// if the texture pack is null, you don't have it yet
	if(tp==nullptr)
	{
#if TO_BE_IMPLEMENTED
		// Upsell

		CXuiCtrl4JList::LIST_ITEM_INFO ListItem;
		// get the current index of the list, and then get the data
		ListItem=m_pTexturePacksList->GetData(m_currentTexturePackIndex);


		// upsell the texture pack
		// tell sentient about the upsell of the full version of the skin pack
		ULONGLONG ullOfferID_Full;
		app.GetDLCFullOfferIDForPackID(ListItem.iData,&ullOfferID_Full);

		TelemetryManager->RecordUpsellPresented(ProfileManager.GetPrimaryPad(), eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);

		UINT uiIDA[3];

		uiIDA[0]=IDS_TEXTUREPACK_FULLVERSION;
		uiIDA[1]=IDS_TEXTURE_PACK_TRIALVERSION;
		uiIDA[2]=IDS_CONFIRM_CANCEL;


		// Give the player a warning about the texture pack missing
		ui.RequestErrorMessage(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 3, ProfileManager.GetPrimaryPad(),&:TexturePackDialogReturned,this);

		// do set the texture pack id, and on the user pressing create world, check they have it	
		m_MoreOptionsParams.dwTexturePack = ListItem.iData;
		return ;
#endif
	}
	else
	{
		m_MoreOptionsParams.dwTexturePack = tp->getId();
	}
}

int IUIScene_StartGame::TrialTexturePackWarningReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	IUIScene_StartGame* pScene = static_cast<IUIScene_StartGame *>(pParam);

	if(result==C4JStorage::EMessage_ResultAccept)
	{
		pScene->checkStateAndStartGame();
	}
	else
	{
		pScene->m_bIgnoreInput=false;
	}
	return 0;
}

int IUIScene_StartGame::UnlockTexturePackReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	IUIScene_StartGame* pScene = static_cast<IUIScene_StartGame *>(pParam);

	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(ProfileManager.IsSignedIn(iPad))
		{	
#if defined _XBOX //|| defined _XBOX_ONE			
			ULONGLONG ullIndexA[1];
			DLC_INFO *pDLCInfo = app.GetDLCInfoForTrialOfferID(pScene->m_pDLCPack->getPurchaseOfferId());

			if(pDLCInfo!=nullptr)
			{
				ullIndexA[0]=pDLCInfo->ullOfferID_Full;
			}
			else
			{
				ullIndexA[0]=pScene->m_pDLCPack->getPurchaseOfferId();
			}


			StorageManager.InstallOffer(1,ullIndexA,nullptr,nullptr);
#elif defined _XBOX_ONE
			//StorageManager.InstallOffer(1,StorageManager.GetOffer(iIndex).wszProductID,nullptr,nullptr);
#endif

			// the license change coming in when the offer has been installed will cause this scene to refresh	
		}
	}
	else
	{
#if defined _XBOX
		TelemetryManager->RecordUpsellResponded(iPad, eSet_UpsellID_Texture_DLC, ( pScene->m_pDLCPack->getPurchaseOfferId() & 0xFFFFFFFF ), eSen_UpsellOutcome_Declined);
#endif
	}

	pScene->m_bIgnoreInput = false;

	return 0;
}

int IUIScene_StartGame::TexturePackDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	IUIScene_StartGame *pClass = static_cast<IUIScene_StartGame *>(pParam);


#ifdef _XBOX
	// Exit with or without saving
	// Decline means install full version of the texture pack in this dialog
	if(result==C4JStorage::EMessage_ResultDecline || result==C4JStorage::EMessage_ResultAccept) 
	{
		// we need to enable background downloading for the DLC
		XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);

		ULONGLONG ullOfferID_Full;
		ULONGLONG ullIndexA[1];
		CXuiCtrl4JList::LIST_ITEM_INFO ListItem;
		// get the current index of the list, and then get the data
		ListItem=pClass->m_pTexturePacksList->GetData(pClass->m_currentTexturePackIndex);
		app.GetDLCFullOfferIDForPackID(ListItem.iData,&ullOfferID_Full);

		if( result==C4JStorage::EMessage_ResultAccept ) // Full version
		{
			ullIndexA[0]=ullOfferID_Full;
			StorageManager.InstallOffer(1,ullIndexA,nullptr,nullptr);

		}
		else // trial version
		{
			// if there is no trial version, this is a Cancel
			DLC_INFO *pDLCInfo=app.GetDLCInfoForFullOfferID(ullOfferID_Full);
			if(pDLCInfo->ullOfferID_Trial!=0LL)
			{

				ullIndexA[0]=pDLCInfo->ullOfferID_Trial;
				StorageManager.InstallOffer(1,ullIndexA,nullptr,nullptr);
			}
		}		
	}
#elif defined _XBOX_ONE
	// Get the product id from the texture pack id
	if(result==C4JStorage::EMessage_ResultAccept) 
	{

		if(ProfileManager.IsSignedIn(iPad))
		{	
			if (ProfileManager.IsSignedInLive(iPad))
			{
				wstring ProductId;
				app.GetDLCFullOfferIDForPackID(pClass->m_MoreOptionsParams.dwTexturePack,ProductId);


				StorageManager.InstallOffer(1,(WCHAR *)ProductId.c_str(),nullptr,nullptr);

				// the license change coming in when the offer has been installed will cause this scene to refresh	
			}
			else
			{	
				// 4J-JEV: Fix for XB1: #165863 - XR-074: Compliance: With no active network connection user is unable to convert from Trial to Full texture pack and is not messaged why.
				UINT uiIDA[1] = { IDS_CONFIRM_OK };
				ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1, iPad); 
			}
		}
	}

#endif
	pClass->m_bIgnoreInput=false;
	return 0;
}