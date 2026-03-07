#pragma once

#include "UIScene.h"
#include "Common/UI/UIControl_CheckBox.h"
#include "Common/UI/UIControl_Slider.h"

class UIScene_SettingsGraphicsMenu : public UIScene
{
private:
	enum EControls
	{
		eControl_Clouds,
		eControl_BedrockFog,
		eControl_CustomSkinAnim,
		eControl_RenderDistance,
		eControl_Gamma,
		eControl_FOV,
		eControl_InterfaceOpacity
	};

	UIControl_CheckBox m_checkboxClouds, m_checkboxBedrockFog, m_checkboxCustomSkinAnim; // Checkboxes
	UIControl_Slider m_sliderRenderDistance, m_sliderGamma, m_sliderFOV, m_sliderInterfaceOpacity; // Sliders
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_checkboxClouds, "Clouds")
		UI_MAP_ELEMENT( m_checkboxBedrockFog, "BedrockFog")
		UI_MAP_ELEMENT( m_checkboxCustomSkinAnim, "CustomSkinAnim")
		UI_MAP_ELEMENT( m_sliderRenderDistance, "RenderDistance")
		UI_MAP_ELEMENT( m_sliderGamma, "Gamma")
		UI_MAP_ELEMENT(m_sliderFOV, "FOV")
		UI_MAP_ELEMENT( m_sliderInterfaceOpacity, "InterfaceOpacity")
	UI_END_MAP_ELEMENTS_AND_NAMES()

	bool m_bNotInGame;
public:
	UIScene_SettingsGraphicsMenu(int iPad, void *initData, UILayer *parentLayer);
	virtual ~UIScene_SettingsGraphicsMenu();

	virtual EUIScene getSceneType() { return eUIScene_SettingsGraphicsMenu;}
	
	virtual void updateTooltips();
	virtual void updateComponents();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

	virtual void handleSliderMove(F64 sliderId, F64 currentValue);

	static int LevelToDistance(int dist);

	static int DistanceToLevel(int dist);
};