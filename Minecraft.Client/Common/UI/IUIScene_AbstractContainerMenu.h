#pragma once

// Uncomment to enable tap input detection to jump 1 slot. Doesn't work particularly well yet, and I feel the system does not need it.
// Would probably be required if we decide to slow down the pointer movement.
// 4J Stu - There was a request to be able to navigate the scenes with the dpad, so I have used much of the TAP_DETECTION
// code as it worked well for that situation. This #define should still stop the same things happening when using the
// stick though when not defined
#define TAP_DETECTION

// Uncomment to enable acceleration on pointer input.
//#define USE_POINTER_ACCEL

#define POINTER_INPUT_TIMER_ID		(0)		// Arbitrary timer ID. 
#define POINTER_SPEED_FACTOR		(13.0f)		// Speed of pointer.
//#define POINTER_PANEL_OVER_REACH	(42.0f)		// Amount beyond edge of panel which pointer can go over to drop items. - comes from the pointer size in the scene

#define MAX_INPUT_TICKS_FOR_SCALING		(7)
#define MAX_INPUT_TICKS_FOR_TAPPING		(15)

class AbstractContainerMenu;
class Slot;

class IUIScene_AbstractContainerMenu
{
protected:
	// Sections of this scene containing items selectable by the pointer.
	// 4J Stu - Always make the Using section the first one
	enum ESceneSection
	{
		eSectionNone = -1,
		eSectionContainerUsing = 0,
		eSectionContainerInventory,
		eSectionContainerChest,
		eSectionContainerMax,
		
		eSectionFurnaceUsing,
		eSectionFurnaceInventory,
		eSectionFurnaceIngredient,
		eSectionFurnaceFuel,
		eSectionFurnaceResult,
		eSectionFurnaceMax,
		
		eSectionInventoryUsing,
		eSectionInventoryInventory,
		eSectionInventoryArmor,
		eSectionInventoryMax,
		
		eSectionTrapUsing,
		eSectionTrapInventory,
		eSectionTrapTrap,
		eSectionTrapMax,

		eSectionInventoryCreativeUsing,
		eSectionInventoryCreativeSelector,
		eSectionInventoryCreativeTab_0,
		eSectionInventoryCreativeTab_1,
		eSectionInventoryCreativeTab_2,
		eSectionInventoryCreativeTab_3,
		eSectionInventoryCreativeTab_4,
		eSectionInventoryCreativeTab_5,
		eSectionInventoryCreativeTab_6,
		eSectionInventoryCreativeTab_7,
		eSectionInventoryCreativeSlider,
		eSectionInventoryCreativeMax,
		
		eSectionEnchantUsing,
		eSectionEnchantInventory,
		eSectionEnchantSlot,
		eSectionEnchantButton1,
		eSectionEnchantButton2,
		eSectionEnchantButton3,
		eSectionEnchantMax,

		eSectionBrewingUsing,
		eSectionBrewingInventory,
		eSectionBrewingBottle1,
		eSectionBrewingBottle2,
		eSectionBrewingBottle3,
		eSectionBrewingIngredient,
		eSectionBrewingMax,

		eSectionAnvilUsing,
		eSectionAnvilInventory,
		eSectionAnvilItem1,
		eSectionAnvilItem2,
		eSectionAnvilResult,
		eSectionAnvilName,
		eSectionAnvilMax,

		eSectionBeaconUsing,
		eSectionBeaconInventory,
		eSectionBeaconItem,
		eSectionBeaconPrimaryTierOneOne,
		eSectionBeaconPrimaryTierOneTwo,
		eSectionBeaconPrimaryTierTwoOne,
		eSectionBeaconPrimaryTierTwoTwo,
		eSectionBeaconPrimaryTierThree,
		eSectionBeaconSecondaryOne,
		eSectionBeaconSecondaryTwo,
		eSectionBeaconConfirm,
		eSectionBeaconMax,

		eSectionHopperUsing,
		eSectionHopperInventory,
		eSectionHopperContents,
		eSectionHopperMax,

		eSectionHorseUsing,
		eSectionHorseInventory,
		eSectionHorseChest,
		eSectionHorseArmor,
		eSectionHorseSaddle,
		eSectionHorseMax,

		eSectionFireworksUsing,
		eSectionFireworksInventory,
		eSectionFireworksResult,
		eSectionFireworksIngredients,
		eSectionFireworksMax,
	};

	AbstractContainerMenu* m_menu;
	bool m_autoDeleteMenu;

	eTutorial_State m_previousTutorialState;

	UIVec2D m_pointerPos;
	bool m_bPointerDrivenByMouse;

	// Offset from pointer image top left to centre (we use the centre as the actual pointer).
	float	m_fPointerImageOffsetX;
	float	m_fPointerImageOffsetY;

	// Min and max extents for the pointer.
	float	m_fPointerMinX;
	float	m_fPointerMaxX;
	float	m_fPointerMinY;
	float	m_fPointerMaxY;

	// Min and max extents of the panel.
	float	m_fPanelMinX;
	float	m_fPanelMaxX;
	float	m_fPanelMinY;
	float	m_fPanelMaxY;

	int		m_iConsectiveInputTicks;

	// Used for detecting quick "taps" in a direction, should jump cursor to next slot.
	enum ETapState
	{
		eTapStateNoInput = 0,
		eTapStateUp,
		eTapStateDown,
		eTapStateLeft,
		eTapStateRight,
		eTapStateJump,
		eTapNone
	};

