#include "stdafx.h"
#include "UI.h"
#include "UIScene_AbstractContainerMenu.h"

#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\MultiplayerLocalPlayer.h"
#ifdef _WINDOWS64
#include "..\..\Windows64\KeyboardMouseInput.h"
#endif

UIScene_AbstractContainerMenu::UIScene_AbstractContainerMenu(int iPad, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	m_focusSection = eSectionNone;
	// in this scene, we override the press sound with our own for crafting success or fail
	ui.OverrideSFX(m_iPad,ACTION_MENU_A,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_OK,true);
#ifdef __ORBIS__
	ui.OverrideSFX(m_iPad,ACTION_MENU_TOUCHPAD_PRESS,true);
#endif
	ui.OverrideSFX(m_iPad,ACTION_MENU_X,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_Y,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT_SCROLL,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT_SCROLL,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_UP,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_DOWN,true);

	m_bIgnoreInput=false;
#ifdef _WINDOWS64
	m_bMouseDragSlider=false;
	m_bHasMousePosition = false;
	m_lastMouseX = 0;
	m_lastMouseY = 0;
#endif
}

UIScene_AbstractContainerMenu::~UIScene_AbstractContainerMenu()
{
	app.DebugPrintf("UIScene_AbstractContainerMenu::~UIScene_AbstractContainerMenu\n");
}

void UIScene_AbstractContainerMenu::handleDestroy()
{
	app.DebugPrintf("UIScene_AbstractContainerMenu::handleDestroy\n");
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[m_iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];
		if(gameMode != NULL) gameMode->getTutorial()->changeTutorialState(m_previousTutorialState);
	}

	// 4J Stu - Fix for #11302 - TCR 001: Network Connectivity: Host crashed after being killed by the client while accessing a chest during burst packet loss.
	// We need to make sure that we call closeContainer() anytime this menu is closed, even if it is forced to close by some other reason (like the player dying)	
	if(pMinecraft->localplayers[m_iPad] != NULL && pMinecraft->localplayers[m_iPad]->containerMenu->containerId == m_menu->containerId)
	{
		pMinecraft->localplayers[m_iPad]->closeContainer();
	}

	ui.OverrideSFX(m_iPad,ACTION_MENU_A,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_OK,false);
#ifdef __ORBIS__
	ui.OverrideSFX(m_iPad,ACTION_MENU_TOUCHPAD_PRESS,false);
#endif
	ui.OverrideSFX(m_iPad,ACTION_MENU_X,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_Y,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT_SCROLL,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT_SCROLL,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_UP,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_DOWN,false);
}

void UIScene_AbstractContainerMenu::InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex)
{
}

