#include "stdafx.h"
#include "HumanoidModel.h"
#include "..\Minecraft.World\Mth.h"
#include "..\Minecraft.World\Entity.h"
#include "ModelPart.h"

// 4J added 

ModelPart * HumanoidModel::AddOrRetrievePart(SKIN_BOX *pBox)
{
	ModelPart *pAttachTo = NULL;

	switch(pBox->ePart)
	{
	case eBodyPart_Head:
		pAttachTo = head;
		break;
	case eBodyPart_Body:
		pAttachTo = body;
		break;
	case eBodyPart_Arm0:
		pAttachTo = arm0;
		break;
	case eBodyPart_Arm1:
		pAttachTo = arm1;
		break;
	case eBodyPart_Leg0:
		pAttachTo = leg0;
		break;
	case eBodyPart_Leg1:
		pAttachTo = leg1;
		break;
	}

	// first check this box doesn't already exist
	ModelPart *pNewBox = pAttachTo->retrieveChild(pBox);

	if(pNewBox)
	{
		if((pNewBox->getfU() != (int)pBox->fU) || (pNewBox->getfV() != (int)pBox->fV))
		{
			app.DebugPrintf("HumanoidModel::AddOrRetrievePart - Box geometry was found, but with different uvs\n");
			pNewBox = NULL;
		}
	}
	if(pNewBox == NULL)
	{
		pNewBox = new ModelPart(this, (int)pBox->fU, (int)pBox->fV);
		pNewBox->visible = false;
		pNewBox->addHumanoidBox(pBox->fX, pBox->fY, pBox->fZ, pBox->fW, pBox->fH, pBox->fD, 0); 
		pAttachTo->addChild(pNewBox);
	}

	return pNewBox;
}

