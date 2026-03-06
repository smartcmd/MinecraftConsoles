#include "stdafx.h"
#include "Minecraft.h"
#include "GameMode.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.storage.h"
#include "Input.h"
#include "..\Minecraft.Client\LocalPlayer.h"
#include "Options.h"
#ifdef _WINDOWS64
#include "Windows64\KeyboardMouseInput.h"
#endif

Input::Input()
{
	xa = 0;
	ya = 0;
	wasJumping = false;
	jumping = false;
	sneaking = false;
	sprinting = false;

	lReset = false;
    rReset = false;
}

void Input::tick(LocalPlayer *player)
{
	// 4J Stu -  Assume that we only need one input class, even though the java has subclasses for keyboard/controller
	// This function is based on the ControllerInput class in the Java, and will probably need changed
	//OutputDebugString("INPUT: Beginning input tick\n");

	Minecraft *pMinecraft=Minecraft::GetInstance();
	int iPad=player->GetXboxPad();

	float controllerXA = 0.0f;
	float controllerYA = 0.0f;

	// 4J-PB minecraft movement seems to be the wrong way round, so invert x!
	if( pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LEFT) || pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_RIGHT) )
		controllerXA = -InputManager.GetJoypadStick_LX(iPad);

	if( pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_FORWARD) || pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_BACKWARD) )
		controllerYA = InputManager.GetJoypadStick_LY(iPad);

	float kbXA = 0.0f;
	float kbYA = 0.0f;
#ifdef _WINDOWS64
	if (iPad == 0 && g_KBMInput.IsMouseGrabbed() && g_KBMInput.IsKBMActive())
	{
		if( pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LEFT) || pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_RIGHT) )
			kbXA = g_KBMInput.GetMoveX();
		if( pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_FORWARD) || pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_BACKWARD) )
			kbYA = g_KBMInput.GetMoveY();
	}
#endif

	if (kbXA != 0.0f || kbYA != 0.0f)
	{
		xa = kbXA;
		ya = kbYA;
	}
	else
	{
		xa = controllerXA;
		ya = controllerYA;
	}

#ifndef _CONTENT_PACKAGE
	if (app.GetFreezePlayers())
	{
		xa = ya = 0.0f;
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
    }

	// 4J: In flying mode, don't actually toggle sneaking (unless we're riding in which case we need to sneak to dismount)
	if(!player->abilities.flying || player->riding != NULL)
	{
		if((player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_SNEAK_TOGGLE)) && pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_SNEAK_TOGGLE))
		{
			sneaking=!sneaking;
		}
	}

#ifdef _WINDOWS64
	if (iPad == 0 && g_KBMInput.IsMouseGrabbed() && g_KBMInput.IsKBMActive())
	{
		// Left Shift = sneak (hold to crouch)
		if (pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_SNEAK_TOGGLE))
		{
			if (!player->abilities.flying)
			{
				sneaking = g_KBMInput.IsKeyDown(KeyboardMouseInput::KEY_SNEAK);
			}
		}

		// Ctrl + forward = sprint (hold to sprint, including while flying)
		{
			bool ctrlHeld = g_KBMInput.IsKeyDown(KeyboardMouseInput::KEY_SPRINT);
			bool movingForward = (kbYA > 0.0f);

			if (ctrlHeld && movingForward)
			{
				sprinting = true;
			}
			else
			{
				sprinting = false;
			}
		}
	}
	else if (iPad == 0)
	{
		sprinting = false;
	}
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

	float turnX = tx * abs(tx) * turnSpeed;
	float turnY = ty * abs(ty) * turnSpeed;

#ifdef _WINDOWS64
	if (iPad == 0 && g_KBMInput.IsMouseGrabbed() && g_KBMInput.IsKBMActive())
	{
		float mouseSensitivity = ((float)app.GetGameSettings(iPad,eGameSetting_Sensitivity_InGame)) / 100.0f;
		float mouseLookScale = 5.0f;
		float mx = g_KBMInput.GetLookX(mouseSensitivity * mouseLookScale);
		float my = g_KBMInput.GetLookY(mouseSensitivity * mouseLookScale);

		if ( app.GetGameSettings(iPad,eGameSetting_ControlInvertLook) )
		{
			my = -my;
		}

		turnX += mx;
		turnY += my;
	}
#endif

	player->interpolateTurn(turnX, turnY);

    //jumping = controller.isButtonPressed(0);

	unsigned int jump = InputManager.GetValue(iPad, MINECRAFT_ACTION_JUMP);
	bool kbJump = false;
#ifdef _WINDOWS64
	kbJump = (iPad == 0) && g_KBMInput.IsMouseGrabbed() && g_KBMInput.IsKBMActive() && g_KBMInput.IsKeyDown(KeyboardMouseInput::KEY_JUMP);
#endif
	if( (jump > 0 || kbJump) && pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_JUMP) )
		jumping = true;
	else
 		jumping = false;

#ifndef _CONTENT_PACKAGE
	if (app.GetFreezePlayers())	jumping = false;
#endif

#ifdef _WINDOWS64
	if (iPad == 0 && g_KBMInput.IsKeyPressed(VK_ESCAPE) && g_KBMInput.IsMouseGrabbed())
	{
		g_KBMInput.SetMouseGrabbed(false);
	}
#endif

	//OutputDebugString("INPUT: End input tick\n");
}
