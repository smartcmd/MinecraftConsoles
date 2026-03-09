// gdraw_psp2.h - author: Fabian Giesen - copyright 2014 RAD Game Tools
//
// Interface for creating a PSP2 GDraw driver.

#include "gdraw.h"

#define IDOC
//idoc(parent,GDraw_psp2)

// Size and alignment requirements of GDraw context memory.
#define GDRAW_PSP2_CONTEXT_MEM_SIZE             (16*1024)

// Alignment requirements for different resource types (in bytes)
#define GDRAW_PSP2_TEXTURE_ALIGNMENT            16
#define GDRAW_PSP2_VERTEXBUFFER_ALIGNMENT       16

typedef enum gdraw_psp2_resourcetype
{
   GDRAW_PSP2_RESOURCE_texture,
   GDRAW_PSP2_RESOURCE_vertexbuffer,

   GDRAW_PSP2_RESOURCE__count,
} gdraw_psp2_resourcetype;

typedef struct
{
   U32 allocs_attempted;         // number of allocations attempted from the staging buffer
   U32 allocs_succeeded;         // number of allocations that succeeded
   U32 bytes_attempted;          // number of bytes attempted to allocate
   U32 bytes_succeeded;          // number of bytes successfully allocated
   U32 largest_bytes_attempted;  // number of bytes in largest attempted alloc
   U32 largest_bytes_succeeded;  // number of bytes in lagrest successful alloc
} gdraw_psp2_dynamic_stats;

typedef struct
{
   void *start;         // pointer to the start of the buffer
   U32 size_in_bytes;   // size of the buffer in bytes
   U64 sync;            // used internally by GDraw for synchronization.

   gdraw_psp2_dynamic_stats stats; // stats on buffer usage - these are for your benefit!
} gdraw_psp2_dynamic_buffer;

IDOC extern void gdraw_psp2_InitDynamicBuffer(gdraw_psp2_dynamic_buffer *buffer, void *ptr, U32 num_bytes);
/* Initializes a GDraw dynamic buffer struct.

   The "dynamic buffer" is where GDraw stores all transient data for a scene - dynamic
   vertex and index data, uniform buffers and the like. We wrap them in a struct
   so we can handle synchronization with the GPU.

   "ptr" should point to non-cached, GPU-mapped readable memory. "num_bytes" is the size of
   the buffer in bytes. */

IDOC extern void gdraw_psp2_WaitForDynamicBufferIdle(gdraw_psp2_dynamic_buffer *buffer);
/* Waits until a GDraw dynamic buffer is idle, i.e. not being used by the
   GPU anymore. You need to call this if you intend to free the allocated storage. */

IDOC extern int gdraw_psp2_SetResourceMemory(gdraw_psp2_resourcetype type, S32 num_handles, void *ptr, S32 num_bytes);
/* Sets up the resource pools that GDraw uses for its video memory management.
   
   It sets both the number of handles and the address and size of memory to use.
   GDraw keeps track of allocations in each pool, and will free old resources in
   a LRU manner to make space if one of the limits is about to be exceeded. It will
   also automatically defragment memory if necessary to fulfill an allocation
   request.

   "ptr" points to the address of the resource pool. This memory needs to be
   mapped to the GPU and *writeable*. If it isn't, the GPU will crash during
   either this function or CreateContext!

   Pass in nullptr for "ptr" and zero "num_bytes" to free the memory allocated to
   a specific pool.

   GDraw can run into cases where resource memory gets fragmented; we defragment
   automatically in that case. However, to make this work, GDraw on PSP2 needs
   resource pool memory equivalent to *twice* the largest working set in any
   scene. So for example, if you use 10MB worth of textures, GDraw needs at least
   a 20MB texture pool! On other platforms, we can avoid this extra cost by
   draining the GPU pipeline in the middle of a frame in certain rare cases, but
   doing so on PSP2 would mean not supporting deferred contexts.

   You need to set up all of the resource pools before you can start rendering.
   If you modify this at runtime, you need to call IggyPlayerFlushAll on all
   active Iggys (if any) since this call invalidates all resource handles they
   currently hold.

   Resource pool memory has certain alignment requirements - see the #defines
   above. If you pass in an unaligned pointer, GDraw will automatically clip off
   some bytes at the front to make the buffer aligned - in other words, you get
   somewhat less usable bytes, but it should work fine.

   If any Iggy draw calls are in flight, this call will block waiting for
   those calls to finish (i.e. for the resource memory to become
   unused).
*/

