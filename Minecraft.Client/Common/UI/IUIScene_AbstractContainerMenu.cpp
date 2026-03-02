#include "stdafx.h"

#include "IUIScene_AbstractContainerMenu.h"

#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.crafting.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\..\MultiplayerLocalPlayer.h"
#include "..\..\Minecraft.h"

#ifdef __ORBIS__
#include <pad.h>
#endif

IUIScene_AbstractContainerMenu::IUIScene_AbstractContainerMenu()
{
	m_menu = NULL;
	m_autoDeleteMenu = false;
	m_lastPointerLabelSlot = NULL;

	m_pointerPos.x = 0.0f;
	m_pointerPos.y = 0.0f;
	m_bPointerDrivenByMouse = false;

}

IUIScene_AbstractContainerMenu::~IUIScene_AbstractContainerMenu()
{
	// Delete associated menu if we were requested to on initialisation. Most menus are
	// created just before calling CXuiSceneAbstractContainer::Initialize, but the player's inventorymenu
	// is also passed directly and we don't want to go deleting that
	if( m_autoDeleteMenu ) delete m_menu;
}

void IUIScene_AbstractContainerMenu::Initialize(int iPad, AbstractContainerMenu* menu, bool autoDeleteMenu, int startIndex,ESceneSection firstSection,ESceneSection maxSection, bool bNavigateBack)
{
	assert( menu != NULL );

	m_menu = menu;
	m_autoDeleteMenu = autoDeleteMenu;

	Minecraft::GetInstance()->localplayers[iPad]->containerMenu = menu;

	// 4J WESTY - New tool tips to support pointer prototype.
	//UpdateTooltips();
	// Default tooltips.
	for ( int i = 0; i < eToolTipNumButtons; ++i )
	{
		m_aeToolTipSettings[ i ] = eToolTipNone;
	}
	// 4J-PB - don't set the eToolTipPickupPlace_OLD here - let the timer do it.
	/*SetToolTip( eToolTipButtonA, eToolTipPickupPlace_OLD );*/
	SetToolTip( eToolTipButtonB, eToolTipExit );
	SetToolTip( eToolTipButtonA, eToolTipNone );
	SetToolTip( eToolTipButtonX, eToolTipNone );
	SetToolTip( eToolTipButtonY, eToolTipNone );

	// 4J WESTY : To indicate if pointer has left menu window area.
	m_bPointerOutsideMenu = false;

	// 4J Stu - Store the enum range for the current scene
	m_eFirstSection = firstSection;
	m_eMaxSection = maxSection;

	m_iConsectiveInputTicks = 0;

	m_bNavigateBack = bNavigateBack;

	// Put the pointer over first item in use row to start with.
#ifdef TAP_DETECTION
	m_eCurrSection = firstSection;
	m_eCurrTapState = eTapStateNoInput;
	m_iCurrSlotX = 0;
	m_iCurrSlotY = 0;
#endif // TAP_DETECTION
	//
	// 	for(int i=0;i<XUSER_MAX_COUNT;i++)
	// 	{
	// 		m_bFirstTouchStored[i]=false;
	// 	}

#ifdef __ORBIS__
	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		m_bFirstTouchStored[i]=false;
	}
#endif

	PlatformInitialize(iPad,startIndex);
}

int	IUIScene_AbstractContainerMenu::GetSectionDimensions( ESceneSection eSection, int* piNumColumns, int* piNumRows )
{
	if(IsSectionSlotList(eSection))
	{
		*piNumRows = getSectionRows( eSection );
		*piNumColumns = getSectionColumns( eSection );
	}
	else
	{
		*piNumRows = 0;
		*piNumColumns = 0;
	}
	return( ( *piNumRows ) * ( *piNumColumns ) );
}

void IUIScene_AbstractContainerMenu::updateSlotPosition( ESceneSection eSection, ESceneSection newSection, ETapState eTapDirection, int *piTargetX, int *piTargetY, int xOffset, int yOffset  )
{
	// Update the target slot based on the size of the current section
	int columns, rows;

	// The return value of this function is unused, but the output params are required.
	//int iItemsNum = GetSectionDimensions( newSection, &columns, &rows );
	GetSectionDimensions( newSection, &columns, &rows );

	if( newSection != eSection )
	{
		// Update Y
		if(eTapDirection == eTapStateUp)
		{
			(*piTargetY) = rows - 1;
		}
		else if(eTapDirection == eTapStateDown)
		{
			(*piTargetY) = 0;
		}
		int offsetY = (*piTargetY) - yOffset;
		if( offsetY < 0 )
		{
			(*piTargetY) = 0;
		}
		else if(offsetY >= rows)
		{
			(*piTargetY) = rows - 1;
		}
		else
		{
			(*piTargetY) = offsetY;
		}

		// Update X
		int offsetX = (*piTargetX) - xOffset;
		if( offsetX < 0 )
		{
			*piTargetX = 0;
		}
		else if (offsetX >= columns)
		{
			*piTargetX = columns - 1;
		}
		else
		{
			*piTargetX = offsetX;
		}
	}
	else
	{
		// Update X
		int offsetX = (*piTargetX) - xOffset;
		if( offsetX < 0 )
		{
			*piTargetX = columns - 1;
		}
		else if (offsetX >= columns)
		{
			*piTargetX = 0;
		}
		else
		{
			*piTargetX = offsetX;
		}
	}
}

#ifdef TAP_DETECTION
IUIScene_AbstractContainerMenu::ETapState IUIScene_AbstractContainerMenu::GetTapInputType( float fInputX, float fInputY )
{
	if ( ( fabs( fInputX ) < 0.3f ) && ( fabs( fInputY ) < 0.3f ) )
	{
		return eTapStateNoInput;
	}
	else if ( ( fInputX < -0.3f ) && ( fabs( fInputY ) < 0.3f ) )
	{
		return eTapStateLeft;
	}
	else if ( ( fInputX > 0.3f ) && ( fabs( fInputY ) < 0.3f ) )
	{
		return eTapStateRight;
	}
	else if ( ( fInputY < -0.3f ) && ( fabs( fInputX ) < 0.3f ) )
	{
		return eTapStateDown;
	}
	else if ( ( fInputY > 0.3f ) && ( fabs( fInputX ) < 0.3f ) )
	{
		return eTapStateUp;
	}
	else
	{
		return eTapNone;
	}
}
#endif // TAP_DETECTION

void IUIScene_AbstractContainerMenu::SetToolTip( EToolTipButton eButton, EToolTipItem eItem )
{
	if ( m_aeToolTipSettings[ eButton ] != eItem )
	{
		m_aeToolTipSettings[ eButton ] = eItem;
		UpdateTooltips();
	}
}

