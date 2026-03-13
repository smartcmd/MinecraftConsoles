// Iggy -- Copyright 2008-2013 RAD Game Tools

#ifndef __RAD_INCLUDE_IGGY_H__
#define __RAD_INCLUDE_IGGY_H__

#include <stdlib.h> // size_t

#define IggyVersion "1.2.30"
#define IggyFlashVersion "9,1,2,30"

#include "rrCore.h"   // base data types, macros

RADDEFSTART

#ifndef IGGY_GDRAW_SHARED_TYPEDEF

   #define IGGY_GDRAW_SHARED_TYPEDEF

   typedef struct GDrawFunctions GDrawFunctions;
   typedef struct GDrawTexture   GDrawTexture;

#endif//IGGY_GDRAW_SHARED_TYPEDEF

#define IDOCN // Used by documentation generation system

////////////////////////////////////////////////////////////
//
// Basic Operations
//

typedef enum IggyResult
{
   IGGY_RESULT_SUCCESS = 0,

   IGGY_RESULT_Warning_None                            =   0,

   IGGY_RESULT_Warning_Misc                            = 100,
   IGGY_RESULT_Warning_GDraw                           = 101,
   IGGY_RESULT_Warning_ProgramFlow                     = 102,
   IGGY_RESULT_Warning_Actionscript                    = 103,
   IGGY_RESULT_Warning_Graphics                        = 104,
   IGGY_RESULT_Warning_Font                            = 105,
   IGGY_RESULT_Warning_Timeline                        = 106,
   IGGY_RESULT_Warning_Library                         = 107,
   IGGY_RESULT_Warning_ValuePath                       = 108,
   IGGY_RESULT_Warning_Audio                           = 109,

   IGGY_RESULT_Warning_CannotSustainFrameRate          = 201, // During a call to $IggyPlayerReadyToTick, Iggy detected that its rendering of a Flash file was not keeping up with the frame rate requested.
   IGGY_RESULT_Warning_ThrewException                  = 202,

   IGGY_RESULT_Error_Threshhold                        = 400,

   IGGY_RESULT_Error_Misc                              = 400,  // an uncategorized error
   IGGY_RESULT_Error_GDraw                             = 401,  // an error occured in GDraw
   IGGY_RESULT_Error_ProgramFlow                       = 402,  // an error occured with the user's program flow through the Iggy API (e.g. reentrancy issues)
   IGGY_RESULT_Error_Actionscript                      = 403,  // an error occurred in Actionscript processing
   IGGY_RESULT_Error_Graphics                          = 404,
   IGGY_RESULT_Error_Font                              = 405,
   IGGY_RESULT_Error_Create                            = 406,
   IGGY_RESULT_Error_Library                           = 407,
   IGGY_RESULT_Error_ValuePath                         = 408,  // an error occurred while processing a ValuePath
   IGGY_RESULT_Error_Audio                             = 409,

   IGGY_RESULT_Error_Internal                          = 499,

   IGGY_RESULT_Error_InvalidIggy                       = 501,
   IGGY_RESULT_Error_InvalidArgument                   = 502,
   IGGY_RESULT_Error_InvalidEntity                     = 503,
   IGGY_RESULT_Error_UndefinedEntity                   = 504,

   IGGY_RESULT_Error_OutOfMemory                       = 1001, // Iggy ran out of memory while processing the SWF. The Iggy player is now invalid and you cannot do anything further with it (except read AS3 variables). Should this happen, you'll want to $IggyPlayerDestroy and reopen the $Iggy.
} IggyResult;

typedef enum IggyDatatype
{
   IGGY_DATATYPE__invalid_request, // Set only when there is an error

   IGGY_DATATYPE_undefined, // Undefined data type
   IGGY_DATATYPE_null, // No data type
   IGGY_DATATYPE_boolean, // Data of type rrbool

   IGGY_DATATYPE_number, // Data of type F64
   IGGY_DATATYPE_string_UTF8, // Data of type $IggyStringUTF8
   IGGY_DATATYPE_string_UTF16, // Data of type $IggyStringUTF16
   IGGY_DATATYPE_fastname, // Only used when calling functions (avoids a copy operation)
   IGGY_DATATYPE_valuepath, // Only used when calling functions
   IGGY_DATATYPE_valueref, // Only used when calling functions

   // the following datatypes can be queried, but cannot appear
   // as function arguments

   IGGY_DATATYPE_array,      // Data of type Array in AS3 (appears in datatype query, never as arguments)
   IGGY_DATATYPE_object,     // Data of type Object (or a subclass) in AS3 (appears in datatype query, never as arguments)
   IGGY_DATATYPE_displayobj, // Data of type DisplayObject (or a subclass) in AS3 (only appears in callbacks)

   IGGY_DATATYPE_xml,        // Data of type XML or XMLList in AS3 (appears in datatype query, never as arguments)

   // the following datatypes also exists, but you can't access any data
   // from within them. we give you the exact type for e.g. debugging
   IGGY_DATATYPE_namespace, // Data of type Namespace in AS3 (appears in datatype query, never as arguments)
   IGGY_DATATYPE_qname,     // Data of type QName in AS3 (appears in datatype query, never as arguments)
   IGGY_DATATYPE_function,  // Data of type Function in AS3 (appears in datatype query, never as arguments)
   IGGY_DATATYPE_class,     // Data of type Class in AS3 (appears in datatype query, never as arguments)
} IggyDatatype;
/* Describes an AS3 datatype visible through iggy interface. */

#ifdef __RADWIN__
#include <stddef.h>
IDOCN typedef wchar_t IggyUTF16;
#else
typedef unsigned short IggyUTF16;
#endif

typedef struct IggyStringUTF16
{
   IggyUTF16 *string; // Null-terminated, UTF16-encoded characters
   S32  length; // Count of 16-bit characters in <tt>string</tt>, not including the null terminator
} IggyStringUTF16;

typedef struct IggyStringUTF8
{
   char *string; // Null-terminated, UTF8-encoded characters
   S32  length; // Count of 8-bit bytes in <tt>string</tt>, not including the null terminator
} IggyStringUTF8;

typedef UINTa IggyName;
typedef struct IggyValuePath IggyValuePath;
typedef void *IggyValueRef;
typedef UINTa IggyTempRef;

typedef struct IggyDataValue
{
   S32 type; // an $IggyDatatype which determines which of the union members is valid.
   #ifdef __RAD64__
   S32 padding;
   #endif
   IggyTempRef temp_ref;  // An opaque temporary reference which you can efficiently turn into an $IggyValueRef; this is written by Iggy on callbacks but never read by Iggy
   union {
      IggyStringUTF16 string16; // A UTF16 string, valid if type = $(IggyDatatype::IGGY_DATATYPE_string_UTF16)
      IggyStringUTF8  string8;  // A UTF8 string, valid if type = $(IggyDatatype::IGGY_DATATYPE_string_UTF8)
      F64             number;   // A 64-bit floating point number (a double); valid if type = $(IggyDatatype::IGGY_DATATYPE_number)
      rrbool          boolval;  // A boolean value, valid if type = $(IggyDatatype::IGGY_DATATYPE_boolean)
      IggyName        fastname; // A fast name, valid if type = $(IggyDatatype::IGGY_DATATYPE_fastname); this is only an "in" type; Iggy will never define these itself
      void          * userdata; // A userdata pointer from a DisplayObject, valid if type = $(IggyDatatype::IGGY_DATATYPE_displayobj)
      IggyValuePath * valuepath;// A path to an object in the AS3 VM, valid if type = $(IggyDatatype::IGGY_DATATYPE_valuepath); this is only an "in" type--Iggy will never output this
      IggyValueRef    valueref; // An IggyValueRef, valid if type = $(IggyDatatype::IGGY_DATATYPE_valueref);  this is only an "in" type--Iggy will never output this
   };
} IggyDataValue;

typedef struct IggyExternalFunctionCallUTF16
{
   IggyStringUTF16 function_name; // The name of the function
   S32 num_arguments; // The number of arguments that must be passed to the function
   S32 padding;
   IggyDataValue arguments[1]; // The argument types, assumed to contain <tt>num_arguments</tt> elements
} IggyExternalFunctionCallUTF16;

typedef struct IggyExternalFunctionCallUTF8
{
   IggyStringUTF8 function_name; // The name of the function
   S32 num_arguments; // The number of arguments that must be passed to the function
   S32 padding;
   IggyDataValue arguments[1]; // The argument types, assumed to contain <tt>num_arguments</tt> elements
} IggyExternalFunctionCallUTF8;

typedef void * RADLINK Iggy_AllocateFunction(void *alloc_callback_user_data, size_t size_requested, size_t *size_returned);
typedef void   RADLINK Iggy_DeallocateFunction(void *alloc_callback_user_data, void *ptr);

typedef struct IggyAllocator
{
   void                    *user_callback_data;
   Iggy_AllocateFunction   *mem_alloc;
   Iggy_DeallocateFunction *mem_free;
   #ifndef __RAD64__
   void                    *struct_padding;  // pad to 8-byte boundary
   #endif
} IggyAllocator;

RADEXPFUNC void RADEXPLINK IggyInit(IggyAllocator *allocator);
RADEXPFUNC void RADEXPLINK IggyShutdown(void);

typedef enum IggyConfigureBoolName
{
   IGGY_CONFIGURE_BOOL_StartupExceptionsAreWarnings, // if true, ActionScript exceptions thrown during startup will not prevent Iggy from being created (default false)
   IGGY_CONFIGURE_BOOL_IgnoreFlashVersion,
   IGGY_CONFIGURE_BOOL_NeverDelayGotoProcessing,
   IGGY_CONFIGURE_BOOL_SuppressAntialiasingOnAllBitmaps,
   IGGY_CONFIGURE_BOOL_SuppressAntialiasingOn9SliceBitmaps,
} IggyConfigureBoolName;