IDOC extern void gdraw_psp2_ResetAllResourceMemory();
/* Frees all resource pools managed by GDraw.

   Use this as a quick way of freeing (nearly) all memory allocated by GDraw
   without shutting it down completely. For example, you might want to use this
   to quickly flush all memory allocated by GDraw when transitioning between the
   main menu and the game proper. Like with SetResourceMemory, you need to call
   IggyPlayerFlushAll on all currently active Iggy players if you do this - although
   we recommend that you only use this function when there aren't any. */

IDOC extern GDrawFunctions * gdraw_psp2_CreateContext(SceGxmShaderPatcher *shader_patcher, void *context_mem, volatile U32 *notification, SceGxmOutputRegisterFormat reg_format);
/* Creates a GDraw context for rendering using GXM. You need to pass in a pointer to
   the shader patcher to use, a pointer to "context memory" which holds a few persistent
   resources shared by all Iggys (it needs to hold GDRAW_PSP2_CONTEXT_MEM_SIZE bytes
   and must be mapped to be GPU readable), and a pointer to the notificaiton to use for
   GDraw sync.

   "reg_format" specifies the output register format to specify when creating
   GDraw's fragment shaders. This should match your color surface. Typical choices
   are:
   - SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4 (32-bit output register size in color surface)
   - SCE_GXM_OUTPUT_REGISTER_FORMAT_HALF4 (64-bit output register size in color surface)

   There can only be one GDraw context active at any one time. The shader_patcher must
   be valid for as long as a GDraw context is alive.

   If initialization fails for some reason (the main reason would be an out of memory condition),
   nullptr is returned. Otherwise, you can pass the return value to IggySetGDraw. */

IDOC extern void gdraw_psp2_DestroyContext(void);
/* Destroys the current GDraw context, if any.

   If any Iggy draw calls are in flight, this call will block waiting for
   those calls to finish (i.e. for the resource memory to become
   unused).
*/

IDOC extern void gdraw_psp2_Begin(SceGxmContext *context, const SceGxmColorSurface *color, const SceGxmDepthStencilSurface *depth,
                                  gdraw_psp2_dynamic_buffer *dynamic_buffer);
/* This sets the SceGxmContext that GDraw writes its commands to. It also specifies
   the dynamic buffer to use. Any GDraw / Iggy rendering calls outside a
   Begin / End bracket are an error and will be treated as such. The GXM context
   and dynamic buffer must be live during the entire Begin / End bracket.

   NOTE: If you are passing in a deferred context, see important notes below!

   GDraw uses the color and depth surfaces for parameter validation. Make sure to
   pass in the same values you passed in to sceGxmBeginScene. Also, inside a given
   scene, you may have only *one* $gdraw_psp2_Begin / $gdraw_psp2_End pair.

   GDraw maintains a persistent resource cache shared across all Iggys. Because of this,
   it is *vital* that all command lists generated from GDraw be executed in the order
   they were generated, or the resource pools might get corrupted.

   If the dynamic buffer is of insufficient size, GDraw will not be able to allocate dynamic
   vertex data or upload new texture/vertex buffer data during some frames, resulting in
   glitching! (When this happens, it will be reported as a warning, so make sure to install
   a warning callback).

   You should use multiple dynamic buffers (at least double-buffer it); otherwise,
   GDraw needs to stall every frame to wait for the GPU to process the previous one.

   GDraw starts no scenes of its own; you must call BeginScene before you issue
   gdraw_psp2_Begin.

   If you are a using a deferred context:
   --------------------------------------

   It's allowed to pass in a deferred context as "context". You may perform Iggy
   rendering on a separate thread. However, because there is only a single global
   resource pool that is modified directly by GDraw, rendering is *not* thread-safe;
   that is, you may do Iggy rendering on any thread, but only a single thread may
   be inside a GDraw Begin / End bracket at any given time.

   Furthermore, if you use a deferred context, you must use (and execute!) the
   resulting command list and end the containing scene *before* you call
   $gdraw_psp2_Begin again. That is, usage must look like this:

   Main thread:                              |  Other thread:
     <other work>                            |    gdraw_psp2_Begin(deferred_ctx, ...)
                                             |    <render Iggys>
                                             |    gdraw_psp2_End(deferred_ctx, ...)
                                             |    ...
                                             |    sceGxmEndCommandList(deferred_ctx, &cmd_list)
                                             | 
     sceGxmBeginScene(...)                   |    <other work>
     sceGxmExecuteCommandList(..., cmd_list) | 
     sceGxmEndScene()                        |

   The second thread may *not* start another $gdraw_psp2_Begin before the main thread
   calls sceGxmEndScene().

   This is inconvenient, but unfortunately required by our resource management: every
   GDraw Begin / End Bracket may end up having to wait for the GPU to finish work done
   during a previous bracket, and this only works if the corresponding jobs have been
   submitted to the GPU by the point $gdraw_psp2_Begin is called!
*/

