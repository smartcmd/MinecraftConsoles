#pragma once

#include "TLSStorage.h"

typedef struct _RECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *PRECT;

typedef void ID3D11Device;
typedef void ID3D11DeviceContext;
typedef void IDXGISwapChain;
typedef RECT D3D11_RECT;
typedef void ID3D11Buffer;
typedef DWORD (*PTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;
