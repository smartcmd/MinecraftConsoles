#include "stdafx.h"
#include "..\..\Minecraft.h"
#include "..\..\ScreenSizeCalculator.h"
#include "..\..\EntityRenderDispatcher.h"
#include "..\..\PlayerRenderer.h"
#include "..\..\HumanoidModel.h"
#include "..\..\Lighting.h"
#include "..\..\ModelPart.h"
#include "..\..\Options.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "UIControl_PlayerSkinPreview.h"

//#define SKIN_PREVIEW_BOB_ANIM
#define SKIN_PREVIEW_WALKING_ANIM

UIControl_PlayerSkinPreview::UIControl_PlayerSkinPreview()
{
	UIControl::setControlType(UIControl::ePlayerSkinPreview);
	m_bDirty = FALSE;
	m_fScale = 1.0f;
	m_fAlpha = 1.0f;

	Minecraft *pMinecraft=Minecraft::GetInstance();

	ScreenSizeCalculator ssc(pMinecraft->options, pMinecraft->width_phys, pMinecraft->height_phys);
	m_fScreenWidth=static_cast<float>(pMinecraft->width_phys);
	m_fRawWidth=static_cast<float>(ssc.rawWidth);
	m_fScreenHeight=static_cast<float>(pMinecraft->height_phys);
	m_fRawHeight=static_cast<float>(ssc.rawHeight);

	m_customTextureUrl = L"default";
	m_backupTexture = TN_MOB_CHAR;
	m_capeTextureUrl = L"";

	m_yRot = 0;
	m_xRot = 0;

	m_swingTime = 0.0f;
	m_bobTick = 0.0f;
	m_walkAnimSpeedO = 0.0f;
	m_walkAnimSpeed = 0.0f;
	m_walkAnimPos = 0.0f;

	m_bAutoRotate = false;
	m_bRotatingLeft = false;

	m_incXRot = false;
	m_decXRot = false;
	m_incYRot = false;
	m_decYRot = false;

	m_currentAnimation = e_SkinPreviewAnimation_Walking;

	m_fTargetRotation = 0.0f;
	m_fOriginalRotation = 0.0f;
	m_framesAnimatingRotation = 0;
	m_bAnimatingToFacing = false;
	m_pvAdditionalModelParts=nullptr;
	m_uiAnimOverrideBitmask=0L;
}

void UIControl_PlayerSkinPreview::tick()
{
	UIControl::tick();

	if( m_bAnimatingToFacing )
	{
		++m_framesAnimatingRotation;
		m_yRot = m_fOriginalRotation + m_framesAnimatingRotation * ( (m_fTargetRotation - m_fOriginalRotation) / CHANGING_SKIN_FRAMES );

		//if(m_framesAnimatingRotation == CHANGING_SKIN_FRAMES) m_bAnimatingToFacing = false;
	}
	else
	{
		if( m_incXRot ) IncrementXRotation();
		if( m_decXRot ) DecrementXRotation();
		if( m_incYRot ) IncrementYRotation();
		if( m_decYRot ) DecrementYRotation();

		if(m_bAutoRotate)
		{
			++m_rotateTick;

			if(m_rotateTick%4==0)
			{
				if(m_yRot >= LOOK_LEFT_EXTENT)
				{
					m_bRotatingLeft = false;
				}
				else if(m_yRot <= LOOK_RIGHT_EXTENT)
				{
					m_bRotatingLeft = true;
				}

				if(m_bRotatingLeft)
				{
					IncrementYRotation();
				}
				else
				{
					DecrementYRotation();
				}
			}
		}
	}
}

void UIControl_PlayerSkinPreview::SetTexture(const wstring &url, TEXTURE_NAME backupTexture)
{
	m_customTextureUrl = url;
	m_backupTexture = backupTexture;

	unsigned int uiAnimOverrideBitmask = Player::getSkinAnimOverrideBitmask( app.getSkinIdFromPath(m_customTextureUrl) );

	if(app.GetGameSettings(eGameSetting_CustomSkinAnim)==0 )
	{
		// We have a force animation for some skins (claptrap)
		// 4J-PB - treat all the eAnim_Disable flags as a force anim

		if((uiAnimOverrideBitmask & HumanoidModel::m_staticBitmaskIgnorePlayerCustomAnimSetting)!=0)
		{
			m_uiAnimOverrideBitmask=uiAnimOverrideBitmask;
		}
		else
		{
			m_uiAnimOverrideBitmask=0;
		}
	}
	else
	{
		m_uiAnimOverrideBitmask = uiAnimOverrideBitmask;
	}

	m_pvAdditionalModelParts=app.GetAdditionalModelParts(app.getSkinIdFromPath(m_customTextureUrl));
}

