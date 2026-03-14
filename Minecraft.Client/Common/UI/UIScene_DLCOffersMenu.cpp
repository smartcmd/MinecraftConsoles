#include "stdafx.h"
#include "UI.h"
#include "UIScene_DLCOffersMenu.h"
#include "../../../Minecraft.World/StringHelpers.h"
#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
#include "Common/Network/Sony/SonyHttp.h"
#endif

#ifdef __PSVITA__
#include "PSVita/Network/SonyCommerce_Vita.h"
#endif

#define PLAYER_ONLINE_TIMER_ID 0
#define PLAYER_ONLINE_TIMER_TIME 100

UIScene_DLCOffersMenu::UIScene_DLCOffersMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	m_bProductInfoShown=false;
	DLCOffersParam *param=static_cast<DLCOffersParam *>(initData);
	m_iProductInfoIndex=param->iType;
	m_iCurrentDLC=0;
	m_iTotalDLC=0;
#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
	m_pvProductInfo=nullptr;
#endif
	m_bAddAllDLCButtons=true;

	// Setup all the Iggy references we need for this scene
	initialiseMovie();
	// Alert the app the we want to be informed of ethernet connections
	app.SetLiveLinkRequired( true );

	m_bIsSD=!RenderManager.IsHiDef() && !RenderManager.IsWidescreen();

	m_labelOffers.init(app.GetString(IDS_DOWNLOADABLE_CONTENT_OFFERS));
	m_buttonListOffers.init(eControl_OffersList);
	m_labelHTMLSellText.init(L" ");
	m_labelPriceTag.init(L" ");
	TelemetryManager->RecordMenuShown(m_iPad, eUIScene_DLCOffersMenu, 0);

	m_bHasPurchased = false;
	m_bIsSelected = false;

	if(m_loadedResolution == eSceneResolution_1080)
	{
#ifdef _DURANGO
		m_labelXboxStore.init( app.GetString(IDS_XBOX_STORE) );
#else
		m_labelXboxStore.init( L"" );
#endif
	}

#ifdef _DURANGO
	m_pNoImageFor_DLC = nullptr;
	// If we don't yet have this DLC, we need to display a timer
	m_bDLCRequiredIsRetrieved=false;
	m_bIgnorePress=true;
	m_bSelectionChanged=true;
	// display a timer
	m_Timer.setVisible(true);

#endif

#ifdef __ORBIS__
	//sceNpCommerceShowPsStoreIcon(SCE_NP_COMMERCE_PS_STORE_ICON_CENTER);
#endif

#if ( defined __PS3__ || defined __ORBIS__ || defined __PSVITA__ )
	addTimer( PLAYER_ONLINE_TIMER_ID, PLAYER_ONLINE_TIMER_TIME );
#endif

#ifdef __PSVITA__
	ui.TouchBoxRebuild(this);
#endif
}

UIScene_DLCOffersMenu::~UIScene_DLCOffersMenu()
{
	// Alert the app the we no longer want to be informed of ethernet connections
	app.SetLiveLinkRequired( false );
}

void UIScene_DLCOffersMenu::handleTimerComplete(int id)
{
#if ( defined __PS3__ || defined __ORBIS__  || defined __PSVITA__)
	switch(id)
	{
	case PLAYER_ONLINE_TIMER_ID:
#ifndef _WINDOWS64
		if(ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad())==false)
		{
			// check the player hasn't gone offline
			// If they have, bring up the PSN warning and exit from the DLC menu
			unsigned int uiIDA[1];
			uiIDA[0]=IDS_OK;
			C4JStorage::EMessageResult result = ui.RequestErrorMessage( IDS_CONNECTION_LOST, g_NetworkManager.CorrectErrorIDS(IDS_CONNECTION_LOST_LIVE_NO_EXIT), uiIDA,1,ProfileManager.GetPrimaryPad(),UIScene_DLCOffersMenu::ExitDLCOffersMenu,this);
		}
#endif
		break;
	}
#endif
}