RADEXPFUNC void RADEXPLINK IggyConfigureBool(IggyConfigureBoolName prop, rrbool value);

typedef enum
{
   IGGY_VERSION_1_0_21 = 1,         // behavior from 1.0.21 and earlier
   IGGY_VERSION_1_0_24 = 3,         // behavior from 1.0.24 and earlier
   IGGY_VERSION_1_1_1  = 5,         // behavior from 1.1.1 and earlier
   IGGY_VERSION_1_1_8  = 7,         // behavior from 1.1.8 and earlier
   IGGY_VERSION_1_2_28 = 9,         // behavior from 1.2.28 and earlier
   IGGY_VERSION_default=0x7fffffff, // default (current) Iggy behavior
} IggyVersionNumber;

typedef enum
{
   IGGY_VERSIONED_BEHAVIOR_movieclip_gotoand=128, // This changes the behavior of AS3 gotoAndPlay and gotoAndStop. Valid values: IGGY_VERSION_1_0_21, IGGY_VERSION_default
   IGGY_VERSIONED_BEHAVIOR_textfield_position=129, // This changes the behavior of textfield positioning as reported by AS3 getBounds/getRect and width/height. Values with different behavior: IGGY_VERSION_1_0_24, IGGY_VERSION_default.
   IGGY_VERSIONED_BEHAVIOR_bitmap_smoothing=130,
   IGGY_VERSIONED_BEHAVIOR_textfield_autoscroll=131, // This makes textfield autoscrolling behave specially: Valid values: IGGY_VERSION_1_1_8, IGGY_VERSION_default
   IGGY_VERSIONED_BEHAVIOR_fast_text_effects=132, // This fixes the behavior of fast text effects to be in the correct direction; Valid values: IGGY_VERSION_1_2_28, IGGY_VERSION_default
} IggyVersionedBehaviorName;

RADEXPFUNC void RADEXPLINK IggyConfigureVersionedBehavior(IggyVersionedBehaviorName prop, IggyVersionNumber value);

typedef enum IggyTelemetryAmount
{
   IGGY_TELEMETRY_normal,     // Normal amount for users debugging applications using Iggy
   IGGY_TELEMETRY_internal,   // Shows more internal details, useful when optimizing Iggy itself
} IggyTelemetryAmount;

RADEXPFUNC void RADEXPLINK IggyUseTmLite(void * context, IggyTelemetryAmount amount);
RADEXPFUNC void RADEXPLINK IggyUseTelemetry(void * context, IggyTelemetryAmount amount);

////////////////////////////////////////////////////////////
//
// Translation
//


typedef struct
{
   IggyUTF16 *object_name;  /* null-terminated Textfield.name value at the time the text is set */
   rrbool     autosize;     /* true if the autosize value is non-zero at the time the text is set */
   F32        width;  /* the objectspace width of the textfield at the time the text is set */
   F32        height; /* the objectspace height of the textfield at the time the text is set */
   rrbool     is_html_text; /* whether the provided text is going through Textfield.htmlText or Textfield.text */
} IggyTextfieldInfo;

typedef void   RADLINK Iggy_TranslationFreeFunction(void *callback_data, void *data, S32 length);
typedef rrbool RADLINK Iggy_TranslateFunctionUTF16(void *callback_data, IggyStringUTF16 *src, IggyStringUTF16 *dest);
typedef rrbool RADLINK Iggy_TranslateFunctionUTF8(void *callback_data, IggyStringUTF8 *src, IggyStringUTF8 *dest);
typedef rrbool RADLINK Iggy_TextfieldTranslateFunctionUTF16(void *callback_data, IggyStringUTF16 *src, IggyStringUTF16 *dest, IggyTextfieldInfo *textfield);
typedef rrbool RADLINK Iggy_TextfieldTranslateFunctionUTF8(void *callback_data, IggyStringUTF8 *src, IggyStringUTF8 *dest, IggyTextfieldInfo *textfield);

RADEXPFUNC void RADEXPLINK IggySetLoadtimeTranslationFunction(Iggy_TranslateFunctionUTF16 *func, void *callback_data, Iggy_TranslationFreeFunction *freefunc, void *free_callback_data);
RADEXPFUNC void RADEXPLINK IggySetLoadtimeTranslationFunctionUTF16(Iggy_TranslateFunctionUTF16 *func, void *callback_data, Iggy_TranslationFreeFunction *freefunc, void *free_callback_data);
RADEXPFUNC void RADEXPLINK IggySetLoadtimeTranslationFunctionUTF8(Iggy_TranslateFunctionUTF8 *func, void *callback_data, Iggy_TranslationFreeFunction *freefunc, void *free_callback_data);
RADEXPFUNC void RADEXPLINK IggySetRuntimeTranslationFunction(Iggy_TranslateFunctionUTF16 *func, void *callback_data, Iggy_TranslationFreeFunction *freefunc, void *free_callback_data);
RADEXPFUNC void RADEXPLINK IggySetRuntimeTranslationFunctionUTF16(Iggy_TranslateFunctionUTF16 *func, void *callback_data, Iggy_TranslationFreeFunction *freefunc, void *free_callback_data);
RADEXPFUNC void RADEXPLINK IggySetRuntimeTranslationFunctionUTF8(Iggy_TranslateFunctionUTF8 *func, void *callback_data, Iggy_TranslationFreeFunction *freefunc, void *free_callback_data);
RADEXPFUNC void RADEXPLINK IggySetTextfieldTranslationFunctionUTF16(Iggy_TextfieldTranslateFunctionUTF16 *func, void *callback_data, Iggy_TranslationFreeFunction *freefunc, void *free_callback_data);
RADEXPFUNC void RADEXPLINK IggySetTextfieldTranslationFunctionUTF8(Iggy_TextfieldTranslateFunctionUTF8 *func, void *callback_data, Iggy_TranslationFreeFunction *freefunc, void *free_callback_data);

typedef enum
{
   IGGY_LANG_default,
   IGGY_LANG_ja,
   IGGY_LANG_ja_flash, // more strictly matches Flash
} IggyLanguageCode;

RADEXPFUNC void RADEXPLINK IggySetLanguage(IggyLanguageCode lang);

////////////////////////////////////////////////////////////
//
// Playback
//

typedef struct Iggy Iggy;
typedef S32 IggyLibrary;

typedef void   RADLINK Iggy_TraceFunctionUTF16(void *user_callback_data, Iggy *player, IggyUTF16 const *utf16_string, S32 length_in_16bit_chars);
typedef void   RADLINK Iggy_TraceFunctionUTF8(void *user_callback_data, Iggy *player, char const *utf8_string, S32 length_in_bytes);
typedef void   RADLINK Iggy_WarningFunction(void *user_callback_data, Iggy *player, IggyResult error_code, char const *error_message);

typedef struct
{
   S32  total_storage_in_bytes;       // the total memory to use for the AS3 heap and garbage collector
   S32  stack_size_in_bytes;          // size of the stack used for AS3 expression evaluation and function activation records
   S32  young_heap_size_in_bytes;     // size of the heap from which initial allocations are made
   S32  old_heap_size_in_bytes;       // this parameter is not supported yet
   S32  remembered_set_size_in_bytes; // storage used to keep track of pointers from old heap to young heap
   S32  greylist_size_in_bytes;       // storage used to keep track of partially-garbage collected objects on the old heap
   S32  rootstack_size_in_bytes;      // size of the stack used for exposing temporaries to the garbage collector
   S32  padding;
} IggyPlayerGCSizes;

typedef struct
{
   IggyAllocator allocator;
   IggyPlayerGCSizes gc;
   char *filename;
   char *user_name;
   rrbool load_in_place;
   rrbool did_load_in_place;
} IggyPlayerConfig;

RADEXPFUNC Iggy * RADEXPLINK IggyPlayerCreateFromFileAndPlay(
                                char const *           filename,
                                IggyPlayerConfig const*config);

RADEXPFUNC Iggy * RADEXPLINK IggyPlayerCreateFromMemory(
                                void const *           data,
                                U32                    data_size_in_bytes,
                                IggyPlayerConfig      *config);

#define IGGY_INVALID_LIBRARY -1

RADEXPFUNC  IggyLibrary  RADEXPLINK IggyLibraryCreateFromMemory(
                                char const *           url_utf8_null_terminated,
                                void const *           data,
                                U32                    data_size_in_bytes,
                                IggyPlayerConfig      *config);

RADEXPFUNC  IggyLibrary  RADEXPLINK IggyLibraryCreateFromMemoryUTF16(
                                IggyUTF16 const *      url_utf16_null_terminated,
                                void const *           data,
                                U32                    data_size_in_bytes,
                                IggyPlayerConfig      *config);

RADEXPFUNC void RADEXPLINK IggyPlayerDestroy(Iggy *player);
RADEXPFUNC void RADEXPLINK IggyLibraryDestroy(IggyLibrary lib);
RADEXPFUNC void RADEXPLINK IggySetWarningCallback(Iggy_WarningFunction *error, void *user_callback_data);
RADEXPFUNC void RADEXPLINK IggySetTraceCallbackUTF8(Iggy_TraceFunctionUTF8 *trace_utf8, void *user_callback_data);
RADEXPFUNC void RADEXPLINK IggySetTraceCallbackUTF16(Iggy_TraceFunctionUTF16 *trace_utf16, void *user_callback_data);

