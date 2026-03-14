#pragma once

#include "../Common/UI/UIController.h"

class ConsoleUIController : public UIController
{
private:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;
public:
	void init(Microsoft::WRL::ComPtr<ID3D11Device> dev, Microsoft::WRL::ComPtr<ID3D11DeviceContext> ctx, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTargetView, Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView, S32 w, S32 h);

	void render();
	void beginIggyCustomDraw4J(IggyCustomDrawCallbackRegion *region, CustomDrawData *customDrawRegion);
	virtual CustomDrawData *setupCustomDraw(UIScene *scene, IggyCustomDrawCallbackRegion *region);
	virtual CustomDrawData *calculateCustomDraw(IggyCustomDrawCallbackRegion *region);
	virtual void endCustomDraw(IggyCustomDrawCallbackRegion *region);

protected:
	virtual void setTileOrigin(S32 xPos, S32 yPos);

public:
	GDrawTexture *getSubstitutionTexture(int textureId);
	void destroySubstitutionTexture(void *destroyCallBackData, GDrawTexture *handle);

public:
	void shutdown();
};

extern ConsoleUIController ui;