void UIScene_AbstractContainerMenu::PlatformInitialize(int iPad, int startIndex)
{

	m_labelInventory.init( app.GetString(IDS_INVENTORY) );

	if(startIndex >= 0)
	{
		m_slotListInventory.addSlots(startIndex, 27);
		m_slotListHotbar.addSlots(startIndex + 27, 9);
	}

	// Determine min and max extents for pointer, it needs to be able to move off the container to drop items.
	float fPanelWidth, fPanelHeight;
	float fPanelX, fPanelY;
	float fPointerWidth, fPointerHeight;

	// We may have varying depths of controls here, so base off the pointers parent
#if TO_BE_IMPLEMENTED
	HXUIOBJ parent;
	XuiElementGetBounds( m_pointerControl->m_hObj, &fPointerWidth, &fPointerHeight );	
#else
	fPointerWidth = 50;
	fPointerHeight = 50;
#endif

	fPanelWidth = m_controlBackgroundPanel.getWidth();
	fPanelHeight = m_controlBackgroundPanel.getHeight();
	fPanelX = m_controlBackgroundPanel.getXPos();
	fPanelY = m_controlBackgroundPanel.getYPos();
	// Get size of pointer
	m_fPointerImageOffsetX = 0; //floor(fPointerWidth/2.0f);
	m_fPointerImageOffsetY = 0; //floor(fPointerHeight/2.0f);
	
	m_fPanelMinX = fPanelX;
	m_fPanelMaxX = fPanelX + fPanelWidth;
	m_fPanelMinY = fPanelY;
	m_fPanelMaxY = fPanelY + fPanelHeight;

#ifdef __ORBIS__
	// we need to map the touchpad rectangle to the UI rectangle. While it works great for the creative menu, it is much too sensitive for the smaller menus.
	//X coordinate of the touch point (0 to 1919)    
	//Y coordinate of the touch point (0 to 941: DUALSHOCK�4 wireless controllers and the CUH-ZCT1J/CAP-ZCT1J/CAP-ZCT1U controllers for the PlayStation�4 development tool,    
	//0 to 753: JDX-1000x series controllers for the PlayStation�4 development tool,)     
	m_fTouchPadMulX=fPanelWidth/1919.0f;
	m_fTouchPadMulY=fPanelHeight/941.0f;
	m_fTouchPadDeadZoneX=15.0f*m_fTouchPadMulX;
	m_fTouchPadDeadZoneY=15.0f*m_fTouchPadMulY;

#endif

	// 4J-PB - need to limit this in splitscreen
	if(app.GetLocalPlayerCount()>1)
	{
		// don't let the pointer go into someone's screen
		m_fPointerMinY = floor(fPointerHeight/2.0f);
	}
	else
	{
		m_fPointerMinY = fPanelY -fPointerHeight;
	}
	m_fPointerMinX = fPanelX - fPointerWidth;
	m_fPointerMaxX = m_fPanelMaxX + fPointerWidth;
	m_fPointerMaxY = m_fPanelMaxY + (fPointerHeight/2);

// 	m_hPointerText=NULL;
// 	m_hPointerTextBkg=NULL;

	// Put the pointer over first item in use row to start with.
	UIVec2D itemPos;
	UIVec2D itemSize;
	GetItemScreenData( m_eCurrSection, 0, &( itemPos ), &( itemSize ) );

	UIVec2D sectionPos;
	GetPositionOfSection( m_eCurrSection, &( sectionPos ) );

	UIVec2D vPointerPos = sectionPos;
	vPointerPos += itemPos;
	vPointerPos.x += ( itemSize.x / 2.0f );
	vPointerPos.y += ( itemSize.y / 2.0f );

	vPointerPos.x -= m_fPointerImageOffsetX;
	vPointerPos.y -= m_fPointerImageOffsetY;

	//m_pointerControl->SetPosition( &vPointerPos );
	m_pointerPos = vPointerPos;

	IggyEvent mouseEvent;
	S32 width, height;
	m_parentLayer->getRenderDimensions(width, height);
	S32 x = m_pointerPos.x*((float)width/m_movieWidth);
	S32 y = m_pointerPos.y*((float)height/m_movieHeight); 
	IggyMakeEventMouseMove( &mouseEvent, x, y);

	IggyEventResult result;
	IggyPlayerDispatchEventRS ( getMovie() , &mouseEvent , &result );

#ifdef USE_POINTER_ACCEL	
	m_fPointerVelX = 0.0f;
	m_fPointerVelY = 0.0f;
	m_fPointerAccelX = 0.0f;
	m_fPointerAccelY = 0.0f;
#endif
}

