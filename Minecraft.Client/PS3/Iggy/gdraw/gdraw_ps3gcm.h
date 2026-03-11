#pragma once
// gdraw_ps3gcm.h - copyright 2010-2011 RAD Game Tools
//
// Interface for creating a GCM GDraw driver.

#include "gdraw.h"

#define IDOC
//idoc(parent, GDraw_ps3gcm)

typedef enum gdraw_gcm_resourcetype
{
   GDRAW_GCM_RESOURCE_texture,
   GDRAW_GCM_RESOURCE_vertexbuffer,
   GDRAW_GCM_RESOURCE_dyn_vertexbuffer,

   GDRAW_GCM_RESOURCE__count,
} gdraw_gcm_resourcetype;

IDOC extern int gdraw_GCM_SetResourceMemory(gdraw_gcm_resourcetype type, S32 num_handles, void *ptr, S32 num_bytes);
/* This sets up the resource pools that GDraw uses for its video memory management.
   
   It sets both the number of handles and the address and size of memory to use.
   GDraw keeps track of allocations in each pool, and will free old resources in
   a LRU manner to make space if one of the limits is about to be exceeded. It will
   also automatically defragment memory if necessary to fulfill an allocation
   request. The pointer should be either to RSX local memory or to RSX-mapped
   main memory. It also needs to be aligned to a cache line boundary (128 bytes).

   You need to set up all of the resource pools before you can start rendering.
   If you modify this at runtime, you need to call IggyPlayerFlushAll on all
   active Iggys (if any) since this call invalidates all resource handles they
   currently hold.

   If any Iggy draw calls are in flight, this call will block waiting for
   those calls to finish (i.e. for the resource memory to become
   unused).
*/
   

IDOC extern int gdraw_GCM_SetRendertargetMemory(void *ptr, S32 num_bytes, S32 width, S32 height, S32 pitch);
/* Specifies the start address, size and physical dimensions of the region used to allocate render targets from.

   All render targets allocated by GDraw have the same size (width x height pixels) and will use the same pitch;
   thus you can pack them all into one tile region, which is recommended from a performance perspective.
   Pitch must allow for 4-byte-per-pixel rendertargets (so pitch must be at least width*4).
   
   On other platforms, Iggy does not require you to correctly specify the width and height of the
   rendertargets; the size you specify is a hint, and GDraw will detect the actual
   needed size and reallocate the rendertargets to be large enough as needed.
   Because of the need to set pitch accurately, this is not true on the PS3; instead,
   the width and height you specify here _must_ be sufficiently large. To be explicit,
   each of width and height be as large as the largest Iggy you will render in that
   dimension, or if you're rendering with tiles, the largest tile you'll render,
   including padding (possibly on both sides).

   For example, if all you ever render is a 400x300 Iggy, you could call this
   with width,height=300. If all you render is a 1920x1080 Iggy split into two
   horizontal tiles with 8 pixels of padding, you could call this with
   width=960+8*2, height=1080+8*2. (This is assuming there is padding on all
   sides of the tile; in practice, with this particular arrangement, there is
   only ever one column of horizontal padding, so technically it is large
   enough to use <960+8,1080>. I prefer to just always pad all sides so that
   I don't have to worry about how things change depending on 3-tiles-in-a-row
   versus 4-quadrant-tiles, but this does waste some memory.)

   If you render multiple Iggys in one frame (or, generally, during a single "major mode",
   since you don't want to reallocate rendertargets on the fly), you must take the max
   of each dimension. For example, if you were to render:
      $* A 500x200 Iggy
      $* An 800x100 Iggy
      $* One Iggy using two 400x300 tiles, each with 8 pixels of padding
   Then the necessary width would be max(500,800,300+8*2) and the necessary height
   would be max(200,100,400+8*2); in other words, you would need to call this
   function with width and height no smaller than <800,416>.

   You can use this memory for your own purposes as well. IggyPlayerDraw* calls will write to it,
   and you should not touch it _during_ IggyDraw calls. GDraw only touches this memory from
   the GPU, so you must only access it from the GPU.

   Calling this function causes GDraw to wait for all outstanding GDraw operations
   to drain from the GPU pipeline.
*/