int UIScene_DLCOffersMenu::ExitDLCOffersMenu(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_DLCOffersMenu* pClass = static_cast<UIScene_DLCOffersMenu *>(pParam);

#if defined __ORBIS__ || defined __PSVITA__
	app.GetCommerce()->HidePsStoreIcon();
#endif
	ui.NavigateToHomeMenu();//iPad,eUIScene_MainMenu);

	return 0;
}

wstring UIScene_DLCOffersMenu::getMoviePath()
{
	return L"DLCOffersMenu";
}

void UIScene_DLCOffersMenu::updateTooltips()
{
	int iA = -1;
	if(m_bIsSelected)
	{
		if( !m_bHasPurchased )
		{
			iA = IDS_TOOLTIPS_INSTALL;
		}
		else
		{
			iA = IDS_TOOLTIPS_REINSTALL;
		}
	}
	ui.SetTooltips( m_iPad, iA,IDS_TOOLTIPS_BACK);
}

void UIScene_DLCOffersMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			navigateBack();
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		sendInputToMovie(key, repeat, pressed, released);
		break;
	case ACTION_MENU_UP:
		if(pressed)
		{
			// 4J - TomK don't proceed if there is no DLC to navigate through
			if(m_iTotalDLC > 0)
			{
				if(m_iCurrentDLC > 0)
					m_iCurrentDLC--;

				m_bProductInfoShown = false;
			}
		}
		sendInputToMovie(key, repeat, pressed, released);
		break;

	case ACTION_MENU_DOWN:
		if(pressed)
		{
			// 4J - TomK don't proceed if there is no DLC to navigate through
			if(m_iTotalDLC > 0)
			{
				if(m_iCurrentDLC < (m_iTotalDLC - 1))
					m_iCurrentDLC++;

				m_bProductInfoShown = false;
			}
		}
		sendInputToMovie(key, repeat, pressed, released);
		break;

	case ACTION_MENU_LEFT:
		/*
#ifdef _DEBUG
	static int iTextC=0;
	switch(iTextC)
	{
	case 0:
		m_labelHTMLSellText.init("Voici un fantastique mini-pack de 24 apparences pour personnaliser votre personnage Minecraft et vous mettre dans l'ambiance des f�tes de fin d'ann�e.<br><br>1-4 joueurs<br>2-8 joueurs en r�seau<br><br>  Cet article fait l�objet d�une licence ou d�une sous-licence de Sony Computer Entertainment America, et est soumis aux conditions g�n�rales du service du r�seau, au contrat d�utilisateur, aux restrictions d�utilisation de cet article et aux autres conditions applicables, disponibles sur le site www.us.playstation.com/support/useragreements. Si vous ne souhaitez pas accepter ces conditions, ne t�l�chargez pas ce produit. Cet article peut �tre utilis� avec un maximum de deux syst�mes PlayStation�3 activ�s associ�s � ce compte Sony Entertainment Network.�<br><br>'Minecraft' est une marque commerciale de Notch Development AB.");
		break;
	case 1:
		m_labelHTMLSellText.init("Un fabuloso minipack de 24 aspectos para personalizar tu personaje de Minecraft y ponerte a tono con las fiestas.<br><br>1-4 jugadores<br>2-8 jugadores en red<br><br>  Sony Computer Entertainment America le concede la licencia o sublicencia de este art�culo, que est� sujeto a los t�rminos de servicio y al acuerdo de usuario de la red. Las restricciones de uso de este art�culo, as� como otros t�rminos aplicables, se encuentran en www.us.playstation.com/support/useragreements. Si no desea aceptar todos estos t�rminos, no descargue este art�culo. Este art�culo puede usarse en hasta dos sistemas PlayStation�3 activados asociados con esta cuenta de Sony Entertainment Network.�<br><br>'Minecraft' es una marca comercial de Notch Development AB.");
		break;
	case 2:
		m_labelHTMLSellText.init("Este � um incr�vel pacote com 24 capas para personalizar seu personagem no Minecraft e entrar no clima de final de ano.<br><br>1-4 Jogadores<br>Jogadores em rede 2-8<br><br>  Este item est� sendo licenciado ou sublicenciado para voc� pela Sony Computer Entertainment America e est� sujeito aos Termos de Servi�o da Rede e Acordo do Usu�rio, as restri��es de uso deste item e outros termos aplic�veis est�o localizados em www.us.playstation.com/support/useragreements. Caso n�o queira aceitar todos esses termos, n�o baixe este item. Este item pode ser usado com at� 2 sistemas PlayStation�3 ativados associados a esta Conta de Rede Sony Entertainment.�<br><br>'Minecraft' � uma marca registrada da Notch Development AB");
		break;
	}
	iTextC++;
	if(iTextC>2) iTextC=0;
#endif
	*/
	case ACTION_MENU_RIGHT:
	case ACTION_MENU_OTHER_STICK_DOWN:
	case ACTION_MENU_OTHER_STICK_UP:
	// don't pass down PageUp or PageDown because this will cause conflicts between the buttonlist and scrollable html text component
	//case ACTION_MENU_PAGEUP:
	//case ACTION_MENU_PAGEDOWN:
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_DLCOffersMenu::handlePress(F64 controlId, F64 childId)
{
	switch(static_cast<int>(controlId))
	{
	case eControl_OffersList:
		{
#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
			// buy the DLC

			vector<SonyCommerce::ProductInfo >::iterator it = m_pvProductInfo->begin();
			string teststring;
			for(int i=0;i<childId;i++)
			{
				it++;
			}

			SonyCommerce::ProductInfo info = *it;

#ifdef __PS3__
			// is the item purchasable?
			if(info.purchasabilityFlag==1)
			{
				// can be bought
				app.Checkout(info.skuId);
			}
			else
			{
				if((info.annotation & (SCE_NP_COMMERCE2_SKU_ANN_PURCHASED_CANNOT_PURCHASE_AGAIN | SCE_NP_COMMERCE2_SKU_ANN_PURCHASED_CAN_PURCHASE_AGAIN))!=0)
				{
 					app.DownloadAlreadyPurchased(info.skuId);
				}
			}
#else // __ORBIS__
			// is the item purchasable?
			if(info.purchasabilityFlag==SCE_TOOLKIT_NP_COMMERCE_NOT_PURCHASED)
			{
				// can be bought
				app.Checkout(info.skuId);
			}
			else
			{
				app.DownloadAlreadyPurchased(info.skuId);
			}
#endif // __PS3__
#elif defined _XBOX_ONE
			int iIndex = (int)childId;
			StorageManager.InstallOffer(1,StorageManager.GetOffer(iIndex).wszProductID,nullptr,nullptr);
#else
			int iIndex = static_cast<int>(childId);

			ULONGLONG ullIndexA[1];
			ullIndexA[0]=StorageManager.GetOffer(iIndex).qwOfferID;
			StorageManager.InstallOffer(1,ullIndexA,nullptr,nullptr);
#endif
		}
		break;
	}
}

