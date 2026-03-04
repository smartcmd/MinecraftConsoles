#pragma once

#include "TLSStorage.h"
#include <chrono>

using namespace std::chrono;

// thread local storage

inline DWORD TlsAlloc(VOID) { return TLSStorageCross64::Instance()->Alloc(); }

typedef struct _SYSTEMTIME {
  WORD wYear;
  WORD wMonth;
  WORD wDayOfWeek;
  WORD wDay;
  WORD wHour;
  WORD wMinute;
  WORD wSecond;
  WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

inline VOID GetSystemTime(LPSYSTEMTIME lpSystemTime) {
  const auto dateTime = system_clock::now();
  const auto dp = floor<days>(dateTime);

  const year_month_day ymd{dp};
  const hh_mm_ss hms{dateTime - dp};
  const weekday wd{dp};

  lpSystemTime->wYear = ymd.year();
  lpSystemTime->wMonth = ymd.month();
  lpSystemTime->wDayOfWeek = wd.c_encoding(); // 0 = sunday, etc
  lpSystemTime->wHour = hms.hours();
  lpSystemTime->wMinute = hms.minutes();
  lpSystemTime->wSecond = hms.seconds();

  auto ms = duration_cast<milliseconds>(hms.subseconds());
  lpSystemTime->wMilliseconds = ms.count();
};

inline VOID GetLocalTime(LPSYSTEMTIME lpSystemTime) {
  const auto dateTime = system_clock::now();

  const auto tz = current_zone();
  const auto dateTimeLocal = tz->to_local(dateTime);
  const auto dp = floor<days>(dateTimeLocal);

  const year_month_day ymd{dp};
  const hh_mm_ss hms{dateTimeLocal - dp};
  const weekday wd{dp};

  lpSystemTime->wYear = ymd.year();
  lpSystemTime->wMonth = ymd.month();
  lpSystemTime->wDayOfWeek = wd.c_encoding();
  lpSystemTime->wHour = hms.hours();
  lpSystemTime->wMinute = hms.minutes();
  lpSystemTime->wSecond = hms.seconds();

  auto ms = duration_cast<milliseconds>(hms.subseconds());
  lpSystemTime->wMilliseconds = ms.count();
};

// d3d11 stub work, i sure hope it does
typedef struct _RECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *PRECT;

typedef enum D3D11_BLEND {
  D3D11_BLEND_ZERO = 1,
  D3D11_BLEND_ONE = 2,
  D3D11_BLEND_SRC_COLOR = 3,
  D3D11_BLEND_INV_SRC_COLOR = 4,
  D3D11_BLEND_SRC_ALPHA = 5,
  D3D11_BLEND_INV_SRC_ALPHA = 6,
  D3D11_BLEND_DEST_ALPHA = 7,
  D3D11_BLEND_INV_DEST_ALPHA = 8,
  D3D11_BLEND_DEST_COLOR = 9,
  D3D11_BLEND_INV_DEST_COLOR = 10,
  D3D11_BLEND_SRC_ALPHA_SAT = 11,
  D3D11_BLEND_BLEND_FACTOR = 14,
  D3D11_BLEND_INV_BLEND_FACTOR = 15,
  D3D11_BLEND_SRC1_COLOR = 16,
  D3D11_BLEND_INV_SRC1_COLOR = 17,
  D3D11_BLEND_SRC1_ALPHA = 18,
  D3D11_BLEND_INV_SRC1_ALPHA = 19
} D3D11_BLEND;

typedef void ID3D11Device;
typedef void ID3D11DeviceContext;
typedef void IDXGISwapChain;
typedef RECT D3D11_RECT;
typedef void ID3D11Buffer;
typedef DWORD (*PTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;