void HumanoidModel::_init(float g, float yOffset, int texWidth, int texHeight, bool slimArms)
{
	isSlimArms = slimArms;
	m_ePlayerModelType = slimArms ? ePlayerModel_Alex : ePlayerModel_Steve;

	this->texWidth = texWidth;
	this->texHeight = texHeight;

	m_fYOffset = yOffset;
	hasLayer2 = (texWidth == 64 && texHeight == 64);

	bodyWear = NULL;
	arm0Wear = NULL;
	arm1Wear = NULL;
	leg0Wear = NULL;
	leg1Wear = NULL;

    cloak = new ModelPart(this, 0, 0);
    cloak->addHumanoidBox(-5, 0, -1, 10, 16, 1, g); // Cloak

    ear = new ModelPart(this, 24, 0);
    ear->addHumanoidBox(-3, -6, -1, 6, 6, 1, g); // Ear
        
    head = new ModelPart(this, 0, 0);
    head->addHumanoidBox(-4, -8, -4, 8, 8, 8, g); // Head
    head->setPos(0, 0 + yOffset, 0);

    hair = new ModelPart(this, 32, 0);
    hair->addHumanoidBox(-4, -8, -4, 8, 8, 8, g + 0.5f); // Head overlay
    hair->setPos(0, 0 + yOffset, 0);

    body = new ModelPart(this, 16, 16);
    body->addHumanoidBox(-4, 0, -2, 8, 12, 4, g); // Body
    body->setPos(0, 0 + yOffset, 0);

    arm0 = new ModelPart(this, 40, 16);
	arm1 = new ModelPart(this, 40, 16);

	if (isSlimArms)
	{
		// Alex
		arm0->addHumanoidBox(-2, -2, -2, 3, 12, 4, g);
		arm0->setPos(-5.0f, 2.0f + yOffset, 0.0f);

		arm1->mirror();
		arm1->addHumanoidBox(-1, -2, -2, 3, 12, 4, g);
		arm1->setPos(5.0f, 2.0f + yOffset, 0.0f);
	}
	else
	{
		// Steve
		arm0->addHumanoidBox(-3, -2, -2, 4, 12, 4, g);
		arm0->setPos(-5.0f, 2.0f + yOffset, 0.0f);

		arm1->mirror();
		arm1->addHumanoidBox(-1, -2, -2, 4, 12, 4, g);
		arm1->setPos(5.0f, 2.0f + yOffset, 0.0f);
	}

    leg0 = new ModelPart(this, 0, 16);
    leg0->addHumanoidBox(-2, 0, -2, 4, 12, 4, g); // Leg0
    leg0->setPos(-1.9f, 12 + yOffset, 0);

    leg1 = new ModelPart(this, 0, 16);
    leg1->bMirror = true;
    leg1->addHumanoidBox(-2, 0, -2, 4, 12, 4, g); // Leg1
    leg1->setPos(1.9f, 12 + yOffset, 0);

	// second layer / wear layer only for 64x64 skins
	if (hasLayer2)
	{
		bodyWear = new ModelPart(this, 16, 32);
		bodyWear->addHumanoidBox(-4, 0, -2, 8, 12, 4, g + 0.25f);
		bodyWear->setPos(0.0f, 0.0f + yOffset, 0.0f);

		arm0Wear = new ModelPart(this, 40, 32);
		arm1Wear = new ModelPart(this, 40, 32);

		if (isSlimArms)
		{
			arm0Wear->addHumanoidBox(-2, -2, -2, 3, 12, 4, g + 0.25f);
			arm0Wear->setPos(-5.0f, 2.0f + yOffset, 0.0f);

			arm1Wear->mirror();
			arm1Wear->addHumanoidBox(-1, -2, -2, 3, 12, 4, g + 0.25f);
			arm1Wear->setPos(5.0f, 2.0f + yOffset, 0.0f);
		}
		else
		{
			arm0Wear->addHumanoidBox(-3, -2, -2, 4, 12, 4, g + 0.25f);
			arm0Wear->setPos(-5.0f, 2.0f + yOffset, 0.0f);

			arm1Wear->mirror();
			arm1Wear->addHumanoidBox(-1, -2, -2, 4, 12, 4, g + 0.25f);
			arm1Wear->setPos(5.0f, 2.0f + yOffset, 0.0f);
		}

		leg0Wear = new ModelPart(this, 0, 32);
		leg0Wear->addHumanoidBox(-2, 0, -2, 4, 12, 4, g + 0.25f);
		leg0Wear->setPos(-1.9f, 12.0f + yOffset, 0.0f);

		leg1Wear = new ModelPart(this, 0, 48);
		leg1Wear->mirror();
		leg1Wear->addHumanoidBox(-2, 0, -2, 4, 12, 4, g + 0.25f);
		leg1Wear->setPos(1.9f, 12.0f + yOffset, 0.0f);
	}

	// compile now to avoid first-render hitch
	cloak->compile(1.0f / 16.0f);
	ear->compile(1.0f / 16.0f);
	head->compile(1.0f / 16.0f);
	body->compile(1.0f / 16.0f);
	arm0->compile(1.0f / 16.0f);
	arm1->compile(1.0f / 16.0f);
	leg0->compile(1.0f / 16.0f);
	leg1->compile(1.0f / 16.0f);
	hair->compile(1.0f / 16.0f);

	if(bodyWear) bodyWear->compile(1.0f / 16.0f);
	if(arm0Wear) arm0Wear->compile(1.0f / 16.0f);
	if(arm1Wear) arm1Wear->compile(1.0f / 16.0f);
	if(leg0Wear) leg0Wear->compile(1.0f / 16.0f);
	if(leg1Wear) leg1Wear->compile(1.0f / 16.0f);

	holdingLeftHand = 0;
	holdingRightHand = 0;
	sneaking = false;
	idle = false;
	bowAndArrow = false;

	eating = false;
	eating_t = 0.0f;
	eating_swing = 0.0f;
	m_uiAnimOverrideBitmask = 0L;
}

HumanoidModel::HumanoidModel()
{
	_init(0.0f, 0.0f, 64, 32, false);
}