void IUIScene_AbstractContainerMenu::UpdateTooltips()
{
	// Table gives us text id for tooltip.
	static const DWORD kaToolTipextIds[ eNumToolTips ] =
	{
		IDS_TOOLTIPS_PICKUPPLACE,			//eToolTipPickupPlace_OLD
		IDS_TOOLTIPS_EXIT,					// eToolTipExit
		IDS_TOOLTIPS_PICKUP_GENERIC,		// eToolTipPickUpGeneric
		IDS_TOOLTIPS_PICKUP_ALL,			// eToolTipPickUpAll
		IDS_TOOLTIPS_PICKUP_HALF,			// eToolTipPickUpHalf
		IDS_TOOLTIPS_PLACE_GENERIC,			// eToolTipPlaceGeneric
		IDS_TOOLTIPS_PLACE_ONE,				// eToolTipPlaceOne
		IDS_TOOLTIPS_PLACE_ALL,				// eToolTipPlaceAll
		IDS_TOOLTIPS_DROP_GENERIC,			// eToolTipDropGeneric
		IDS_TOOLTIPS_DROP_ONE,				// eToolTipDropOne
		IDS_TOOLTIPS_DROP_ALL,				// eToolTipDropAll
		IDS_TOOLTIPS_SWAP,					// eToolTipSwap
		IDS_TOOLTIPS_QUICK_MOVE,			// eToolTipQuickMove
		IDS_TOOLTIPS_QUICK_MOVE_INGREDIENT,	// eToolTipQuickMoveIngredient
		IDS_TOOLTIPS_QUICK_MOVE_FUEL,		// eToolTipQuickMoveTool
		IDS_TOOLTIPS_WHAT_IS_THIS,			// eToolTipWhatIsThis
		IDS_TOOLTIPS_EQUIP,					// eToolTipEquip
		IDS_TOOLTIPS_CLEAR_QUICK_SELECT,	// eToolTipClearQuickSelect
		IDS_TOOLTIPS_QUICK_MOVE_TOOL,		// eToolTipQuickMoveTool
		IDS_TOOLTIPS_QUICK_MOVE_ARMOR,		// eToolTipQuickMoveTool
		IDS_TOOLTIPS_QUICK_MOVE_WEAPON,		// eToolTipQuickMoveTool
		IDS_TOOLTIPS_DYE,					// eToolTipDye
		IDS_TOOLTIPS_REPAIR,				// eToolTipRepair
	};

	BYTE focusUser = getPad();

	for ( int i = 0; i < eToolTipNumButtons; ++i )
	{
		if ( m_aeToolTipSettings[ i ] == eToolTipNone )
		{
			ui.ShowTooltip( focusUser, i, FALSE );
		}
		else
		{
			ui.SetTooltipText( focusUser, i, kaToolTipextIds[ m_aeToolTipSettings[ i ] ] );
			ui.ShowTooltip( focusUser, i, TRUE );
		}
	}
}

void IUIScene_AbstractContainerMenu::onMouseTick()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[getPad()] != NULL)
	{
		Tutorial *tutorial = pMinecraft->localgameModes[getPad()]->getTutorial();
		if(tutorial != NULL)
		{
			if(ui.IsTutorialVisible(getPad()) && !tutorial->isInputAllowed(ACTION_MENU_UP))
			{
				return;
			}
		}
	}

	// Offset to display carried item attached to pointer.
	// 	static const float kfCarriedItemOffsetX = -5.0f;
	// 	static const float kfCarriedItemOffsetY = -5.0f;
	float fInputDirX=0.0f;
	float fInputDirY=0.0f;

	// Get current pointer position.
	UIVec2D vPointerPos = m_pointerPos;

	// Offset to image centre.
	vPointerPos.x += m_fPointerImageOffsetX;
	vPointerPos.y += m_fPointerImageOffsetY;

	// Get stick input.
	int iPad = getPad();

	bool bStickInput = false;
	float fInputX = InputManager.GetJoypadStick_LX( iPad, false )*((float)app.GetGameSettings(iPad,eGameSetting_Sensitivity_InMenu)/100.0f); // apply the sensitivity
	float fInputY = InputManager.GetJoypadStick_LY( iPad, false )*((float)app.GetGameSettings(iPad,eGameSetting_Sensitivity_InMenu)/100.0f); // apply the sensitivity