void UIControl_PlayerSkinPreview::SetFacing(ESkinPreviewFacing facing, bool bAnimate /*= false*/)
{
	switch(facing)
	{
	case e_SkinPreviewFacing_Forward:
		m_fTargetRotation = 0;
		m_bRotatingLeft = true;
		break;
	case e_SkinPreviewFacing_Left:
		m_fTargetRotation = LOOK_LEFT_EXTENT;
		m_bRotatingLeft = false;
		break;
	case e_SkinPreviewFacing_Right:
		m_fTargetRotation = LOOK_RIGHT_EXTENT;
		m_bRotatingLeft = true;
		break;
	}

	if(!bAnimate)
	{
		m_yRot = m_fTargetRotation;
		m_bAnimatingToFacing = false;
	}
	else
	{
		m_fOriginalRotation = m_yRot;
		m_bAnimatingToFacing = true;
		m_framesAnimatingRotation = 0;
	}
}

void UIControl_PlayerSkinPreview::CycleNextAnimation()
{
	m_currentAnimation = static_cast<ESkinPreviewAnimations>(m_currentAnimation + 1);
	if(m_currentAnimation >= e_SkinPreviewAnimation_Count) m_currentAnimation = e_SkinPreviewAnimation_Walking;

	m_swingTime = 0.0f;
}

void UIControl_PlayerSkinPreview::CyclePreviousAnimation()
{
	m_currentAnimation = static_cast<ESkinPreviewAnimations>(m_currentAnimation - 1);
	if(m_currentAnimation < e_SkinPreviewAnimation_Walking) m_currentAnimation = static_cast<ESkinPreviewAnimations>(e_SkinPreviewAnimation_Count - 1);

	m_swingTime = 0.0f;
}

void UIControl_PlayerSkinPreview::render(IggyCustomDrawCallbackRegion *region)
{
	Minecraft *pMinecraft=Minecraft::GetInstance();

	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_COLOR_MATERIAL);
	glPushMatrix();

	float width = region->x1 - region->x0;
	float height = region->y1 - region->y0;
	float xo = width/2;
	float yo = height;

	glTranslatef(xo, yo - 3.5f, 50.0f);
	//glTranslatef(120.0f, 294, 0.0f);

	float ss;

	// Base scale on height of this control
	// Potentially we might want separate x & y scales here
	ss = width / (m_fScreenWidth / m_fScreenHeight);

	glScalef(-ss, ss, ss);
	glRotatef(180, 0, 0, 1);

	//glRotatef(45 + 90, 0, 1, 0);
	Lighting::turnOn();
	//glRotatef(-45 - 90, 0, 1, 0);

	glRotatef(-static_cast<float>(m_xRot), 1, 0, 0);

	// 4J Stu - Turning on hideGui while we do this stops the name rendering in split-screen
	bool wasHidingGui = pMinecraft->options->hideGui;
	pMinecraft->options->hideGui = true;

	//EntityRenderDispatcher::instance->render(pMinecraft->localplayers[0], 0, 0, 0, 0, 1);
	EntityRenderer *renderer = EntityRenderDispatcher::instance->getRenderer(eTYPE_LOCALPLAYER);
	if (renderer != nullptr)
	{
		// 4J-PB - any additional parts to turn on for this player (skin dependent)
		//vector<ModelPart *> *pAdditionalModelParts=mob->GetAdditionalModelParts();

		if(m_pvAdditionalModelParts && m_pvAdditionalModelParts->size()!=0)
		{
			for(auto& pModelPart : *m_pvAdditionalModelParts)
			{
				pModelPart->visible=true;
			}
		}

		render(renderer,0,0,0,0,1);
		//renderer->postRender(entity, x, y, z, rot, a);

		// hide the additional parts
		if(m_pvAdditionalModelParts && m_pvAdditionalModelParts->size()!=0)
		{
			for(auto& pModelPart : *m_pvAdditionalModelParts)
			{
				pModelPart->visible=false;
			}
		}
	}

	pMinecraft->options->hideGui = wasHidingGui;

	glPopMatrix();
	Lighting::turnOff();
	glDisable(GL_RESCALE_NORMAL);
}

