#pragma once
#include "Model.h"

class HumanoidModel : public Model
{
public:
	enum ePlayerModelType
	{
		ePlayerModel_Steve = 0,
		ePlayerModel_Alex
	};

	ModelPart *head, *hair, *body, *arm0, *arm1, *leg0, *leg1, *ear, *cloak, *bodyWear, *arm0Wear, *arm1Wear, *leg0Wear, *leg1Wear;

	bool hasLayer2;
	bool isSlimArms;
	int m_ePlayerModelType;

	int holdingLeftHand;
    int holdingRightHand;
	bool idle;
	bool sneaking;
	bool bowAndArrow;
	bool eating;
	float eating_t;
	float eating_swing;
	unsigned int m_uiAnimOverrideBitmask;
	float m_fYOffset;
	enum animbits
	{
		eAnim_ArmsDown =0,
		eAnim_ArmsOutFront,
		eAnim_NoLegAnim,
		eAnim_HasIdle,
		eAnim_ForceAnim,
		eAnim_SingleLegs,
		eAnim_SingleArms,
		eAnim_StatueOfLiberty,
		eAnim_DontRenderArmour,
		eAnim_NoBobbing,
		eAnim_DisableRenderHead,
		eAnim_DisableRenderArm0,
		eAnim_DisableRenderArm1,
		eAnim_DisableRenderTorso,
		eAnim_DisableRenderLeg0,
		eAnim_DisableRenderLeg1,
		eAnim_DisableRenderHair,
		eAnim_SmallModel
	};

	static const unsigned int m_staticBitmaskIgnorePlayerCustomAnimSetting= (1<<HumanoidModel::eAnim_ForceAnim) |
		(1<<HumanoidModel::eAnim_DisableRenderArm0) |
		(1<<HumanoidModel::eAnim_DisableRenderArm1) |
		(1<<HumanoidModel::eAnim_DisableRenderTorso) |
		(1<<HumanoidModel::eAnim_DisableRenderLeg0) |
		(1<<HumanoidModel::eAnim_DisableRenderLeg1) |
		(1<<HumanoidModel::eAnim_DisableRenderHair);

	void _init(float g, float yOffset, int texWidth, int texHeight, bool slimArms);
    HumanoidModel();
    HumanoidModel(float g);
    HumanoidModel(float g, float yOffset, int texWidth, int texHeight);
	HumanoidModel(float g, float yOffset, int texWidth, int texHeight, bool slimArms);
	void setPlayerModelType(int type);
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
    virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);
    void renderHair(float scale, bool usecompiled);
    void renderEars(float scale, bool usecompiled);
    void renderCloak(float scale, bool usecompiled); 
    void render(HumanoidModel *model, float scale, bool usecompiled);

	ModelPart * AddOrRetrievePart(SKIN_BOX *pBox);
};
