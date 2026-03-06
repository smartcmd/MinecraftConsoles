#pragma once
#ifdef _DEBUG_MENUS_ENABLED
#include "UIScene.h"
#include "..\..\Common\GameRules\ConsoleSchematicFile.h"

class UIScene_DebugCreateSchematic : public UIScene
{
private:
	enum eControls
	{
		eControl_Name,
		eControl_StartX,
		eControl_StartY,
		eControl_StartZ,
		eControl_EndX,
		eControl_EndY,
		eControl_EndZ,
		eControl_SaveMobs,
		eControl_UseCompression,
		eControl_Create,
	};

	eControls m_keyboardCallbackControl;

	ConsoleSchematicFile::XboxSchematicInitParam *m_data;


public:
	UIScene_DebugCreateSchematic(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_DebugCreateSchematic;}

protected:
	UIControl_TextInput m_textInputStartX, m_textInputStartY, m_textInputStartZ, m_textInputEndX, m_textInputEndY, m_textInputEndZ, m_textInputName;
	UIControl_CheckBox m_checkboxSaveMobs, m_checkboxUseCompression;
	UIControl_Button m_buttonCreate;
	UIControl_Label m_labelStartX, m_labelStartY, m_labelStartZ, m_labelEndX, m_labelEndY, m_labelEndZ, m_labelTitle;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_textInputStartX, "StartX")
		UI_MAP_ELEMENT( m_textInputStartY, "StartY")
		UI_MAP_ELEMENT( m_textInputStartZ, "StartZ")
		UI_MAP_ELEMENT( m_textInputEndX, "EndX")
		UI_MAP_ELEMENT( m_textInputEndY, "EndY")
		UI_MAP_ELEMENT( m_textInputEndZ, "EndZ")
		UI_MAP_ELEMENT( m_textInputName, "Name")

		UI_MAP_ELEMENT( m_checkboxSaveMobs, "SaveMobs")
		UI_MAP_ELEMENT( m_checkboxUseCompression, "UseCompression")

		UI_MAP_ELEMENT( m_buttonCreate, "Create")

		UI_MAP_ELEMENT( m_labelStartX, "LabelStartX")
		UI_MAP_ELEMENT( m_labelStartY, "LabelStartY")
		UI_MAP_ELEMENT( m_labelStartZ, "LabelStartZ")
		UI_MAP_ELEMENT( m_labelEndX, "LabelEndX")
		UI_MAP_ELEMENT( m_labelEndY, "LabelEndY")
		UI_MAP_ELEMENT( m_labelEndZ, "LabelEndZ")
		UI_MAP_ELEMENT( m_labelTitle, "LabelTitle")
	UI_END_MAP_ELEMENTS_AND_NAMES()

	virtual wstring getMoviePath();
#ifdef _WINDOWS64
	virtual void getDirectEditInputs(vector<UIControl_TextInput*> &inputs);
	virtual void onDirectEditFinished(UIControl_TextInput *input, UIControl_TextInput::EDirectEditResult result);
	virtual bool handleMouseClick(F32 x, F32 y);
#endif

public:
	virtual void tick();
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

protected:
	void handlePress(F64 controlId, F64 childId);	
	virtual void handleCheckboxToggled(F64 controlId, bool selected);

private:
	UIControl_TextInput* getTextInputForControl(eControls ctrl);
	static int KeyboardCompleteCallback(LPVOID lpParam,const bool bRes);
};
#endif