#include "stdafx.h"

#ifdef _WINDOWS64

#include "Windows64_DInputManager.h"
#include <xinput.h>
#include <mmsystem.h>
#include <imagehlp.h>   // for ImageDirectoryEntryToData / manual PE walk
#pragma comment(lib, "winmm.lib")

// ---------------------------------------------------------------------------
// Internal types & state
// ---------------------------------------------------------------------------

typedef DWORD (WINAPI* PFN_XInputGetState      )(DWORD, XINPUT_STATE*);
typedef DWORD (WINAPI* PFN_XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES*);

static PFN_XInputGetState       s_origGetState = nullptr;
static PFN_XInputGetCapabilities s_origGetCaps  = nullptr;

// WinMM joystick ID assigned to each XInput slot as a DInput fallback.
// -1 means "no DInput fallback for this slot".
static int s_dInputForSlot[XUSER_MAX_COUNT] = { -1, -1, -1, -1 };

// Simple frame-rate throttle: only re-scan for new DInput devices every N ticks.
static int s_rescanCountdown = 0;
static const int RESCAN_INTERVAL = 90; // ~3 s at 30 fps

// ---------------------------------------------------------------------------
// WinMM -> XInput axis conversion helpers
// ---------------------------------------------------------------------------

// WinMM range: 0-65535, centre ~32767
// XInput range: -32768..32767
static SHORT WinMMAxisToXInput(DWORD winmmAxis)
{
    int v = (int)winmmAxis - 32768;
    if (v < -32768) v = -32768;
    if (v >  32767) v =  32767;
    return (SHORT)v;
}

// WinMM trigger range: 0-65535 (0 = not pressed)
// XInput trigger range: 0-255
static BYTE WinMMTriggerToXInput(DWORD winmmAxis)
{
    return (BYTE)(winmmAxis >> 8);
}

// ---------------------------------------------------------------------------
// Build an XINPUT_STATE from a WinMM joyGetPosEx call.
// Returns true if the joystick responded successfully.
// ---------------------------------------------------------------------------
static bool WinMMToXInputState(UINT wmId, XINPUT_STATE* out)
{
    JOYINFOEX ji = {};
    ji.dwSize  = sizeof(ji);
    ji.dwFlags = JOY_RETURNALL;

    if (joyGetPosEx(wmId, &ji) != JOYERR_NOERROR)
        return false;

    ZeroMemory(out, sizeof(*out));
    out->dwPacketNumber = GetTickCount();

    WORD buttons = 0;

    // ---- Face buttons (DS4 standard Windows HID driver mapping) ----
    if (ji.dwButtons & (1u <<  0)) buttons |= XINPUT_GAMEPAD_X;              // Square   -> X
    if (ji.dwButtons & (1u <<  1)) buttons |= XINPUT_GAMEPAD_A;              // Cross    -> A
    if (ji.dwButtons & (1u <<  2)) buttons |= XINPUT_GAMEPAD_B;              // Circle   -> B
    if (ji.dwButtons & (1u <<  3)) buttons |= XINPUT_GAMEPAD_Y;              // Triangle -> Y
    if (ji.dwButtons & (1u <<  4)) buttons |= XINPUT_GAMEPAD_LEFT_SHOULDER;  // L1 -> LB
    if (ji.dwButtons & (1u <<  5)) buttons |= XINPUT_GAMEPAD_RIGHT_SHOULDER; // R1 -> RB
    // buttons 6 & 7 are digital L2/R2 – we use the analog axes instead
    if (ji.dwButtons & (1u <<  8)) buttons |= XINPUT_GAMEPAD_BACK;           // Share   -> Back
    if (ji.dwButtons & (1u <<  9)) buttons |= XINPUT_GAMEPAD_START;          // Options -> Start
    if (ji.dwButtons & (1u << 10)) buttons |= XINPUT_GAMEPAD_LEFT_THUMB;     // L3
    if (ji.dwButtons & (1u << 11)) buttons |= XINPUT_GAMEPAD_RIGHT_THUMB;    // R3

    // ---- D-pad via POV hat ----
    // dwPOV is in 100ths of a degree. JOY_POVCENTERED (0xFFFF) = not pressed.
    if (ji.dwPOV != JOY_POVCENTERED)
    {
        DWORD pov = ji.dwPOV;
        if (pov <= 4500 || pov >= 31500) buttons |= XINPUT_GAMEPAD_DPAD_UP;
        if (pov >=  4500 && pov <= 13500) buttons |= XINPUT_GAMEPAD_DPAD_RIGHT;
        if (pov >= 13500 && pov <= 22500) buttons |= XINPUT_GAMEPAD_DPAD_DOWN;
        if (pov >= 22500 && pov <= 31500) buttons |= XINPUT_GAMEPAD_DPAD_LEFT;
    }

    out->Gamepad.wButtons = buttons;

    // ---- Sticks ----
    // Left stick: X / Y  (WinMM Y: 0=top, XInput Y: positive=up -> invert)
    out->Gamepad.sThumbLX = WinMMAxisToXInput(ji.dwXpos);
    out->Gamepad.sThumbLY = (SHORT)(-WinMMAxisToXInput(ji.dwYpos) - 1);

    // Right stick: Z (RSX) / Rz (RSY)
    // DS4 HID axis order: X, Y, Z(RSX), Rz(RSY), Ry(R2), Rx(L2)
    // WinMM maps these sequentially: dwZpos=RSX, dwRpos=RSY, dwUpos=R2, dwVpos=L2
    out->Gamepad.sThumbRX = WinMMAxisToXInput(ji.dwZpos);
    out->Gamepad.sThumbRY = (SHORT)(-WinMMAxisToXInput(ji.dwRpos) - 1);

    // ---- Triggers ----
    // Ry (dwUpos) = R2, Rx (dwVpos) = L2
    out->Gamepad.bLeftTrigger  = WinMMTriggerToXInput(ji.dwVpos);
    out->Gamepad.bRightTrigger = WinMMTriggerToXInput(ji.dwUpos);

    return true;
}

