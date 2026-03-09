#pragma once // 4J

// gdraw_orbis.h - author: Fabian Giesen - copyright 2012 RAD Game Tools
//
// Interface for creating an Orbis GDraw driver.

#include "gdraw.h"

#define IDOC
//idoc(parent,GDraw_orbis)

// Size and alignment requirements of GDraw context memory.
#define GDRAW_ORBIS_CONTEXT_MEM_SIZE            (64*1024)

// Alignment requirements for different resource types
#define GDRAW_ORBIS_TEXTURE_ALIGNMENT           256  // NOTE this may change in the future!
#define GDRAW_ORBIS_VERTEXBUFFER_ALIGNMENT      256  // NOTE this may change in the future!

typedef enum gdraw_orbis_resourcetype
{
   GDRAW_ORBIS_RESOURCE_rendertarget,        // put in video private memory if possible, must be GPU visible
   GDRAW_ORBIS_RESOURCE_texture,             // put in video private memory if possible, must be GPU visible
   GDRAW_ORBIS_RESOURCE_vertexbuffer,        // put in any GPU visible memory

   GDRAW_ORBIS_RESOURCE__count,
} gdraw_orbis_resourcetype;

typedef struct
{
   U32 allocs_attempted;         // number of allocations attempted from the staging buffer
   U32 allocs_succeeded;         // number of allocations that succeeded
   U32 bytes_attempted;          // number of bytes attempted to allocate
   U32 bytes_succeeded;          // number of bytes successfully allocated
   U32 largest_bytes_attempted;  // number of bytes in largest attempted alloc
   U32 largest_bytes_succeeded;  // number of bytes in lagrest successful alloc
} gdraw_orbis_staging_stats;

IDOC extern int gdraw_orbis_SetResourceMemory(gdraw_orbis_resourcetype type, S32 num_handles, void *ptr, S32 num_bytes);
/* Sets up the resource pools that GDraw uses for its video memory management.
   
   It sets both the number of handles and the address and size of memory to use.
   GDraw keeps track of allocations in each pool, and will free old resources in
   a LRU manner to make space if one of the limits is about to be exceeded. It will
   also automatically defragment memory if necessary to fulfill an allocation
   request.

   You need to set up all of the resource pools before you can start rendering.
   If you modify this at runtime, you need to call IggyPlayerFlushAll on all
   active Iggys (if any) since this call invalidates all resource handles they
   currently hold.

   SetResourceMemory takes a void* argument for the address of the resource pool.
   Pass in nullptr and zero bytes to reset a specific pool.

   Resource pool memory has certain alignment requirements - see the #defines
   above. If you pass in an unaligned pointer, GDraw will automatically clip off
   some bytes at the front to make the buffer aligned - in other words, you get
   somewhat less usable bytes, but it should work fine.

   "rendertarget" memory is only used during Iggy rendering. In other words, you
   are free to use that memory for other purposes such as your own render targets
   or depth buffers as long as it doesn't need to be preserved across IggyPlayerDraw*
   calls. 

   If any Iggy draw calls are in flight, this call will block waiting for
   those calls to finish (i.e. for the resource memory to become
   unused).
*/

IDOC extern void gdraw_orbis_ResetAllResourceMemory();
/* Frees all resource pools managed by GDraw.

   Use this as a quick way of freeing (nearly) all memory allocated by GDraw
   without shutting it down completely. For example, you might want to use this
   to quickly flush all memory allocated by GDraw when transitioning between the
   main menu and the game proper. Like with SetResourceMemory, you need to call
   IggyPlayerFlushAll on all currently active Iggy players if you do this - although
   we recommend that you only use this function when there aren't any. */

IDOC extern GDrawFunctions * gdraw_orbis_CreateContext(S32 w, S32 h, void *context_mem);
/* Creates a GDraw context for rendering using GNM. You need to pass in the width/height
   of the Iggy content for use by internal render targets. context_mem must point to an
   area in video shared memory that is GDRAW_ORBIS_CONTEXT_MEM_SIZE bytes big - this is
   used by GDraw to store labels and shaders in.

   There can only be one GDraw context active at any one time.

   If initialization fails for some reason (the main reason would be an out of memory condition),
   nullptr is returned. Otherwise, you can pass the return value to IggySetGDraw. */

IDOC extern void gdraw_orbis_DestroyContext(void);
/* Destroys the current GDraw context, if any.

   If any Iggy draw calls are in flight, this call will block waiting for
   those calls to finish (i.e. for the resource memory to become
   unused).
*/

IDOC extern void gdraw_orbis_Begin(sce::Gnmx::GfxContext *context, void *staging_buffer, U32 staging_buf_bytes);
/* This sets the GfxContext that GDraw writes its commands to. It also sets the
   address and size of the staging buffer used for dynamic vertex data and in-frame
   resource uploads. Any GDraw / Iggy rendering calls outside a Begin / End bracket
   are an error and will be treated as such. The GfxContext must be live during the
   entire Begin / End bracket.

   GDraw maintains a persistent resource cache shared across all Iggys. Because of this,
   it is *vital* that all GfxContexts generated from GDraw be kicked in the order
   they were generated, or the resource pools might get corrupted. Hence it's recommended
   to use only one GfxContext for all Iggy rendering during a frame.

   The staging buffer should be allocated in write-combined memory. If it is of insufficient
   size, GDraw will not be able to allocate dynamic vertex data or upload new texture/vertex
   buffer data during some frames, resulting in glitching! (When this happens, it will be
   reported as a warning, so make sure to install a warning callback).

   The user is expected to handle GfxContext and staging buffer synchronization; there are
   no safeguards on the GDraw side to prevent CPU-GPU race conditions. Please make sure that
   the GPU is finished with a GfxContext / staging buffer before reusing its memory!
   Typically, this would be accomplished by double buffering everything. */

