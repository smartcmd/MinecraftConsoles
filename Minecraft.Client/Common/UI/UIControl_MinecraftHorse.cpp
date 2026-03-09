#include "stdafx.h"
#include "..\..\Minecraft.h"
#include "..\..\ScreenSizeCalculator.h"
#include "..\..\EntityRenderDispatcher.h"

#include "..\..\PlayerRenderer.h"
#include "..\..\HorseRenderer.h"

#include "..\..\HumanoidModel.h"
#include "..\..\ModelHorse.h"

#include "..\..\Lighting.h"
#include "..\..\ModelPart.h"
#include "..\..\Options.h"

#include "..\..\..\Minecraft.World\net.minecraft.world.entity.player.h"
//#include "..\..\..\Minecraft.World\net.minecraft.world.entity.animal.EntityHorse.h"

#include "..\..\MultiplayerLocalPlayer.h"
#include "UI.h"
#include "UIControl_MinecraftHorse.h"

UIControl_MinecraftHorse::UIControl_MinecraftHorse()
{
	UIControl::setControlType(UIControl::eMinecraftHorse);

	Minecraft *pMinecraft=Minecraft::GetInstance();

	ScreenSizeCalculator ssc(pMinecraft->options, pMinecraft->width_phys, pMinecraft->height_phys);
	m_fScreenWidth=static_cast<float>(pMinecraft->width_phys);
	m_fRawWidth=static_cast<float>(ssc.rawWidth);
	m_fScreenHeight=static_cast<float>(pMinecraft->height_phys);
	m_fRawHeight=static_cast<float>(ssc.rawHeight);
}

void UIControl_MinecraftHorse::render(IggyCustomDrawCallbackRegion *region)
{	
	Minecraft *pMinecraft = Minecraft::GetInstance();
	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_COLOR_MATERIAL);
	glPushMatrix();

	float width = region->x1 - region->x0;
	float height = region->y1 - region->y0;
	float xo = width/2;
	float yo = height;
	
	// dynamic y offset according to region height
	glTranslatef(xo, yo - (height / 7.5f), 50.0f);

	//UIScene_InventoryMenu *containerMenu = (UIScene_InventoryMenu *)m_parentScene;
	UIScene_HorseInventoryMenu *containerMenu = static_cast<UIScene_HorseInventoryMenu *>(m_parentScene);

	shared_ptr<LivingEntity> entityHorse = containerMenu->m_horse;

	// Base scale on height of this control
	// Potentially we might want separate x & y scales here
	float ss = width / (m_fScreenWidth / m_fScreenHeight) * 0.71f;

	glScalef(-ss, ss, ss);
	glRotatef(180, 0, 0, 1);

	float oybr = entityHorse->yBodyRot;
	float oyr = entityHorse->yRot;
	float oxr = entityHorse->xRot;
	float oyhr = entityHorse->yHeadRot;

	//float xd = ( matrix._41 + ( (bwidth*matrix._11)/2) ) - m_pointerPos.x;
	float xd = (m_x + m_width/2) - containerMenu->m_pointerPos.x;

	// Need to base Y on head position, not centre of mass
	//float yd = ( matrix._42 + ( (bheight*matrix._22) / 2) - 40 ) - m_pointerPos.y;
	float yd = (m_y + m_height/2 - 40) - containerMenu->m_pointerPos.y;

	glRotatef(45 + 90, 0, 1, 0);
	Lighting::turnOn();
	glRotatef(-45 - 90, 0, 1, 0);

	glRotatef(-(float) atan(yd / 40.0f) * 20, 1, 0, 0);

	entityHorse->yBodyRot = (float) atan(xd / 40.0f) * 20;
	entityHorse->yRot = (float) atan(xd / 40.0f) * 40;
	entityHorse->xRot = -(float) atan(yd / 40.0f) * 20;
	entityHorse->yHeadRot = entityHorse->yRot;
	//entityHorse->glow = 1;
	glTranslatef(0, entityHorse->heightOffset, 0);
	EntityRenderDispatcher::instance->playerRotY = 180;

	// 4J Stu - Turning on hideGui while we do this stops the name rendering in split-screen
	bool wasHidingGui = pMinecraft->options->hideGui;
	pMinecraft->options->hideGui = true;
	EntityRenderDispatcher::instance->render(entityHorse, 0, 0, 0, 0, 1, false, false);
	pMinecraft->options->hideGui = wasHidingGui;
	//entityHorse->glow = 0;

	entityHorse->yBodyRot = oybr;
	entityHorse->yRot = oyr;
	entityHorse->xRot = oxr;
	entityHorse->yHeadRot = oyhr;
	glPopMatrix();
	Lighting::turnOff();
	glDisable(GL_RESCALE_NORMAL);
}