typedef struct IggyProperties
{
   S32  movie_width_in_pixels;            // the width of the "document" specified in the SWF file
   S32  movie_height_in_pixels;           // the height of the "document" specified in the SWF file

   F32  movie_frame_rate_current_in_fps;   // the current frame rate Iggy is trying to achieve for the file
   F32  movie_frame_rate_from_file_in_fps; // the frame rate specified in the SWF file

   S32  frames_passed;  // the number of times Tick() has been called
   S32  swf_major_version_number; // the major SWF version number of the file, currently always 9

   F64  time_passed_in_seconds;  // the total time passed since starting the file
   F64  seconds_since_last_tick; // the number of seconds that have ocurred 
   F64  seconds_per_drawn_frame; // 1/render fps, updated on $IggyPlayerDrawTilesStart
} IggyProperties;

RADEXPFUNC IggyProperties * RADEXPLINK IggyPlayerProperties(Iggy *player);

typedef enum
{
   IGGY_PAUSE_continue_audio,
   IGGY_PAUSE_pause_audio,
   IGGY_PAUSE_stop_audio
} IggyAudioPauseMode;

RADEXPFUNC void * RADEXPLINK IggyPlayerGetUserdata(Iggy *player);
RADEXPFUNC void RADEXPLINK IggyPlayerSetUserdata(Iggy *player, void *userdata);

RADEXPFUNC void RADEXPLINK IggyPlayerInitializeAndTickRS(Iggy *player);
RADEXPFUNC rrbool RADEXPLINK IggyPlayerReadyToTick(Iggy *player);
RADEXPFUNC void RADEXPLINK IggyPlayerTickRS(Iggy *player);
RADEXPFUNC void RADEXPLINK IggyPlayerPause(Iggy *player, IggyAudioPauseMode pause_audio);
RADEXPFUNC void RADEXPLINK IggyPlayerPlay(Iggy *player);
RADEXPFUNC void RADEXPLINK IggyPlayerSetFrameRate(Iggy *player, F32 frame_rate_in_fps);
RADEXPFUNC void RADEXPLINK IggyPlayerGotoFrameRS(Iggy *f, S32 frame, rrbool stop);

#ifndef __RAD_HIGGYEXP_
#define __RAD_HIGGYEXP_
typedef void * HIGGYEXP;
/* An IggyExplorer context, it represents a connection to Iggy Explorer. */
#endif

#ifndef __RAD_HIGGYPERFMON_
#define __RAD_HIGGYPERFMON_
typedef void * HIGGYPERFMON;
/* An IggyPerfMon context */
#endif


IDOCN typedef void RADLINK iggyexp_detach_callback(void *ptr);

IDOCN typedef struct
{
   U64 tick_ticks;
   U64 draw_ticks;
} IggyPerfmonStats;

IDOCN typedef struct
{
   void (RADLINK *get_stats)(Iggy* swf, IggyPerfmonStats* pdest);
   const char* (RADLINK *get_display_name)(Iggy* swf);
} IggyForPerfmonFunctions;

// This is used by both Iggy Explorer and Perfmon
IDOCN typedef struct
{
   rrbool (RADLINK *connection_valid)(Iggy* swf, HIGGYEXP iggyexp); // Iggy queries this to check if Iggy Explorer is still connected
   S32    (RADLINK *poll_command)(Iggy* swf, HIGGYEXP iggyexp, U8 **buffer); // stores command in *buffer, returns number of bytes
   void   (RADLINK *send_command)(Iggy* swf, HIGGYEXP iggyexp, U8 command, void *buffer, S32 len); // writes a command with a payload of buffer:len
   S32    (RADLINK *get_storage)(Iggy* swf, HIGGYEXP iggyexp, U8 **buffer); // returns temporary storage Iggy can use for assembling commands
   rrbool (RADLINK *attach)(Iggy* swf, HIGGYEXP iggyexp, iggyexp_detach_callback *cb, void *cbdata, IggyForPerfmonFunctions* pmf); // an Iggy file is trying to attach itself to this connection (one at a time)
   rrbool (RADLINK *detach)(Iggy* swf, HIGGYEXP iggyexp); // the current Iggy file should be detached (generate callback)
   void   (RADLINK *draw_tile_hook)(Iggy* swf, HIGGYEXP iggyexp, GDrawFunctions* iggy_gdraw);   // only used by perfmon
} IggyExpFunctions;

RADEXPFUNC void RADEXPLINK IggyInstallPerfmon(void *perfmon_context);

RADEXPFUNC void RADEXPLINK IggyUseExplorer(Iggy *swf, void *context);
IDOCN RADEXPFUNC void RADEXPLINK IggyPlayerSendFrameToExplorer(Iggy *f);

////////////////////////////////////////////////////////////
//
// Fonts
//

typedef struct
{
   F32 ascent;
   F32 descent;
   F32 line_gap;
   F32 average_glyph_width_for_tab_stops; // for embedded fonts, Iggy uses width of 'g'
   F32 largest_glyph_bbox_y1;
} IggyFontMetrics;

typedef struct
{
   F32 x0,y0, x1,y1;   // bounding box
   F32 advance;        // distance to move origin after this character
} IggyGlyphMetrics;

typedef enum {
   IGGY_VERTEX_move  = 1,
   IGGY_VERTEX_line  = 2,
   IGGY_VERTEX_curve = 3,
} IggyShapeVertexType;

typedef struct
{
   F32 x,y;   // if IGGY_VERTEX_move, point to start a new loop; if IGGY_VERTEX_line/curve, endpoint of segment
   F32 cx,cy; // if IGGY_VERTEX_curve, control point on segment; ignored otherwise
   U8 type;   // value from $IggyShapeVertexType

   S8 padding; // ignore
   U16 f0;     // set to 1
   U16 f1;     // set to 0
   U16 line;   // ignore
} IggyShapeVertex;

typedef struct
{
   IggyShapeVertex * vertices;
   S32               num_vertices;
   void            * user_context_for_free;   // you can use this to store data to access on the corresponding free call
} IggyVectorShape;

typedef struct
{
   U8    *pixels_one_per_byte;         // pixels from the top left, 0 is transparent and 255 is opaque
   S32    width_in_pixels;             // this is the actual width of the bitmap data
   S32    height_in_pixels;            // this is the actual height of the bitmap data
   S32    stride_in_bytes;             // the distance from one row to the next
   S32    oversample;                  // this is the amount of oversampling (0 or 1 = not oversample, 2 = 2x oversampled, 4 = 4x oversampled)
   rrbool point_sample;                // if true, the bitmap will be drawn with point sampling; if false, it will be drawn with bilinear
   S32    top_left_x;                  // the offset of the top left corner from the character origin
   S32    top_left_y;                  // the offset of the top left corner from the character origin
   F32    pixel_scale_correct;         // the pixel_scale at which this character should be displayed at width_in_pixels
   F32    pixel_scale_min;             // the smallest pixel_scale to allow using this character (scaled down)
   F32    pixel_scale_max;             // the largest pixels cale to allow using this character (scaled up)
   void * user_context_for_free;       // you can use this to store data to access on the corresponding free call
} IggyBitmapCharacter;

typedef IggyFontMetrics * RADLINK IggyFontGetFontMetrics(void *user_context, IggyFontMetrics *metrics);

#define IGGY_GLYPH_INVALID              -1
typedef S32                RADLINK IggyFontGetCodepointGlyph(void *user_context, U32 codepoint);
typedef IggyGlyphMetrics * RADLINK IggyFontGetGlyphMetrics(void *user_context, S32 glyph, IggyGlyphMetrics *metrics);
typedef rrbool             RADLINK IggyFontIsGlyphEmpty(void *user_context, S32 glyph);
typedef F32                RADLINK IggyFontGetKerningForGlyphPair(void *user_context, S32 first_glyph, S32 second_glyph);

typedef void RADLINK IggyVectorFontGetGlyphShape(void *user_context, S32 glyph, IggyVectorShape *shape);
typedef void RADLINK IggyVectorFontFreeGlyphShape(void *user_context, S32 glyph, IggyVectorShape *shape);

typedef rrbool RADLINK IggyBitmapFontCanProvideBitmap(void *user_context, S32 glyph, F32 pixel_scale);
typedef rrbool RADLINK IggyBitmapFontGetGlyphBitmap(void *user_context, S32 glyph, F32 pixel_scale, IggyBitmapCharacter *bitmap);
typedef void RADLINK IggyBitmapFontFreeGlyphBitmap(void *user_context, S32 glyph, F32 pixel_scale, IggyBitmapCharacter *bitmap);


typedef struct
{
   IggyFontGetFontMetrics          *get_font_metrics;

   IggyFontGetCodepointGlyph       *get_glyph_for_codepoint;
   IggyFontGetGlyphMetrics         *get_glyph_metrics;
   IggyFontIsGlyphEmpty            *is_empty;
   IggyFontGetKerningForGlyphPair  *get_kerning;

   IggyVectorFontGetGlyphShape     *get_shape;
   IggyVectorFontFreeGlyphShape    *free_shape;

   S32                              num_glyphs;

   void *userdata;
} IggyVectorFontProvider;

typedef struct
{
   IggyFontGetFontMetrics          *get_font_metrics;

   IggyFontGetCodepointGlyph       *get_glyph_for_codepoint;
   IggyFontGetGlyphMetrics         *get_glyph_metrics;
   IggyFontIsGlyphEmpty            *is_empty;
   IggyFontGetKerningForGlyphPair  *get_kerning;

   IggyBitmapFontCanProvideBitmap  *can_bitmap;
   IggyBitmapFontGetGlyphBitmap    *get_bitmap;
   IggyBitmapFontFreeGlyphBitmap   *free_bitmap;

   S32                              num_glyphs;

   void *userdata;
} IggyBitmapFontProvider;