IDOC extern void gdraw_orbis_End(gdraw_orbis_staging_stats *staging_stats);
/* This marks the end of GDraw rendering for a frame. It also triggers end-of-frame processing,
   which is important for GDraw's internal resource management. GDraw will not touch the GfxContext
   or staging buffer after this call, so you are free to append other rendering commands after
   this call returns.
   
   staging_stats will be filled with stats for the staging buffer, denoting how much memory
   was actually used and which allocations were attempted. If you're not interested, just
   pass nullptr. */

IDOC extern void gdraw_orbis_SetTileOrigin(sce::Gnm::RenderTarget *color, sce::Gnm::DepthRenderTarget *depth, S32 x, S32 y);
/* This sets the main color and depth buffers that GDraw should render to and the
   x/y position of the output location of the top-left pixel of the current tile
   (to be used for tiled rendering).
   
   You should call this inside a gdraw_orbis_Begin / gdraw_orbis_End bracket, before
   any Iggy / GDraw rendering takes place. */

IDOC extern void gdraw_orbis_ClearWholeRenderTarget(const F32 clear_color_rgba[4]);
/* You typically need to clear the render target before you start Iggy rendering.

   This is a convenience function, if you don't want to do the clear yourself. Fast clears
   will be used if the render target has them enabled. Note that if fast clears are enabled,
   you also need to call gdraw_orbis_EliminateFastClears once you're done with the render target
   to make sure the clears are actually properly completed.

   This counts as a rendering operation, so it must be called inside a
   gdraw_orbis_Begin / gdraw_orbis_End bracket, after gdraw_orbis_SetTileOrigin has been called. */

IDOC extern void gdraw_orbis_EliminateFastClears(void);
/* If the render target specified in gdraw_orbis_SetTileOrigin has fast clears enabled, you
   need to do a post-process step to make sure the clears get properly computed. This function
   performs that step. If your render target doesn't have fast clears enabled, it simply does
   nothing.

   This counts as a rendering operation, so it must be called inside a
   gdraw_orbis_Begin / gdraw_orbis_End bracket, after gdraw_orbis_SetTileOrigin has been called. */

IDOC extern void RADLINK gdraw_orbis_CalculateCustomDraw_4J(IggyCustomDrawCallbackRegion *region, float matrix[16]);
IDOC extern void RADLINK gdraw_orbis_BeginCustomDraw(IggyCustomDrawCallbackRegion *region, float matrix[16]);
/* Call at the beginning of Iggy custom draw callback to clear any odd render states GDraw has
   set, and to get the current 2D object-to-world transformation. */

IDOC extern void RADLINK gdraw_orbis_EndCustomDraw(IggyCustomDrawCallbackRegion *region);
/* Call at the end of Iggy custom draw callback so GDraw can restore its render states. */

IDOC extern GDrawTexture *gdraw_orbis_WrappedTextureCreate(sce::Gnm::Texture *tex);
/* Create a wrapped texture from a GNM texture.
   A wrapped texture can be used to let Iggy draw using the contents of a texture
   you create and manage on your own. For example, you might render to this texture,
   or stream video into it. Wrapped textures take up a handle. They will never be
   freed or otherwise modified by GDraw; nor will GDraw change any reference counts.
   All this is up to the application.
   GDraw makes a copy of the contents of the Gnm::Texture (the contents of the struct
   that is, not the data it points to). If you later modify the fields of "tex", you
   need to call $gdraw_orbis_WrappedTextureChange.
   */

IDOC extern void gdraw_orbis_WrappedTextureChange(GDrawTexture *handle, sce::Gnm::Texture *tex);
/* Switch an existing GDrawTexture * that represents a wrapped texture to use
   a new underlying GNM texture. For example, you might internally double-buffer
   a dynamically updated texture. As above, GDraw will leave this texture alone
   and not touch any reference counts. */

IDOC extern void gdraw_orbis_WrappedTextureDestroy(GDrawTexture *handle);
/* Destroys the GDraw wrapper for a wrapped texture object. This will free up
   a GDraw texture handle but not release the associated GNM texture; that is
   up to you. */

IDOC extern GDrawTexture * RADLINK gdraw_orbis_MakeTextureFromResource(U8 *file_in_memory, S32 length, IggyFileTexturePS4 *tex);
/* Sets up a texture loaded from a .sekrit2.iggytex file. */

extern void RADLINK gdraw_orbis_DestroyTextureFromResource(GDrawTexture *tex);

// 4J added
extern void RADLINK gdraw_orbis_setViewport_4J();