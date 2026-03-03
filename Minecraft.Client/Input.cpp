#include "stdafx.h"
#include "Minecraft.h"
#include "GameMode.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.storage.h"
#include "Input.h"
#include "..\Minecraft.Client\LocalPlayer.h"
#include "Options.h"

Input::Input()
{
	xa = 0;
	ya = 0;
	sprintForward = 0;
	wasJumping = false;
	jumping = false;
	sneaking = false;
	usingKeyboardMovement = false;

	lReset = false;
    rReset = false;
	m_gamepadSneaking = false;
}

void Input::tick(LocalPlayer *player)
{
	// 4J Stu -  Assume that we only need one input class, even though the java has subclasses for keyboard/controller
	// This function is based on the ControllerInput class in the Java, and will probably need changed
	//OutputDebugString("INPUT: Beginning input tick\n");

	Minecraft *pMinecraft=Minecraft::GetInstance();
	int iPad=player->GetXboxPad();

	// 4J-PB minecraft movement seems to be the wrong way round, so invert x!
	if( pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LEFT) || pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_RIGHT) )
		xa = -InputManager.GetJoypadStick_LX(iPad);
	else
		xa = 0.0f;

	if( pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_FORWARD) || pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_BACKWARD) )
		ya = InputManager.GetJoypadStick_LY(iPad);
	else
		ya = 0.0f;
	sprintForward = ya;
	usingKeyboardMovement = false;

#ifdef _WINDOWS64
	// WASD movement (combine with gamepad)
	if (iPad == 0 && KMInput.IsCaptured())
	{
		float kbX = 0.0f, kbY = 0.0f;
		if (KMInput.IsKeyDown('W')) { kbY += 1.0f; sprintForward += 1.0f; usingKeyboardMovement = true; }
		if (KMInput.IsKeyDown('S')) { kbY -= 1.0f; sprintForward -= 1.0f; usingKeyboardMovement = true; }
		if (KMInput.IsKeyDown('A')) { kbX += 1.0f; usingKeyboardMovement = true; }  // inverted like gamepad
		if (KMInput.IsKeyDown('D')) { kbX -= 1.0f; usingKeyboardMovement = true; }
		// Normalize diagonal
		if (kbX != 0.0f && kbY != 0.0f) { kbX *= 0.707f; kbY *= 0.707f; }
		if (pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LEFT) || pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_RIGHT))
			xa = max(min(xa + kbX, 1.0f), -1.0f);
		if (pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_FORWARD) || pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_BACKWARD))
			ya = max(min(ya + kbY, 1.0f), -1.0f);
	}
#endif
	sprintForward = max(min(sprintForward, 1.0f), -1.0f);

#ifndef _CONTENT_PACKAGE
	if (app.GetFreezePlayers())
	{
		xa = ya = 0.0f;
		sprintForward = 0.0f;
		player->abilities.flying = true;
	}
#endif

    if (!lReset)
    {
        if (xa*xa+ya*ya==0.0f)
        {
            lReset = true;
        }
        xa = ya = 0.0f;
		sprintForward = 0.0f;
    }

	// 4J: In flying mode, don't actually toggle sneaking (unless we're riding in which case we need to sneak to dismount)
	if(!player->abilities.flying || player->riding != NULL)
	{
		if((player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_SNEAK_TOGGLE)) && pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_SNEAK_TOGGLE))
		{
			m_gamepadSneaking=!m_gamepadSneaking;
		}
	}
	sneaking = m_gamepadSneaking;

#ifdef _WINDOWS64
	// Keyboard hold-to-sneak (overrides gamepad toggle)
	if (iPad == 0 && KMInput.IsCaptured() && KMInput.IsKeyDown(VK_SHIFT) && !player->abilities.flying)
		sneaking = true;
#endif

	if(sneaking)
	{
		xa*=0.3f;
		ya*=0.3f;
	}

    float turnSpeed = 50.0f;

	float tx = 0.0f;
	float ty = 0.0f;
	if( pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LOOK_LEFT) || pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LOOK_RIGHT) )
		tx = InputManager.GetJoypadStick_RX(iPad)*(((float)app.GetGameSettings(iPad,eGameSetting_Sensitivity_InGame))/100.0f); // apply sensitivity to look
	if( pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LOOK_UP) || pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LOOK_DOWN) )
		ty = InputManager.GetJoypadStick_RY(iPad)*(((float)app.GetGameSettings(iPad,eGameSetting_Sensitivity_InGame))/100.0f); // apply sensitivity to look

#ifndef _CONTENT_PACKAGE
	if (app.GetFreezePlayers())	tx = ty = 0.0f;
#endif

	// 4J: WESTY : Invert look Y if required.
	if ( app.GetGameSettings(iPad,eGameSetting_ControlInvertLook) )
	{
		ty = -ty;
	}

    if (!rReset)
    {
        if (tx*tx+ty*ty==0.0f)
        {
            rReset = true;
        }
        tx = ty = 0.0f;
    }
	player->interpolateTurn(tx * abs(tx) * turnSpeed, ty * abs(ty) * turnSpeed);

#ifdef _WINDOWS64
	// Mouse look is now handled per-frame in Minecraft::applyFrameMouseLook()
	// to eliminate the 20Hz tick delay. Only flush any remaining delta here
	// as a safety measure.
	if (iPad == 0 && KMInput.IsCaptured())
	{
		float rawDx, rawDy;
		KMInput.ConsumeMouseDelta(rawDx, rawDy);
		// Delta should normally be 0 since applyFrameMouseLook() already consumed it
		if (rawDx != 0.0f || rawDy != 0.0f)
		{
			float mouseSensitivity = ((float)app.GetGameSettings(iPad, eGameSetting_Sensitivity_InGame)) / 100.0f;
			float mdx = rawDx * mouseSensitivity;
			float mdy = -rawDy * mouseSensitivity;
			if (app.GetGameSettings(iPad, eGameSetting_ControlInvertLook))
				mdy = -mdy;
			player->interpolateTurn(mdx, mdy);
		}
	}
#endif

    //jumping = controller.isButtonPressed(0);


	unsigned int jump = InputManager.GetValue(iPad, MINECRAFT_ACTION_JUMP);
	if( jump > 0 && pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_JUMP) )
		jumping = true;
	else
 		jumping = false;

#ifdef _WINDOWS64
	// Keyboard jump (Space)
	if (iPad == 0 && KMInput.IsCaptured() && KMInput.IsKeyDown(VK_SPACE) && pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_JUMP))
		jumping = true;
#endif

#ifndef _CONTENT_PACKAGE
	if (app.GetFreezePlayers())	jumping = false;
#endif

	//OutputDebugString("INPUT: End input tick\n");
}