// 4J Stu - Modified version of MobRenderer::render that does not require an actual entity
void UIControl_PlayerSkinPreview::render(EntityRenderer *renderer, double x, double y, double z, float rot, float a)
{
	glPushMatrix();
	glDisable(GL_CULL_FACE);

	HumanoidModel *model = static_cast<HumanoidModel *>(renderer->getModel());

	//getAttackAnim(mob, a);
	//if (armor != nullptr) armor->attackTime = model->attackTime;
	//model->riding = mob->isRiding();
	//if (armor != nullptr) armor->riding = model->riding;

	// 4J Stu - Remember to reset these values once the rendering is done if you add another one
	model->attackTime = 0;
	model->sneaking = false;
	model->holdingRightHand = false;
	model->holdingLeftHand = false;
	model->idle = false;
	model->eating = false;
	model->eating_swing = 0;
	model->eating_t = 0;
	model->young = false;
	model->riding = false;

	model->m_uiAnimOverrideBitmask = m_uiAnimOverrideBitmask;

	if( !m_bAnimatingToFacing )
	{
		switch( m_currentAnimation )
		{
		case e_SkinPreviewAnimation_Sneaking:
			model->sneaking = true;
			break;
		case e_SkinPreviewAnimation_Attacking:
			model->holdingRightHand = true;
			m_swingTime++;
			if (m_swingTime >= (Player::SWING_DURATION * 3) )
			{
				m_swingTime = 0;
			}
			model->attackTime = m_swingTime / static_cast<float>(Player::SWING_DURATION * 3);
			break;
		default:
			break;
		};
	}


	float bodyRot = m_yRot; //(mob->yBodyRotO + (mob->yBodyRot - mob->yBodyRotO) * a);
	float headRot = m_yRot; //(mob->yRotO + (mob->yRot - mob->yRotO) * a);
	float headRotx = 0; //(mob->xRotO + (mob->xRot - mob->xRotO) * a);

	//setupPosition(mob, x, y, z);
	// is equivalent to
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));

	//float bob = getBob(mob, a);
#ifdef SKIN_PREVIEW_BOB_ANIM
	float bob = (m_bobTick + a)/2;

	++m_bobTick;
	if(m_bobTick>=360*2) m_bobTick = 0;
#else
	float bob = 0.0f;
#endif

	//setupRotations(mob, bob, bodyRot, a);
	// is equivalent to
	glRotatef(180 - bodyRot, 0, 1, 0);

	float _scale = 1 / 16.0f;
	glEnable(GL_RESCALE_NORMAL);
	glScalef(-1, -1, 1);

	//scale(mob, a);
	// is equivalent to
	float s = 15 / 16.0f;
	glScalef(s, s, s);

	// 4J - TomK - pull up character a bit more to make sure extra geo around feet doesn't cause rendering problems on PSVita
#ifdef __PSVITA__
	glTranslatef(0, -24 * _scale - 1.0f / 16.0f, 0);
#else
	glTranslatef(0, -24 * _scale - 0.125f / 16.0f, 0);
#endif

#ifdef SKIN_PREVIEW_WALKING_ANIM
	m_walkAnimSpeedO = m_walkAnimSpeed;
	m_walkAnimSpeed += (0.1f - m_walkAnimSpeed) * 0.4f;
	m_walkAnimPos += m_walkAnimSpeed;
	float ws = m_walkAnimSpeedO + (m_walkAnimSpeed - m_walkAnimSpeedO) * a;
	float wp = m_walkAnimPos - m_walkAnimSpeed * (1 - a);
#else
	float ws = 0;
	float wp = 0;