typedef struct
{
   IggyBitmapFontCanProvideBitmap  *can_bitmap;
   IggyBitmapFontGetGlyphBitmap    *get_bitmap;
   IggyBitmapFontFreeGlyphBitmap   *free_bitmap;
   void *userdata;
} IggyBitmapFontOverride;

RADEXPFUNC void RADEXPLINK IggySetInstalledFontMaxCount(S32 num);
RADEXPFUNC void RADEXPLINK IggySetIndirectFontMaxCount(S32 num);

#define IGGY_FONTFLAG_none    0
#define IGGY_FONTFLAG_bold    1
#define IGGY_FONTFLAG_italic  2
#define IGGY_FONTFLAG_all    (~0U)  // indirection only

#define IGGY_TTC_INDEX_none   0

RADEXPFUNC void RADEXPLINK IggyFontInstallTruetypeUTF8(const void *truetype_storage, S32 ttc_index, const char *fontname, S32 namelen_in_bytes, U32 fontflags);
RADEXPFUNC void RADEXPLINK IggyFontInstallTruetypeUTF16(const void *truetype_storage, S32 ttc_index, const U16 *fontname, S32 namelen_in_16bit_quantities, U32 fontflags);
RADEXPFUNC void RADEXPLINK IggyFontInstallTruetypeFallbackCodepointUTF8(const char *fontname, S32 len, U32 fontflags, S32 fallback_codepoint);
RADEXPFUNC void RADEXPLINK IggyFontInstallTruetypeFallbackCodepointUTF16(const U16 *fontname, S32 len, U32 fontflags, S32 fallback_codepoint);
RADEXPFUNC void RADEXPLINK IggyFontInstallVectorUTF8(const IggyVectorFontProvider *vfp, const char *fontname, S32 namelen_in_bytes, U32 fontflags);
RADEXPFUNC void RADEXPLINK IggyFontInstallVectorUTF16(const IggyVectorFontProvider *vfp, const U16 *fontname, S32 namelen_in_16bit_quantities, U32 fontflags);
RADEXPFUNC void RADEXPLINK IggyFontInstallBitmapUTF8(const IggyBitmapFontProvider *bmf, const char *fontname, S32 namelen_in_bytes, U32 fontflags);
RADEXPFUNC void RADEXPLINK IggyFontInstallBitmapUTF16(const IggyBitmapFontProvider *bmf, const U16 *fontname, S32 namelen_in_16bit_quantities, U32 fontflags);
RADEXPFUNC void RADEXPLINK IggyFontInstallBitmapOverrideUTF8(const IggyBitmapFontOverride *bmf, const char *fontname, S32 namelen_in_bytes, U32 fontflags);
RADEXPFUNC void RADEXPLINK IggyFontInstallBitmapOverrideUTF16(const IggyBitmapFontOverride *bmf, const U16 *fontname, S32 namelen_in_16bit_quantities, U32 fontflags);

RADEXPFUNC void RADEXPLINK IggyFontRemoveUTF8(const char *fontname, S32 namelen_in_bytes, U32 fontflags);
RADEXPFUNC void RADEXPLINK IggyFontRemoveUTF16(const U16 *fontname, S32 namelen_in_16bit_quantities, U32 fontflags);

RADEXPFUNC void RADEXPLINK IggyFontSetIndirectUTF8(const char *request_name, S32 request_namelen, U32 request_flags, const char *result_name, S32 result_namelen, U32 result_flags);
RADEXPFUNC void RADEXPLINK IggyFontSetIndirectUTF16(const U16 *request_name, S32 request_namelen, U32 request_flags, const U16 *result_name, S32 result_namelen, U32 result_flags);

RADEXPFUNC void RADEXPLINK IggyFontSetFallbackFontUTF8(const char *fontname, S32 fontname_len, U32 fontflags);
RADEXPFUNC void RADEXPLINK IggyFontSetFallbackFontUTF16(const U16 *fontname, S32 fontname_len, U32 fontflags);

////////////////////////////////////////////////////////////
//
// Audio
//

struct _RadSoundSystem;
IDOCN typedef S32 (*IGGYSND_OPEN_FUNC)(struct _RadSoundSystem* i_SoundSystem, U32 i_MinBufferSizeInMs, U32 i_Frequency, U32 i_ChannelCount, U32 i_MaxLockSize, U32 i_Flags);

IDOCN RADEXPFUNC void RADEXPLINK IggyAudioSetDriver(IGGYSND_OPEN_FUNC driver_open, U32 flags);

// These functions cause Iggy to use a specific audio API, most of which
// are only actually defined on one target platform. Probably, you'll just
// want to call IggyAudioUseDefault.

IDOCN RADEXPFUNC void RADEXPLINK IggyAudioUseDirectSound(void);
IDOCN RADEXPFUNC void RADEXPLINK IggyAudioUseWaveOut(void);
IDOCN RADEXPFUNC void RADEXPLINK IggyAudioUseXAudio2(void);
IDOCN RADEXPFUNC void RADEXPLINK IggyAudioUseLibAudio(void);
IDOCN RADEXPFUNC void RADEXPLINK IggyAudioUseAX(void);
IDOCN RADEXPFUNC void RADEXPLINK IggyAudioUseCoreAudio(void);

RADEXPFUNC void RADEXPLINK IggyAudioUseDefault(void);

#ifndef __RAD_DEFINE_IGGYMP3__
#define __RAD_DEFINE_IGGYMP3__
IDOCN typedef struct IggyMP3Interface IggyMP3Interface;
IDOCN typedef rrbool IggyGetMP3Decoder(IggyMP3Interface *decoder);
#endif

#ifdef __RADNT__
   RADEXPFUNC void RADEXPLINK IggyAudioInstallMP3Decoder(void);
   RADEXPFUNC void RADEXPLINK IggySetDLLDirectory(char *path);
   RADEXPFUNC void RADEXPLINK IggySetDLLDirectoryW(wchar_t *path);
#else
   // this is overkill for non-DLL implementations, which could call into Iggy
   // directly, but it means everything goes through the same indirection internally
   IDOCN RADEXPFUNC IggyGetMP3Decoder* RADEXPLINK IggyAudioGetMP3Decoder(void);
   IDOCN RADEXPFUNC void RADEXPLINK IggyAudioInstallMP3DecoderExplicit(IggyGetMP3Decoder *init);

   #define IggyAudioInstallMP3Decoder() \
      IggyAudioInstallMP3DecoderExplicit(IggyAudioGetMP3Decoder()) IDOCN 
#endif

RADEXPFUNC rrbool RADEXPLINK IggyAudioSetMaxBufferTime(S32 ms);
RADEXPFUNC void   RADEXPLINK IggyAudioSetLatency(S32 ms);
RADEXPFUNC void   RADEXPLINK IggyPlayerSetAudioVolume(Iggy *iggy, F32 attenuation);

#define   IGGY_AUDIODEVICE_default    0
#define   IGGY_AUDIODEVICE_primary    1
#define   IGGY_AUDIODEVICE_secondary  2

IDOCN RADEXPFUNC void   RADEXPLINK IggyPlayerSetAudioDevice(Iggy *iggy, S32 device);


////////////////////////////////////////////////////////////
//
// Rendering
//

typedef struct IggyCustomDrawCallbackRegion
{
    IggyUTF16 *name;        // the name of the DisplayObject being substituted
    F32 x0, y0, x1, y1;     // the bounding box of the original DisplayObject, in object space
    F32 rgba_mul[4];        // any multiplicative color effect specified for the DisplayObject or its parents
    F32 rgba_add[4];        // any additive color effect specified for the DisplayObject or its parents
    S32 scissor_x0, scissor_y0, scissor_x1, scissor_y1;  // optional scissor rect box
    U8 scissor_enable;      // if non-zero, clip to the scissor rect
    U8 stencil_func_mask;   // D3DRS_STENCILMASK or equivalent
    U8 stencil_func_ref;    // D3DRS_STENCILREF or equivalent
    U8 stencil_write_mask;  // if non-zero, D3DRS_STENCILWRITEMASK or equivalent 
    struct gswf_matrix *o2w; // Iggy object-to-world matrix (used internally)
} IggyCustomDrawCallbackRegion;

typedef void RADLINK Iggy_CustomDrawCallback(void *user_callback_data, Iggy *player, IggyCustomDrawCallbackRegion *Region);
typedef GDrawTexture* RADLINK Iggy_TextureSubstitutionCreateCallback(void *user_callback_data, IggyUTF16 *texture_name, S32 *width, S32 *height, void **destroy_callback_data);
typedef void RADLINK Iggy_TextureSubstitutionDestroyCallback(void *user_callback_data, void *destroy_callback_data, GDrawTexture *handle);
typedef GDrawTexture* RADLINK Iggy_TextureSubstitutionCreateCallbackUTF8(void *user_callback_data, char *texture_name, S32 *width, S32 *height, void **destroy_callback_data);

RADEXPFUNC void RADEXPLINK IggySetCustomDrawCallback(Iggy_CustomDrawCallback *custom_draw, void *user_callback_data);
RADEXPFUNC void RADEXPLINK IggySetTextureSubstitutionCallbacks(Iggy_TextureSubstitutionCreateCallback *texture_create, Iggy_TextureSubstitutionDestroyCallback *texture_destroy, void *user_callback_data);
RADEXPFUNC void RADEXPLINK IggySetTextureSubstitutionCallbacksUTF8(Iggy_TextureSubstitutionCreateCallbackUTF8 *texture_create, Iggy_TextureSubstitutionDestroyCallback *texture_destroy, void *user_callback_data);

