#pragma once
#include "Model.h"

class HumanoidModel : public Model
{
public:
	ModelPart *head, *hair, *body, *arm0, *arm1, *leg0, *leg1, *ear, *cloak;
	//ModelPart *hat; 

	int holdingLeftHand;
    int holdingRightHand;
	bool idle;
	bool sneaking;
	bool bowAndArrow;
	bool eating;		// 4J added
	float eating_t;			// 4J added
	float eating_swing;	// 4J added
	unsigned int m_uiAnimOverrideBitmask; // 4J added
	float m_fYOffset; // 4J added
	enum animbits
	{
		eAnim_ArmsDown =0,
		eAnim_ArmsOutFront,
		eAnim_NoLegAnim,
		eAnim_HasIdle,
		eAnim_ForceAnim, // Claptrap looks bad if the user turns off custom skin anim
		// 4J-PB - DaveK wants Fish characters to move both legs in the same way
		eAnim_SingleLegs,
		eAnim_SingleArms,
		eAnim_StatueOfLiberty, // Dr Who Weeping Angel
		eAnim_DontRenderArmour, // Dr Who Daleks
		eAnim_NoBobbing, // Dr Who Daleks
		eAnim_DisableRenderHead,
		eAnim_DisableRenderArm0,
		eAnim_DisableRenderArm1,
		eAnim_DisableRenderTorso,
		eAnim_DisableRenderLeg0,
		eAnim_DisableRenderLeg1,
		eAnim_DisableRenderHair,
		eAnim_SmallModel // Maggie Simpson for riding horse, etc

	};

	static const unsigned int m_staticBitmaskIgnorePlayerCustomAnimSetting= (1<<HumanoidModel::eAnim_ForceAnim) |
		(1<<HumanoidModel::eAnim_DisableRenderArm0) |
		(1<<HumanoidModel::eAnim_DisableRenderArm1) |
		(1<<HumanoidModel::eAnim_DisableRenderTorso) |
		(1<<HumanoidModel::eAnim_DisableRenderLeg0) |
		(1<<HumanoidModel::eAnim_DisableRenderLeg1) |
		(1<<HumanoidModel::eAnim_DisableRenderHair);


	void _init(float g, float yOffset, int texWidth, int texHeight);	// 4J added
    HumanoidModel();
    HumanoidModel(float g);
    HumanoidModel(float g, float yOffset, int texWidth, int texHeight);
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
    virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);
    void renderHair(float scale, bool usecompiled);
    void renderEars(float scale, bool usecompiled);
    void renderCloak(float scale, bool usecompiled); 
    void render(HumanoidModel *model, float scale, bool usecompiled);

// Add new bits to models
	ModelPart * AddOrRetrievePart(SKIN_BOX *pBox);
};