HumanoidModel::HumanoidModel(float g)
{
	_init(g, 0.0f, 64, 32, false);
}

HumanoidModel::HumanoidModel(float g, float yOffset, int texWidth, int texHeight)
{
	_init(g, yOffset, texWidth, texHeight, false);
}

HumanoidModel::HumanoidModel(float g, float yOffset, int texWidth, int texHeight, bool slimArms)
{
	_init(g, yOffset, texWidth, texHeight, slimArms);
}

void HumanoidModel::setPlayerModelType(int type)
{
	m_ePlayerModelType = type;
	isSlimArms = (type == ePlayerModel_Alex);
}

void HumanoidModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{	
	if(entity != NULL)
	{
		m_uiAnimOverrideBitmask = entity->getAnimOverrideBitmask();
	}

	setupAnim(time, r, bob, yRot, xRot, scale, entity, m_uiAnimOverrideBitmask);

	if (young)
	{
		float ss = 2.0f;

		glPushMatrix();
		glScalef(1.5f / ss, 1.5f / ss, 1.5f / ss);
		glTranslatef(0, 16 * scale, 0);
		head->render(scale, usecompiled);
		hair->render(scale, usecompiled);
		glPopMatrix();

		glPushMatrix();
		glScalef(1 / ss, 1 / ss, 1 / ss);
		glTranslatef(0, 24 * scale, 0);
		body->render(scale, usecompiled);
		arm0->render(scale, usecompiled);
		arm1->render(scale, usecompiled);
		leg0->render(scale, usecompiled);
		leg1->render(scale, usecompiled);

		if(bodyWear) bodyWear->render(scale, usecompiled);
		if(arm0Wear) arm0Wear->render(scale, usecompiled);
		if(arm1Wear) arm1Wear->render(scale, usecompiled);
		if(leg0Wear) leg0Wear->render(scale, usecompiled);
		if(leg1Wear) leg1Wear->render(scale, usecompiled);

		glPopMatrix();
	}
	else
	{
		head->render(scale, usecompiled, (m_uiAnimOverrideBitmask & (1 << eAnim_DisableRenderHead)) > 0);
		body->render(scale, usecompiled, (m_uiAnimOverrideBitmask & (1 << eAnim_DisableRenderTorso)) > 0);
		arm0->render(scale, usecompiled, (m_uiAnimOverrideBitmask & (1 << eAnim_DisableRenderArm0)) > 0);
		arm1->render(scale, usecompiled, (m_uiAnimOverrideBitmask & (1 << eAnim_DisableRenderArm1)) > 0);
		leg0->render(scale, usecompiled, (m_uiAnimOverrideBitmask & (1 << eAnim_DisableRenderLeg0)) > 0);
		leg1->render(scale, usecompiled, (m_uiAnimOverrideBitmask & (1 << eAnim_DisableRenderLeg1)) > 0);
		hair->render(scale, usecompiled, (m_uiAnimOverrideBitmask & (1 << eAnim_DisableRenderHair)) > 0);

		if(bodyWear) bodyWear->render(scale, usecompiled);
		if(arm0Wear) arm0Wear->render(scale, usecompiled);
		if(arm1Wear) arm1Wear->render(scale, usecompiled);
		if(leg0Wear) leg0Wear->render(scale, usecompiled);
		if(leg1Wear) leg1Wear->render(scale, usecompiled);
	}
}

void HumanoidModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	head->yRot = yRot / (float)(180.0f / PI);
	head->xRot = xRot / (float)(180.0f / PI);
	hair->yRot = head->yRot;
	hair->xRot = head->xRot;
	body->z = 0.0f;

	// Does the skin have an override for anim?
	if(uiBitmaskOverrideAnim & (1 << eAnim_ArmsDown))
	{
		arm0->xRot = 0.0f;
		arm1->xRot = 0.0f;
		arm0->zRot = 0.0f;
		arm1->zRot = 0.0f;
	}
	else if(uiBitmaskOverrideAnim & (1 << eAnim_ArmsOutFront))
	{
		arm0->xRot = -HALF_PI;
		arm1->xRot = -HALF_PI;
		arm0->zRot = 0.0f;
		arm1->zRot = 0.0f;
	}
	else if(uiBitmaskOverrideAnim & (1 << eAnim_SingleArms))
	{
		arm0->xRot = (Mth::cos(time * 0.6662f + PI) * 2.0f) * r * 0.5f;
		arm1->xRot = (Mth::cos(time * 0.6662f + PI) * 2.0f) * r * 0.5f;
		arm0->zRot = 0.0f;
		arm1->zRot = 0.0f;
	}
	else if((uiBitmaskOverrideAnim & (1 << eAnim_StatueOfLiberty)) && (holdingRightHand == 0) && (attackTime == 0.0f))
	{
		arm0->xRot = -PI;
		arm0->zRot = -0.3f;
		arm1->xRot = (Mth::cos(time * 0.6662f) * 2.0f) * r * 0.5f;
		arm1->zRot = 0.0f;
	}
	else
	{
		arm0->xRot = (Mth::cos(time * 0.6662f + PI) * 2.0f) * r * 0.5f;
		arm1->xRot = (Mth::cos(time * 0.6662f) * 2.0f) * r * 0.5f;
		arm0->zRot = 0.0f;
		arm1->zRot = 0.0f;
	}

	leg0->yRot = 0.0f;
	leg1->yRot = 0.0f;

	if (riding)
	{
		if((uiBitmaskOverrideAnim & (1 << eAnim_SmallModel)) == 0)
		{
			arm0->xRot += -HALF_PI * 0.4f;
			arm1->xRot += -HALF_PI * 0.4f;
			leg0->xRot = -HALF_PI * 0.8f;
			leg1->xRot = -HALF_PI * 0.8f;
			leg0->yRot = HALF_PI * 0.2f;
			leg1->yRot = -HALF_PI * 0.2f;
		}
		else
		{
			arm0->xRot += -HALF_PI * 0.4f;
			arm1->xRot += -HALF_PI * 0.4f;
			leg0->xRot = -HALF_PI * 0.4f;
			leg1->xRot = -HALF_PI * 0.4f;
		}
	}
	else if(idle && !sneaking)
	{
		leg0->xRot = -HALF_PI;
		leg1->xRot = -HALF_PI;
		leg0->yRot = HALF_PI * 0.2f;
		leg1->yRot = -HALF_PI * 0.2f;
	}
	else if(uiBitmaskOverrideAnim & (1 << eAnim_NoLegAnim))
	{
		leg0->xRot = 0.0f;
		leg0->zRot = 0.0f;
		leg1->xRot = 0.0f;
		leg1->zRot = 0.0f;
		leg0->yRot = 0.0f;
		leg1->yRot = 0.0f;		
	}
	else if(uiBitmaskOverrideAnim & (1 << eAnim_SingleLegs))
	{
		leg0->xRot = (Mth::cos(time * 0.6662f) * 1.4f) * r;
		leg1->xRot = (Mth::cos(time * 0.6662f) * 1.4f) * r;
	}
	else
	{
		leg0->xRot = (Mth::cos(time * 0.6662f) * 1.4f) * r;
		leg1->xRot = (Mth::cos(time * 0.6662f + PI) * 1.4f) * r;
	}

	if (holdingLeftHand != 0) 
	{
		arm1->xRot = arm1->xRot * 0.5f - HALF_PI * 0.2f * holdingLeftHand;
	}
	if (holdingRightHand != 0) 
	{
		arm0->xRot = arm0->xRot * 0.5f - HALF_PI * 0.2f * holdingRightHand;
	}

	arm0->yRot = 0.0f;
	arm1->yRot = 0.0f;

	if (attackTime > -9990.0f)
	{
		float swing = attackTime;
		body->yRot = Mth::sin(sqrt(swing) * PI * 2.0f) * 0.2f;
		arm0->z = Mth::sin(body->yRot) * 5.0f;
		arm0->x = -Mth::cos(body->yRot) * 5.0f;
		arm1->z = -Mth::sin(body->yRot) * 5.0f;
		arm1->x = Mth::cos(body->yRot) * 5.0f;
		arm0->yRot += body->yRot;
		arm1->yRot += body->yRot;
		arm1->xRot += body->yRot;

		swing = 1.0f - attackTime;
		swing *= swing;
		swing *= swing;
		swing = 1.0f - swing;
		float aa = Mth::sin(swing * PI);
		float bb = Mth::sin(attackTime * PI) * -(head->xRot - 0.7f) * 0.75f;
		arm0->xRot -= aa * 1.2f + bb;
		arm0->yRot += body->yRot * 2.0f;         

		if((uiBitmaskOverrideAnim & (1 << eAnim_StatueOfLiberty)) && (holdingRightHand == 0) && (attackTime == 0.0f))
		{
			arm0->zRot -= Mth::sin(attackTime * PI) * -0.4f;
		}
		else
		{
			arm0->zRot = Mth::sin(attackTime * PI) * -0.4f;
		}
	}

	if(eating)
	{
		float is = 1 - eating_swing;
		is = is * is * is;
		is = is * is * is;
		is = is * is * is;
		float iss = 1 - is;
		arm0->xRot = -Mth::abs(Mth::cos(eating_t / 4.0f * PI) * 0.1f) * (eating_swing > 0.2 ? 1.0f : 0.0f) * 2.0f;
		arm0->yRot -= iss * 0.5f;
		arm0->xRot -= iss * 1.2f;
	}

	if (sneaking)
	{
		if(uiBitmaskOverrideAnim & (1 << eAnim_SmallModel))
		{
			body->xRot = -0.5f;
			arm0->xRot += 0.4f;
			arm1->xRot += 0.4f;
			leg0->z = -4.0f;
			leg1->z = -4.0f;
			body->z = 2.0f;
			body->y = 0.0f;
			arm0->y = 2.0f;
			arm1->y = 2.0f;
			leg0->y = 9.0f;
			leg1->y = 9.0f;
			head->y = 1.0f;
			hair->y = 1.0f;
			ear->y = 1.0f;
			cloak->y = 0.0f;
		}
		else
		{
			body->xRot = 0.5f;
			arm0->xRot += 0.4f;
			arm1->xRot += 0.4f;
			leg0->z = 4.0f;
			leg1->z = 4.0f;
			body->y = 0.0f;
			arm0->y = 2.0f;
			arm1->y = 2.0f;
			leg0->y = 9.0f;
			leg1->y = 9.0f;
			head->y = 1.0f;
			hair->y = 1.0f;
			ear->y = 1.0f;
			cloak->y = 0.0f;
		}
	}
	else
	{
		body->xRot = 0.0f;
		leg0->z = 0.1f;
		leg1->z = 0.1f;

		if(!riding && idle)
		{
			leg0->y = 22.0f;
			leg1->y = 22.0f;
			body->y = 10.0f;
			arm0->y = 12.0f;
			arm1->y = 12.0f;
			head->y = 10.0f;
			hair->y = 10.0f;
			ear->y = 11.0f;
			cloak->y = 10.0f;
		}
		else
		{
			leg0->y = 12.0f;
			leg1->y = 12.0f;
			body->y = 0.0f;
			arm0->y = 2.0f;
			arm1->y = 2.0f;
			head->y = 0.0f;
			hair->y = 0.0f;
			ear->y = 1.0f;
			cloak->y = 0.0f;
		}
	}

	arm0->zRot += ((Mth::cos(bob * 0.09f)) * 0.05f + 0.05f);
	arm1->zRot -= ((Mth::cos(bob * 0.09f)) * 0.05f + 0.05f);
	arm0->xRot += ((Mth::sin(bob * 0.067f)) * 0.05f);
	arm1->xRot -= ((Mth::sin(bob * 0.067f)) * 0.05f);

	if (bowAndArrow) 
	{
		float attack2 = 0.0f;
		float attack = 0.0f;

		arm0->zRot = 0.0f;
		arm1->zRot = 0.0f;
		arm0->yRot = -(0.1f - attack2 * 0.6f) + head->yRot;
		arm1->yRot = +(0.1f - attack2 * 0.6f) + head->yRot + 0.4f;
		arm0->xRot = -HALF_PI + head->xRot;
		arm1->xRot = -HALF_PI + head->xRot;
		arm0->xRot -= attack2 * 1.2f - attack * 0.4f;
		arm1->xRot -= attack2 * 1.2f - attack * 0.4f;
		arm0->zRot += ((float)(Mth::cos(bob * 0.09f)) * 0.05f + 0.05f);
		arm1->zRot -= ((float)(Mth::cos(bob * 0.09f)) * 0.05f + 0.05f);
		arm0->xRot += ((float)(Mth::sin(bob * 0.067f)) * 0.05f);
		arm1->xRot -= ((float)(Mth::sin(bob * 0.067f)) * 0.05f);
	}

	// sync second layer after ALL animation is applied
	if(bodyWear)
	{
		bodyWear->xRot = body->xRot;
		bodyWear->yRot = body->yRot;
		bodyWear->zRot = body->zRot;
		bodyWear->x = body->x;
		bodyWear->y = body->y;
		bodyWear->z = body->z;

		arm0Wear->xRot = arm0->xRot;
		arm0Wear->yRot = arm0->yRot;
		arm0Wear->zRot = arm0->zRot;
		arm0Wear->x = arm0->x;
		arm0Wear->y = arm0->y;
		arm0Wear->z = arm0->z;

		arm1Wear->xRot = arm1->xRot;
		arm1Wear->yRot = arm1->yRot;
		arm1Wear->zRot = arm1->zRot;
		arm1Wear->x = arm1->x;
		arm1Wear->y = arm1->y;
		arm1Wear->z = arm1->z;

		leg0Wear->xRot = leg0->xRot;
		leg0Wear->yRot = leg0->yRot;
		leg0Wear->zRot = leg0->zRot;
		leg0Wear->x = leg0->x;
		leg0Wear->y = leg0->y;
		leg0Wear->z = leg0->z;

		leg1Wear->xRot = leg1->xRot;
		leg1Wear->yRot = leg1->yRot;
		leg1Wear->zRot = leg1->zRot;
		leg1Wear->x = leg1->x;
		leg1Wear->y = leg1->y;
		leg1Wear->z = leg1->z;
	}
}