typedef enum {
   IGGY_FLUSH_no_callback,         // <i>do not</i> generate the $Iggy_TextureSubstitutionDestroyCallback
   IGGY_FLUSH_destroy_callback,    // do generate the $Iggy_TextureSubstitutionDestroyCallback
} IggyTextureSubstitutionFlushMode;

RADEXPFUNC void RADEXPLINK IggyTextureSubstitutionFlush(GDrawTexture *handle, IggyTextureSubstitutionFlushMode do_destroy_callback);
RADEXPFUNC void RADEXPLINK IggyTextureSubstitutionFlushAll(IggyTextureSubstitutionFlushMode do_destroy_callback);

RADEXPFUNC void RADEXPLINK IggySetGDraw(GDrawFunctions *gdraw);
RADEXPFUNC void RADEXPLINK IggyPlayerGetBackgroundColor(Iggy *player, F32 output_color[3]);

typedef enum
{
   IGGY_ROTATION_0_degrees = 0,
   IGGY_ROTATION_90_degrees_counterclockwise = 1,
   IGGY_ROTATION_180_degrees = 2,
   IGGY_ROTATION_90_degrees_clockwise = 3,
} Iggy90DegreeRotation;

RADEXPFUNC void RADEXPLINK IggyPlayerSetDisplaySize(Iggy *f, S32 w, S32 h);
RADEXPFUNC void RADEXPLINK IggyPlayerSetPixelShape(Iggy *swf, F32 pixel_x, F32 pixel_y);
RADEXPFUNC void RADEXPLINK IggyPlayerSetStageRotation(Iggy *f, Iggy90DegreeRotation rot);
RADEXPFUNC void RADEXPLINK IggyPlayerDraw(Iggy *f);
RADEXPFUNC void RADEXPLINK IggyPlayerSetStageSize(Iggy *f, S32 w, S32 h);
RADEXPFUNC void RADEXPLINK IggyPlayerSetFaux3DStage(Iggy *f, F32 *top_left, F32 *top_right, F32 *bottom_left, F32 *bottom_right, F32 depth_scale);
RADEXPFUNC void RADEXPLINK IggyPlayerForceMipmaps(Iggy *f, rrbool force_mipmaps);

RADEXPFUNC void RADEXPLINK IggyPlayerDrawTile(Iggy *f, S32 x0, S32 y0, S32 x1, S32 y1, S32 padding);
RADEXPFUNC void RADEXPLINK IggyPlayerDrawTilesStart(Iggy *f);
RADEXPFUNC void RADEXPLINK IggyPlayerDrawTilesEnd(Iggy *f);
RADEXPFUNC void RADEXPLINK IggyPlayerSetRootTransform(Iggy *f, F32 mat[4], F32 tx, F32 ty);
RADEXPFUNC void RADEXPLINK IggyPlayerFlushAll(Iggy *player);
RADEXPFUNC void RADEXPLINK IggyLibraryFlushAll(IggyLibrary h);
RADEXPFUNC void RADEXPLINK IggySetTextCursorPixelWidth(S32 width);
RADEXPFUNC void RADEXPLINK IggyForceBitmapSmoothing(rrbool force_on);
RADEXPFUNC void RADEXPLINK IggyFlushInstalledFonts(void);
RADEXPFUNC void RADEXPLINK IggyFastTextFilterEffects(rrbool enable);

typedef enum IggyAntialiasing
{
   IGGY_ANTIALIASING_FontsOnly = 2,  // Anti-aliasing of bitmapped fonts only
   IGGY_ANTIALIASING_FontsAndLinesOnly = 4, // Anti-aliasing of fonts and lines, but nothing else
   IGGY_ANTIALIASING_PrettyGood = 8, // High-quality anti-aliasing on everything, but no rendertargets required
   IGGY_ANTIALIASING_Good = 10, // High-quality anti-aliasing on everything (on platforms where GDraw doesn't support rendertargets, such as the Wii, this behaves the same as PrettyGood)
} IggyAntialiasing;

RADEXPFUNC void RADEXPLINK IggyPlayerSetAntialiasing(Iggy *f, IggyAntialiasing antialias_mode);

RADEXPFUNC void RADEXPLINK IggyPlayerSetBitmapFontCaching(
    Iggy *f,
    S32 tex_w,
    S32 tex_h,
    S32 max_char_pix_width,
    S32 max_char_pix_height);

RADEXPFUNC void RADEXPLINK IggySetFontCachingCalculationBuffer(
    S32 max_chars,
    void *optional_temp_buffer,
    S32 optional_temp_buffer_size_in_bytes);

typedef struct IggyGeneric IggyGeneric;

RADEXPFUNC IggyGeneric * RADEXPLINK IggyPlayerGetGeneric(Iggy *player);
RADEXPFUNC IggyGeneric * RADEXPLINK IggyLibraryGetGeneric(IggyLibrary lib);

// each texture metadata block contains one of these, where
// texture_info is an array of per-format data
IDOCN typedef struct
{
   U16 num_textures;
   U16 load_alignment_log2;
   U32 texture_file_size;
   void *texture_info;
} IggyTextureResourceMetadata;

RADEXPFUNC void RADEXPLINK IggyGenericInstallResourceFile(IggyGeneric *g, void *data, S32 data_length, rrbool *can_free_now);
RADEXPFUNC IggyTextureResourceMetadata *RADEXPLINK IggyGenericGetTextureResourceMetadata(IggyGeneric *f);
RADEXPFUNC void RADEXPLINK IggyGenericSetTextureFromResource(IggyGeneric *f, U16 id, GDrawTexture *handle);

// this is the encoding for the "raw" texture type, which doesn't
// depend on any platform headers
typedef enum
{
   IFT_FORMAT_rgba_8888,
   IFT_FORMAT_rgba_4444_LE,
   IFT_FORMAT_rgba_5551_LE,
   IFT_FORMAT_la_88,
   IFT_FORMAT_la_44,
   IFT_FORMAT_i_8,
   IFT_FORMAT_i_4,
   IFT_FORMAT_l_8,
   IFT_FORMAT_l_4,
   IFT_FORMAT_DXT1,
   IFT_FORMAT_DXT3,
   IFT_FORMAT_DXT5,
} IggyFileTexture_Format;

typedef struct
{
   U32 file_offset;
   U8  format;
   U8  mipmaps;
   U16 w,h;
   U16 swf_id;   
} IggyFileTextureRaw;

IDOCN typedef struct
{
   U32 file_offset;
   U16 swf_id;
   U16 padding;
   struct {
      U32 data[13];
   } texture;
} IggyFileTexture360;

IDOCN typedef struct
{
   U32 file_offset;
   U16 swf_id;
   U8  format;
   U8  padding;
   struct {
      U32 data[6];      
   } texture;
} IggyFileTexturePS3;

IDOCN typedef struct
{
   U32 file_offset1;
   U32 file_offset2;
   U16 swf_id;
   U8  format;
   U8  padding;
   struct {
      U32 data1[39];
   } texture;
} IggyFileTextureWiiu;

IDOCN typedef struct
{
   U32 file_offset;
   U16 swf_id;
   U8  format;
   U8  padding;
   struct {
      U32 data[8];
   } texture;
} IggyFileTexturePS4;

IDOCN typedef struct
{
   U32 file_offset;
   U16 swf_id;
   U8  format;
   U8  padding;
   struct {
      U32 format;
      U32 type;
      U16 width;
      U16 height;
      U8 mip_count;
      U8 pad[3];
   } texture;
} IggyFileTexturePSP2;

////////////////////////////////////////////////////////////
//
// AS3
//

typedef rrbool RADLINK Iggy_AS3ExternalFunctionUTF8(void *user_callback_data, Iggy *player, IggyExternalFunctionCallUTF8 *call);
typedef rrbool RADLINK Iggy_AS3ExternalFunctionUTF16(void *user_callback_data, Iggy *player, IggyExternalFunctionCallUTF16 *call);

RADEXPFUNC void RADEXPLINK IggySetAS3ExternalFunctionCallbackUTF8(Iggy_AS3ExternalFunctionUTF8 *as3_external_function_utf8, void *user_callback_data);
RADEXPFUNC void RADEXPLINK IggySetAS3ExternalFunctionCallbackUTF16(Iggy_AS3ExternalFunctionUTF16 *as3_external_function_utf16, void *user_callback_data);
RADEXPFUNC IggyName RADEXPLINK IggyPlayerCreateFastName(Iggy *f, IggyUTF16 const *name, S32 len);
RADEXPFUNC IggyName RADEXPLINK IggyPlayerCreateFastNameUTF8(Iggy *f, char const *name, S32 len);
RADEXPFUNC IggyResult RADEXPLINK IggyPlayerCallFunctionRS(Iggy *player, IggyDataValue *result, IggyName function, S32 numargs, IggyDataValue *args);
RADEXPFUNC IggyResult RADEXPLINK IggyPlayerCallMethodRS(Iggy *f, IggyDataValue *result, IggyValuePath *target, IggyName methodname, S32 numargs, IggyDataValue *args);
RADEXPFUNC void RADEXPLINK IggyPlayerGarbageCollect(Iggy *player, S32 strength);

#define IGGY_GC_MINIMAL  0
#define IGGY_GC_NORMAL   30
#define IGGY_GC_MAXIMAL  100