	ETapState		m_eCurrTapState;
	ESceneSection	m_eCurrSection;
	int				m_iCurrSlotX;
	int				m_iCurrSlotY;

#ifdef __ORBIS__
	bool m_bFirstTouchStored[XUSER_MAX_COUNT]; // monitor the first position of a touch, so we can use relative distances of movement
	UIVec2D m_oldvPointerPos;
	UIVec2D m_oldvTouchPos;
	// store the multipliers to map the UI window to the touchpad window
	float m_fTouchPadMulX;
	float m_fTouchPadMulY;
	float m_fTouchPadDeadZoneX; // usese the multipliers
	float m_fTouchPadDeadZoneY;


#endif

	// ENum indexes of the first section for this scene, and 1+the last section
	ESceneSection m_eFirstSection, m_eMaxSection;

	// 4J - WESTY  - Added for pointer prototype.
	// Current tooltip settings.
	EToolTipItem	m_aeToolTipSettings[ eToolTipNumButtons ];
	
	// 4J - WESTY  - Added for pointer prototype.
	// Indicates if pointer is outside UI window (used to drop items).
	bool			m_bPointerOutsideMenu;
	Slot *m_lastPointerLabelSlot;

	bool m_bSplitscreen;
	bool	m_bNavigateBack; // should we exit the xuiscenes or just navigate back on exit?
	
	virtual bool IsSectionSlotList( ESceneSection eSection ) { return eSection != eSectionNone; }
	virtual bool CanHaveFocus( ESceneSection eSection ) { return true; }
	virtual bool IsVisible( ESceneSection eSection ) { return true; }
	int	GetSectionDimensions( ESceneSection eSection, int* piNumColumns, int* piNumRows );
	virtual int getSectionColumns(ESceneSection eSection) = 0;
	virtual int getSectionRows(ESceneSection eSection) = 0;
	virtual ESceneSection GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY ) = 0;
	virtual void GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition ) = 0;
	virtual void GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize ) = 0;
	void updateSlotPosition( ESceneSection eSection, ESceneSection newSection, ETapState eTapDirection, int *piTargetX, int *piTargetY, int xOffset = 0, int yOffset = 0 );

	#ifdef TAP_DETECTION
		ETapState GetTapInputType( float fInputX, float fInputY );
	#endif

	// Current tooltip settings.
	void SetToolTip( EToolTipButton eButton, EToolTipItem eItem );
	void UpdateTooltips();

	// 4J - WESTY  - Added for pointer prototype.
	void SetPointerOutsideMenu( bool bOutside )		{ m_bPointerOutsideMenu = bOutside; }
	
	void Initialize(int m_iPad, AbstractContainerMenu* menu, bool autoDeleteMenu, int startIndex,ESceneSection firstSection,ESceneSection maxSection, bool bNavigateBack=FALSE);
	virtual void PlatformInitialize(int iPad, int startIndex) = 0;
	virtual void InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex = 0) = 0;

	void onMouseTick();
	bool handleKeyDown(int iPad, int iAction, bool bRepeat);
	virtual bool handleValidKeyPress(int iUserIndex, int buttonNum, BOOL quickKeyHeld);
	virtual void handleOutsideClicked(int iPad, int buttonNum, BOOL quickKeyHeld);
	virtual void handleOtherClicked(int iPad, ESceneSection eSection, int buttonNum, bool quickKey);
	virtual void handleAdditionalKeyPress(int iAction);
	virtual void handleSlotListClicked(ESceneSection eSection, int buttonNum, BOOL quickKeyHeld);
	virtual void handleSectionClick(ESceneSection eSection) = 0;
	void slotClicked(int slotId, int buttonNum, bool quickKey);
	int getCurrentIndex(ESceneSection eSection);
	virtual int getSectionStartOffset(ESceneSection eSection) = 0;
	virtual bool doesSectionTreeHaveFocus(ESceneSection eSection) = 0;
	virtual void setSectionFocus(ESceneSection eSection, int iPad) = 0;
	virtual void setSectionSelectedSlot(ESceneSection eSection, int x, int y) = 0;
	virtual void setFocusToPointer(int iPad) = 0;
	virtual void SetPointerText(vector<HtmlString> *description, bool newSlot) = 0;
	virtual vector<HtmlString> *GetSectionHoverText(ESceneSection eSection);
	virtual shared_ptr<ItemInstance> getSlotItem(ESceneSection eSection, int iSlot) = 0;
	virtual Slot *getSlot(ESceneSection eSection, int iSlot) = 0;
	virtual bool isSlotEmpty(ESceneSection eSection, int iSlot) = 0;
	virtual void adjustPointerForSafeZone() = 0;

	virtual bool overrideTooltips(
		ESceneSection sectionUnderPointer,
		shared_ptr<ItemInstance> itemUnderPointer,
		bool bIsItemCarried,
		bool bSlotHasItem,
		bool bCarriedIsSameAsSlot,
		int iSlotStackSizeRemaining,
		EToolTipItem &buttonA,
		EToolTipItem &buttonX,
		EToolTipItem &buttonY,
		EToolTipItem &buttonRT,
		EToolTipItem &buttonBack
	) { return false; }

private:
	bool IsSameItemAs(shared_ptr<ItemInstance> itemA, shared_ptr<ItemInstance> itemB);
	int GetEmptyStackSpace(Slot *slot);

	vector<HtmlString> *GetItemDescription(Slot *slot);

protected:

	IUIScene_AbstractContainerMenu();
	virtual ~IUIScene_AbstractContainerMenu();

public:
	virtual int getPad() = 0;
	virtual int getMovieWidth() = 0;
	virtual int getMovieHeight() = 0;
	virtual void getMouseToSWFScale(float &scaleX, float &scaleY) = 0;
};