void UIScene_AbstractContainerMenu::tick()
{
	UIScene::tick();

#ifdef _WINDOWS64
	bool mouseActive = (m_iPad == 0 && !KMInput.IsCaptured());
	bool drivePointerFromMouse = false;
	float rawMouseMovieX = 0, rawMouseMovieY = 0;
	int scrollDelta = 0;
	// Map Windows mouse position to the virtual pointer in movie coordinates
	if (mouseActive)
	{
		RECT clientRect;
		GetClientRect(KMInput.GetHWnd(), &clientRect);
		int clientWidth = clientRect.right;
		int clientHeight = clientRect.bottom;
		if (clientWidth > 0 && clientHeight > 0)
		{
			int mouseX = KMInput.GetMouseX();
			int mouseY = KMInput.GetMouseY();
			bool mouseMoved = !m_bHasMousePosition || mouseX != m_lastMouseX || mouseY != m_lastMouseY;

			m_bHasMousePosition = true;
			m_lastMouseX = mouseX;
			m_lastMouseY = mouseY;
			scrollDelta = KMInput.ConsumeScrollDelta();

			// Convert mouse position to movie coordinates using the movie/client ratio
			float mx = (float)mouseX * ((float)m_movieWidth / (float)clientWidth);
			float my = (float)mouseY * ((float)m_movieHeight / (float)clientHeight);

			rawMouseMovieX = mx;
			rawMouseMovieY = my;

			// Once the mouse has taken over the container cursor, keep following the OS cursor
			// until explicit controller input takes ownership back.
			drivePointerFromMouse = m_bPointerDrivenByMouse || mouseMoved || KMInput.IsMouseDown(0) || KMInput.IsMouseDown(1) || KMInput.IsMouseDown(2) || scrollDelta != 0;
			if (drivePointerFromMouse)
			{
				m_bPointerDrivenByMouse = true;
				m_eCurrTapState = eTapStateNoInput;
				m_pointerPos.x = mx;
				m_pointerPos.y = my;
			}
		}
	}
#endif

	onMouseTick();

#ifdef _WINDOWS64
	// Dispatch mouse clicks AFTER onMouseTick() has updated m_eCurrSection from the new pointer position
	if (mouseActive)
	{
		if (KMInput.ConsumeMousePress(0))
		{
			if (m_eCurrSection == eSectionInventoryCreativeSlider)
			{
				// Scrollbar click: use raw mouse position (onMouseTick may have snapped m_pointerPos)
				m_bMouseDragSlider = true;
				m_pointerPos.x = rawMouseMovieX;
				m_pointerPos.y = rawMouseMovieY;
				handleOtherClicked(m_iPad, eSectionInventoryCreativeSlider, 0, false);
			}
			else
			{
				handleKeyDown(m_iPad, ACTION_MENU_A, false);
			}
		}
		else if (m_bMouseDragSlider && KMInput.IsMouseDown(0))
		{
			// Continue scrollbar drag: update scroll position from current mouse Y
			m_pointerPos.x = rawMouseMovieX;
			m_pointerPos.y = rawMouseMovieY;
			handleOtherClicked(m_iPad, eSectionInventoryCreativeSlider, 0, false);
		}

		if (!KMInput.IsMouseDown(0))
			m_bMouseDragSlider = false;

		if (KMInput.ConsumeMousePress(1))
		{
			handleKeyDown(m_iPad, ACTION_MENU_X, false);
		}
		if (KMInput.ConsumeMousePress(2))
		{
			handleKeyDown(m_iPad, ACTION_MENU_Y, false);
		}

		// Mouse scroll wheel for page scrolling
		if (scrollDelta > 0)
		{
			handleKeyDown(m_iPad, ACTION_MENU_OTHER_STICK_UP, false);
		}
		else if (scrollDelta < 0)
		{
			handleKeyDown(m_iPad, ACTION_MENU_OTHER_STICK_DOWN, false);
		}

		// ESC to close — must be last since it may destroy this scene
		if (KMInput.ConsumeKeyPress(VK_ESCAPE))
		{
			handleKeyDown(m_iPad, ACTION_MENU_B, false);
			return;
		}
	}
#endif

	IggyEvent mouseEvent;
	S32 width, height;
	m_parentLayer->getRenderDimensions(width, height);

#ifdef _WINDOWS64
	S32 x, y;
	if (mouseActive && m_bPointerDrivenByMouse)
	{
		// Send raw mouse position directly as Iggy event to avoid coordinate round-trip errors
		// Scale mouse client coords to the Iggy display space (which was set to getRenderDimensions())
		RECT clientRect;
		GetClientRect(KMInput.GetHWnd(), &clientRect);
		x = (S32)((float)KMInput.GetMouseX() * ((float)width / (float)clientRect.right));
		y = (S32)((float)KMInput.GetMouseY() * ((float)height / (float)clientRect.bottom));
	}
	else
	{
		x = (S32)(m_pointerPos.x * ((float)width / m_movieWidth));
		y = (S32)(m_pointerPos.y * ((float)height / m_movieHeight));
	}
#else
	S32 x = m_pointerPos.x*((float)width/m_movieWidth);
	S32 y = m_pointerPos.y*((float)height/m_movieHeight);
#endif
	IggyMakeEventMouseMove( &mouseEvent, x, y);

	// 4J Stu - This seems to be broken on Durango, so do it ourself
#ifdef _DURANGO
	//mouseEvent.x = x;
	//mouseEvent.y = y;
#endif

	IggyEventResult result;
	IggyPlayerDispatchEventRS ( getMovie() , &mouseEvent , &result );
}