void UIScene_DLCOffersMenu::handleSelectionChanged(F64 selectedId)
{

}

void UIScene_DLCOffersMenu::handleFocusChange(F64 controlId, F64 childId)
{
	app.DebugPrintf("UIScene_DLCOffersMenu::handleFocusChange\n");

#ifdef __PSVITA__
	// set this here on Vita, in case we've came from a touch screen press. Fixes bug #5794
	if((int)controlId == eControl_OffersList)
	{
		m_bProductInfoShown = false;
		m_iCurrentDLC = (int)childId;
	}
#endif

#ifdef _DURANGO
	m_bSelectionChanged=true; // to tell the tick to update the display
	// 4J-PB can't call settexturename from a callback
	/*if(m_buttonListOffers.hasFocus() && (childId>-1))
	{
		int iIndex = (int)childId;
		MARKETPLACE_CONTENTOFFER_INFO xOffer = StorageManager.GetOffer(iIndex);
		UpdateDisplay(xOffer);
	}*/
#endif

#if defined __PSVITA__ || defined __ORBIS__
	if(m_pvProductInfo)
	{
		m_bIsSelected = true;
		vector<SonyCommerce::ProductInfo >::iterator it = m_pvProductInfo->begin();
		string teststring;
		for(int i=0;i<childId;i++)
		{
			it++;
		}

		SonyCommerce::ProductInfo info = *it;
		if(info.purchasabilityFlag==SCE_TOOLKIT_NP_COMMERCE_NOT_PURCHASED)
		{
			m_bHasPurchased=false;
		}
		else
		{
			m_bHasPurchased=true;
		}

		updateTooltips();
	}
#endif
}