#ifdef __ORBIS__
	// should have sensitivity for the touchpad
	//(float)app.GetGameSettings(iPad,eGameSetting_Sensitivity_TouchPadInMenu)/100.0f

	// get the touchpad input and treat it as a map to the window
	ScePadTouchData *pTouchPadData=InputManager.GetTouchPadData(iPad);

	// make sure the touchpad button isn't down (it's the pausemenu)

	if((!InputManager.ButtonDown(iPad, ACTION_MENU_TOUCHPAD_PRESS)) && (pTouchPadData->touchNum>0))
	{
		if(m_bFirstTouchStored[iPad]==false)
		{
			m_oldvTouchPos.x=(float)pTouchPadData->touch[0].x;
			m_oldvTouchPos.y=(float)pTouchPadData->touch[0].y;
			m_oldvPointerPos.x=vPointerPos.x;
			m_oldvPointerPos.y=vPointerPos.y;
			m_bFirstTouchStored[iPad]=true;
		}

		// should take the average of multiple touch points

		float fNewX=(((float)pTouchPadData->touch[0].x)-m_oldvTouchPos.x) * m_fTouchPadMulX;
		float fNewY=(((float)pTouchPadData->touch[0].y)-m_oldvTouchPos.y) * m_fTouchPadMulY;
		// relative positions - needs a deadzone

		if(fNewX>m_fTouchPadDeadZoneX)
		{
			vPointerPos.x=m_oldvPointerPos.x+((fNewX-m_fTouchPadDeadZoneX)*((float)app.GetGameSettings(iPad,eGameSetting_Sensitivity_InMenu)/100.0f));
		}
		else if(fNewX<-m_fTouchPadDeadZoneX)
		{
			vPointerPos.x=m_oldvPointerPos.x+((fNewX+m_fTouchPadDeadZoneX)*((float)app.GetGameSettings(iPad,eGameSetting_Sensitivity_InMenu)/100.0f));
		}

		if(fNewY>m_fTouchPadDeadZoneY)
		{
			vPointerPos.y=m_oldvPointerPos.y+((fNewY-m_fTouchPadDeadZoneY)*((float)app.GetGameSettings(iPad,eGameSetting_Sensitivity_InMenu)/100.0f));
		}
		else if(fNewY<-m_fTouchPadDeadZoneY)
		{
			vPointerPos.y=m_oldvPointerPos.y+((fNewY+m_fTouchPadDeadZoneY)*((float)app.GetGameSettings(iPad,eGameSetting_Sensitivity_InMenu)/100.0f));
		}

		// Clamp to pointer extents.
		if ( vPointerPos.x < m_fPointerMinX )				vPointerPos.x = m_fPointerMinX;
		else if ( vPointerPos.x > m_fPointerMaxX )			vPointerPos.x = m_fPointerMaxX;
		if ( vPointerPos.y < m_fPointerMinY )				vPointerPos.y = m_fPointerMinY;
		else if ( vPointerPos.y > m_fPointerMaxY )			vPointerPos.y = m_fPointerMaxY;

		bStickInput = true;
		m_eCurrTapState=eTapStateNoInput;
	}
	else
	{
		// reset the touch flag
		m_bFirstTouchStored[iPad]=false;
	
#endif



	// If there is any input on sticks, move the pointer.
	if ( ( fabs( fInputX ) >= 0.01f ) || ( fabs( fInputY ) >= 0.01f ) )
	{
		m_bPointerDrivenByMouse = false;
		fInputDirX = ( fInputX > 0.0f ) ? 1.0f : ( fInputX < 0.0f )?-1.0f : 0.0f;
		fInputDirY = ( fInputY > 0.0f ) ? 1.0f : ( fInputY < 0.0f )?-1.0f : 0.0f;

#ifdef TAP_DETECTION
		// Check for potential tap input to jump slot.
		ETapState eNewTapInput = GetTapInputType( fInputX, fInputY );

		switch( m_eCurrTapState )
		{
		case eTapStateNoInput:
			m_eCurrTapState = eNewTapInput;
			break;

		case eTapStateUp:
		case eTapStateDown:
		case eTapStateLeft:
		case eTapStateRight:
			if ( ( eNewTapInput != m_eCurrTapState ) && ( eNewTapInput != eTapStateNoInput ) )
			{
				// Input is no longer suitable for tap.
				m_eCurrTapState = eTapNone;
			}
			break;

		case eTapNone:
			/// Nothing to do, input is not a tap.
			break;
		}
#endif // TAP_DETECTION

		// Square it so we get more precision for small inputs.
		fInputX = fInputX * fInputX * fInputDirX * POINTER_SPEED_FACTOR;
		fInputY = fInputY * fInputY * fInputDirY * POINTER_SPEED_FACTOR;
		//fInputX = fInputX * POINTER_SPEED_FACTOR;
		//fInputY = fInputY * POINTER_SPEED_FACTOR;
		float fInputScale = 1.0f;

		// Ramp up input from zero when new input is recieved over INPUT_TICKS_FOR_SCALING ticks. This is to try to improve tapping stick to move 1 box.
		if ( m_iConsectiveInputTicks < MAX_INPUT_TICKS_FOR_SCALING )
		{
			++m_iConsectiveInputTicks;
			fInputScale = ( (float)( m_iConsectiveInputTicks) / (float)(MAX_INPUT_TICKS_FOR_SCALING) );
		}
#ifdef TAP_DETECTION
		else if ( m_iConsectiveInputTicks < MAX_INPUT_TICKS_FOR_TAPPING )
		{
			++m_iConsectiveInputTicks;
		}
		else
		{
			m_eCurrTapState = eTapNone;
		}
#endif
		// 4J Stu - The cursor moves too fast in SD mode
		// The SD/splitscreen scenes are approximately 0.6 times the size of the fullscreen on
		if(!RenderManager.IsHiDef() || app.GetLocalPlayerCount() > 1) fInputScale *= 0.6f;

		fInputX *= fInputScale;
		fInputY *= fInputScale;

#ifdef USE_POINTER_ACCEL		
		m_fPointerAccelX += fInputX / 50.0f;
		m_fPointerAccelY += fInputY / 50.0f;

		if ( fabsf( fInputX ) > fabsf( m_fPointerVelX + m_fPointerAccelX ) )
		{
			m_fPointerVelX += m_fPointerAccelX;
		}
		else
		{
			m_fPointerAccelX = fInputX - m_fPointerVelX;
			m_fPointerVelX = fInputX;
		}

		if ( fabsf( fInputY ) > fabsf( m_fPointerVelY + m_fPointerAccelY ) )
		{
			m_fPointerVelY += m_fPointerAccelY;
		}
		else
		{
			m_fPointerAccelY = fInputY - m_fPointerVelY;
			m_fPointerVelY = fInputY;
		}
		//printf( "IN %.2f  VEL %.2f  ACC %.2f\n", fInputY, m_fPointerVelY, m_fPointerAccelY );

		vPointerPos.x += m_fPointerVelX;
		vPointerPos.y -= m_fPointerVelY;
#else
		// Add input to pointer position.
		vPointerPos.x += fInputX;
		vPointerPos.y -= fInputY;
#endif
		// Clamp to pointer extents.
		if ( vPointerPos.x < m_fPointerMinX )				vPointerPos.x = m_fPointerMinX;
		else if ( vPointerPos.x > m_fPointerMaxX )		vPointerPos.x = m_fPointerMaxX;
		if ( vPointerPos.y < m_fPointerMinY )				vPointerPos.y = m_fPointerMinY;
		else if ( vPointerPos.y > m_fPointerMaxY )		vPointerPos.y = m_fPointerMaxY;

		bStickInput = true;
	}
	else
	{
		m_iConsectiveInputTicks = 0;
#ifdef USE_POINTER_ACCEL	
		m_fPointerVelX = 0.0f;
		m_fPointerVelY = 0.0f;
		m_fPointerAccelX = 0.0f;
		m_fPointerAccelY = 0.0f;
#endif		
	}

#ifdef __ORBIS__
	}