typedef struct
{
   U32 young_heap_size;           // the size of the young heap is the smaller of this number and the size the young heap was originally allocated when the Iggy was created
   U32 base_old_amount;           // the base number of words to process on each minor cycle, default 200
   F32 old_heap_fraction;         // the fraction 0..1 (default 0.125) of the outstanding allocations from the last major GC cycle to traverse during one GC cycle
   F32 new_allocation_multiplier; // a number from 1..infinity (default 2) which is the amount of the allocations in the last cycle to traverse
   F32 sweep_multiplier;          // a positive number (default 2) which weights the amount of data swept vs marked
} IggyGarbageCollectorControl;

typedef enum
{
   IGGY_GC_EVENT_tenure,
   IGGY_GC_EVENT_mark_increment,
   IGGY_GC_EVENT_mark_roots,
   IGGY_GC_EVENT_sweep_finalize,
   IGGY_GC_EVENT_sweep_increment,
   IGGY_GC_WARNING_greylist_overflow,     // the grey list overflowed, increase the size of $(IggyPlayerGCSizes::greylist_size_in_bytes).
   IGGY_GC_WARNING_remembered_overflow,   // the remembered set overflowed, increase the size of $(IggyPlayerGCSizes::remembered_set_size_in_bytes).
} IggyGarbageCollectionEvent;

typedef struct
{
   U64 event_time_in_microseconds;
   U64 total_marked_bytes;                // total bytes ever marked by the GC
   U64 total_swept_bytes;                 // total bytes ever swept by the GC
   U64 total_allocated_bytes;             // total bytes ever allocated from the old heap
   U64 total_gc_time_in_microseconds;     // total time spent in GC while notify callback was active

   char *name;

   IggyGarbageCollectionEvent event;      // the type of garbage collection event that was just performed

   U32 increment_processing_bytes;        // the number of bytes that were processed in that event

   U32 last_slice_tenured_bytes;          // the number of bytes that were tenured from young-to-old heap since the previous GC step
   U32 last_slice_old_allocation_bytes;   // the number of bytes that were tenured or were directly allocated from the old heap since the previous GC step

   U32 heap_used_bytes;                   // the number of bytes in use in the old heap (the young heap is empty)
   U32 heap_size_bytes;                   // the number of bytes allocated for the old heap

   U32 onstage_display_objects;           // the number of on-stage display objects (MovieClips, TextFields, Shapes, etc) visited during tenuring only
   U32 offstage_display_objects;          // the number of off-stage display objects visited during tenuring only
} IggyGarbageCollectionInfo;

typedef void RADLINK Iggy_GarbageCollectionCallback(Iggy *player, IggyGarbageCollectionInfo *info);
RADEXPFUNC void RADEXPLINK IggyPlayerConfigureGCBehavior(Iggy *player, Iggy_GarbageCollectionCallback *notify_callack, IggyGarbageCollectorControl *control);
RADEXPFUNC void RADEXPLINK IggyPlayerQueryGCSizes(Iggy *player, IggyPlayerGCSizes *sizes);

RADEXPFUNC rrbool RADEXPLINK IggyPlayerGetValid(Iggy *f);

IDOCN struct IggyValuePath
{
    Iggy *f;
    IggyValuePath *parent;
                                 //align 0 mod 8
    IggyName name;
    IggyValueRef ref;
                                 //align 0 mod 8
    S32 index;
    S32 type;
                                 //align 0 mod 8
};

typedef enum
{
   IGGY_ValueRef,
   IGGY_ValueRef_Weak,
} IggyValueRefType;

RADEXPFUNC rrbool       RADEXPLINK IggyValueRefCheck(IggyValueRef ref);
RADEXPFUNC void         RADEXPLINK IggyValueRefFree(Iggy *p, IggyValueRef ref);
RADEXPFUNC IggyValueRef RADEXPLINK IggyValueRefFromPath(IggyValuePath *var, IggyValueRefType reftype);
RADEXPFUNC rrbool       RADEXPLINK IggyIsValueRefSameObjectAsTempRef(IggyValueRef value_ref, IggyTempRef temp_ref);
RADEXPFUNC rrbool       RADEXPLINK IggyIsValueRefSameObjectAsValuePath(IggyValueRef value_ref, IggyValuePath *path, IggyName sub_name, char const *sub_name_utf8);
RADEXPFUNC void         RADEXPLINK IggySetValueRefLimit(Iggy *f, S32 max_value_refs);
RADEXPFUNC S32          RADEXPLINK IggyDebugGetNumValueRef(Iggy *f);
RADEXPFUNC IggyValueRef RADEXPLINK IggyValueRefCreateArray(Iggy *f, S32 num_slots);
RADEXPFUNC IggyValueRef RADEXPLINK IggyValueRefCreateEmptyObject(Iggy *f);
RADEXPFUNC IggyValueRef RADEXPLINK IggyValueRefFromTempRef(Iggy *f, IggyTempRef temp_ref, IggyValueRefType reftype);

RADEXPFUNC IggyValuePath * RADEXPLINK IggyPlayerRootPath(Iggy *f);
RADEXPFUNC IggyValuePath * RADEXPLINK IggyPlayerCallbackResultPath(Iggy *f);
RADEXPFUNC rrbool RADEXPLINK IggyValuePathMakeNameRef(IggyValuePath *result, IggyValuePath *parent, char const *text_utf8);
RADEXPFUNC void RADEXPLINK IggyValuePathFromRef(IggyValuePath *result, Iggy *iggy, IggyValueRef ref);

RADEXPFUNC void RADEXPLINK IggyValuePathMakeNameRefFast(IggyValuePath *result, IggyValuePath *parent, IggyName name);
RADEXPFUNC void RADEXPLINK IggyValuePathMakeArrayRef(IggyValuePath *result, IggyValuePath *array_path, int array_index);

RADEXPFUNC void RADEXPLINK IggyValuePathSetParent(IggyValuePath *result, IggyValuePath *new_parent);            
RADEXPFUNC void RADEXPLINK IggyValuePathSetArrayIndex(IggyValuePath *result, int new_index);

RADEXPFUNC void RADEXPLINK IggyValuePathSetName(IggyValuePath *result, IggyName name);
RADEXPFUNC IggyResult RADEXPLINK IggyValueGetTypeRS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, IggyDatatype *result);

RADEXPFUNC IggyResult RADEXPLINK IggyValueGetF64RS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, F64 *result);
RADEXPFUNC IggyResult RADEXPLINK IggyValueGetF32RS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, F32 *result);
RADEXPFUNC IggyResult RADEXPLINK IggyValueGetS32RS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, S32 *result);
RADEXPFUNC IggyResult RADEXPLINK IggyValueGetU32RS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, U32 *result);
RADEXPFUNC IggyResult RADEXPLINK IggyValueGetStringUTF8RS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, S32 max_result_len, char *utf8_result, S32 *result_len);
RADEXPFUNC IggyResult RADEXPLINK IggyValueGetStringUTF16RS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, S32 max_result_len, IggyUTF16 *utf16_result, S32 *result_len);
RADEXPFUNC IggyResult RADEXPLINK IggyValueGetBooleanRS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, rrbool *result);
RADEXPFUNC IggyResult RADEXPLINK IggyValueGetArrayLengthRS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, S32 *result);

RADEXPFUNC rrbool RADEXPLINK IggyValueSetF64RS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, F64 value);
RADEXPFUNC rrbool RADEXPLINK IggyValueSetF32RS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, F32 value);
RADEXPFUNC rrbool RADEXPLINK IggyValueSetS32RS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, S32 value);
RADEXPFUNC rrbool RADEXPLINK IggyValueSetU32RS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, U32 value);
RADEXPFUNC rrbool RADEXPLINK IggyValueSetStringUTF8RS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, char const *utf8_string, S32 stringlen);
RADEXPFUNC rrbool RADEXPLINK IggyValueSetStringUTF16RS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, IggyUTF16 const *utf16_string, S32 stringlen);
RADEXPFUNC rrbool RADEXPLINK IggyValueSetBooleanRS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, rrbool value);
RADEXPFUNC rrbool RADEXPLINK IggyValueSetValueRefRS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, IggyValueRef value_ref);

RADEXPFUNC rrbool RADEXPLINK IggyValueSetUserDataRS(IggyValuePath *result, void const *userdata);
RADEXPFUNC IggyResult RADEXPLINK IggyValueGetUserDataRS(IggyValuePath *result, void **userdata);


////////////////////////////////////////////////////////////
//
// Input Events
//

typedef enum IggyEventType
{
   IGGY_EVENTTYPE_None,
   IGGY_EVENTTYPE_MouseLeftDown,
   IGGY_EVENTTYPE_MouseLeftUp,
   IGGY_EVENTTYPE_MouseRightDown,
   IGGY_EVENTTYPE_MouseRightUp,
   IGGY_EVENTTYPE_MouseMiddleDown,
   IGGY_EVENTTYPE_MouseMiddleUp,
   IGGY_EVENTTYPE_MouseMove,
   IGGY_EVENTTYPE_MouseWheel,
   IGGY_EVENTTYPE_KeyUp,
   IGGY_EVENTTYPE_KeyDown,
   IGGY_EVENTTYPE_Char,
   IGGY_EVENTTYPE_Activate,
   IGGY_EVENTTYPE_Deactivate,
   IGGY_EVENTTYPE_Resize,
   IGGY_EVENTTYPE_MouseLeave,
   IGGY_EVENTTYPE_FocusLost,
} IggyEventType;