void UIScene_DLCOffersMenu::tick()
{
	UIScene::tick();

#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)

	if(m_bAddAllDLCButtons)
	{
		// need to fill out all the dlc buttons

		if((m_bProductInfoShown==false) && app.GetCommerceProductListRetrieved() && app.GetCommerceProductListInfoRetrieved())
		{
			m_bAddAllDLCButtons=false;
			// add the categories to the list box
			if(m_pvProductInfo==nullptr)
			{
				m_pvProductInfo=app.GetProductList(m_iProductInfoIndex);
				if(m_pvProductInfo==nullptr)
				{
					m_iTotalDLC=0;
					// need to display text to say no downloadable content available yet
					m_labelOffers.setLabel(app.GetString(IDS_NO_DLCCATEGORIES));

					m_bProductInfoShown=true;
					return;
				}
				else m_iTotalDLC=m_pvProductInfo->size();
			}

			vector<SonyCommerce::ProductInfo >::iterator it = m_pvProductInfo->begin();
			string teststring;
			bool bFirstItemSet=false;
			for(int i=0;i<m_iTotalDLC;i++)
			{
				SonyCommerce::ProductInfo info = *it;

				if(strncmp(info.productName,"Minecraft ",10)==0)
				{
					teststring=&info.productName[10];

				}
				else
				{
					teststring=info.productName;
				}

				bool bDLCIsAvailable=false;

#ifdef __PS3__
				// is the item purchasable?
				if(info.purchasabilityFlag==1)
				{
					// can be bought
					app.DebugPrintf("Adding DLC (%s) - not bought\n",teststring.c_str());
					m_buttonListOffers.addItem(teststring,false,i);
					bDLCIsAvailable=true;
				}
				else
				{
					if((info.annotation & (SCE_NP_COMMERCE2_SKU_ANN_PURCHASED_CANNOT_PURCHASE_AGAIN | SCE_NP_COMMERCE2_SKU_ANN_PURCHASED_CAN_PURCHASE_AGAIN))!=0)
					{
						app.DebugPrintf("Adding DLC (%s) - bought\n",teststring.c_str());
						m_buttonListOffers.addItem(teststring,true,i);
						bDLCIsAvailable=true;
					}
				}
#else // __ORBIS__
				// is the item purchasable?
				if(info.purchasabilityFlag==SCE_TOOLKIT_NP_COMMERCE_NOT_PURCHASED)
				{
					// can be bought
					m_buttonListOffers.addItem(teststring,false,i);
					bDLCIsAvailable=true;
				}
				else
				{
					m_buttonListOffers.addItem(teststring,true,i);
					bDLCIsAvailable=true;
				}
#endif // __PS3__

				// set the other details for the first item
				if(bDLCIsAvailable && (bFirstItemSet==false))
				{
					bFirstItemSet=true;

					// 4J-PB - info.longDescription isn't null terminated
					char chLongDescription[SCE_NP_COMMERCE2_PRODUCT_LONG_DESCRIPTION_LEN+1];
					memcpy(chLongDescription,info.longDescription,SCE_NP_COMMERCE2_PRODUCT_LONG_DESCRIPTION_LEN);
					chLongDescription[SCE_NP_COMMERCE2_PRODUCT_LONG_DESCRIPTION_LEN]=0;
					m_labelHTMLSellText.setLabel(chLongDescription);

					if(info.ui32Price==0)
					{
						m_labelPriceTag.setLabel(app.GetString(IDS_DLC_PRICE_FREE));
					}
					else
					{
						teststring=info.price;
						m_labelPriceTag.setLabel(teststring);
					}

					// get the image - if we haven't already
					wstring textureName = filenametowstring(info.imageUrl);

					if(hasRegisteredSubstitutionTexture(textureName)==false)
					{
						PBYTE pbImageData;
						int iImageDataBytes=0;
						bool bDeleteData;
#ifdef __ORBIS__
						// check the local files first
						SONYDLC *pSONYDLCInfo=app.GetSONYDLCInfoFromKeyname(info.productId);

						// does the DLC info have an image?
						if(pSONYDLCInfo && pSONYDLCInfo->dwImageBytes!=0)
						{
							pbImageData=pSONYDLCInfo->pbImageData;
							iImageDataBytes=pSONYDLCInfo->dwImageBytes;
							bDeleteData=false; // we'll clean up the local LDC images
						}
						else
#endif
						if(info.imageUrl[0]!=0)
						{
							SonyHttp::getDataFromURL(info.imageUrl,(void **)&pbImageData,&iImageDataBytes);
							bDeleteData=true;
						}

						if(iImageDataBytes!=0)
						{
							// set the image
							registerSubstitutionTexture(textureName,pbImageData,iImageDataBytes,bDeleteData);
							m_bitmapIconOfferImage.setTextureName(textureName);
							// 4J Stu - Don't delete this
							//delete [] pbImageData;
						}
						else
						{
							m_bitmapIconOfferImage.setTextureName(L"");
						}
					}
					else
					{
						m_bitmapIconOfferImage.setTextureName(textureName);
					}
				}
				it++;
			}

			if(bFirstItemSet==false)
			{
				// we were not able to add any items to the list
				m_labelOffers.setLabel(app.GetString(IDS_NO_DLCCATEGORIES));
			}
			else
			{
				// set the focus to the first thing in the categories if there are any
				if(m_pvProductInfo->size()>0)
				{
					m_buttonListOffers.setFocus(true);
				}
				else
				{
					// need to display text to say no downloadable content available yet
					m_labelOffers.setLabel(app.GetString(IDS_NO_DLCCATEGORIES));
				}
			}

			m_Timer.setVisible(false);
			m_bProductInfoShown=true;
		}
	}
	else
	{
#ifdef __PSVITA__
		// MGH - fixes bug 5768 on Vita - should be extended properly to work for other platforms
		if((SonyCommerce_Vita::getPurchasabilityUpdated()) && app.GetCommerceProductListRetrieved()&& app.GetCommerceProductListInfoRetrieved() && m_iTotalDLC > 0)
		{

			{
				vector<SonyCommerce::ProductInfo >::iterator it = m_pvProductInfo->begin();
				for(int i=0;i<m_iTotalDLC;i++)
				{
					SonyCommerce::ProductInfo info = *it;
					// is the item purchasable?
					if(info.purchasabilityFlag==SCE_TOOLKIT_NP_COMMERCE_NOT_PURCHASED)
					{
						// can be bought
						m_buttonListOffers.showTick(i, false);
					}
					else
					{
						m_buttonListOffers.showTick(i, true);
					}
					it++;
				}
			}
		}
#endif


		// just update the details based on what the current selection is / TomK-4J - don't proceed if total DLC is 0 (bug 4757)
		if((m_bProductInfoShown==false) && app.GetCommerceProductListRetrieved()&& app.GetCommerceProductListInfoRetrieved() && m_iTotalDLC > 0)
		{


			vector<SonyCommerce::ProductInfo >::iterator it = m_pvProductInfo->begin();
			string teststring;
			for(int i=0;i<m_iCurrentDLC;i++)
			{
				it++;
			}

			SonyCommerce::ProductInfo info = *it;

			// 4J-PB - info.longDescription isn't null terminated
			char chLongDescription[SCE_NP_COMMERCE2_PRODUCT_LONG_DESCRIPTION_LEN+1];
			memcpy(chLongDescription,info.longDescription,SCE_NP_COMMERCE2_PRODUCT_LONG_DESCRIPTION_LEN);
			chLongDescription[SCE_NP_COMMERCE2_PRODUCT_LONG_DESCRIPTION_LEN]=0;
			m_labelHTMLSellText.setLabel(chLongDescription);

			if(info.ui32Price==0)
			{
				m_labelPriceTag.setLabel(app.GetString(IDS_DLC_PRICE_FREE));
			}
			else
			{
				teststring=info.price;
				m_labelPriceTag.setLabel(teststring);
			}

			// get the image

			// then retrieve from the web
			wstring textureName = filenametowstring(info.imageUrl);

			if(hasRegisteredSubstitutionTexture(textureName)==false)
			{
				PBYTE pbImageData;
				int iImageDataBytes=0;
				bool bDeleteData;
#ifdef __ORBIS__
				// check the local files first
				SONYDLC *pSONYDLCInfo=app.GetSONYDLCInfoFromKeyname(info.productId);

				// does the DLC info have an image?
				if(pSONYDLCInfo->dwImageBytes!=0)
				{
					pbImageData=pSONYDLCInfo->pbImageData;
					iImageDataBytes=pSONYDLCInfo->dwImageBytes;
					bDeleteData=false; // we'll clean up the local LDC images
				}
				else
#endif
				{
					SonyHttp::getDataFromURL(info.imageUrl,(void **)&pbImageData,&iImageDataBytes);
					bDeleteData=true;
				}

				if(iImageDataBytes!=0)
				{
					// set the image
					registerSubstitutionTexture(textureName,pbImageData,iImageDataBytes, bDeleteData);
					m_bitmapIconOfferImage.setTextureName(textureName);

					// 4J Stu - Don't delete this
					//delete [] pbImageData;
				}
				else
				{
					m_bitmapIconOfferImage.setTextureName(L"");
				}
			}
			else
			{
				m_bitmapIconOfferImage.setTextureName(textureName);
			}
			m_bProductInfoShown=true;
			m_Timer.setVisible(false);
		}

	}
#elif defined _XBOX_ONE
	if(m_bAddAllDLCButtons)
	{
		// Is the DLC we're looking for available?
		if(!m_bDLCRequiredIsRetrieved)
		{
			// DLCContentRetrieved is to see if the type of content has been retrieved - and on Durango there is only type 0 - XMARKETPLACE_OFFERING_TYPE_CONTENT
			if(app.DLCContentRetrieved(e_Marketplace_Content))
			{
				m_bDLCRequiredIsRetrieved=true;

				// Retrieve the info
				GetDLCInfo(app.GetDLCOffersCount(), false);
				m_bIgnorePress=false;
				m_bAddAllDLCButtons=false;

				// hide the timer
				m_Timer.setVisible(false);
			}
		}
	}

	// have to wait until we have the offers
	if(m_bSelectionChanged && m_bDLCRequiredIsRetrieved)
	{
		// need to update text and icon
		if(m_buttonListOffers.hasFocus() && (getControlChildFocus()>-1))
		{
			int iIndex = getControlChildFocus();
			MARKETPLACE_CONTENTOFFER_INFO xOffer = StorageManager.GetOffer(iIndex);

			if (!ui.UsingBitmapFont()) // 4J-JEV: Replace characters we don't have.
			{
				for (int i=0; xOffer.wszCurrencyPrice[i]!=0; i++)
				{
					WCHAR *c = &xOffer.wszCurrencyPrice[i];
					if (*c == L'\u20A9')		*c = L'\uFFE6'; // Korean Won.
					else if (*c == L'\u00A5')	*c = L'\uFFE5'; // Japanese Yen.
				}
			}

			if(UpdateDisplay(xOffer))
			{
				// image was available
				m_bSelectionChanged=false;
			}
		}
	}
#endif
}