#endif

	// Determine which slot the pointer is currently over.
	ESceneSection eSectionUnderPointer = eSectionNone;
	int iNewSlotX = -1;
	int iNewSlotY = -1;
	int iNewSlotIndex = -1;
	bool bPointerIsOverSlot = false;

	// Centre position of item under pointer, use this to snap pointer to item.
	D3DXVECTOR3 vSnapPos;

	for ( int iSection = m_eFirstSection; iSection < m_eMaxSection; ++iSection )
	{
		// Do not check any further if we have already found the item under the pointer.
		if(m_eCurrTapState == eTapStateJump)
		{
			eSectionUnderPointer = m_eCurrSection;
		}
		else if ( eSectionUnderPointer == eSectionNone )
		{
			ESceneSection eSection = ( ESceneSection )( iSection );

			// Get position of this section.
			UIVec2D sectionPos;
			GetPositionOfSection( eSection, &( sectionPos ) );

			if(!IsSectionSlotList(eSection))
			{
				UIVec2D itemPos;
				UIVec2D itemSize;
				GetItemScreenData( eSection, 0, &( itemPos ), &( itemSize ) );

				UIVec2D itemMax = itemSize;
				itemMax += itemPos;

				if ( ( vPointerPos.x >= sectionPos.x ) && ( vPointerPos.x <= itemMax.x ) &&
					( vPointerPos.y >= sectionPos.y ) && ( vPointerPos.y <= itemMax.y ) )
				{
					// Pointer is over this control!
					eSectionUnderPointer = eSection;

					vSnapPos.x = itemPos.x + ( itemSize.x / 2.0f );
					vSnapPos.y = itemPos.y + ( itemSize.y / 2.0f );

					// Does this section already have focus.
					if ( !doesSectionTreeHaveFocus( eSection ) )
					{
						// Give focus to this section.
						setSectionFocus(eSection, getPad());
					}

					bPointerIsOverSlot = false;

					// Have we actually changed slot? If so, input cannot be a tap.
					if ( ( eSectionUnderPointer != m_eCurrSection ) || ( iNewSlotX != m_iCurrSlotX ) || ( iNewSlotY != m_iCurrSlotY ) )
					{
						m_eCurrTapState = eTapNone;
					}

					// Store what is currently under the pointer.
					m_eCurrSection = eSectionUnderPointer;
				}
			}
			else
			{

				// Get dimensions of this section.
				int iNumRows;
				int iNumColumns;
				int iNumItems = GetSectionDimensions( eSection, &( iNumColumns ), &( iNumRows ) );

				// Check each item to see if pointer is over it.
				for ( int iItem = 0; iItem < iNumItems; ++iItem )
				{
					UIVec2D itemPos;
					UIVec2D itemSize;
					GetItemScreenData( eSection, iItem, &( itemPos ), &( itemSize ) );

					itemPos += sectionPos;

					UIVec2D itemMax = itemSize;
					itemMax += itemPos;

					if ( ( vPointerPos.x >= itemPos.x ) && ( vPointerPos.x <= itemMax.x ) &&
						( vPointerPos.y >= itemPos.y ) && ( vPointerPos.y <= itemMax.y ) )
					{
						// Pointer is over this slot!
						eSectionUnderPointer = eSection;
						iNewSlotIndex = iItem;
						iNewSlotX = iNewSlotIndex % iNumColumns;
						iNewSlotY = iNewSlotIndex / iNumColumns;

						vSnapPos.x = itemPos.x + ( itemSize.x / 2.0f );
						vSnapPos.y = itemPos.y + ( itemSize.y / 2.0f );

						// Does this section already have focus.
						if ( !doesSectionTreeHaveFocus( eSection ) )
						{
							// Give focus to this section.
							setSectionFocus(eSection, getPad());
						}

						// Set the highlight marker.
						setSectionSelectedSlot(eSection, iNewSlotX, iNewSlotY );

						bPointerIsOverSlot = true;

#ifdef TAP_DETECTION
						// Have we actually changed slot? If so, input cannot be a tap.
						if ( ( eSectionUnderPointer != m_eCurrSection ) || ( iNewSlotX != m_iCurrSlotX ) || ( iNewSlotY != m_iCurrSlotY ) )
						{
							m_eCurrTapState = eTapNone;
						}

						// Store what is currently under the pointer.
						m_eCurrSection = eSectionUnderPointer;
						m_iCurrSlotX = iNewSlotX;
						m_iCurrSlotY = iNewSlotY;
#endif // TAP_DETECTION
						// No need to check any further slots, the pointer can only ever be over one.
						break;
					}
				}
			}
		}
	}

	// 4J - TomK - set to section none if this is a non-visible section
	if(!IsVisible(eSectionUnderPointer)) eSectionUnderPointer = eSectionNone;

	// If we are not over any slot, set focus elsewhere.
	if ( eSectionUnderPointer == eSectionNone )
	{
		setFocusToPointer( getPad() );
#ifdef TAP_DETECTION
		// Input cannot be a tap.
		m_eCurrTapState = eTapNone;

		// Store what is currently under the pointer.
		m_eCurrSection = eSectionNone;
		m_iCurrSlotX = -1;
		m_iCurrSlotY = -1;
#endif // TAP_DETECTION
	}
	else
	{
		if ( !bStickInput )
		{
			// Did we get a tap input?
			int iDesiredSlotX = -1;
			int iDesiredSlotY = -1;

			switch( m_eCurrTapState )
			{
			case eTapStateUp:
				iDesiredSlotX = m_iCurrSlotX;
				iDesiredSlotY = m_iCurrSlotY - 1;
				break;
			case eTapStateDown:
				iDesiredSlotX = m_iCurrSlotX;
				iDesiredSlotY = m_iCurrSlotY + 1;
				break;
			case eTapStateLeft:
				iDesiredSlotX = m_iCurrSlotX - 1;
				iDesiredSlotY = m_iCurrSlotY;
				break;
			case eTapStateRight:
				iDesiredSlotX = m_iCurrSlotX + 1;
				iDesiredSlotY = m_iCurrSlotY;
				break;
			case eTapStateJump:
				iDesiredSlotX = m_iCurrSlotX;
				iDesiredSlotY = m_iCurrSlotY;
				break;
			}

			int iNumRows;
			int iNumColumns;
			int iNumItems = GetSectionDimensions( eSectionUnderPointer, &( iNumColumns ), &( iNumRows ) );


			if ( (m_eCurrTapState != eTapNone && m_eCurrTapState != eTapStateNoInput) &&
				( !IsSectionSlotList(eSectionUnderPointer) ||
				( ( iDesiredSlotX < 0 ) || ( iDesiredSlotX >= iNumColumns ) || ( iDesiredSlotY < 0 ) || ( iDesiredSlotY >= iNumRows ) )
				))
			{

				eSectionUnderPointer = GetSectionAndSlotInDirection( eSectionUnderPointer, m_eCurrTapState, &iDesiredSlotX, &iDesiredSlotY );

				if(!IsSectionSlotList(eSectionUnderPointer)) bPointerIsOverSlot = false;

				// Get the details for the new section
				iNumItems = GetSectionDimensions( eSectionUnderPointer, &( iNumColumns ), &( iNumRows ) );
			}

			if ( !IsSectionSlotList(eSectionUnderPointer) || ( ( iDesiredSlotX >= 0 ) && ( iDesiredSlotX < iNumColumns ) && ( iDesiredSlotY >= 0 ) && ( iDesiredSlotY < iNumRows ) ) )
			{
				// Desired slot after tap input is valid, so make the jump to this slot.
				UIVec2D sectionPos;
				GetPositionOfSection( eSectionUnderPointer, &( sectionPos ) );

				iNewSlotIndex = ( iDesiredSlotY * iNumColumns ) + iDesiredSlotX;

				UIVec2D itemPos;
				UIVec2D itemSize;
				GetItemScreenData( eSectionUnderPointer, iNewSlotIndex, &( itemPos ), &( itemSize ) );

				if(IsSectionSlotList(eSectionUnderPointer)) itemPos += sectionPos;

				vSnapPos.x = itemPos.x + ( itemSize.x / 2.0f);
				vSnapPos.y = itemPos.y + ( itemSize.y / 2.0f);

				m_eCurrSection = eSectionUnderPointer;
				m_iCurrSlotX = iDesiredSlotX;
				m_iCurrSlotY = iDesiredSlotY;
			}

			m_eCurrTapState = eTapStateNoInput;

			// If there is no stick input, and we are over a slot, then snap pointer to slot centre.
			// 4J - TomK - only if this particular component allows so!
			if(!m_bPointerDrivenByMouse && CanHaveFocus(eSectionUnderPointer))
			{
				vPointerPos.x = vSnapPos.x;
				vPointerPos.y = vSnapPos.y;
			}
		}
	}

	// Clamp to pointer extents.
	if ( vPointerPos.x < m_fPointerMinX )				vPointerPos.x = m_fPointerMinX;
	else if ( vPointerPos.x > m_fPointerMaxX )		vPointerPos.x = m_fPointerMaxX;
	if ( vPointerPos.y < m_fPointerMinY )				vPointerPos.y = m_fPointerMinY;
	else if ( vPointerPos.y > m_fPointerMaxY )		vPointerPos.y = m_fPointerMaxY;

	// Check if the pointer is outside of the panel.
	bool bPointerIsOutsidePanel = false;
	if ( ( vPointerPos.x < m_fPanelMinX ) || ( vPointerPos.x > m_fPanelMaxX ) || ( vPointerPos.y < m_fPanelMinY ) || ( vPointerPos.y > m_fPanelMaxY ) )
	{
		bPointerIsOutsidePanel = true;
	}

	// Determine appropriate context sensitive tool tips, based on what is carried on the pointer and what is under the pointer.

	// What are we carrying on pointer.
	shared_ptr<LocalPlayer> player = Minecraft::GetInstance()->localplayers[getPad()];
	shared_ptr<ItemInstance> carriedItem = nullptr;
	if(player != NULL) carriedItem = player->inventory->getCarried();

	shared_ptr<ItemInstance> slotItem = nullptr;
	Slot *slot = NULL;
	int slotIndex = 0;
	if(bPointerIsOverSlot)
	{
		slotIndex = iNewSlotIndex + getSectionStartOffset( eSectionUnderPointer );
		slot = m_menu->getSlot(slotIndex);
	}
	bool bIsItemCarried = carriedItem != NULL;
	int iCarriedCount = 0;
	bool bCarriedIsSameAsSlot = false;	// Indicates if same item is carried on pointer as is in slot under pointer.
	if ( bIsItemCarried )
	{
		iCarriedCount = carriedItem->count;
	}

	// What is in the slot that we are over.
	bool bSlotHasItem = false;
	bool bMayPlace = false;
	bool bCanPlaceOne = false;
	bool bCanPlaceAll = false;
	bool bCanCombine = false;
	bool bCanDye = false;
	int iSlotCount = 0;
	int iSlotStackSizeRemaining = 0;	// How many more items can be stacked on this slot.
	if ( bPointerIsOverSlot )
	{
		slotItem = slot->getItem();
		bSlotHasItem = slotItem != NULL;
		if ( bSlotHasItem )
		{
			iSlotCount = slotItem->GetCount();

			if ( bIsItemCarried )
			{
				bCarriedIsSameAsSlot = IsSameItemAs(carriedItem, slotItem);
				bCanCombine = m_menu->mayCombine(slot,carriedItem);
				bCanDye = bCanCombine && dynamic_cast<ArmorItem *>(slot->getItem()->getItem());

				if ( bCarriedIsSameAsSlot )
				{
					iSlotStackSizeRemaining = GetEmptyStackSpace( m_menu->getSlot(slotIndex) );
				}
			}
		}

		if( bIsItemCarried)
		{
			bMayPlace = slot->mayPlace(carriedItem);

			if ( bSlotHasItem ) iSlotStackSizeRemaining = GetEmptyStackSpace( slot );
			else iSlotStackSizeRemaining = slot->getMaxStackSize();

			if(bMayPlace && iSlotStackSizeRemaining > 0) bCanPlaceOne = true;
			if(bMayPlace && iSlotStackSizeRemaining > 1 && carriedItem->count > 1) bCanPlaceAll = true;
		}
	}

	if( bPointerIsOverSlot && bSlotHasItem )
	{
		vector<HtmlString> *desc = GetItemDescription(slot);
		SetPointerText(desc, slot != m_lastPointerLabelSlot);
		m_lastPointerLabelSlot = slot;
		delete desc;
	}
	else if (eSectionUnderPointer != eSectionNone && !IsSectionSlotList(eSectionUnderPointer) )
	{
		vector<HtmlString> *desc = GetSectionHoverText(eSectionUnderPointer);
		SetPointerText(desc, false);
		m_lastPointerLabelSlot = NULL;
		delete desc;
	}
	else
	{
		SetPointerText(NULL, false);
		m_lastPointerLabelSlot = NULL;
	}

	EToolTipItem buttonA, buttonX, buttonY, buttonRT, buttonBack;
	buttonA = buttonX = buttonY = buttonRT = buttonBack = eToolTipNone;
	if ( bPointerIsOverSlot )
	{
		SetPointerOutsideMenu( false );
		if ( bIsItemCarried )
		{
			if ( bSlotHasItem )
			{
				// Item in hand and item in slot ... is item in slot the same as in out hand? If so, can we stack on to it?
				if ( bCarriedIsSameAsSlot )
				{
					// Can we stack more into this slot?
					if ( iSlotStackSizeRemaining == 0 )
					{
						// Cannot stack any more.
						buttonRT = eToolTipWhatIsThis;
					}
					else if ( iSlotStackSizeRemaining == 1 )
					{
						// Can only put 1 more on the stack.
						buttonA = eToolTipPlaceGeneric;
						buttonRT = eToolTipWhatIsThis;
					}
					else // can put 1 or all.
					{
						if(bCanPlaceAll)
						{
							// Multiple items in hand.
							buttonA = eToolTipPlaceAll;
							buttonX = eToolTipPlaceOne;
						}
						else if(bCanPlaceOne)
						{
							if(iCarriedCount > 1) buttonA = eToolTipPlaceOne;
							else buttonA = eToolTipPlaceGeneric;
						}
						buttonRT = eToolTipWhatIsThis;
					}
				}
				else // items are different, click here will swap them.
				{

					if(bMayPlace) buttonA = eToolTipSwap;
					buttonRT = eToolTipWhatIsThis;
				}
				if(bCanDye)
				{
					buttonX = eToolTipDye;
				}
				else if(bCanCombine)
				{
					buttonX = eToolTipRepair;
				}
			}
			else // slot empty.
			{
				// Item in hand, slot is empty.
				if ( iCarriedCount == 1 )
				{
					// Only one item in hand.
					buttonA = eToolTipPlaceGeneric;
				}
				else
				{
					if(bCanPlaceAll)
					{
						// Multiple items in hand.
						buttonA = eToolTipPlaceAll;
						buttonX = eToolTipPlaceOne;
					}
					else if(bCanPlaceOne)
					{
						buttonA = eToolTipPlaceOne;
					}
				}
			}
		}
		else // no object in hand
		{
			if ( bSlotHasItem )
			{
				if ( iSlotCount == 1 )
				{
					buttonA = eToolTipPickUpGeneric;
				}
				else
				{
					// Multiple items in slot.
					buttonA = eToolTipPickUpAll;
					buttonX = eToolTipPickUpHalf;
				}

#ifdef __PSVITA__
				if (!InputManager.IsVitaTV())
				{
					buttonBack = eToolTipWhatIsThis;
				}
				else
#endif
				{
					buttonRT = eToolTipWhatIsThis;
				}
			}
			else
			{
				// Nothing in slot and nothing in hand.
			}
		}

		if ( bSlotHasItem )
		{
			// Item in slot

			// 4J-PB - show tooltips for quick use of armour

			if((eSectionUnderPointer==eSectionInventoryUsing)||(eSectionUnderPointer==eSectionInventoryInventory))
			{
				shared_ptr<ItemInstance> item = getSlotItem(eSectionUnderPointer, iNewSlotIndex);
				ArmorRecipes::_eArmorType eArmourType=ArmorRecipes::GetArmorType(item->id);

				if(eArmourType==ArmorRecipes::eArmorType_None)
				{
					buttonY = eToolTipQuickMove;
				}
				else
				{
					// check that the slot required is empty
					switch(eArmourType)
					{
					case ArmorRecipes::eArmorType_Helmet:
						if(isSlotEmpty(eSectionInventoryArmor,0))
						{
							buttonY = eToolTipEquip;
						}
						else
						{
							buttonY = eToolTipQuickMove;
						}
						break;
					case ArmorRecipes::eArmorType_Chestplate:
						if(isSlotEmpty(eSectionInventoryArmor,1))
						{
							buttonY = eToolTipEquip;
						}
						else
						{
							buttonY = eToolTipQuickMove;
						}
						break;
					case ArmorRecipes::eArmorType_Leggings:
						if(isSlotEmpty(eSectionInventoryArmor,2))
						{
							buttonY = eToolTipEquip;
						}
						else
						{
							buttonY = eToolTipQuickMove;
						}
						break;
					case ArmorRecipes::eArmorType_Boots:
						if(isSlotEmpty(eSectionInventoryArmor,3))
						{
							buttonY = eToolTipEquip;
						}
						else
						{
							buttonY = eToolTipQuickMove;
						}
						break;
					default:
						buttonY = eToolTipQuickMove;
						break;
					}

				}
			}
			// 4J-PB - show tooltips for quick use of fuel or ingredient
			else if((eSectionUnderPointer==eSectionFurnaceUsing)||(eSectionUnderPointer==eSectionFurnaceInventory))
			{
				// Get the info on this item.
				shared_ptr<ItemInstance> item = getSlotItem(eSectionUnderPointer, iNewSlotIndex);
				bool bValidFuel = FurnaceTileEntity::isFuel(item);
				bool bValidIngredient = FurnaceRecipes::getInstance()->getResult(item->getItem()->id) != NULL;

				if(bValidIngredient)
				{
					// is there already something in the ingredient slot?
					if(!isSlotEmpty(eSectionFurnaceIngredient,0))
					{
						// is it the same as this item
						shared_ptr<ItemInstance> IngredientItem = getSlotItem(eSectionFurnaceIngredient,0);
						if(IngredientItem->id == item->id)
						{
							buttonY = eToolTipQuickMoveIngredient;
						}
						else
						{
							if(FurnaceRecipes::getInstance()->getResult(item->id)==NULL)
							{
								buttonY = eToolTipQuickMove;
							}
							else
							{
								buttonY = eToolTipQuickMoveIngredient;
							}
						}
					}
					else
					{
						// ingredient slot empty
						buttonY = eToolTipQuickMoveIngredient;
					}
				}
				else if(bValidFuel)
				{
					// Is there already something in the fuel slot?
					if(!isSlotEmpty(eSectionFurnaceFuel,0))
					{
						// is it the same as this item
						shared_ptr<ItemInstance> fuelItem = getSlotItem(eSectionFurnaceFuel,0);
						if(fuelItem->id == item->id)
						{
							buttonY = eToolTipQuickMoveFuel;
						}
						else if(bValidIngredient)
						{
							// check if the ingredient slot is empty, or the same as this
							if(!isSlotEmpty(eSectionFurnaceIngredient,0))
							{
								// is it the same as this item
								shared_ptr<ItemInstance> IngredientItem = getSlotItem(eSectionFurnaceIngredient,0);
								if(IngredientItem->id == item->id)
								{
									buttonY = eToolTipQuickMoveIngredient;
								}
								else
								{
									if(FurnaceRecipes::getInstance()->getResult(item->id)==NULL)
									{
										buttonY = eToolTipQuickMove;
									}
									else
									{
										buttonY = eToolTipQuickMoveIngredient;
									}
								}
							}
							else
							{
								// ingredient slot empty
								buttonY = eToolTipQuickMoveIngredient;
							}
						}
						else
						{
							buttonY = eToolTipQuickMove;
						}
					}
					else
					{
						buttonY = eToolTipQuickMoveFuel;
					}
				}
				else
				{
					buttonY = eToolTipQuickMove;
				}
			}
			// 4J-PB - show tooltips for quick use of ingredients in brewing
			else if((eSectionUnderPointer==eSectionBrewingUsing)||(eSectionUnderPointer==eSectionBrewingInventory))
			{
				// Get the info on this item.
				shared_ptr<ItemInstance> item = getSlotItem(eSectionUnderPointer, iNewSlotIndex);
				int iId=item->id;

				// valid ingredient?
				bool bValidIngredient=false;
				//bool bValidIngredientBottom=false;

				if(Item::items[iId]->hasPotionBrewingFormula() || (iId == Item::netherwart_seeds_Id))
				{
					bValidIngredient=true;
				}

				if(bValidIngredient)
				{
					// is there already something in the ingredient slot?
					if(!isSlotEmpty(eSectionBrewingIngredient,0))
					{
						// is it the same as this item
						shared_ptr<ItemInstance> IngredientItem = getSlotItem(eSectionBrewingIngredient,0);
						if(IngredientItem->id == item->id)
						{
							buttonY = eToolTipQuickMoveIngredient;
						}
						else
						{
							buttonY=eToolTipQuickMove;
						}
					}
					else
					{
						// ingredient slot empty
						buttonY = eToolTipQuickMoveIngredient;
					}
				}
				else
				{
					// valid potion? Glass bottle with water in it is a 'potion' too.
					if(iId==Item::potion_Id)
					{
						// space available?
						if(isSlotEmpty(eSectionBrewingBottle1,0) ||
							isSlotEmpty(eSectionBrewingBottle2,0) ||
							isSlotEmpty(eSectionBrewingBottle3,0))
						{
							buttonY = eToolTipQuickMoveIngredient;
						}
						else
						{
							buttonY=eToolTipNone;
						}
					}
					else
					{
						buttonY=eToolTipQuickMove;
					}
				}
			}
			else if((eSectionUnderPointer==eSectionEnchantUsing)||(eSectionUnderPointer==eSectionEnchantInventory))
			{
				// Get the info on this item.
				shared_ptr<ItemInstance> item = getSlotItem(eSectionUnderPointer, iNewSlotIndex);
				int iId=item->id;

				// valid enchantable tool?
				if(Item::items[iId]->isEnchantable(item))
				{
					// is there already something in the ingredient slot?
					if(isSlotEmpty(eSectionEnchantSlot,0))
					{
						// tool slot empty
						switch(iId)
						{
						case Item::bow_Id:
						case Item::sword_wood_Id:
						case Item::sword_stone_Id:
						case Item::sword_iron_Id:
						case Item::sword_diamond_Id:
							buttonY=eToolTipQuickMoveWeapon;
							break;

						case Item::helmet_leather_Id:
						case Item::chestplate_leather_Id:
						case Item::leggings_leather_Id:
						case Item::boots_leather_Id:

						case Item::helmet_chain_Id:
						case Item::chestplate_chain_Id:
						case Item::leggings_chain_Id:
						case Item::boots_chain_Id:

						case Item::helmet_iron_Id:
						case Item::chestplate_iron_Id:
						case Item::leggings_iron_Id:
						case Item::boots_iron_Id:

						case Item::helmet_diamond_Id:
						case Item::chestplate_diamond_Id:
						case Item::leggings_diamond_Id:
						case Item::boots_diamond_Id:

						case Item::helmet_gold_Id:
						case Item::chestplate_gold_Id:
						case Item::leggings_gold_Id:
						case Item::boots_gold_Id:
							buttonY=eToolTipQuickMoveArmor;

							break;
						case Item::book_Id:
							buttonY = eToolTipQuickMove;
							break;
						default:
							buttonY=eToolTipQuickMoveTool;
							break;
						}
					}
					else
					{
						buttonY = eToolTipQuickMove;
					}
				}
				else
				{
					buttonY=eToolTipQuickMove;
				}
			}
			else
			{
				buttonY = eToolTipQuickMove;
			}
		}
	}

	if ( bPointerIsOutsidePanel )
	{
		SetPointerOutsideMenu( true );
		// Outside window, we dropping items.
		if ( bIsItemCarried )
		{
			//int iCount = m_pointerControl->GetObjectCount( m_pointerControl->m_hObj );
			if ( iCarriedCount > 1 )
			{
				buttonA = eToolTipDropAll;
				buttonX = eToolTipDropOne;
			}
			else
			{
				buttonA = eToolTipDropGeneric;
			}
		}
	}
	else // pointer is just over dead space ... can't really do anything.
	{
		SetPointerOutsideMenu( false );
	}

	shared_ptr<ItemInstance> item = nullptr;
	if(bPointerIsOverSlot && bSlotHasItem) item = getSlotItem(eSectionUnderPointer, iNewSlotIndex);
	overrideTooltips(eSectionUnderPointer, item, bIsItemCarried, bSlotHasItem, bCarriedIsSameAsSlot, iSlotStackSizeRemaining, buttonA, buttonX, buttonY, buttonRT, buttonBack);

	SetToolTip( eToolTipButtonA, buttonA );
	SetToolTip( eToolTipButtonX, buttonX );
	SetToolTip( eToolTipButtonY, buttonY );
	SetToolTip( eToolTipButtonRT, buttonRT );
	SetToolTip( eToolTipButtonBack, buttonBack );

	// Offset back to image top left.
	vPointerPos.x -= m_fPointerImageOffsetX;
	vPointerPos.y -= m_fPointerImageOffsetY;

	// Update pointer position.
	// 4J-PB - do not allow sub pixel positions or we get broken lines in box edges

	// problem here when sensitivity is low - we'll be moving a sub pixel size, so it'll clamp, and we'll never move. In that case, move 1 pixel
	if(fInputDirX!=0.0f)
	{
		if(fInputDirX==1.0f)
		{
			vPointerPos.x+=0.999999f;
		}
		else
		{
			vPointerPos.x-=0.999999f;
		}
	}

	if(fInputDirY!=0.0f)
	{
		if(fInputDirY==1.0f)
		{
			vPointerPos.y+=0.999999f;
		}
		else
		{
			vPointerPos.y-=0.999999f;
		}
	}

	vPointerPos.x = floor(vPointerPos.x);
	vPointerPos.x += ( (int)vPointerPos.x%2);
	vPointerPos.y = floor(vPointerPos.y);
	vPointerPos.y += ( (int)vPointerPos.y%2);
	m_pointerPos = vPointerPos;

	adjustPointerForSafeZone();
}