typedef enum IggyKeyloc
{
   IGGY_KEYLOC_Standard = 0, // For keys that have no variants
   // TODO(casey): Shouldn't these work for ALT and CONTROL too?  The code in D3DTEST looks like it only handles VK_SHIFT...
   IGGY_KEYLOC_Left = 1, // Specifies the left-hand-side key for keys with left/right variants (such as $(IggyKeycode::IGGY_KEYCODE_SHIFT), $(IggyKeycode::IGGY_KEYCODE_ALTERNATE), etc.) */
   IGGY_KEYLOC_Right = 2, // Specifies the right-hand-side key for keys with left/right variants (such as $(IggyKeycode::IGGY_KEYCODE_SHIFT), $(IggyKeycode::IGGY_KEYCODE_ALTERNATE), etc.) */
   IGGY_KEYLOC_Numpad = 3, // TODO(casey): Is this ever used?
} IggyKeyloc;

typedef enum IggyKeyevent
{
   IGGY_KEYEVENT_Up = IGGY_EVENTTYPE_KeyUp,
   IGGY_KEYEVENT_Down = IGGY_EVENTTYPE_KeyDown,
} IggyKeyevent;

typedef enum IggyMousebutton
{
   IGGY_MOUSEBUTTON_LeftDown = IGGY_EVENTTYPE_MouseLeftDown,
   IGGY_MOUSEBUTTON_LeftUp = IGGY_EVENTTYPE_MouseLeftUp,
   IGGY_MOUSEBUTTON_RightDown = IGGY_EVENTTYPE_MouseRightDown,
   IGGY_MOUSEBUTTON_RightUp = IGGY_EVENTTYPE_MouseRightUp,
   IGGY_MOUSEBUTTON_MiddleDown = IGGY_EVENTTYPE_MouseMiddleDown,
   IGGY_MOUSEBUTTON_MiddleUp = IGGY_EVENTTYPE_MouseMiddleUp,
} IggyMousebutton;

typedef enum IggyActivestate
{
   IGGY_ACTIVESTATE_Activated = IGGY_EVENTTYPE_Activate,
   IGGY_ACTIVESTATE_Deactivated = IGGY_EVENTTYPE_Deactivate,
} IggyActivestate;

typedef enum IggyKeycode
{
   IGGY_KEYCODE_A                     = 65,
   IGGY_KEYCODE_B                     = 66,
   IGGY_KEYCODE_C                     = 67,
   IGGY_KEYCODE_D                     = 68,
   IGGY_KEYCODE_E                     = 69,
   IGGY_KEYCODE_F                     = 70,
   IGGY_KEYCODE_G                     = 71,
   IGGY_KEYCODE_H                     = 72,
   IGGY_KEYCODE_I                     = 73,
   IGGY_KEYCODE_J                     = 74,
   IGGY_KEYCODE_K                     = 75,
   IGGY_KEYCODE_L                     = 76,
   IGGY_KEYCODE_M                     = 77,
   IGGY_KEYCODE_N                     = 78,
   IGGY_KEYCODE_O                     = 79,
   IGGY_KEYCODE_P                     = 80,
   IGGY_KEYCODE_Q                     = 81,
   IGGY_KEYCODE_R                     = 82,
   IGGY_KEYCODE_S                     = 83,
   IGGY_KEYCODE_T                     = 84,
   IGGY_KEYCODE_U                     = 85,
   IGGY_KEYCODE_V                     = 86,
   IGGY_KEYCODE_W                     = 87,
   IGGY_KEYCODE_X                     = 88,
   IGGY_KEYCODE_Y                     = 89,
   IGGY_KEYCODE_Z                     = 90,

   IGGY_KEYCODE_0                     = 48,
   IGGY_KEYCODE_1                     = 49,
   IGGY_KEYCODE_2                     = 50,
   IGGY_KEYCODE_3                     = 51,
   IGGY_KEYCODE_4                     = 52,
   IGGY_KEYCODE_5                     = 53,
   IGGY_KEYCODE_6                     = 54,
   IGGY_KEYCODE_7                     = 55,
   IGGY_KEYCODE_8                     = 56,
   IGGY_KEYCODE_9                     = 57,

   IGGY_KEYCODE_F1                    = 112,
   IGGY_KEYCODE_F2                    = 113,
   IGGY_KEYCODE_F3                    = 114,
   IGGY_KEYCODE_F4                    = 115,
   IGGY_KEYCODE_F5                    = 116,
   IGGY_KEYCODE_F6                    = 117,
   IGGY_KEYCODE_F7                    = 118,
   IGGY_KEYCODE_F8                    = 119,
   IGGY_KEYCODE_F9                    = 120,
   IGGY_KEYCODE_F10                   = 121,
   IGGY_KEYCODE_F11                   = 122,
   IGGY_KEYCODE_F12                   = 123,
   IGGY_KEYCODE_F13                   = 124,
   IGGY_KEYCODE_F14                   = 125,
   IGGY_KEYCODE_F15                   = 126,

   IGGY_KEYCODE_COMMAND               = 15,
   IGGY_KEYCODE_SHIFT                 = 16,
   IGGY_KEYCODE_CONTROL               = 17,
   IGGY_KEYCODE_ALTERNATE             = 18,

   IGGY_KEYCODE_BACKQUOTE             = 192,
   IGGY_KEYCODE_BACKSLASH             = 220,
   IGGY_KEYCODE_BACKSPACE             = 8,
   IGGY_KEYCODE_CAPS_LOCK             = 20,
   IGGY_KEYCODE_COMMA                 = 188,
   IGGY_KEYCODE_DELETE                = 46,
   IGGY_KEYCODE_DOWN                  = 40,
   IGGY_KEYCODE_END                   = 35,
   IGGY_KEYCODE_ENTER                 = 13,
   IGGY_KEYCODE_EQUAL                 = 187,
   IGGY_KEYCODE_ESCAPE                = 27,
   IGGY_KEYCODE_HOME                  = 36,
   IGGY_KEYCODE_INSERT                = 45,
   IGGY_KEYCODE_LEFT                  = 37,
   IGGY_KEYCODE_LEFTBRACKET           = 219,
   IGGY_KEYCODE_MINUS                 = 189,
   IGGY_KEYCODE_NUMPAD                = 21,
   IGGY_KEYCODE_NUMPAD_0              = 96,
   IGGY_KEYCODE_NUMPAD_1              = 97,
   IGGY_KEYCODE_NUMPAD_2              = 98,
   IGGY_KEYCODE_NUMPAD_3              = 99,
   IGGY_KEYCODE_NUMPAD_4              = 100,
   IGGY_KEYCODE_NUMPAD_5              = 101,
   IGGY_KEYCODE_NUMPAD_6              = 102,
   IGGY_KEYCODE_NUMPAD_7              = 103,
   IGGY_KEYCODE_NUMPAD_8              = 104,
   IGGY_KEYCODE_NUMPAD_9              = 105,
   IGGY_KEYCODE_NUMPAD_ADD            = 107,
   IGGY_KEYCODE_NUMPAD_DECIMAL        = 110,
   IGGY_KEYCODE_NUMPAD_DIVIDE         = 111,
   IGGY_KEYCODE_NUMPAD_ENTER          = 108,
   IGGY_KEYCODE_NUMPAD_MULTIPLY       = 106,
   IGGY_KEYCODE_NUMPAD_SUBTRACT       = 109,
   IGGY_KEYCODE_PAGE_DOWN             = 34,
   IGGY_KEYCODE_PAGE_UP               = 33,
   IGGY_KEYCODE_PERIOD                = 190,
   IGGY_KEYCODE_QUOTE                 = 222,
   IGGY_KEYCODE_RIGHT                 = 39,
   IGGY_KEYCODE_RIGHTBRACKET          = 221,
   IGGY_KEYCODE_SEMICOLON             = 186,
   IGGY_KEYCODE_SLASH                 = 191,
   IGGY_KEYCODE_SPACE                 = 32,
   IGGY_KEYCODE_TAB                   = 9,
   IGGY_KEYCODE_UP                    = 38,
} IggyKeycode;

typedef enum IggyEventFlag
{
    IGGY_EVENTFLAG_PreventDispatchToObject = 0x1,
    IGGY_EVENTFLAG_PreventFocusTabbing = 0x2,
    IGGY_EVENTFLAG_PreventDefault = 0x4,
    IGGY_EVENTFLAG_RanAtLeastOneHandler = 0x8,
} IggyEventFlag;

typedef struct IggyEvent
{
   S32 type;  // an $IggyEventType
   U32 flags;
   S32 x,y; // mouse position at time of event
   S32 keycode,keyloc; // keyboard inputs
} IggyEvent;

typedef enum IggyFocusChange
{
    IGGY_FOCUS_CHANGE_None, // The keyboard focus didn't change
    IGGY_FOCUS_CHANGE_TookFocus, // The keyboard focus changed to something in this Iggy
    IGGY_FOCUS_CHANGE_LostFocus, // The keyboard focus was lost from this Iggy
} IggyFocusChange;

typedef struct IggyEventResult
{
    U32 new_flags;
    S32 focus_change; // an $IggyFocusChange that indicates how the focus (may have) changed in response to the event
    S32 focus_direction; //
} IggyEventResult;

RADEXPFUNC void RADEXPLINK IggyMakeEventNone(IggyEvent *event);