#if defined _XBOX_ONE
void UIScene_DLCOffersMenu::GetDLCInfo( int iOfferC, bool bUpdateOnly )
{
	MARKETPLACE_CONTENTOFFER_INFO xOffer;
	int iCount=0;
	bool bNoDLCToDisplay = true;
	unsigned int uiDLCCount=0;


	if(bUpdateOnly) // Just update the info on the current list
	{

	}
	else
	{
		// clear out the list
		m_buttonListOffers.clearList();

		// need to reorder the DLC display according to dlc uiSortIndex
		SORTINDEXSTRUCT *OrderA = new SORTINDEXSTRUCT [iOfferC];

		for(int i = 0; i < iOfferC; i++)
		{
			xOffer = StorageManager.GetOffer(i);
			// Check that this is in the list of known DLC
			DLC_INFO *pDLC=app.GetDLCInfoForFullOfferID(xOffer.wszProductID);

			if(pDLC!=nullptr)
			{
				OrderA[uiDLCCount].uiContentIndex=i;
				OrderA[uiDLCCount++].uiSortIndex=pDLC->uiSortIndex;
			}
			else
			{
				app.DebugPrintf("Unknown offer - %ls\n",xOffer.wszOfferName);
			}
		}

		qsort( OrderA, uiDLCCount, sizeof(SORTINDEXSTRUCT), OrderSortFunction );

		for(int i = 0; i < uiDLCCount; i++)
		{
			xOffer = StorageManager.GetOffer(OrderA[i].uiContentIndex);

			// Check that this is in the list of known DLC
			DLC_INFO *pDLC=app.GetDLCInfoForFullOfferID(xOffer.wszProductID);

			if(pDLC==nullptr)
			{
				// skip this one
				app.DebugPrintf("Unknown offer - %ls\n",xOffer.wszOfferName);
				continue;
			}

			if(pDLC->eDLCType==(eDLCContentType)m_iProductInfoIndex)
			{
				wstring wstrTemp=xOffer.wszOfferName;

				// 4J-PB - Rog requested we remove the Minecraft at the start of the name. It's required for the Bing search, but gets in the way here
				app.DebugPrintf("Adding %ls at %d\n",wstrTemp.c_str(), i);

				if(wcsncmp(L"Minecraft ",wstrTemp.c_str(),10)==0)
				{
					app.DebugPrintf("Removing Minecraft from name\n");
					WCHAR *pwchNewName=(WCHAR *)wstrTemp.c_str();
					wstrTemp=&pwchNewName[10];
				}

#ifdef _XBOX_ONE
				// 4J-PB - the hasPurchased comes from the local installed package info
				// find the DLC in the installed packages
				XCONTENT_DATA *pContentData=StorageManager.GetInstalledDLC(xOffer.wszProductID);

				if(pContentData!=nullptr)
				{
					m_buttonListOffers.addItem(wstrTemp,!pContentData->bTrialLicense,OrderA[i].uiContentIndex);
				}
				else
				{
					m_buttonListOffers.addItem(wstrTemp,false,OrderA[i].uiContentIndex);
				}
#else
				m_buttonListOffers.addItem(wstrTemp,xOffer.fUserHasPurchased,OrderA[i].uiContentIndex);
#endif

				// add the required image to the retrieval queue
				m_vIconRetrieval.push_back(pDLC->wchBanner);

				/** 4J JEV:
					* We've filtered results out from the list, need to keep track
					* of the 'actual' list index.
					*/
				iCount++;
			}
		}


		// Check if there is nothing to display, and display the default "nothing available at this time"
		if(iCount>0)
		{
			bNoDLCToDisplay=false;
			xOffer = StorageManager.GetOffer(OrderA[0].uiContentIndex);
			//m_buttonListOffers.setCurrentSelection(0);

			UpdateDisplay(xOffer);
		}
		delete OrderA;
	}

	// turn off the timer display
	//m_Timer.SetShow(FALSE);
	if(bNoDLCToDisplay)
	{
		// set the default text

		wchar_t formatting[40];
		wstring wstrTemp = app.GetString(IDS_NO_DLCOFFERS);
// 		swprintf(formatting, 40, L"<font size=\"%d\">", m_bIsSD?12:14);
// 		wstrTemp = formatting + wstrTemp;

		m_labelHTMLSellText.setLabel(wstrTemp);
		m_labelPriceTag.setVisible(false);
	}
}