bool IUIScene_AbstractContainerMenu::handleKeyDown(int iPad, int iAction, bool bRepeat)
{
	bool bHandled = false;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[getPad()] != NULL )
	{
		Tutorial *tutorial = pMinecraft->localgameModes[getPad()]->getTutorial();
		if(tutorial != NULL)
		{
			tutorial->handleUIInput(iAction);
			if(ui.IsTutorialVisible(getPad()) && !tutorial->isInputAllowed(iAction))
			{
				return S_OK;
			}
		}
	}

#ifdef _XBOX
	ui.AnimateKeyPress(iPad, iAction);
#else
	ui.AnimateKeyPress(iPad, iAction, bRepeat, true, false);
#endif

	int buttonNum=0; // 0 = LeftMouse, 1 = RightMouse
	BOOL quickKeyHeld=FALSE; // Represents shift key on PC

	BOOL validKeyPress = FALSE;
	bool itemEditorKeyPress = false;

	// Ignore input from other players
	//if(pMinecraft->player->GetXboxPad()!=pInputData->UserIndex) return S_OK;

	switch(iAction)
	{
#ifdef _DEBUG_MENUS_ENABLED
	case ACTION_MENU_OTHER_STICK_PRESS:
		itemEditorKeyPress = TRUE;
		break;
#endif
	case ACTION_MENU_A:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		if(!bRepeat)
		{
			validKeyPress = TRUE;

			// Standard left click
			buttonNum = 0;
			quickKeyHeld = FALSE;

			if( IsSectionSlotList( m_eCurrSection ) )
			{
				int currentIndex = getCurrentIndex( m_eCurrSection ) - getSectionStartOffset(m_eCurrSection);

				bool bSlotHasItem = !isSlotEmpty(m_eCurrSection, currentIndex);
				if ( bSlotHasItem )
					ui.PlayUISFX(eSFX_Press);
			}
			//
		}
		break;
	case ACTION_MENU_X:
		if(!bRepeat)
		{
			validKeyPress = TRUE;

			// Standard right click
			buttonNum = 1;
			quickKeyHeld = FALSE;

			if( IsSectionSlotList( m_eCurrSection ) )
			{
				int currentIndex = getCurrentIndex( m_eCurrSection ) - getSectionStartOffset(m_eCurrSection);

				bool bSlotHasItem = !isSlotEmpty(m_eCurrSection, currentIndex);
				if ( bSlotHasItem )
					ui.PlayUISFX(eSFX_Press);
			}
		}
		break;
	case ACTION_MENU_Y:
		if(!bRepeat)
		{
			//bool bIsItemCarried = !m_pointerControl->isEmpty( m_pointerControl->m_hObj );

			// 4J Stu - TU8: Remove this fix, and fix the tooltip display instead as customers liked the feature

			// Fix for #58583 - TU6: Content: UI: The Quick Move button prompt disappears even though it still works
			// No quick move tooltip is shown if something is carried, so disable the action as well
			//if(!bIsItemCarried)
			{
				validKeyPress = TRUE;

				// Shift and left click
				buttonNum = 0;
				quickKeyHeld = TRUE;
				if( IsSectionSlotList( m_eCurrSection ) )
				{
					int currentIndex = getCurrentIndex( m_eCurrSection ) - getSectionStartOffset(m_eCurrSection);

					bool bSlotHasItem = !isSlotEmpty(m_eCurrSection, currentIndex);
					if ( bSlotHasItem )
						ui.PlayUISFX(eSFX_Press);
				}
			}
		}
		break;
		// 4J Stu - Also enable start to exit the scene. This key is also not constrained by the tutorials.
	case ACTION_MENU_PAUSEMENU:
	case ACTION_MENU_B:
		{

			ui.SetTooltips(iPad, -1);

			// 4J Stu - Fix for #11302 - TCR 001: Network Connectivity: Host crashed after being killed by the client while accessing a chest during burst packet loss.
			// We need to make sure that we call closeContainer() anytime this menu is closed, even if it is forced to close by some other reason (like the player dying)
			// Therefore I have moved this call to the OnDestroy() method to make sure that it always happens.
			//Minecraft::GetInstance()->localplayers[pInputData->UserIndex]->closeContainer();

			// Return to the game. We should really callback to the app here as well
			// to let it know that we have closed the ui incase we need to do things when that happens

			if(m_bNavigateBack)
			{
				ui.NavigateBack(iPad);
			}
			else
			{
				ui.CloseUIScenes(iPad);
			}

			bHandled = true;
			return S_OK;
		}
		break;
	case ACTION_MENU_LEFT:
		{
			//ui.PlayUISFX(eSFX_Focus);
			m_eCurrTapState = eTapStateLeft;
		}
		break;
	case ACTION_MENU_RIGHT:
		{
			//ui.PlayUISFX(eSFX_Focus);
			m_eCurrTapState = eTapStateRight;
		}
		break;
	case ACTION_MENU_UP:
		{
			//ui.PlayUISFX(eSFX_Focus);
			m_eCurrTapState = eTapStateUp;
		}
		break;
	case ACTION_MENU_DOWN:
		{
			//ui.PlayUISFX(eSFX_Focus);
			m_eCurrTapState = eTapStateDown;
		}
		break;
	case ACTION_MENU_PAGEUP:
		{
			// 4J Stu - Do nothing except stop this being passed anywhere else
			bHandled = true;
		}
		break;
	case ACTION_MENU_PAGEDOWN:
		{
			if( IsSectionSlotList( m_eCurrSection ) )
			{
				int currentIndex = getCurrentIndex( m_eCurrSection ) - getSectionStartOffset(m_eCurrSection);

				bool bSlotHasItem = !isSlotEmpty(m_eCurrSection, currentIndex);
				if ( bSlotHasItem )
				{
					shared_ptr<ItemInstance> item = getSlotItem(m_eCurrSection, currentIndex);
					if( Minecraft::GetInstance()->localgameModes[iPad] != NULL )
					{
						Tutorial::PopupMessageDetails *message = new Tutorial::PopupMessageDetails;
						message->m_messageId = item->getUseDescriptionId();

						if(Item::items[item->id] != NULL) message->m_titleString = Item::items[item->id]->getHoverName(item);
						message->m_titleId = item->getDescriptionId();

						message->m_icon = item->id;
						message->m_iAuxVal = item->getAuxValue();
						message->m_forceDisplay = true;

						TutorialMode *gameMode = (TutorialMode *)Minecraft::GetInstance()->localgameModes[iPad];
						gameMode->getTutorial()->setMessage(NULL, message);
						ui.PlayUISFX(eSFX_Press);
					}
				}
			}
			bHandled = TRUE;
		}
		break;
	};

	if( validKeyPress == TRUE )
	{
		if(handleValidKeyPress(iPad,buttonNum,quickKeyHeld))
		{
			// Used to allow overriding certain keypresses, so do nothing here
		}
		else
		{
			if( IsSectionSlotList( m_eCurrSection ) )
			{
				handleSlotListClicked(m_eCurrSection,buttonNum,quickKeyHeld);
			}
			else
			{
				// TODO Clicked something else, like for example the craft result. Do something here

				// 4J WESTY : For pointer system we can legally drop items outside of the window panel here, or may press button while
				// pointer is over empty panel space.
				if ( m_bPointerOutsideMenu )
				{
					handleOutsideClicked(iPad, buttonNum, quickKeyHeld);
				}
				else //
				{
					// over empty space or something else???
					handleOtherClicked(iPad,m_eCurrSection,buttonNum,quickKeyHeld?true:false);
					//assert( FALSE );
				}
			}
		}
		bHandled = true;
	}