RADEXPFUNC void RADEXPLINK IggyMakeEventResize(IggyEvent *event);
RADEXPFUNC void RADEXPLINK IggyMakeEventActivate(IggyEvent *event, IggyActivestate event_type);
RADEXPFUNC void RADEXPLINK IggyMakeEventMouseLeave(IggyEvent *event);
RADEXPFUNC void RADEXPLINK IggyMakeEventMouseMove(IggyEvent *event, S32 x, S32 y);
RADEXPFUNC void RADEXPLINK IggyMakeEventMouseButton(IggyEvent *event, IggyMousebutton event_type);
RADEXPFUNC void RADEXPLINK IggyMakeEventMouseWheel(IggyEvent *event, S16 mousewheel_delta);
RADEXPFUNC void RADEXPLINK IggyMakeEventKey(IggyEvent *event, IggyKeyevent event_type, IggyKeycode keycode, IggyKeyloc keyloc);
RADEXPFUNC void RADEXPLINK IggyMakeEventChar(IggyEvent *event, S32 charcode);
RADEXPFUNC void RADEXPLINK IggyMakeEventFocusLost(IggyEvent *event);
RADEXPFUNC void RADEXPLINK IggyMakeEventFocusGained(IggyEvent *event, S32 focus_direction);
RADEXPFUNC rrbool RADEXPLINK IggyPlayerDispatchEventRS(Iggy *player, IggyEvent *event, IggyEventResult *result);
RADEXPFUNC void RADEXPLINK IggyPlayerSetShiftState(Iggy *f, rrbool shift, rrbool control, rrbool alt, rrbool command);
RADEXPFUNC void RADEXPLINK IggySetDoubleClickTime(S32 time_in_ms_from_first_down_to_second_up);
RADEXPFUNC void RADEXPLINK IggySetTextCursorFlash(U32 cycle_time_in_ms, U32 visible_time_in_ms);

RADEXPFUNC rrbool RADEXPLINK IggyPlayerHasFocusedEditableTextfield(Iggy *f);
RADEXPFUNC rrbool RADEXPLINK IggyPlayerPasteUTF16(Iggy *f, U16 *string, S32 stringlen);
RADEXPFUNC rrbool RADEXPLINK IggyPlayerPasteUTF8(Iggy *f, char *string, S32 stringlen);
RADEXPFUNC rrbool RADEXPLINK IggyPlayerCut(Iggy *f);

#define IGGY_PLAYER_COPY_no_focused_textfield       -1
#define IGGY_PLAYER_COPY_textfield_has_no_selection  0
RADEXPFUNC S32 RADEXPLINK IggyPlayerCopyUTF16(Iggy *f, U16 *buffer, S32 bufferlen);
RADEXPFUNC S32 RADEXPLINK IggyPlayerCopyUTF8(Iggy *f, char *buffer, S32 bufferlen);


////////////////////////////////////////////////////////////
//
// IME
//

#ifdef __RADNT__
#define IGGY_IME_SUPPORT
#endif

RADEXPFUNC void RADEXPLINK IggyPlayerSetIMEFontUTF8(Iggy *f, const char *font_name_utf8, S32 namelen_in_bytes);
RADEXPFUNC void RADEXPLINK IggyPlayerSetIMEFontUTF16(Iggy *f, const IggyUTF16 *font_name_utf16, S32 namelen_in_2byte_words);

#ifdef IGGY_IME_SUPPORT

#define IGGY_IME_MAX_CANDIDATE_LENGTH 256    // matches def in ImeUi.cpp, so no overflow checks needed when copying out.

IDOCN typedef enum {
    IGGY_IME_COMPOSITION_STYLE_NONE,
    IGGY_IME_COMPOSITION_STYLE_UNDERLINE_DOTTED,
    IGGY_IME_COMPOSITION_STYLE_UNDERLINE_DOTTED_THICK,
    IGGY_IME_COMPOSITION_STYLE_UNDERLINE_SOLID,
    IGGY_IME_COMPOSITION_STYLE_UNDERLINE_SOLID_THICK,
} IggyIMECompositionDrawStyle;

IDOCN typedef enum {
    IGGY_IME_COMPOSITION_CLAUSE_NORMAL,
    IGGY_IME_COMPOSITION_CLAUSE_START,
} IggyIMECompositionClauseState;

IDOCN typedef struct
{
    IggyUTF16 str[IGGY_IME_MAX_CANDIDATE_LENGTH];
    IggyIMECompositionDrawStyle   char_style[IGGY_IME_MAX_CANDIDATE_LENGTH];
    IggyIMECompositionClauseState clause_state[IGGY_IME_MAX_CANDIDATE_LENGTH];
    S32 cursor_pos;
    rrbool display_block_cursor;
    int candicate_clause_start_pos;
    int candicate_clause_end_pos;           // inclusive
} IggyIMECompostitionStringState;

IDOCN RADEXPFUNC void RADEXPLINK IggyIMEWin32SetCompositionState(Iggy* f, IggyIMECompostitionStringState* s);

IDOCN RADEXPFUNC void RADEXPLINK IggyIMEGetTextExtents(Iggy* f, U32* pdw, U32* pdh, const IggyUTF16* str, U32 text_height);
IDOCN RADEXPFUNC void RADEXPLINK IggyIMEDrawString(Iggy* f, S32 px, S32 py, const IggyUTF16* str, U32 text_height, const U8 rgba[4]);

IDOCN RADEXPFUNC void RADEXPLINK IggyIMEWin32GetCandidatePosition(Iggy* f, F32* pdx, F32* pdy, F32* pdcomp_str_height);
IDOCN RADEXPFUNC void* RADEXPLINK IggyIMEGetFocusedTextfield(Iggy* f);
IDOCN RADEXPFUNC void RADEXPLINK IggyIMEDrawRect(S32 x0, S32 y0, S32 x1, S32 y1, const U8 rgb[3]);

#endif

////////////////////////////////////////////////////////////
//
// Input focus handling
//

typedef void *IggyFocusHandle;

#define IGGY_FOCUS_NULL    0

typedef struct
{
   IggyFocusHandle object;          // unique identifier of Iggy object
   F32 x0, y0, x1, y1;              // bounding box of displayed shape
} IggyFocusableObject;

RADEXPFUNC rrbool RADEXPLINK IggyPlayerGetFocusableObjects(Iggy *f, IggyFocusHandle *current_focus,
                IggyFocusableObject *objs, S32 max_obj, S32 *num_obj);
RADEXPFUNC void RADEXPLINK IggyPlayerSetFocusRS(Iggy *f, IggyFocusHandle object, int focus_key_char);

////////////////////////////////////////////////////////////
//
// GDraw helper functions accessors
//

RADEXPFUNC void * RADEXPLINK IggyGDrawMalloc(SINTa size);
#define IggyGDrawMalloc(size)  IggyGDrawMallocAnnotated(size, __FILE__, __LINE__) IDOCN
IDOCN RADEXPFUNC void * RADEXPLINK IggyGDrawMallocAnnotated(SINTa size, const char *file, int line);

RADEXPFUNC void RADEXPLINK IggyGDrawFree(void *ptr);
RADEXPFUNC void RADEXPLINK IggyGDrawSendWarning(Iggy *f, char const *message, ...);
RADEXPFUNC void RADEXPLINK IggyWaitOnFence(void *id, U32 fence);
RADEXPFUNC void RADEXPLINK IggyDiscardVertexBufferCallback(void *owner, void *vertex_buffer);
RADEXPFUNC void RADEXPLINK IggyPlayerDebugEnableFilters(Iggy *f, rrbool enable);
RADEXPFUNC void RADEXPLINK IggyPlayerDebugSetTime(Iggy *f, F64 time);

IDOCN RADEXPFUNC void RADEXPLINK IggyPlayerDebugBatchStartFrame(void);
IDOCN RADEXPFUNC void RADEXPLINK IggyPlayerDebugBatchInit(void);
IDOCN RADEXPFUNC void RADEXPLINK IggyPlayerDebugBatchMove(S32 dir);
IDOCN RADEXPFUNC void RADEXPLINK IggyPlayerDebugBatchSplit(void);
IDOCN RADEXPFUNC void RADEXPLINK IggyPlayerDebugBatchChooseEnd(S32 end);

////////////////////////////////////////////////////////////
//
// debugging
//

IDOCN RADEXPFUNC void RADEXPLINK IggyPlayerDebugUpdateReadyToTickWithFakeRender(Iggy *f);
IDOCN RADEXPFUNC void RADEXPLINK IggyDebugBreakOnAS3Exception(void);


typedef struct
{
   S32 size;
   char *source_file;
   S32   source_line;
   char *iggy_file;
   char *info;
} IggyLeakResultData;

typedef void RADLINK IggyLeakResultCallback(IggyLeakResultData *data);

typedef struct
{
   char *subcategory;
   S32   subcategory_stringlen;

   S32   static_allocation_count;  // number of non-freeable allocations for this subcategory
   S32   static_allocation_bytes;  // bytes of non-freeable allocations for this subcategory

   S32   dynamic_allocation_count; // number of freeable allocations for this subcategory
   S32   dynamic_allocation_bytes; // estimated bytes of freeable allocations for this subcategory
} IggyMemoryUseInfo;

RADEXPFUNC rrbool RADEXPLINK IggyDebugGetMemoryUseInfo(Iggy *player, IggyLibrary lib, char const *category_string, S32 category_stringlen, S32 iteration, IggyMemoryUseInfo *data);
RADEXPFUNC void RADEXPLINK IggyDebugSetLeakResultCallback(IggyLeakResultCallback *leak_result_func);

IDOCN RADEXPFUNC void RADEXPLINK iggy_sync_check_todisk(char *filename_or_null, U32 flags);
IDOCN RADEXPFUNC void RADEXPLINK iggy_sync_check_fromdisk(char *filename_or_null, U32 flags);
IDOCN RADEXPFUNC void RADEXPLINK iggy_sync_check_end(void);
#define IGGY_SYNCCHECK_readytotick  1U  IDOCN

RADDEFEND

#endif