void UIScene_AbstractContainerMenu::render(S32 width, S32 height, C4JRender::eViewportType viewpBort)
{
	m_cacheSlotRenders = true;

	m_needsCacheRendered = m_needsCacheRendered || m_menu->needsRendered();

	if(m_needsCacheRendered)
	{
		m_expectedCachedSlotCount = GetBaseSlotCount();
		unsigned int count = m_menu->getSize();
		for(unsigned int i = 0; i < count; ++i)
		{
			if(m_menu->getSlot(i)->hasItem())
			{
				++m_expectedCachedSlotCount;
			}
		}
	}

	UIScene::render(width, height, viewpBort);

	m_needsCacheRendered = false;
}

void UIScene_AbstractContainerMenu::customDraw(IggyCustomDrawCallbackRegion *region)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->localplayers[m_iPad] == NULL || pMinecraft->localgameModes[m_iPad] == NULL) return;

	shared_ptr<ItemInstance> item = nullptr;
	int slotId = -1;
	if(wcscmp((wchar_t *)region->name,L"pointerIcon")==0)
	{		
		m_cacheSlotRenders = false;
		item = pMinecraft->localplayers[m_iPad]->inventory->getCarried();
	}
	else
	{
		swscanf((wchar_t*)region->name,L"slot_%d",&slotId);
		if (slotId == -1)
		{
			app.DebugPrintf("This is not the control we are looking for\n");
		}
		else
		{			
			m_cacheSlotRenders = true;
			Slot *slot = m_menu->getSlot(slotId);
			item = slot->getItem();
		}
	}

	if(item != NULL) customDrawSlotControl(region,m_iPad,item,m_menu->isValidIngredient(item, slotId)?1.0f:0.5f,item->isFoil(),true);
}

void UIScene_AbstractContainerMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	if(m_bIgnoreInput) return;

	//app.DebugPrintf("UIScene_InventoryMenu handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	if(pressed)
	{
		m_bPointerDrivenByMouse = false;
		handled = handleKeyDown(m_iPad, key, repeat);
	}
}

void UIScene_AbstractContainerMenu::SetPointerText(vector<HtmlString> *description, bool newSlot)
{
	m_cursorPath.setLabel(HtmlString::Compose(description), false, newSlot);
}

void UIScene_AbstractContainerMenu::setSectionFocus(ESceneSection eSection, int iPad)
{
	UIControl *newFocus = getSection(eSection);
	if(newFocus) newFocus->setFocus(true);

	if(m_focusSection != eSectionNone)
	{
		UIControl *currentFocus = getSection(m_focusSection);
		// 4J-TomK only set current focus to false if it differs from last (previously this continuously fired iggy functions when they were identical!
		if(currentFocus != newFocus)
			if(currentFocus) currentFocus->setFocus(false);
	}

	m_focusSection = eSection;
}

void UIScene_AbstractContainerMenu::setFocusToPointer(int iPad)
{
	if(m_focusSection != eSectionNone)
	{
		UIControl *currentFocus = getSection(m_focusSection);
		if(currentFocus) currentFocus->setFocus(false);
	}
	m_focusSection = eSectionNone;
}

shared_ptr<ItemInstance> UIScene_AbstractContainerMenu::getSlotItem(ESceneSection eSection, int iSlot)
{
	Slot *slot = m_menu->getSlot( getSectionStartOffset(eSection) + iSlot );
	if(slot) return slot->getItem();
	else return nullptr;
}

Slot *UIScene_AbstractContainerMenu::getSlot(ESceneSection eSection, int iSlot)
{
	Slot *slot = m_menu->getSlot( getSectionStartOffset(eSection) + iSlot );
	if(slot) return slot;
	else return NULL;
}

bool UIScene_AbstractContainerMenu::isSlotEmpty(ESceneSection eSection, int iSlot)
{
	Slot *slot = m_menu->getSlot( getSectionStartOffset(eSection) + iSlot );
	if(slot) return !slot->hasItem();
	else return false;
}

void UIScene_AbstractContainerMenu::adjustPointerForSafeZone()
{
	// Handled by AS
}
