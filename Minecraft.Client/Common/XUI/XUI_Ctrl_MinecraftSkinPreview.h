#pragma once

#include <string>
#include <XuiApp.h>
#include "../../Textures.h"
//#include "..\..\Xbox\DLC\DLCSkinFile.h"
#include "../../Model.h"

using namespace std;

class EntityRenderer;

//-----------------------------------------------------------------------------
//  CXuiCtrlMinecraftSkinPreview class
//-----------------------------------------------------------------------------
class CXuiCtrlMinecraftSkinPreview : public CXuiControlImpl
{
private:
	static const int LOOK_LEFT_EXTENT = 45;
	static const int LOOK_RIGHT_EXTENT = -45;

	static const int CHANGING_SKIN_FRAMES = 15;

	enum ESkinPreviewAnimations
	{
		e_SkinPreviewAnimation_Walking,
		e_SkinPreviewAnimation_Sneaking,
		e_SkinPreviewAnimation_Attacking,

		e_SkinPreviewAnimation_Count,
	};
public:
	enum ESkinPreviewFacing
	{
		e_SkinPreviewFacing_Forward,
		e_SkinPreviewFacing_Left,
		e_SkinPreviewFacing_Right,
	};
public:
	XUI_IMPLEMENT_CLASS(CXuiCtrlMinecraftSkinPreview, L"CXuiCtrlMinecraftSkinPreview", XUI_CLASS_LABEL)

	CXuiCtrlMinecraftSkinPreview();
	virtual ~CXuiCtrlMinecraftSkinPreview() { };

	void SetTexture(const wstring &url, TEXTURE_NAME backupTexture = TN_MOB_CHAR);
	void SetCapeTexture(const wstring &url) { m_capeTextureUrl = url; }
	void ResetRotation() { m_xRot = 0; m_yRot = 0; }
	void IncrementYRotation() { m_yRot = (m_yRot+4); if(m_yRot >= 180) m_yRot = -180; }
	void DecrementYRotation() { m_yRot = (m_yRot-4); if(m_yRot <= -180) m_yRot = 180; }
	void IncrementXRotation() { m_xRot = (m_xRot+2); if(m_xRot > 22) m_xRot = 22; }
	void DecrementXRotation() { m_xRot = (m_xRot-2); if(m_xRot < -22) m_xRot = -22; }
	void SetAutoRotate(bool autoRotate) { m_bAutoRotate = autoRotate; }
	void SetFacing(ESkinPreviewFacing facing, bool bAnimate = false);

	void CycleNextAnimation();
	void CyclePreviousAnimation();

	bool m_incXRot, m_decXRot;
	bool m_incYRot, m_decYRot;

protected:

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_RENDER(OnRender)
	XUI_END_MSG_MAP()

	HRESULT OnInit(XUIMessageInit* pInitData, BOOL& rfHandled);
	HRESULT OnRender(XUIMessageRender *pRenderData, BOOL &rfHandled);

private:
	void render(EntityRenderer *renderer, double x, double y, double z, float rot, float a);
	bool bindTexture(const wstring& urlTexture, int backupTexture);
	bool bindTexture(const wstring& urlTexture, const wstring& backupTexture);

	BOOL m_bDirty;
	float m_fScale,m_fAlpha;

	wstring m_customTextureUrl;
	TEXTURE_NAME m_backupTexture;
	wstring m_capeTextureUrl;
	unsigned int m_uiAnimOverrideBitmask;

	float m_fScreenWidth,m_fScreenHeight;
	float m_fRawWidth,m_fRawHeight;

	int m_yRot,m_xRot;
	
	float m_bobTick;

	float m_walkAnimSpeedO;
	float m_walkAnimSpeed;
	float m_walkAnimPos;

	bool m_bAutoRotate, m_bRotatingLeft;
	BYTE m_rotateTick;
	float m_fTargetRotation, m_fOriginalRotation;
	int m_framesAnimatingRotation;
	bool m_bAnimatingToFacing;

	float m_swingTime;

	ESkinPreviewAnimations m_currentAnimation;
	//vector<Model::SKIN_BOX *> *m_pvAdditionalBoxes;
	vector<ModelPart *> *m_pvAdditionalModelParts;
};