#ifdef _DEBUG_MENUS_ENABLED
	else if(itemEditorKeyPress == TRUE)
	{
		if( IsSectionSlotList( m_eCurrSection ) )
		{			
			ItemEditorInput *initData = new ItemEditorInput();
			initData->iPad = getPad();
			initData->slot = getSlot( m_eCurrSection, getCurrentIndex(m_eCurrSection) );
			initData->menu = m_menu;

			ui.NavigateToScene(getPad(),eUIScene_DebugItemEditor,(void *)initData);
		}
	}
#endif
	else
	{
		handleAdditionalKeyPress(iAction);
	}

	UpdateTooltips();

	return bHandled;
}

bool IUIScene_AbstractContainerMenu::handleValidKeyPress(int iUserIndex, int buttonNum, BOOL quickKeyHeld)
{
	return false;
}

void IUIScene_AbstractContainerMenu::handleOutsideClicked(int iPad, int buttonNum, BOOL quickKeyHeld)
{
	// Drop items.

	//pMinecraft->localgameModes[m_iPad]->handleInventoryMouseClick(menu->containerId, AbstractContainerMenu::CLICKED_OUTSIDE, buttonNum, quickKeyHeld?true:false, pMinecraft->localplayers[m_iPad] );
	slotClicked(AbstractContainerMenu::SLOT_CLICKED_OUTSIDE, buttonNum, quickKeyHeld?true:false);
}