IDOC extern SceGxmNotification gdraw_psp2_End();
/* This marks the end of GDraw rendering for a frame. It also triggers end-of-frame processing,
   which is important for GDraw's internal resource management. GDraw will not touch the GXM context
   or staging buffer after this call, so you are free to append other rendering commands after
   this call returns.

   This function will also update the "stats" field in the dynamic buffer you passed
   to "Begin".
   
   You are *required* to pass the returned SceGxmNotification as your "fragment notification"
   when calling sceGxmEndScene. This is necessary to make GDraw's resource management work! */

IDOC extern void gdraw_psp2_SetTileOrigin(S32 x, S32 y);
/* This sets the x/y position of the output location of the top-left pixel of the current
   tile. Iggy has support for manual splitting of rendering into multiple tiles; PSP2 is
   already a tiled renderer that handles all this in hardware, so in practice
   you will probably always pass (0,0) here.
   
   You should call this inside a gdraw_psp2_Begin / gdraw_psp2_End bracket, before
   any Iggy / GDraw rendering takes place. */

IDOC extern void gdraw_psp2_ClearBeforeNextRender(const F32 clear_color_rgba[4]);
/* You often want to clear the render target before you start Iggy rendering.

   This is a convenience function, if you don't want to do the clear yourself.
   Iggy always clears the depth/stencil buffers when it starts rendering; if you
   call this function first, it will also clear the color surface to the
   specified color during that initial clear. If this function is not called
   before rendering, GDraw will leave the contents of the color surface alone. 
   
   This function does not do any rendering; it just sets some internal state
   that GDraw processes when it starts rendering. That state gets reset for every
   call to IggyPlayerDraw or IggyPlayerDrawTile. */

IDOC extern void RADLINK gdraw_psp2_CalculateCustomDraw_4J(IggyCustomDrawCallbackRegion *region, float matrix[16]);
IDOC extern void RADLINK gdraw_psp2_BeginCustomDraw(IggyCustomDrawCallbackRegion *region, float matrix[16]);
/* Call at the beginning of Iggy custom draw callback to clear any odd render states GDraw has
   set, and to get the current 2D object-to-world transformation. */

IDOC extern void RADLINK gdraw_psp2_EndCustomDraw(IggyCustomDrawCallbackRegion *region);
/* Call at the end of Iggy custom draw callback so GDraw can restore its render states. */

IDOC extern GDrawTexture *gdraw_psp2_WrappedTextureCreate(SceGxmTexture *tex);
/* Create a wrapped texture from a GXM texture.
   A wrapped texture can be used to let Iggy draw using the contents of a texture
   you create and manage on your own. For example, you might render to this texture,
   or stream video into it. Wrapped textures take up a handle. They will never be
   freed or otherwise modified by GDraw; nor will GDraw change any reference counts.
   All this is up to the application.
   GDraw makes a copy of the contents of the sceGxmTexture (the contents of the struct
   that is, not the data it points to). If you later modify the fields of "tex", you
   need to call $gdraw_psp2_WrappedTextureChange.
   */

IDOC extern void gdraw_psp2_WrappedTextureChange(GDrawTexture *handle, SceGxmTexture *tex);
/* Switch an existing GDrawTexture * that represents a wrapped texture to use
   a new underlying GXM texture. For example, you might internally double-buffer
   a dynamically updated texture. As above, GDraw will leave this texture alone
   and not touch any reference counts. */

IDOC extern void gdraw_psp2_WrappedTextureDestroy(GDrawTexture *handle);
/* Destroys the GDraw wrapper for a wrapped texture object. This will free up
   a GDraw texture handle but not release the associated GXM texture; that is
   up to you. */

IDOC extern GDrawTexture * RADLINK gdraw_psp2_MakeTextureFromResource(U8 *file_in_memory, S32 length, IggyFileTexturePSP2 *tex);
/* Sets up a texture loaded from a .psp2.iggytex file. */

extern void RADLINK gdraw_psp2_DestroyTextureFromResource(GDrawTexture *tex);