int UIScene_DLCOffersMenu::OrderSortFunction(const void* a, const void* b)
{
	return ((SORTINDEXSTRUCT*)b)->uiSortIndex - ((SORTINDEXSTRUCT*)a)->uiSortIndex;
}

void UIScene_DLCOffersMenu::UpdateTooltips(MARKETPLACE_CONTENTOFFER_INFO& xOffer)
{
	m_bHasPurchased = xOffer.fUserHasPurchased;
	m_bIsSelected = true;
	updateTooltips();
}

bool UIScene_DLCOffersMenu::UpdateDisplay(MARKETPLACE_CONTENTOFFER_INFO& xOffer)
{
	bool bImageAvailable=false;
#ifdef _XBOX_ONE
	DLC_INFO *dlc = app.GetDLCInfoForFullOfferID(xOffer.wszProductID);
#else
	DLC_INFO *dlc = app.GetDLCInfoForFullOfferID(xOffer.wszOfferName);
#endif

	if (dlc != nullptr)
	{
		WCHAR *cString = dlc->wchBanner;


		// is the file in the local DLC images?
		// is the file in the TMS XZP?
		//int iIndex = app.GetLocalTMSFileIndex(cString, true);

		if(dlc->dwImageBytes!=0)
		{
			//app.LoadLocalTMSFile(cString);

			// set the image - no delete
			registerSubstitutionTexture(cString,dlc->pbImageData,dlc->dwImageBytes,false);
			m_bitmapIconOfferImage.setTextureName(cString);
			bImageAvailable=true;
		}
		else
		{
			bool bPresent = app.IsFileInMemoryTextures(cString);
			if (!bPresent)
			{
				// Image has not come in yet
				// Set the item monitored in the timer, so we can set the image when it comes in
				m_pNoImageFor_DLC=dlc;

				app.AddTMSPPFileTypeRequest(dlc->eDLCType,true);
				bImageAvailable=false;
				//m_bitmapIconOfferImage.setTextureName(L"");
			}
			else
			{
				if(hasRegisteredSubstitutionTexture(cString)==false)
				{
					BYTE *pData=nullptr;
					DWORD dwSize=0;
					app.GetMemFileDetails(cString,&pData,&dwSize);
					// set the image
#ifdef _XBOX_ONE
					registerSubstitutionTexture(cString,pData,dwSize);
#else
					registerSubstitutionTexture(cString,pData,dwSize,true);
#endif
					m_bitmapIconOfferImage.setTextureName(cString);
				}
				else
				{
					m_bitmapIconOfferImage.setTextureName(cString);
				}
				bImageAvailable=true;
			}
		}

		m_labelHTMLSellText.setLabel(xOffer.wszSellText);

		// set the price info
		m_labelPriceTag.setVisible(true);
		m_labelPriceTag.setLabel(xOffer.wszCurrencyPrice);

		UpdateTooltips(xOffer);
	}
	else
	{
		wchar_t formatting[40];
		wstring wstrTemp = app.GetString(IDS_NO_DLCOFFERS);
		m_labelHTMLSellText.setLabel(wstrTemp.c_str());
		m_labelPriceTag.setVisible(false);
	}

	return bImageAvailable;
}
#endif

#ifdef _XBOX_ONE
void UIScene_DLCOffersMenu::HandleDLCLicenseChange()
{
	// flag an update of the display
	int iOfferC=app.GetDLCOffersCount();

	GetDLCInfo(iOfferC,false);
}
#endif // _XBOX_ONE

#ifdef __PS3__
void UIScene_DLCOffersMenu::HandleDLCInstalled()
{
	app.DebugPrintf(4,"UIScene_DLCOffersMenu::HandleDLCInstalled\n");

// 	m_buttonListOffers.clearList();
// 	m_bAddAllDLCButtons=true;
// 	m_bProductInfoShown=false;
}

// void UIScene_DLCOffersMenu::HandleDLCMountingComplete()
// {
//	app.DebugPrintf(4,"UIScene_SkinSelectMenu::HandleDLCMountingComplete\n");
//}


#endif