void IUIScene_AbstractContainerMenu::handleOtherClicked(int iPad, ESceneSection eSection, int buttonNum, bool quickKey)
{
	// Do nothing
}

void IUIScene_AbstractContainerMenu::handleAdditionalKeyPress(int iAction)
{
	// Do nothing
}

void IUIScene_AbstractContainerMenu::handleSlotListClicked(ESceneSection eSection, int buttonNum, BOOL quickKeyHeld)
{
	int currentIndex = getCurrentIndex(eSection);

	//pMinecraft->localgameModes[m_iPad]->handleInventoryMouseClick(menu->containerId, currentIndex, buttonNum, quickKeyHeld?true:false, pMinecraft->localplayers[m_iPad] );
	slotClicked(currentIndex, buttonNum, quickKeyHeld?true:false);

	handleSectionClick(eSection);
}

void IUIScene_AbstractContainerMenu::slotClicked(int slotId, int buttonNum, bool quickKey)
{
	// 4J Stu - Removed this line as unused
	//if (slot != NULL) slotId = slot->index;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	pMinecraft->localgameModes[getPad()]->handleInventoryMouseClick(m_menu->containerId, slotId, buttonNum, quickKey, pMinecraft->localplayers[getPad()] );
}

int IUIScene_AbstractContainerMenu::getCurrentIndex(ESceneSection eSection)
{
	int rows, columns;
	GetSectionDimensions( eSection, &columns, &rows );
	int currentIndex = (m_iCurrSlotY * columns) + m_iCurrSlotX;

	return currentIndex + getSectionStartOffset(eSection);
}