IDOC extern void gdraw_GCM_ResetAllResourceMemory();
/* Frees all resource pools managed by GDraw. (This includes rendertargets)

   Use this as a quick way of freeing (nearly) all memory allocated by GDraw
   without shutting it down completely. For example, you might want to use this
   to quickly flush all memory allocated by GDraw when transitioning between the
   main menu and the game proper. Like with SetResourceMemory, you need to call
   IggyPlayerFlushAll on all currently active Iggy players if you do this - although
   we recommend that you only use this function when there aren't any. */

#define GDRAW_GCM_LOCAL_WORKMEM_SIZE (64*1024)
// GDraw needs some amount of RSX local memory for itself, to allocate fragment programs and other system resources in.

IDOC extern GDrawFunctions * gdraw_GCM_CreateContext(CellGcmContextData *gcm, void *local_workmem, U8 rsx_label_index);
/* Creates a GDraw context for rendering using GCM. You need to pass in the GcmContextData
   (this needs to be your main command buffer; GDraw does its own resource management,
   inserts sync commands, and expects them to be executed at some point!).

   You also need to pass a pointer to GDRAW_GCM_LOCAL_WORKMEM_SIZE bytes of RSX
   local memory; GDraw stores its fragment programs and some other small resources
   there. Finally, you need to pass in the number of a free RSX label index to be
   used by GDraw.

   There can only be one GCM GDraw context active at any one time.

   If initialization fails for some reason (the main reason would be an out of memory condition),
   nullptr is returned. Otherwise, you can pass the return value to IggySetGDraw. */

IDOC extern void gdraw_GCM_DestroyContext(void);
/* Destroys the current GDraw context, if any. */

IDOC extern void gdraw_GCM_SetTileOrigin(CellGcmSurface *rt, S32 x, S32 y);
/* This sets the surface that GDraw should render to and the x/y position of the
   output location of the top-left pixel of the current tile (to be used for tiled rendering).

   The main rendertarget needs to be non-multisampled ARGB with 32 bits/pixel;
   multisampling is not currently supported (nor is it necessary, as Iggy does its
   own antialiasing). The Z-buffer needs to be 24-bit depth with 8 bits of stencil.
   Rendertargets 1-3 should be disabled (i.e. no MRT).

   You need to call this before Iggy calls any rendering functions. */

IDOC extern void gdraw_GCM_NoMoreGDrawThisFrame(void);
/* Tells GDraw that no more rendering operations will occur this frame. This triggers
   some end-of-frame processing, which is important for GDraws resource management to
   work, so please do not forget to call this every frame! (As long as Iggy does any
   rendering, that is) */

IDOC extern void RADLINK gdraw_GCM_CalculateCustomDraw_4J(IggyCustomDrawCallbackRegion *Region, F32 mat[16]);
IDOC extern void RADLINK gdraw_GCM_BeginCustomDraw(IggyCustomDrawCallbackRegion *Region, float *mat);
/* Call at the beginning of Iggy custom draw callback to clear any odd render states GDraw has
   set on the RSX, and to get the current 2D object-to-world transformation. */

IDOC extern void RADLINK gdraw_GCM_EndCustomDraw(IggyCustomDrawCallbackRegion *Region);
/* Call at the end of Iggy custom draw callback so GDraw can restore its render states. */

IDOC extern GDrawTexture *gdraw_GCM_WrappedTextureCreate(CellGcmTexture *gcm_tex);
/* Create a wrapped texture from a CellGcmTexture.
   A wrapped texture can be used to let Iggy draw using the contents of a texture
   you create and manage on your own. For example, you might render to this texture,
   or stream video into it. Wrapped textures take up a handle. They will never be
   freed or otherwise modified by GDraw; object lifetime management is up to you. */

IDOC extern void gdraw_GCM_WrappedTextureChange(GDrawTexture *tex, CellGcmTexture *gcm_tex);
/* Switch an existing GDrawTexture * that represents a wrapped texture to use
   a new underlying GCM texture. For example, you might internally double-buffer
   a dynamically updated texture. As above, GDraw will leave this texture alone
   and not do any lifetime management. */

IDOC extern void gdraw_GCM_WrappedTextureDestroy(GDrawTexture *tex);
/* Destroys the GDraw wrapper for a wrapped texture object. This will free up
   a GDraw texture handle, nothing else. */

extern GDrawTexture * RADLINK gdraw_GCM_MakeTextureFromResource(U8 *resource_file, S32 length, IggyFileTexturePS3 *texture);
extern void RADLINK gdraw_GCM_DestroyTextureFromResource(GDrawTexture *tex);
