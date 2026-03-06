//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#ifndef MSS_VERSION

// also update versions below for the docs

// for cdep and installs
#define MILESVERSION      "9.3m"
// see below in docs section
#define MILESMAJORVERSION 9
#define MILESMINORVERSION 3
#define MILESSUBVERSION   11
#define MILESBUILDVERSION 0
#define MILESVERSIONDATE  "20-Jun-14"
#define MILESCOPYRIGHT    "Copyright (C) 1991-2014, RAD Game Tools, Inc."

// source files use these defines
#define MSS_VERSION       MILESVERSION
#define MSS_MAJOR_VERSION MILESMAJORVERSION
#define MSS_MINOR_VERSION MILESMINORVERSION
#define MSS_SUB_VERSION   MILESSUBVERSION
#define MSS_BUILD_VERSION MILESBUILDVERSION

#define MSS_VERSION_DATE  MILESVERSIONDATE
#define MSS_COPYRIGHT     MILESCOPYRIGHT

#endif

#if !defined(MSS_H) && !defined(__RADRES__)
#define MSS_H

// doc system stuff
#ifndef EXPAPI
#define EXPAPI
#endif
#ifndef EXPTYPE
#define EXPTYPE
#endif
#ifndef EXPMACRO
#define EXPMACRO
#endif
#ifndef EXPCONST
#define EXPCONST
#endif
#ifndef EXPOUT
#define EXPOUT
#endif
#ifndef EXPTYPEBEGIN
#define EXPTYPEBEGIN
#endif
#ifndef EXPTYPEEND
#define EXPTYPEEND
#endif
#ifndef EXPGROUP
#define EXPGROUP(GroupName)
#endif
#ifndef DEFGROUP
#define DEFGROUP(GroupName, Info)
#endif

// For docs
EXPGROUP(_NullGroup)
#define MilesVersion "9.3m" EXPMACRO
#define MilesMajorVersion    9  EXPMACRO
#define MilesMinorVersion    3  EXPMACRO
#define MilesBuildNumber     11  EXPMACRO
#define MilesCustomization   0  EXPMACRO
EXPGROUP(_RootGroup)


// IS_WINDOWS for Windows or Win32
// IS_WIN64 for Win64
// IS_WIN32 for Win32
// IS_WIN32API for Windows, Xbox and Xenon
// IS_64REGS when CPU registers are 64-bit - Xenon, PS3, Win64 and PS2
// IS_32 for at least 32-bit pointers
// IS_LE for little endian (PCs)
// IS_BE for big endian (Macs, x360, ps3)
// IS_X86 for Intel
// IS_MAC for Mac
// IS_MACHO for Macho Mac
// IS_PPC for PPC Mac
// IS_68K for 68K Mac
// IS_LINUX for Linux
// IS_XBOX for Xbox
// IS_XENON for Xbox 360
// IS_PS2 for PS/2
// IS_PS3 for PS/3
// IS_SPU for PS3 SPU
// IS_WII for Wii

#include "rrCore.h"

//#define MILES_CHECK_OFFSETS
#ifdef MILES_CHECK_OFFSETS
    #include <stddef.h>
#endif

#ifdef __RADNT__
#define IS_WIN32
#if defined(__RAD64__)
#define IS_WIN64
#endif
#endif

#if defined(__RADWIN__) && !defined(__RADXENON__) && !defined(__RADXBOX__) && !defined(__RADWINRTAPI__)
#define IS_WINDOWS
#endif

#if defined(__RADWIN__)
#define IS_WIN32API
#endif

#if defined(__RAD64__) && defined(__RADWIN__)
#define IS_WIN64
#endif

// 16-bit not supported anymore
#define IS_32

#ifdef __RADLITTLEENDIAN__
#define IS_LE
#endif

#ifdef __RADBIGENDIAN__
#define IS_BE
#endif

#ifdef __RADX86__
#define IS_X86
#endif

#ifdef __RADMAC__
#define IS_MAC
#endif

#ifdef __RADPPC__
#define IS_PPC
#endif

#ifdef __RAD68K__
#define IS_68K
#endif

#ifdef __RADLINUX__
#define IS_LINUX
#endif

//
// MSS_STATIC_RIB is used to determine whether anything loaded
// through the RIB interface is loaded via RIB_load_application_providers
// or via a static declaration from the user (Register_RIB)
// mirror this in rib.h
//
#if defined(__RADANDROID__) || defined(__RADPSP__) || defined(__RADPSP2__) || \
    defined(__RADWII__) || defined(__RADWIIU__) || defined(__RAD3DS__) || defined(__RADIPHONE__) || \
    defined(__RADXENON__) || defined(__RADPS4__) || defined(__RADPS3__) || defined(__RADSPU__) || \
    defined(__RADDURANGO__) || defined(__RADWINRTAPI__)
  #define MSS_STATIC_RIB
  // WinRT is weird in that we statically pull in the RIBs, but we dynamically link Midi
  #ifndef __RADWINRTAPI__
    #define MSS_STATIC_MIDI
  #endif
#elif defined(__RADWIN__) || defined(__RADLINUX__) || defined(__RADMAC__)
  // not static.
#else
  #error "MSS needs to know whether it is being distributed as a static lib!"
#endif

// Retain the old IS_STATIC define for example code
#ifdef MSS_STATIC_RIB
    #define IS_STATIC
#endif

#ifdef __RADXBOX__
#define IS_XBOX
#endif

#ifdef __RADXENON__
#define IS_XENON
#endif

#ifdef __RADWII__
#define IS_WII
#endif

#ifdef __RADWIIU__
#define IS_WIIU
#endif

#ifdef __RADPS2__
#define IS_PS2
#endif

#ifdef __RADPS3__
#define IS_PS3
#ifndef HOST_SPU_PROCESS
  #define HOST_SPU_PROCESS
#endif
#endif

#ifdef __RADSPU__
#define IS_PS3
#define IS_SPU
#endif

#ifdef __RADPSP__
#define IS_PSP
#endif

#ifdef __RADPSP2__
#define IS_PSP2
#endif

#ifdef __RADDOS__
#define IS_DOS
#endif

#ifdef __RAD64REGS__
#define IS_64REGS
#endif

#ifdef __RADMACH__
#define IS_MACHO
#endif

#ifdef __RADIPHONE__
#define IS_IPHONE
#endif

#ifdef __RADIPHONESIM__
#define IS_IPHONESIM
#endif

#ifdef __RAD3DS__
#define IS_3DS
#endif

#define MSSRESTRICT RADRESTRICT

#define MSS_STRUCT RADSTRUCT

#define C8 char
typedef void VOIDFUNC(void);


#if (!defined(IS_LE) && !defined(IS_BE))
  #error MSS.H did not detect your platform.  Define _WINDOWS, WIN32, WIN64, or macintosh.
#endif

//
// Pipeline filters supported on following platforms
//

#define MSS_FLT_SUPPORTED 1
#define EXTRA_BUILD_BUFFERS 1
#define FLT_A (MAX_SPEAKERS)

#if defined(IS_WIN32API)
  #define MSS_VFLT_SUPPORTED 1
#endif

#define MSS_REVERB_SUPPORTED 1

//================

EXPGROUP(Basic Types)
#define AILCALL EXPTAG(AILCALL)
/*
   Internal calling convention that all external Miles functions use.

   Usually cdecl or stdcall on Windows.
*/

#define AILCALLBACK EXPTAG(AILCALLBACK docproto)
/*
   Calling convention that user supplied callbacks <b>from</b> Miles use.

   Usually cdecl or stdcall on Windows.
*/

EXPGROUP(_RootGroup)
#undef AILCALL
#undef AILCALLBACK
//================

RADDEFSTART

#define MSSFOURCC U32
#ifdef IS_LE
  #define MSSMAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((U32)(U8)(ch0) | ((U32)(U8)(ch1) << 8) |   \
                ((U32)(U8)(ch2) << 16) | ((U32)(U8)(ch3) << 24 ))
#else

  #define MSSMAKEFOURCC(ch0, ch1, ch2, ch3)                              \
               (((U32)(U8)(ch0) << 24) | ((U32)(U8)(ch1) << 16) |   \
               ((U32)(U8)(ch2) <<  8) | ((U32)(U8)(ch3)      ))
#endif

#define MSSmmioFOURCC(w,x,y,z) MSSMAKEFOURCC(w,x,y,z)

#if defined(__RADWINRTAPI__)

    #define AILLIBCALLBACK RADLINK
    #define AILCALL RADLINK
    #define AILEXPORT RADEXPLINK
    #define AILCALLBACK  RADLINK
    #define DXDEF RADEXPFUNC
    #define DXDEC RADEXPFUNC

#elif defined(IS_WINDOWS)

  typedef char CHAR;
  typedef short SHORT;
  typedef int                 BOOL;
  typedef long LONG;
  typedef CHAR *LPSTR, *PSTR;

  #ifdef IS_WIN64
      typedef unsigned long long ULONG_PTR, *PULONG_PTR;
  #else
    #ifdef _Wp64
      #if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
        typedef __w64 unsigned long ULONG_PTR, *PULONG_PTR;
      #else
        typedef unsigned long ULONG_PTR, *PULONG_PTR;
      #endif
    #else
      typedef unsigned long ULONG_PTR, *PULONG_PTR;
    #endif
  #endif

  typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
  typedef unsigned long       DWORD;
  typedef unsigned short      WORD;
  typedef unsigned int        UINT;
  typedef struct HWAVE__ *HWAVE;
  typedef struct HWAVEIN__ *HWAVEIN;
  typedef struct HWAVEOUT__ *HWAVEOUT;
  typedef HWAVEIN  *LPHWAVEIN;
  typedef HWAVEOUT  *LPHWAVEOUT;

  #ifndef WAVE_MAPPER
    #define WAVE_MAPPER     ((UINT)-1)
  #endif

  typedef struct waveformat_tag *LPWAVEFORMAT;

  typedef struct HMIDIOUT__ *HMIDIOUT;
  typedef HMIDIOUT  *LPHMIDIOUT;
  typedef struct HWND__ *HWND;
  typedef struct HINSTANCE__ *HINSTANCE;
  typedef HINSTANCE HMODULE;
  typedef struct wavehdr_tag *LPWAVEHDR;

  #define MSS_MAIN_DEF __cdecl

  //
  // If compiling MSS DLL, use __declspec(dllexport) for both
  // declarations and definitions
  //

  #ifdef IS_WIN32

    #if !defined(FORNONWIN) && !defined(__RADNTBUILDLINUX__)
      #define AILLIBCALLBACK __stdcall
      #define AILCALL        __stdcall
      #define AILCALLBACK    __stdcall
      #define AILEXPORT      __stdcall
    #else
      #define AILLIBCALLBACK __cdecl
      #define AILCALL        __cdecl
      #define AILCALLBACK    __cdecl
      #define AILEXPORT      __cdecl
    #endif

    #ifdef __RADINDLL__
      #define DXDEC __declspec(dllexport)
      #define DXDEF __declspec(dllexport)
    #else

      #if defined( __BORLANDC__ ) || defined( MSS_SPU_PROCESS )
        #define DXDEC extern
      #else
        #define DXDEC __declspec(dllimport)
      #endif

    #endif

    #ifdef IS_WIN64
      #define MSSDLLNAME "MSS64.DLL"
      #define MSS_REDIST_DIR_NAME "redist64"
    #else
      #define MSSDLLNAME "MSS32.DLL"
      #define MSS_REDIST_DIR_NAME "redist"
    #endif

    #define MSS_DIR_SEP "\\"
    #define MSS_DIR_UP ".." MSS_DIR_SEP
    #define MSS_DIR_UP_TWO MSS_DIR_UP MSS_DIR_UP

  #endif

  typedef void * LPVOID;
  typedef LPVOID AILLPDIRECTSOUND;
  typedef LPVOID AILLPDIRECTSOUNDBUFFER;

#elif defined( IS_MAC ) || defined(IS_IPHONE) || defined(IS_LINUX)

  #if defined(__RADARM__) || defined(__RADX64__)
    #define AILLIBCALLBACK
    #define AILCALL
    #define AILEXPORT
    #define AILCALLBACK
  #elif defined(__RADX86__)
    #define AILLIBCALLBACK __attribute__((cdecl))
    #define AILCALL        __attribute__((cdecl))
    #define AILCALLBACK    __attribute__((cdecl))
    #define AILEXPORT      __attribute__((cdecl))
  #else
    #error "No fn call decorators specified"
  #endif

  #ifdef __RADINDLL__
    #define DXDEC __attribute__((visibility("default")))
    #define DXDEF __attribute__((visibility("default")))
  #else
    #define DXDEC extern
    #define DXDEF
  #endif

  #ifdef __RADX64__
    #define MSS_REDIST_DIR_NAME "redist/x64"
  #elif defined(IS_X86)
    #define MSS_REDIST_DIR_NAME "redist/x86"
  #elif defined(__RADARM__)
    #define MSS_REDIST_DIR_NAME ""
  #else
    #error "No Redist Dir Specified"
  #endif

  #define MSS_DIR_SEP "/"
  #define MSS_DIR_UP ".." MSS_DIR_SEP
  #define MSS_DIR_UP_TWO MSS_DIR_UP MSS_DIR_UP

  #define MSS_MAIN_DEF

#elif defined(IS_XENON)

  #define AILLIBCALLBACK __stdcall
  #define AILCALL        __stdcall
  #define AILEXPORT      __stdcall
  #define AILCALLBACK    __stdcall

  #define DXDEC extern
  #define DXDEF

  typedef void * AILLPDIRECTSOUND;
  typedef void * AILLPDIRECTSOUNDBUFFER;

#else

  #define AILLIBCALLBACK
  #define AILCALL
  #define AILEXPORT
  #define AILCALLBACK

  #define DXDEC extern
  #define DXDEF

#endif


//
// Misc. constant definitions
//

#define MAX_DRVRS                16       // Max. # of simultaneous drivers
#define MAX_TIMERS               16       // Max. # of simultaneous timers
#define MAX_NOTES                32       // Max # of notes "on"
#define FOR_NEST                 4        // # of nested XMIDI FOR loops
#define NUM_CHANS                16       // # of possible MIDI channels
#define MAX_W_VOICES             16       // Max virtual wave synth voice cnt
#define MAX_W_ENTRIES            512      // 512 wave library entries max.
#ifdef IS_WIN32
#define MAX_SPEAKERS             9        // Up to 9 hardware output channels supported on Win32
#elif defined(IS_PS3) || defined(IS_WII) || defined(IS_WIIU) || defined(__RADWINRTAPI__) || defined(__RADSEKRIT2__)
#define MAX_SPEAKERS             8        // Up to 8 hardware output channels on PS3, PS2, Wii, WiiU
#elif defined(IS_PSP) || defined(IS_IPHONE) || defined(IS_3DS) || defined(IS_PSP2) || defined(__RADANDROID__)
#define MAX_SPEAKERS             2        // Up to 2 hardware output channels on PSP
#else
#define MAX_SPEAKERS             6        // Up to 6 hardware output channels supported on other platforms
#endif
#define MAX_RECEIVER_SPECS       32       // Up to 32 receiver point specifications

#define MAX_BUSSES               4        // # of busses that can be active.
#define MILES_MAX_STATES         4        // # of state pushes allowed.


#define MIN_CHAN                 ( 1-1)   // Min channel recognized (0-based)
#define MAX_CHAN                 (16-1)   // Max channel recognized
#define MIN_LOCK_CHAN            ( 1-1)   // Min channel available for locking
#define MAX_LOCK_CHAN            (16-1)   // Max channel available for locking
#define PERCUSS_CHAN             (10-1)   // Percussion channel (no locking)

#define AIL_MAX_FILE_HEADER_SIZE 8192     // AIL_set_named_sample_file() requires at least 8K
                                          // of data or the entire file image, whichever is less,
                                          // to determine sample format
#define DIG_F_16BITS_MASK          1
#define DIG_F_STEREO_MASK          2
#define DIG_F_ADPCM_MASK           4
#define DIG_F_XBOX_ADPCM_MASK      8
#define DIG_F_MULTICHANNEL_MASK    16
#define DIG_F_OUTPUT_FILTER_IN_USE 32

#define DIG_F_MONO_8                     0        // PCM data formats
#define DIG_F_MONO_16                    (DIG_F_16BITS_MASK)
#define DIG_F_STEREO_8                   (DIG_F_STEREO_MASK)
#define DIG_F_MULTICHANNEL_8             (DIG_F_MULTICHANNEL_MASK)   // (not actually supported)
#define DIG_F_STEREO_16                  (DIG_F_STEREO_MASK|DIG_F_16BITS_MASK)
#define DIG_F_MULTICHANNEL_16            (DIG_F_MULTICHANNEL_MASK|DIG_F_16BITS_MASK)
#define DIG_F_ADPCM_MONO_16              (DIG_F_ADPCM_MASK |DIG_F_16BITS_MASK)
#define DIG_F_ADPCM_STEREO_16            (DIG_F_ADPCM_MASK |DIG_F_16BITS_MASK|DIG_F_STEREO_MASK)
#define DIG_F_ADPCM_MULTICHANNEL_16      (DIG_F_ADPCM_MASK |DIG_F_16BITS_MASK|DIG_F_STEREO_MASK)
#define DIG_F_XBOX_ADPCM_MONO_16         (DIG_F_XBOX_ADPCM_MASK |DIG_F_16BITS_MASK)
#define DIG_F_XBOX_ADPCM_STEREO_16       (DIG_F_XBOX_ADPCM_MASK |DIG_F_16BITS_MASK|DIG_F_STEREO_MASK)
#define DIG_F_XBOX_ADPCM_MULTICHANNEL_16 (DIG_F_XBOX_ADPCM_MASK |DIG_F_16BITS_MASK|DIG_F_MULTICHANNEL_MASK)

#define DIG_F_NOT_8_BITS (DIG_F_16BITS_MASK | DIG_F_ADPCM_MASK | DIG_F_XBOX_ADPCM_MASK | DIG_F_MULTICHANNEL_MASK)

#define DIG_F_USING_ASI          16

#define DIG_PCM_POLARITY         0x0004   // PCM flags used by driver hardware
#define DIG_PCM_SPLIT            0x0008
#define DIG_BUFFER_SERVICE       0x0010
#define DIG_DUAL_DMA             0x0020
#define DIG_RECORDING_SUPPORTED  0x8000

#ifndef WAVE_FORMAT_PCM
  #define WAVE_FORMAT_PCM          1
#endif
#ifndef WAVE_FORMAT_IMA_ADPCM
  #define WAVE_FORMAT_IMA_ADPCM    0x0011
#endif
#ifndef WAVE_FORMAT_XBOX_ADPCM
  #define WAVE_FORMAT_XBOX_ADPCM   0x0069
#endif
#ifndef WAVE_FORMAT_EXTENSIBLE
  #define WAVE_FORMAT_EXTENSIBLE   0xFFFE
#endif

typedef enum
{
   MSS_SPEAKER_MONO                  = 0,
   MSS_SPEAKER_FRONT_LEFT            = 0,     // Speaker order indexes correspond to
   MSS_SPEAKER_FRONT_RIGHT           = 1,     // bitmasks in PSDK's ksmedia.h
   MSS_SPEAKER_FRONT_CENTER          = 2,     // Also see microsoft.com/whdc/device/audio/multichaud.mspx
   MSS_SPEAKER_LOW_FREQUENCY         = 3,
   MSS_SPEAKER_BACK_LEFT             = 4,
   MSS_SPEAKER_BACK_RIGHT            = 5,
   MSS_SPEAKER_FRONT_LEFT_OF_CENTER  = 6,
   MSS_SPEAKER_FRONT_RIGHT_OF_CENTER = 7,
   MSS_SPEAKER_BACK_CENTER           = 8,
   MSS_SPEAKER_SIDE_LEFT             = 9,
   MSS_SPEAKER_SIDE_RIGHT            = 10,
   MSS_SPEAKER_TOP_CENTER            = 11,
   MSS_SPEAKER_TOP_FRONT_LEFT        = 12,
   MSS_SPEAKER_TOP_FRONT_CENTER      = 13,
   MSS_SPEAKER_TOP_FRONT_RIGHT       = 14,
   MSS_SPEAKER_TOP_BACK_LEFT         = 15,
   MSS_SPEAKER_TOP_BACK_CENTER       = 16,
   MSS_SPEAKER_TOP_BACK_RIGHT        = 17,
   MSS_SPEAKER_MAX_INDEX             = 17,
   MSS_SPEAKER_FORCE_32              = 0x7fffffff
} MSS_SPEAKER;

//
// Pass to AIL_midiOutOpen for NULL MIDI driver
//

#define MIDI_NULL_DRIVER ((U32)(S32)-2)


//
// Non-specific XMIDI/MIDI controllers and event types
//

#define SYSEX_BYTE        105
#define PB_RANGE          106
#define CHAN_MUTE         107
#define CALLBACK_PFX      108
#define SEQ_BRANCH        109
#define CHAN_LOCK         110
#define CHAN_PROTECT      111
#define VOICE_PROTECT     112
#define TIMBRE_PROTECT    113
#define PATCH_BANK_SEL    114
#define INDIRECT_C_PFX    115
#define FOR_LOOP          116
#define NEXT_LOOP         117
#define CLEAR_BEAT_BAR    118
#define CALLBACK_TRIG     119
#define SEQ_INDEX         120

#define GM_BANK_MSB       0
#define MODULATION        1
#define DATA_MSB          6
#define PART_VOLUME       7
#define PANPOT            10
#define EXPRESSION        11
#define GM_BANK_LSB       32
#define DATA_LSB          38
#define SUSTAIN           64
#define REVERB            91
#define CHORUS            93
#define RPN_LSB           100
#define RPN_MSB           101
#define RESET_ALL_CTRLS   121
#define ALL_NOTES_OFF     123

#define EV_NOTE_OFF       0x80
#define EV_NOTE_ON        0x90
#define EV_POLY_PRESS     0xa0
#define EV_CONTROL        0xb0
#define EV_PROGRAM        0xc0
#define EV_CHAN_PRESS     0xd0
#define EV_PITCH          0xe0
#define EV_SYSEX          0xf0
#define EV_ESC            0xf7
#define EV_META           0xff

#define META_EOT          0x2f
#define META_TEMPO        0x51
#define META_TIME_SIG     0x58

//
// SAMPLE.system_data[] usage
//

#define VOC_BLK_PTR       1  // Pointer to current block
#define VOC_REP_BLK       2  // Pointer to beginning of repeat loop block
#define VOC_N_REPS        3  // # of iterations left in repeat loop
#define VOC_MARKER        4  // Marker to search for, or -1 if all
#define VOC_MARKER_FOUND  5  // Desired marker found if 1, else 0
#define STR_HSTREAM       6  // Stream, if any, that owns the HSAMPLE
#define SSD_TEMP          7  // Temporary storage location for general use
#define EVT_HANDLE_MAGIC  1  // EventSystem handle.magic
#define EVT_HANDLE_INDEX  2  // EventSystem handle.index

//
// Timer status values
//

#define AILT_FREE         0         // Timer handle is free for allocation
#define AILT_STOPPED      1         // Timer is stopped
#define AILT_RUNNING      2         // Timer is running

//
// SAMPLE.status flag values
//

#define SMP_FREE          0x0001    // Sample is available for allocation

#define SMP_DONE          0x0002    // Sample has finished playing, or has
                                    // never been started

#define SMP_PLAYING       0x0004    // Sample is playing

#define SMP_STOPPED       0x0008    // Sample has been stopped

#define SMP_PLAYINGBUTRELEASED 0x0010 // Sample is playing, but digital handle
                                      // has been temporarily released



//
// SEQUENCE.status flag values
//

#define SEQ_FREE          0x0001    // Sequence is available for allocation

#define SEQ_DONE          0x0002    // Sequence has finished playing, or has
                                    // never been started

#define SEQ_PLAYING       0x0004    // Sequence is playing

#define SEQ_STOPPED       0x0008    // Sequence has been stopped

#define SEQ_PLAYINGBUTRELEASED 0x0010 // Sequence is playing, but MIDI handle
                                      // has been temporarily released

#ifdef IS_WINDOWS

//
// AIL_set_direct_buffer_control() command values
//

#define AILDS_RELINQUISH  0         // App returns control of secondary buffer
#define AILDS_SEIZE       1         // App takes control of secondary buffer
#define AILDS_SEIZE_LOOP  2         // App wishes to loop the secondary buffer

#endif

#ifndef MSS_BASIC

#ifndef FILE_ERRS
  #define FILE_ERRS

  #define AIL_NO_ERROR        0
  #define AIL_IO_ERROR        1
  #define AIL_OUT_OF_MEMORY   2
  #define AIL_FILE_NOT_FOUND  3
  #define AIL_CANT_WRITE_FILE 4
  #define AIL_CANT_READ_FILE  5
  #define AIL_DISK_FULL       6
  #define AIL_NO_AVAIL_ASYNC  7
#endif

#define MIN_VAL 0
#define NOM_VAL 1
#define MAX_VAL 2


EXPGROUP(Basic Types)
EXPTYPEBEGIN typedef SINTa HMSSENUM;
#define MSS_FIRST ((HMSSENUM)-1)
EXPTYPEEND
/*
  specifies a type used to enumerate through a list of properties.

  $:MSS_FIRST use this value to start the enumeration process.

The Miles enumeration functions all work similarly - you set a local variable of type HMSSENUM to MSS_FIRST and then call
the enumeration function until it returns 0.

*/




//
// Preference names and default values
//

#define AIL_MM_PERIOD              0
#define DEFAULT_AMP                1      // Default MM timer period = 5 msec.

#define AIL_TIMERS                 1
#define DEFAULT_AT                 16     // 16 allocatable HTIMER handles

#define AIL_ENABLE_MMX_SUPPORT     2      // Enable MMX support if present
#define DEFAULT_AEMS               YES    // (may be changed at any time)


#define DIG_MIXER_CHANNELS         3
#define DEFAULT_DMC                64     // 64 allocatable SAMPLE structures

#define DIG_ENABLE_RESAMPLE_FILTER 4      // Enable resampling filter by
#define DEFAULT_DERF               YES    // default

#define DIG_RESAMPLING_TOLERANCE   5
#define DEFAULT_DRT                131    // Resampling triggered at +/- 0.2%

// 1 ms per mix. The PS3 has frag count restrictions, so we use 5 ms.
#define DIG_DS_FRAGMENT_SIZE        6
#ifdef __RADPS3__
#   define DEFAULT_DDFS             5
#else
#   define DEFAULT_DDFS             1
#endif

// We want ~256 ms of buffers. PS3 must be 8, 16, or 32.
#define DIG_DS_FRAGMENT_CNT         7
#ifdef __RADPS3__
#   define DEFAULT_DDFC             32
#else
#   define DEFAULT_DDFC             256
#endif

// Mix ahead ~48 ms. PS3 is based off on 5 ms frag size above...
#define DIG_DS_MIX_FRAGMENT_CNT     8
#ifdef __RADPS3__
#   define DEFAULT_DDMFC            8
#else
#   define DEFAULT_DDMFC            48
#endif

#define DIG_LEVEL_RAMP_SAMPLES     9
#define DEFAULT_DLRS               32     // Ramp level changes over first 32 samples in each buffer to reduce zipper noise


#define DIG_MAX_PREDELAY_MS        10
#define DEFAULT_MPDMS              500    // Max predelay reverb time in ms

#define DIG_3D_MUTE_AT_MAX         11
#define DEFAULT_D3MAM              YES    // on by default

#define DIG_DS_USE_PRIMARY         12
#define DEFAULT_DDUP               NO     // Mix into secondary DirectSound buffer by default


#define DIG_DS_DSBCAPS_CTRL3D      13
#define DEFAULT_DDDC               NO     // Do not use DSBCAPS_CTRL3D by default

#define DIG_DS_CREATION_HANDLER    14
#define DEFAULT_DDCH               0      // Use DirectSoundCreate() by default


#define DIG_MAX_CHAIN_ELEMENT_SIZE 15
#define DEFAULT_MCES               8192   // max of 8192 bytes/waveOut buffer

#define DIG_MIN_CHAIN_ELEMENT_TIME 16
#define DEFAULT_MCET               100    // 100 milliseconds buffers


#define DIG_USE_WAVEOUT            17
#define DEFAULT_DUW                NO     // Use DirectSound by default

#define DIG_OUTPUT_BUFFER_SIZE     18
#define DEFAULT_DOBS               49152  // Windows: waveout 48K output buffer size


#define DIG_PREFERRED_WO_DEVICE    19
#define DEFAULT_DPWOD              ((UINTa)-1)  // Preferred WaveOut device == WAVE_MAPPER

#define DIG_PREFERRED_DS_DEVICE    20
#define DEFAULT_DPDSD              0      // Preferred DirectSound device == default NULL GUID


#define MDI_SEQUENCES              21
#define DEFAULT_MS                 8      // 8 sequence handles/driver

#define MDI_SERVICE_RATE           22
#define DEFAULT_MSR                120    // XMIDI sequencer timing = 120 Hz

#define MDI_DEFAULT_VOLUME         23
#define DEFAULT_MDV                127    // Default sequence volume = 127 (0-127)

#define MDI_QUANT_ADVANCE          24
#define DEFAULT_MQA                1      // Beat/bar count +1 interval

#define MDI_ALLOW_LOOP_BRANCHING   25
#define DEFAULT_ALB                NO     // Branches cancel XMIDI FOR loops

#define MDI_DEFAULT_BEND_RANGE     26
#define DEFAULT_MDBR               2      // Default pitch-bend range = 2

#define MDI_DOUBLE_NOTE_OFF        27
#define DEFAULT_MDNO               NO     // For stuck notes on SB daughterboards

#define MDI_SYSEX_BUFFER_SIZE      28
#define DEFAULT_MSBS               1536   // Default sysex buffer = 1536 bytes


#define DLS_VOICE_LIMIT            29
#define DEFAULT_DVL                64     // 64 voices supported

#define DLS_TIMEBASE               30
#define DEFAULT_DTB                120    // 120 intervals/second by default

#define DLS_BANK_SELECT_ALIAS      31
#define DEFAULT_DBSA               NO     // Do not treat controller 114 as bank

#define DLS_STREAM_BOOTSTRAP       32     // Don't submit first stream buffer
#define DEFAULT_DSB                YES    // until at least 2 available

#define DLS_VOLUME_BOOST           33
#define DEFAULT_DVB                0      // Boost final volume by 0 dB

#define DLS_ENABLE_FILTERING       34     // Filtering = on by default
#define DEFAULT_DEF                YES    // (may be changed at any time)


#define DLS_GM_PASSTHROUGH         35     // Pass unrecognized traffic on to
#define DEFAULT_DGP                YES    // default GM driver layer
                                          // (may be changed at any time)

#define DLS_ADPCM_TO_ASI_THRESHOLD 36     // Size in samples to switch to ASI
#define DEFAULT_DATAT              32768

#ifdef __RAD3DS__
#   define AIL_3DS_USE_SYSTEM_CORE 32    // Defaults to 0
#endif

#define N_PREFS 40                        // # of preference types

#if defined(IS_WIN32API) || defined(IS_WII)
  #pragma pack(push, 1)
#endif

typedef struct Mwavehdr_tag {
    C8 *       lpData;
    U32       dwBufferLength;
    U32       dwBytesRecorded;
    UINTa   dwUser;
    U32       dwFlags;
    U32       dwLoops;
    struct Mwavehdr_tag  *lpNext;
    UINTa   reserved;
} MWAVEHDR;
typedef MSS_STRUCT Mwaveformat_tag {
    U16    wFormatTag;
    U16    nChannels;
    U32    nSamplesPerSec;
    U32    nAvgBytesPerSec;
    U16    nBlockAlign;
} MWAVEFORMAT;
typedef MSS_STRUCT Mpcmwaveformat_tag {
    MWAVEFORMAT  wf;
    U16        wBitsPerSample;
} MPCMWAVEFORMAT;
typedef MSS_STRUCT Mwaveformatex_tag {
    U16    wFormatTag;
    U16    nChannels;
    U32    nSamplesPerSec;
    U32    nAvgBytesPerSec;
    U16    nBlockAlign;
    U16    wBitsPerSample;
    U16    cbSize;
} MWAVEFORMATEX;
typedef MSS_STRUCT Mwaveformatextensible_tag {
  MWAVEFORMATEX Format;
  union {
      U16 wValidBitsPerSample;
      U16 wSamplesPerBlock;
      U16 wReserved;
  } Samples;
  U32 dwChannelMask;
  U8 SubFormat[16];
} MWAVEFORMATEXTENSIBLE;

#if defined(IS_WIN32API) || defined(IS_WII)
  #pragma pack(pop)
#endif

// This will fail if structure packing isn't correct for the compiler we are running.
RR_COMPILER_ASSERT(sizeof(MWAVEFORMATEXTENSIBLE) == 40);


typedef struct _AILSOUNDINFO {
  S32 format;
  void const* data_ptr;
  U32 data_len;
  U32 rate;
  S32 bits;
  S32 channels;
  U32 channel_mask;
  U32 samples;
  U32 block_size;
  void const* initial_ptr;
} AILSOUNDINFO;

// asis use these callbacks
typedef void * (AILCALL MSS_ALLOC_TYPE)( UINTa size, UINTa user, char const * filename, U32 line );
typedef void (AILCALL MSS_FREE_TYPE)( void * ptr, UINTa user, char const * filename, U32 line );

// helper functions that just turn around and call AIL_mem_alloc_lock
DXDEC void * AILCALL MSS_alloc_info( UINTa size, UINTa user, char const * filename, U32 line );
DXDEC void AILCALL MSS_free_info( void * ptr, UINTa user, char const * filename, U32 line );

#if defined(STANDALONEMIXRIB) && !defined(FORNONWIN)
#define MSS_CALLBACK_ALIGNED_NAME( name ) name##_fixup
#define MSS_DEC_CB_STACK_ALIGN( name ) DXDEC void AILCALL MSS_CALLBACK_ALIGNED_NAME(name)(void);
#else
#define MSS_CALLBACK_ALIGNED_NAME( name ) name
#define MSS_DEC_CB_STACK_ALIGN( name )
#endif

MSS_DEC_CB_STACK_ALIGN( MSS_alloc_info )
MSS_DEC_CB_STACK_ALIGN( MSS_free_info)


#ifndef RIB_H        // RIB.H contents included if RIB.H not already included

#define RIB_H
#define ARY_CNT(x) (sizeof((x)) / sizeof((x)[0]))

// ----------------------------------
// RIB data types
// ----------------------------------

typedef S32 RIBRESULT;

#define RIB_NOERR                    0   // Success -- no error
#define RIB_NOT_ALL_AVAILABLE        1   // Some requested functions/attribs not available
#define RIB_NOT_FOUND                2   // Resource not found
#define RIB_OUT_OF_MEM               3   // Out of system RAM

//
// Handle to interface provider
//

typedef UINTa HPROVIDER;

//
// Handle representing token used to obtain property data
//
// This needs to be large enough to store a function pointer
//

typedef UINTa HPROPERTY;

//
// Data types for RIB properties
//

typedef enum
{
   RIB_NONE = 0, // No type
   RIB_CUSTOM,   // Used for pointers to application-specific structures
   RIB_DEC,      // Used for 32-bit integer values to be reported in decimal
   RIB_HEX,      // Used for 32-bit integer values to be reported in hex
   RIB_FLOAT,    // Used for 32-bit single-precision FP values
   RIB_PERCENT,  // Used for 32-bit single-precision FP values to be reported as percentages
   RIB_BOOL,     // Used for Boolean-constrained integer values to be reported as TRUE or FALSE
   RIB_STRING,   // Used for pointers to null-terminated ASCII strings
   RIB_READONLY = 0x80000000  // Property is read-only
}
RIB_DATA_SUBTYPE;

//
// RIB_ENTRY_TYPE structure, used to register an interface or request one
//

typedef enum
{
   RIB_FUNCTION = 0,
   RIB_PROPERTY,       // Property: read-only or read-write data type
   RIB_ENTRY_FORCE_32 = 0x7fffffff
}
RIB_ENTRY_TYPE;

//
// RIB_INTERFACE_ENTRY, used to represent a function or data entry in an
// interface
//

typedef struct
{
   RIB_ENTRY_TYPE   type;        // See list above
   const C8          *entry_name;  // Name of desired function or property
   UINTa            token;       // Function pointer or property token
   RIB_DATA_SUBTYPE subtype;     // Property subtype
}
RIB_INTERFACE_ENTRY;

//
// Standard RAD Interface Broker provider identification properties
//

#define PROVIDER_NAME    ((U32) (S32) (-100))    // RIB_STRING name of decoder
#define PROVIDER_VERSION ((U32) (S32) (-101))    // RIB_HEX BCD version number

//
// Standard function to obtain provider properties (see PROVIDER_ defines
// above)
//
// Each provider of a searchable interface must export this function
//

typedef S32 (AILCALL *PROVIDER_PROPERTY) (HPROPERTY        index,
                                              void *       before_value,
                                              void const * new_value,
                                              void *       after_value
                                              );

//
// Macros to simplify interface registrations/requests for functions,
// and properties
//

#define FN(entry_name)        { RIB_FUNCTION, #entry_name, (UINTa) &(entry_name), RIB_NONE }
#define REG_FN(entry_name)    { RIB_FUNCTION, #entry_name, (UINTa) &(entry_name), RIB_NONE }

#define PR(entry_name,ID)             { RIB_PROPERTY, (entry_name), (UINTa) &(ID), RIB_NONE }
#define REG_PR(entry_name,ID,subtype) { RIB_PROPERTY, (entry_name), (UINTa)  (ID), subtype  }

#define RIB_register(x,y,z)   RIB_register_interface  ((HPROVIDER)(x), y, ARY_CNT(z), z)
#define RIB_unregister(x,y,z) RIB_unregister_interface((HPROVIDER)(ssx), y, ARY_CNT(z), z)
#define RIB_unregister_all(x) RIB_unregister_interface((HPROVIDER)(x), 0, 0, 0)
#define RIB_free_libraries()  RIB_free_provider_library((HPROVIDER)(0));
#define RIB_request(x,y,z)    RIB_request_interface   (x, y, ARY_CNT(z), z)

// passed to RIB DLLs in Miles 9 and up (so RIBS don't have to link to MSS32.dll)
typedef HPROVIDER AILCALL RIB_ALLOC_PROVIDER_HANDLE_TYPE(long           module);

typedef RIBRESULT AILCALL RIB_REGISTER_INTERFACE_TYPE (HPROVIDER                      provider,
                                                       C8 const                   *interface_name,
                                                       S32                            entry_count,
                                                       RIB_INTERFACE_ENTRY const *rlist);

typedef RIBRESULT  AILCALL RIB_UNREGISTER_INTERFACE_TYPE (HPROVIDER                      provider,
                                                          C8 const                  *interface_name,
                                                          S32                            entry_count,
                                                          RIB_INTERFACE_ENTRY const *rlist);

#define RIB_registerP(x,y,z)   rib_reg   ((HPROVIDER)(x), y, ARY_CNT(z), z)
#define RIB_unregister_allP(x) rib_unreg ((HPROVIDER)(x), 0, 0, 0)


// ----------------------------------
// Standard RIB API prototypes
// ----------------------------------

DXDEC  HPROVIDER  AILCALL RIB_alloc_provider_handle   (long           module);
DXDEC  void       AILCALL RIB_free_provider_handle    (HPROVIDER      provider);

DXDEC  HPROVIDER  AILCALL RIB_load_provider_library   (C8 const *filename);
DXDEC  void       AILCALL RIB_free_provider_library   (HPROVIDER      provider);

DXDEC  RIBRESULT  AILCALL RIB_register_interface      (HPROVIDER                      provider,
                                                       C8 const                   *interface_name,
                                                       S32                            entry_count,
                                                       RIB_INTERFACE_ENTRY const *rlist);

DXDEC  RIBRESULT  AILCALL RIB_unregister_interface    (HPROVIDER                      provider,
                                                       C8 const                  *interface_name,
                                                       S32                            entry_count,
                                                       RIB_INTERFACE_ENTRY const *rlist);

DXDEC  RIBRESULT  AILCALL RIB_request_interface       (HPROVIDER                provider,
                                                       C8 const            *interface_name,
                                                       S32                      entry_count,
                                                       RIB_INTERFACE_ENTRY *rlist);

DXDEC  RIBRESULT  AILCALL RIB_request_interface_entry (HPROVIDER                provider,
                                                       C8 const            *interface_name,
                                                       RIB_ENTRY_TYPE           entry_type,
                                                       C8 const            *entry_name,
                                                       UINTa               *token);

DXDEC  S32        AILCALL RIB_enumerate_interface     (HPROVIDER                provider,
                                                       C8 const                *interface_name,
                                                       RIB_ENTRY_TYPE           type,
                                                       HMSSENUM            *next,
                                                       RIB_INTERFACE_ENTRY *dest);

DXDEC  S32        AILCALL RIB_enumerate_providers     (C8 const            *interface_name,
                                                       HMSSENUM            *next,
                                                       HPROVIDER           *dest);

DXDEC  C8 *   AILCALL RIB_type_string             (void const *         data,
                                                       RIB_DATA_SUBTYPE         subtype);

DXDEC  HPROVIDER  AILCALL RIB_find_file_provider      (C8 const            *interface_name,
                                                       C8 const            *property_name,
                                                       C8 const            *file_suffix);

DXDEC  HPROVIDER  AILCALL RIB_find_provider           (C8 const            *interface_name,
                                                       C8 const            *property_name,
                                                       void const          *property_value);

//
// Static library definitions
//

#ifdef MSS_STATIC_RIB
  #define RIB_MAIN_NAME( name ) name##_RIB_Main

  DXDEC S32 AILCALL RIB_MAIN_NAME(SRS)( HPROVIDER provider_handle, U32 up_down, RIB_ALLOC_PROVIDER_HANDLE_TYPE * rib_alloc, RIB_REGISTER_INTERFACE_TYPE * rib_reg, RIB_UNREGISTER_INTERFACE_TYPE * rib_unreg );
  DXDEC S32 AILCALL RIB_MAIN_NAME(DTS)( HPROVIDER provider_handle, U32 up_down, RIB_ALLOC_PROVIDER_HANDLE_TYPE * rib_alloc, RIB_REGISTER_INTERFACE_TYPE * rib_reg, RIB_UNREGISTER_INTERFACE_TYPE * rib_unreg );
  DXDEC S32 AILCALL RIB_MAIN_NAME(DolbySurround)( HPROVIDER provider_handle, U32 up_down, RIB_ALLOC_PROVIDER_HANDLE_TYPE * rib_alloc, RIB_REGISTER_INTERFACE_TYPE * rib_reg, RIB_UNREGISTER_INTERFACE_TYPE * rib_unreg );
  DXDEC S32 AILCALL RIB_MAIN_NAME(MP3Dec)( HPROVIDER provider_handle, U32 up_down, RIB_ALLOC_PROVIDER_HANDLE_TYPE * rib_alloc, RIB_REGISTER_INTERFACE_TYPE * rib_reg, RIB_UNREGISTER_INTERFACE_TYPE * rib_unreg );
  DXDEC S32 AILCALL RIB_MAIN_NAME(OggDec)( HPROVIDER provider_handle, U32 up_down, RIB_ALLOC_PROVIDER_HANDLE_TYPE * rib_alloc, RIB_REGISTER_INTERFACE_TYPE * rib_reg, RIB_UNREGISTER_INTERFACE_TYPE * rib_unreg );
  DXDEC S32 AILCALL RIB_MAIN_NAME(BinkADec)( HPROVIDER provider_handle, U32 up_down, RIB_ALLOC_PROVIDER_HANDLE_TYPE * rib_alloc, RIB_REGISTER_INTERFACE_TYPE * rib_reg, RIB_UNREGISTER_INTERFACE_TYPE * rib_unreg );
  DXDEC S32 AILCALL RIB_MAIN_NAME(SpxDec)( HPROVIDER provider_handle, U32 up_down, RIB_ALLOC_PROVIDER_HANDLE_TYPE * rib_alloc, RIB_REGISTER_INTERFACE_TYPE * rib_reg, RIB_UNREGISTER_INTERFACE_TYPE * rib_unreg );
  DXDEC S32 AILCALL RIB_MAIN_NAME(SpxEnc)( HPROVIDER provider_handle, U32 up_down, RIB_ALLOC_PROVIDER_HANDLE_TYPE * rib_alloc, RIB_REGISTER_INTERFACE_TYPE * rib_reg, RIB_UNREGISTER_INTERFACE_TYPE * rib_unreg );
  DXDEC S32 AILCALL RIB_MAIN_NAME(Voice)( HPROVIDER provider_handle, U32 up_down, RIB_ALLOC_PROVIDER_HANDLE_TYPE * rib_alloc, RIB_REGISTER_INTERFACE_TYPE * rib_reg, RIB_UNREGISTER_INTERFACE_TYPE * rib_unreg );
  DXDEC S32 AILCALL RIB_MAIN_NAME(SpxVoice)( HPROVIDER provider_handle, U32 up_down, RIB_ALLOC_PROVIDER_HANDLE_TYPE * rib_alloc, RIB_REGISTER_INTERFACE_TYPE * rib_reg, RIB_UNREGISTER_INTERFACE_TYPE * rib_unreg );
  DXDEC S32 AILCALL RIB_MAIN_NAME(DSP)( HPROVIDER provider_handle, U32 up_down, RIB_ALLOC_PROVIDER_HANDLE_TYPE * rib_alloc, RIB_REGISTER_INTERFACE_TYPE * rib_reg, RIB_UNREGISTER_INTERFACE_TYPE * rib_unreg );

#ifdef IS_XENON
  DXDEC S32 AILCALL RIB_MAIN_NAME(XMADec)( HPROVIDER provider_handle, U32 up_down, RIB_ALLOC_PROVIDER_HANDLE_TYPE * rib_alloc, RIB_REGISTER_INTERFACE_TYPE * rib_reg, RIB_UNREGISTER_INTERFACE_TYPE * rib_unreg );
#endif

  #define Register_RIB(name) RIB_load_static_provider_library(RIB_MAIN_NAME(name),#name)

#else // MSS_STATIC_RIB
  #define RIB_MAIN_NAME( name ) RIB_Main
  DXDEC S32 AILCALL RIB_Main( HPROVIDER provider_handle, U32 up_down, RIB_ALLOC_PROVIDER_HANDLE_TYPE * rib_alloc, RIB_REGISTER_INTERFACE_TYPE * rib_reg, RIB_UNREGISTER_INTERFACE_TYPE * rib_unreg );
#endif // MSS_STATIC_RIB

typedef S32 ( AILCALL * RIB_MAIN_FUNC) ( HPROVIDER provider_handle, U32 up_down, RIB_ALLOC_PROVIDER_HANDLE_TYPE * rib_alloc, RIB_REGISTER_INTERFACE_TYPE * rib_reg, RIB_UNREGISTER_INTERFACE_TYPE * rib_unreg );

DXDEC HPROVIDER AILCALL RIB_load_static_provider_library (RIB_MAIN_FUNC main, const char* description);


DXDEC  HPROVIDER  AILCALL RIB_find_files_provider     (C8 const *interface_name,
                                                       C8 const *property_name_1,
                                                       C8 const *file_suffix_1,
                                                       C8 const *property_name_2,
                                                       C8 const *file_suffix_2);

DXDEC  HPROVIDER  AILCALL RIB_find_file_dec_provider  (C8 const *interface_name,
                                                       C8 const *property_name_1,
                                                       U32           decimal_property_value_1,
                                                       C8 const *property_name_2,
                                                       C8 const *file_suffix_2);

DXDEC  S32        AILCALL RIB_load_application_providers
                                                      (C8 const  *filespec);

DXDEC  void       AILCALL RIB_set_provider_user_data  (HPROVIDER provider,
                                                       U32       index,
                                                       SINTa     value);

DXDEC  SINTa      AILCALL RIB_provider_user_data      (HPROVIDER provider,
                                                       U32       index);

DXDEC  void       AILCALL RIB_set_provider_system_data
                                                      (HPROVIDER provider,
                                                       U32       index,
                                                       SINTa     value);

DXDEC  SINTa      AILCALL RIB_provider_system_data    (HPROVIDER provider,
                                                       U32       index);

DXDEC  C8 *  AILCALL  RIB_error                   (void);

#endif // RIB_H


#ifndef MSS_ASI_VERSION // MSSASI.H contents included if MSSASI.H not already included

#define AIL_ASI_VERSION  1
#define AIL_ASI_REVISION 0

//
// Handle to stream being managed by ASI codec
//

typedef SINTa HASISTREAM;

//
// ASI result codes
//

typedef S32 ASIRESULT;

#define ASI_NOERR                   0   // Success -- no error
#define ASI_NOT_ENABLED             1   // ASI not enabled
#define ASI_ALREADY_STARTED         2   // ASI already started
#define ASI_INVALID_PARAM           3   // Invalid parameters used
#define ASI_INTERNAL_ERR            4   // Internal error in ASI driver
#define ASI_OUT_OF_MEM              5   // Out of system RAM
#define ASI_ERR_NOT_IMPLEMENTED     6   // Feature not implemented
#define ASI_NOT_FOUND               7   // ASI supported device not found
#define ASI_NOT_INIT                8   // ASI not initialized
#define ASI_CLOSE_ERR               9   // ASI not closed correctly

// ----------------------------------
// Application-provided ASI callbacks
// ----------------------------------

//
// AILASIFETCHCB: Called by ASI to obtain data from stream source
//
// offset normally will be either 0 at the first call made by the codec
// or -1 to specify a continuous stream, except when ASI_stream_seek()
// is called to restart the stream codec at a new stream offset.  In this
// case, the application must execute the seek operation on the ASI codec's
// behalf.
//
// In response to this callback, the application should read the requested
// data and copy it to the specified destination buffer, returning the number
// of bytes copied (which can be less than bytes_requested if the end of
// the stream is reached).
//


typedef S32 (AILCALLBACK * AILASIFETCHCB) (UINTa     user,            // User value passed to ASI_open_stream()
                                               void *dest,            // Location to which stream data should be copied by app
                                               S32       bytes_requested, // # of bytes requested by ASI codec
                                               S32       offset);         // If not -1, application should seek to this point in stream

//############################################################################
//##                                                                        ##
//## Interface "ASI codec"                                                  ##
//##                                                                        ##
//############################################################################

//
// Initialize ASI stream codec
//
// No other ASI functions may be called outside an ASI_startup() /
// ASI_shutdown() pair, except for the standard RIB function
// PROVIDER_property() where appropriate.
//

typedef ASIRESULT (AILCALL *ASI_STARTUP)(void);

//
// Shut down ASI codec
//

typedef ASIRESULT (AILCALL * ASI_SHUTDOWN)(void);

//
// Return codec error message, or NULL if no errors have occurred since
// last call
//
// The ASI error text state is global to all streams
//

typedef C8 *  (AILCALL * ASI_ERROR)(void);

//############################################################################
//##                                                                        ##
//## Interface "ASI stream"                                                 ##
//##                                                                        ##
//############################################################################

//
// Open a stream, returning handle to stream
//

typedef HASISTREAM (AILCALL *ASI_STREAM_OPEN) (MSS_ALLOC_TYPE * palloc,
                                               MSS_FREE_TYPE  * pfree,
                                               UINTa         user,              // User value passed to fetch callback
                                               AILASIFETCHCB fetch_CB,          // Source data fetch handler
                                               U32           total_size);       // Total size for %-done calculations (0=unknown)

//
// Translate data in stream, returning # of bytes actually decoded or encoded
//
// Any number of bytes may be requested.  Requesting more data than is
// available in the codec's internal buffer will cause the AILASIFETCHCB
// handler to be called to fetch more data from the stream.
//

typedef S32  (AILCALL *ASI_STREAM_PROCESS) (HASISTREAM  stream,              // Handle of stream
                                                void   *buffer,              // Destination for processed data
                                                S32         buffer_size);        // # of bytes to return in buffer

//
// Restart stream decoding process at new offset
//
// Relevant for decoders only
//
// Seek destination is given as offset in bytes from beginning of stream
//
// At next ASI_stream_process() call, decoder will seek to the closest possible
// point in the stream which occurs at or after the specified position
//
// This function has no effect for decoders which do not support random
// seeks on a given stream type
//
// Warning: some decoders may need to implement seeking by reparsing
// the entire stream up to the specified offset, through multiple calls
// to the data-fetch callback.  This operation may be extremely
// time-consuming on large files or slow network connections.
//
// A stream_offset value of -1 may be used to inform the decoder that the
// application has changed the input stream offset on its own, e.g. for a
// double-buffering application where the ASI decoder is not accessing the
// stream directly.  ASI decoders should respond to this by flushing all
// internal buffers and resynchronizing themselves to the data stream.
//

typedef ASIRESULT (AILCALL *ASI_STREAM_SEEK)   (HASISTREAM stream,
                                                     S32        stream_offset);

//
// Retrieve or set a property value by index (returns 1 on success)
//

typedef S32 (AILCALL *ASI_STREAM_PROPERTY) (HASISTREAM stream,
                                                HPROPERTY  property,
                                                void * before_value,
                                                void const * new_value,
                                                void * after_value
                                                );

//
// Close stream, freeing handle and all internally-allocated resources
//

typedef ASIRESULT (AILCALL *ASI_STREAM_CLOSE) (HASISTREAM stream);

#endif // MSS_ASI_VERSION

//############################################################################
//##                                                                        ##
//## Interface "MSS mixer services"                                         ##
//##                                                                        ##
//############################################################################

//
// Operation flags used by mixer and filter modules
//

#define M_DEST_STEREO  1       // Set to enable stereo mixer output
#define M_SRC_16       2       // Set to enable mixing of 16-bit samples
#define M_FILTER       4       // Set to enable filtering when resampling
#define M_SRC_STEREO   8       // Set to enable mixing of stereo input samples
#define M_RESAMPLE     16      // Set to enable playback ratios other than 65536
#define M_VOL_SCALING  32      // Set to enable volume scalars other than 2048
#define M_COPY16_NOVOL 64

#ifdef IS_32

//
// Initialize mixer
//
// No other mixer functions may be called outside a MIXER_startup() /
// MIXER_shutdown() pair, except for the standard RIB function
// PROVIDER_property() as appropriate.
//

typedef void (AILCALL *MIXER_STARTUP)(void);

//
// Shut down mixer
//

typedef void (AILCALL *MIXER_SHUTDOWN)(void);

//
// Flush mixer buffer
//

typedef void (AILCALL *MIXER_FLUSH)   (S32 *dest,
                                           S32      len
#ifdef IS_X86
                                           ,U32             MMX_available
#endif
                                           );

//
// Perform audio mixing operation
//

typedef void (AILCALL *MIXER_MERGE)   (void const * *src,
                                           U32        *src_fract,
                                           void const *src_end,
                                           S32  * *dest,
                                           void       *dest_end,
                                           S32        *left_val,
                                           S32        *right_val,
                                           S32             playback_ratio,
                                           S32             scale_left,
                                           S32             scale_right,
                                           U32             operation
#ifdef IS_X86
                                           ,U32             MMX_available
#endif
                                           );

//
// Translate mixer buffer contents to final output format
//
// "option" parameter is big_endian_output on Mac, MMX on x86, overwrite flag on PS2
//

typedef void (AILCALL *MIXER_COPY) (void const  *src,
                                        S32       src_len,
                                        void *dest,
                                        U32       operation
#if defined(IS_BE) || defined(IS_X86)
                                        ,U32       option
#endif
                                        );

#else

//
// Initialize mixer
//
// No other mixer functions may be called outside a MIXER_startup() /
// MIXER_shutdown() pair, except for the standard RIB function
// PROVIDER_property() as appropriate.
//

typedef void (AILCALL *MIXER_STARTUP)(void);

//
// Shut down mixer
//

typedef void (AILCALL *MIXER_SHUTDOWN)(void);

//
// Flush mixer buffer
//

typedef void (AILCALL *MIXER_FLUSH)   (S32 *dest,
                                           S32      len,
                                           U32      MMX_available);

//
// Perform audio mixing operation
//

typedef void (AILCALL *MIXER_MERGE)   (U32             src_sel,
                                           U32             dest_sel,
                                           U32        *src_fract,
                                           U32        *src_offset,
                                           U32        *dest_offset,
                                           U32             src_end_offset,
                                           U32             dest_end_offset,
                                           S32        *left_val,
                                           S32        *right_val,
                                           S32             playback_ratio,
                                           S32             scale_both,
                                           U32             operation);

//
// Translate mixer buffer contents to final output format
//

typedef void (AILCALL *MIXER_COPY) (void const *src,
                                        S32       src_len,
                                        void *dest,
                                        U32       operation,
                                        U32       option);
#endif


typedef struct _MSS_BB              // Used in both MC and conventional mono/stereo configurations
{
   S32 *buffer;                 // Build buffer
   S32      bytes;                  // Size in bytes
   S32      chans;                  // Always mono (1) or stereo (2)

   S32      speaker_offset;         // Destination offset in interleaved PCM block for left channel
} MSS_BB;

typedef struct _ADPCMDATATAG
{
  U32   blocksize;
  U32   extrasamples;
  U32   blockleft;
  U32   step;
  UINTa savesrc;
  U32   sample;
  UINTa destend;
  UINTa srcend;
  U32   samplesL;
  U32   samplesR;
  U16   moresamples[16];
} ADPCMDATA;

typedef void (AILCALL * MIXER_MC_COPY) ( MSS_BB * build,
                                             S32 n_build_buffers,
                                             void * lpWaveAddr,
                                             S32 hw_format,
#ifdef IS_X86
                                             S32 use_MMX,
#endif
                                             S32 samples_per_buffer,
                                             S32 physical_channels_per_sample );


typedef void (AILCALL * MIXER_ADPCM_DECODE ) ( void * dest,
                                                   void const * in,
                                                   S32 out_len,
                                                   S32 in_len,
                                                   S32 input_format,
                                                   ADPCMDATA *adpcm_data);

//
// Type definitions
//

struct _DIG_DRIVER;

struct _MDI_DRIVER;

typedef struct _DIG_DRIVER * HDIGDRIVER;    // Handle to digital driver

typedef struct _MDI_DRIVER * HMDIDRIVER;    // Handle to XMIDI driver

typedef struct _SAMPLE * HSAMPLE;           // Handle to sample

typedef struct _SEQUENCE * HSEQUENCE;       // Handle to sequence

typedef S32 HTIMER;                             // Handle to timer


//
// Function pointer types
//

typedef void (AILCALLBACK* AILINCB)       (void const *data, S32 len, UINTa user_data);

typedef void (AILCALLBACK* AILTRACECB)    (C8 *text, S32 nest_depth);

typedef void (AILCALLBACK* AILTIMERCB)    (UINTa user);

typedef void (AILCALLBACK* AILSAMPLECB)   (HSAMPLE sample);

typedef void (AILCALLBACK* AILMIXERCB)    (HDIGDRIVER dig);

typedef F32  (AILCALLBACK* AILFALLOFFCB)  (HSAMPLE sample, F32 distance, F32 rolloff_factor, F32 min_dist, F32 max_dist);

typedef S32  (AILCALLBACK* AILEVENTCB)    (HMDIDRIVER hmi,HSEQUENCE seq,S32 status,S32 data_1,S32 data_2);

typedef S32  (AILCALLBACK* AILTIMBRECB)   (HMDIDRIVER hmi,S32 bank,S32 patch);

typedef S32  (AILCALLBACK* AILPREFIXCB)   (HSEQUENCE seq,S32 log,S32 data);

typedef void (AILCALLBACK* AILTRIGGERCB)  (HSEQUENCE seq,S32 log,S32 data);

typedef void (AILCALLBACK* AILBEATCB)     (HMDIDRIVER hmi,HSEQUENCE seq,S32 beat,S32 measure);

typedef void (AILCALLBACK* AILSEQUENCECB) (HSEQUENCE seq);

typedef S32 (AILCALLBACK *SS_STREAM_CB)   (HSAMPLE  S, S16 *dest_mono_sample_buffer, S32 dest_buffer_size);

//
// Handle to sample and driver being managed by pipeline filter
//

typedef SINTa HSAMPLESTATE;
typedef SINTa HDRIVERSTATE;

//
// Digital pipeline stages
//
// These are the points at which external modules may be installed into
// a given HSAMPLE or HDIGDRIVER's processing pipeline
//

typedef enum
{
   SP_ASI_DECODER = 0,          // Must be "ASI codec stream" provider
   SP_FILTER,                   // Must be "MSS pipeline filter" provider
   SP_FILTER_0 = SP_FILTER,     // Must be "MSS pipeline filter" provider
   SP_FILTER_1,                 // Must be "MSS pipeline filter" provider
   SP_FILTER_2,                 // Must be "MSS pipeline filter" provider
   SP_FILTER_3,                 // Must be "MSS pipeline filter" provider
   SP_FILTER_4,                 // Must be "MSS pipeline filter" provider
   SP_FILTER_5,                 // Must be "MSS pipeline filter" provider
   SP_FILTER_6,                 // Must be "MSS pipeline filter" provider
   SP_FILTER_7,                 // Must be "MSS pipeline filter" provider
   SP_MERGE,                    // Must be "MSS mixer" provider
   N_SAMPLE_STAGES,             // Placeholder for end of list (= # of valid sample pipeline stages)
   SP_OUTPUT = N_SAMPLE_STAGES, // Used to set/get prefs/attribs on a driver's output or matrix filter (if present)
   SAMPLE_ALL_STAGES            // Used to signify all pipeline stages, for shutdown
}
SAMPLESTAGE;

#define N_SP_FILTER_STAGES 8    // SP_FILTER_0 ... SP_FILTER_7

typedef enum
{
   DP_FLUSH = 0,      // Must be "MSS mixer" provider
   DP_DEFAULT_FILTER, // Must be "MSS pipeline filter" provider (sets the default)
   DP_DEFAULT_MERGE,  // Must be "MSS mixer" provider (sets the default)
   DP_COPY,           // Must be "MSS mixer" provider
   DP_MC_COPY,        // Must be "MSS mixer" provider
   DP_ADPCM_DECODE,   // Must be "MSS mixer" provider
   N_DIGDRV_STAGES,   // Placeholder for end of list (= # of valid stages)
   DIGDRV_ALL_STAGES  // Used to signify all pipeline stages, for shutdown
}
DIGDRVSTAGE;

typedef struct
   {
   ASI_STREAM_OPEN           ASI_stream_open;
   ASI_STREAM_PROCESS        ASI_stream_process;
   ASI_STREAM_SEEK           ASI_stream_seek;
   ASI_STREAM_CLOSE          ASI_stream_close;
   ASI_STREAM_PROPERTY       ASI_stream_property;

   HPROPERTY INPUT_BIT_RATE;
   HPROPERTY INPUT_SAMPLE_RATE;
   HPROPERTY INPUT_BITS;
   HPROPERTY INPUT_CHANNELS;
   HPROPERTY OUTPUT_BIT_RATE;
   HPROPERTY OUTPUT_SAMPLE_RATE;
   HPROPERTY OUTPUT_BITS;
   HPROPERTY OUTPUT_CHANNELS;
   HPROPERTY OUTPUT_CHANNEL_MASK;
   HPROPERTY OUTPUT_RESERVOIR;
   HPROPERTY POSITION;
   HPROPERTY PERCENT_DONE;
   HPROPERTY MIN_INPUT_BLOCK_SIZE;
   HPROPERTY RAW_RATE;
   HPROPERTY RAW_BITS;
   HPROPERTY RAW_CHANNELS;
   HPROPERTY REQUESTED_RATE;
   HPROPERTY REQUESTED_BITS;
   HPROPERTY REQUESTED_CHANS;
   HPROPERTY STREAM_SEEK_POS;
   HPROPERTY DATA_START_OFFSET;
   HPROPERTY DATA_LEN;
   HPROPERTY EXACT_SEEK;
   HPROPERTY EXACT_GETPOS;
   HPROPERTY SEEK_LOOKUP;
   HPROPERTY SET_LOOPING_SAMPLES;
   HPROPERTY CLEAR_LOOP_META;

   HASISTREAM stream;
   }
ASISTAGE;

typedef struct
   {
   struct _FLTPROVIDER *provider;
   HSAMPLESTATE             sample_state[MAX_SPEAKERS];
   }
FLTSTAGE;

typedef struct
{
   S32       active;    // Pass-through if 0, active if 1
   HPROVIDER provider;

   union
      {
      ASISTAGE ASI;
      MIXER_MERGE MSS_mixer_merge;
      FLTSTAGE FLT;
      }
   TYPE;
}
SPINFO;

typedef struct
{
   S32       active;    // Pass-through if 0, active if 1
   HPROVIDER provider;

   union
      {
      MIXER_FLUSH         MSS_mixer_flush;
      MIXER_COPY          MSS_mixer_copy;
      MIXER_MC_COPY       MSS_mixer_mc_copy;
      MIXER_ADPCM_DECODE  MSS_mixer_adpcm_decode;
      }
   TYPE;
}
DPINFO;

//
// Other data types
//

typedef enum
{
   WIN32_HWAVEOUT,       // waveOut handle for HDIGDRIVER, if any
   WIN32_HWAVEIN,        // waveIn handle for HDIGINPUT, if any
   WIN32_LPDS,           // lpDirectSound pointer for HSAMPLE
   WIN32_LPDSB,          // lpDirectSoundBuffer pointer for HSAMPLE
   WIN32_HWND,           // HWND that will be used to open DirectSound driver
   WIN32_POSITION_ERR,   // Nonzero if DirectSound play cursor stops moving (e.g., headphones removed)

   PS3_AUDIO_PORT,       // cellaudio port that Miles is using
   PS3_AUDIO_ADDRESS,    // address of cellaudio sound buffer
   PS3_AUDIO_LENGTH,     // length of cellaudio sound buffer
   PS3_AUDIO_POSITION,   // current playback position of cellaudio sound buffer

   PSP_SUBMIT_THREAD,    // Handle to thread submitting chucks of audio to the hw
   PSP_AUDIO_PORT,       // Port # Miles is using, -1 for simple audio, >= 0 for libwave

   PSP2_SUBMIT_THREAD,    // Handle to thread submitting chucks of audio to the hw
   PSP2_AUDIO_PORT,       // Port # Miles is using

   OAL_CONTEXT,           // OpenAL Context
   OAL_DEVICE,            // OpenAL Device

   XB_LPDS,              // lpDirectSound pointer for HSAMPLE
   XB_LPDSB,             // lpDirectSoundBuffer pointer for HSAMPLE

   XB360_LPXAB           // IXAudioSourceVoice pointer for HDIGDRIVER
}
MSS_PLATFORM_PROPERTY;


typedef struct _AIL_INPUT_INFO        // Input descriptor type
{
   AILINCB    callback;        // Callback function to receive incoming data
   UINTa      user_data;       // this is a user defined value
   U32        device_ID;       // DS LPGUID or wave device ID
   U32        hardware_format; // e.g., DIG_F_STEREO_16
   U32        hardware_rate;   // e.g., 22050
   S32        buffer_size;     // Maximum # of bytes to be passed to callback (-1 to use DIG_INPUT_LATENCY)
} AIL_INPUT_INFO;

typedef struct _AILTIMER                 // Timer instance
{
   AILTIMERCB callback;
   U64 next;
   U64 delta;
   UINTa    user;
   U32      status;
} AILTIMERSTR;

#ifndef IS_WIN64

  #define OFSblocksize     0     // these constants valid for 32-bit versions only!
  #define OFSextrasamples  4
  #define OFSblockleft     8
  #define OFSstep         12
  #define OFSsavesrc      16
  #define OFSsample       20
  #define OFSdestend      24
  #define OFSsrcend       28
  #define OFSsamplesL     32
  #define OFSsamplesR     36
  #define OFSmoresamples  40

#endif

typedef struct LOWPASS_INFO
{
  S32 X0, X1;
  S32 Y0, Y1;
  S32 A,  B0,  B1;
  S32 flags;
  S32 queuedA, queuedB;
  F32 calculated_cut;
  F32 cutoff;
} LOWPASS_INFO;


typedef union STAGE_BUFFER
{
  union STAGE_BUFFER * next;
  U8 data[ 1 ];
} STAGE_BUFFER;

typedef struct _MSSVECTOR3D
{
  F32 x;
  F32 y;
  F32 z;
} MSSVECTOR3D;

#define MILES_TANGENT_LINEAR 0
#define MILES_TANGENT_CURVE 1
#define MILES_TANGENT_STEP 2
#define MILES_MAX_FALLOFF_GRAPH_POINTS 5

#define MILES_MAX_SEGMENT_COUNT 10

typedef struct _MSSGRAPHPOINT
{
    F32 X, Y, ITX, ITY, OTX, OTY; // Point & tangents.
    S32 IType, OType;
} MSSGRAPHPOINT;

typedef struct _S3DSTATE           // Portion of HSAMPLE that deals with 3D positioning
{
   MSSVECTOR3D   position;         // 3D position
   MSSVECTOR3D   face;             // 3D orientation
   MSSVECTOR3D   up;               // 3D up-vector
   MSSVECTOR3D   velocity;         // 3D velocity

   S32           doppler_valid;    // TRUE if OK to apply Doppler shift
   F32           doppler_shift;    // Scalar for S->playback rate

   F32           inner_angle;      // Cone attenuation parameters
   F32           outer_angle;      // (Angles divided by two and convered to rads for dot-product comparisons)
   F32           outer_volume;
   S32           cone_enabled;

   F32           max_dist;         // Sample distances
   F32           min_dist;
   S32           dist_changed;     // TRUE if min/max distances have changed and need to be sent to the hardware

   S32           auto_3D_atten;    // TRUE if distance/cone attenuation should be applied to wet signal
   F32           atten_3D;         // Attenuation due to distance/cone effects, calculated by software 3D positioner
   F32           rolloff;          // per sample rolloff factor to use instead of global rolloff, if non zero.

   F32           exclusion_3D;     // exclusion value computed by falloff graph. -1 if not affected.
   F32           lowpass_3D;       // low pass cutoff computed by falloff graph. -1 if not affected.

   F32           spread;

   HSAMPLE       owner;            // May be NULL if used for temporary/internal calculations
   AILFALLOFFCB  falloff_function; // User function for min/max distance calculations, if desired

   MSSVECTOR3D   position_graph[MILES_MAX_SEGMENT_COUNT];
   S32           position_graph_count;

   MSSGRAPHPOINT volgraph[MILES_MAX_FALLOFF_GRAPH_POINTS];
   MSSGRAPHPOINT excgraph[MILES_MAX_FALLOFF_GRAPH_POINTS];
   MSSGRAPHPOINT lpgraph[MILES_MAX_FALLOFF_GRAPH_POINTS];
   MSSGRAPHPOINT spreadgraph[MILES_MAX_FALLOFF_GRAPH_POINTS];

   U8 volgraphcnt;
   U8 excgraphcnt;
   U8 lpgraphcnt;
   U8 spreadgraphcnt;

} S3DSTATE;

typedef struct _SMPBUF
{
   void const *start;          // Sample buffer address (W)
   U32             len;            // Sample buffer size in bytes (W)
   U32             pos;            // Index to next byte (R/W)
   U32             done;           // Nonzero if buffer with len=0 sent by app
   S32             reset_ASI;      // Reset the ASI decoder at the end of the buffer
   S32             reset_seek_pos; // New destination offset in stream source data, for ASI codecs that care
} SMPBUF;

typedef struct _SAMPLE             // Sample instance
{
   U32        tag;                 // HSAM

   HDIGDRIVER driver;              // Driver for playback

   S32        index;               // Numeric index of this sample

   SMPBUF     buf[8];              // Source data buffers

   U32        src_fract;           // Fractional part of source address

   U32        mix_delay;           // ms until start mixing (decreased every buffer mix)
   F32        max_output_mix_volume; // max_volume of any speaker at last mix

   U64        mix_bytes;           // total number of bytes sent to the mixer for this sample.

   S32        group_id;            // ID for grouped operations.

   // size of the next dynamic arrays
   U32      chan_buf_alloced;
   U32      chan_buf_used;
   U8*      chan_buf_ptr;

   // these are dynamic arrays sized as n_channels long (so 1 for mono, 2 stereo, 6 for 5.1)
   S32     *left_val;
   S32     *right_val;
   S32     *last_decomp;
   LOWPASS_INFO *lp;             // low pass info


   // these are dynamic arrays pointing to dynamic arrays, each of the sub arrays are n_channels long or [MAX_SPEAKERS][n_channels]
   F32    **user_channel_levels; // Channel levels set by AIL_set_sample_channel_levels() [source_channels][driver->logical_channels]
   S32    **cur_scale;           // Calculated 11-bit volume scale factors for current/previous mixing interval
   S32    **prev_scale;          // (These are all indexed by build buffer*2, not speaker indexes!)
   S32    **ramps_left;

   // these are dynamic arrays
   F32     *auto_3D_channel_levels;         // Channel levels set by 3D positioner (always 1.0 if not 3D-positioned)
   F32     *speaker_levels;                 // one level per speaker (multiplied after user or 3D)

   S8      *speaker_enum_to_source_chan;    // array[MSS_SPEAKER_xx] = -1 if not present, else channel #
                                            // 99% of the time this is a 1:1 mapping and is zero.

   S32      lp_any_on;                      // are any of the low pass filters on?
   S32      user_channels_need_deinterlace; // do any of the user channels require a stereo sample to be deinterlaced?

   S32      n_buffers;           // # of buffers (default = 2)
   S32      head;
   S32      tail;
   S32      starved;             // Buffer stream has run out of data
   S32      exhaust_ASI;         // Are we prolonging the buffer lifetime until ASI output is exhausted?

   S32      loop_count;          // # of cycles-1 (1=one-shot, 0=indefinite)
   S32      loop_start;          // Starting offset of loop block (0=SOF)
   S32      loop_end;            // End offset of loop block (-1=EOF)
   S32      orig_loop_count;     // Original loop properties specified by app, before any
   S32      orig_loop_start;     // alignment constraints
   S32      orig_loop_end;

   S32      format;              // DIG_F format (8/16 bits, mono/stereo/multichannel)
   S32      n_channels;          // # of channels (which can be >2 for multichannel formats)
   U32      channel_mask;        // Same as WAVEFORMATEXTENSIBLE.dwChannelMask

   S32      original_playback_rate; // Playback rate in hertz
   F32      playback_rate_factor;   // Fractional playback rate, normally 1.0

   F32      save_volume;         // Sample volume 0-1.0
   F32      save_pan;            // Mono panpot/stereo balance (0=L ... 1.0=R)

   F32      left_volume;         // Left/mono volume 0 to 1.0
   F32      right_volume;        // Right volume 0 to 1.0
   F32      wet_level;           // reverb level 0 to 1.0
   F32      dry_level;           // non-reverb level 0 to 1.0
   F32      sys_level;           // system control

   F32      extra_volume;        // Volume scalar for ramping or otherwise.
   F32      extra_wet;
   F32      extra_lp;
   F32      extra_rate;


   U32      low_pass_changed;    // bit mask for what channels changed.

   S32      bus;                 // Bus assignment for this sample.
   S32      bus_comp_sends;      // Which buses this bus routes compressor input to.
   S32      bus_comp_installed;  // Nonzero if we have a compressor installed.
   U32      bus_comp_input;      // The input to use for this bus's compressor, if we have one installed
   S32      bus_override_wet;    // If true, samples on this bus will use the bus's wet level instead of their own.
   U32      bus_signal_strength; // The bus level.
   S32      bus_enable_limiter;  // If true, a basic limiter will be run on the samples prior to clamping to S16.
   S32      bus_limiter_atten;   // The attenuation that was applied on the last bus pass.

   S32      fade_to_stop;        // # of samples to fade to stop over. ( currently fixed at the volramp count )

   U64      mix_start_time;      // arbitrary non-zero id for starting sounds synced.

   S16      pop_fade_total;
   S16      pop_fade_time;
   U8       pop_fade_stop;       // nonzero we end the sample when it fades out.

   U8       state_level;         // Level the sample was started at.
#ifdef IS_WIIU
   S8      route_to_drc;
#endif

   F32      obstruction;
   F32      occlusion;
   F32      exclusion;

   S32      service_type;        // 1 if single-buffered; 2 if streamed

   AILSAMPLECB  SOB;             // Start-of-block callback function
   AILSAMPLECB  EOB;             // End-of-buffer callback function
   AILSAMPLECB  EOS;             // End-of-sample callback function

   SINTa    user_data  [8];      // Miscellaneous user data
   SINTa    system_data[8];      // Miscellaneous system data
   SINTa    hl_marker_list;

   ADPCMDATA adpcm;

   S32      doeob;               // Flags to trigger callbacks
   S32      dosob;
   S32      doeos;

   S32      vol_ramps;
   S32      resamp_tolerance;
   S32      enable_resamp_filter;

   //
   // Sample pipeline stages
   //

   SPINFO   pipeline[N_SAMPLE_STAGES];
   S32      n_active_filters;    // # of SP_FILTER_n stages active

   //
   // 3D-related state for all platforms (including Xbox)
   //

   S32      is_3D;               // TRUE if channel levels are derived automatically from 3D positional state, FALSE if they're controlled manually

   S3DSTATE S3D;                 // Software version applies 3D positioning only if is_3D == TRUE, but output filters always use it

#ifdef MSS_VFLT_SUPPORTED
   void *voice;              // Optional object used by output filter to store per-sample information such as DS3D buffers
#endif

   F32 leftb_volume;         // Left/mono volume 0 to 1.0 (back)
   F32 rightb_volume;        // Right volume 0 to 1.0 (back)
   F32 center_volume;        // Center volume 0 to 1.0
   F32 low_volume;           // Low volume 0 to 1.0
   F32 save_fb_pan;          // Sample volume 0-1.0
   F32 save_center;          // saved center level
   F32 save_low;             // saved sub level

#if defined(HOST_SPU_PROCESS) || defined(MSS_SPU_PROCESS)
   S32    spu_on;
   U32    align[1];
#endif

#if defined(IS_WINDOWS)

   //
   // DirectSound-specific data
   //

   S32      service_interval;    // Service sample every n ms
   S32      service_tick;        // Current service countdown value
   S32      buffer_segment_size; // Buffer segment size to fill

   S32      prev_segment;        // Previous segment # (0...n)
   S32      prev_cursor;         // Previous play cursor location

   S32      bytes_remaining;     // # of bytes left to play (if not -1)

   S32      direct_control;      // 1 if app controls buffer, 0 if MSS

#endif
} SAMPLE;

#ifdef MILES_CHECK_OFFSETS
    RR_COMPILER_ASSERT((RR_MEMBER_OFFSET(SAMPLE, save_low) & 3) == 0);
#endif

//
// used for AIL_process
//

typedef struct _AILMIXINFO {
  AILSOUNDINFO Info;
  ADPCMDATA mss_adpcm;
  U32 src_fract;
  S32 left_val;
  S32 right_val;
} AILMIXINFO;



DXDEC  U32     AILCALL  AIL_get_timer_highest_delay   (void);

DXDEC  void    AILCALL AIL_serve(void);

#ifdef IS_MAC

  typedef void * LPSTR;

  #define WHDR_DONE 0

  typedef struct _WAVEIN
  {
    long temp;
  } * HWAVEIN;

  typedef struct _WAVEHDR
  {
    S32  dwFlags;
    S32  dwBytesRecorded;
    S32  dwUser;
    S32  temp;
    void * lpData;
    S32  dwBufferLength;
    S32  longdwLoops;
    S32  dwLoops;
    void * lpNext;
    U32  * reserved;

  } WAVEHDR, * LPWAVEHDR;

#endif

#define N_WAVEIN_BUFFERS 8     // Use a ring of 8 buffers by default

typedef struct _DIG_INPUT_DRIVER *HDIGINPUT; // Handle to digital input driver

#ifdef IS_MAC

  #define AIL_DIGITAL_INPUT_DEFAULT 0

  typedef struct _DIG_INPUT_DRIVER    // Handle to digital input driver
  {
     U32  tag;                      // HDIN
     S32 input_enabled;               // 1 if enabled, 0 if not
     U32 incoming_buffer_size;
     void * incoming_buffer[ 2 ];
     void* outgoing_buffer;
     U32 which_buffer;
     AIL_INPUT_INFO info;             // Input device descriptor
     AILMIXINFO incoming_info;
     long device;
#ifdef IS_MAC
     char InternalRecordingState[128]; // Hide this so we dont' have to #include OS stuff everywhere.
#endif
  } DIG_INPUT_DRIVER;

#else

#define AIL_DIGITAL_INPUT_DEFAULT ((U32)WAVE_MAPPER)

typedef struct _DIG_INPUT_DRIVER    // Handle to digital input driver
{
   U32     tag;                   // HDIN

   HTIMER background_timer;         // Background timer handle

   AIL_INPUT_INFO info;             // Input device descriptor

   S32       input_enabled;         // 1 if enabled, 0 if not

   UINTa     callback_user;         // Callback user value

   //
   // Provider-independent data
   //

   U32       DMA_size;              // Size of each DMA sub-buffer in bytes
   void *DMA[N_WAVEIN_BUFFERS]; // Simulated DMA buffers

   U32       silence;               // Silence value for current format (0 or 128)

   S32       device_active;         // 1 if buffers submittable, 0 if not

#if defined(IS_WINDOWS) && !defined(__RADWINRTAPI__)
   //
   // waveOut-specific data
   //

   HWAVEIN          hWaveIn;                        // Handle to wave input device
   volatile MWAVEHDR wavehdr[N_WAVEIN_BUFFERS];     // Handles to wave headers

#endif
} DIG_INPUT_DRIVER;
#endif


typedef struct REVERB_CONSTANT_INFO
{
  F32* start0,* start1,* start2,* start3,* start4,* start5;
  F32* end0,* end1,* end2,* end3,* end4,* end5;
  F32 C0, C1, C2, C3, C4, C5;
  F32 A;
  F32 B0, B1;
} REVERB_CONSTANT_INFO;

typedef struct REVERB_UPDATED_INFO
{
  F32 * address0, * address1, * address2, * address3, * address4, * address5;
  F32 X0, X1, Y0, Y1;
} REVERB_UPDATED_INFO;

typedef struct REVERB_INFO
{
  REVERB_UPDATED_INFO u;
  REVERB_CONSTANT_INFO c;
} REVERB_INFO;

typedef struct REVERB_SETTINGS
{
    S32      room_type;           // Changes to this drive master_wet and duration/damping/predelay!
    F32      master_wet;          // Master reverb level 0-1.0
    F32      master_dry;          // Master non-reverb level 0-1.0

    REVERB_INFO ri;

    S32      *reverb_build_buffer;
    S32      reverb_total_size;
    S32      reverb_fragment_size;
    S32      reverb_buffer_size;
    S32      reverb_on;
    U32      reverb_off_time_ms;

    U32      reverb_duration_ms;

    F32      reverb_decay_time_s;
    F32      reverb_predelay_s;
    F32      reverb_damping;

    S32      reverb_head;
    S32      reverb_tail;
} REVERB_SETTINGS;


typedef struct _MSS_RECEIVER_LIST
{
   MSSVECTOR3D direction;                      // Normalized direction vector from listener

   S32         speaker_index[MAX_SPEAKERS];    // List of speakers affected by sounds in this direction
   F32         speaker_level[MAX_SPEAKERS];    // Each speaker's degree of effect from this source
   S32         n_speakers_affected;
} MSS_RECEIVER_LIST;

typedef struct _D3DSTATE
{
   S32         mute_at_max;

   MSSVECTOR3D listen_position;
   MSSVECTOR3D listen_face;
   MSSVECTOR3D listen_up;
   MSSVECTOR3D listen_cross;
   MSSVECTOR3D listen_velocity;

   F32         rolloff_factor;
   F32         doppler_factor;
   F32         distance_factor;
   F32         falloff_power;

   //
   // Precalculated listener info
   //

   S32         ambient_channels    [MAX_SPEAKERS];      // E.g., LFE
   S32         n_ambient_channels;

   S32         directional_channels[MAX_SPEAKERS+1];    // Channel index, or -1 if virtual
   MSSVECTOR3D listener_to_speaker [MAX_SPEAKERS+1];
   S32         n_directional_channels;

   MSS_RECEIVER_LIST receiver_specifications[MAX_RECEIVER_SPECS];  // Constellation of receiver vectors
   S32               n_receiver_specs;

   MSSVECTOR3D speaker_positions           [MAX_SPEAKERS]; // Listener-relative speaker locations

   F32         speaker_wet_reverb_response [MAX_SPEAKERS]; // Reverb sensitivity of each speaker
   F32         speaker_dry_reverb_response [MAX_SPEAKERS];
} D3DSTATE;

typedef enum
{
   MSS_MC_INVALID             = 0,       // Used for configuration-function errors
   MSS_MC_MONO                = 1,       // For compatibility with S32 channel param
   MSS_MC_STEREO              = 2,
   MSS_MC_USE_SYSTEM_CONFIG   = 0x10,    // Leave space between entries for new variations
   MSS_MC_HEADPHONES          = 0x20,    // with similar quality levels/speaker counts
   MSS_MC_DOLBY_SURROUND      = 0x30,
   MSS_MC_SRS_CIRCLE_SURROUND = 0x40,
   MSS_MC_40_DTS              = 0x48,
   MSS_MC_40_DISCRETE         = 0x50,
   MSS_MC_51_DTS              = 0x58,
   MSS_MC_51_DISCRETE         = 0x60,
   MSS_MC_61_DISCRETE         = 0x70,
   MSS_MC_71_DISCRETE         = 0x80,
   MSS_MC_81_DISCRETE         = 0x90,
   MSS_MC_DIRECTSOUND3D       = 0xA0,
   MSS_MC_EAX2                = 0xC0,
   MSS_MC_EAX3                = 0xD0,
   MSS_MC_EAX4                = 0xE0,
   MSS_MC_FORCE_32            = 0x7fffffff
}
MSS_MC_SPEC;


typedef struct _DIG_DRIVER          // Handle to digital audio driver
{
   U32         tag;                 // HDIG

   HTIMER      backgroundtimer;     // Background timer handle

   U32         num_mixes;           // incrementing number of mixes

   S32         mix_ms;              // rough ms per mix


   F32         master_volume;       // Master sample volume 0-1.0

   S32         DMA_rate;            // Hardware sample rate
   S32         hw_format;           // DIG_F code in use
   S32         n_active_samples;    // # of samples being processed

   MSS_MC_SPEC channel_spec;        // Original "channels" value passed to AIL_open_digital_driver()

   D3DSTATE    D3D;                 // 3D listener parms for all platforms


#if defined(IS_PSP2) || defined(IS_PSP) || defined(IS_XENON) || defined(IS_IPHONE) || defined(IS_MAC) || defined(IS_LINUX) || defined(IS_3DS) || defined(IS_WIIU) || defined(__RADWINRTAPI__) || defined(__RADSEKRIT2__) || defined(__RADANDROID__) // generic dig platforms
#define IS_GENERICDIG
   void*       dig_ss;              // Sound system ptr (embed in mss.h?)
   void*       dig_heap;            // Sound system heap.
#endif

#ifdef IS_XENON
   void* x2_voiceptr;           //! \todo get rid of this? Only expose dig_ss?
#endif

   S32         quiet;               // # of consecutive quiet sample periods
   S32         playing;             // Playback active if non-zero

   S32         bytes_per_channel;            // # of bytes per channel (always 1 or 2 for 8- or 16-bit hardware output)
   S32         samples_per_buffer;           // # of samples per build buffer / half-buffer
   S32         physical_channels_per_sample; // # of channels per *physical* sample (1 or 2, or more in discrete MC mode)
   S32         logical_channels_per_sample;  // # of logical channels per sample (may differ from physical channel count in matrix formats)

#ifdef IS_LINUX
   S32         released;            // has the sound manager been released?
#endif

   HSAMPLE     samples;             // Pointer to list of SAMPLEs

   U32        *sample_status;       // SMP_ flags: _FREE, _DONE, _PLAYING, moved out of SAMPLEs for faster iteration
   S32         n_samples;           // # of SAMPLEs

   SINTa       system_data[8];      // Miscellaneous system data

   HSAMPLE     bus_samples[MAX_BUSSES];      // Sample handles the bus will route through.
   S32         bus_active_count[MAX_BUSSES]; // Number of samples mixed on the bus last mix.
   void*       bus_ptrs[MAX_BUSSES];         // Buffers for each bus to mix in to.

   void*       pushed_states[MILES_MAX_STATES];
   U8          state_index;

   //
   // Build buffers
   //
   // In multichannel mode, source samples may be mixed into more than one
   // build buffer
   //

   MSS_BB      build[MAX_SPEAKERS+EXTRA_BUILD_BUFFERS];
   S32         n_build_buffers;      // # of build buffers actually used for output processing

   S32         hardware_buffer_size; // Size of each output buffer

   S32         enable_limiter;
   S32         limiter_atten;       // attenuation level from last hw copy.

   S32         scheduled_sample_count; // # of samples that are waiting to be started at an exact time.

   AILMIXERCB  mixcb;               // callback for each mix.

#if defined(IS_WINDOWS) && !defined(__RADWINRTAPI__)

   //
   // waveOut-specific interface data
   //

   HWAVEOUT    hWaveOut;            // Wave output driver

   U32         reset_works;         // TRUE if OK to do waveOutReset
   U32         request_reset;       // If nonzero, do waveOutReset ASAP

   LPWAVEHDR   first;               // Pointer to first WAVEHDR in chain
   S32         n_buffers;           // # of output WAVEHDRs in chain

   LPWAVEHDR volatile *return_list; // Circular list of returned WAVEHDRs
   S32       volatile      return_head; // Head of WAVEHDR list (insertion point)
   S32       volatile      return_tail; // Tail of WAVEHDR list (retrieval point)

   //
   // DirectSound-specific interface data
   //

   UINTa                  guid;        // The guid id of the ds driver
   AILLPDIRECTSOUND       pDS;         // DirectSound output driver (don't
                                       // use with Smacker directly anymore!)

   U32                    ds_priority; // priority opened with

   S32                    emulated_ds; // is ds emulated or not?
   AILLPDIRECTSOUNDBUFFER lppdsb;      // primary buffer or null

   UINTa                  dsHwnd;      // HWND used with DirectSound

   AILLPDIRECTSOUNDBUFFER * lpbufflist;   // List of pointers to secondary buffers
   HSAMPLE         *samp_list;      // HSAMPLE associated with each buffer
   S32             *sec_format;     // DIG_F_ format for secondary buffer
   S32                  max_buffs;      // Max. allowable # of secondary buffers

   //
   // Driver output configuration
   //
   // Note: # of "logical" (source) channels per sample = dig->channels_per_sample
   //       # of "physical" (DAC) channels per sample = dig->wformat.wf.nChannels
   //
   //       These may be different if a matrix format (e.g., Dolby/SRS)
   //       is in use!
   //

   MPCMWAVEFORMAT wformat;          // format from waveout open
   C8             wfextra[32];      // Extension to PCMWAVEFORMAT (e.g., WAVE_FORMAT_EXTENSIBLE)

   //
   // Misc. data
   //

   S32         released;            // has the sound manager been released?

   HDIGDRIVER  next;                // Pointer to next HDIGDRIVER in use

   //
   // Vars for waveOut emulation
   //

   S32 DS_initialized;

   AILLPDIRECTSOUNDBUFFER DS_sec_buff;    // Secondary buffer (or NULL if none)
   AILLPDIRECTSOUNDBUFFER DS_out_buff;    // Output buffer (may be sec or prim)
   S32 DS_buffer_size;                    // Size of entire output buffer

   S32 DS_frag_cnt;                 // Total fragment count and size, and
   S32 DS_frag_size;                // last fragment occupied by play cursor
   S32 DS_last_frag;
   S32 DS_last_write;
   S32 DS_last_timer;
   S32 DS_skip_time;

   S32 DS_use_default_format;       // 1 to force use of default DS primary buffer format

   U32 position_error;              // last status from position report (can be used
                                    //   to watch for headset removal)
   U32 last_ds_play;
   U32 last_ds_write;
   U32 last_ds_move;

#endif

#ifdef IS_X86
   S32         use_MMX;             // Use MMX with this driver if TRUE
#endif

   U64 mix_total;
   U64 last_polled;
   U32 last_percent;

   void * MC_buffer;
   //
   // Digital driver pipeline filter stages
   //

   DPINFO pipeline[N_DIGDRV_STAGES];

#ifdef MSS_VFLT_SUPPORTED
   struct _FLTPROVIDER *voice_filter;
   SS_STREAM_CB             stream_callback;
#endif

   struct _FLTPROVIDER *matrix_filter;

   //
   // Reverb
   // If no busses are active, 0 is still used as the base reverb.
   //
   REVERB_SETTINGS reverb[MAX_BUSSES];

#ifdef IS_PS3
   HDIGDRIVER  next;                // Pointer to next HDIGDRIVER in use

   void * hw_buf;
   U32    hw_datarate;
   U32    hw_align;
   U32    port;
   S32    hw_buffer_size;
   S32    snd_frag_cnt;
   S32    snd_frag_size;
   S32    snd_last_frag;
   S32    snd_last_write;
   S32    snd_skip_time;
   U32    snd_last_play;
   U32    snd_last_move;
   S32    snd_last_timer;
#endif

#ifdef IS_GENERICDIG
   HDIGDRIVER next;
#endif

#if defined(IS_WINDOWS)
   S32                  no_wom_done;    // don't process WOM_DONEs on this driver
   U32                  wom_done_buffers;
#endif

#if defined(HOST_SPU_PROCESS) || defined(MSS_SPU_PROCESS)
   U32    spu_num;
   S32    spu_on;
   U64    spu_total;
   U64    spu_last_polled;
   U32    spu_last_percent;
   #ifdef IS_PS3
     U32    align[ 2 ];
   #else
     U32    align[ 1 ];
   #endif
#endif

   U64 adpcm_time;
   U64 deinterlace_time;
   U64 mix_time;
   U64 rev_time;
   U64 reformat_time;
   U64 lowpass_time;
   U64 filter_time;
   U64 copy_time;
   U64 sob_time;
   U64 eob_time;
   U64 eos_time;
   U64 spu_wait_time;
   U64 asi_times[4];

   HSAMPLE adpcm_sam;
   HSAMPLE deinterlace_sam;
   HSAMPLE mix_sam;
   HSAMPLE rev_sam;
   HSAMPLE reformat_sam;
   HSAMPLE lowpass_sam;
   HSAMPLE filter_sam;
   HSAMPLE asi_sams[4];

   U32 adpcm_num;
   U32 deinterlace_num;
   U32 mix_num;
   U32 rev_num;
   U32 reformat_num;
   U32 lowpass_num;
   U32 filter_num;
   U32 asi_nums[4];


   // these clauses have to be at the end of the structure!!
#ifdef IS_WII
   HDIGDRIVER  next;                // Pointer to next HDIGDRIVER in use

   U32    hw_datarate;
   S32    hw_buffer_size;
   S32    each_buffer_size;
   S32    snd_frag_cnt;
   S32    snd_frag_size;
   S32    snd_last_frag;
   S32    snd_last_write;
   S32    snd_skip_time;
   U32    snd_last_play;
   U32    snd_last_move;
   S32    snd_last_timer;

   void * buffer[ 2 ];
   U32    physical[ 2 ];

   #ifdef AX_OUTPUT_BUFFER_DOUBLE
   AXVPB* voice[ 2 ];
   #endif

#endif

#ifdef XAUDIOFRAMESIZE_NATIVE
   XAUDIOPACKET packet;
#endif

} DIG_DRIVER;

#ifdef MILES_CHECK_OFFSETS
    RR_COMPILER_ASSERT((RR_MEMBER_OFFSET(DIG_DRIVER, filter_num) & 3) == 0);
#endif

typedef struct                      // MIDI status log structure
   {
   S32      program   [NUM_CHANS];  // Program Change
   S32      pitch_l   [NUM_CHANS];  // Pitch Bend LSB
   S32      pitch_h   [NUM_CHANS];  // Pitch Bend MSB

   S32      c_lock    [NUM_CHANS];  // Channel Lock
   S32      c_prot    [NUM_CHANS];  // Channel Lock Protection
   S32      c_mute    [NUM_CHANS];  // Channel Mute
   S32      c_v_prot  [NUM_CHANS];  // Voice Protection
   S32      bank      [NUM_CHANS];  // Patch Bank Select
   S32      gm_bank_l [NUM_CHANS];  // GM Bank Select
   S32      gm_bank_m [NUM_CHANS];  // GM Bank Select
   S32      indirect  [NUM_CHANS];  // ICA indirect controller value
   S32      callback  [NUM_CHANS];  // Callback Trigger

   S32      mod       [NUM_CHANS];  // Modulation
   S32      vol       [NUM_CHANS];  // Volume
   S32      pan       [NUM_CHANS];  // Panpot
   S32      exp       [NUM_CHANS];  // Expression
   S32      sus       [NUM_CHANS];  // Sustain
   S32      reverb    [NUM_CHANS];  // Reverb
   S32      chorus    [NUM_CHANS];  // Chorus

   S32      bend_range[NUM_CHANS];  // Bender Range (data MSB, RPN 0 assumed)

   S32      RPN_L     [NUM_CHANS];  // RPN # LSB
   S32      RPN_M     [NUM_CHANS];  // RPN # MSB
   }
CTRL_LOG;

typedef struct _SEQUENCE                  // XMIDI sequence state table
{
   char     tag[4];                       // HSEQ

   HMDIDRIVER driver;                     // Driver for playback

   U32      status;                       // SEQ_ flagsstruct

   void const   *TIMB;                    // XMIDI IFF chunk pointers
   void const   *RBRN;
   void const   *EVNT;

   U8 const     *EVNT_ptr;                // Current event pointer

   U8      *ICA;                          // Indirect Controller Array

   AILPREFIXCB   prefix_callback;         // XMIDI Callback Prefix handler
   AILTRIGGERCB  trigger_callback;        // XMIDI Callback Trigger handler
   AILBEATCB     beat_callback;           // XMIDI beat/bar change handler
   AILSEQUENCECB EOS;                     // End-of-sequence callback function

   S32      loop_count;                   // 0=one-shot, -1=indefinite, ...

   S32      interval_count;               // # of intervals until next event
   S32      interval_num;                 // # of intervals since start

   S32      volume;                       // Sequence volume 0-127
   S32      volume_target;                // Target sequence volume 0-127
   S32      volume_accum;                 // Accumulated volume period
   S32      volume_period;                // Period for volume stepping

   S32      tempo_percent;                // Relative tempo percentage 0-100
   S32      tempo_target;                 // Target tempo 0-100
   S32      tempo_accum;                  // Accumulated tempo period
   S32      tempo_period;                 // Period for tempo stepping
   S32      tempo_error;                  // Error counter for tempo DDA

   S32      beat_count;                   // Sequence playback position
   S32      measure_count;

   S32      time_numerator;               // Sequence timing data
   S32      time_fraction;
   S32      beat_fraction;
   S32      time_per_beat;

   U8 const *FOR_ptrs[FOR_NEST];          // Loop stack
   S32      FOR_loop_count [FOR_NEST];

   S32      chan_map       [NUM_CHANS];   // Physical channel map for sequence

   CTRL_LOG shadow;                       // Controller values for sequence

   S32      note_count;                   // # of notes "on"

   S32      note_chan      [MAX_NOTES];   // Channel for queued note (-1=free)
   S32      note_num       [MAX_NOTES];   // Note # for queued note
   S32      note_time      [MAX_NOTES];   // Remaining duration in intervals

   SINTa    user_data  [8];               // Miscellaneous user data
   SINTa    system_data[8];               // Miscellaneous system data

} SEQUENCE;

#if defined(IS_MAC) || defined(IS_LINUX) || defined(IS_XENON) || defined(IS_PS3) || defined(IS_WII) || defined(IS_PSP) || defined(IS_PSP2) || defined(__RADWINRTAPI__) || defined(__RADSEKRIT2__) || defined(__RADANDROID__)

struct MIDIHDR;
struct MIDIOUT;
typedef struct MIDIOUT* HMIDIOUT;
typedef HMIDIOUT* LPHMIDIOUT;

#endif

typedef struct _MDI_DRIVER          // Handle to XMIDI driver
{
   char     tag[4];                 // HMDI

   HTIMER      timer;               // XMIDI quantization timer
   S32         interval_time;       // XMIDI quantization timer interval in uS

   HSEQUENCE   sequences;           // Pointer to list of SEQUENCEs
   S32         n_sequences;         // # of SEQUENCEs

   S32         lock  [NUM_CHANS];   // 1 if locked, 2 if protected, else 0
   HSEQUENCE   locker[NUM_CHANS];   // HSEQUENCE which locked channel
   HSEQUENCE   owner [NUM_CHANS];   // HSEQUENCE which owned locked channel
   HSEQUENCE   user  [NUM_CHANS];   // Last sequence to use channel
   S32         state [NUM_CHANS];   // Lock state prior to being locked

   S32         notes [NUM_CHANS];   // # of active notes in channel

   AILEVENTCB  event_trap;          // MIDI event trap callback function
   AILTIMBRECB timbre_trap;         // Timbre request callback function

   S32         master_volume;       // Master XMIDI note volume 0-127

   SINTa       system_data[8];      // Miscellaneous system data

#if (defined(IS_WINDOWS) && !defined(__RADWINRTAPI__)) || defined(IS_MAC) || defined(IS_LINUX)

   S32         released;            // has the hmidiout handle been released
   U32         deviceid;            // ID of the MIDI device
   U8      *sysdata;            // SysEx buffer

#endif

#if defined(IS_XENON) || defined(IS_WII) || defined(IS_PS3) || defined(IS_PSP) || defined(IS_3DS) || defined(IS_IPHONE) || defined(IS_PSP2) || defined(IS_WIIU) || defined(__RADWINRTAPI__) || defined(__RADSEKRIT2__) || defined(__RADANDROID__)
   HMDIDRIVER  next;                // Pointer to next HMDIDRIVER in use
#endif

#ifdef IS_LINUX
   struct MIDIHDR *mhdr;        // SysEx header
   HMDIDRIVER  next;                // Pointer to next HMDIDRIVER in use
   HMIDIOUT    hMidiOut;            // MIDI output driver
#endif

#if defined(IS_WINDOWS) && !defined(__RADWINRTAPI__)

   struct midihdr_tag *mhdr;               // SysEx header

   HMDIDRIVER  next;                // Pointer to next HMDIDRIVER in use

   HMIDIOUT    hMidiOut;            // MIDI output driver

#else

   #if defined(IS_MAC)
     struct MIDIHDR *mhdr;           // SysEx header
     HMDIDRIVER  next;                // Pointer to next HMDIDRIVER in use
     HMIDIOUT    hMidiOut;            // MIDI output driver
   #endif

#endif

} MDI_DRIVER;

#if defined(IS_WIN32API) || defined(IS_WII)
  #pragma pack(push, 1)
#endif

typedef MSS_STRUCT                      // XMIDI TIMB IFF chunk
   {
   S8    name[4];

   U8    msb;
   U8    lsb;
   U8    lsb2;
   U8    lsb3;

   U16   n_entries;

   U16   timbre[1];
   }
TIMB_chunk;

typedef MSS_STRUCT                      // XMIDI RBRN IFF entry
   {
   S16   bnum;
   U32   offset;
   }
RBRN_entry;

#if defined(IS_WIN32API) || defined(IS_WII)
  #pragma pack(pop)
#endif

typedef struct                      // Wave library entry
{
   S32   bank;                      // XMIDI bank, MIDI patch for sample
   S32   patch;

   S32   root_key;                  // Root MIDI note # for sample (or -1)

   U32   file_offset;               // Offset of wave data from start-of-file
   U32   size;                      // Size of wave sample in bytes

   S32   format;                    // DIG_F format (8/16 bits, mono/stereo)
   S32   playback_rate;             // Playback rate in hertz
}
WAVE_ENTRY;

typedef struct                      // Virtual "wave synthesizer" descriptor
{
   HMDIDRIVER mdi;                  // MIDI driver for use with synthesizer
   HDIGDRIVER dig;                  // Digital driver for use with synthesizer

   WAVE_ENTRY *library;         // Pointer to wave library

   AILEVENTCB  prev_event_fn;       // Previous MIDI event trap function
   AILTIMBRECB prev_timb_fn;        // Previous timbre request trap function

   CTRL_LOG    controls;            // MIDI controller states

   WAVE_ENTRY *wave [NUM_CHANS];// Pointer to WAVE_ENTRY for each channel

   HSAMPLE     S    [MAX_W_VOICES]; // List of HSAMPLE voices
   S32         n_voices;            // Actual # of voices allocated to synth

   S32         chan [MAX_W_VOICES]; // MIDI channel for each voice, or -1
   S32         note [MAX_W_VOICES]; // MIDI note number for voice
   S32         root [MAX_W_VOICES]; // MIDI root note for voice
   S32         rate [MAX_W_VOICES]; // Playback rate for voice
   S32         vel  [MAX_W_VOICES]; // MIDI note velocity for voice
   U32         time [MAX_W_VOICES]; // Timestamp for voice

   U32         event;               // Event counter for LRU timestamps
}
WAVE_SYNTH;

typedef WAVE_SYNTH * HWAVESYNTH;// Handle to virtual wave synthesizer


//
// DIG_DRIVER list
//

extern HDIGDRIVER     DIG_first;

//
// MDI_DRIVER list
//

extern HMDIDRIVER     MDI_first;

//
// Miscellaneous system services
//

#define FILE_READ_WITH_SIZE ((void*)(SINTa)-1)


typedef void * (AILCALLBACK *AILMEMALLOCCB)(UINTa size);
typedef void (AILCALLBACK *AILMEMFREECB)(void *);

#define  AIL_mem_alloc_lock_trk(size) AIL_mem_alloc_lock_info( size, __FILE__, __LINE__ )
#define  AIL_file_read_trk(file,dest) AIL_file_read_info( file, dest, __FILE__, __LINE__ )
#define  AIL_file_size_trk(file)      AIL_file_size_info( file, __FILE__, __LINE__ )

//#define MSS_NONTRACKED
#ifdef MSS_NONTRACKED
  DXDEC  void * AILCALL AIL_mem_alloc_lock(UINTa     size);
  DXDEC  void * AILCALL AIL_file_read(char const * filename, void * dest );
  DXDEC  S32    AILCALL AIL_file_size(char const * filename );
#else
  #define  AIL_mem_alloc_lock(size) AIL_mem_alloc_lock_trk(size)
  #define  AIL_file_read(file,dest) AIL_file_read_trk(file,dest)
  #define  AIL_file_size(file)      AIL_file_size_trk(file)
#endif

DXDEC  void *     AILCALL AIL_mem_alloc_lock_info(UINTa     size, char const * file, U32 line);
DXDEC  void       AILCALL AIL_mem_free_lock (void *ptr);


DXDEC  S32        AILCALL AIL_file_error   (void);

DXDEC  S32        AILCALL AIL_file_size_info(char const  *filename,
                                             char const * caller,
                                             U32 line);

DXDEC  void *     AILCALL AIL_file_read_info(char const   *filename,
                                             void *dest,
                                             char const * caller,
                                             U32 line);

DXDEC  S32        AILCALL AIL_file_write   (char const    *filename,
                                            void const  *buf,
                                            U32       len);

DXDEC  S32        AILCALL AIL_WAV_file_write
                                           (char const    *filename,
                                            void const  *buf,
                                            U32       len,
                                            S32       rate,
                                            S32       format);

DXDEC  S32        AILCALL AIL_file_append  (char const *filename,
                                            void const *buf, U32 len);

DXDEC AILMEMALLOCCB AILCALL AIL_mem_use_malloc(AILMEMALLOCCB fn);
DXDEC AILMEMFREECB AILCALL AIL_mem_use_free  (AILMEMFREECB fn);


#define MSSBreakPoint RR_BREAK


//
// Compiler-independent CRTL helper functions for PS2
// Exported here for use in demo programs as well as MSS itself
//

#if defined(IS_PSP)

  DXDEC F32 AILCALL AIL_sin(F32 x);
  DXDEC F32 AILCALL AIL_cos(F32 x);
  DXDEC F32 AILCALL AIL_tan( F32 x );
  DXDEC F32 AILCALL AIL_acos(F32 x);
  DXDEC F32 AILCALL AIL_atan(F32 x);
  DXDEC F32 AILCALL AIL_ceil( F32 x );
  DXDEC F32 AILCALL AIL_floor( F32 x );
  DXDEC F32 AILCALL AIL_fsqrt( F32 x );
  DXDEC F32 AILCALL AIL_fabs ( F32 x );
  DXDEC F32 AILCALL AIL_log10( F32 x );
  DXDEC F32 AILCALL AIL_log( F32 x );
  DXDEC F32 AILCALL AIL_pow( F32 x, F32 p );
  DXDEC F32 AILCALL AIL_frexpf( F32 x, S32 *pw2 );
  DXDEC F32 AILCALL AIL_ldexpf( F32 x, S32 pw2 );
  #define AIL_exp(x) AIL_pow(2.718281828F,(x))

#else

  #ifdef IS_WATCOM
    #define AIL_pow   powf
    #define AIL_tan   tanf
  #else
    #define AIL_tan   tan
    #define AIL_pow   pow
  #endif

  #define AIL_sin   sin
  #define AIL_cos   cos
  #define AIL_acos  acos
  #define AIL_atan  atan
  #define AIL_ceil  ceil
  #define AIL_floor floor

  #if defined(IS_PS3) && !defined(IS_SPU)
    DXDEC F32 AILCALL AIL_fsqrt( F32 val );
  #else
    #define AIL_fsqrt(arg) ((F32) sqrt(arg))
  #endif

  #define AIL_fabs  fabs
  #define AIL_log10 log10
  #define AIL_log   log
  #define AIL_frexpf(a1,a2) ((F32) frexp(a1,a2))
  #define AIL_ldexpf(a1,a2) ((F32) ldexp(a1,a2))
  #define AIL_exp   exp

#endif

//
// High-level support services
//

DXDEC  S32     AILCALL  AIL_startup                   (void);

DXDEC  SINTa   AILCALL  AIL_get_preference            (U32         number);

DXDEC  void    AILCALL  AIL_shutdown                  (void);

DXDEC  SINTa   AILCALL  AIL_set_preference            (U32         number,
                                                       SINTa       value);

DXDEC char *AILCALL  AIL_last_error                (void);

DXDEC  void    AILCALL  AIL_set_error                 (char const * error_msg);

#ifdef IS_WIIU
DXDEC void AILCALL AIL_set_wiiu_file_client         (void* ptr_to_fsclient, void* ptr_to_fscmdblock);
#endif

#ifdef __RADIPHONE__
//
// On iOS, audio session interruptions stop the audio queues, and we have to manually restart them.
//
// This should be called whenever you get an Interruption Ended msg via your Audio Session callback.
//
DXDEC void AILCALL AIL_ios_post_audio_session_interrupt_end(HDIGDRIVER dig);
#endif

//
// Low-level support services
//

#ifdef IS_X86

DXDEC  U32     AILCALL  AIL_MMX_available             (void);

#endif

#define AIL_lock   AIL_lock_mutex
#define AIL_unlock AIL_unlock_mutex

DXDEC  void    AILCALL  AIL_lock_mutex                (void);
DXDEC  void    AILCALL  AIL_unlock_mutex              (void);

#define AIL_delay AIL_sleep
DXDEC  void    AILCALL  AIL_sleep                     (U32         ms);

DXDEC  AILTRACECB AILCALL AIL_configure_logging       (char const * filename,
                                                       AILTRACECB cb,
                                                       S32 level);


//
// Process services
//

DXDEC  HTIMER  AILCALL  AIL_register_timer            (AILTIMERCB  fn);

DXDEC  UINTa   AILCALL  AIL_set_timer_user            (HTIMER      timer,
                                                       UINTa       user);

DXDEC  void    AILCALL  AIL_set_timer_period          (HTIMER      timer,
                                                       U32         microseconds);

DXDEC  void    AILCALL  AIL_set_timer_frequency       (HTIMER      timer,
                                                       U32         hertz);

DXDEC  void    AILCALL  AIL_set_timer_divisor         (HTIMER      timer,
                                                       U32         PIT_divisor);

DXDEC  void    AILCALL  AIL_start_timer               (HTIMER      timer);
DXDEC  void    AILCALL  AIL_start_all_timers          (void);

DXDEC  void    AILCALL  AIL_stop_timer                (HTIMER      timer);
DXDEC  void    AILCALL  AIL_stop_all_timers           (void);

DXDEC  void    AILCALL  AIL_release_timer_handle      (HTIMER      timer);
DXDEC  void    AILCALL  AIL_release_all_timers        (void);

DXDEC  S32     AILCALL AIL_timer_thread_handle(void* o_handle);

#ifdef IS_MAC
    #if defined(__PROCESSES__)
        DXDEC ProcessSerialNumber AIL_Process(void);
    #endif
#endif

//
// high-level digital services
//

#define AIL_OPEN_DIGITAL_FORCE_PREFERENCE 1
#define AIL_OPEN_DIGITAL_NEED_HW_3D       2
#define AIL_OPEN_DIGITAL_NEED_FULL_3D     4
#define AIL_OPEN_DIGITAL_NEED_LIGHT_3D    8
#define AIL_OPEN_DIGITAL_NEED_HW_REVERB   16
#define AIL_OPEN_DIGITAL_NEED_REVERB      32
#define AIL_OPEN_DIGITAL_USE_IOP_CORE0    64

#define AIL_OPEN_DIGITAL_USE_SPU0        (1<<24)
#define AIL_OPEN_DIGITAL_USE_SPU1        (2<<24)
#define AIL_OPEN_DIGITAL_USE_SPU2        (3<<24)
#define AIL_OPEN_DIGITAL_USE_SPU3        (4<<24)
#define AIL_OPEN_DIGITAL_USE_SPU4        (5<<24)
#define AIL_OPEN_DIGITAL_USE_SPU5        (6<<24)
#define AIL_OPEN_DIGITAL_USE_SPU6        (7<<24)

#define AIL_OPEN_DIGITAL_USE_SPU( num )   ( ( num + 1 ) << 24 )

#ifdef IS_GENERICDIG

    struct _RadSoundSystem;
    typedef S32 (*RADSS_OPEN_FUNC)(struct _RadSoundSystem* i_SoundSystem, U32 i_MinBufferSizeInMs, U32 i_Frequency, U32 i_ChannelCount, U32 i_MaxLockSize, U32 i_Flags);

    DXDEC HDIGDRIVER AILCALL AIL_open_generic_digital_driver(U32 frequency, S32 bits, S32 channel, U32 flags, RADSS_OPEN_FUNC dig_open);

    #ifdef IS_WIN32
        DXDEC RADSS_OPEN_FUNC AILCALL RADSS_DSInstallDriver(UINTa, UINTa);
        DXDEC RADSS_OPEN_FUNC AILCALL RADSS_WOInstallDriver(UINTa, UINTa);

        #define AIL_open_digital_driver(frequency, bits, channel, flags) \
            AIL_open_generic_digital_driver(frequency, bits, channel, flags, RADSS_DSInstallDriver(0, 0))

    #elif defined(IS_3DS)
        DXDEC RADSS_OPEN_FUNC AILCALL RADSS_3DSInstallDriver(UINTa, UINTa);

        #define AIL_open_digital_driver(frequency, bits, channel, flags) \
            AIL_open_generic_digital_driver(frequency, bits, channel, flags, RADSS_3DSInstallDriver(0, 0))

    #elif defined(__RADANDROID__)

        DXDEC void AILCALL AIL_set_asset_manager(void* asset_manager);

        DXDEC RADSS_OPEN_FUNC AILCALL RADSS_SLESInstallDriver(UINTa, UINTa);

        #define AIL_open_digital_driver(frequency, bits, channel, flags) \
            AIL_open_generic_digital_driver(frequency, bits, channel, flags, RADSS_SLESInstallDriver(0, 0))


    #elif defined(IS_PSP2)
        DXDEC RADSS_OPEN_FUNC AILCALL RADSS_PSP2InstallDriver(UINTa, UINTa);

        #define AIL_open_digital_driver(frequency, bits, channel, flags) \
            AIL_open_generic_digital_driver(frequency, bits, channel, flags, RADSS_PSP2InstallDriver(0, 0))

    #elif defined(__RADSEKRIT2__)
        DXDEC RADSS_OPEN_FUNC AILCALL RADSS_SonyInstallDriver(UINTa, UINTa);

        #define AIL_open_digital_driver(frequency, bits, channel, flags) \
            AIL_open_generic_digital_driver(frequency, bits, channel, flags, RADSS_SonyInstallDriver(0, 0))

    #elif defined(IS_PSP)
        DXDEC RADSS_OPEN_FUNC AILCALL RADSS_PSPInstallDriver(UINTa, UINTa);

        #define AIL_OPEN_DIGITAL_USE_SIMPLEAUDIO ~0U

        #define AIL_open_digital_driver(frequency, bits, channel, flags) \
            AIL_open_generic_digital_driver(frequency, bits, channel, flags, RADSS_PSPInstallDriver(0, 0))

    #elif defined(IS_XENON) || defined(__RADWINRTAPI__)
        DXDEC RADSS_OPEN_FUNC AILCALL RADSS_XAudio2InstallDriver(UINTa, UINTa);

        #define AIL_open_digital_driver(frequency, bits, channel, flags) \
            AIL_open_generic_digital_driver(frequency, bits, channel, flags, RADSS_XAudio2InstallDriver(0, 0))

    #elif defined(IS_WIIU)
        DXDEC RADSS_OPEN_FUNC AILCALL RADSS_AXInstallDriver(UINTa, UINTa);

        #define AIL_open_digital_driver(frequency, bits, channel, flags) \
            AIL_open_generic_digital_driver(frequency, bits, channel, flags, RADSS_AXInstallDriver(0, 0))

    #elif defined(IS_MAC) || defined(IS_IPHONE)
        DXDEC RADSS_OPEN_FUNC AILCALL RADSS_OalInstallDriver(UINTa, UINTa);
        DXDEC RADSS_OPEN_FUNC AILCALL RADSS_CAInstallDriver(UINTa, UINTa);

        #define AIL_open_digital_driver(frequency, bits, channel, flags) \
            AIL_open_generic_digital_driver(frequency, bits, channel, flags, RADSS_CAInstallDriver(0, 0))

    #elif defined(IS_LINUX)
        DXDEC RADSS_OPEN_FUNC AILCALL RADSS_OalInstallDriver(UINTa, UINTa);

        #define AIL_open_digital_driver(frequency, bits, channel, flags) \
            AIL_open_generic_digital_driver(frequency, bits, channel, flags, RADSS_OalInstallDriver(0, 0))
    #endif
#else // IS_GENERICDIG

DXDEC HDIGDRIVER AILCALL AIL_open_digital_driver( U32 frequency,
                                                  S32 bits,
                                                  S32 channel,
                                                  U32 flags );

#endif // not IS_GENERICDIG

DXDEC void AILCALL AIL_close_digital_driver( HDIGDRIVER dig );

#ifdef IS_LINUX

#define AIL_MSS_version(str,len)        \
{                                       \
  strncpy(str, MSS_VERSION, len);   \
}

DXDEC  S32          AILCALL AIL_digital_handle_release(HDIGDRIVER drvr);

DXDEC  S32          AILCALL AIL_digital_handle_reacquire
                                                     (HDIGDRIVER drvr);
#elif defined( IS_WINDOWS )

#define AIL_MSS_version(str,len)        \
{                                       \
  HINSTANCE l=LoadLibrary(MSSDLLNAME);  \
  if ((UINTa)l<=32)                     \
    *(str)=0;                           \
  else {                                \
    LoadString(l,1,str,len);            \
    FreeLibrary(l);                     \
  }                                     \
}

DXDEC  S32          AILCALL AIL_digital_handle_release(HDIGDRIVER drvr);

DXDEC  S32          AILCALL AIL_digital_handle_reacquire
                                                     (HDIGDRIVER drvr);

#elif defined( IS_MAC )

#if defined(__RESOURCES__)

  typedef MSS_STRUCT MSS_VersionType_
  {
    Str255 version_name;
  } MSS_VersionType;

  #define AIL_MSS_version(str,len)                        \
  {                                                       \
    long _res = HOpenResFile(0,0,"\p" MSSDLLNAME,fsRdPerm);    \
    if (_res==-1)                                         \
    {                                                     \
      str[0]=0;                                           \
    }                                                     \
    else                                                  \
    {                                                     \
      Handle _H;                                          \
      short _Err;                                         \
      long _cur= CurResFile();                            \
      UseResFile(_res);                                   \
      _H = GetResource('vers', 2);                        \
      _Err = ResError();                                  \
      if((_Err != noErr) || (_H==0))                      \
      {                                                   \
        str[0]=0;                                         \
        UseResFile(_cur);                                 \
        CloseResFile(_res);                               \
      }                                                   \
      else                                                \
      {                                                   \
        if (GetHandleSize(_H)==0)                         \
        {                                                 \
          str[0]=0;                                       \
          UseResFile(_cur);                               \
          CloseResFile(_res);                             \
        }                                                 \
        else                                              \
        {                                                 \
          MSS_VersionType * _vt = (MSS_VersionType*)*_H;  \
          if ((U32)_vt->version_name[6]>4)                \
            _vt->version_name[6]-=4;                      \
          else                                            \
            _vt->version_name[6]=0;                       \
          if (((U32)len) <= ((U32)_vt->version_name[6]))  \
            _vt->version_name[6] = (U8)len-1;             \
          memcpy( str, _vt->version_name+11, _vt->version_name[6] ); \
          str[_vt->version_name[6]]=0;                    \
          UseResFile(_cur);                               \
          CloseResFile(_res);                             \
        }                                                 \
        ReleaseResource(_H);                              \
      }                                                   \
    }                                                     \
  }

  #endif

  DXDEC  S32          AILCALL AIL_digital_handle_release(HDIGDRIVER drvr);

  DXDEC  S32          AILCALL AIL_digital_handle_reacquire
                                                     (HDIGDRIVER drvr);

#endif

DXDEC void          AILCALL AIL_debug_log (char const * ifmt, ...);

DXDEC S32           AILCALL AIL_sprintf(char *dest,
                                        char const *fmt, ...);

DXDEC char*         AILCALL AIL_set_redist_directory(char const*dir);

DXDEC  S32          AILCALL AIL_background_CPU_percent (void);

DXDEC  S32          AILCALL AIL_digital_CPU_percent   (HDIGDRIVER dig);

#ifdef HOST_SPU_PROCESS
DXDEC  S32          AILCALL AIL_digital_SPU_percent   (HDIGDRIVER dig);
#endif

DXDEC  S32          AILCALL AIL_digital_latency       (HDIGDRIVER dig);

DXDEC  HSAMPLE      AILCALL AIL_allocate_sample_handle
                                                      (HDIGDRIVER dig);


EXPGROUP(Digital Audio Services)

#define MILES_PUSH_REVERB   1
#define MILES_PUSH_VOLUME   2
#define MILES_PUSH_3D       4
#define MILES_PUSH_RESET    8

DXDEC EXPAPI void AILCALL AIL_push_system_state(HDIGDRIVER dig, U32 flags, S16 crossfade_ms);
/*
    Pushes the current system state, allowing for a temporary "clean" driver to use, and then
    revert from.

    $:dig The driver to push
    $:flags Logical "or" of options controlling the extent of the push. See discussion.
    $:crossfade_ms The number of milliseconds to fade the transition over. [0, 32767]

    By default (ie flags == 0), effectively nothing happens. Since the operation neither affects
    any subsystems nor resets the playing samples, a push immediately followed by a pop should
    have no audible effects.

    However, any samples started during the push will be stopped (via $AIL_end_fade_sample) when the system is popped.
    Streams will return SMP_DONE via $AIL_stream_status. It is up to the client code to perform any cleanup required.

    The flags can alter the above behavior in the following ways:

    $* <b>MILES_PUSH_RESET</b> - This flag causes the system to revert to a "new" state when pushed. Without any
        other flags this will only be apparent with samples - any playing samples will cease to be processed
        (though they will still report SMP_PLAYING). When the system is popped, these samples will resume.

    $* <b>MILES_PUSH_REVERB</b> - When present, reverb state will be affected in addition to sample state.
        If MILES_PUSH_RESET is present, the reverb will be cleared to zero on push. Otherwise, it will be retained,
        and only affected when popped.

    $* <b>MILES_PUSH_3D</b> - When present, 3d listener state will be affected in addition to sample state.
        If MILES_PUSH_RESET is present, the 3d listener state will be reverted to the same state as a new driver. Otherwise
        it will be retained and only affected when popped.

    $* <b>MILES_PUSH_VOLUME</b> - When present, master volume will be affected in addition to sample state.
        If MILES_PUSH_RESET is present, the master volume will be set to 1.0f, otherwise it will be retained and only
        affected when popped.

    $-

    If you want more control over whether a sample will be affected by a push or a pop operation,
    see $AIL_set_sample_level_mask.

*/

DXDEC EXPAPI void AILCALL AIL_pop_system_state(HDIGDRIVER dig, S16 crossfade_ms);
/*
    Pops the current system state and returns the system to the way it
    was before the last push.

    $:dig The driver to pop.
    $:crossfade_ms The number of milliseconds to crossfade the transition over - [0, 32767]

    See $AIL_push_system_state for documentation.
*/

DXDEC EXPAPI U8 AILCALL AIL_system_state_level(HDIGDRIVER dig);
/*
    Returns the current level the system has been pushed to.

    $:dig The driver to inspect
    $:return A value between 0 and MILES_MAX_STATES, representing the depth of the current system stack.
*/

DXDEC EXPAPI void AILCALL AIL_set_sample_level_mask(HSAMPLE S, U8 mask);
/*
    Sets the system levels at which a sample will play.

    $:S The sample to set the mask for.
    $:mask The bitmask of levels for which the sample will play.

    Under normal push/pop operations, a sample's mask is set when it is
    started to the level the system is at. If the system is pushed
    without a reset, then the mask is adjusted to include the new level.
    When a system is popped, if the sample is going to continue playing,
    the state mask is adjusted to remove the level the system is popping
    <i>from</i>.

    If you have a sample playing on a higher system level that needs
    to continue after a pop, you can adjust the sample's mask by using
    this function in conjunction with $AIL_system_state_level and
    $AIL_sample_level_mask:

    ${
        AIL_set_sample_level_mask(S, AIL_sample_level_mask(S) |= (1 << (AIL_system_state_level(dig) - 1)));
    $}
*/

DXDEC EXPAPI U8 AILCALL AIL_sample_level_mask(HSAMPLE S);
/*
    Return the mask used to determine if the sample will play at a given system level.

    $:S The sample to inspect.
    $:return The level mask for the sample.

    See $AIL_set_sample_level_mask.
*/

DXDEC EXPAPI U64 AILCALL AIL_digital_mixed_samples(HDIGDRIVER dig);
/*
    Returns the number of samples that have been mixed in to the hardware.

    Used for timing samples for start via $AIL_schedule_start_sample.
*/

#define AIL_digital_samples_per_second(dig) (dig->DMA_rate)


DXDEC EXPAPI void AILCALL AIL_enable_limiter(HDIGDRIVER dig, S32 on_off);
/*
    Enables a basic limiter to prevent clipping.

    $:dig The driver to enable the limiter on.
    $:on_off If non-zero, the limiter will be enabled, otherwise it will be disabled.

    By default limiters are off. Currently they are not configurable. They kick on around
    -10 db, and with a 0db signal will attenuate by about -18 db. Limiters run prior to
    the 16 bit clamp.

    See also $AIL_bus_enable_limiter.
*/

EXPGROUP(bus_section)

DXDEC EXPAPI HSAMPLE AILCALL AIL_allocate_bus(HDIGDRIVER dig);
/*
    Allocates a bus to mix samples to.

    $:dig The HDIGDRIVER to allocate the bus on.
    $:return The HSAMPLE for the new bus.

    A bus allows you to treat a group of samples as one sample. With the bus sample you can
    do almost all of the things you can do with a normal sample handle. The only exception
    is you can't adjust the playback rate of the sample.

    Use $AIL_bus_sample_handle to get the HSAMPLE associated with a bus.

    Each call to AIL_allocate_bus adds a new bus, up to a total bus count of MAX_BUSSES. After
    the first call, two busses exist - the main bus and the first aux bus. The HSAMPLE returned
    is for the first aux bus (index 1)
*/

DXDEC EXPAPI void AILCALL AIL_bus_enable_limiter(HDIGDRIVER dig, S32 bus_index, S32 on_off);
/*
    Enables a basic limiter to prevent clipping.

    $:dig The driver containing the bus to enable the limiter on.
    $:bus_index The index of the bus to enable the limiter on.
    $:on_off If non-zero, the limiter will be enabled, otherwise it will be disabled.

    By default limiters are off. Currently they are not configurable. They kick on around
    -10 db, and with a 0db signal will attenuate by about -18 db. Limiters run prior to
    the 16 bit clamp.

    See also $AIL_enable_limiter.
*/

DXDEC EXPAPI HSAMPLE AILCALL AIL_bus_sample_handle(HDIGDRIVER dig, S32 bus_index);
/*
    Returns the HSAMPLE associated with a bus.

    $:dig The HDIGDRIVER the bus resides within.
    $:bus_index The index of the bus to return the HSAMPLE for.
    $:return The HSAMPLE for the bus index.

    If the bus has not been allocated, or no busses have been allocated, this returns 0. This
    means that for the "Main Bus" - index 0 - it will still return zero if no additional busses
    have been allocated.
*/

DXDEC EXPAPI void AILCALL AIL_set_sample_bus(HSAMPLE S, S32 bus_index);
/*
    Assigns an HSAMPLE to a bus.

    $:S The HSAMPLE to assign.
    $:bus_index The bus index to assign the sample to.

    If the given bus has not been allocated, this function has no effect.
*/

DXDEC EXPAPI S32 AILCALL AIL_sample_bus(HSAMPLE S);
/*
    Returns the bus an HSAMPLE is assigned to.

    $:S The HSAMPLE to check.
    $:return The index of the bus the sample is assigned.

    All samples by default are assigned to bus 0.
*/

DXDEC EXPAPI S32 AILCALL AIL_install_bus_compressor(HDIGDRIVER dig, S32 bus_index, SAMPLESTAGE filter_stage, S32 input_bus_index);
/*
    Installs the Compressor filter on to a bus, using another bus as the input for
    compression/limiting.

    $:dig The driver the busses exist on.
    $:bus_index The index of the bus the compressor will affect.
    $:filter_stage The SAMPLESTAGE the compressor will use on the bus HSAMPLE.
    $:input_bus_index The bus index the compressor will use as <b>input</b>.

    This installs a side chain compressor in to a given bus. It acts exactly like
    any other filter you would put on an HSAMPLE, except the input_bus_index bus pipe's
    its signal strength to the filter, allowing it to attenuate the bus_index bus based
    on another bus's contents.

    To control the compressor parameters, access the bus's HSAMPLE via $AIL_bus_sample_handle and
    use $AIL_sample_stage_property exactly as you would any other filter. The filter's properties
    are documented under $(Compressor Filter)
*/

DXDEC void          AILCALL AIL_set_speaker_configuration
                                                      (HDIGDRIVER       dig,
                                                       MSSVECTOR3D *array,
                                                       S32              n_channels,
                                                       F32              falloff_power);

DXDEC MSSVECTOR3D *
                    AILCALL AIL_speaker_configuration
                                                      (HDIGDRIVER       dig,
                                                       S32         *n_physical_channels,
                                                       S32         *n_logical_channels,
                                                       F32         *falloff_power,
                                                       MSS_MC_SPEC *channel_spec);

DXDEC void          AILCALL AIL_set_listener_relative_receiver_array
                                                      (HDIGDRIVER             dig,
                                                       MSS_RECEIVER_LIST *array,
                                                       S32                    n_receivers);

DXDEC MSS_RECEIVER_LIST *
                    AILCALL AIL_listener_relative_receiver_array
                                                      (HDIGDRIVER dig,
                                                       S32   *n_receivers);
DXDEC void         AILCALL AIL_set_speaker_reverb_levels
                                                      (HDIGDRIVER   dig,
                                                       F32     *wet_array,
                                                       F32     *dry_array,
                                                       MSS_SPEAKER const *speaker_index_array,
                                                       S32          n_levels);

DXDEC S32          AILCALL AIL_speaker_reverb_levels  (HDIGDRIVER                   dig,
                                                       F32               * *wet_array,
                                                       F32               * *dry_array,
                                                       MSS_SPEAKER const * *speaker_index_array);


DXDEC
void AILCALL AIL_set_sample_speaker_scale_factors (HSAMPLE                 S, //)
                                                   MSS_SPEAKER const * dest_speaker_indexes,
                                                   F32         const * levels,
                                                   S32                     n_levels );
DXDEC
void AILCALL AIL_sample_speaker_scale_factors (HSAMPLE                 S, //)
                                               MSS_SPEAKER const * dest_speaker_indexes,
                                               F32               * levels,
                                               S32                     n_levels );

DXDEC
S32 AILEXPORT AIL_set_sample_is_3D                   (HSAMPLE                S, //)
                                                      S32                    onoff);

//DXDEC F32 AILEXPORT AIL_calculate_sample_final_attenuation(HSAMPLE S);
/*
    Returns the attenuation that a sample will have.

    $:S Sample to compute.
*/

DXDEC
S32   AILEXPORT AIL_calculate_3D_channel_levels      (HDIGDRIVER                   dig, //)
                                                      F32                     *channel_levels,
                                                      MSS_SPEAKER const * *speaker_array,
                                                      MSSVECTOR3D             *src_pos,
                                                      MSSVECTOR3D             *src_face,
                                                      MSSVECTOR3D             *src_up,
                                                      F32                          src_inner_angle,
                                                      F32                          src_outer_angle,
                                                      F32                          src_outer_volume,
                                                      F32                          src_max_dist,
                                                      F32                          src_min_dist,
                                                      MSSVECTOR3D             *listen_pos,
                                                      MSSVECTOR3D             *listen_face,
                                                      MSSVECTOR3D             *listen_up,
                                                      F32                          rolloff_factor,
                                                      MSSVECTOR3D             *doppler_velocity,
                                                      F32                     *doppler_shift);


DXDEC  void         AILCALL AIL_release_sample_handle (HSAMPLE S);

DXDEC  S32          AILCALL AIL_init_sample         (HSAMPLE S,
                                                     S32     format);

DXDEC  S32          AILCALL AIL_set_sample_file       (HSAMPLE   S,
                                                       void const *file_image,
                                                       S32       block);

DXDEC  S32          AILCALL AIL_set_sample_info       (HSAMPLE   S,
                                                       AILSOUNDINFO const * info);

DXDEC  S32          AILCALL AIL_set_named_sample_file (HSAMPLE   S,
                                                       C8 const   *file_type_suffix,
                                                       void const *file_image,
                                                       U32       file_size,
                                                       S32       block);

DXDEC  HPROVIDER    AILCALL AIL_set_sample_processor  (HSAMPLE     S,
                                                       SAMPLESTAGE pipeline_stage,
                                                       HPROVIDER   provider);

DXDEC  HPROVIDER    AILCALL AIL_set_digital_driver_processor
                                                      (HDIGDRIVER  dig,
                                                       DIGDRVSTAGE pipeline_stage,
                                                       HPROVIDER   provider);

DXDEC  HPROVIDER    AILCALL AIL_sample_processor      (HSAMPLE     S,
                                                       SAMPLESTAGE pipeline_stage);

DXDEC  HPROVIDER    AILCALL AIL_digital_driver_processor
                                                      (HDIGDRIVER  dig,
                                                       DIGDRVSTAGE pipeline_stage);

DXDEC  void         AILCALL AIL_set_sample_adpcm_block_size
                                                     (HSAMPLE S,
                                                     U32     blocksize);

DXDEC  void         AILCALL AIL_set_sample_address    (HSAMPLE S,
                                                     void const   *start,
                                                     U32     len);

DXDEC  void         AILCALL AIL_start_sample          (HSAMPLE S);

EXPGROUP(Digital Audio Services)

DXDEC EXPAPI void   AILCALL AIL_schedule_start_sample(HSAMPLE S, U64 mix_time_to_start);
/*
    Marks the specified sample to begin at the exact time specified.

    $:S The sample to start
    $:mix_time_to_start The time to start the sample, in samples.

    Once set, the sample will have $AIL_start_sample called automatically
    when the mixer reaches the specified time. The sample's delay will
    be automatically adjusted such that the sample starts mid-block.

    ${
        // Get the current time.
        U64 mix_time = AIL_digital_mixed_samples(dig);

        // Schedule to start 1 second out
        mix_time += AIL_digital_samples_per_second(dig);
        AIL_schedule_start_sample(S, mix_time );
    $}
*/

DXDEC EXPAPI U64    AILCALL AIL_sample_schedule_time(HSAMPLE S);
/*
    Returns the mix time the sample is scheduled to start at, or 0 if not scheduled.

    $:S The sample to query.
*/

DXDEC  void         AILCALL AIL_stop_sample           (HSAMPLE S);

DXDEC  void         AILCALL AIL_end_fade_sample      (HSAMPLE S);

DXDEC  void         AILCALL AIL_resume_sample         (HSAMPLE S);

DXDEC  void         AILCALL AIL_end_sample            (HSAMPLE S);

DXDEC EXPAPI void   AILCALL AIL_set_sample_id(HSAMPLE S, S32 id);
/*
    Set an ID on a sample for use in synchronized control.

    $:S The sample to alter
    $:id The id to use.
*/

DXDEC EXPAPI S32    AILCALL AIL_sample_id(HSAMPLE S);
/*
    Return the current ID for a sample.

    $:S Sample to access
*/

DXDEC EXPAPI void   AILCALL AIL_start_sample_group(HDIGDRIVER dig, S32 start_id, S32 set_to_id);
/*
    Start a group of samples at the same time.

    $:dig The driver the samples are allocated from.
    $:start_id The ID to start
    $:set_to_id The ID to set the samples to once they have started.

    This function atomically calls $AIL_start_sample on all the samples to ensure the samples start in sync.
*/

DXDEC EXPAPI void   AILCALL AIL_stop_sample_group(HDIGDRIVER dig, S32 stop_id, S32 set_to_id);
/*
    Stops a group of samples at the same time.

    $:dig The driver the samples are allocated from.
    $:stop_id The ID to stop
    $:set_to_id The ID to set the samples to once they have stopped.

    This function atomically calls $AIL_stop_sample on all the samples to ensure they stop at the same point.
*/

DXDEC EXPAPI void   AILCALL AIL_resume_sample_group(HDIGDRIVER dig, S32 resume_id, S32 set_to_id);
/*
    Resumes a group of samples at the same time.

    $:dig The driver the samples are allocated from.
    $:resume_id The ID to resume
    $:set_to_id The ID to set the samples to once they have resumed.

    This function atomically calls $AIL_resume_sample on all the samples to ensure the samples start in sync.
*/

DXDEC EXPAPI void   AILCALL AIL_end_sample_group(HDIGDRIVER dig, S32 end_id);
/*
    Ends a group of samples at the same time.

    $:dig The driver the samples are allocated from.
    $:end_id The ID to end

    This function atomically calls $AIL_end_sample on all the samples.
*/

DXDEC  void         AILCALL AIL_set_sample_playback_rate
                                                     (HSAMPLE S,
                                                      S32     playback_rate);

DXDEC  void         AILCALL AIL_set_sample_playback_rate_factor
                                                     (HSAMPLE S,
                                                      F32     playback_rate_factor);

DXDEC  void         AILCALL AIL_set_sample_playback_delay
                                                     (HSAMPLE S,
                                                      S32     playback_delay);

DXDEC  void         AILCALL AIL_set_sample_volume_pan (HSAMPLE S,
                                                       F32     volume,
                                                       F32     pan);

DXDEC  void         AILCALL AIL_set_sample_volume_levels(HSAMPLE S,
                                                         F32     left_level,
                                                         F32     right_level);

DXDEC  void         AILCALL AIL_set_sample_channel_levels (HSAMPLE        S,
                                                           MSS_SPEAKER const *source_speaker_indexes,
                                                           MSS_SPEAKER const *dest_speaker_indexes,
                                                           F32         const *levels,
                                                           S32                    n_levels);

DXDEC  void         AILCALL AIL_set_sample_reverb_levels(HSAMPLE S,
                                                         F32     dry_level,
                                                         F32     wet_level);

DXDEC  void         AILCALL AIL_set_sample_low_pass_cut_off(HSAMPLE S,
                                                            S32 /*-1 or MSS_SPEAKER*/ channel,
                                                            F32         cut_off);

DXDEC  void         AILCALL AIL_set_sample_loop_count (HSAMPLE S,
                                                       S32     loop_count);

DXDEC  void         AILCALL AIL_set_sample_loop_block (HSAMPLE S,
                                                       S32     loop_start_offset,
                                                       S32     loop_end_offset);

DXDEC EXPAPI S32    AILCALL AIL_set_sample_loop_samples(HSAMPLE S, S32 loop_start_samples, S32 loop_end_samples);
/*
    Defines the loop points on a sample in samples rather than bytes.

    $:S The sample to alter.
    $:loop_start_samples The sample count in to the file to start the looping.
    $:loop_end_samples The sample count in the file to end the looping.
    $:return 1 if successful, 0 otherwise. Check $AIL_last_error for details.

    For uncompressed samples, this largely reverts to $AIL_set_sample_loop_block, since the mapping
    is straightforward. For compressed formats (like bink audio or mp3), looping in sample space is
    non trivial and must be handled on a format-by-format basis. For the moment, only Bink Audio
    supports this functionality - all other ASI formats will return failure.

    If a loop's length is too short, it may be extended.
*/


DXDEC  S32          AILCALL AIL_sample_loop_block     (HSAMPLE S,
                                                       S32    *loop_start_offset,
                                                       S32    *loop_end_offset);

DXDEC  U32          AILCALL AIL_sample_status         (HSAMPLE S);

DXDEC  U32          AILCALL AIL_sample_mixed_ms       (HSAMPLE S);

DXDEC  S32          AILCALL AIL_sample_playback_rate   (HSAMPLE S);

DXDEC  F32          AILCALL AIL_sample_playback_rate_factor (HSAMPLE S);

DXDEC  S32          AILCALL AIL_sample_playback_delay (HSAMPLE S);

DXDEC  void         AILCALL AIL_sample_volume_pan     (HSAMPLE S, F32* volume, F32* pan);

DXDEC  S32          AILCALL AIL_sample_channel_count  (HSAMPLE S, U32 *mask);

DXDEC  void         AILCALL AIL_sample_channel_levels (HSAMPLE                S,
                                                       MSS_SPEAKER const *source_speaker_indexes,
                                                       MSS_SPEAKER const *dest_speaker_indexes,
                                                       F32               *levels,
                                                       S32                    n_levels);

DXDEC  void         AILCALL AIL_sample_volume_levels  (HSAMPLE  S,
                                                       F32 *left_level,
                                                       F32 *right_level);

DXDEC  void         AILCALL AIL_sample_reverb_levels  (HSAMPLE  S,
                                                       F32 *dry_level,
                                                       F32 *wet_level);

DXDEC  F32          AILCALL AIL_sample_output_levels  (HSAMPLE            S,
                                                       MSS_SPEAKER const *source_speaker_indexes,
                                                       MSS_SPEAKER const *dest_speaker_indexes,
                                                       F32               *levels,
                                                       S32                n_levels);

DXDEC  F32          AILCALL AIL_sample_low_pass_cut_off(HSAMPLE S, S32 /*-1 or MSS_SPEAKER*/ channel);

DXDEC  S32          AILCALL AIL_sample_loop_count     (HSAMPLE S);

DXDEC  void         AILCALL AIL_set_digital_master_volume_level
                                                     (HDIGDRIVER dig,
                                                      F32        master_volume);

DXDEC  F32          AILCALL AIL_digital_master_volume_level (HDIGDRIVER dig);

DXDEC  void         AILCALL AIL_set_sample_51_volume_pan( HSAMPLE S,
                                                          F32     volume,
                                                          F32     pan,
                                                          F32     fb_pan,
                                                          F32     center_level,
                                                          F32     sub_level );

DXDEC  void         AILCALL AIL_sample_51_volume_pan    ( HSAMPLE S,
                                                          F32* volume,
                                                          F32* pan,
                                                          F32* fb_pan,
                                                          F32* center_level,
                                                          F32* sub_level );

DXDEC  void         AILCALL AIL_set_sample_51_volume_levels( HSAMPLE S,
                                                             F32     f_left_level,
                                                             F32     f_right_level,
                                                             F32     b_left_level,
                                                             F32     b_right_level,
                                                             F32     center_level,
                                                             F32     sub_level );

DXDEC  void         AILCALL AIL_sample_51_volume_levels    ( HSAMPLE S,
                                                             F32* f_left_level,
                                                             F32* f_right_level,
                                                             F32* b_left_level,
                                                             F32* b_right_level,
                                                             F32* center_level,
                                                             F32* sub_level );
DXDEC  void         AILCALL AIL_set_digital_master_reverb
                                                     (HDIGDRIVER dig,
                                                      S32        bus_index,
                                                      F32        reverb_decay_time,
                                                      F32        reverb_predelay,
                                                      F32        reverb_damping);

DXDEC  void         AILCALL AIL_digital_master_reverb
                                                     (HDIGDRIVER dig,
                                                      S32    bus_index,
                                                      F32*   reverb_time,
                                                      F32*   reverb_predelay,
                                                      F32*   reverb_damping);

DXDEC  void         AILCALL AIL_set_digital_master_reverb_levels
                                                     (HDIGDRIVER dig,
                                                      S32        bus_index,
                                                      F32        dry_level,
                                                      F32        wet_level);

DXDEC  void         AILCALL AIL_digital_master_reverb_levels
                                                     (HDIGDRIVER dig,
                                                      S32    bus_index,
                                                      F32 *  dry_level,
                                                      F32 *  wet_level);


//
// low-level digital services
//

DXDEC  S32      AILCALL AIL_minimum_sample_buffer_size(HDIGDRIVER dig,
                                                     S32         playback_rate,
                                                     S32         format);

DXDEC  S32      AILCALL AIL_set_sample_buffer_count  (HSAMPLE S,
                                                      S32     n_buffers);

DXDEC  S32      AILCALL AIL_sample_loaded_len        (HSAMPLE S);

DXDEC  S32      AILCALL AIL_sample_buffer_count      (HSAMPLE S);

DXDEC  S32      AILCALL AIL_sample_buffer_available (HSAMPLE S);

DXDEC  S32      AILCALL AIL_load_sample_buffer      (HSAMPLE S,
                                                     S32     buff_num,
                                                     void const *buffer,
                                                     U32     len);

DXDEC  void     AILCALL AIL_request_EOB_ASI_reset   (HSAMPLE S,
                                                     U32     buff_num,
                                                     S32     new_stream_position);

DXDEC  S32      AILCALL AIL_sample_buffer_info      (HSAMPLE S, //)
                                                     S32     buff_num,
                                                     U32     *pos,
                                                     U32     *len,
                                                     S32     *head,
                                                     S32     *tail);

DXDEC  U32      AILCALL AIL_sample_granularity      (HSAMPLE S);

DXDEC  void     AILCALL AIL_set_sample_position      (HSAMPLE S,
                                                      U32     pos);

DXDEC  U32      AILCALL AIL_sample_position          (HSAMPLE S);

DXDEC  AILSAMPLECB AILCALL AIL_register_SOB_callback
                                                    (HSAMPLE S,
                                                     AILSAMPLECB SOB);

DXDEC  AILSAMPLECB AILCALL AIL_register_EOB_callback
                                                    (HSAMPLE S,
                                                     AILSAMPLECB EOB);

DXDEC  AILSAMPLECB AILCALL AIL_register_EOS_callback
                                                    (HSAMPLE S,
                                                     AILSAMPLECB EOS);

DXDEC  AILMIXERCB AILCALL AIL_register_mix_callback(HDIGDRIVER dig, AILMIXERCB mixcb);

DXDEC  AILFALLOFFCB AILCALL AIL_register_falloff_function_callback
                                                    (HSAMPLE S,
                                                     AILFALLOFFCB falloff_cb);

DXDEC  void     AILCALL AIL_set_sample_user_data   (HSAMPLE S,
                                                    U32     index,
                                                    SINTa   value);

DXDEC  SINTa    AILCALL AIL_sample_user_data       (HSAMPLE S,
                                                    U32     index);

DXDEC  S32      AILCALL AIL_active_sample_count    (HDIGDRIVER dig);

DXDEC  void     AILCALL AIL_digital_configuration  (HDIGDRIVER dig,
                                                      S32   *rate,
                                                      S32   *format,
                                                      char  *string);

DXDEC S32     AILCALL AIL_platform_property (void                 *object,
                                       MSS_PLATFORM_PROPERTY property,
                                       void             *before_value,
                                       void const       *new_value,
                                       void             *after_value);


DXDEC  void     AILCALL AIL_set_sample_ms_position (HSAMPLE    S, //)
                                                    S32        milliseconds);

DXDEC U32       AILCALL AIL_sample_ms_lookup       (HSAMPLE    S, //)
                                                    S32        milliseconds,
                                                    S32*       actualms);

DXDEC  void     AILCALL AIL_sample_ms_position     (HSAMPLE    S, //)
                                                    S32 *  total_milliseconds,
                                                    S32 *  current_milliseconds);

//
// Digital input services
//

#if defined(IS_WINDOWS)
  #define MSS_HAS_INPUT 1
#elif defined(IS_MAC)
  #define MSS_HAS_INPUT 1
#else
  #define MSS_HAS_INPUT 0
#endif

#if MSS_HAS_INPUT

DXDEC HDIGINPUT AILCALL AIL_open_input             (AIL_INPUT_INFO *info);

DXDEC void      AILCALL AIL_close_input            (HDIGINPUT         dig);

DXDEC AIL_INPUT_INFO *
                AILCALL AIL_get_input_info         (HDIGINPUT         dig);

DXDEC S32       AILCALL AIL_set_input_state        (HDIGINPUT         dig,
                                                    S32               enable);
#endif


//
// High-level XMIDI services
//

DXDEC HMDIDRIVER AILCALL AIL_open_XMIDI_driver( U32 flags );

#define AIL_OPEN_XMIDI_NULL_DRIVER 1

DXDEC void AILCALL AIL_close_XMIDI_driver( HMDIDRIVER mdi );

#if defined(IS_MAC) || defined(IS_LINUX)

DXDEC  S32          AILCALL AIL_MIDI_handle_release
                                                 (HMDIDRIVER mdi);

DXDEC  S32          AILCALL AIL_MIDI_handle_reacquire
                                                 (HMDIDRIVER mdi);

#elif defined( IS_WINDOWS )

DXDEC  S32          AILCALL AIL_midiOutOpen(HMDIDRIVER *drvr,
                                            LPHMIDIOUT *lphMidiOut,
                                            S32             dwDeviceID);

DXDEC  void         AILCALL AIL_midiOutClose      (HMDIDRIVER mdi);

DXDEC  S32          AILCALL AIL_MIDI_handle_release
                                                 (HMDIDRIVER mdi);

DXDEC  S32          AILCALL AIL_MIDI_handle_reacquire
                                                 (HMDIDRIVER mdi);

#endif

DXDEC  HSEQUENCE    AILCALL AIL_allocate_sequence_handle
                                                     (HMDIDRIVER mdi);

DXDEC  void         AILCALL AIL_release_sequence_handle
                                                     (HSEQUENCE S);

DXDEC  S32          AILCALL AIL_init_sequence         (HSEQUENCE S,
                                                     void const     *start,
                                                     S32       sequence_num);

DXDEC  void         AILCALL AIL_start_sequence        (HSEQUENCE S);

DXDEC  void         AILCALL AIL_stop_sequence         (HSEQUENCE S);

DXDEC  void         AILCALL AIL_resume_sequence       (HSEQUENCE S);

DXDEC  void         AILCALL AIL_end_sequence          (HSEQUENCE S);

DXDEC  void         AILCALL AIL_set_sequence_tempo    (HSEQUENCE S,
                                                       S32       tempo,
                                                       S32       milliseconds);

DXDEC  void         AILCALL AIL_set_sequence_volume   (HSEQUENCE S,
                                                       S32       volume,
                                                       S32       milliseconds);

DXDEC  void         AILCALL AIL_set_sequence_loop_count
                                                     (HSEQUENCE S,
                                                      S32       loop_count);

DXDEC  U32          AILCALL AIL_sequence_status       (HSEQUENCE S);

DXDEC  S32          AILCALL AIL_sequence_tempo        (HSEQUENCE S);

DXDEC  S32          AILCALL AIL_sequence_volume       (HSEQUENCE S);

DXDEC  S32          AILCALL AIL_sequence_loop_count   (HSEQUENCE S);

DXDEC  void         AILCALL AIL_set_XMIDI_master_volume
                                                     (HMDIDRIVER mdi,
                                                      S32         master_volume);

DXDEC  S32          AILCALL AIL_XMIDI_master_volume   (HMDIDRIVER mdi);


//
// Low-level XMIDI services
//

DXDEC  S32      AILCALL AIL_active_sequence_count     (HMDIDRIVER mdi);

DXDEC  S32      AILCALL AIL_controller_value          (HSEQUENCE S,
                                                      S32       channel,
                                                      S32       controller_num);

DXDEC  S32      AILCALL AIL_channel_notes             (HSEQUENCE S,
                                                      S32       channel);

DXDEC  void     AILCALL AIL_sequence_position         (HSEQUENCE S,
                                                      S32      *beat,
                                                      S32      *measure);

DXDEC  void     AILCALL AIL_branch_index              (HSEQUENCE  S,
                                                      U32        marker);

DXDEC  AILPREFIXCB AILCALL AIL_register_prefix_callback
                                                     (HSEQUENCE  S,
                                                      AILPREFIXCB callback);

DXDEC  AILTRIGGERCB AILCALL AIL_register_trigger_callback
                                                     (HSEQUENCE  S,
                                                      AILTRIGGERCB callback);

DXDEC  AILSEQUENCECB AILCALL AIL_register_sequence_callback
                                                     (HSEQUENCE  S,
                                                      AILSEQUENCECB callback);

DXDEC  AILBEATCB AILCALL AIL_register_beat_callback   (HSEQUENCE  S,
                                                      AILBEATCB callback);

DXDEC  AILEVENTCB AILCALL AIL_register_event_callback (HMDIDRIVER mdi,
                                                      AILEVENTCB callback);

DXDEC  AILTIMBRECB AILCALL AIL_register_timbre_callback
                                                     (HMDIDRIVER mdi,
                                                      AILTIMBRECB callback);

DXDEC  void     AILCALL AIL_set_sequence_user_data    (HSEQUENCE S,
                                                       U32       index,
                                                       SINTa     value);

DXDEC  SINTa    AILCALL AIL_sequence_user_data        (HSEQUENCE S,
                                                       U32       index);

DXDEC  void     AILCALL AIL_register_ICA_array        (HSEQUENCE S,
                                                      U8       *array);

DXDEC  S32      AILCALL AIL_lock_channel              (HMDIDRIVER mdi);

DXDEC  void     AILCALL AIL_release_channel           (HMDIDRIVER mdi,
                                                      S32         channel);

DXDEC  void     AILCALL AIL_map_sequence_channel      (HSEQUENCE S,
                                                      S32       seq_channel,
                                                      S32       new_channel);

DXDEC  S32      AILCALL AIL_true_sequence_channel     (HSEQUENCE S,
                                                      S32       seq_channel);

DXDEC  void     AILCALL AIL_send_channel_voice_message
                                                     (HMDIDRIVER  mdi,
                                                      HSEQUENCE   S,
                                                      S32         status,
                                                      S32         data_1,
                                                      S32         data_2);

DXDEC  void     AILCALL AIL_send_sysex_message        (HMDIDRIVER mdi,
                                                       void const       *buffer);

DXDEC  HWAVESYNTH
                AILCALL AIL_create_wave_synthesizer   (HDIGDRIVER dig,
                                                    HMDIDRIVER mdi,
                                                       void const       *wave_lib,
                                                       S32         polyphony);

DXDEC  void     AILCALL AIL_destroy_wave_synthesizer  (HWAVESYNTH W);

DXDEC  void     AILCALL AIL_set_sequence_ms_position  (HSEQUENCE S, //)
                                                       S32       milliseconds);

DXDEC  void     AILCALL AIL_sequence_ms_position(HSEQUENCE S, //)
                                                 S32 *total_milliseconds,
                                                 S32 *current_milliseconds);



//
// red book functions
//

#ifdef IS_WINDOWS

#pragma pack(push, 1)

typedef MSS_STRUCT _REDBOOK {
  U32 DeviceID;
  U32 paused;
  U32 pausedsec;
  U32 lastendsec;
} REDBOOK;

#pragma pack(pop)

typedef MSS_STRUCT _REDBOOK* HREDBOOK;

#define REDBOOK_ERROR    0
#define REDBOOK_PLAYING  1
#define REDBOOK_PAUSED   2
#define REDBOOK_STOPPED  3


DXDEC  HREDBOOK   AILCALL AIL_redbook_open(U32 which);

DXDEC  HREDBOOK   AILCALL AIL_redbook_open_drive(S32 drive);

DXDEC  void       AILCALL AIL_redbook_close(HREDBOOK hand);

DXDEC  void       AILCALL AIL_redbook_eject(HREDBOOK hand);

DXDEC  void       AILCALL AIL_redbook_retract(HREDBOOK hand);

DXDEC  U32        AILCALL AIL_redbook_status(HREDBOOK hand);

DXDEC  U32        AILCALL AIL_redbook_tracks(HREDBOOK hand);

DXDEC  U32        AILCALL AIL_redbook_track(HREDBOOK hand);

DXDEC  void       AILCALL AIL_redbook_track_info(HREDBOOK hand,U32 tracknum,
                                                     U32* startmsec,U32* endmsec);

DXDEC  U32        AILCALL AIL_redbook_id(HREDBOOK hand);

DXDEC  U32        AILCALL AIL_redbook_position(HREDBOOK hand);

DXDEC  U32        AILCALL AIL_redbook_play(HREDBOOK hand,U32 startmsec, U32 endmsec);

DXDEC  U32        AILCALL AIL_redbook_stop(HREDBOOK hand);

DXDEC  U32        AILCALL AIL_redbook_pause(HREDBOOK hand);

DXDEC  U32        AILCALL AIL_redbook_resume(HREDBOOK hand);

DXDEC  F32        AILCALL AIL_redbook_volume_level(HREDBOOK hand);

DXDEC  F32        AILCALL AIL_redbook_set_volume_level(HREDBOOK hand, F32 volume);

#endif

DXDEC U32 AILCALL AIL_ms_count(void);
DXDEC U32 AILCALL AIL_us_count(void);
DXDEC U64 AILCALL AIL_ms_count64(void);
DXDEC U64 AILCALL AIL_us_count64(void);
DXDEC U64 AILCALL AIL_get_time(void);
DXDEC U64 AILCALL AIL_time_to_ms(U64 time);
DXDEC U64 AILCALL AIL_ms_to_time(U64 ms);

DXDEC void AILCALL MilesUseTelemetry( void * context );
DXDEC void AILCALL MilesUseTmLite( void* context );

//
//
//

#define MSSIO_FLAGS_DONT_CLOSE_HANDLE 1
#define MSSIO_FLAGS_QUERY_SIZE_ONLY 2
#define MSSIO_FLAGS_DONT_USE_OFFSET 4

#define MSSIO_STATUS_COMPLETE                     1
#define MSSIO_STATUS_ERROR_FAILED_OPEN       0x1003
#define MSSIO_STATUS_ERROR_FAILED_READ       0x1004
#define MSSIO_STATUS_ERROR_SHUTDOWN          0x1005
#define MSSIO_STATUS_ERROR_CANCELLED         0x1006
#define MSSIO_STATUS_ERROR_MEMORY_ALLOC_FAIL 0x1007
#define MSSIO_STATUS_ERROR_MASK              0x1000

// returns percent full (1.0 = 100%)
typedef F32 (AILCALLBACK *MilesAsyncStreamCallback)(void* i_User);

struct MilesAsyncRead
{
    char FileName[256];
    U64 Offset;
    S64 Count;
    void* Buffer;
    void* StreamUserData;
    MilesAsyncStreamCallback StreamCB;
    char const * caller;
    U32 caller_line;
    UINTa FileHandle;
    S32 Flags;
    S32 ReadAmt; // current read amt.
    S32 AdditionalBuffer;
    S32 volatile Status; // This is only valid after a call to MilesAsyncFileWait or MilesAsyncFileCancel has succeeded.
    char Internal[48+128];
};

DXDEC S32 AILCALL MilesAsyncFileRead(struct MilesAsyncRead* i_Request);
DXDEC S32 AILCALL MilesAsyncFileCancel(struct MilesAsyncRead* i_Request); // 1 if the request has completed, 0 otherwise. Use Wait if needed.
DXDEC S32 AILCALL MilesAsyncFileStatus(struct MilesAsyncRead* i_Request, U32 i_MS); // 1 if complete, 0 if timeout exceeded.
DXDEC S32 AILCALL MilesAsyncStartup();
DXDEC S32 AILCALL MilesAsyncShutdown();
DXDEC S32 AILCALL AIL_IO_thread_handle(void* o_Handle);
DXDEC void AILCALL MilesAsyncSetPaused(S32 i_IsPaused);

typedef S32 (AILCALLBACK * MilesAsyncFileRead_callback)(struct MilesAsyncRead* i_Request);
typedef S32 (AILCALLBACK * MilesAsyncFileCancel_callback)(struct MilesAsyncRead* i_Request); // 1 if the request has completed, 0 otherwise. Use Wait if needed.
typedef S32 (AILCALLBACK * MilesAsyncFileStatus_callback)(struct MilesAsyncRead* i_Request, U32 i_MS); // 1 if complete, 0 if timeout exceeded.
typedef S32 (AILCALLBACK * MilesAsyncStartup_callback)();
typedef S32 (AILCALLBACK * MilesAsyncShutdown_callback)();
typedef void (AILCALLBACK * MilesAsyncSetPaused_callback)(S32 i_IsPaused);
typedef S32 (AILCALLBACK * AIL_IO_thread_handle_callback)(void* o_Handle);

DXDEC  void  AILCALL AIL_set_async_callbacks(
  MilesAsyncFileRead_callback read,
  MilesAsyncFileCancel_callback cancel,
  MilesAsyncFileStatus_callback status,
  MilesAsyncStartup_callback startup,
  MilesAsyncShutdown_callback shutdown,
  MilesAsyncSetPaused_callback setpaused,
  AIL_IO_thread_handle_callback threadhandle);

//
//
//

typedef struct _STREAM* HSTREAM;           // Handle to stream

typedef void (AILCALLBACK* AILSTREAMCB)   (HSTREAM stream);

#define MSS_STREAM_CHUNKS 8

typedef struct _STREAM
{
  S32 block_oriented; // 1 if this is an ADPCM or ASI-compressed stream
  S32 using_ASI;      // 1 if using ASI decoder to uncompress stream data
  ASISTAGE *ASI;      // handy pointer to our ASI coded

  HSAMPLE samp;       // the sample handle

  UINTa fileh;        // the open file handle

  U8* bufs[MSS_STREAM_CHUNKS];           // the data buffers
  S32 reset_ASI[MSS_STREAM_CHUNKS];      // should we reset the ASI at the end of the buffer?
  S32 reset_seek_pos[MSS_STREAM_CHUNKS]; // new stream position after reset
  S32 bufstart[MSS_STREAM_CHUNKS];       // offset of where this buffer started
  S32 loadedsizes[MSS_STREAM_CHUNKS];    // sizes of the data to be started

  struct MilesAsyncRead asyncs[MSS_STREAM_CHUNKS];
  S32 asyncs_loaded[MSS_STREAM_CHUNKS];  // 0=unloaded, 1=loading, 2=loaded, but not started
  S32 next_read_offset; // offset to pass to the next read, so the seek occurs internally. -1 to not seek.

  S32 into_Miles_index; // index of buffer that we will async into next
  S32 read_IO_index;    // index of buffer to be loaded into Miles next

  S32 bufsize;        // size of each buffer

  U32 datarate;       // datarate in bytes per second
  S32 filerate;       // original datarate of the file
  S32 filetype;       // file format type
  U32 filemask;       // channel mask for stream file
  S32 totallen;       // total length of the sound data

  S32 substart;       // subblock loop start
  S32 sublen;         // subblock loop len

  U32 blocksize;      // ADPCM block size

  S32 loadedsome;     // have we done any loads?

  U32 startpos;       // point that the sound data begins
  U32 async_pos;      // position if the last async completed

  U32 loopsleft;      // how many loops are left

  U32 error;          // read error has occurred

  S32 preload;        // preload the file into the first buffer
  U32 preloadpos;     // position to use in preload
  U32 noback;         // no background processing
  S32 alldone;        // alldone
  S32 primeamount;    // amount to load after a seek
  S32 primeleft;      // amount to read before starting

  S32 playcontrol;    // control: 0=stopped, 1=started, |8=paused, |16=sample paused

  AILSTREAMCB callback;  // end of stream callback

  SINTa user_data[8]; // Miscellaneous user data
  void* next;         // pointer to next stream

  S32 autostreaming;  // are we autostreaming this stream

  F32 level;          // io percent full
  F32 last_level;     // old io percent
  F32 percent_mult;   // factor to scale by
  S32 stream_count;   // unique number of the stream

  S32 docallback;     // set when it time to poll for a callback

  S32 was_popped;     // set to 1 if the stream needs to be freed due to a system push/pop - causes SMP_DONE to be stream_status
} MSTREAM_TYPE;


DXDEC HSTREAM AILCALL AIL_open_stream(HDIGDRIVER dig, char const * filename, S32 stream_mem);

DXDEC void AILCALL AIL_close_stream(HSTREAM stream);

DXDEC HSAMPLE  AILCALL AIL_stream_sample_handle(HSTREAM stream);

DXDEC S32 AILCALL AIL_service_stream(HSTREAM stream, S32 fillup);

DXDEC void AILCALL AIL_start_stream(HSTREAM stream);

DXDEC void AILCALL AIL_pause_stream(HSTREAM stream, S32 onoff);

DXDEC S32 AILCALL AIL_stream_loop_count(HSTREAM stream);

DXDEC void AILCALL AIL_set_stream_loop_count(HSTREAM stream, S32 count);

DXDEC void AILCALL AIL_set_stream_loop_block (HSTREAM S,
                                              S32     loop_start_offset,
                                              S32     loop_end_offset);

DXDEC S32 AILCALL AIL_stream_status(HSTREAM stream);

DXDEC F32 AILCALL AIL_stream_filled_percent(HSTREAM stream);

DXDEC void AILCALL AIL_set_stream_position(HSTREAM stream,S32 offset);

DXDEC S32 AILCALL AIL_stream_position(HSTREAM stream);

DXDEC void AILCALL AIL_stream_info(HSTREAM stream, S32* datarate, S32* sndtype, S32* length, S32* memory);

DXDEC AILSTREAMCB AILCALL AIL_register_stream_callback(HSTREAM stream, AILSTREAMCB callback);

DXDEC void AILCALL AIL_auto_service_stream(HSTREAM stream, S32 onoff);

DXDEC void     AILCALL AIL_set_stream_user_data   (HSTREAM S,
                                                   U32     index,
                                                   SINTa   value);

DXDEC SINTa    AILCALL AIL_stream_user_data       (HSTREAM S,
                                                   U32     index);

DXDEC  void     AILCALL AIL_set_stream_ms_position   (HSTREAM S,
                                                      S32        milliseconds);

DXDEC  void     AILCALL AIL_stream_ms_position     (HSTREAM    S, //)
                                                    S32 *  total_milliseconds,
                                                    S32 *  current_milliseconds);

//! \todo MSS_FILE not needed anymore?
typedef char MSS_FILE;

typedef U32  (AILCALLBACK*AIL_file_open_callback)  (MSS_FILE const* Filename,
                                                        UINTa* FileHandle);

typedef void (AILCALLBACK*AIL_file_close_callback) (UINTa FileHandle);

#define AIL_FILE_SEEK_BEGIN   0
#define AIL_FILE_SEEK_CURRENT 1
#define AIL_FILE_SEEK_END     2

typedef S32  (AILCALLBACK*AIL_file_seek_callback)  (UINTa FileHandle,
                                                        S32 Offset,
                                                        U32 Type);

typedef U32  (AILCALLBACK*AIL_file_read_callback)  (UINTa FileHandle,
                                                        void* Buffer,
                                                        U32 Bytes);

DXDEC  void  AILCALL AIL_set_file_callbacks  (AIL_file_open_callback opencb,
                                              AIL_file_close_callback closecb,
                                              AIL_file_seek_callback seekcb,
                                              AIL_file_read_callback readcb);

DXDEC void AILCALL AIL_file_callbacks(AIL_file_open_callback* opencb,
                                      AIL_file_close_callback* closecb,
                                      AIL_file_seek_callback* seekcb,
                                      AIL_file_read_callback* readcb);

#ifdef IS_32

typedef void* (AILCALLBACK *AIL_file_async_read_callback) (UINTa FileHandle,
                                                                   void* Buffer,
                                                                   U32 Bytes);

typedef S32 (AILCALLBACK*AIL_file_async_status_callback)  (void* async,
                                                               S32 wait,
                                                               U32* BytesRead);

DXDEC  void  AILCALL AIL_set_file_async_callbacks (AIL_file_open_callback opencb,
                                                   AIL_file_close_callback closecb,
                                                   AIL_file_seek_callback seekcb,
                                                   AIL_file_async_read_callback areadcb,
                                                   AIL_file_async_status_callback statuscb);

#endif

//
// High-level DLS functions
//

typedef struct _DLSFILEID {
  SINTa id;
  struct _DLSFILEID* next;
} DLSFILEID;

typedef struct _DLSFILEID* HDLSFILEID;

typedef struct _DLSDEVICE {
  VOIDFUNC* pGetPref;
  VOIDFUNC* pSetPref;
  VOIDFUNC* pMSSOpen;
  VOIDFUNC* pOpen;
  VOIDFUNC* pClose;
  VOIDFUNC* pLoadFile;
  VOIDFUNC* pLoadMem;
  VOIDFUNC* pUnloadFile;
  VOIDFUNC* pUnloadAll;
  VOIDFUNC* pGetInfo;
  VOIDFUNC* pCompact;
  VOIDFUNC* pSetAttr;
  SINTa DLSHandle;
  U32 format;
  U32 buffer_size;
  void* buffer[2];
  HSAMPLE sample;
  HMDIDRIVER mdi;
  HDIGDRIVER dig;
  HDLSFILEID first;
#if defined(__RADNT__)

  #ifdef MSS_STATIC_RIB
    #error "Bad defines - can't have a static rib on NT"
  #endif
  HMODULE lib;
#elif defined(MSS_STATIC_RIB)
  char* DOSname;
#endif
} DLSDEVICE;

typedef struct _DLSDEVICE* HDLSDEVICE;

typedef struct _AILDLSINFO {
  char Description[128];
  S32 MaxDLSMemory;
  S32 CurrentDLSMemory;
  S32 LargestSize;
  S32 GMAvailable;
  S32 GMBankSize;
} AILDLSINFO;

#ifdef MSS_STATIC_RIB

typedef struct _AILSTATICDLS {
  char* description;
  VOIDFUNC* pDLSOpen;
  VOIDFUNC* pMSSOpen;
  VOIDFUNC* pOpen;
  VOIDFUNC* pClose;
  VOIDFUNC* pLoadFile;
  VOIDFUNC* pLoadMem;
  VOIDFUNC* pUnloadFile;
  VOIDFUNC* pUnloadAll;
  VOIDFUNC* pGetInfo;
  VOIDFUNC* pCompact;
  VOIDFUNC* pSetAttr;
} AILSTATICDLS;

#endif // MSS_STATIC_RIB


DXDEC  HDLSDEVICE AILCALL AIL_DLS_open(HMDIDRIVER mdi, HDIGDRIVER dig,
#ifdef MSS_STATIC_RIB
                                          AILSTATICDLS const * staticdls,
#elif defined(__RADNT__)
                                          char const * libname,
#endif
                                          U32 flags, U32 rate, S32 bits, S32 channels);

//
// Parameters for the dwFlag used in DLSClose() and flags in AIL_DLS_close
//

#define RETAIN_DLS_COLLECTION   0x00000001
#define RETURN_TO_BOOTUP_STATE  0x00000002
#define RETURN_TO_GM_ONLY_STATE 0x00000004
#define DLS_COMPACT_MEMORY      0x00000008

DXDEC  void   AILCALL AIL_DLS_close(HDLSDEVICE dls, U32 flags);

DXDEC  HDLSFILEID AILCALL AIL_DLS_load_file(HDLSDEVICE dls, char const* filename, U32 flags);

DXDEC  HDLSFILEID AILCALL AIL_DLS_load_memory(HDLSDEVICE dls, void const* memfile, U32 flags);

//
// other parameters for AIL_DLS_unload
//

#define AIL_DLS_UNLOAD_MINE 0
#define AIL_DLS_UNLOAD_ALL  ((HDLSFILEID)(UINTa)(SINTa)-1)

DXDEC  void   AILCALL AIL_DLS_unload(HDLSDEVICE dls, HDLSFILEID dlsid);

DXDEC  void   AILCALL AIL_DLS_compact(HDLSDEVICE dls);

DXDEC  void   AILCALL AIL_DLS_get_info(HDLSDEVICE dls, AILDLSINFO* info, S32* PercentCPU);

DXDEC HSAMPLE AILCALL AIL_DLS_sample_handle(HDLSDEVICE dls);


//
// Quick-integration service functions and data types
//

typedef struct
{
   U32 const *data;
   S32  size;
   S32  type;
   void *handle;
   S32  status;
   void* next;
   S32  speed;
   F32  volume;
   F32  extravol;
   F32  dry;
   F32  wet;
   F32  cutoff;
   HDLSFILEID dlsid;
   void* dlsmem;
   void* dlsmemunc;
   S32  milliseconds;
   S32  length;
   SINTa userdata;
}
AUDIO_TYPE;


#define QSTAT_DONE          1       // Data has finished playing
#define QSTAT_LOADED        2       // Data has been loaded, but not yet played
#define QSTAT_PLAYING       3       // Data is currently playing

typedef AUDIO_TYPE * HAUDIO;        // Generic handle to any audio data type

#define AIL_QUICK_USE_WAVEOUT             2
#define AIL_QUICK_MIDI_AND_DLS            2
#define AIL_QUICK_DLS_ONLY                3
#define AIL_QUICK_MIDI_AND_VORTEX_DLS     4
#define AIL_QUICK_MIDI_AND_SONICVIBES_DLS 5

DXDEC S32     AILCALL
                       AIL_quick_startup           (
                                                    S32         use_digital,
                                                    S32         use_MIDI,
                                                    U32         output_rate,
                                                    S32         output_bits,
                                                    S32         output_channels);

DXDEC void    AILCALL AIL_quick_shutdown            (void);

DXDEC void    AILCALL AIL_quick_handles             (HDIGDRIVER* pdig,
                                                     HMDIDRIVER* pmdi,
                                                     HDLSDEVICE* pdls );

DXDEC HAUDIO  AILCALL AIL_quick_load                (char const   *filename);

DXDEC HAUDIO  AILCALL AIL_quick_load_mem            (void const   *mem,
                                                     U32    size);

DXDEC HAUDIO  AILCALL AIL_quick_load_named_mem      (void const   *mem,
                                                     char const   *filename,
                                                     U32    size);

DXDEC HAUDIO  AILCALL AIL_quick_copy                (HAUDIO      audio);

DXDEC void    AILCALL AIL_quick_unload              (HAUDIO      audio);

DXDEC S32     AILCALL AIL_quick_play                (HAUDIO      audio,
                                                    U32         loop_count);

DXDEC void    AILCALL AIL_quick_halt                (HAUDIO      audio);

DXDEC S32     AILCALL AIL_quick_status              (HAUDIO      audio);

DXDEC HAUDIO  AILCALL AIL_quick_load_and_play       (char const   *filename,
                                                    U32         loop_count,
                                                    S32         wait_request);

DXDEC void   AILCALL AIL_quick_set_speed (HAUDIO audio, S32 speed);

DXDEC void   AILCALL AIL_quick_set_volume (HAUDIO audio, F32 volume, F32 extravol);

DXDEC void   AILCALL AIL_quick_set_reverb_levels (HAUDIO audio,
                                                  F32    dry_level,
                                                  F32    wet_level);

DXDEC void   AILCALL AIL_quick_set_low_pass_cut_off(HAUDIO S,
                                                    S32 channel,
                                                    F32 cut_off);

DXDEC void   AILCALL AIL_quick_set_ms_position(HAUDIO audio,S32 milliseconds);

DXDEC S32    AILCALL AIL_quick_ms_position(HAUDIO audio);

DXDEC S32    AILCALL AIL_quick_ms_length(HAUDIO audio);


#define AIL_QUICK_XMIDI_TYPE        1
#define AIL_QUICK_DIGITAL_TYPE      2
#define AIL_QUICK_DLS_XMIDI_TYPE    3
#define AIL_QUICK_MPEG_DIGITAL_TYPE 4
#define AIL_QUICK_OGG_VORBIS_TYPE   5
#define AIL_QUICK_V12_VOICE_TYPE    6
#define AIL_QUICK_V24_VOICE_TYPE    7
#define AIL_QUICK_V29_VOICE_TYPE    8
#define AIL_QUICK_OGG_SPEEX_TYPE    9
#define AIL_QUICK_S8_VOICE_TYPE     10
#define AIL_QUICK_S16_VOICE_TYPE    11
#define AIL_QUICK_S32_VOICE_TYPE    12
#define AIL_QUICK_BINKA_TYPE        13

DXDEC S32    AILCALL AIL_quick_type(HAUDIO audio);

DXDEC S32 AILCALL AIL_WAV_info(void const* WAV_image, AILSOUNDINFO* info);

DXDEC S32 AILCALL AIL_WAV_marker_count(void const *WAV_image);

DXDEC S32 AILCALL AIL_WAV_marker_by_index(void const *WAV_image, S32 n, C8 const **name);

DXDEC S32 AILCALL AIL_WAV_marker_by_name(void const *WAV_image, C8 *name);

DXDEC S32 AILCALL AIL_size_processed_digital_audio(
                                 U32             dest_rate,
                                 U32             dest_format,
                                 S32             num_srcs,
                                 AILMIXINFO const * src);

DXDEC S32 AILCALL AIL_process_digital_audio(
                                 void       *dest_buffer,
                                 S32             dest_buffer_size,
                                 U32             dest_rate,
                                 U32             dest_format,
                                 S32             num_srcs,
                                 AILMIXINFO* src);

#define AIL_LENGTHY_INIT           0
#define AIL_LENGTHY_SET_PROPERTY   1
#define AIL_LENGTHY_UPDATE         2
#define AIL_LENGTHY_DONE           3

typedef S32 (AILCALLBACK* AILLENGTHYCB)(U32 state,UINTa user);

typedef S32 (AILCALLBACK* AILCODECSETPROP)(char const* property,void const * value);

DXDEC S32 AILCALL AIL_compress_ASI(AILSOUNDINFO const * info, //)
                                   char const* filename_ext,
                                   void** outdata,
                                   U32* outsize,
                                   AILLENGTHYCB callback);

DXDEC S32 AILCALL AIL_decompress_ASI(void const* indata, //)
                                     U32 insize,
                                     char const* filename_ext,
                                     void** wav,
                                     U32* wavsize,
                                     AILLENGTHYCB callback);

DXDEC S32 AILCALL AIL_compress_ADPCM(AILSOUNDINFO const * info,
                                     void** outdata, U32* outsize);

DXDEC S32 AILCALL AIL_decompress_ADPCM(AILSOUNDINFO const * info,
                                       void** outdata, U32* outsize);

DXDEC S32 AILCALL AIL_compress_DLS(void const* dls,
                                   char const* compression_extension,
                                   void** mls, U32* mlssize,
                                   AILLENGTHYCB callback);

DXDEC S32 AILCALL AIL_merge_DLS_with_XMI(void const* xmi, void const* dls,
                                         void** mss, U32* msssize);

DXDEC  S32 AILCALL AIL_extract_DLS( void const *source_image, //)
                                    U32             source_size,
                                    void * *XMI_output_data,
                                    U32        *XMI_output_size,
                                    void * *DLS_output_data,
                                    U32        *DLS_output_size,
                                    AILLENGTHYCB   callback);

#define AILFILTERDLS_USINGLIST 1

DXDEC S32 AILCALL AIL_filter_DLS_with_XMI(void const* xmi, void const* dls,
                                         void** dlsout, U32* dlssize,
                                         S32  flags, AILLENGTHYCB callback);

#define AILMIDITOXMI_USINGLIST 1
#define AILMIDITOXMI_TOLERANT  2

DXDEC  S32 AILCALL AIL_MIDI_to_XMI       (void const*  MIDI,
                                         U32        MIDI_size,
                                         void* *XMIDI,
                                         U32 *  XMIDI_size,
                                         S32        flags);

#define AILDLSLIST_ARTICULATION 1
#define AILDLSLIST_DUMP_WAVS    2

#if defined(IS_WIN32) || defined(IS_MAC) || defined(IS_LINUX)

DXDEC  S32          AILCALL AIL_list_DLS          (void const* DLS,
                                                   char** lst,
                                                   U32 * lst_size,
                                                   S32       flags,
                                                   C8  * title);

#define AILMIDILIST_ROLANDSYSEX 1
#define AILMIDILIST_ROLANDUN    2
#define AILMIDILIST_ROLANDAB    4

DXDEC  S32          AILCALL AIL_list_MIDI         (void const* MIDI,
                                                  U32       MIDI_size,
                                                  char** lst,
                                                  U32 * lst_size,
                                                  S32       flags);
#endif

#define AILFILETYPE_UNKNOWN         0
#define AILFILETYPE_PCM_WAV         1
#define AILFILETYPE_ADPCM_WAV       2
#define AILFILETYPE_OTHER_WAV       3
#define AILFILETYPE_VOC             4
#define AILFILETYPE_MIDI            5
#define AILFILETYPE_XMIDI           6
#define AILFILETYPE_XMIDI_DLS       7
#define AILFILETYPE_XMIDI_MLS       8
#define AILFILETYPE_DLS             9
#define AILFILETYPE_MLS            10
#define AILFILETYPE_MPEG_L1_AUDIO  11
#define AILFILETYPE_MPEG_L2_AUDIO  12
#define AILFILETYPE_MPEG_L3_AUDIO  13
#define AILFILETYPE_OTHER_ASI_WAV  14
#define AILFILETYPE_XBOX_ADPCM_WAV 15
#define AILFILETYPE_OGG_VORBIS     16
#define AILFILETYPE_V12_VOICE      17
#define AILFILETYPE_V24_VOICE      18
#define AILFILETYPE_V29_VOICE      19
#define AILFILETYPE_OGG_SPEEX      20
#define AILFILETYPE_S8_VOICE       21
#define AILFILETYPE_S16_VOICE      22
#define AILFILETYPE_S32_VOICE      23
#define AILFILETYPE_BINKA          24

DXDEC S32 AILCALL AIL_file_type(void const* data, U32 size);

DXDEC S32 AILCALL AIL_file_type_named(void const* data, char const* filename, U32 size);

DXDEC S32 AILCALL AIL_find_DLS       (void const*      data, U32 size,
                                      void** xmi, U32* xmisize,
                                      void** dls, U32* dlssize);
typedef struct
{
   //
   // File-level data accessible to app
   //
   // This is valid after AIL_inspect_MP3() is called (even if the file contains no valid frames)
   //

   U8 *MP3_file_image;       // Original MP3_file_image pointer passed to AIL_inspect_MP3()
   S32     MP3_image_size;       // Original MP3_image_size passed to AIL_inspect_MP3()

   U8 *ID3v2;                // ID3v2 tag, if not NULL
   S32     ID3v2_size;           // Size of tag in bytes

   U8 *ID3v1;                // ID3v1 tag, if not NULL (always 128 bytes long if present)

   U8 *start_MP3_data;       // Pointer to start of data area in file (not necessarily first valid frame)
   U8 *end_MP3_data;         // Pointer to last valid byte in MP3 data area (before ID3v1 tag, if any)

   //
   // Information about current frame being inspected, valid if AIL_enumerate_MP3_frames() returns
   // TRUE
   //

   S32 sample_rate;              // Sample rate in Hz (normally constant across all frames in file)
   S32 bit_rate;                 // Bits/second for current frame
   S32 channels_per_sample;      // 1 or 2
   S32 samples_per_frame;        // Always 576 or 1152 samples in each MP3 frame, depending on rate

   S32 byte_offset;              // Offset of frame from start_MP3_data (i.e., suitable for use as loop point)
   S32 next_frame_expected;      // Anticipated offset of next frame to be enumerated, if any
   S32 average_frame_size;       // Average source bytes per frame, determined solely by bit rate and sample rate
   S32 data_size;                // # of data-only bytes in this particular frame
   S32 header_size;              // 4 or 6 bytes, depending on CRC
   S32 side_info_size;           // Valid for layer 3 side info only
   S32 ngr;                      // Always 2 for MPEG1, else 1
   S32 main_data_begin;          // Always 0 in files with no bit reservoir
   S32 hpos;                     // Current bit position in header/side buffer

   S32 MPEG1;                    // Data copied directly from frame header, see ISO docs for info...
   S32 MPEG25;
   S32 layer;
   S32 protection_bit;
   S32 bitrate_index;
   S32 sampling_frequency;
   S32 padding_bit;
   S32 private_bit;
   S32 mode;
   S32 mode_extension;
   S32 copyright;
   S32 original;
   S32 emphasis;

   //
   // LAME/Xing info tag data
   //

   S32 Xing_valid;
   S32 Info_valid;
   U32 header_flags;
   S32 frame_count;
   S32 byte_count;
   S32 VBR_scale;
   U8  TOC[100];
   S32 enc_delay;
   S32 enc_padding;

   //
   // Private (undocumented) data used during frame enumeration
   //

   U8 *ptr;
   S32 bytes_left;

   S32 check_valid;
   S32 check_MPEG1;
   S32 check_MPEG25;
   S32 check_layer;
   S32 check_protection_bit;
   S32 check_sampling_frequency;
   S32 check_mode;
   S32 check_copyright;
   S32 check_original;
}
MP3_INFO;

DXDEC void AILCALL AIL_inspect_MP3 (MP3_INFO *inspection_state,
                                    U8       *MP3_file_image,
                                    S32           MP3_image_size);

DXDEC S32 AILCALL AIL_enumerate_MP3_frames (MP3_INFO *inspection_state);

typedef struct
{
   //
   // File-level data accessible to app
   //
   // This is valid after AIL_inspect_Ogg() is called (even if the file contains no valid pages)
   //

   U8 *Ogg_file_image;       // Originally passed to AIL_inspect_Ogg()
   S32     Ogg_image_size;       // Originally passed to AIL_inspect_Ogg()

   U8 *start_Ogg_data;       // Pointer to start of data area in file
   U8 *end_Ogg_data;         // Pointer to last valid byte in data area

   // Information lifted from the header after AIL_inspect_Ogg() is called.
   S32 channel_count;
   S32 sample_rate;

   //
   // Information about current page being inspected, valid if AIL_enumerate_Ogg_pages() returns
   // TRUE
   //

   S32 page_num;                 // 32-bit page sequence number from OggS header at byte offset 16

   S32 sample_count;             // Total # of samples already generated by encoder at the time the current page was written

   S32 byte_offset;              // Offset of page from start_Ogg_data (i.e., suitable for use as loop point)
   S32 next_page_expected;       // Anticipated offset of next page to be enumerated, if any

   //
   // Private (undocumented) data used during page enumeration
   //

   U8 *ptr;
   S32     bytes_left;
}
OGG_INFO;

DXDEC void AILCALL AIL_inspect_Ogg (OGG_INFO *inspection_state,
                                    U8       *Ogg_file_image,
                                    S32           Ogg_file_size);

DXDEC S32 AILCALL AIL_enumerate_Ogg_pages (OGG_INFO *inspection_state);

typedef struct
{
    const char* file_image;
    S32 image_size;

    S32 channel_count;
    S32 sample_rate;

    S32 total_samples;
    S32 samples_per_frame;

    const char* current_frame;

    // output data - byte offset for current frame.
    S32 byte_offset;
} BINKA_INFO;

DXDEC U32 AILCALL AIL_inspect_BinkA(BINKA_INFO* state, char const* file_image, S32 file_size);
DXDEC S32 AILCALL AIL_enumerate_BinkA_frames(BINKA_INFO* state);

//
// RAD room types - currently the same as EAX
//

enum
{
    ENVIRONMENT_GENERIC,                // factory default
    ENVIRONMENT_PADDEDCELL,
    ENVIRONMENT_ROOM,                   // standard environments
    ENVIRONMENT_BATHROOM,
    ENVIRONMENT_LIVINGROOM,
    ENVIRONMENT_STONEROOM,
    ENVIRONMENT_AUDITORIUM,
    ENVIRONMENT_CONCERTHALL,
    ENVIRONMENT_CAVE,
    ENVIRONMENT_ARENA,
    ENVIRONMENT_HANGAR,
    ENVIRONMENT_CARPETEDHALLWAY,
    ENVIRONMENT_HALLWAY,
    ENVIRONMENT_STONECORRIDOR,
    ENVIRONMENT_ALLEY,
    ENVIRONMENT_FOREST,
    ENVIRONMENT_CITY,
    ENVIRONMENT_MOUNTAINS,
    ENVIRONMENT_QUARRY,
    ENVIRONMENT_PLAIN,
    ENVIRONMENT_PARKINGLOT,
    ENVIRONMENT_SEWERPIPE,
    ENVIRONMENT_UNDERWATER,
    ENVIRONMENT_DRUGGED,
    ENVIRONMENT_DIZZY,
    ENVIRONMENT_PSYCHOTIC,

    ENVIRONMENT_COUNT           // total number of environments
};

//
// enumerated values for EAX
//

#ifndef EAX_H_INCLUDED

enum
{
    EAX_ENVIRONMENT_GENERIC,                // factory default
    EAX_ENVIRONMENT_PADDEDCELL,
    EAX_ENVIRONMENT_ROOM,              // standard environments
    EAX_ENVIRONMENT_BATHROOM,
    EAX_ENVIRONMENT_LIVINGROOM,
    EAX_ENVIRONMENT_STONEROOM,
    EAX_ENVIRONMENT_AUDITORIUM,
    EAX_ENVIRONMENT_CONCERTHALL,
    EAX_ENVIRONMENT_CAVE,
    EAX_ENVIRONMENT_ARENA,
    EAX_ENVIRONMENT_HANGAR,
    EAX_ENVIRONMENT_CARPETEDHALLWAY,
    EAX_ENVIRONMENT_HALLWAY,
    EAX_ENVIRONMENT_STONECORRIDOR,
    EAX_ENVIRONMENT_ALLEY,
    EAX_ENVIRONMENT_FOREST,
    EAX_ENVIRONMENT_CITY,
    EAX_ENVIRONMENT_MOUNTAINS,
    EAX_ENVIRONMENT_QUARRY,
    EAX_ENVIRONMENT_PLAIN,
    EAX_ENVIRONMENT_PARKINGLOT,
    EAX_ENVIRONMENT_SEWERPIPE,
    EAX_ENVIRONMENT_UNDERWATER,
    EAX_ENVIRONMENT_DRUGGED,
    EAX_ENVIRONMENT_DIZZY,
    EAX_ENVIRONMENT_PSYCHOTIC,

    EAX_ENVIRONMENT_COUNT           // total number of environments
};

#define EAX_REVERBMIX_USEDISTANCE (-1.0F)

#endif

#define MSS_BUFFER_HEAD (-1)

//
// Auxiliary 2D interface calls
//

DXDEC HDIGDRIVER AILCALL AIL_primary_digital_driver  (HDIGDRIVER new_primary);

//
// 3D-related calls
//

DXDEC  S32      AILCALL AIL_room_type                (HDIGDRIVER dig,
                                                      S32 bus_index);

DXDEC  void     AILCALL AIL_set_room_type            (HDIGDRIVER dig,
                                                      S32 bus_index,
                                                      S32        room_type);

DXDEC  F32      AILCALL AIL_3D_rolloff_factor        (HDIGDRIVER dig);

DXDEC  void     AILCALL AIL_set_3D_rolloff_factor    (HDIGDRIVER dig,
                                                      F32       factor);

DXDEC  F32      AILCALL AIL_3D_doppler_factor        (HDIGDRIVER dig);

DXDEC  void     AILCALL AIL_set_3D_doppler_factor    (HDIGDRIVER dig,
                                                      F32       factor);

DXDEC  F32      AILCALL AIL_3D_distance_factor       (HDIGDRIVER dig);

DXDEC  void     AILCALL AIL_set_3D_distance_factor   (HDIGDRIVER dig,
                                                      F32       factor);

DXDEC void       AILCALL AIL_set_sample_obstruction  (HSAMPLE S,
                                                      F32     obstruction);

DXDEC void       AILCALL AIL_set_sample_occlusion    (HSAMPLE S,
                                                      F32     occlusion);

DXDEC void       AILCALL AIL_set_sample_exclusion    (HSAMPLE S,
                                                      F32     exclusion);

DXDEC F32        AILCALL AIL_sample_obstruction      (HSAMPLE S);

DXDEC F32        AILCALL AIL_sample_occlusion        (HSAMPLE S);

DXDEC F32        AILCALL AIL_sample_exclusion        (HSAMPLE S);

EXPGROUP(3D Digital Audio Services)

DXDEC EXPAPI void       AILCALL AIL_set_sample_3D_volume_falloff(HSAMPLE S, MSSGRAPHPOINT* graph, S32 pointcount);
/*
    Sets a sample's volume falloff graph.

    $:S Sample to affect
    $:graph The array of points to use as the graph.
    $:pointcount The number of points passed in. Must be less than or equal to MILES_MAX_FALLOFF_GRAPH_POINTS. Passing 0 removes the graph.

    This marks a sample as having a volume falloff graph. If a sample has a volume graph, it no
    longer attenuates as per the default falloff function, and as such, its "minimum distance" no
    longer has any effect. However, the "max distance" still clamps the sample to full attenuation.

    A graph with only one point is treated as a line, returning graph[0].Y always.

    Otherwise, the graph is evaluated as follows:



    The distance to the listener is evaluated.

    The two points with X values bounding "distance" are located.

    If the distance is past the last graph point, graph[pointcount-1].Y is returned.

    If either the output tangent type of the previous point, or the input tangent type of the next point are
        MILES_TANGENT_STEP, previous->Y is returned.

    Otherwise, the segment is evaluated as a hermite curve. ITX and ITY are ignore if ITYpe is MILES_TANGENT_LINEAR,
        and likewise OTX and OTY are ignored if OType is MILES_TANGENT_LINEAR.
*/

DXDEC EXPAPI void       AILCALL AIL_set_sample_3D_lowpass_falloff(HSAMPLE S, MSSGRAPHPOINT* graph, S32 pointcount);
/*
    Sets a sample's low pass cutoff falloff graph.

    $:S Sample to affect
    $:graph The array of points to use as the graph.
    $:pointcount The number of points passed in. Must be less than or equal to MILES_MAX_FALLOFF_GRAPH_POINTS. Passing 0 removes the graph.

    This marks a sample as having a low pass cutoff that varies as a function of distance to the listener. If
    a sample has such a graph, $AIL_set_sample_low_pass_cut_off will be called constantly, and thus shouldn't be
    called otherwise.

    The graph is evaluated the same as $AIL_set_sample_3D_volume_falloff.
*/

DXDEC EXPAPI void       AILCALL AIL_set_sample_3D_exclusion_falloff(HSAMPLE S, MSSGRAPHPOINT* graph, S32 pointcount);
/*
    Sets a sample's exclusion falloff graph.

    $:S Sample to affect
    $:graph The array of points to use as the graph.
    $:pointcount The number of points passed in. Must be less than or equal to MILES_MAX_FALLOFF_GRAPH_POINTS. Passing 0 removes the graph.

    This marks a sample as having an exclusion that varies as a function of distance to the listener. If
    a sample has such a graph, auto_3D_wet_atten will be disabled to prevent double affects, as exclusion
    affects reverb wet level.

    The graph is evaluated the same as $AIL_set_sample_3D_volume_falloff.
*/

DXDEC EXPAPI void AILCALL AIL_set_sample_3D_spread_falloff(HSAMPLE S, MSSGRAPHPOINT* graph, S32 pointcount);
/*
    Sets a sample's spread falloff graph.

    $:S Sample to affect
    $:graph The array of points to use as the graph.
    $:pointcount The number of points passed in. Must be less than or equal to MILES_MAX_FALLOFF_GRAPH_POINTS. Passing 0 removes the graph.

    This marks a sample as having a spread that varies as a function of distance to the listener. See
    $AIL_set_sample_3D_spread.

    The graph is evaluated the same as $AIL_set_sample_3D_volume_falloff.
*/

DXDEC EXPAPI void AILCALL AIL_set_sample_3D_position_segments(HSAMPLE S, MSSVECTOR3D* points, S32 point_count);
/*
    Sets a sample's position as a series of line segments.

    $:S Sample to affect
    $:points The 3D points representing the line segments. 0 reverts to classic point based positioning. All
        segments are connected - N points represents N - 1 chained line segments.
    $:point_count Size of points array. Minimum 2 (unless removing), max MILES_MAX_SEGMENT_COUNT

    This marks a sample as having a position that is not a single point. When 3D attenuation is computed,
    the closest point to the listener is found by walking each segment. That position is then used in all
    other computations (cones, falloffs, etc). Spatialization is done using all segments as a directional
    source.

    If there is neither spread falloff nor volume falloff specified, spread will be automatically applied
    when the listener is within min_distance to the closest point. See $AIL_set_sample_3D_spread_falloff
    and $AIL_set_sample_3D_volume_falloff.

*/

DXDEC EXPAPI void AILCALL AIL_set_sample_3D_spread(HSAMPLE S, F32 spread);
/*
    Sets a sample's "spread" value.

    $:S Sample to affect.
    $:spread The value to set the spread to.

    Spread is how much the directionality of a sample "spreads" to more speakers - emulating
    the effect a sound has when it occupies more than a point source. For instance, a sound
    point source that sits directly to the left of the listener would have a very strong left
    speaker signal, and a fairly weak right speaker signal. Via spread, the signal would be
    more even, causing the source to feel as though it is coming from an area, rather than
    a point source.

    A spread of 1 will effectively negate any spatialization effects other than distance attenuation.
*/

DXDEC void       AILCALL AIL_set_sample_3D_distances (HSAMPLE S,
                                                      F32     max_dist,
                                                      F32     min_dist,
                                                      S32     auto_3D_wet_atten);


DXDEC void       AILCALL AIL_sample_3D_distances     (HSAMPLE S,
                                                      F32 * max_dist,
                                                      F32 * min_dist,
                                                      S32 * auto_3D_wet_atten);

DXDEC void       AILCALL AIL_set_sample_3D_cone        (HSAMPLE S,
                                                        F32     inner_angle,
                                                        F32     outer_angle,
                                                        F32     outer_volume_level);

DXDEC void       AILCALL AIL_sample_3D_cone            (HSAMPLE S,
                                                        F32*  inner_angle,
                                                        F32*  outer_angle,
                                                        F32*  outer_volume_level);

DXDEC void       AILCALL AIL_set_sample_3D_position    (HSAMPLE obj,
                                                        F32     X,
                                                        F32     Y,
                                                        F32     Z);

DXDEC void       AILCALL AIL_set_sample_3D_velocity    (HSAMPLE obj,
                                                        F32     dX_per_ms,
                                                        F32     dY_per_ms,
                                                        F32     dZ_per_ms,
                                                        F32     magnitude);

DXDEC void       AILCALL AIL_set_sample_3D_velocity_vector  (HSAMPLE obj,
                                                             F32     dX_per_ms,
                                                             F32     dY_per_ms,
                                                             F32     dZ_per_ms);

DXDEC void       AILCALL AIL_set_sample_3D_orientation      (HSAMPLE obj,
                                                             F32     X_face,
                                                             F32     Y_face,
                                                             F32     Z_face,
                                                             F32     X_up,
                                                             F32     Y_up,
                                                             F32     Z_up);

DXDEC S32        AILCALL AIL_sample_3D_position             (HSAMPLE  obj,
                                                             F32 *X,
                                                             F32 *Y,
                                                             F32 *Z);

DXDEC void       AILCALL AIL_sample_3D_velocity             (HSAMPLE  obj,
                                                             F32 *dX_per_ms,
                                                             F32 *dY_per_ms,
                                                             F32 *dZ_per_ms);

DXDEC void       AILCALL AIL_sample_3D_orientation          (HSAMPLE  obj,
                                                             F32 *X_face,
                                                             F32 *Y_face,
                                                             F32 *Z_face,
                                                             F32 *X_up,
                                                             F32 *Y_up,
                                                             F32 *Z_up);

DXDEC void       AILCALL AIL_update_sample_3D_position      (HSAMPLE obj,
                                                             F32     dt_milliseconds);

DXDEC void       AILCALL AIL_set_listener_3D_position         (HDIGDRIVER dig,
                                                               F32     X,
                                                               F32     Y,
                                                               F32     Z);

DXDEC void       AILCALL AIL_set_listener_3D_velocity         (HDIGDRIVER dig,
                                                               F32     dX_per_ms,
                                                               F32     dY_per_ms,
                                                               F32     dZ_per_ms,
                                                               F32     magnitude);

DXDEC void       AILCALL AIL_set_listener_3D_velocity_vector  (HDIGDRIVER dig,
                                                               F32     dX_per_ms,
                                                               F32     dY_per_ms,
                                                               F32     dZ_per_ms);

DXDEC void       AILCALL AIL_set_listener_3D_orientation      (HDIGDRIVER dig,
                                                               F32     X_face,
                                                               F32     Y_face,
                                                               F32     Z_face,
                                                               F32     X_up,
                                                               F32     Y_up,
                                                               F32     Z_up);

DXDEC void       AILCALL AIL_listener_3D_position             (HDIGDRIVER  dig,
                                                               F32 *X,
                                                               F32 *Y,
                                                               F32 *Z);

DXDEC void       AILCALL AIL_listener_3D_velocity             (HDIGDRIVER  dig,
                                                               F32 *dX_per_ms,
                                                               F32 *dY_per_ms,
                                                               F32 *dZ_per_ms);

DXDEC void       AILCALL AIL_listener_3D_orientation          (HDIGDRIVER  dig,
                                                               F32 *X_face,
                                                               F32 *Y_face,
                                                               F32 *Z_face,
                                                               F32 *X_up,
                                                               F32 *Y_up,
                                                               F32 *Z_up);

DXDEC void       AILCALL AIL_update_listener_3D_position      (HDIGDRIVER dig,
                                                               F32     dt_milliseconds);

#if defined( HOST_SPU_PROCESS )

DXDEC S32 AILCALL MilesStartAsyncThread( S32 thread_num, void const * param );

DXDEC S32 AILCALL MilesRequestStopAsyncThread( S32 thread_num );

DXDEC S32 AILCALL MilesWaitStopAsyncThread( S32 thread_num );

#endif


//-----------------------------------------------------------------------------
//
// MSS 8 Bank API
//
//-----------------------------------------------------------------------------

EXPGROUP(Miles High Level Event System)

// misc character maxes.
#define MSS_MAX_ASSET_NAME_BYTES 512
#define MSS_MAX_PATH_BYTES       512

#ifdef DOCS_ONLY

EXPTYPE typedef struct MSSSOUNDBANK {};
/*
   Internal structure.

   Use $HMSOUNDBANK instead.
*/

#endif

EXPTYPE typedef struct SoundBank *HMSOUNDBANK;
/*
  Describes a handle to an open sound bank.

  This handle typedef refers to an open soundbank which is usually obtained from the $AIL_add_soundbank function.
*/

EXPGROUP(highlevel_util)

DXDEC EXPAPI HMSOUNDBANK AILCALL AIL_open_soundbank(char const *filename, char const* name);
/*
    Open a sound bank. If you are using the event execution engine, use the add soundbank function
    provided there.

    $:return 0 on fail, or a valid HMSOUNDBANK.
    $:filename The filename of the soundbank to open.

    Opens a sound bank for use with the MSS8 high level functions. The sound bank must be
    closed with $AIL_close_soundbank. Use $AIL_add_soundbank if the Miles Event system is used.
*/

DXDEC EXPAPI void AILCALL AIL_close_soundbank(HMSOUNDBANK bank);
/*
    Close a soundbank previously opened with $AIL_open_soundbank.

    $:bank Soundbank to close.

    Close a soundbank previously opened with $AIL_open_soundbank. Presets/events loaded from
            this soundbank are no longer valid.
*/

DXDEC EXPAPI char const * AILCALL AIL_get_soundbank_filename(HMSOUNDBANK bank);
/*
    Return the filename used to open the given soundbank.

    $:bank Soundbank to query.

    $:return A pointer to the filename for the given soundbank, or 0 if bank is invalid.

    Returns a pointer to the filename for a soundbank. This pointer should not be deleted.
*/

DXDEC EXPAPI char const * AILCALL AIL_get_soundbank_name(HMSOUNDBANK bank);
/*
    Return the name of the given soundbank.

    $:bank Soundbank to query.

    $:return A pointer to the name of the sound bank, or 0 if the bank is invalid.

    The name of the bank is the name used in asset names. This is distinct from the
    file name of the bank.

    The return value should not be deleted.
*/

DXDEC EXPAPI S32 AILCALL AIL_get_soundbank_mem_usage(HMSOUNDBANK bank);
/*
    Returns the amount of data used by the soundbank management structures.

    $:bank Soundbank to query.
    $:return Total memory allocated.

    Returns the memory used via AIL_mem_alloc_lock during the creation of this structure.
*/

DXDEC EXPAPI S32 AILCALL AIL_enumerate_sound_presets(HMSOUNDBANK bank, HMSSENUM* next, char const* list, char const** name);
/*
  Enumerate the sound presets stored in a soundbank.

  $:bank Containing soundbank.
  $:next Enumeration token. Prior to first call, initialize to MSS_FIRST
  $:list Optional filter. If specified, presets will only enumerate from the given preset sound preset list.
  $:name The pointer to the currently enumerated preset name. This should not be deleted.

  $:return Returns 0 when enumeration is complete.

   Enumerates the sound presets available inside of a bank file. Example usage:

   ${
            HMSSENUM Token = MSS_FIRST;
            const char* PresetName = 0;
            while (AIL_enumerate_sound_presets(MyBank, &Token, 0, &PresetName))
            {
                printf("Found a preset named %s!", PresetName);

                $AIL_apply_sound_preset(MySample, MyBank, PresetName);
            }
    $}

    Note that name should NOT be deleted by the caller - this points at memory owned by
    Miles.
*/

DXDEC EXPAPI S32 AILCALL AIL_enumerate_environment_presets(HMSOUNDBANK bank, HMSSENUM* next, char const* list, char const** name);
/*
    Enumerate the environment presets stored in a soundbank.

    $:bank Containing soundbank.
    $:next Enumeration token. Prior to first call, initialize to MSS_FIRST
    $:list Optional filter. If specified, presets will only enumerate from the given environment preset list.
    $:name The pointer to the currently enumerated preset name. This should not be deleted.
    $:return Returns 0 when enumeration is complete.

    Enumerates the environment presets available inside of a bank file. Example usage:

    ${
            HMSSENUM Token = MSS_FIRST;
            const char* PresetName = 0;
            while (AIL_enumerate_environment_presets(MyBank, &Token, 0, &PresetName))
            {
                printf("Found a preset named %s!", PresetName);

                AIL_apply_environment_preset(MyDriver, MyBank, PresetName);
            }
    $}

    Note that name should NOT be deleted by the caller - this points at memory owned by
    Miles.
*/


DXDEC EXPAPI S32 AILCALL AIL_enumerate_sound_assets(HMSOUNDBANK bank, HMSSENUM* next, char const** name);
/*
    Enumerate sounds stored in a soundbank.

    $:bank Containing soundbank.
    $:next Enumeration token. Prior to first call, initialize to MSS_FIRST
    $:name The pointer to the currently enumerated sound name. This should not be deleted.
    $:return Returns 0 when enumeration is complete.

    Enumerates the sounds available inside of a bank file. Example usage:

    ${
            HMSSENUM Token = MSS_FIRST;
            const char* SoundName = 0;
            while (AIL_enumerate_sound_assets(MyBank, &Token, &SoundName))
            {
                char filename[MSS_MAX_PATH_BYTES];
                AIL_sound_asset_filename(MyBank, SoundName, filename);

                printf("Found a sound named %s!", SoundName);

                S32* pData = (S32*)AIL_file_read(filename, FILE_READ_WITH_SIZE);
                AIL_mem_free_lock(pData);
            }
    $}

    Note that name should NOT be deleted by the caller - this points at memory owned by
    Miles.
*/

DXDEC EXPAPI S32 AILCALL AIL_enumerate_events(HMSOUNDBANK bank, HMSSENUM* next, char const * list, char const ** name);
/*
    Enumerate the events stored in a soundbank.

    $:bank Soundbank to enumerate within.
    $:next Enumeration token. Prior to first call, initialize to MSS_FIRST
    $:list Optional filter. If specified, event will only enumerate from the given event list.
    $:name The pointer to the currently enumerated preset name. This should not be deleted.
    $:return Returns 0 when enumeration is complete.

    Enumerates the events available inside of a bank file. Example usage:

    ${
            HMSSENUM Token = MSS_FIRST;
            const char* EventName = 0;
            while (AIL_enumerate_events(MyBank, &Token, 0, &EventName))
            {
                printf("Found an event named %s!", EventName);

                const U8* EventContents = 0;
                AIL_get_event_contents(MyBank, EventName, &EventContents);

                AIL_enqueue_event(EventContents, 0, 0, 0, 0);
            }
    $}

    Note that name should NOT be deleted by the caller - this points at memory owned by
    Miles.
*/

DXDEC EXPAPI void* AILCALL AIL_find_environment_preset(HMSOUNDBANK bank, char const *name);
/*
    Returns the raw environment data associated with the given name.

    $:bank The bank to look within
    $:name The name of the asset to search for, including bank name.

    $:return Raw environment data. This should not be deleted.

    This function is designed to be used with $AIL_apply_raw_environment_preset.
*/

DXDEC EXPAPI void* AILCALL AIL_find_sound_preset(HMSOUNDBANK bank, char const* name);
/*
    Returns the raw preset data associated with the given name.

    $:bank The bank to look within
    $:name The name of the asset to search for, including bank name.

    $:return Raw preset data. This should not be deleted.

    This function is designed to be used with $AIL_apply_raw_sound_preset.
*/

DXDEC EXPAPI S32 AILCALL AIL_apply_raw_sound_preset(HSAMPLE sample, void* preset);
/*
    Applies the sound preset to the given sample.

    $:sample The sample to modify.
    $:preset The raw preset data to apply, returned from $AIL_find_sound_preset

    Updates sample properties based on the desired settings specified in the given preset.
*/

DXDEC EXPAPI S32 AILCALL AIL_apply_sound_preset(HSAMPLE sample, HMSOUNDBANK bank, char const *name);
/*
    Apply the sound preset to the given sample.

    $:sample The sample that will have its properties updated by the preset.
    $:bank The sound bank containing the named preset.
    $:name The name of the preset to apply.
    $:return Returns 0 on fail - check for sample/bank validity, and that the preset is in the correct bank.

    This will alter the properties on a given sample, based on the given preset.
*/

DXDEC EXPAPI S32 AILCALL AIL_unapply_raw_sound_preset(HSAMPLE sample, void* preset);
/*
    Returns the properties altered by the preset to their default state.

    $:sample The sample to update.
    $:preset The raw preset data to unapply, returned from $AIL_find_sound_preset
*/

DXDEC EXPAPI S32 AILCALL AIL_unapply_sound_preset(HSAMPLE sample, HMSOUNDBANK bank, char const *name);
/*
    Restore the properties affected by the given preset to defaults.

    $:sample The sample that will have its properties updated by the preset.
    $:bank The sound bank containing the named preset.
    $:name The name of the preset to apply.
    $:return Returns 0 on fail - check for sample/bank validity, and that the preset is in the correct bank.

    Presets may or may not affect any given property. Only the properties affected by the specified
    preset will have their values restored to default.
*/

typedef S32 (*MilesResolveFunc)(void* context, char const* exp, S32 explen, EXPOUT void* output, S32 isfloat);
/*
    Callback type for resolving variable expressions to values.

    $:context Value passed to AIL_resolve_raw_*_preset().
    $:exp The string expression to resolve.
    $:explen Length of exp.
    $:output Pointer to the memory to receive the result value.
    $:isfloat nonzero if the output needs to be a float.

    The function callback should convert variable expressions in to an output value of the
    requested type.
*/

DXDEC EXPAPI S32 AILCALL AIL_resolve_raw_sound_preset(void* preset, void* context, MilesResolveFunc eval);
/*
    Compute the value of properties for the current value of variables using the given lookup function.

    $:preset The raw preset as returns from $AIL_find_sound_preset.
    $:context The context to pass in to the resolution function.
    $:eval A function pointer to use for resolving expressions to values.
    $:return 0 if the preset is invalid.

    This function converts variable expressions that were stored in the preset in to values
    that can be used by the event system. The values are stored in the preset itself, all that
    has to happen is this is called with a valid resolve function prior to calling
    $AIL_apply_raw_sound_preset.
*/

DXDEC EXPAPI S32 AILCALL AIL_resolve_raw_environment_preset(void* env, MilesResolveFunc eval);
/*
    Compute the value of properties for the current value of variables using the given lookup function.

    $:env The raw preset as returns from $AIL_find_environment_preset.
    $:context The context to pass in to the resolution function.
    $:eval A function pointer to use for resolving expressions to values.
    $:return 0 if the preset is invalid.

    This function converts variable expressions that were stored in the environment in to values
    that can be used by the event system. The values are stored in the environment itself, all that
    has to happen is this is called with a valid resolve function prior to calling
    $AIL_apply_raw_environment_preset.
*/


DXDEC EXPAPI S32 AILCALL AIL_apply_raw_environment_preset(HDIGDRIVER dig, void* environment);
/*
    Applies the environment to the given driver.

    $:dig The driver to modify.
    $:environment The raw environment data to apply, returned from $AIL_find_environment_preset

    Updates driver properties based on the desired settings specified in the given environment.
*/

DXDEC EXPAPI S32 AILCALL AIL_apply_environment_preset(HDIGDRIVER dig, HMSOUNDBANK bank, char const *name);
/*
    Apply the environment preset to the given driver.

    $:dig The driver that will have its properties updated by the preset.
    $:bank The sound bank containing the named preset.
    $:name The name of the preset to apply.
    $:return Returns 0 on fail - check for sample/bank validity, and that the preset is in the correct bank.

    This will alter properties on a given driver, based on the given preset.
*/

DXDEC EXPAPI S32 AILCALL AIL_unapply_raw_environment_preset(HDIGDRIVER dig, void* environment);
/*
    Returns the properties the environment affects to default state.

    $:dig The driver to modify.
    $:environment The raw environment data to unapply, returned from $AIL_find_environment_preset
*/

DXDEC EXPAPI S32 AILCALL AIL_unapply_environment_preset(HDIGDRIVER dig, HMSOUNDBANK bank, char const *name);
/*
    Restore the properties affected by the given preset to defaults.

    $:dig The driver that will have its properties updated by the preset.
    $:bank The sound bank containing the named preset.
    $:name The name of the preset to apply.
    $:return Returns 0 on fail - check for sample/bank validity, and that the preset is in the correct bank.

    Presets may or may not affect any given property. Only the properties affected by the specified
    preset will have its value restored to default.
*/

EXPTYPE typedef struct _MILESBANKSOUNDINFO
{
    // If this changes at all, compiled banks must be versioned...
    S32 ChannelCount;
    U32 ChannelMask;
    S32 Rate;
    S32 DataLen;
    S32 SoundLimit;
    S32 IsExternal;
    U32 DurationMs;
    S32 StreamBufferSize;
    S32 IsAdpcm;
    S32 AdpcmBlockSize;
    F32 MixVolumeDAC;
} MILESBANKSOUNDINFO;
/*
    Structure containing all metadata associated with a sound asset.

    $:ChannelCount The number of channels the sound assets contains.
    $:ChannelMask The channel mask for the sound asset.
    $:Rate The sample rate for the sound asset.
    $:DataLen The byte count the asset requires if fully loaded.
    $:SoundLimit The maximum number of instances of this sound that is allowed to play at once.
    $:IsExternal Nonzero if the sound is stored external to the sound bank. See the eventexternal sample.
    $:DurationMs The length of the sound asset, in milliseconds.
    $:StreamBufferSize If the sound is played as a stream, this is the buffer to use for this sound.
    $:IsAdpcm Nonzero if the asset is an adpcm sound, and needs to be initialized as such.
    $:AdpcmBlockSize The adpcm block size if the asset is adpcm encoded.
    $:MixVolumeDAC The attenuation to apply to all instances of this sound, as a DAC scalar.

    See $AIL_sound_asset_info.
*/


DXDEC EXPAPI S32 AILCALL AIL_sound_asset_info(HMSOUNDBANK bank, char const* name, char* out_name, MILESBANKSOUNDINFO* out_info);
/*
    Return the meta data associated with a sound assets in a sound bank.

    $:bank The soundbank containing the sound asset.
    $:name The name of the sound asset to find.
    $:out_name Optional - Pointer to a buffer that is filled with the sound filename to use for loading.
    $:out_info Pointer to a $MILESBANKSOUNDINFO structure that is filled with meta data about the sound asset.
    $:return Returns the byte size of the buffer required for out_name.

    This function must be called in order to resolve the sound asset name to
    something that can be used by miles. To ensure safe buffer containment, call
    once with out_name as null to get the size needed.

    For external deployment see the eventexternal example program.
*/

DXDEC EXPAPI SINTa AILCALL AIL_get_marker_list(HMSOUNDBANK bank, char const* sound_name);
/*
    Return an opaque value representing the list of markers attached to a given sound name.

    $:bank The bank containing the sound asset.
    $:sound_name The name of the sound asset.

    $:return on fail/nonexistent list, or a nonzero opaque value to be passed to $AIL_find_marker_in_list.

    Returns the marker list for a given sound asset. This value should just be passed directly to $AIL_find_marker_in_list
    to retrieve the offset for a marker by name.
*/

DXDEC EXPAPI S32 AILCALL AIL_find_marker_in_list(SINTa marker_list, char const * marker_name, S32* is_samples);
/*
   Returns the byte offset into a sample corresponding to the given marker name.

   $:marker_list The marker list returned from $AIL_get_marker_list.
   $:marker_name The name of the marker to look up.
   $:is_samples returns whether the marker is at a sample location instead of a byte location.

   $:return -1 if the marker was not found, or the byte offset of the marker.

   Looks up an offset to use in functions such as $AIL_set_sample_position. marker_list can be retrieved with
   $AIL_get_marker_list.
*/

// ----------------------------
// End MSS8 declarations
// ----------------------------

//
// Event routines
//
typedef struct _MEMDUMP* HMEMDUMP;
#define HMSSEVENTCONSTRUCT HMEMDUMP

/*!
    function
    {
        ExcludeOn = 1

        Name = "AIL_create_event", "Creates an empty event to be filled with steps."

        ReturnType = "HMSSEVENTCONSTRUCT", "An empty event to be passed to the various step addition functions, or 0 if out of memory."

        Discussion = "Primarily designed for offline use, this function is the first step in
            creating an event that can be consumed by the MilesEvent system. Usage is as follows:

            HMSSEVENTCONSTRUCT hEvent = AIL_create_event();

            // misc add functions
            AIL_add_start_sound_event_step(hEvent, ...);
            AIL_add_control_sounds_event_step(hEvent, ...);
            // etc

            char* pEvent = AIL_close_event(hEvent);

            // Do something with the event

            AIL_mem_free_lock(pEvent);

            Note that if immediately passed to AIL_enqueue_event(), the memory must remain valid until the following
            $AIL_complete_event_queue_processing.

            Events are generally tailored to the MilesEvent system, even though there is nothing preventing you
            from writing your own event system, or creation ui.
            "
    }
*/
DXDEC HMSSEVENTCONSTRUCT AILCALL AIL_create_event(void);

/*!
    function
    {
        ExcludeOn = 1

        Name = "AIL_close_event", "Returns a completed event, ready for enqueueing in to the MilesEvent system."

        In = "HMSSEVENTCONSTRUCT", "i_Event", "The event to complete."

        ReturnType = "char*", "An allocated event string that can be passed to AIL_next_event_step or enqueued in the
            MilesEvent system via AIL_enqueue_event."

        Discussion = "The returned pointer must be deleted via AIL_mem_free_lock(). Note that if the MilesEvent system
            is used, the event pointer must remain valid through the following $AIL_complete_event_queue_processing call."

    }
*/
DXDEC U8* AILCALL AIL_close_event(HMSSEVENTCONSTRUCT i_Event);

EXPTYPEBEGIN typedef S32 MILES_START_STEP_EVICTION_TYPE;
#define MILES_START_STEP_PRIORITY 0
#define MILES_START_STEP_DISTANCE 1
#define MILES_START_STEP_VOLUME 2
#define MILES_START_STEP_OLDEST 3
EXPTYPEEND
/*
    Determines the behavior of a sound if it encounters a limit trying to play.

    $:MILES_START_STEP_PRIORITY Evict a sound less than our priority.
    $:MILES_START_STEP_DISTANCE Evict the farthest sound from the listener.
    $:MILES_START_STEP_VOLUME Evict the quietest sound after mixing, using the loudest channel as the qualifier.
    $:MILES_START_STEP_OLDEST Evict the sound that has been playing the longest.

    See also $AIL_add_start_sound_event_step.
*/

EXPTYPEBEGIN typedef S32 MILES_START_STEP_SELECTION_TYPE;
#define MILES_START_STEP_RANDOM 0
#define MILES_START_STEP_NO_REPEATS 1
#define MILES_START_STEP_IN_ORDER 2
#define MILES_START_STEP_RANDOM_ALL_BEFORE_REPEAT 3
#define MILES_START_STEP_BLENDED 4
#define MILES_START_STEP_SELECT_MASK 0x7
#define MILES_START_STEP_SELECT_BITS 3
EXPTYPEEND
/*
    Determines the usage of the sound names list in the $AIL_add_start_sound_event_step.

    $:MILES_START_STEP_RANDOM Randomly select from the list, and allow the same
        sound to play twice in a row. This is the only selection type that doesn't require
        a state variable.
    $:MILES_START_STEP_NO_REPEATS Randomly select from the list, but prevent the last sound from being the same.
    $:MILES_START_STEP_IN_ORDER Play the list in order, looping.
    $:MILES_START_STEP_RANDOM_ALL_BEFORE_REPEAT Randomly select from the list, but don't allow duplicates until all sounds have been played.
    $:MILES_START_STEP_BLENDED Play *all* of the sounds, using the state variable as both the variable name to poll,
        and the name of the blend function to look up. The blend should have been specified prior to execution of
        this step in the runtime, see $AIL_add_setblend_event_step.
    $:MILES_START_STEP_SELECT_MASK Expect a value from the game to determine which sound to play, added in to the other selection type.
*/

/*!
    function
    {
        ExcludeOn = 1

        Name = "AIL_add_start_sound_event_step", "Adds a step to a given event to start a sound with the given specifications."

        In = "HMSSEVENTCONSTRUCT", "i_Event", "The event to add the step to."
        In = "const char*", "i_SoundNames", "The names and associated weights for the event step to choose from.
            If there are multiple names listed, the sound will be chosen at random based on the given weights. This
            string is of the form 'BankName1/SoundName1:Weight1:BankName2/SoundName2:Weight2:' etc. The string must always
            terminate in a ':'. Weight must be between 0 and 200. To provide a null sound to randomly choose to not play anything, use
            an empty string as an entry."

        In = "const char*", "i_PresetName", "[optional] The name of the preset, of the form 'PresetList/PresetName'"
        In = "U8", "i_PresetIsDynamic", "Nonzero if the preset should poll the value of variables every frame, instead of only when applied."
        In = "const char*", "i_EventName", "[optional] The name of the event to execute upon completion of the sound, of the form 'PresetList/PresetName'"
        In = "const char*", "i_StartMarker", "[optional] The name of a marker to use as the loop start point."
        In = "const char*", "i_EndMarker", "[optional] The name of a marker to use as the loop end point."
        In = "const char*", "i_StateVar", "[optional] The name of a variable to use for storing state associated with this start sound step."
        In = "char const*", "i_VarInit", "[optional] A list of variable names, mins, and maxes to use for randomizing the sound instance state."
        In = "const char*", "i_Labels", "[optional] A comma delimited list of labels to assign to the sound."
        In = "U32", "i_Streaming", "If nonzero, the sound will be set up and started as a stream."
        In = "U8", "i_CanLoad", "If nonzero, the sound is allowed to hit the disk instead of only accessing cached sounds. If true, this might cause a hitch."
        In = "U16", "i_Delay", "The minimum delay in ms to apply to the sound before start."
        In = "U16", "i_DelayMax", "The maximum delay in ms to apply to the sound before start."
        In = "U8", "i_Priority", "The priority to assign to the sound. If a sound encounters a limit based on its labels, it will evict any sound
            with a priority strictly less than the given priority."
        In = "U8", "i_LoopCount", "The loop count as per AIL_set_sample_loop_count."
        In = "const char*", "i_StartOffset", "[optional] The name of the marker to use as the sound's initial offset."
        In = "F32", "i_VolMin", "The min volume value to randomly select for initial volume for the sound. In LinLoud."
        In = "F32", "i_VolMax", "The max volume value to randomly select for initial volume for the sound. In LinLoud."
        In = "F32", "i_PitchMin", "The min pitch to randomly select from for initial playback. In sT."
        In = "F32", "i_PitchMax", "The max pitch to randomly select from for initial playback. In sT."
        In = "F32", "i_FadeInTime", "The time to fade the sound in over. Interpolation is linear in loudness."
        In = "U8", "i_EvictionType", "The basis for deciding what sound will get kicked out if a limit is hit when trying to play this sound. See $MILES_START_STEP_EVICTION_TYPE."
        In = "U8", "i_SelectType", "The method to use for selecting the sound to play from the sound name list. See $MILES_START_SOUND_SELECTION_TYPE."

        ReturnType = "S32", "Returns 1 on success."

        Discussion = "Adds an event that can start a sound. If the sound names list contains multiple entries, one will be selected
            randomly based on the given weights and the selection type. Weights are effectively ratios for likelihood. A sound with 100 weight will be twice as likely
            as a sound with 50 weight. Some times you may want to have an event that only *might* play a sound. To do this, add a empty sound name
            with an associated weight.
            "
    }
*/
DXDEC
S32
AILCALL
AIL_add_start_sound_event_step(
    HMSSEVENTCONSTRUCT i_Event,
    const char* i_SoundNames,
    const char* i_PresetName,
    U8 i_PresetIsDynamic,
    const char* i_EventName,
    const char* i_StartMarker, const char* i_EndMarker,
    char const* i_StateVar, char const* i_VarInit,
    const char* i_Labels, U32 i_Streaming, U8 i_CanLoad,
    U16 i_Delay, U16 i_DelayMax, U8 i_Priority, U8 i_LoopCount,
    const char* i_StartOffset,
    F32 i_VolMin, F32 i_VolMax, F32 i_PitchMin, F32 i_PitchMax,
    F32 i_FadeInTime,
    U8 i_EvictionType,
    U8 i_SelectType
    );

/*!
    function
    {
        ExcludeOn = 1

        Name = "AIL_add_cache_sounds_event_step", "Adds a step to an event to load a list of sounds in to memory for play."

        In = "HMSSEVENTCONSTRUCT", "i_Event", "The event to add on to."
        In = "const char*", "bankName", "The bank filename containing all of the sounds."
        In = "const char*", "i_Sounds", "A list of colon separated sounds to load from the bank file."

        ReturnType = "S32", "Returns 1 on success."

        Discussion = "In general events are not allowed to hit the disk in order to prevent unexpected hitching during
            gameplay. In order to facilitate that, sounds need to be preloaded by this event. Each cache step can only
            load sounds from a single bank file, so for multiple bank files, multiple steps will be needed.

            In order to release the data loaded by this event, AIL_add_uncache_sounds_event_step() needs to
            be called with the same parameters.

            If you are using MilesEvent, the data is refcounted so the sound will not be freed until all
            samples using it complete."
    }
*/
DXDEC
S32
AILCALL
AIL_add_cache_sounds_event_step(
    HMSSEVENTCONSTRUCT i_Event, const char* bankName, const char* i_Sounds);


/*!
    function
    {
        ExcludeOn = 1

        Name = "AIL_add_uncache_sounds_event_step", "Adds a step to an event to free a list of sounds previously loaded in to memory for play."

        In = "HMSSEVENTCONSTRUCT", "i_Event", "The event to add on to."
        In = "const char*", "bankName", "The bank filename containing all of the sounds."
        In = "const char*", "i_Sounds", "A list of colon separated sounds from the bank file to uncache."

        ReturnType = "S32", "Returns 1 on success."

        Discussion = "This event released sounds loaded via AIL_add_cache_sounds_event_step()"
    }
*/
DXDEC
S32
AILCALL
AIL_add_uncache_sounds_event_step(
    HMSSEVENTCONSTRUCT i_Event, const char* bankName, const char* i_Sounds);


EXPTYPEBEGIN typedef S32 MILES_CONTROL_STEP_TYPE;
#define MILES_CONTROL_STEP_STOP 3
#define MILES_CONTROL_STEP_STOP_NO_EVENTS 4
#define MILES_CONTROL_STEP_PASS 0
#define MILES_CONTROL_STEP_PAUSE 1
#define MILES_CONTROL_STEP_RESUME 2
#define MILES_CONTROL_STEP_STOP_FADE 5

EXPTYPEEND
/*
    Determines how the playhead is adjusted during a $AIL_add_control_sounds_event_step.

    $:MILES_CONTROL_STEP_STOP Stop the affected sounds.
    $:MILES_CONTROL_STEP_PASS Do not change the playhead.
    $:MILES_CONTROL_STEP_PAUSE Pause the affected sounds.
    $:MILES_CONTROL_STEP_RESUME Resume the affected sounds.
    $:MILES_CONTROL_STEP_STOP_NO_EVENTS Stop the affected sounds, and prevent their completion events from playing.
    $:MILES_CONTROL_STEP_STOP_FADE Stop the sound after fading the sound out linearly in loudness.
*/

#define MILES_CONTROL_STEP_IGNORELOOP 255

/*!
    function
    {
        ExcludeOn = 1

        Name = "AIL_add_control_sounds_event_step", "Adds a step to an event to control sample playback by label."

        In = "HMSSEVENTCONSTRUCT", "i_Event", "The event to add on to."
        In = "const char*", "i_Labels", "[optional] A comma seperated list of labels to control."
        In = "const char*", "i_MarkerStart", "[optional] If exists, sets the loop start to the marker's offset."
        In = "const char*", "i_MarkerEnd", "[optional] If exists, sets the loop end to the marker's offset."
        In = "const char*", "i_Position", "[optional] If exists, sets the current playback position to the marker's offset."
        In = "const char*", "i_PresetName", "[optional] The name of the preset to apply, of the form Bank/PresetList/PresetName."
        In = "U8", "i_PresetApplyType", "If nonzero, the preset is applied dynamically(the variables are polled every frame)."
        In = "U8", "i_LoopCount", "If the loop count is not to be affected, pass MILES_CONTROL_STEP_IGNORELOOP. Otherwise, the sample's loop count will be set to this value."
        In = "U8", "i_Type", "The control type requested. See $MILES_CONTROL_STEP_TYPE."

        ReturnType = "S32", "Returns 1 on success."

        Discussion = "Controls playback of current instances. The sounds are matched either on name or label. If
            i_Labels is null, all sounds will be controlled.
            "
    }
*/
DXDEC
S32
AILCALL
AIL_add_control_sounds_event_step(
    HMSSEVENTCONSTRUCT i_Event,
    const char* i_Labels, const char* i_MarkerStart, const char* i_MarkerEnd, const char* i_Position,
    const char* i_PresetName,
    U8 i_PresetApplyType,
    F32 i_FadeOutTime,
    U8 i_LoopCount, U8 i_Type);


/*!
    function
    {
        ExcludeOn = 1

        Name = "AIL_add_apply_environment_event_step", "Adds a step to an event to apply an environment preset."

        In = "HMSSEVENTCONSTRUCT", "i_Event", "The event to add on to."
        In = "const char*", "i_EnvName", "The name of the environment preset to apply, of the form EnvList/EnvName."
        In = "U8", "i_IsDynamic", "If nonzero, any variables in the environment are polled every frame."

        ReturnType = "S32", "Returns 1 on success."

        Discussion = "Applies the specified environment preset to the current HDIGDRIVER."
    }
*/
DXDEC S32 AILCALL AIL_add_apply_environment_event_step(HMSSEVENTCONSTRUCT i_Event, const char* i_EnvName, U8 i_IsDynamic);

/*!
    function
    {
        ExcludeOn = 1

        Name = "AIL_add_comment_event_step", "Adds a step that represents a comment to the user of the editing tool."

        In = "HMSSEVENTCONSTRUCT", "i_Event", "The event to add on to."
        In = "const char*", "i_Comment", "A string to display in the editing tool."

        ReturnType = "S32", "Returns 1 on success."

        Discussion = "This event is ignored in the runtime, and only exist for editing convenience."
    }
*/
DXDEC S32 AILCALL AIL_add_comment_event_step(HMSSEVENTCONSTRUCT i_Event, const char* i_Comment);

EXPTYPEBEGIN typedef S32 MILES_RAMP_TYPE;
#define MILES_RAMPTYPE_VOLUME 0
#define MILES_RAMPTYPE_WET 1
#define MILES_RAMPTYPE_LOWPASS 2
#define MILES_RAMPTYPE_RATE 3
EXPTYPEEND
/*
    The different values the ramps can affect.

    $:MILES_RAMPTYPE_VOLUME The ramp will adjust the sample's volume, and will interpolate in loudness level. Target is in dB.
    $:MILES_RAMPTYPE_WET The ramp will affect the sample's reverb wet level, and will interpolate in loudness. Target is in dB.
    $:MILES_RAMPTYPE_LOWPASS The ramp will affect the sample's low pass cutoff. Interpolation and target are in Hz.
    $:MILES_RAMPTYPE_RATE The ramp will affect the sample's playback rate. Interpolation and target are in sT.
*/

EXPTYPEBEGIN typedef S32 MILES_INTERP_TYPE;
#define MILES_INTERP_LINEAR 0
#define MILES_INTERP_EXP 1
#define MILES_INTERP_SCURVE 2
EXPTYPEEND
/*
    The different ways the interpolation occurs for a ramp.

    $:MILES_INTERP_LINEAR The ramp will lerp between the current value and the target.
    $:MILES_INTERP_EXP The ramp will move toward the target slowly at first, then faster as it closes on its total time.
    $:MILES_INTERP_SCURVE The ramp will quickly move to about halfway, then slowly move, then move more quickly as it ends.
*/

DXDEC EXPAPI S32 AILCALL AIL_add_ramp_event_step(
    HMSSEVENTCONSTRUCT i_Event, char const* i_Name, char const* i_Labels,
    F32 i_Time, char const* i_Target, U8 i_Type, U8 i_ApplyToNew, U8 i_InterpolationType);
/*
    Add an event step that updates or creates a new ramp in the runtime.

    $:i_Event The event to add the step to.
    $:i_Name The name of the ramp. If this name already exists, the ramp will shift its target to the new value.
    $:i_Labels The label query determining the sounds the ramp will affect.
    $:i_Time The length the time in seconds the ramp will take to reach its target.
    $:i_Target The target value, or a variable expression representing the target value. The target's type is
        dependent on i_Type.
    $:i_Type One of the $MILES_RAMP_TYPE values.
    $:i_ApplyToNew If 1, the ramp will affect sounds that start after the ramp is created. If not, it will only affect sounds that
        are playing when the ramp is created. This value can not be changed once the ramp has been created.
    $:i_InterpolationType The method the ramp will affect the target values. One of $MILES_INTERP_TYPE values.

    Ramps are means of interpolating aspects of samples. They are removed from the system if they are targeted to
    a value for their type that is a non-op - meaning 0 dB, 0 sT, or >24000 Hz.

    Ramps use the current value as the start point for the interpolation. They stay at the target point,
    so you can use the same ramp name to adjust a sound's volume down, and later ramp it back up.
*/

DXDEC EXPAPI S32 AILCALL AIL_add_setblend_event_step(HMSSEVENTCONSTRUCT i_Event,
    char const* i_Name, S32 i_SoundCount, F32 const* i_InMin, F32 const* i_InMax,
    F32 const* i_OutMin, F32 const* i_OutMax, F32 const* i_MinP, F32 const* i_MaxP);
/*
    Defines a named blend function to be referenced by a blended sound later.

    $:i_Event The event to add the step to.
    $:i_Name The name of the blend. This is the name that will be
        referenced by the state variable in start sound, as well as the variable name
        to set by the game to update the blend for an instance.
    $:i_SoundCount The number of sounds this blend will affect. Max 10.
    $:i_InMin Array of length i_SoundCount representing the value of the blend variable the sound will start to fade in.
    $:i_InMax Array of length i_SoundCount representing the value of the blend variable the sound will reach full volume.
    $:i_OutMin Array of length i_SoundCount representing the value of the blend variable the sound will start to fade out.
    $:i_OutMax Array of length i_SoundCount representing the value of the blend variable the sound will cease to be audible.
    $:i_MinP Array of length i_SoundCount representing the pitch of the sound when it starts to fade in.
    $:i_MaxP Array of length i_SoundCount representing the pitch of the sound when it has completed fading out.

    This step only sets up the lookup for when a blended sound is actually started. When a blended sound plays, every frame it
    polls its state variable, then searches for a blend of the same name. If it finds both, then it uses its index in
    the start sounds list to find its relevant values from the blended sound definition.

    Once it has the correct values, it uses them to affect the sample as stated in the parameter docs above.
*/

/*!
    function
    {
        ExcludeOn = 1

        Name = "AIL_add_sound_limit_event_step", "Adds a step that defines the maximum number of playing sounds per label."

        In = "HMSSEVENTCONSTRUCT", "i_Event", "The event to add on to."
        In = "const char*", "i_SoundLimits", "A string of the form `"label count:anotherlabel count`"."

        ReturnType = "S32", "Returns 1 on success."

        Discussion = "Defines limits for instances of sounds on a per label basis. Sounds with multiple labels
            must fit under the limits for all of their labels. By default sounds are not limited other than the
            Miles max sample count."
    }
*/
DXDEC S32 AILCALL
AIL_add_sound_limit_event_step(HMSSEVENTCONSTRUCT i_Event, char const* i_LimitName, const char* i_SoundLimits);

/*!
    function
    {
        ExcludeOn = 1

        Name = "AIL_add_persist_preset_event_step", "Adds a preset that applies to current sound instances, and continues to be applied to new sounds as they are started."
        In = "HMSSEVENTCONSTRUCT", "i_Event", "The event to add on to."
        In = "const char*", "i_PresetName", "The name of the preset, of the form PresetList/PresetName. See discussion."
        In = "const char*", "i_PersistName", "The name of this persisted preset, for future removal."
        In = "const char*", "i_Labels", "The labels to apply this preset to."
        In = "U8", "i_IsDynamic", "If nonzero, the preset polls its variables every frame."

        ReturnType = "S32", "Returns 1 on success."

        Discussion = "Defines a preset by name that remains in the system, testing against all started sounds for label match. If a
            match occurs, then the preset is applied to the new sound, before the preset specified in the startsound step itself.

            In order to remove a persisted preset, refer to it by name, but leave all other parameters null.

            Example:

            // Persist a preset for players.
            AIL_add_persist_preset_event_step(hEvent, , `"Bank/PlayerEffects/Underwater`", `"Underwater`", `"player`");

            // Remove the above preset.
            AIL_add_persist_preset_event_step(hEvent, 0, `"Underwater`", 0);"
    }
*/
DXDEC S32 AILCALL
AIL_add_persist_preset_event_step(HMSSEVENTCONSTRUCT i_Event, const char* i_PresetName, const char* i_PersistName,
    const char* i_Labels, U8 i_IsDynamic
    );

DXDEC EXPAPI S32 AILCALL AIL_get_event_contents(HMSOUNDBANK bank, char const * name, U8 const** event);
/*
    Return the event data for an event, by name.

    $:bank Soundbank containing the event.
    $:name Name of the event to retrieve.
    $:event Returns an output pointer to the event contents. Note that this string isn't null terminated, and
      thus shouldn't be checked via strlen, etc.
    $:return Returns 0 on fail.

    Normally, event contents are meant to be handled by the Miles high-level system via $AIL_enqueue_event,
    rather than inspected directly.
*/

DXDEC EXPAPI S32 AILCALL AIL_add_clear_state_event_step(HMSSEVENTCONSTRUCT i_Event);
/*
    Clears all persistent state in the runtime.

    $:i_Event The event to add the step to.

    This removes all state that can stick around after an event in done executing. Ramps, Blends, Persisted
    Preset, etc.
*/

DXDEC EXPAPI S32 AILCALL AIL_add_exec_event_event_step(HMSSEVENTCONSTRUCT i_Event, char const* i_EventName);
/*
    Adds a step to run another named event.

    $:i_Event The event to add the step to.
    $:i_EventName The name of the event, of the form "Bank/Path/To/Event".

    When this step is encountered, the event is enqueued, so it will be executed the following frame (currently). It has the same parent
    event mechanics as a completion event, so the QueuedId for a sound started by it will be for the event
    that fired this step.
*/


DXDEC EXPAPI S32 AILCALL AIL_add_enable_limit_event_step(HMSSEVENTCONSTRUCT i_Event, char const* i_LimitName);
/*
    Adds a step to set the currently active limit.

    $:i_Event The event to add the step to.
    $:i_EventName The name of the limit, as defined by a set_limits event.

*/

DXDEC EXPAPI S32 AILCALL AIL_add_set_lfo_event_step(HMSSEVENTCONSTRUCT i_Event, char const* i_Name, char const* i_Base, char const* i_Amp, char const* i_Freq, S32 i_Invert, S32 i_Polarity, S32 i_Waveform, S32 i_DutyCycle, S32 i_IsLFO);
/*
    Adds a step to define a variable that oscillates over time.

    $:i_Event The event to add the step to.
	$:i_Name The nane of the variable to oscillate.
	$:i_Base The value to oscillate around, or a variable name to use as the base.
	$:i_Amp The maximum value to reach, or a variable name to use as the amplitude.
	$:i_Freq The rate at which the oscillation occurs, or a variable name to use as the rate. Rate should not exceed game tick rate / 2.
	$:i_Invert Whether the waveform should be inverted.
	$:i_Polarity Bipolar (1) or Unipolar (0) - whether the waveform goes around the base or only above it.
	$:i_Waveform Sine wave (0), Triangle (1), Saw (2), or Square(3)
	$:i_DutyCycle Only valid for square, determines what percent of the wave is "on". (0-100)
	$:i_IsLFO If zero, Base is the default value to assign the variable when the settings are applied, and the rest of the parameters are ignored.
*/

DXDEC EXPAPI S32 AILCALL AIL_add_move_var_event_step(HMSSEVENTCONSTRUCT i_Event, char const* i_Name, const F32 i_Times[2], const S32 i_InterpolationTypes[2], const F32 i_Values[3]);
/*
    Adds a step to set and move a variable over time on a curve.

    $:i_Event The event to add the step to.
	$:i_Name The variable to move.
	$:i_Times The midpoint and final times for the curves
	$:i_InterpolationTypes The curve type for the two curves - Curve In (0), Curve Out (1), S-Curve (2), Linear (3)
	$:i_Values The initial, midpoint, and final values for the variable.

	The variable is locked to this curve over the timeperiod - no interpolation from a previous value is done.

	If an existing move var exists when the new one is added, the old one is replaced.
*/

enum EVENT_STEPTYPE
{
    EVENT_STEPTYPE_STARTSOUND = 1,
    EVENT_STEPTYPE_CONTROLSOUNDS,
    EVENT_STEPTYPE_APPLYENV,
    EVENT_STEPTYPE_COMMENT,
    EVENT_STEPTYPE_CACHESOUNDS,
    EVENT_STEPTYPE_PURGESOUNDS,
    EVENT_STEPTYPE_SETLIMITS,
    EVENT_STEPTYPE_PERSIST,
    EVENT_STEPTYPE_VERSION,
    EVENT_STEPTYPE_RAMP,
    EVENT_STEPTYPE_SETBLEND,
    EVENT_STEPTYPE_CLEARSTATE,
    EVENT_STEPTYPE_EXECEVENT,
    EVENT_STEPTYPE_ENABLELIMIT,
    EVENT_STEPTYPE_SETLFO,
    EVENT_STEPTYPE_MOVEVAR
};

//! Represents an immutable string that is not null terminated, and shouldn't be deleted.
struct _MSSSTRINGC
{
    const char* str;
    S32 len;
};
typedef struct _MSSSTRINGC MSSSTRINGC;


/*!
    Represents a single step that needs to be executed for an event.

    All of the members in the structures share the same definition as
    their counterpart params in the functions that added them during
    event construction.
*/
struct EVENT_STEP_INFO
{
    //! type controls which struct in the union is accessed.
    enum EVENT_STEPTYPE type;
    union
    {
        struct
        {
            MSSSTRINGC soundname;
            MSSSTRINGC presetname;
            MSSSTRINGC eventname;
            MSSSTRINGC labels;
            MSSSTRINGC markerstart;
            MSSSTRINGC markerend;
            MSSSTRINGC startoffset;
            MSSSTRINGC statevar;
            MSSSTRINGC varinit;
            U32 stream;
            F32 volmin,volmax,pitchmin,pitchmax;
            F32 fadeintime;
            U16 delaymin;
            U16 delaymax;
            U8 canload;
            U8 priority;
            U8 loopcount;
            U8 evictiontype;
            U8 selecttype;
            U8 presetisdynamic;
        } start;

        struct
        {
            MSSSTRINGC labels;
            MSSSTRINGC markerstart;
            MSSSTRINGC markerend;
            MSSSTRINGC position;
            MSSSTRINGC presetname;
            F32 fadeouttime;
            U8 presetapplytype;
            U8 loopcount;
            U8 type;
        } control;

        struct
        {
            MSSSTRINGC envname;
            U8 isdynamic;
        } env;

        struct
        {
            MSSSTRINGC comment;
        } comment;

        struct
        {
            MSSSTRINGC lib;
            const char** namelist;
            S32 namecount;
        } load;

        struct
        {
            MSSSTRINGC limits;
            MSSSTRINGC name;
        } limits;

        struct
        {
            MSSSTRINGC name;
            MSSSTRINGC presetname;
            MSSSTRINGC labels;
            U8 isdynamic;
        } persist;

        struct
        {
            MSSSTRINGC name;
            MSSSTRINGC labels;
            MSSSTRINGC target;
            F32 time;
            U8 type;
            U8 apply_to_new;
            U8 interpolate_type;
        } ramp;

        struct
        {
            MSSSTRINGC name;
            F32 inmin[10];
            F32 inmax[10];
            F32 outmin[10];
            F32 outmax[10];
            F32 minp[10];
            F32 maxp[10];
            U8 count;
        } blend;

        struct
        {
            MSSSTRINGC eventname;
        } exec;

        struct
        {
            MSSSTRINGC limitname;
        } enablelimit;

        struct
        {
            MSSSTRINGC name;
            MSSSTRINGC base;
            MSSSTRINGC amplitude;
            MSSSTRINGC freq;
            S32 invert;
            S32 polarity;
            S32 waveform;
            S32 dutycycle;
            S32 islfo;
        } setlfo;

        struct
        {
            MSSSTRINGC name;
            F32 time[2];
            S32 interpolate_type[2];
            F32 value[3];
        } movevar;
    };
};

/*!
    function
    {
        ExcludeOn = 1

        Name = "AIL_next_event_step", "Retrieves the next step in the event buffer, parsing it in to a provided buffer."

        In = "const U8*", "i_EventString", "The event returned by $AIL_close_event, or a previous call to $AIL_next_event_step"
        Out = "const EVENT_STEP_INFO*", "o_Step", "A pointer to the step struct will be stored here."
        In = "void*", "i_Buffer", "A working buffer for the function to use for parsing."
        In = "S32", "i_BufferSize", "The size in bytes of the working buffer."

        ReturnType = "U8 char*", "Returns 0 on fail or when the event string has been exhausted of steps. Otherwise, returns
            the string location of the next event step in the buffer."

        Discussion = "This function parses the event string in to a struct for usage by the user. This function should only be
            used by the MilesEvent system. It returns the pointer to the next step to be passed to this function to get the
            next step. In this manner it can be used in a loop:

            // Create an event to stop all sounds.
            HMSSEVENTCONSTRUCT hEvent = AIL_create_event();
            AIL_add_control_sound_event_step(hEvent, 0, 0, 0, 0, 0, 0, 255, 3);
            char* pEvent = AIL_close_event(hEvent);

            char EventBuffer[4096];
            EVENT_STEP_INFO* pStep = 0;
            char* pCurrentStep = pEvent;

            while (pCurrentStep)
            {
                pStep = 0;
                pCurrentStep = AIL_next_event_step(pCurrentStep, &pStep, EventBuffer, 4096);
                if (pStep == 0)
                {
                    // Error, or an empty event. If $AIL_last_error is an empty string, then it was an empty event.
                    break;
                }

                // Handle event step.
                switch (pStep->type)
                {
                    default: break;
                }
            }

            AIL_mem_free_lock(pEvent);
            "
    }
*/
DXDEC const U8* AILCALL AIL_next_event_step(const U8* i_EventString, struct EVENT_STEP_INFO** o_Step, void* i_Buffer, S32 i_BufferSize);


// Old style names.
#define AIL_find_event                      MilesFindEvent
#define AIL_clear_event_queue               MilesClearEventQueue
#define AIL_register_random                 MilesRegisterRand
#define AIL_enumerate_sound_instances       MilesEnumerateSoundInstances
#define AIL_enumerate_preset_persists       MilesEnumeratePresetPersists
#define AIL_enqueue_event                   MilesEnqueueEvent
#define AIL_enqueue_event_system            MilesEnqueueEventContext
#define AIL_enqueue_event_by_name           MilesEnqueueEventByName
#define AIL_begin_event_queue_processing    MilesBeginEventQueueProcessing
#define AIL_complete_event_queue_processing MilesCompleteEventQueueProcessing
#define AIL_startup_event_system            MilesStartupEventSystem
#define AIL_shutdown_event_system           MilesShutdownEventSystem
#define AIL_add_soundbank                   MilesAddSoundBank
#define AIL_release_soundbank               MilesReleaseSoundBank
#define AIL_set_sound_label_limits          MilesSetSoundLabelLimits
#define AIL_text_dump_event_system          MilesTextDumpEventSystem
#define AIL_event_system_state              MilesGetEventSystemState
#define AIL_get_event_length                MilesGetEventLength
#define AIL_stop_sound_instances            MilesStopSoundInstances
#define AIL_pause_sound_instances           MilesPauseSoundInstances
#define AIL_resume_sound_instances          MilesResumeSoundInstances
#define AIL_start_sound_instance            MilesStartSoundInstance
#define AIL_set_event_error_callback        MilesSetEventErrorCallback
#define AIL_set_event_bank_functions        MilesSetBankFunctions
#define AIL_get_event_bank_functions        MilesGetBankFunctions

#define AIL_set_variable_int                MilesSetVarI
#define AIL_set_variable_float              MilesSetVarF
#define AIL_variable_int                    MilesGetVarI
#define AIL_variable_float                  MilesGetVarF

#define AIL_set_sound_start_offset          MilesSetSoundStartOffset
#define AIL_requeue_failed_asyncs           MilesRequeueAsyncs
#define AIL_add_event_system                MilesAddEventSystem

#define AIL_audition_local_host             MilesAuditionLocalHost
#define AIL_audition_connect                MilesAuditionConnect
#define AIL_audition_startup                MilesAuditionStartup
#define AIL_audition_shutdown               MilesAuditionShutdown
EXPGROUP(Miles High Level Event System)

EXPTYPE typedef void* HEVENTSYSTEM;
/*
    The type used to distinguish between running event systems.

    Only used if multiple event systems are running. See the eventmultiple example.
*/

DXDEC EXPAPI HEVENTSYSTEM AILCALL AIL_startup_event_system(HDIGDRIVER dig, S32 command_buf_len, EXPOUT char* memory_buf, S32 memory_len);
/*
   Initializes the Miles Event system and associates it with an open digital driver.

   $:dig The digital sound driver that this event system should use.
   $:command_buf_len An optional number of bytes to use for the command buffer. If you pass 0, a reasonable default will be used (currently 5K).
   $:memory_buf An optional pointer to a memory buffer buffer that the event system will use for all event allocations.
            Note that the sound data itself is not stored in this buffer - it is only for internal buffers, the command buffer, and instance data.
            Use 0 to let Miles to allocate this buffer itself.
   $:memory_len If memory_buf is non-null, then this parameter provides the length.  If memory_buf is null, the Miles will
      allocate this much memory for internal buffers. If both memory_buf and memory_len are null, the Miles will allocate reasonable default (currently 64K).
   $:return Returns 0 on startup failure.

   This function starts up the Miles Event System, which is used to trigger events throughout your game.
   You call it after $AIL_open_digital_driver.
*/

DXDEC EXPAPI HEVENTSYSTEM AILCALL AIL_add_event_system(HDIGDRIVER dig);
/*
    Creates an additional event system attached to a different driver, in the event that you need to trigger events
    tied to different sound devices.

    $:dig The digital sound driver to attach the new event system to.
    $:return A handle to the event system to use in various high level functions.

    Both systems will access the same set of loaded soundbanks, and are updated when $AIL_begin_event_queue_processing is called.

    To enqueue events to the new system, use $AIL_enqueue_event_system.

    To iterate the sounds for the new system, pass the $HEVENTSYSTEM as the first parameter to $AIL_enumerate_sound_instances.

    To access or set global variables for the new system, pass the $HEVENTSYSTEM as the context in the variable access functions.

    See also the <i>eventmultiple.cpp</i> example program.
*/

DXDEC EXPAPI void AILCALL AIL_shutdown_event_system( void );
/*
  Shuts down the Miles event system.

  This function will closes everything in the event system - it ignores reference counts. It will free
  all event memory, sound banks, and samples used by the system.
*/

DXDEC EXPAPI HMSOUNDBANK AILCALL AIL_add_soundbank(char const * filename, char const* name);
/*
    Open and add a sound bank for use with the event system.

    $:filename Filename of the bank to load.
    $:name The name of the soundbank to load - this is only used for auditioning.
    $:return The handle to the newly loaded soundbank (zero on failure).

    This function opens the sound bank and makes it available to the event system.  The filename
    is the name on the media, and the name is the symbolic name you used in the Miles Sound Studio.
    You might, for example, be using a soundbank with a platform extension, like: 'gamebank_ps3.msscmp',
    and while using the name 'gamebank' for authoring and auditioning.

    Sound data is not loaded when this function is called - it is only loaded when the relevant Cache Sounds
    is played, or a sound requiring it plays.

    This function will access the disc, so you will usually call it at level load time.

    If you are using the Auditioner, $AIL_audition_startup and $AIL_audition_connect must be called prior
    to this function.
*/

DXDEC EXPAPI S32 AILCALL AIL_release_soundbank(HMSOUNDBANK bank);
/*
   Releases a sound bank from the event system.

   $:bank   The bank to close.
   $:return Returns non-zero for success (zero on failure).

   This function closes a given soundbank. Any data references in the event system need to be removed beforehand - with
   $AIL_enqueue_event_by_name usage this should only be pending sounds with completion events.

   Any other data references still existing (queued events, persisted presets, etc) will report errors when used,
   but will not crash.

   Releasing a sound bank does not free any cached sounds loaded from the bank - any sounds from the bank should be freed
   via a Purge Sounds event step. If this does not occur, the sound data will still be loaded, but the
   sound metadata will be gone, so Start Sound events will not work. Purge Sounds will still work.

   This is different from Miles 8, which would maintain a reference count for all data.
*/

DXDEC U8 const * AILCALL AIL_find_event(HMSOUNDBANK bank,char const* event_name);
/*
    (EXPAPI removed to prevent release in docs)

    Searches for an event by name in the event system.

    $:bank The soundbank to search within, or 0 to search all open banks (which is the normal case).
    $:event_name The name of the event to find.  This name should be of the form "soundbank/event_list/event_name".
    $:return A pointer to the event contents (or 0, if the event isn't found).

    This function is normally used as the event parameter for $AIL_enqueue_event. It
    searches one or all open soundbanks for a particular event name.

    <b>This is deprecated</b>. If you know the event name, you should use $AIL_enqueue_event_by_name, or $AIL_enqueue_event with
    MILESEVENT_ENQUEUE_BY_NAME.

    Events that are not enqueued by name can not be tracked by the Auditioner.
*/

DXDEC EXPAPI U64 AILCALL AIL_enqueue_event_system(HEVENTSYSTEM system, U8 const * event, void* user_buffer, S32 user_buffer_len, S32 enqueue_flags, U64 apply_to_ID );
/*
    Enqueue an event to a specific system. Used only if you have multiple event systems running.

    $:system The event system to attach the event to.
    $:return See $AIL_enqueue_event for return description.

    For full information on the parameters, see $AIL_enqueue_event.
*/

DXDEC EXPAPI U64 AILCALL AIL_enqueue_event_by_name(char const* name);
/*
    Enqueue an event by name.

    $:name The full name of the event, eg "soundbank/path/to/event".
    $:return See $AIL_enqueue_event for return description.

    This is the most basic way to enqueue an event. It enqueues an event by name, and as a result the event will be tracked by the auditioner.

    For when you need more control over the event, but still want it to be tracked by the auditioner, it is equivalent
    to calling $AIL_enqueue_event_end_named($AIL_enqueue_event_start(), name)

    For introduction to the auditioning system, see $integrating_events.
*/

DXDEC EXPAPI S32 AILCALL AIL_enqueue_event_start();
/*
    Start assembling a packet to use for enqueuing an event.

    $:return A token used for passing to functions that add data to the event.

    This is used to pass more data to an event that will be executed. For instance, if
    an event is going to spatialize a sound, but there's no need to move the sound over the course of
    its lifetime, you can add positional data to the event via $AIL_enqueue_event_position. When a
    sound is started it will use that for its initial position, and there is no need to do any
    game object <-> event id tracking.

    ${
        // Start the enqueue.
        S32 enqueue_token = AIL_enqueue_event_start();

        // Tell all sounds started by the event to position at (100, 100, 100)
        AIL_enqueue_event_position(&enqueue_token, 100, 100, 100);

        // Complete the token and enqueue the event to the command buffer.
        AIL_enqueue_event_end_named(enqueue_token);
    $}

    The enqueue process is still completely thread safe. No locks are used, however only 8
    enqueues can be "assembling" at the same time - if more than that occur, the $AIL_enqueue_event_start
    will yield the thread until a slot is open.

    The ONLY time that should happen is if events enqueues are started but never ended:

    ${
        // Start the enqueue
        S32 enqueue_token = AIL_enqueue_event_start();

        // Try to get the game position
        Vector3* position = GetPositionOfSomething(my_game_object);
        if (position == 0)
            return; // OOPS! enqueue_token was leaked here, never to be reclaimed.

    $}

    Each event has a limit to the amount of data that can be attached to it. Currently this
    amount is 512 bytes - which should cover all use cases. If any enqueue functions return 0,
    then this amount has been reached. The ErrorHandler will be called as well, with $AIL_last_error
    reporting that the enqueue buffer was filled.
*/

DXDEC EXPAPI void AILCALL AIL_enqueue_event_cancel(S32 token);
/*
    Clears a enqueue token without passing it to the command buffer

    $:token A token created with $AIL_enqueue_event_start.

    Used to handle the case where you decided to not actually enqueue the event you've assembled.

    In general it's better to handle anything that can fail before actually starting
    to create the enqueue.
*/

DXDEC EXPAPI S32 AILCALL AIL_enqueue_event_position(S32* token, F32 x, F32 y, F32 z);
/*
    Pass an initial position to an event to use for sound spatialization.

    $:token A token created with $AIL_enqueue_event_start.
    $:return 0 if the enqueue buffer is full

    If the event queued starts a sound, the sound's position will be set to the given coordinates.

    Setting the position of a sample automatically enables 3D spatialization.
*/

DXDEC EXPAPI S32 AILCALL AIL_enqueue_event_velocity(S32* token, F32 vx, F32 vy, F32 vz, F32 mag);
/*
    Pass an initial velocity to an event to use for sound spatialization.

    $:token A token created with $AIL_enqueue_event_start.
    $:return 0 if the enqueue buffer is full

    If the event queued starts a sound, the sound's velocity will be set to the given vector.

    Setting the velocity of a sample does NOT automatically enable 3D spatialization.
*/

DXDEC EXPAPI S32 AILCALL AIL_enqueue_event_buffer(S32* token, void* user_buffer, S32 user_buffer_len, S32 user_buffer_is_ptr);
/*
    Attaches a user buffer to the event.

    $:token A token created with $AIL_enqueue_event_start.
    $:user_buffer Pointer to a user buffer to pass with the event. If user_buffer_is_ptr is 1, the pointer is copied
        directly and user_buffer_len is ignored.
    $:user_buffer_len The size of the user_buffer to attach to the event.
    $:user_buffer_is_ptr If 1, the pointer is copied and user_buffer_len is ignored.
    $:return 0 if the enqueue buffer is full

    User buffers are helpful for bridging the gap between game objects and sound objects.

    There are two use cases available in this function

    $* <b>Pointer</b> If user_buffer_is_ptr is 1, then the value passed to user_buffer is copied directly as the
        user buffer contents, and then exposed during sound enumeration. This is equivalent in spirit to
        the void* value that often accompanies callbacks. In this case, user_buffer_len is ignored, as
        user_buffer is never dereferenced.
    $* <b>Buffer</b> If user_buffer_is_ptr is 0, then user_buffer_len bytes are copied from user_buffer and
        carried with the event. During sound enumeration this buffer is made available, and you never have to
        worry about memory management.
    $-

    Pointer-
    ${
        struct useful_data
        {
            S32 game_stat;
            S32 needed_info;
        };

        useful_data* data = (useful_data*)malloc(sizeof(useful_data));
        data->game_stat = 1;
        data->needed_info = 2;

        // Pointer - the "data" pointer will be copied directly, so we can't free() "data" until after the sound
        // completes and we're done using it in the enumeration loop.
        S32 ptr_token = AIL_enqueue_event_start();
        AIL_enqueue_event_buffer(&ptr_token, data, 0, 1);
        AIL_enqueue_event_end_named(ptr_token, "mybank/myevent");
    $}

    Buffer-
    ${
        struct useful_data
        {
            S32 game_stat;
            S32 needed_info;
        };

        useful_data data;
        data.game_stat = 1;
        data.needed_info = 2;

        // Buffer - the "data" structure will be copied internally, so we can free() the data - or just use
        // a stack variable like this
        S32 buf_token = AIL_enqueue_event_start();
        AIL_enqueue_event_buffer(&buf_token, &data, sizeof(data), 0);
        AIL_enqueue_event_end_named(buf_token, "mybank/myevent");
    $}

    As noted in $AIL_enqueue_event_start(), there's only 512 bytes available to an enqueue, so that
    places an upper limit on the amount of data you can pass along. If the data is huge, then you
    should use user_buffer_is_ptr.
*/

DXDEC EXPAPI S32 AILCALL AIL_enqueue_event_variablef(S32* token, char const* name, F32 value);
/*
    Attaches a variable's value to the event enqueue.

    $:token A token created with $AIL_enqueue_event_start
    $:name The variable name to set.
    $:value The value of the variable to set.
    $:return 0 if the enqueue buffer is full

    When a sound starts, the given variable will be set to the given value prior to any possible
    references being used by presets.
*/

DXDEC EXPAPI S32 AILCALL AIL_enqueue_event_filter(S32* token, U64 apply_to_ID);
/*
    Limits the effects of the event to sounds started by the given ID.

    $:token A token created with $AIL_enqueue_event_start
    $:apply_to_ID The ID to use for filtering. This can be either a sound or event ID. For an
        event, it will apply to all sounds started by the event, and any events queued by that event.
    $:return 0 if the enqueue buffer is full

    IDs are assigned to events and sounds - for events, it is returned via the $AIL_enqueue_event_end_named function
    (or any other enqueue function). For sounds, you can access the assigned id during the enumeration process.
*/

DXDEC EXPAPI S32 AILCALL AIL_enqueue_event_context(S32* token, HEVENTSYSTEM system);
/*
    Causes the event to run on a separate running event system.

    $:token A token created with $AIL_enqueue_event_start
    $:system An event system $AIL_add_event_system
    $:return 0 if the enqueue buffer is full

    If you are running multiple event systems, this is required to get events
    to queue on the additional event systems.
*/

DXDEC EXPAPI S32 AILCALL AIL_enqueue_event_selection(S32* token, U32 selection);
/*
    Passes in a selection value for start sound events to use for picking sounds.

    $:token A token created with $AIL_enqueue_event_start.
    $:selection The value to use for selecting the sound to play.
    $:return 0 if the enqueue buffer is full

    The selection index is used to programatically select a sound from the
    loaded banks. The index passed in replaces any numeric value at the end
    of the sound name existing in any start sound event step. For example, if
    a start sound event plays "mybank/sound1", and the event is queued with
    a selection, then the selection will replace the "1" with the number passed in:

    ${
        // Enqueue with a selection of 5
        S32 token = AIL_enqueue_event_start();
        AIL_enqueue_event_selection(&token, 50;
        AIL_enqueue_event_end_named(token, "mybank/myevent");
    $}

    Assuming mybank/myevent starts sound "mybank/sound1", the sound
    that will actually be played will be "mybank/sound5". If the sound does
    not exist, it is treated the same as if any other sound was not found.

    The selection process replaces ALL trailing numbers with a representation
    of the selection index using the same number of digits, meaning in the above
    example, "mybank/sound123" would have become "mybank/sound005".
*/

DXDEC EXPAPI U64 AILCALL AIL_enqueue_event_end_named(S32 token, char const* event_name);
/*
    Completes assembling the event and queues it to the command buffer to be run during next tick.

    $:token A token created with $AIL_enqueue_event_start.
    $:event_name The name of the event to run.
    $:return A unique ID for the event that can be used to identify sounds started by this event,
        or for filtering future events to the sounds started by this event.

    This function takes all of the data accumulated via the various enqueue functions and assembles
    it in to the command buffer to be run during the next $AIL_begin_event_queue_processing.

    As with all of the enqueue functions it is completely thread-safe.

    Upon completion of this function, the enqueue slot is release and available for another
    $AIL_enqueue_event_start.
*/

DXDEC EXPAPI U64 AILCALL AIL_enqueue_event(U8 const * event_or_name, void* user_buffer, S32 user_buffer_len, S32 enqueue_flags, U64 apply_to_ID );
/*
    Enqueue an event to be processed by the next $AIL_begin_event_queue_processing function.

    $:event_or_name Pointer to the event contents to queue, or the name of the event to find and queue.
            If an event, the contents must be valid until the next call to $AIL_begin_event_queue_processing.
            If a name, the string is copied internally and does not have any lifetime requirements, and MILES_ENQUEUE_BY_NAME must be present in enqueue_flags.
    $:user_buffer Pointer to a user buffer. Depending on $(AIL_enqueue_event::enqueue_flags), this pointer can be saved directly, or its contents copied into the sound instance.
            This data is then accessible later, when enumerating the instances.
    $:user_buffer_len Size of the buffer pointed to by user_buffer.
    $:enqueue_flags Optional $MILESEVENTENQUEUEFLAGS logically OR'd together that control how to enqueue this event (default is 0).
    $:apply_to_ID Optional value that is used for events that affect sound instances. Normally,
      when Miles triggers one of these event steps, it matches the name and labels stored with the event step. However, if
      you specify an apply_to_ID value, then event step will only run on sounds that matches this QueuedID,InstanceID,or EventID too.  This is how you
      execute events only specific sound instances.  QueuedIDs are returned from each call $AIL_enqueue_event.
      InstanceIDs and EventIDs are returned from $AIL_enumerate_sound_instances.
    $:return  On success, returns QueuedID value that is unique to this queued event for the rest of this
              program run (you can use this ID to uniquely identify sounds triggered from this event).

     This function enqueues an event to be triggered - this is how you begin execution of an event.  First, you
     queue it, and then later (usually once a game frame), you call $AIL_begin_event_queue_processing to
     execute an event.

     This function is very lightweight.  It does nothing more than post the event and data to a
     command buffer that gets executed via $AIL_begin_event_queue_processing.

    The user_buffer parameter can be used in different ways. If no flags are passed in, then
    Miles will copy the data from user_buffer (user_buffer_len bytes long) and store the data with
    the queued sound - you can then free the user_buffer data completely!  This lets Miles keep track
    of all your sound related memory directly and is the normal way to use the system (it is very
    convenient once you get used to it).

    If you instead pass the MILESEVENT_ENQUEUE_BUFFER_PTR flag, then user_buffer pointer will
    simply be associated with each sound that this event may start. In this case, user_buffer_len
    is ignored.

    In both cases, when you later enumerate the sound instances, you can access your sound data
    with the $(MILESEVENTSOUNDINFO::UserBuffer) field.

    You can call this function from any number threads - it's designed to be called from anywhere in your game.

    If you want events you queue to be captured by Miles Studio, then they have to be passed by name. This can be done
    by either using the convenience function $AIL_enqueue_event_by_name, or by using the MILESEVENT_ENQUEUE_BY_NAME flag and
    passing the name in event_or_name. For introduction to the auditioning system, see $integrating_events.
*/

EXPTYPEBEGIN typedef S32 MILESEVENTENQUEUEFLAGS;
#define MILESEVENT_ENQUEUE_BUFFER_PTR 0x1
#define MILESEVENT_ENQUEUE_FREE_EVENT 0x2
#define MILESEVENT_ENQUEUE_BY_NAME 0x4
// 0x8 can't be used, internal.
EXPTYPEEND
/*
    The available flags to pass in $AIL_enqueue_event or $AIL_enqueue_event_system.

    $:MILESEVENT_ENQUEUE_BUFFER_PTR The user_buffer parameter passed in should not be duplicated, and instead
        should just tranparently pass the pointer on to the event, so that the $(MILESEVENTSOUNDINFO::UserBuffer)
        during sound iteration is just the same pointer. user_buffer_len is ignored in this case.

    $:MILESEVENT_ENQUEUE_FREE_EVENT The ownership of the memory for the event is passed to the event system. If this
        is present, once the event completes $AIL_mem_free_lock will be called on the raw pointer passed in to $AIL_enqueue_event or
        $AIL_enqueue_event_system. This is rarely used.

    $:MILESEVENT_ENQUEUE_BY_NAME The event passed in is actually a string. The event system will then look for this event
        in the loaded sound banks during queue processing.
*/


DXDEC EXPAPI S32 AILCALL AIL_begin_event_queue_processing( void );
/*
    Begin execution of all of the enqueued events.

    $:return Return 0 on failure. The only failures are unrecoverable errors in the queued events
      (out of memory, bank file not found, bad data, etc).  You can get the specific error by
      calling $AIL_last_error.

      This function executes all the events currently in the queue. This is where all major
            processing takes place in the event system.

            Once you execute this functions, then sound instances will be in one of three states:

            $(MILESEVENTSOUNDSTATUS::MILESEVENT_SOUND_STATUS_PENDING)[MILESEVENT_SOUND_STATUS_PENDING] - these are new sound instances that were
            created by events that had a "Start Sound Step". Note that these instances aren't audible yet,
            so that you have a chance to modify game driven properties (like the 3D position)
            on the sound before Miles begins to play it.

            $(MILESEVENTSOUNDSTATUS::MILESEVENT_SOUND_STATUS_PLAYING)[MILESEVENT_SOUND_STATUS_PLAYING] - these are sound instances that were previously
            started and are continuing to play (you might update the 3D position for these, for example).

            $(MILESEVENTSOUNDSTATUS::MILESEVENT_SOUND_STATUS_COMPLETE)[MILESEVENT_SOUND_STATUS_COMPLETE] - these are sound instances that finished playing
            since the last this frame (you might use this status to free any game related memory, for example).

            You will normally enumerate the active sound instances in-between calls to $AIL_begin_event_queue_processing
            and $AIL_complete_event_queue_processing with $AIL_enumerate_sound_instances.

            $AIL_complete_event_queue_processing must be called after this function to commit
            all the changes.

            Example usage:
${
            // enqueue an event
            $AIL_enqueue_event( EventThatStartsSounds, game_data_ptr, 0, MILESEVENT_ENQUEUE_BUFFER_PTR, 0 );

            // now process that event
            $AIL_begin_event_queue_processing( );

            // next, enumerate the pending and complete sounds for game processing
            MILESEVENTSOUNDINFO Info;

            HMSSENUM SoundEnum = MSS_FIRST;
            while ( $AIL_enumerate_sound_instances( &SoundEnum, MILESEVENT_SOUND_STATUS_PENDING | MILESEVENT_SOUND_STATUS_COMPLETE, 0, &Info ) )
            {
                game_type * game_data = (game_type*) Info.UserBuffer; // returns the game_data pointer from the enqueue

                if ( Info.Status == MILESEVENT_SOUND_STATUS_PENDING )
                {
                    // setup initial state
                    AIL_set_sample_3D_position( Info.Sample, game_data->x, game_data->y, game_data->z );
                }
                else if ( Info.Status == MILESEVENT_SOUND_STATUS_COMPLETE )
                {
                    // Free some state we have associated with the sound now that its done.
                    game_free( game_data );
                }
            }

            $AIL_complete_event_queue_processing( );
  $}

            Note that if any event step drastically fails, the rest of the command queue is
            skipped, and this function returns 0! For this reason, you shouldn't assume
            that a start sound event will always result in a completed sound later.

            Therefore, you should allocate memory that you want associated with a sound instance
            during the enumeration loop, rather than at enqueue time.  Otherwise, you
            need to detect that the sound didn't start and then free the memory (which can be complicated).
*/

// Returned by AIL_enumerate_sound_instances()
EXPTYPE typedef struct _MILESEVENTSOUNDINFO
{
    U64 QueuedID;
    U64 InstanceID;
    U64 EventID;
    HSAMPLE Sample;
    HSTREAM Stream;
    void* UserBuffer;
    S32 UserBufferLen;
    S32 Status;
    U32 Flags;
    S32 UsedDelay;
    F32 UsedVolume;
    F32 UsedPitch;
    char const* UsedSound;
    S32 HasCompletionEvent;
} MILESEVENTSOUNDINFO;
/*
  Sound instance data that is associated with each active sound instance.

  $:QueuedID A unique ID that identifies the queued event that started this sound. Returned from each call to $AIL_enqueue_event.
  $:EventID A unique ID that identifies the actual event that started this sound. This is the same as QueuedID unless the sound
    was started by a completion event or a event exec step. In that case, the QueuedID represents the ID returned from
    $AIL_enqueue_event, and EventID represents the completion event.
  $:InstanceID A unique ID that identified this specific sound instance (note that one QueuedID can trigger multiple InstanceIDs).
  $:Sample The $HSAMPLE for this playing sound.
  $:Stream The $HSTREAM for this playing sound (if it is being streamed, zero otherwise).
  $:UserBuffer A pointer to the user data for this sound instance.
  $:UserBufferLen The length in bytes of the user data (if known by Miles).
  $:Status One of the $MILESEVENTSOUNDSTATUS status values.
  $:Flags One or more of the $MILESEVENTSOUNDFLAG flags.
  $:UsedDelay The value actually used as a result of the randomization of delay for this instance
  $:UsedVolume The value actually used as a result of the randomization of pitch for this instance
  $:UsedPitch The value actually used as a result of the randomization of volume for this instance
  $:UsedSound The name of the sound used as a result of randomization. This pointer should NOT be deleted
    and is only valid for the until the next call in to Miles.
  $:HasCompletionEvent Nonzero if the sound will fire an event upon completion.

  This structure is returned by the $AIL_enumerate_sound_instances function. It
  returns information about an active sound instance.
*/

DXDEC EXPAPI void AILCALL AIL_set_variable_int(UINTa context, char const* name, S32 value);
/*
    Sets a named variable that the designer can reference in the tool.

    $:context The context the variable is set for. Can be either a $HEVENTSYSTEM
            to set a global variable for a specific system, 0 to set a global variable
            for the default system, or an $HMSSENUM from $AIL_enumerate_sound_instances.
    $:name The name of the variable to set.
    $:value The value of the variable to set.

    Variables are tracked per sound instance and globally, and when a variable is needed
    by an event, it will check the relevant sound instance first, before falling back to
    the global variable list:

    ${
        $HMSSENUM FirstSound = MSS_FIRST;
        $MILESEVENTSOUNDINFO Info;

        // Grab the first sound, whatever it is.
        $AIL_enumerate_sound_instances(0, &FirstSound, 0, 0, 0, &Info);

        // Set a variable on that sound.
        $AIL_set_variable_int(FirstSound, "MyVar", 10);

        // Set a global variable by the same name.
        $AIL_set_variable_int(0, "MyVar", 20);

        // A preset referencing "MyVar" for FirstSound will get 10. Any other sound will
        // get 20.
    $}

*/

DXDEC EXPAPI void AILCALL AIL_set_variable_float(UINTa context, char const* name, F32 value);
/*
    Sets a named variable that the designer can reference in the tool.

    $:context The context the variable is set for. Can be either a $HEVENTSYSTEM
            to set a global variable for a specific system, 0 to set a global variable
            for the default system, or an $HMSSENUM from $AIL_enumerate_sound_instances.
    $:name The name of the variable to set.
    $:value The value of the variable to set.

    Variables are tracked per sound instance and globally, and when a variable is needed
    by an event, it will check the relevant sound instance first, before falling back to
    the global variable list.

    ${
        $HMSSENUM FirstSound = MSS_FIRST;
        $MILESEVENTSOUNDINFO Info;

        // Grab the first sound, whatever it is.
        $AIL_enumerate_sound_instances(0, &FirstSound, 0, 0, 0, &Info);

        // Set a variable on that sound.
        $AIL_set_variable_float(FirstSound, "MyVar", 10.0);

        // Set a global variable by the same name.
        $AIL_set_variable_float(0, "MyVar", 20.0);

        // A preset referencing "MyVar" for FirstSound will get 10. Any other sound will
        // get 20.
    $}
*/

DXDEC EXPAPI S32 AILCALL AIL_variable_int(UINTa context, char const* name, S32* value);
/*
    Retrieves a named variable.

    $:context The context to start the lookup at, same as $AIL_set_variable_int.
    $:name The name to look up.
    $:value Pointer to an int to store the value in.
    $:return 1 if the variable was found, 0 otherwise.

    This function follows the same lookup pattern as the runtime - if the context is a
    sound instance, it checks the instance before falling back to global variables.
*/

DXDEC EXPAPI S32 AILCALL AIL_variable_float(UINTa context, char const* name, F32* value);
/*
    Retrieves a named variable.

    $:context The context to start the lookup at, same as $AIL_set_variable_float.
    $:name The name to look up.
    $:value Pointer to a float to store the value in.
    $:return 1 if the variable was found, 0 otherwise.

    This function follows the same lookup pattern as the runtime - if the context is a
    sound instance, it checks the instance before falling back to global variables.
*/

DXDEC EXPAPI void AILCALL AIL_requeue_failed_asyncs();
/*
    Requeues any failed asynchronous loads for sound sources.

    Use this function when a disc error causes a slew of failed caches. Any sound source that
    has failed due to asynchronous load will get retried.
*/

DXDEC EXPAPI void AILCALL AIL_set_sound_start_offset(HMSSENUM sound, S32 offset, S32 isms);
/*
    Specify the starting position for a pending sound.

    $:sound The enumeration from $AIL_enumerate_sound_instances representing the desired sound.
            The sound must be in the pending state.
    $:offset The offset to use for the starting position of the sound.
    $:isms If nonzero, the offset is in milliseconds, otherwise bytes.

    Use this function instead of manipulating the sample position directly via low level Miles calls prior to
    the sound starting. Generally you don't need to do this manually, since the sound designer should do
    this, however if you need to restart a sound that stopped - for example a stream that went to error -
    you will have to set the start position via code.

    However, since there can be a delay between the time the sound is first seen in the sound iteration and
    the time it gets set to the data, start positions set via the low level miles calls can get lost, so
    use this.

    See the <i>eventstreamerror.cpp</i> example program for usage.
*/

DXDEC EXPAPI S32 AILCALL AIL_enumerate_sound_instances(HEVENTSYSTEM system, HMSSENUM* next, S32 statuses, char const* label_query, U64 search_for_ID, EXPOUT MILESEVENTSOUNDINFO* info);
/*
    Enumerated the active sound instances managed by the event system.

    $:next Enumeration token - initialize to MSS_FIRST before the first call. You can pass 0 here, if you just want the first instance that matches.
    $:statuses Or-ed list of status values to enumerate. Use 0 for all status types.
    $:label_query A query to match sound instance labels against.  Use 0 to skip label matching.
    $:search_for_ID Match only instances that have a QueuedID,InstanceID,or EventID that matches this value. Use 0 to skip ID matching.
    $:info Returns the data for each sound instance.
    $:return  Returns 0 when enumeration is complete.

    Enumerates the sound instances. This will generally be used between
      calls to $AIL_begin_event_queue_processing and $AIL_complete_event_queue_processing to
      manage the sound instances.

    The label_query is a list of labels to match, separated by commas.  By default, comma-separated
      values only have to match at least one label.  So, if you used "level1, wind", then all sound instances
      that had either "level1" <i>or</i> "wind" would match.  If you want to match <i>all</i> labels,
      then use the + sign first (for example, "+level1, +wind" would only match sound instances that
      had <i>both</i> "level1" and "wind").  You can also use the - sign before a label to <i>not</i>
      match that label (so, "level1, -wind" would match all "level1" labeled sound instances that didn't have
      a "wind" label).  Finally, you can also use * and ? to match wildcard style labels (so, "gun*"
      would match any sound instance with a label that starts with "gun").

      Valid status flags are:

            $(MILESEVENTSOUNDSTATUS::MILESEVENT_SOUND_STATUS_PENDING)[MILESEVENT_SOUND_STATUS_PENDING] - these are new sound instances that were
            created by events that had a "Start Sound Step". Note that these instances aren't audible yet,
            so that you have a chance to modify game driven properties (like the 3D position)
            on the sound before Miles begins to play it.

            $(MILESEVENTSOUNDSTATUS::MILESEVENT_SOUND_STATUS_PLAYING)[MILESEVENT_SOUND_STATUS_PLAYING] - these are sound instances that were previously
            started and are continuing to play (you might update the 3D position for these, for example).

            $(MILESEVENTSOUNDSTATUS::MILESEVENT_SOUND_STATUS_COMPLETE)[MILESEVENT_SOUND_STATUS_COMPLETE] - these are sound instances that finished playing
            since the last this frame (you might use this status to free any game related memory, for example).

            Example Usage:
${
            HMSSENUM SoundEnum = MSS_FIRST;
            MILESEVENTSOUNDINFO Info;

            while ( $AIL_enumerate_sound_instances( &SoundEnum, 0, 0, &Info ) )
            {
              if ( Info.Status != MILESEVENT_SOUND_STATUS_COMPLETE )
              {
                game_SoundState* game_data= (game_SoundState*)( Info.UserBuffer );
                $AIL_set_sample_is_3D( Info.Sample, 1 );
                $AIL_set_sample_3D_position( Info.Sample, game_data->x, game_data->y, game_date->z );
              }
            }

$}
*/

EXPTYPEBEGIN typedef S32 MILESEVENTSOUNDSTATUS;
#define MILESEVENT_SOUND_STATUS_PENDING 0x1
#define MILESEVENT_SOUND_STATUS_PLAYING 0x2
#define MILESEVENT_SOUND_STATUS_COMPLETE 0x4
EXPTYPEEND
/*
  Specifies the status of a sound instance.

  $:MILESEVENT_SOUND_STATUS_PENDING New sound instances that were
            created by events that had a "Start Sound Step". Note that these instances aren't audible yet,
            so that you have a chance to modify game driven properties (like the 3D position)
            on the sound before Miles begins to play it.

  $:MILESEVENT_SOUND_STATUS_PLAYING Sound instances that were previously
            started and are continuing to play (you might update the 3D position for these, for example).

  $:MILESEVENT_SOUND_STATUS_COMPLETE Sound instances that finished playing
            since the last this frame (you might use this status to free any game related memory, for example).

  These are the status values that each sound instance can have.  Use $AIL_enumerate_sound_instances to retrieve them.
*/

EXPTYPEBEGIN typedef U32 MILESEVENTSOUNDFLAG;
#define MILESEVENT_SOUND_FLAG_MISSING_SOUND     0x1
#define MILESEVENT_SOUND_FLAG_EVICTED           0x2
#define MILESEVENT_SOUND_FLAG_WAITING_ASYNC     0x4
#define MILESEVENT_SOUND_FLAG_PENDING_ASYNC     0x8
#define MILESEVENT_SOUND_FLAG_FAILED_HITCH      0x10
#define MILESEVENT_SOUND_FLAG_FAILED_ASYNC      0x20
EXPTYPEEND
/*
  Specifies the status of a sound instance.

  $:MILESEVENT_SOUND_FLAG_MISSING_SOUND The event system tried to look up the sound requested from a Start Sound event
    and couldn't find anything in the loaded banks.
  $:MILESEVENT_SOUND_FLAG_EVICTED The sound was evicted due to a sound instance limit being hit. Another sound was selected
    as being higher priority, and this sound was stopped as a result. This can be the result of either a Label Sound Limit,
    or a limit on the sound itself.
  $:MILESEVENT_SOUND_FLAG_WAITING_ASYNC The sound is pending because the data for it is currently being loaded.
    The sound will start when sufficient data has been loaded to hopefully avoid a skip.
  $:MILESEVENT_SONUD_FLAG_PENDING_ASYNC The sound has started playing, but the data still isn't completely loaded, and it's possible
    that the sound playback will catch up to the read position under poor I/O conditions.
  $:MILESEVENT_SOUND_FLAG_FAILED_HITCH The sound meta data was found, but the sound was not in memory, and the Start Sound event
    was marked as "Must Be Cached". To prevent this, either clear the flag in the event, which will cause a start delay as the
    sound data is asynchronously loaded, or specify the sound in a Cache Sounds step prior to attempting to start it.
  $:MILESEVENT_SOUND_FLAG_FAILED_ASYNC The sound tried to load and the asynchronous I/O operation failed - most likely either the media
    was removed during load, or the file was not found.

  These are the flag values that each sound instance can have.  Use $AIL_enumerate_sound_instances to retrieve them. Instances
  may have more than one flag, logically 'or'ed together.
*/

DXDEC EXPAPI S32 AILCALL AIL_complete_event_queue_processing( void );
/*
  Completes the queue processing (which is started with $AIL_begin_event_queue_processing ).

  $:return Returns 0 on failure.

  This function must be called as a pair with $AIL_begin_event_queue_processing.

  In $AIL_begin_event_queue_processing, all the new sound instances are queued up, but they haven't
  started playing yet.  Old sound instances that have finished playing are still valid - they
  haven't been freed yet.  $AIL_complete_event_queue_processing actually starts the sound instances
  and frees the completed ones - it's the 2nd half of the event processing.

  Usually you call $AIL_enumerate_sound_instances before this function to manage all the sound
  instances.
*/

DXDEC EXPAPI U64 AILCALL AIL_stop_sound_instances(char const * label_query, U64 apply_to_ID);
/*
    Allows the programmer to manually enqueue a stop sound event into the event system.

    $:label_query A query to match sound instance labels against.  Use 0 to skip label matching.
    $:apply_to_ID An optional value returned from a previous $AIL_enqueue_event or $AIL_enumerate_sound_instances that
      tells Miles to stop only those instances who's QueuedID,InstanceID,or EventID matches this value.
    $:return Returns a non-zero queue ID on success.

    Enqueues an event to stop all sounds matching the specified label query (see $AIL_enumerate_sound_instances
    for a description of the label_query format).

    Usually the programmer should trigger a named event that the sound designed can fill out to stop the necessary sounds,
    however, if a single sound (for example associated with an enemy that the player just killed) needs to be stopped,
    this function accomplishes that, and is captured by the auditioner for replay.
*/

DXDEC EXPAPI U64 AILCALL AIL_pause_sound_instances(char const * label_query, U64 apply_to_ID);
/*
    Allows the programmer to manually enqueue a pause sound event into the event system.

    $:label_query A query to match sound instance labels against.  Use 0 to skip label matching.
    $:apply_to_ID An optional value returned from a previous $AIL_enqueue_event or $AIL_enumerate_sound_instances that
      tells Miles to pause only those instances who's QueuedID,InstanceID,or EventID matches this value.
    $:return Returns a non-zero queue ID on success.

    Enqueues an event to pause all sounds matching the specified label query (see $AIL_enumerate_sound_instances
    for a description of the label_query format).

    Usually the programmer should trigger a named event that the sound designed can fill out to pause the necessary sounds,
    however, if a single sound (for example associated with an enemy that has been put in to stasis) needs to be paused,
    this function accomplishes that, and is captured by the auditioner for replay.
*/

DXDEC EXPAPI U64 AILCALL AIL_resume_sound_instances(char const * label_query, U64 apply_to_ID);
/*
    Allows the programmer to manually enqueue a resume sound event into the event system.

    $:label_query A query to match sound instance labels against.  Use 0 to skip label matching.
    $:apply_to_ID An optional value returned from a previous $AIL_enqueue_event or $AIL_enumerate_sound_instances that
      tells Miles to resume only those instances who's QueuedID,InstanceID,or EventID matches this value.
    $:return Returns a non-zero enqueue ID on success.

    Enqueues an event to resume all sounds matching the specified label query (see $AIL_enumerate_sound_instances
    for a description of the label_query format).

    Usually the programmer should trigger a named event that the sound designed can fill out to resume the necessary sounds,
    however, if a single sound (for example associated with an enemy that has been restored from stasis) needs to be resumed,
    this function accomplishes that, and is captured by the auditioner for replay.
*/

DXDEC EXPAPI U64 AILCALL AIL_start_sound_instance(HMSOUNDBANK bank, char const * sound, U8 loop_count,
    S32 should_stream, char const * labels, void* user_buffer, S32 user_buffer_len, S32 enqueue_flags );
/*
    Allows the programmer to manually enqueue a start sound event into the event system.

    $:bank The bank containing the sound to start.
    $:sound The name of the sound file to start, including bank name, e.g. "BankName/SoundName"
    $:loop_count The loop count to assign to the sound. 0 for infinite, 1 for play once, or just the number of times to loop.
    $:stream Non-zero if the sound playback should stream off the disc.
    $:labels An optional comma-delimited list of labels to assign to the sound playback.
    $:user_buffer See the user_buffer description in $AIL_enqueue_event.
    $:user_buffer_len See the user_buffer_len description in $AIL_enqueue_event.
    $:enqueue_flags See the enqueue_flags description in $AIL_enqueue_event.
    $:return Returns a non-zero EnqueueID on success.

    Enqueues an event to start the specified sound asset.

    Usually the programmer should trigger an event that the sound designer has specifically
    create to start the appropriate sounds, but this function gives the programmer
    manual control, if necessary. <b>This function is not captured by the auditioner.</b>
*/

DXDEC EXPAPI void AILCALL AIL_clear_event_queue( void );
/*
   Removes all pending events that you have enqueued.

   This function will clears the list of all events that you have previously enqueued.
*/


DXDEC EXPAPI S32 AILCALL AIL_set_sound_label_limits(HEVENTSYSTEM system, char const* sound_limits);
/*
  Sets the maximum number of sounds that matches a particular label.

  $:sound_limits A string that defines one or more limits on a label by label basis.  The string should
    be of the form "label1name label1count:label2name label2count".
  $:return Returns 0 on failure (usually a bad limit string).

  Every time an event triggers a sound to be played, the sound limits are checked, and, if exceeded, a sound is dropped (based
  on the settings in the event step).

  Usually event limits are set by a sound designer via an event, but this lets the programmer override the limits at runtime.
  Note that this replaces those events, it does not supplement.
*/

DXDEC EXPAPI S32 AILCALL AIL_enumerate_preset_persists(HEVENTSYSTEM system, HMSSENUM* next, EXPOUT char const ** name);
/*
    Enumerates the current persisted presets that active in the system.

    $:system The system to enumerate the persists for, or 0 to use the default system.
    $:next Enumeration token - initialize to MSS_FIRST before the first call.
    $:name Pointer to a char* that receives the name of the persist. NOTE
            that this pointer can change frame to frame and should be immediately copied to a client-allocated
            buffer if persistence is desired.
    $:return Returns 0 when enumeration is complete.

    This function lets you enumerate all the persisting presets that are currently active in the system.  It
    is mostly a debugging aid.
*/

DXDEC EXPAPI char * AILCALL AIL_text_dump_event_system(void);
/*
    Returns a big string describing the current state of the event system.

    $:return String description of current systems state.

    This function is a debugging aid - it can be used to show all of the active allocations,
    active sounds, etc.

    You must delete the pointer returned from this function with $AIL_mem_free_lock.
*/

EXPTYPE typedef struct _MILESEVENTSTATE
{
    S32 CommandBufferSize;
    S32 HeapSize;
    S32 HeapRemaining;
    S32 LoadedSoundCount;
    S32 PlayingSoundCount;
    S32 LoadedBankCount;
    S32 PersistCount;

    S32 SoundBankManagementMemory;
    S32 SoundDataMemory;
} MILESEVENTSTATE;
/*
  returns the current state of the Miles Event System.

  $:CommandBufferSize The size of the command buffer in bytes. See also the $AIL_startup_event_system.
  $:HeapSize The total size of memory used by the event system for management structures, and is allocated during startup. This does not include loaded file sizes.
  $:HeapRemaining The number of bytes in HeapSize that is remaining.
  $:LoadedSoundCount The number of sounds loaded and ready to play via cache event steps.
  $:PlayingSoundCount The number of sounds currently playing via start sound event steps.
  $:LoadedBankCount The number of sound banks loaded in the system via cache event steps, or AIL_add_soundbank.
  $:PersistCount The number of presets persisted via the persist event step.
  $:SoundBankManagementMemory The number of bytes used for the management of the loaded sound banks.
  $:SoundDataMemory The number of bytes used in file sizes - remember this is not included in HeapSize. Streaming overhead is not included in this number, only fully loaded sounds.

  This structure returns debugging info about the event system. It is used with $AIL_event_system_state.
*/

EXPGROUP(Miles High Level Callbacks)

EXPAPI typedef void AILCALLBACK MilesBankFreeAll( void );
/*
  callback to free all user managed bank memory.
*/

EXPAPI typedef void * AILCALLBACK MilesBankGetPreset( char const * name );
/*
  callback to retrieve a sound preset.
*/

EXPAPI typedef void * AILCALLBACK MilesBankGetEnvironment( char const * name );
/*
  callback to retrieve an environment preset.
*/
EXPAPI typedef S32 AILCALLBACK MilesBankGetSound(char const* SoundAssetName, char* SoundFileName, MILESBANKSOUNDINFO* o_SoundInfo );
/*
  callback to return whether the sound asset is in the bank, and, if so, what the final data filename is.

  In order to externally deploy sound files, you will need to register your own GetSound callback. This is detailed in the
  eventexternal example program.

  This returns the len of the buffer required for the output file name if SoundFileName is zero.
*/

EXPAPI typedef void * AILCALLBACK MilesBankGetEvent( char const * name );
/*
  callback to retrieve an event.
*/

EXPAPI typedef void * AILCALLBACK MilesBankGetMarkerList( char const * name );
/*
  callback to retrieve a sound marker list.
*/

EXPAPI typedef S32 AILCALLBACK MilesBankGetLoadedCount( void );
/*
  callback to retrieve the number of loaded sound banks.
*/

EXPAPI typedef S32 AILCALLBACK MilesBankGetMemUsage( void );
/*
  callback to retrieve the total memory in use.
*/

EXPAPI typedef char const * AILCALLBACK MilesBankGetLoadedName( S32 index );
/*
  callback to retrieve the file name of a sound index.
*/


EXPTYPE typedef struct _MILESBANKFUNCTIONS
{
  MilesBankFreeAll * FreeAll;
  MilesBankGetPreset * GetPreset;
  MilesBankGetEnvironment * GetEnvironment;
  MilesBankGetSound * GetSound;
  MilesBankGetEvent * GetEvent;
  MilesBankGetMarkerList * GetMarkerList;
  MilesBankGetLoadedCount * GetLoadedCount;
  MilesBankGetMemUsage * GetMemUsage;
  MilesBankGetLoadedName * GetLoadedName;
} MILESBANKFUNCTIONS;
/*
  specifies callbacks for each of the Miles event system.

  $:FreeAll Callback that tells you to free all user-side bank memory.
  $:GetPreset Callback to retrieve a sound preset.
  $:GetEnvironment Callback to retrieve an environment preset.
  $:GetSound Callback to return the actual filename of a sound asset.
  $:GetEvent Callback to retrieve a sound event.
  $:GetMarkerList Callback to retrieve a sound marker list.
  $:GetLoadedCount Callback to retrieve a count of loaded sound banks.
  $:GetMemUsage Callback to retrieve the amount of memory in use.
  $:GetLoadedName Callback to retrieve the filename for a sound asset index.

  This structure is used to provide overrides for all of the high-level loading
  functionality.
*/

EXPGROUP(Miles High Level Event System)

DXDEC EXPAPI void AILCALL AIL_set_event_sample_functions(HSAMPLE (*CreateSampleCallback)(char const* SoundName, char const* SoundFileName, HDIGDRIVER dig, void* UserBuffer, S32 UserBufferLen), void (*ReleaseSampleCallback)(HSAMPLE));
/*
    Allows you to manage sound data availability and sample handles.

    $:CreateSampleCallback Function that will be called when a sample handle is needed.
    $:ReleaseSampleCallback Function that will be called when a sample is no longer needed.

    A created sample is required to have all data pointers necessary to play - e.g.
    the event system needs to be able to just do a AIL_start_sample() on the returned
    handle and have it work.

    In the callback, SoundName is the name of the asset in Miles Studio, and SoundFileName
    is the value returned from Container_GetSound() (see also $AIL_set_event_bank_functions).

*/

DXDEC EXPAPI void AILCALL AIL_set_event_bank_functions(MILESBANKFUNCTIONS const * Functions);
/*
  Allows you to override the internal bank file resource management..

  $:Functions A pointer to a structure containing all the callback functions.

  This function is used to completely override the high-level resource management system.
  It's not for overriding the IO - it's when you need much higher-level of control. Primarily
  targeted internally for the Auditioner to use, it also is used when deploying sound files
  externally.
*/

DXDEC EXPAPI MILESBANKFUNCTIONS const* AILCALL AIL_get_event_bank_functions();
/*
    Returns the current functions used to retrieve and poll bank assets.
*/


typedef S32     AILCALLBACK AuditionStatus();
typedef S32     AILCALLBACK AuditionPump();
typedef void*   AILCALLBACK AuditionOpenBank(char const* i_FileName);
typedef S32     AILCALLBACK AuditionOpenComplete(void* i_Bank);
typedef void    AILCALLBACK AuditionCloseBank(void* i_Bank);

typedef void    AILCALLBACK AuditionSuppress(S32 i_IsSuppressed);
typedef void    AILCALLBACK AuditionFrameStart();
typedef void    AILCALLBACK AuditionFrameEnd();
typedef void    AILCALLBACK AuditionDefragStart();
typedef void    AILCALLBACK AuditionSetBlend(U64 i_EventId, char const* i_Name);
typedef void    AILCALLBACK AuditionSetPersist(U64 i_EventId, char const* i_Name, char const* i_Preset);
typedef void    AILCALLBACK AuditionEvent(char const* i_EventName, U64 i_EventId, U64 i_Filter, S32 i_Exists, void* i_InitBlock, S32 i_InitBlockLen);
typedef void    AILCALLBACK AuditionSound(U64 i_EventId, U64 i_SoundId, char const* i_Sound, char const* i_Labels, float i_Volume, S32 i_Delay, float i_Pitch);
typedef void    AILCALLBACK AuditionSoundComplete(U64 i_SoundId);
typedef void    AILCALLBACK AuditionSoundPlaying(U64 i_SoundId);
typedef void    AILCALLBACK AuditionSoundFlags(U64 i_SoundId, S32 i_Flags);
typedef void    AILCALLBACK AuditionSoundLimited(U64 i_SoundId, char const* i_Label);
typedef void    AILCALLBACK AuditionSoundEvicted(U64 i_SoundId, U64 i_ForSound, S32 i_Reason);
typedef void    AILCALLBACK AuditionControl(U64 i_EventId, char const* i_Labels, U8 i_ControlType, U64 i_Filter);
typedef void    AILCALLBACK AuditionSoundBus(U64 i_SoundId, U8 i_BusIndex);

typedef void    AILCALLBACK AuditionError(U64 i_Id, char const* i_Details);

typedef void    AILCALLBACK AuditionAsyncQueued(U64 i_RelevantId, S32 i_AsyncId, char const* i_Asset);
typedef void    AILCALLBACK AuditionAsyncLoad(S32 i_AsyncId, S32 i_ExpectedData);
typedef void    AILCALLBACK AuditionAsyncError(S32 i_AsyncId);
typedef void    AILCALLBACK AuditionAsyncComplete(S32 i_AsyncId, S32 i_DataLoaded);
typedef void    AILCALLBACK AuditionAsyncCancel(S32 i_AsyncId);
typedef void    AILCALLBACK AuditionListenerPosition(float x, float y, float z);
typedef void    AILCALLBACK AuditionSoundPosition(U64 i_Sound, float x, float y, float z);
typedef void    AILCALLBACK AuditionSendCPU(HDIGDRIVER i_Driver);
typedef void    AILCALLBACK AuditionUpdateDataCount(S32 i_CurrentDataLoaded);
typedef void    AILCALLBACK AuditionSendCount(S32 i_Count);
typedef void    AILCALLBACK AuditionHandleSystemLoad(S32 i_Avail, S32 i_Total);
typedef void    AILCALLBACK AuditionVarState(char const* i_Var, U64 i_SoundId, S32 i_Int, void* i_4ByteValue);
typedef void    AILCALLBACK AuditionRampState(char const* i_Ramp, U64 i_SoundId, S32 i_Type, float i_Current);
typedef void    AILCALLBACK AuditionSoundState(U64 i_SoundId, float i_FinalVol, float i_3DVol, float i_BlendVol, float i_BlendPitch, float i_RampVol, float i_RampWet, float i_RampLp, float i_RampRate);

typedef void    AILCALLBACK AuditionClearState();
typedef void    AILCALLBACK AuditionCompletionEvent(U64 i_CompletionEventId, U64 i_ParentSoundId);
typedef void    AILCALLBACK AuditionAddRamp(U64 i_ParentSoundId, S32 i_Type, char const* i_Name, char const* i_Query, U64 i_EventId);

typedef struct _MILESAUDITIONFUNCTIONS
{
    AuditionStatus* Status;
    AuditionPump* Pump;
    AuditionOpenBank* OpenBank;
    AuditionOpenComplete* OpenComplete;
    AuditionCloseBank* CloseBank;

    AuditionSuppress* Suppress;
    AuditionFrameStart* FrameStart;
    AuditionFrameEnd* FrameEnd;
    AuditionDefragStart* DefragStart;
    AuditionSetBlend* SetBlend;
    AuditionSetPersist* SetPersist;
    AuditionEvent* Event;
    AuditionSound* Sound;
    AuditionSoundComplete* SoundComplete;
    AuditionSoundPlaying* SoundPlaying;
    AuditionSoundFlags* SoundFlags;
    AuditionSoundLimited* SoundLimited;
    AuditionSoundEvicted* SoundEvicted;
    AuditionControl* Control;
    AuditionSoundBus* SoundBus;

    AuditionError* Error;

    AuditionAsyncQueued* AsyncQueued;
    AuditionAsyncLoad* AsyncLoad;
    AuditionAsyncError* AsyncError;
    AuditionAsyncComplete* AsyncComplete;
    AuditionAsyncCancel* AsyncCancel;
    AuditionListenerPosition* ListenerPosition;
    AuditionSoundPosition* SoundPosition;
    AuditionSendCPU* SendCPU;
    AuditionSendCount* SendCount;
    AuditionUpdateDataCount* UpdateDataCount;
    AuditionHandleSystemLoad* HandleSystemLoad;
    AuditionVarState* VarState;
    AuditionRampState* RampState;
    AuditionSoundState* SoundState;

    AuditionClearState* ClearState;
    AuditionCompletionEvent* CompletionEvent;
    AuditionAddRamp* AddRamp;
} MILESAUDITIONFUNCTIONS;

DXDEC void AILCALL MilesEventSetAuditionFunctions(MILESAUDITIONFUNCTIONS const* i_Functions);

// Auditioner lib functions.
EXPGROUP(auditioning)

EXPTYPEBEGIN typedef S32 MILESAUDITIONCONNECTRESULT;
#define MILES_CONNECTED         0
#define MILES_CONNECT_FAILED    1
#define MILES_HOST_NOT_FOUND    2
#define MILES_SERVER_ERROR      3
EXPTYPEEND
/*
    Return values for $AIL_audition_connect.

    $:MILES_CONNECTED The Auditioner connected and successfully executed the handshake.
    $:MILES_CONNECT_FAILED The Auditioner couldn't connect - either the IP wasn't valid, or Miles Sound Studio wasn't accepting connections.
    $:MILES_HOST_NOT_FOUND The given host name could not be resolved to an IP.
    $:MILES_SERVER_ERROR We connected, but the server was either another app on the same port, or the server version was incorrect.
*/

DXDEC EXPAPI S32 AILCALL AIL_audition_connect(char const* i_Address);
/*
    Connect to a currently running Miles Sound Studio.

    $:i_Address The IP or host name of the computer running Miles Sound Studio. Use $AIL_audition_local_host to connect to the same machine as the runtime.
    $:return One of $MILESAUDITIONCONNECTRESULT

    The is a synchronous connection attempt to Miles Sound Studio - it will not return until it is happy with the connection
    and the server, or a failure occurs.

    This must be called before any $AIL_add_soundbank calls.
*/

DXDEC EXPAPI char const* AILCALL AIL_audition_local_host();
/*
    Return the host name of the local machine.
*/

// Defines - must match values in studio/Common.h
EXPTYPEBEGIN typedef S32 MILESAUDITIONLANG;
#define MILES_LANG_ENGLISH      1
#define MILES_LANG_FRENCH       2
#define MILES_LANG_GERMAN       3
#define MILES_LANG_SPANISH      4
#define MILES_LANG_ITALIAN      5
#define MILES_LANG_JAPANESE     6
#define MILES_LANG_KOREAN       7
#define MILES_LANG_CHINESE      8
#define MILES_LANG_RUSSIAN      9
EXPTYPEEND
/*
    Values representing the various languages the high level tool allows.

    $:MILES_LANG_ENGLISH English
    $:MILES_LANG_FRENCH French
    $:MILES_LANG_GERMAN German
    $:MILES_LANG_SPANISH Spanish
    $:MILES_LANG_ITALIAN Italian
    $:MILES_LANG_JAPANESE Japanese
    $:MILES_LANG_KOREAN Korean
    $:MILES_LANG_CHINESE Chinese
    $:MILES_LANG_RUSSIAN Russian

    Values representing the various languages the high level tool allows.
*/

EXPTYPEBEGIN typedef S32 MILESAUDITIONPLAT;
#define MILES_PLAT_WIN          1
#define MILES_PLAT_MAC          2
#define MILES_PLAT_PS3          3
#define MILES_PLAT_360          4
#define MILES_PLAT_3DS          5
#define MILES_PLAT_PSP          6
#define MILES_PLAT_IPHONE       7
#define MILES_PLAT_LINUX        8
#define MILES_PLAT_WII          9
#define MILES_PLAT_PSP2         10
#define MILES_PLAT_WIIU         11
#define MILES_PLAT_SEKRIT       12
#define MILES_PLAT_SEKRIT2      13
#define MILES_PLAT_WIN64        14
#define MILES_PLAT_LINUX64      15
#define MILES_PLAT_MAC64        16
#define MILES_PLAT_WINRT32      17
#define MILES_PLAT_WINRT64      18
#define MILES_PLAT_WINPH32      19
#define MILES_PLAT_ANDROID      20

EXPTYPEEND
/*
    Values representing the various platforms the high level tool allows.

    $:MILES_PLAT_WIN Microsoft Win32/64
    $:MILES_PLAT_MAC Apple OSX
    $:MILES_PLAT_PS3 Sony PS3
    $:MILES_PLAT_360 Microsoft XBox360
    $:MILES_PLAT_3DS Nintendo 3DS
    $:MILES_PLAT_PSP Sony PSP
    $:MILES_PLAT_IPHONE Apple iDevices
    $:MILES_PLAT_LINUX Linux Flavors
    $:MILES_PLAT_WII Nintendo Wii
    $:MILES_PLAT_PSP2 Sony NGP

    Values representing the various platforms the high level tool allows.
*/

DXDEC EXPAPI S32 AILCALL AIL_audition_startup(S32 i_ProfileOnly, S32 i_Language, S32 i_Platform);
/*
    Binds the Auditioner to the Miles Event Runtime.

    $:i_ProfileOnly Specify 0 to use assets from the connected Miles Sound Studio, and 1 to use assets from disc.
    $:i_Language One of $MILESAUDITIONLANG, or zero to use Default assets. See comments below.
    $:i_Platform One of $MILESAUDITIONPLAT, or zero to use the current platform. See comments below.

    The Auditioner can run in one of two modes - the first is standard mode, where all assets
    are loaded from the server, and profiling data is sent back to the server. The second is
    Profiling mode, where the assets are loaded exactly as they would be under normal execution,
    but all of the profiling data is sent to the server.

    The $(AIL_audition_startup::i_Language) and the $(AIL_audition_startup::i_Platform) are used to determine what assets Miles Sound Studio sends
    the Auditioner, and as a result are not used in Profiling Mode. Otherwise these are equivalent to
    the options selected for compiling banks.

    This must be called before any $AIL_add_soundbank calls.
*/

DXDEC EXPAPI void AILCALL AIL_audition_shutdown();
/*
    Removes the Auditioner from the Miles Event Runtime.
*/

EXPGROUP(Miles High Level Event System)

DXDEC EXPAPI void AILCALL AIL_event_system_state(HEVENTSYSTEM system, MILESEVENTSTATE* state);
/*
  Returns an information structure about the current state of the Miles Event System.

  $:system The system to retrieve information for, or zero for the default system.
  $:state A pointer to a structure to receive the state information.

  This function is a debugging aid - it returns information for the event system.
*/

DXDEC EXPAPI U32 AILCALL AIL_event_system_command_queue_remaining();
/*
    Returns the number of bytes remaining in the command buffer.

    This can be invalid for a number of reasons - first, if the
    command buffer will need to wrap for the next queue, the effective
    bytes remaining will be lower. Second, if an enqueue occurs on another
    thread in the interim, the value will be outdated.
*/

DXDEC EXPAPI S32 AILCALL AIL_get_event_length(char const* i_EventName);
/*
    Returns the length of the first sound referenced in the named event, in milliseconds.

    $:i_EventName The name of an event that starts a sound.
    $:return The length in milliseconds, or 0 if there is an error, or the event has no sound references, or the sound was not found.

    This looks up the given event and searches for the first Start Sound event step, then
    uses the first sound name in its list to look up the length. As such, if the start sound
    step has multiple sounds, the rest will be ignored.
*/

// Callback for the error handler.
EXPAPI typedef void AILCALLBACK AILEVENTERRORCB(S64 i_RelevantId, char const* i_Resource);
/*
  The function prototype to use for a callback that will be made when the event system
  encounters an unrecoverable error.

  $:i_RelevantId The ID of the asset that encountered the error, as best known. EventID or SoundID.
  $:i_Resource A string representing the name of the resource the error is in regards to, or 0 if unknown.

  The error description can be retrieved via $AIL_last_error.
*/



EXPAPI typedef S32 AILCALLBACK MSS_USER_RAND( void );
/*
  The function definition to use when defining your own random function.

  You can define a function with this prototype and pass it to $AIL_register_random
  if you want to tie the Miles random calls in with your game's (for logging and such).
*/

DXDEC EXPAPI void AILCALL AIL_set_event_error_callback(AILEVENTERRORCB * i_ErrorCallback);
/*
    Set the error handler for the event system.

    $:i_ErrorHandler The function to call when an error is encountered.

    Generally the event system handles errors gracefully - the only noticeable effect
    is that a given sound won't play, or a preset doesn't get set. As a result, the errors
    can sometimes be somewhat invisible. This function allows you to see what went wrong,
    when it went wrong.

    The basic usage is to have the callback check $AIL_last_error() for the overall category of
    failure. The parameter passed to the callback might provide some context, but it can and will
    be zero on occasion. Generally it will represent the resource string that is being worked on when the error
    occurred.

    Note that there are two out of memory errors - one is the event system ran out of memory - meaning
    the value passed in to $AIL_startup_event_system was insufficient for the current load, and
    the other is the memory used for sound data - allocated via $AIL_mem_alloc_lock - ran out.
*/


DXDEC EXPAPI void AILCALL AIL_register_random(MSS_USER_RAND * rand_func);
/*
  Sets the function that Miles will call to obtain a random number.

  Use this function to set your own random function that the Miles Event System will call when it needs a random number.
  This lets you control the determinism of the event system.
*/




#ifdef MSS_FLT_SUPPORTED

//
// Filter result codes
//

typedef SINTa FLTRESULT;

#define FLT_NOERR                   0   // Success -- no error
#define FLT_NOT_ENABLED             1   // FLT not enabled
#define FLT_ALREADY_STARTED         2   // FLT already started
#define FLT_INVALID_PARAM           3   // Invalid parameters used
#define FLT_INTERNAL_ERR            4   // Internal error in FLT driver
#define FLT_OUT_OF_MEM              5   // Out of system RAM
#define FLT_ERR_NOT_IMPLEMENTED     6   // Feature not implemented
#define FLT_NOT_FOUND               7   // FLT supported device not found
#define FLT_NOT_INIT                8   // FLT not initialized
#define FLT_CLOSE_ERR               9   // FLT not closed correctly

//############################################################################
//##                                                                        ##
//## Interface "MSS pipeline filter" (some functions shared by              ##
//## "MSS voice filter")                                                    ##
//##                                                                        ##
//############################################################################

typedef FLTRESULT (AILCALL *FLT_STARTUP)(void);

typedef FLTRESULT (AILCALL *FLT_SHUTDOWN)(void);

typedef C8 *  (AILCALL *FLT_ERROR)(void);

typedef HDRIVERSTATE (AILCALL *FLT_OPEN_DRIVER) (MSS_ALLOC_TYPE * palloc,
                                                 MSS_FREE_TYPE  * pfree,
                                                 UINTa         user,
                                                 HDIGDRIVER dig, void * memory);

typedef FLTRESULT    (AILCALL *FLT_CLOSE_DRIVER) (HDRIVERSTATE state);

typedef void         (AILCALL *FLT_PREMIX_PROCESS) (HDRIVERSTATE driver);

typedef S32          (AILCALL *FLT_POSTMIX_PROCESS) (HDRIVERSTATE driver, void *output_buffer);

//############################################################################
//##                                                                        ##
//## Interface "Pipeline filter sample services"                            ##
//##                                                                        ##
//############################################################################

typedef HSAMPLESTATE (AILCALL * FLTSMP_OPEN_SAMPLE) (HDRIVERSTATE driver,
                                                         HSAMPLE      S,
                                                         void * memory);

typedef FLTRESULT    (AILCALL * FLTSMP_CLOSE_SAMPLE) (HSAMPLESTATE state);

typedef void         (AILCALL * FLTSMP_SAMPLE_PROCESS) (HSAMPLESTATE    state,
                                                            void *      source_buffer,
                                                            void *      dest_buffer, // may be the same as src
                                                            S32             n_samples,
                                                            S32             is_stereo );

typedef S32          (AILCALL * FLTSMP_SAMPLE_PROPERTY) (HSAMPLESTATE    state,
                                                             HPROPERTY       property,
                                                             void*       before_value,
                                                             void const* new_value,
                                                             void*       after_value
                                                             );

//############################################################################
//##                                                                        ##
//## Interface "MSS output filter"                                          ##
//##                                                                        ##
//############################################################################

typedef S32 (AILCALL * VFLT_ASSIGN_SAMPLE_VOICE) (HDRIVERSTATE driver,
                                                      HSAMPLE      S);

typedef void (AILCALL * VFLT_RELEASE_SAMPLE_VOICE) (HDRIVERSTATE driver,
                                                        HSAMPLE      S);

typedef S32 (AILCALL * VFLT_START_SAMPLE_VOICE) (HDRIVERSTATE driver,
                                                     HSAMPLE      S);

//############################################################################
//##                                                                        ##
//## Interface "Voice filter driver services"                               ##
//##                                                                        ##
//############################################################################

typedef S32          (AILCALL * VDRV_DRIVER_PROPERTY) (HDRIVERSTATE    driver,
                                                           HPROPERTY       property,
                                                           void*       before_value,
                                                           void const* new_value,
                                                           void*       after_value
                                                           );

typedef S32          (AILCALL * VDRV_FORCE_UPDATE)     (HDRIVERSTATE driver);

//############################################################################
//##                                                                        ##
//## Interface "Voice filter sample services"                               ##
//##                                                                        ##
//############################################################################

typedef S32          (AILCALL * VSMP_SAMPLE_PROPERTY) (HSAMPLE      S,
                                                           HPROPERTY       property,
                                                           void*       before_value,
                                                           void const* new_value,
                                                           void*       after_value
                                                           );

//
// Pipeline filter calls
//

DXDEC HPROVIDER  AILCALL AIL_digital_output_filter (HDIGDRIVER dig);

DXDEC S32        AILCALL AIL_enumerate_filters  (HMSSENUM  *next,
                                                 HPROVIDER *dest,
                                                 C8  * *name);
DXDEC HDRIVERSTATE
                 AILCALL AIL_open_filter        (HPROVIDER  lib,
                                                 HDIGDRIVER dig);

DXDEC void       AILCALL AIL_close_filter       (HDRIVERSTATE filter);

DXDEC S32        AILCALL AIL_find_filter        (C8 const  *name,
                                                 HPROVIDER *ret);

DXDEC S32        AILCALL AIL_enumerate_filter_properties
                                                (HPROVIDER                  lib,
                                                 HMSSENUM *             next,
                                                 RIB_INTERFACE_ENTRY *  dest);

DXDEC S32        AILCALL AIL_filter_property    (HPROVIDER  lib,
                                                 C8 const*   name,
                                                 void*       before_value,
                                                 void const* new_value,
                                                 void*       after_value
                                                 );

DXDEC  S32      AILCALL AIL_enumerate_output_filter_driver_properties
                                                (HPROVIDER                 lib,
                                                 HMSSENUM *            next,
                                                 RIB_INTERFACE_ENTRY * dest);

DXDEC  S32     AILCALL AIL_output_filter_driver_property
                                                (HDIGDRIVER     dig,
                                                 C8 const * name,
                                                 void*       before_value,
                                                 void const* new_value,
                                                 void*       after_value
                                                 );

DXDEC  S32      AILCALL AIL_enumerate_output_filter_sample_properties
                                                (HPROVIDER                 lib,
                                                 HMSSENUM *            next,
                                                 RIB_INTERFACE_ENTRY * dest);

DXDEC  S32      AILCALL AIL_enumerate_filter_sample_properties
                                                (HPROVIDER                 lib,
                                                 HMSSENUM *            next,
                                                 RIB_INTERFACE_ENTRY * dest);

DXDEC S32       AILCALL AIL_enumerate_sample_stage_properties
                                                (HSAMPLE                    S,
                                                 SAMPLESTAGE                stage,
                                                 HMSSENUM *             next,
                                                 RIB_INTERFACE_ENTRY *  dest);

DXDEC  S32      AILCALL AIL_sample_stage_property
                                                (HSAMPLE        S,
                                                 SAMPLESTAGE    stage,
                                                 C8 const * name,
                                                 S32            channel,
                                                 void*       before_value,
                                                 void const* new_value,
                                                 void*       after_value
                                                 );

#define AIL_filter_sample_property(S,name,beforev,newv,afterv) AIL_sample_stage_property((S),SP_FILTER_0,(name),-1,(beforev),(newv),(afterv))

typedef struct _FLTPROVIDER
{
   S32          provider_flags;
   S32          driver_size;
   S32          sample_size;

   PROVIDER_PROPERTY               PROVIDER_property;

   FLT_STARTUP                     startup;
   FLT_ERROR                       error;
   FLT_SHUTDOWN                    shutdown;
   FLT_OPEN_DRIVER                 open_driver;
   FLT_CLOSE_DRIVER                close_driver;
   FLT_PREMIX_PROCESS              premix_process;
   FLT_POSTMIX_PROCESS             postmix_process;

   FLTSMP_OPEN_SAMPLE              open_sample;
   FLTSMP_CLOSE_SAMPLE             close_sample;
   FLTSMP_SAMPLE_PROCESS           sample_process;
   FLTSMP_SAMPLE_PROPERTY          sample_property;

   VFLT_ASSIGN_SAMPLE_VOICE        assign_sample_voice;
   VFLT_RELEASE_SAMPLE_VOICE       release_sample_voice;
   VFLT_START_SAMPLE_VOICE         start_sample_voice;

   VDRV_DRIVER_PROPERTY            driver_property;
   VDRV_FORCE_UPDATE               force_update;

   VSMP_SAMPLE_PROPERTY            output_sample_property;

   HDIGDRIVER   dig;
   HPROVIDER    provider;
   HDRIVERSTATE driver_state;

   struct _FLTPROVIDER *next;
} FLTPROVIDER;

//
// Values for "Flags" property exported by all MSS Pipeline Filter and MSS Output Filter
// providers
//

#define FPROV_ON_SAMPLES 0x0001        // Pipeline filter that operates on input samples (and is enumerated by AIL_enumerate_filters)
#define FPROV_ON_POSTMIX 0x0002        // Pipeline filter that operates on the post mixed output (capture filter)
#define FPROV_MATRIX     0x0004        // This is a matrix output filter (e.g., SRS/Dolby)
#define FPROV_VOICE      0x0008        // This is a per-voice output filter (e.g., DirectSound 3D)
#define FPROV_3D         0x0010        // Output filter uses S3D substructure for positioning
#define FPROV_OCCLUSION  0x0020        // Output filter supports occlusion (doesn't need per-sample lowpass)
#define FPROV_EAX        0x0040        // Output filter supports EAX-compatible environmental reverb
#define FPROV_SIDECHAIN  0x0080        // Filter has an "Input" property on the 3rd index for side chaining.

#define FPROV_SPU_MASK   0xff0000      // Mask here the SPU INDEX STARTS
#define FPROV_SPU_INDEX( val ) ( ( val >> 16 ) & 0xff )
#define FPROV_MAKE_SPU_INDEX( val ) ( val << 16 )



#ifdef IS_WIN32

#define MSS_EAX_AUTO_GAIN   1
#define MSS_EAX_AUTOWAH     2
#define MSS_EAX_CHORUS      3
#define MSS_EAX_DISTORTION  4
#define MSS_EAX_ECHO        5
#define MSS_EAX_EQUALIZER   6
#define MSS_EAX_FLANGER     7
#define MSS_EAX_FSHIFTER    8
#define MSS_EAX_VMORPHER    9
#define MSS_EAX_PSHIFTER   10
#define MSS_EAX_RMODULATOR 11
#define MSS_EAX_REVERB     12

typedef struct EAX_SAMPLE_SLOT_VOLUME
{
  S32 Slot;       // 0, 1, 2, 3
  S32 Send;
  S32 SendHF;
  S32 Occlusion;
  F32 OcclusionLFRatio;
  F32 OcclusionRoomRatio;
  F32 OcclusionDirectRatio;
} EAX_SAMPLE_SLOT_VOLUME;

typedef struct EAX_SAMPLE_SLOT_VOLUMES
{
  U32 NumVolumes;  // 0, 1, or 2
  EAX_SAMPLE_SLOT_VOLUME volumes[ 2 ];
} EAX_SAMPLE_SLOT_VOLUMES;

// Use this structure for EAX REVERB
typedef struct EAX_REVERB
{
  S32 Effect;                  // set to MSS_EAX_REVERB
  S32 Volume;                  // -10000 to 0
  U32 Environment;             // one of the ENVIRONMENT_ enums
  F32 EnvironmentSize;         // environment size in meters
  F32 EnvironmentDiffusion;    // environment diffusion
  S32 Room;                    // room effect level (at mid frequencies)
  S32 RoomHF;                  // relative room effect level at high frequencies
  S32 RoomLF;                  // relative room effect level at low frequencies
  F32 DecayTime;               // reverberation decay time at mid frequencies
  F32 DecayHFRatio;            // high-frequency to mid-frequency decay time ratio
  F32 DecayLFRatio;            // low-frequency to mid-frequency decay time ratio
  S32 Reflections;             // early reflections level relative to room effect
  F32 ReflectionsDelay;        // initial reflection delay time
  F32 ReflectionsPanX;         // early reflections panning vector
  F32 ReflectionsPanY;         // early reflections panning vector
  F32 ReflectionsPanZ;         // early reflections panning vector
  S32 Reverb;                  // late reverberation level relative to room effect
  F32 ReverbDelay;             // late reverberation delay time relative to initial reflection
  F32 ReverbPanX;              // late reverberation panning vector
  F32 ReverbPanY;              // late reverberation panning vector
  F32 ReverbPanZ;              // late reverberation panning vector
  F32 EchoTime;                // echo time
  F32 EchoDepth;               // echo depth
  F32 ModulationTime;          // modulation time
  F32 ModulationDepth;         // modulation depth
  F32 AirAbsorptionHF;         // change in level per meter at high frequencies
  F32 HFReference;             // reference high frequency
  F32 LFReference;             // reference low frequency
  F32 RoomRolloffFactor;       // like DS3D flRolloffFactor but for room effect
  U32 Flags;                   // modifies the behavior of properties
} EAX_REVERB;

// Use this structure for EAX AUTOGAIN
typedef struct EAX_AUTOGAIN
{
  S32 Effect;      // set to MSS_EAX_AUTO_GAIN
  S32 Volume;      // -10000 to 0
  U32 OnOff;       // Switch Compressor on or off (1 or 0)
} EAX_AUTOGAIN;

// Use this structure for EAX AUTOWAH
typedef struct EAX_AUTOWAH
{
   S32 Effect;        // set to MSS_EAX_AUTOWAH
   S32 Volume;        // -10000 to 0
   F32 AttackTime;    // Attack time (seconds)
   F32 ReleaseTime;   // Release time (seconds)
   S32 Resonance;     // Resonance (mB)
   S32 PeakLevel;     // Peak level (mB)
} EAX_AUTOWAH;

// Use this structure for EAX CHORUS
typedef struct EAX_CHORUS
{
  S32 Effect;       // set to MSS_EAX_CHORUS
  S32 Volume;       // -10000 to 0
  U32 Waveform;     // Waveform selector - 0 = sinusoid, 1 = triangle
  S32 Phase;        // Phase (Degrees)
  F32 Rate;         // Rate (Hz)
  F32 Depth;        // Depth (0 to 1)
  F32 Feedback;     // Feedback (-1 to 1)
  F32 Delay;        // Delay (seconds)
} EAX_CHORUS;

// Use this structure for EAX DISTORTION
typedef struct EAX_DISTORTION
{
  S32 Effect;        // set to MSS_EAX_DISTORTION
  S32 Volume;        // -10000 to 0
  F32 Edge;          // Controls the shape of the distortion (0 to 1)
  S32 Gain;          // Controls the post distortion gain (mB)
  F32 LowPassCutOff; // Controls the cut-off of the filter pre-distortion (Hz)
  F32 EQCenter;      // Controls the center frequency of the EQ post-distortion (Hz)
  F32 EQBandwidth;   // Controls the bandwidth of the EQ post-distortion (Hz)
} EAX_DISTORTION;

// Use this structure for EAX ECHO
typedef struct EAX_ECHO
{
  S32 Effect;        // set to MSS_EAX_ECHO
  S32 Volume;        // -10000 to 0
  F32 Delay;         // Controls the initial delay time (seconds)
  F32 LRDelay;       // Controls the delay time between the first and second taps (seconds)
  F32 Damping;       // Controls a low-pass filter that dampens the echoes (0 to 1)
  F32 Feedback;      // Controls the duration of echo repetition (0 to 1)
  F32 Spread;        // Controls the left-right spread of the echoes
} EAX_ECHO;

// Use this structure for EAXEQUALIZER_ALLPARAMETERS
typedef struct EAX_EQUALIZER
{
  S32 Effect;        // set to MSS_EAX_EQUALIZER
  S32 Volume;        // -10000 to 0
  S32 LowGain;       // (mB)
  F32 LowCutOff;     // (Hz)
  S32 Mid1Gain;      // (mB)
  F32 Mid1Center;    // (Hz)
  F32 Mid1Width;     // (octaves)
  F32 Mid2Gain;      // (mB)
  F32 Mid2Center;    // (Hz)
  F32 Mid2Width;     // (octaves)
  S32 HighGain;      // (mB)
  F32 HighCutOff;    // (Hz)
} EAX_EQUALIZER;

// Use this structure for EAX FLANGER
typedef struct EAX_FLANGER
{
  S32 Effect;       // set to MSS_EAX_FLANGER
  S32 Volume;       // -10000 to 0
  U32 Waveform;     // Waveform selector - 0 = sinusoid, 1 = triangle
  S32 Phase;        // Phase (Degrees)
  F32 Rate;         // Rate (Hz)
  F32 Depth;        // Depth (0 to 1)
  F32 Feedback;     // Feedback (0 to 1)
  F32 Delay;        // Delay (seconds)
} EAX_FLANGER;


// Use this structure for EAX FREQUENCY SHIFTER
typedef struct EAX_FSHIFTER
{
  S32 Effect;         // set to MSS_EAX_FSHIFTER
  S32 Volume;         // -10000 to 0
  F32 Frequency;      // (Hz)
  U32 LeftDirection;  // direction - 0 = down, 1 = up, 2 = off
  U32 RightDirection; // direction - 0 = down, 1 = up, 2 = off
} EAX_FSHIFTER;

// Use this structure for EAX VOCAL MORPHER
typedef struct EAX_VMORPHER
{
  S32 Effect;                // set to MSS_EAX_VMORPHER
  S32 Volume;                // -10000 to 0
  U32 PhonemeA;              // phoneme: 0 to 29 - A E I O U AA AE AH AO EH ER IH IY UH UW B D G J K L M N P R S T V Z
  S32 PhonemeACoarseTuning;  // (semitones)
  U32 PhonemeB;              // phoneme: 0 to 29 - A E I O U AA AE AH AO EH ER IH IY UH UW B D G J K L M N P R S T V Z
  S32 PhonemeBCoarseTuning;  // (semitones)
  U32 Waveform;              // Waveform selector - 0 = sinusoid, 1 = triangle, 2 = sawtooth
  F32 Rate;                  // (Hz)
} EAX_VMORPHER;


// Use this structure for EAX PITCH SHIFTER
typedef struct EAX_PSHIFTER
{
  S32 Effect;       // set to MSS_EAX_PSHIFTER
  S32 Volume;       // -10000 to 0
  S32 CoarseTune;   // Amount of pitch shift (semitones)
  S32 FineTune;     // Amount of pitch shift (cents)
} EAX_PSHIFTER;

// Use this structure for EAX RING MODULATOR
typedef struct EAX_RMODULATOR
{
  S32 Effect;          // set to MSS_EAX_RMODULATOR
  S32 Volume;          // -10000 to 0
  F32 Frequency;       // Frequency of modulation (Hz)
  F32 HighPassCutOff;  // Cut-off frequency of high-pass filter (Hz)
  U32 Waveform;        // Waveform selector - 0 = sinusoid, 1 = triangle, 2 = sawtooth
} EAX_RMODULATOR;

#endif

#else // MSS_FLT_SUPPORTED

typedef struct _FLTPROVIDER
{
  U32 junk;
} FLTPROVIDER;

#endif  // MSS_FLT_SUPPORTED

#endif // MSS_BASIC

RADDEFEND

#endif // MSS_H
