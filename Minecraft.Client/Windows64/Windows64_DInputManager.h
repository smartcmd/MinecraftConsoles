#pragma once

#ifdef _WINDOWS64

// DInput / WinMM controller support for Windows.
//
// The 4J_Input.lib uses XInput internally, which only sees XInput-compatible
// controllers (Xbox controllers, XInput-mode adapters, etc.).  Non-XInput
// devices – DualShock 4, generic HID gamepads, most non-Xbox controllers –
// are invisible to XInput and therefore to the game.
//
// This module fixes that by:
//   1. Hooking XInputGetState / XInputGetCapabilities via IAT patching so
//      that every call in the process (including from 4J_Input.lib) passes
//      through our wrapper.
//   2. Using the WinMM joystick API (joyGetPosEx) to enumerate non-XInput
//      devices and feed their state back through the XInput interface as if
//      they were standard XInput gamepads.
//
// The DS4 / DualShock 4 button layout used here matches the standard Windows
// HID driver (no DS4Windows required):
//
//   WinMM button index -> Xbox equivalent
//   0  Square          -> X
//   1  Cross           -> A
//   2  Circle          -> B
//   3  Triangle        -> Y
//   4  L1              -> Left Shoulder
//   5  R1              -> Right Shoulder
//   6  L2 (digital)   -> (analog axis used instead)
//   7  R2 (digital)   -> (analog axis used instead)
//   8  Share           -> Back
//   9  Options         -> Start
//   10 L3              -> Left Thumb
//   11 R3              -> Right Thumb
//
//   Axis mapping (joyGetPosEx) - DS4 standard Windows HID driver:
//   DS4 HID axis order: X, Y, Z(RSX), Rz(RSY), Ry(R2), Rx(L2)
//   WinMM maps HID axes sequentially to dwXpos/Ypos/Zpos/Rpos/Upos/Vpos:
//   dwXpos  -> Left stick X
//   dwYpos  -> Left stick Y  (WinMM: 0=up; inverted for XInput)
//   dwZpos  -> Right stick X (HID Z)
//   dwRpos  -> Right stick Y (HID Rz, inverted for XInput)
//   dwUpos  -> R2 trigger    (HID Ry)
//   dwVpos  -> L2 trigger    (HID Rx)
//   dwPOV   -> D-pad (centidegrees, JOY_POVCENTERED = not pressed)

// Call once after InputManager.Initialise() and before the first Tick().
// Patches the IAT so both game code and 4J_Input.lib see the DInput fallback.
void DInputManager_Init();

// Call once per frame (alongside InputManager.Tick()) to refresh the list of
// connected DInput controllers.
void DInputManager_Tick();

#endif // _WINDOWS64