void HumanoidModel::renderHair(float scale, bool usecompiled)
{
    hair->yRot = head->yRot;
    hair->xRot = head->xRot;
    hair->render(scale, usecompiled);
}

void HumanoidModel::renderEars(float scale, bool usecompiled)
{
    ear->yRot = head->yRot;
    ear->xRot = head->xRot;
    ear->x = 0;
    ear->y = 0;
    ear->render(scale, usecompiled);
}

void HumanoidModel::renderCloak(float scale, bool usecompiled)
{
	cloak->render(scale, usecompiled);
}

void HumanoidModel::render(HumanoidModel *model, float scale, bool usecompiled)
{
    head->yRot = model->head->yRot;
    head->y = model->head->y;
    head->xRot = model->head->xRot;

    hair->y = head->y;
    hair->yRot = head->yRot;
    hair->xRot = head->xRot;

    body->xRot = model->body->xRot;
    body->yRot = model->body->yRot;
    body->zRot = model->body->zRot;
    body->x = model->body->x;
    body->y = model->body->y;
    body->z = model->body->z;

    arm0->xRot = model->arm0->xRot;
    arm0->yRot = model->arm0->yRot;
    arm0->zRot = model->arm0->zRot;
    arm0->x = model->arm0->x;
    arm0->y = model->arm0->y;
    arm0->z = model->arm0->z;

    arm1->xRot = model->arm1->xRot;
    arm1->yRot = model->arm1->yRot;
    arm1->zRot = model->arm1->zRot;
    arm1->x = model->arm1->x;
    arm1->y = model->arm1->y;
    arm1->z = model->arm1->z;

    leg0->xRot = model->leg0->xRot;
    leg0->yRot = model->leg0->yRot;
    leg0->zRot = model->leg0->zRot;
    leg0->x = model->leg0->x;
    leg0->y = model->leg0->y;
    leg0->z = model->leg0->z;

    leg1->xRot = model->leg1->xRot;
    leg1->yRot = model->leg1->yRot;
    leg1->zRot = model->leg1->zRot;
    leg1->x = model->leg1->x;
    leg1->y = model->leg1->y;
    leg1->z = model->leg1->z;

	head->render(scale, usecompiled, (m_uiAnimOverrideBitmask & (1 << eAnim_DisableRenderHead)) > 0);
	body->render(scale, usecompiled, (m_uiAnimOverrideBitmask & (1 << eAnim_DisableRenderTorso)) > 0);
	arm0->render(scale, usecompiled, (m_uiAnimOverrideBitmask & (1 << eAnim_DisableRenderArm0)) > 0);
	arm1->render(scale, usecompiled, (m_uiAnimOverrideBitmask & (1 << eAnim_DisableRenderArm1)) > 0);
	leg0->render(scale, usecompiled, (m_uiAnimOverrideBitmask & (1 << eAnim_DisableRenderLeg0)) > 0);
	leg1->render(scale, usecompiled, (m_uiAnimOverrideBitmask & (1 << eAnim_DisableRenderLeg1)) > 0);
	hair->render(scale, usecompiled, (m_uiAnimOverrideBitmask & (1 << eAnim_DisableRenderHair)) > 0);

	if(bodyWear && model->bodyWear)
	{
		bodyWear->xRot = model->bodyWear->xRot;
		bodyWear->yRot = model->bodyWear->yRot;
		bodyWear->zRot = model->bodyWear->zRot;
		bodyWear->x = model->bodyWear->x;
		bodyWear->y = model->bodyWear->y;
		bodyWear->z = model->bodyWear->z;

		arm0Wear->xRot = model->arm0Wear->xRot;
		arm0Wear->yRot = model->arm0Wear->yRot;
		arm0Wear->zRot = model->arm0Wear->zRot;
		arm0Wear->x = model->arm0Wear->x;
		arm0Wear->y = model->arm0Wear->y;
		arm0Wear->z = model->arm0Wear->z;

		arm1Wear->xRot = model->arm1Wear->xRot;
		arm1Wear->yRot = model->arm1Wear->yRot;
		arm1Wear->zRot = model->arm1Wear->zRot;
		arm1Wear->x = model->arm1Wear->x;
		arm1Wear->y = model->arm1Wear->y;
		arm1Wear->z = model->arm1Wear->z;

		leg0Wear->xRot = model->leg0Wear->xRot;
		leg0Wear->yRot = model->leg0Wear->yRot;
		leg0Wear->zRot = model->leg0Wear->zRot;
		leg0Wear->x = model->leg0Wear->x;
		leg0Wear->y = model->leg0Wear->y;
		leg0Wear->z = model->leg0Wear->z;

		leg1Wear->xRot = model->leg1Wear->xRot;
		leg1Wear->yRot = model->leg1Wear->yRot;
		leg1Wear->zRot = model->leg1Wear->zRot;
		leg1Wear->x = model->leg1Wear->x;
		leg1Wear->y = model->leg1Wear->y;
		leg1Wear->z = model->leg1Wear->z;

		bodyWear->render(scale, usecompiled);
		arm0Wear->render(scale, usecompiled);
		arm1Wear->render(scale, usecompiled);
		leg0Wear->render(scale, usecompiled);
		leg1Wear->render(scale, usecompiled);
	}
}
