#pragma once
using namespace std;
#include "TutorialTask.h"
#include "TutorialConstraint.h"
#include "TutorialHint.h"
#include "TutorialMessage.h"
#include "TutorialEnum.h"

// #define TUTORIAL_HINT_DELAY_TIME 14000 // How long we should wait from displaying one hint to the next
// #define TUTORIAL_DISPLAY_MESSAGE_TIME 7000
// #define TUTORIAL_MINIMUM_DISPLAY_MESSAGE_TIME 2000
// #define TUTORIAL_REMINDER_TIME (TUTORIAL_DISPLAY_MESSAGE_TIME + 20000)
// #define TUTORIAL_CONSTRAINT_DELAY_REMOVE_TICKS 15
// 
// // 0-24000
// #define TUTORIAL_FREEZE_TIME_VALUE 8000

class Level;
class CXuiScene;

class Tutorial
{
public:
	class PopupMessageDetails
	{
	public:
		int m_messageId;
		int m_promptId;
		int m_titleId;
		wstring m_messageString;
		wstring m_promptString;
		wstring m_titleString;
		int m_icon;
		int m_iAuxVal;
		bool m_allowFade;
		bool m_isReminder;
		bool m_replaceCurrent;
		bool m_forceDisplay;
		bool m_delay;

		PopupMessageDetails()
		{
			m_messageId = -1;
			m_promptId = -1;
			m_titleId = -1;
			m_messageString = L"";
			m_promptString = L"";
			m_titleString = L"";
			m_icon = TUTORIAL_NO_ICON;
			m_iAuxVal = 0;
			m_allowFade = true;
			m_isReminder = false;
			m_replaceCurrent = false;
			m_forceDisplay = false;
			m_delay = false;
		}

		bool isSameContent(PopupMessageDetails *other);

	};

private:
	static int m_iTutorialHintDelayTime;
	static int m_iTutorialDisplayMessageTime;
	static int m_iTutorialMinimumDisplayMessageTime;
	static int m_iTutorialExtraReminderTime;
	static int m_iTutorialReminderTime;
	static int m_iTutorialConstraintDelayRemoveTicks;
	static int m_iTutorialFreezeTimeValue;
	eTutorial_State m_CurrentState;
	bool m_hasStateChanged;
#ifdef _XBOX
	HXUIOBJ m_hTutorialScene; // to store the popup scene (splitscreen or normal)
#endif
	bool m_bSceneIsSplitscreen;

	bool m_bHasTickedOnce;
	int m_firstTickTime;

protected:
	unordered_map<int, TutorialMessage *> messages;
	vector<TutorialConstraint *> m_globalConstraints;
	vector<TutorialConstraint *> constraints[e_Tutorial_State_Max];
	vector< pair<TutorialConstraint *, unsigned char> > constraintsToRemove[e_Tutorial_State_Max];
	vector<TutorialTask *> tasks; // We store a copy of the tasks for the main gameplay tutorial so that we could display an overview menu
	vector<TutorialTask *> activeTasks[e_Tutorial_State_Max];
	vector<TutorialHint *> hints[e_Tutorial_State_Max];
	TutorialTask *currentTask[e_Tutorial_State_Max];
	TutorialConstraint *currentFailedConstraint[e_Tutorial_State_Max];

	bool m_freezeTime;
	bool m_timeFrozen;
	//D3DXVECTOR3 m_OriginalPosition;

public:
	DWORD lastMessageTime;
	DWORD m_lastHintDisplayedTime;
private:
	PopupMessageDetails *m_lastMessage;

	eTutorial_State m_lastMessageState;
	unsigned int m_iTaskReminders;

	bool m_allowShow;

public:
	bool m_hintDisplayed;

private:
	bool hasRequestedUI;
	bool uiTempDisabled;

#ifdef _XBOX
	CXuiScene *m_UIScene;
#else
	UIScene *m_UIScene;
#endif

	int m_iPad;
public:
	bool m_allTutorialsComplete;
	bool m_fullTutorialComplete;
	bool m_isFullTutorial;
public:
	Tutorial(int iPad, bool isFullTutorial = false);
	~Tutorial();
	void tick();

	int getPad() { return m_iPad; }

	virtual bool isStateCompleted( eTutorial_State state );
	virtual void setStateCompleted( eTutorial_State state );
	bool isHintCompleted( eTutorial_Hint hint );
	void setHintCompleted( eTutorial_Hint hint );
	void setHintCompleted( TutorialHint *hint );

	// completableId will be either a eTutorial_State value or eTutorial_Hint
	void setCompleted( int completableId );
	bool getCompleted( int completableId );

#ifdef _XBOX
	void changeTutorialState(eTutorial_State newState, CXuiScene *scene = NULL);
#else
	void changeTutorialState(eTutorial_State newState, UIScene *scene = NULL);
#endif
	bool isSelectedItemState();

	bool setMessage(PopupMessageDetails *message);
	bool setMessage(TutorialHint *hint, PopupMessageDetails *message);
	bool setMessage(const wstring &message, int icon, int auxValue);

	void showTutorialPopup(bool show);

	void useItemOn(Level *level, shared_ptr<ItemInstance> item, int x, int y, int z,bool bTestUseOnly=false);
	void useItemOn(shared_ptr<ItemInstance> item, bool bTestUseOnly=false);
	void completeUsingItem(shared_ptr<ItemInstance> item);
	void startDestroyBlock(shared_ptr<ItemInstance> item, Tile *tile);
	void destroyBlock(Tile *tile);
	void attack(shared_ptr<Player> player, shared_ptr<Entity> entity);
	void itemDamaged(shared_ptr<ItemInstance> item);

	void handleUIInput(int iAction);
	void createItemSelected(shared_ptr<ItemInstance> item, bool canMake);
	void onCrafted(shared_ptr<ItemInstance> item);
	void onTake(shared_ptr<ItemInstance> item, unsigned int invItemCountAnyAux, unsigned int invItemCountThisAux);
	void onSelectedItemChanged(shared_ptr<ItemInstance> item);
	void onLookAt(int id, int iData=0);
	void onLookAtEntity(shared_ptr<Entity> entity);
	void onRideEntity(shared_ptr<Entity> entity);
	void onEffectChanged(MobEffect *effect, bool bRemoved=false);

	bool canMoveToPosition(double xo, double yo, double zo, double xt, double yt, double zt);
	bool isInputAllowed(int mapping);

	void AddGlobalConstraint(TutorialConstraint *c);
	void AddConstraint(TutorialConstraint *c);
	void RemoveConstraint(TutorialConstraint *c, bool delayedRemove = false);
	void addTask(eTutorial_State state, TutorialTask *t);
	void addHint(eTutorial_State state, TutorialHint *h);
	void addMessage(int messageId, bool limitRepeats = false, unsigned char numRepeats = TUTORIAL_MESSAGE_DEFAULT_SHOW);

	int	GetTutorialDisplayMessageTime() {return m_iTutorialDisplayMessageTime;}

	// Only for the main gameplay tutorial
	vector<TutorialTask *> *getTasks();
	unsigned int getCurrentTaskIndex();

#ifdef _XBOX
	CXuiScene *getScene() { return m_UIScene; }
#else
	UIScene *getScene() { return m_UIScene; }
#endif
	eTutorial_State getCurrentState() { return m_CurrentState; }

	// These are required so that we have a consistent mapping of the completion bits stored in the profile data
	static void staticCtor();
	static vector<int> s_completableTasks;

	static void debugResetPlayerSavedProgress(int iPad);
};