bool IUIScene_AbstractContainerMenu::IsSameItemAs(shared_ptr<ItemInstance> itemA, shared_ptr<ItemInstance> itemB)
{
	if(itemA == NULL || itemB == NULL) return false;

	return (itemA->id == itemB->id && (!itemB->isStackedByData() || itemB->getAuxValue() == itemA->getAuxValue()) && ItemInstance::tagMatches(itemB, itemA) );
}

int IUIScene_AbstractContainerMenu::GetEmptyStackSpace(Slot *slot)
{
	int iResult = 0;

	if(slot != NULL && slot->hasItem())
	{
		shared_ptr<ItemInstance> item = slot->getItem();
		if ( item->isStackable() )
		{
			int iCount = item->GetCount();
			int iMaxStackSize = min(item->getMaxStackSize(), slot->getMaxStackSize() );

			iResult = iMaxStackSize - iCount;

			if(iResult < 0 ) iResult = 0;
		}
	}

	return iResult;
}

vector<HtmlString> *IUIScene_AbstractContainerMenu::GetItemDescription(Slot *slot)
{
	if(slot == NULL) return NULL;

	vector<HtmlString> *lines = slot->getItem()->getHoverText(nullptr, false);

	// Add rarity to first line
	if (lines->size() > 0)
	{
		lines->at(0).color = slot->getItem()->getRarity()->color;

		if(slot->getItem()->hasCustomHoverName())
		{
			lines->at(0).color = eTextColor_RenamedItemTitle;
		}
	}

	return lines;
}

vector<HtmlString> *IUIScene_AbstractContainerMenu::GetSectionHoverText(ESceneSection eSection)
{
	return NULL;
}