// ---------------------------------------------------------------------------
// Slot assignment: build / refresh the DInput -> XInput slot map.
//
// Heuristic: Windows places XInput controllers in the lowest WinMM joystick
// slots.  Joystick slots at index >= (number of connected XInput controllers)
// are treated as DInput devices.
// ---------------------------------------------------------------------------
static void RefreshDInputSlots()
{
    if (!s_origGetState)
        return;

    // Count connected XInput slots and record which slots are empty.
    bool xinputConnected[XUSER_MAX_COUNT] = {};
    int  numXInputConnected = 0;
    for (DWORD xi = 0; xi < XUSER_MAX_COUNT; xi++)
    {
        XINPUT_STATE st;
        if (s_origGetState(xi, &st) == ERROR_SUCCESS)
        {
            xinputConnected[xi] = true;
            numXInputConnected++;
        }
    }

    // Release any DInput slot whose WinMM device is no longer responding,
    // or whose XInput slot is now occupied by a real XInput controller.
    for (int xi = 0; xi < XUSER_MAX_COUNT; xi++)
    {
        if (s_dInputForSlot[xi] < 0)
            continue;

        UINT wmId = (UINT)s_dInputForSlot[xi];
        JOYINFOEX ji = {};
        ji.dwSize  = sizeof(ji);
        ji.dwFlags = JOY_RETURNBUTTONS;

        bool devGone     = (joyGetPosEx(wmId, &ji) != JOYERR_NOERROR);
        bool slotTaken   = xinputConnected[xi];
        if (devGone || slotTaken)
            s_dInputForSlot[xi] = -1;
    }

    // Build a set of WinMM IDs already in use as DInput slots.
    bool wmIdInUse[16] = {};
    for (int xi = 0; xi < XUSER_MAX_COUNT; xi++)
        if (s_dInputForSlot[xi] >= 0 && s_dInputForSlot[xi] < 16)
            wmIdInUse[s_dInputForSlot[xi]] = true;

    // Enumerate WinMM joysticks.  Those at index >= numXInputConnected are
    // treated as non-XInput (DInput) devices.
    UINT numWinMM = joyGetNumDevs();
    for (UINT wmId = (UINT)numXInputConnected; wmId < numWinMM; wmId++)
    {
        if (wmId >= 16) break;
        if (wmIdInUse[wmId]) continue;

        // Test whether this device is actually connected.
        JOYINFOEX ji = {};
        ji.dwSize  = sizeof(ji);
        ji.dwFlags = JOY_RETURNBUTTONS;
        if (joyGetPosEx(wmId, &ji) != JOYERR_NOERROR)
            continue;

        // Find the first XInput slot without a real XInput controller or a
        // DInput assignment, and assign this device to it.
        for (int xi = 0; xi < XUSER_MAX_COUNT; xi++)
        {
            if (!xinputConnected[xi] && s_dInputForSlot[xi] < 0)
            {
                s_dInputForSlot[xi] = (int)wmId;
                wmIdInUse[wmId]     = true;
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Hooked XInputGetState
// ---------------------------------------------------------------------------
static DWORD WINAPI Hooked_XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
    // Always try the real XInput first.
    DWORD result = s_origGetState
        ? s_origGetState(dwUserIndex, pState)
        : ERROR_DEVICE_NOT_CONNECTED;

    if (result == ERROR_DEVICE_NOT_CONNECTED
        && dwUserIndex < XUSER_MAX_COUNT
        && s_dInputForSlot[dwUserIndex] >= 0)
    {
        if (WinMMToXInputState((UINT)s_dInputForSlot[dwUserIndex], pState))
            return ERROR_SUCCESS;

        // Device disappeared – clear the slot.
        s_dInputForSlot[dwUserIndex] = -1;
    }

    return result;
}

// ---------------------------------------------------------------------------
// Hooked XInputGetCapabilities
// ---------------------------------------------------------------------------
static DWORD WINAPI Hooked_XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags,
                                                  XINPUT_CAPABILITIES* pCapabilities)
{
    DWORD result = s_origGetCaps
        ? s_origGetCaps(dwUserIndex, dwFlags, pCapabilities)
        : ERROR_DEVICE_NOT_CONNECTED;

    if (result == ERROR_DEVICE_NOT_CONNECTED
        && dwUserIndex < XUSER_MAX_COUNT
        && s_dInputForSlot[dwUserIndex] >= 0)
    {
        ZeroMemory(pCapabilities, sizeof(*pCapabilities));
        pCapabilities->Type    = XINPUT_DEVTYPE_GAMEPAD;
        pCapabilities->SubType = XINPUT_DEVSUBTYPE_GAMEPAD;
        pCapabilities->Gamepad.wButtons      = 0xF3FF; // all mapped buttons
        pCapabilities->Gamepad.bLeftTrigger  = 0xFF;
        pCapabilities->Gamepad.bRightTrigger = 0xFF;
        pCapabilities->Gamepad.sThumbLX      = 32767;
        pCapabilities->Gamepad.sThumbLY      = 32767;
        pCapabilities->Gamepad.sThumbRX      = 32767;
        pCapabilities->Gamepad.sThumbRY      = 32767;
        return ERROR_SUCCESS;
    }

    return result;
}

// ---------------------------------------------------------------------------
// IAT patching
//
// Walks the PE import table of the current process and replaces the entries
// for XInputGetState and XInputGetCapabilities with our hooks.  This affects
// all callers in the process, including the statically-linked 4J_Input.lib.
// ---------------------------------------------------------------------------

// Overwrite one function pointer in the IAT with page-permission handling.
static void PatchIATEntry(void** entry, void* newFn, void** oldFn)
{
    if (oldFn)
        *oldFn = *entry;

    DWORD oldProtect = 0;
    VirtualProtect(entry, sizeof(void*), PAGE_READWRITE, &oldProtect);
    *entry = newFn;
    VirtualProtect(entry, sizeof(void*), oldProtect, &oldProtect);
}

static void PatchXInputIAT()
{
    HMODULE hMod = GetModuleHandle(NULL);
    if (!hMod) return;

    auto* dosHdr = reinterpret_cast<IMAGE_DOS_HEADER*>(hMod);
    auto* ntHdrs = reinterpret_cast<IMAGE_NT_HEADERS*>(
        reinterpret_cast<BYTE*>(hMod) + dosHdr->e_lfanew);

    DWORD impRVA = ntHdrs->OptionalHeader
        .DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (!impRVA) return;

    auto* impDesc = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
        reinterpret_cast<BYTE*>(hMod) + impRVA);

    for (; impDesc->Name; impDesc++)
    {
        const char* dllName = reinterpret_cast<char*>(
            reinterpret_cast<BYTE*>(hMod) + impDesc->Name);

        // Match any XInput DLL version (xinput1_4.dll, xinput1_3.dll, etc.)
        if (_strnicmp(dllName, "xinput", 6) != 0)
            continue;

        auto* thunk     = reinterpret_cast<IMAGE_THUNK_DATA*>(
            reinterpret_cast<BYTE*>(hMod) + impDesc->FirstThunk);
        auto* origThunk = reinterpret_cast<IMAGE_THUNK_DATA*>(
            reinterpret_cast<BYTE*>(hMod) + impDesc->OriginalFirstThunk);

        for (; thunk->u1.AddressOfData; thunk++, origThunk++)
        {
            if (origThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
                continue; // ordinal import, skip

            auto* byName = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(
                reinterpret_cast<BYTE*>(hMod) + origThunk->u1.AddressOfData);

            if (strcmp(byName->Name, "XInputGetState") == 0)
            {
                PatchIATEntry(reinterpret_cast<void**>(&thunk->u1.Function),
                              reinterpret_cast<void*>(Hooked_XInputGetState),
                              reinterpret_cast<void**>(&s_origGetState));
            }
            else if (strcmp(byName->Name, "XInputGetCapabilities") == 0)
            {
                PatchIATEntry(reinterpret_cast<void**>(&thunk->u1.Function),
                              reinterpret_cast<void*>(Hooked_XInputGetCapabilities),
                              reinterpret_cast<void**>(&s_origGetCaps));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void DInputManager_Init()
{
    PatchXInputIAT();
    RefreshDInputSlots();
}

void DInputManager_Tick()
{
    if (--s_rescanCountdown <= 0)
    {
        s_rescanCountdown = RESCAN_INTERVAL;
        RefreshDInputSlots();
    }
}

#endif // _WINDOWS64