#endif

	if (ws > 1) ws = 1;

	MemSect(31);
	bindTexture(m_customTextureUrl, m_backupTexture);
	MemSect(0);
	glEnable(GL_ALPHA_TEST);

	//model->prepareMobModel(mob, wp, ws, a);
	model->render(nullptr, wp, ws, bob, headRot - bodyRot, headRotx, _scale, true);
	/*for (int i = 0; i < MAX_ARMOR_LAYERS; i++)
	{
	if (prepareArmor(mob, i, a))
	{
	armor->render(wp, ws, bob, headRot - bodyRot, headRotx, _scale, true);
	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	}
	}*/

	//additionalRendering(mob, a);
	if (bindTexture(m_capeTextureUrl, L"" ))
	{
		glPushMatrix();
		glTranslatef(0, 0, 2 / 16.0f);

		double xd = 0;//(mob->xCloakO + (mob->xCloak - mob->xCloakO) * a) - (mob->xo + (mob->x - mob->xo) * a);
		double yd = 0;//(mob->yCloakO + (mob->yCloak - mob->yCloakO) * a) - (mob->yo + (mob->y - mob->yo) * a);
		double zd = 0;//(mob->zCloakO + (mob->zCloak - mob->zCloakO) * a) - (mob->zo + (mob->z - mob->zo) * a);

		float yr = 1;//mob->yBodyRotO + (mob->yBodyRot - mob->yBodyRotO) * a;

		double xa = sin(yr * PI / 180);
		double za = -cos(yr * PI / 180);

		float flap = static_cast<float>(yd) * 10;
		if (flap < -6) flap = -6;
		if (flap > 32) flap = 32;
		float lean = static_cast<float>(xd * xa + zd * za) * 100;
		float lean2 = static_cast<float>(xd * za - zd * xa) * 100;
		if (lean < 0) lean = 0;

		//float pow = 1;//mob->oBob + (bob - mob->oBob) * a;

		flap += 1;//sin((mob->walkDistO + (mob->walkDist - mob->walkDistO) * a) * 6) * 32 * pow;
		if (model->sneaking)
		{
			flap += 25;
		}

		glRotatef(6.0f + lean / 2 + flap, 1, 0, 0);
		glRotatef(lean2 / 2, 0, 0, 1);
		glRotatef(-lean2 / 2, 0, 1, 0);
		glRotatef(180, 0, 1, 0);
		model->renderCloak(1 / 16.0f,true);
		glPopMatrix();
	}
	/*
	float br = mob->getBrightness(a);
	int overlayColor = getOverlayColor(mob, br, a);

	if (((overlayColor >> 24) & 0xff) > 0 || mob->hurtTime > 0 || mob->deathTime > 0)
	{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_EQUAL);

	// 4J - changed these renders to not use the compiled version of their models, because otherwise the render states set
	// about (in particular the depth & alpha test) don't work with our command buffer versions
	if (mob->hurtTime > 0 || mob->deathTime > 0)
	{
	glColor4f(br, 0, 0, 0.4f);
	model->render(wp, ws, bob, headRot - bodyRot, headRotx, _scale, false);
	for (int i = 0; i < MAX_ARMOR_LAYERS; i++)
	{
	if (prepareArmorOverlay(mob, i, a))
	{
	glColor4f(br, 0, 0, 0.4f);
	armor->render(wp, ws, bob, headRot - bodyRot, headRotx, _scale, false);
	}
	}
	}

	if (((overlayColor >> 24) & 0xff) > 0)
	{
	float r = ((overlayColor >> 16) & 0xff) / 255.0f;
	float g = ((overlayColor >> 8) & 0xff) / 255.0f;
	float b = ((overlayColor) & 0xff) / 255.0f;
	float aa = ((overlayColor >> 24) & 0xff) / 255.0f;
	glColor4f(r, g, b, aa);
	model->render(wp, ws, bob, headRot - bodyRot, headRotx, _scale, false);
	for (int i = 0; i < MAX_ARMOR_LAYERS; i++)
	{
	if (prepareArmorOverlay(mob, i, a))
	{
	glColor4f(r, g, b, aa);
	armor->render(wp, ws, bob, headRot - bodyRot, headRotx, _scale, false);
	}
	}
	}

	glDepthFunc(GL_LEQUAL);
	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);
	}
	*/
	glDisable(GL_RESCALE_NORMAL);

	glEnable(GL_CULL_FACE);

	glPopMatrix();

	MemSect(31);
	//renderName(mob, x, y, z);
	MemSect(0);

	// Reset the model values to stop the changes we made here affecting anything in game (like the player hand render)
	model->attackTime = 0;
	model->sneaking = false;
	model->holdingRightHand = false;
	model->holdingLeftHand = false;
}

bool UIControl_PlayerSkinPreview::bindTexture(const wstring& urlTexture, int backupTexture)
{
	Textures *t = Minecraft::GetInstance()->textures;

	// 4J-PB - no http textures on the xbox, mem textures instead

	//int id = t->loadHttpTexture(urlTexture, backupTexture);
	int id = t->loadMemTexture(urlTexture, backupTexture);

	if (id >= 0)
	{
		t->bind(id);
		return true;
	}
	else
	{
		return false;
	}
}

bool UIControl_PlayerSkinPreview::bindTexture(const wstring& urlTexture, const wstring& backupTexture)
{
	Textures *t = Minecraft::GetInstance()->textures;

	// 4J-PB - no http textures on the xbox, mem textures instead

	//int id = t->loadHttpTexture(urlTexture, backupTexture);
	int id = t->loadMemTexture(urlTexture, backupTexture);

	if (id >= 0)
	{
		t->bind(id);
		return true;
	}
	else
	{
		return false;
	}
}
