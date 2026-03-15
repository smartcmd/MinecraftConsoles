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

static const unsigned int ANIM_64x64_CLASSIC_SKIN = 0x00040000;
static const unsigned int ANIM_64x64_ALEX_SLIM_SKIN = 0x00080000;

UIControl_PlayerSkinPreview::UIControl_PlayerSkinPreview()
{
	UIControl::setControlType(UIControl::ePlayerSkinPreview);
	m_bDirty = FALSE;
	m_fScale = 1.0f;
	m_fAlpha = 1.0f;

	Minecraft *pMinecraft=Minecraft::GetInstance();

	ScreenSizeCalculator ssc(pMinecraft->options, pMinecraft->width_phys, pMinecraft->height_phys);
	m_fScreenWidth=(float)pMinecraft->width_phys;
	m_fRawWidth=(float)ssc.rawWidth;
	m_fScreenHeight=(float)pMinecraft->height_phys;
	m_fRawHeight=(float)ssc.rawHeight;

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
	m_pvAdditionalModelParts=NULL;
	m_uiAnimOverrideBitmask=0L;

	m_pPreviewModel = NULL;
	m_iPreviewModelType = HumanoidModel::ePlayerModel_Steve;
	m_iPreviewTexWidth = 64;
	m_iPreviewTexHeight = 32;
}

UIControl_PlayerSkinPreview::~UIControl_PlayerSkinPreview()
{
	DestroyPreviewModel();
}

void UIControl_PlayerSkinPreview::DestroyPreviewModel()
{
	if (m_pPreviewModel != NULL)
	{
		delete m_pPreviewModel;
		m_pPreviewModel = NULL;
	}
}

void UIControl_PlayerSkinPreview::RebuildPreviewModel()
{
	int iWantedModelType = HumanoidModel::ePlayerModel_Steve;
	int iWantedTexWidth = 64;
	int iWantedTexHeight = 32;

	if ((m_uiAnimOverrideBitmask & ANIM_64x64_ALEX_SLIM_SKIN) != 0)
	{
		iWantedModelType = HumanoidModel::ePlayerModel_Alex;
		iWantedTexHeight = 64;
	}
	else if ((m_uiAnimOverrideBitmask & ANIM_64x64_CLASSIC_SKIN) != 0)
	{
		iWantedModelType = HumanoidModel::ePlayerModel_Steve;
		iWantedTexHeight = 64;
	}

	if (m_pPreviewModel != NULL &&
		m_iPreviewModelType == iWantedModelType &&
		m_iPreviewTexWidth == iWantedTexWidth &&
		m_iPreviewTexHeight == iWantedTexHeight)
	{
		return;
	}

	DestroyPreviewModel();

	bool bSlim = (iWantedModelType == HumanoidModel::ePlayerModel_Alex);
	m_pPreviewModel = new HumanoidModel(0.0f, 0.0f, iWantedTexWidth, iWantedTexHeight, bSlim);
	m_iPreviewModelType = iWantedModelType;
	m_iPreviewTexWidth = iWantedTexWidth;
	m_iPreviewTexHeight = iWantedTexHeight;
}

void UIControl_PlayerSkinPreview::tick()
{
	UIControl::tick();

	if( m_bAnimatingToFacing )
	{
		++m_framesAnimatingRotation;
		m_yRot = m_fOriginalRotation + m_framesAnimatingRotation * ( (m_fTargetRotation - m_fOriginalRotation) / CHANGING_SKIN_FRAMES );
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
	RebuildPreviewModel();
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
	m_currentAnimation = (ESkinPreviewAnimations)(m_currentAnimation + 1);
	if(m_currentAnimation >= e_SkinPreviewAnimation_Count) m_currentAnimation = e_SkinPreviewAnimation_Walking;

	m_swingTime = 0.0f;
}

void UIControl_PlayerSkinPreview::CyclePreviousAnimation()
{
	m_currentAnimation = (ESkinPreviewAnimations)(m_currentAnimation - 1);
	if(m_currentAnimation < e_SkinPreviewAnimation_Walking) m_currentAnimation = (ESkinPreviewAnimations)(e_SkinPreviewAnimation_Count - 1);

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

	float ss;
	ss = width / (m_fScreenWidth / m_fScreenHeight);

	glScalef(-ss, ss, ss);
	glRotatef(180, 0, 0, 1);

	Lighting::turnOn();
	glRotatef(-(float)m_xRot, 1, 0, 0);

	bool wasHidingGui = pMinecraft->options->hideGui;
	pMinecraft->options->hideGui = true;

	EntityRenderer *renderer = EntityRenderDispatcher::instance->getRenderer(eTYPE_LOCALPLAYER);
	if (renderer != NULL)
	{
		if(m_pvAdditionalModelParts && m_pvAdditionalModelParts->size()!=0)
		{
			for(auto& pModelPart : *m_pvAdditionalModelParts)
			{
				pModelPart->visible=true;
			}
		}

		render(renderer,0,0,0,0,1);

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

void UIControl_PlayerSkinPreview::render(EntityRenderer *renderer, double x, double y, double z, float rot, float a)
{
	glPushMatrix();
	glDisable(GL_CULL_FACE);

	HumanoidModel *model = m_pPreviewModel;
	if (model == NULL)
	{
		RebuildPreviewModel();
		model = m_pPreviewModel;
	}

	if (model == NULL)
	{
		glEnable(GL_CULL_FACE);
		glPopMatrix();
		return;
	}

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
			model->attackTime = m_swingTime / (float) (Player::SWING_DURATION * 3);
			break;
		default:
			break;
		};
	}

	float bodyRot = m_yRot;
	float headRot = m_yRot;
	float headRotx = 0;

	glTranslatef((float) x, (float) y, (float) z);

#ifdef SKIN_PREVIEW_BOB_ANIM
	float bob = (m_bobTick + a)/2;
	++m_bobTick;
	if(m_bobTick>=360*2) m_bobTick = 0;
#else
	float bob = 0.0f;
#endif

	glRotatef(180 - bodyRot, 0, 1, 0);

	float _scale = 1 / 16.0f;
	glEnable(GL_RESCALE_NORMAL);
	glScalef(-1, -1, 1);

	float s = 15 / 16.0f;
	glScalef(s, s, s);

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

	model->render(nullptr, wp, ws, bob, headRot - bodyRot, headRotx, _scale, true);

	if (bindTexture(m_capeTextureUrl, L"" ))
	{
		glPushMatrix();
		glTranslatef(0, 0, 2 / 16.0f);

		double xd = 0;
		double yd = 0;
		double zd = 0;

		float yr = 1;

		double xa = sin(yr * PI / 180);
		double za = -cos(yr * PI / 180);

		float flap = (float) yd * 10;
		if (flap < -6) flap = -6;
		if (flap > 32) flap = 32;
		float lean = (float) (xd * xa + zd * za) * 100;
		float lean2 = (float) (xd * za - zd * xa) * 100;
		if (lean < 0) lean = 0;

		flap += 1;
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

	glDisable(GL_RESCALE_NORMAL);
	glEnable(GL_CULL_FACE);
	glPopMatrix();

	MemSect(31);
	MemSect(0);

	model->attackTime = 0;
	model->sneaking = false;
	model->holdingRightHand = false;
	model->holdingLeftHand = false;
}

bool UIControl_PlayerSkinPreview::bindTexture(const wstring& urlTexture, int backupTexture)
{
	Textures *t = Minecraft::GetInstance()->textures;
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
