// gdraw_psp2.cpp - author: Fabian Giesen - copyright 2014 RAD Game Tools
//
// This implements the Iggy graphics driver layer for PSP2.

// GDraw consists of several components that interact fairly loosely with each other;
// e.g. the resource management, drawing and filtering parts are all fairly independent
// of each other. If you want to modify some aspect of GDraw - say the texture allocation
// logic - your best bet is usually to just look for one of the related entry points,
// e.g. MakeTextureBegin, and take it from there. There's a bunch of code in this file,
// but most of it isn't really complicated. The bits that are somewhat tricky have a more
// detailed explanation at the top of the relevant section.
 
#include <kernel.h>
#include <gxm.h>
#include <arm_neon.h>
#include <math.h>
#include <string.h>
#include "iggy.h"
#include "gdraw.h"

#include "gdraw_psp2.h"

typedef union {
   struct {
      SceGxmTexture *gxm;
   } tex;

   struct {
      void *verts;
      void *inds;
   } vbuf;
} GDrawNativeHandle;

#define GDRAW_MANAGE_MEM
#define GDRAW_MANAGE_MEM_TWOPOOL
#define GDRAW_NO_BLURS
#define GDRAW_MIN_FREE_AMOUNT    (64*1024)   // always try to free at least this many bytes when throwing out old textures
#define GDRAW_MAYBE_UNUSED       __attribute__((unused))
#include "gdraw_shared.inl"

#define MAX_SAMPLERS             1
#define AATEX_SAMPLER            1           // sampler that aa_tex gets set in
#define QUAD_IB_COUNT            1024        // quad index buffer has indices for this many quads

#define ASSERT_COUNT(a,b)        ((a) == (b) ? (b) : -1)

#define MAX_TEXTURE2D_DIM        4096
#define MAX_AATEX_WIDTH          64
#define GPU_MEMCPY_ALIGN         16          // bytes

static GDrawFunctions gdraw_funcs;

struct ShaderCode
{
   void *blob;
   union
   {
      void *dummy;
      SceGxmShaderPatcherId id;
   };
   bool registered;
};

// Canonical blends
enum gdraw_canonical_blend
{
   GDRAW_CBLEND_none,      // direct copy
   GDRAW_CBLEND_alpha,     // premultiplied alpha
   GDRAW_CBLEND_add,       // add
   GDRAW_CBLEND_nowrite,   // color writes disabled

   GDRAW_CBLEND__count
};

enum gdraw_outstanding_transfer
{
   GDRAW_TRANSFER_texture = 1 << 0,
   GDRAW_TRANSFER_vertex = 1 << 1,
};

///////////////////////////////////////////////////////////////////////////////
//
//     GDraw data structure
//
//
// This is the primary rendering abstraction, which hides all
// the platform-specific rendering behavior from Iggy. It is
// full of platform-specific graphics state, and also general
// graphics state so that it doesn't have to callback into Iggy
// to get at that graphics state.

struct GDraw
{
   // 16-byte aligned!
   F32 projection[4]; // always 2D scale+2D translate. first two are scale, last two are translate.

   // scale factor converting worldspace to viewspace <0,0>..<w,h>
   F32 world_to_pixel[2];

   // graphics context
   SceGxmContext *gxm;

   // cached state
   U32 scissor_state;      // 0=disabled, 1=enabled.
   U32 z_stencil_key;      // field built from z/stencil test flags. 0 = no z/stencil test, ~0 is used for "unknown state"

   gswf_recti cur_scissor;

   GDrawTexture *active_tex[MAX_SAMPLERS];
   SceGxmFragmentProgram *cur_fp;
   SceGxmVertexProgram *cur_vp;

   // viewport setting (in pixels) for the current tile
   S32 vx, vy;
   S32 fw, fh; // full width/height of virtual display
   S32 tw, th; // actual width/height of current tile
   S32 tpw, tph; // width/height of padded version of tile

   S32 tx0, ty0;
   S32 tx0p, ty0p;

   S32 tx0v, ty0v; // tile bounds relative to tile origin

   struct {
      S32 x0, y0, x1, y1;
   } cview; // current viewport

   gswf_recti screen_bounds;

   SceGxmTexture aa_tex;
   GDrawArena context_arena;
   
   // texture and vertex buffer pools
   GDrawHandleCache *texturecache;
   GDrawHandleCache *vbufcache;

   // dynamic data buffer
   GDrawArena dynamic;
   gdraw_psp2_dynamic_stats dynamic_stats;
   gdraw_psp2_dynamic_buffer *dyn_buf;

   // fragment programs
   SceGxmFragmentProgram *main_fp[GDRAW_TEXTURE__count][3][GDRAW_CBLEND__count]; // [texmode][additive][cblend]
   SceGxmFragmentProgram *clear_fp;
   SceGxmFragmentProgram *mask_update_fp;

   // vertex programs
   SceGxmVertexProgram *vp[GDRAW_vformat__basic_count];
   SceGxmVertexProgram *mask_vp;

   // baked index buffers
   U16 *quad_ib;
   U16 *mask_ib;

   // precomputed mask draw
   SceGxmPrecomputedDraw mask_draw;
   void *mask_draw_gpu;

   // synchronization
   volatile U32 *fence_label;
   U64 next_fence_index;
   GDrawFence scene_end_fence;

   // mipmapping
   GDrawMipmapContext mipmap;

   // shader patcher
   SceGxmShaderPatcher *patcher;

   // clear color
   F32 clear_color_rgba[4];
   const F32 *next_tile_clear;

   // transfers
   U32 outstanding_transfers;
   U32 draw_transfer_flush_mask;
};

static GDraw *gdraw;
static const F32 four_zeros[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

////////////////////////////////////////////////////////////////////////
//
//   Synchronization, pointer wrangling and command buffer management
//

static RADINLINE GDrawFence get_next_fence()
{
   GDrawFence fence;
   fence.value = gdraw->next_fence_index;
   return fence;
}

static SceGxmNotification scene_end_notification()
{
   SceGxmNotification n;
   n.address = (uint32_t *)gdraw->fence_label;
   n.value = (uint32_t)gdraw->next_fence_index;
   gdraw->scene_end_fence.value = gdraw->next_fence_index;
   gdraw->next_fence_index++;
   return n;
}

static RADINLINE rrbool is_fence_pending(GDrawFence fence)
{
   // if it's older than one full wrap of the fence counter,
   // we know it's retired. (can't have >=4 billion frames in
   // flight!)
   if (gdraw->next_fence_index - fence.value > 0xffffffffu)
      return false;

   // this is how far the GPU is.
   U32 retired = *gdraw->fence_label;

   // everything between "retired" (exclusive) and
   // "next_fence_index" (inclusive) is pending. everything else
   // is definitely done.
   //
   // this is a bit subtle; it uses unsigned wraparound to handle
   // the edge case where we've wrapped around recently.
   
   // number of pending fences (next_fence_index hasn't been submitted yet!)
   U32 num_pending = (U32)gdraw->next_fence_index - retired;

   return (U32)(gdraw->next_fence_index - fence.value) < num_pending;
}

static void wait_on_fence(GDrawFence fence)
{
   if (is_fence_pending(fence)) {
      // Is the fence in a scene we haven't even submitted yet? We can't do that.
      //
      // NOTE: you might see this from user code if you try to do the following
      // seqeuence within a single GXM scene:
      // - Render an Iggy
      // - Tick it (or otherwise trigger some event that causes a font cache
      //   update).
      // - Render the same Iggy again.
      // This is not supported. If you modify the state, you *have* to end the
      // scene first.
      if (fence.value >= gdraw->next_fence_index) // uh oh, it's in a scene we haven't even submitted yet. this is a bug!
         RR_BREAK();

      IggyWaitOnFence(&fence, 0);
   }
}

extern "C" void gdraw_psp2_wait(U64 fence_val)
{
   GDrawFence fence;
   fence.value = fence_val;

   // NOTE: if you see this function, either in Razor as a time hog or in the
   // Debugger because you're stuck here, there's a problem with how you're using
   // GDraw. These two cases are related but seperate, and I'll cover them here.
   // 
   // PERF PROBLEM ("why do we spend a lot of time here?"):
   //   GDraw calls this function when it wants to recycle memory in its
   //   resource pools; to do so, it needs to wait until any previous GPU
   //   commands using that memory have finished.
   //
   //   Generally, GDraw will free resources in LRU (least recently used) order.
   //   This means that whatever memory is being freed has (hopefully) not been
   //   touched for a few frames, and GDraw should be able to reuse it without
   //   having to wait.
   //
   //   However, if you see this function show up in your profile, that's
   //   clearly not the case. In that case, you might be thrashing the resource
   //   pools. GDraw will warn about this - do you have an Iggy warning callback
   //   installed? Anyway, if there are resource thrashing warnings, try increasing
   //   the size of the corresponding resource pool and this stall should go away.
   //
   //   The other reason why GDraw would wait is because you're passing a "dynamic
   //   buffer" to gdraw_psp2_Begin() that the GPU is still reading from a previous
   //   frame. If that's the case (just check whether your callstack contains
   //   "WaitForDynamicBufferIdle"), you might be using just one dynamic buffer.
   //   Consider double- or triple-buffering it instead to avoid this kind of stall.
   //
   // DEADLOCK ("the app is stuck here and never makes any progress"):
   //
   //   GDraw uses this function when it needs to wait for the GPU to complete
   //   rendering a previous scene, because we're trying to reuse memory we used
   //   in that scene.
   //
   //   If you end up here, this could mean one of several things:
   //
   //   1. The GPU isn't writing the scene completion notifications that GDraw is
   //      waiting for. Did you remember to pass the SceGxmNotification returned
   //      by gdraw_psp2_End as "fragmentNotification" to sceGxmEndScene? This is
   //      necessary for GDraw's memory management to work!
   //   2. Check that the notification label passed to GDraw (on CreateContext)
   //      is not being used by someone else. Again, GDraw's memory management
   //      mechanism will break if we read unexpected valus! Just make sure that
   //      GDraw has its own notification label. There are lots of notification
   //      labels (SCE_GXM_NOTIFICATION_COUNT, 512 at the time of this writing)
   //      so this shouldn't be an issue.
   //   3. You are using gdraw_psp2_Begin incorrectly. You may only have one call
   //      to gdraw_psp2_Begin per scene (this should not be a limitation since
   //      you can draw multiple Iggys or do other rendering during that time),
   //      and you must make sure that the scene containing a gdraw_psp2_Begin
   //      is submitted before you can call it again.
   //
   //   The latter needs a bit of explanation. If you're on an immediate context,
   //   it's pretty easy. In any GXM scene that's supposed to contain Iggy
   //   rendering, adhere to the following pattern:
   //
   //     sceGxmBeginScene(imm_ctx, ...)
   //     // --- may issue rendering calls here (but not draw Iggys)
   //     gdraw_psp2_Begin(...);
   //     // --- may issue other rendering calls and/or call IggyPlayerDraw
   //     //   (once or several times) here
   //     SceGxmNotification notify = gdraw_psp2_End();
   //     // --- again, may issue rendering calls here (but not draw Iggys)
   //     sceGxmEndScene(imm_ctx, nullptr, &notify);
   //   
   //   That is, exactly one gdraw_psp2_Begin/_End pair for that scene, and
   //   all IggyPlayerDraws must be inside that pair. That's it.
   //
   //   On a deferred context, *the same rules apply* - you must still make
   //   sure to render a GXM scene using the notification returned by
   //   gdraw_psp2_End before you call gdraw_psp2_Begin again. This means
   //   that you can't prepare multiple independent command lists for
   //   separate scenes on one thread and then issue them all later on
   //   the main thread (sorry).
   //
   //   It's definitely easier to stick with immediate contexts. If you
   //   really need deferred contexts and are running into problems with
   //   this, contact Iggy support.

   while (is_fence_pending(fence))
      sceGxmWaitEvent();
}

////////////////////////////////////////////////////////////////////////
//
//   Texture/vertex memory defragmentation support code
//

static void gdraw_gpu_memcpy(GDrawHandleCache *c, void *dst, void *src, U32 num_bytes)
{
   // "If srcFormat is SCE_GXM_TRANSFER_FORMAT_RAW128, then a maximum of 262144 pixels can be copied (512x512)."
   static const U32 max_width = 512;
   static const U32 max_height = 512;
   static const U32 row_size = max_width * GPU_MEMCPY_ALIGN;

   U8 *dstp = (U8 *)dst;
   U8 *srcp = (U8 *)src;
   U32 offs = 0;

   // needs to be properly aligned
   assert(((UINTa)dstp & (GPU_MEMCPY_ALIGN - 1)) == 0);
   assert(((UINTa)srcp & (GPU_MEMCPY_ALIGN - 1)) == 0);

   // copy using 512-wide transfers for the bulk part
   while (num_bytes - offs >= row_size) {
      U32 num_rows = (num_bytes - offs) / row_size;
      num_rows = RR_MIN(num_rows, max_height);

      sceGxmTransferCopy(max_width, num_rows,
                         0, 0, SCE_GXM_TRANSFER_COLORKEY_NONE,
                         SCE_GXM_TRANSFER_FORMAT_RAW128, SCE_GXM_TRANSFER_LINEAR, srcp + offs, 0, 0, row_size,
                         SCE_GXM_TRANSFER_FORMAT_RAW128, SCE_GXM_TRANSFER_LINEAR, dstp + offs, 0, 0, row_size,
                         nullptr, 0, nullptr);

      offs += num_rows * row_size;
   }

   // handle the rest
   // NOTE: our 16-byte alignment guarantees that despite rounding up, we're not going
   // to overwrite memory belonging to another resource.
   if (offs < num_bytes) {
      U32 remaining_pixels = (num_bytes - offs + GPU_MEMCPY_ALIGN - 1) / GPU_MEMCPY_ALIGN;

      sceGxmTransferCopy(remaining_pixels, 1,
                         0, 0, SCE_GXM_TRANSFER_COLORKEY_NONE,
                         SCE_GXM_TRANSFER_FORMAT_RAW128, SCE_GXM_TRANSFER_LINEAR, srcp + offs, 0, 0, row_size,
                         SCE_GXM_TRANSFER_FORMAT_RAW128, SCE_GXM_TRANSFER_LINEAR, dstp + offs, 0, 0, row_size,
                         nullptr, 0, nullptr);
   }

   if (c->is_vertex)
      gdraw->outstanding_transfers |= GDRAW_TRANSFER_vertex;
   else
      gdraw->outstanding_transfers |= GDRAW_TRANSFER_texture;
}

static void gdraw_resource_moved(GDrawHandle *t)
{
   if (!t->cache->is_vertex)
      sceGxmTextureSetData(t->handle.tex.gxm, t->raw_ptr);
   else {
      SINTa index_offs = (U8 *)t->handle.vbuf.inds - (U8 *)t->handle.vbuf.verts;
      t->handle.vbuf.verts = t->raw_ptr;
      t->handle.vbuf.inds = (U8 *)t->raw_ptr + index_offs;
   }
}

static void gdraw_gpu_wait_for_transfer_completion()
{
   if (gdraw->outstanding_transfers) {
      sceGxmTransferFinish();
      gdraw->outstanding_transfers = 0;
   }
}
static void gdraw_defragment_cache(GDrawHandleCache *c, GDrawStats *stats)
{
   // wait until we're done with the previous frame

   wait_on_fence(gdraw->scene_end_fence);

   // reap; after this point, the only remaining dead resources
   // should be ones we've touched in this frame.
   gdraw_res_reap(c, stats);

   // at this point, the pool we're switching to should be empty;
   // we can still have outstanding references to the previous pool
   // (from this frame), but not anything to the pool from before then.
   if (!gfxalloc_is_empty(c->alloc_other)) {
      // if this triggers, there's a bug in GDraw's resource management.
      RR_BREAK();
   }

   rrbool ok = gdraw_TwoPoolDefragmentMain(c, stats);
   if (!ok) // if this went wrong, we had some serious heap corruption.
      RR_BREAK();
}

static void handle_cache_tick(GDrawHandleCache *c, GDrawFence now, GDrawStats *stats)
{
   gdraw_PostDefragmentCleanup(c, stats);
   gdraw_HandleCacheTick(c, now);
}

static void api_free_resource(GDrawHandle *r)
{
   if (!r->cache->is_vertex) {
      for (S32 i=0; i < MAX_SAMPLERS; i++)
         if (gdraw->active_tex[i] == (GDrawTexture *) r)
            gdraw->active_tex[i] = nullptr;
   }
}

static void RADLINK gdraw_UnlockHandles(GDrawStats *stats)
{
   // We're on a tiled renderer; the Iggy side doesn't get to unlock *anything*.
   // We do the unlock ourselves once we're done with the scene, in
   // gdraw_psp2_End.
}

////////////////////////////////////////////////////////////////////////
//
//   Various helpers
//

static void track_dynamic_alloc_attempt(U32 size, U32 align)
{
   gdraw->dynamic_stats.allocs_attempted++;
   gdraw->dynamic_stats.bytes_attempted += size;
   gdraw->dynamic_stats.largest_bytes_attempted = RR_MAX(gdraw->dynamic_stats.largest_bytes_attempted, size);
}

static void track_dynamic_alloc_failed()
{
   if (gdraw->dynamic_stats.allocs_attempted == gdraw->dynamic_stats.allocs_succeeded + 1) { // warn the first time we run out of mem
      IggyGDrawSendWarning(nullptr, "GDraw out of dynamic memory");
   }
}

static void *alloc_dynamic(U32 size, U32 align)
{
   track_dynamic_alloc_attempt(size, align);

   void *ptr = gdraw_arena_alloc(&gdraw->dynamic, size, align);
   if (ptr) {
      gdraw->dynamic_stats.allocs_succeeded++;
      gdraw->dynamic_stats.bytes_succeeded += size;
      gdraw->dynamic_stats.largest_bytes_succeeded = RR_MAX(gdraw->dynamic_stats.largest_bytes_succeeded, size);
   } else
      track_dynamic_alloc_failed();

   return ptr;
}

static void *alloc_and_set_fragment_uniforms(SceGxmContext *gxm, U32 slot, size_t size)
{
   void *ptr = alloc_dynamic(size, sizeof(U32));
   if (ptr)
      sceGxmSetFragmentUniformBuffer(gxm, 0, ptr);
   return ptr;
}

static void *alloc_and_set_vertex_uniforms(SceGxmContext *gxm, U32 slot, size_t size)
{
   void *ptr = alloc_dynamic(size, sizeof(U32));
   if (ptr)
      sceGxmSetVertexUniformBuffer(gxm, 0, ptr);
   return ptr;
}

////////////////////////////////////////////////////////////////////////
//
//   Texture creation/updating/deletion
//

GDrawTexture * RADLINK gdraw_psp2_WrappedTextureCreate(SceGxmTexture *tex)
{
   GDrawStats stats = {};
   GDrawHandle *p = gdraw_res_alloc_begin(gdraw->texturecache, 0, &stats);
   gdraw_HandleCacheAllocateEnd(p, 0, nullptr, GDRAW_HANDLE_STATE_user_owned);
   gdraw_psp2_WrappedTextureChange((GDrawTexture *) p, tex);
   return (GDrawTexture *) p;
}

void RADLINK gdraw_psp2_WrappedTextureChange(GDrawTexture *handle, SceGxmTexture *tex)
{
   GDrawHandle *p = (GDrawHandle *) handle;
   *p->handle.tex.gxm = *tex;
}

void RADLINK gdraw_psp2_WrappedTextureDestroy(GDrawTexture *handle)
{
   GDrawStats stats = {};
   gdraw_res_free((GDrawHandle *) handle, &stats);
}

static void RADLINK gdraw_SetTextureUniqueID(GDrawTexture *tex, void *old_id, void *new_id)
{
   GDrawHandle *p = (GDrawHandle *) tex;
   // if this is still the handle it's thought to be, change the owner;
   // if the owner *doesn't* match, then they're changing a stale handle, so ignore
   if (p->owner == old_id)
      p->owner = new_id;
}

static U32 align_down(U32 x, U32 align)
{
   return x & ~(align - 1);
}

static U32 align_up(U32 x, U32 align)
{
   return (x + align - 1) & ~(align - 1);
}

static U32 round_up_to_pow2(U32 x)
{
   x--;
   x |= x >> 16;
   x |= x >> 8;
   x |= x >> 4;
   x |= x >> 2;
   x |= x >> 1;
   return x + 1;
}

static U32 tex_linear_stride(U32 width)
{
   return align_up(width, 8);
}

static rrbool RADLINK gdraw_MakeTextureBegin(void *owner, S32 width, S32 height, gdraw_texture_format gformat, U32 flags, GDraw_MakeTexture_ProcessingInfo *p, GDrawStats *stats)
{
   S32 bytes_pixel = 4;
   GDrawHandle *t = nullptr;

   SceGxmTextureFormat format = SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ABGR;
   if (width > MAX_TEXTURE2D_DIM || height > MAX_TEXTURE2D_DIM) {
      IggyGDrawSendWarning(nullptr, "GDraw %d x %d texture not supported by hardware (dimension limit %d)", width, height, MAX_TEXTURE2D_DIM);
      return false;
   }

   if (gformat == GDRAW_TEXTURE_FORMAT_font) {
      format = SCE_GXM_TEXTURE_FORMAT_U8_RRRR;
      bytes_pixel = 1;
   }

   // determine the number of mipmaps to use and size of resulting surface
   U32 mipmaps = 0;
   U32 size = 0;
   U32 base_stride = tex_linear_stride(width);

   if (flags & GDRAW_MAKETEXTURE_FLAGS_mipmap) {
      U32 pow2_w = round_up_to_pow2(width);
      U32 pow2_h = round_up_to_pow2(height);
      do {
         // mip offsets are based on size rounded up to pow2, so we pay for that amount of space.
         size += RR_MAX(pow2_w >> mipmaps, 1) * RR_MAX(pow2_h >> mipmaps, 1) * bytes_pixel;
         mipmaps++;
      }
      while ((width >> mipmaps) || (height >> mipmaps));
   } else {
      // no mips
      mipmaps = 1;
      size = base_stride * height * bytes_pixel;
   }

   // allocate a handle and make room in the cache for this much data
   t = gdraw_res_alloc_begin(gdraw->texturecache, size, stats);
   if (!t)
      return false;

   sceGxmTextureInitLinear(t->handle.tex.gxm, t->raw_ptr, format, width, height, mipmaps);

   gdraw_HandleCacheAllocateEnd(t, size, owner, (flags & GDRAW_MAKETEXTURE_FLAGS_never_flush) ? GDRAW_HANDLE_STATE_pinned : GDRAW_HANDLE_STATE_locked);
   stats->nonzero_flags |= GDRAW_STATS_alloc_tex;
   stats->alloc_tex += 1;
   stats->alloc_tex_bytes += size;

   p->texture_type = GDRAW_TEXTURE_TYPE_rgba;
   p->p0 = t;
   if (flags & GDRAW_MAKETEXTURE_FLAGS_mipmap) {
      rrbool ok;

      assert(p->temp_buffer != nullptr);
      ok = gdraw_MipmapBegin(&gdraw->mipmap, width, height, mipmaps,
         bytes_pixel, p->temp_buffer, p->temp_buffer_bytes);
      if (!ok)
         RR_BREAK(); // this should never trigger unless the temp_buffer is way too small (Iggy bug)

      p->p1 = &gdraw->mipmap;
      p->texture_data = gdraw->mipmap.pixels[0];
      p->num_rows = gdraw->mipmap.bheight;
      p->stride_in_bytes = gdraw->mipmap.pitch[0];
      p->i0 = 0; // current output y
      p->i1 = width;
      p->i2 = height;
   } else {
      // non-mipmapped textures, we just upload straight to their destination
      p->p1 = nullptr;
      p->texture_data = (U8 *)t->raw_ptr;
      p->num_rows = height;
      p->stride_in_bytes = base_stride * bytes_pixel;
   }

   return true;
}

static rrbool RADLINK gdraw_MakeTextureMore(GDraw_MakeTexture_ProcessingInfo *p)
{
   GDrawHandle *t = (GDrawHandle *)p->p0;

   if (p->p1) {
      U8 *mipstart = (U8 *)t->raw_ptr;
      GDrawMipmapContext *c = (GDrawMipmapContext *)p->p1;
      U32 width = p->i1;
      U32 height = p->i2;
      U32 bheight = c->bheight;
      U32 w_pow2 = round_up_to_pow2(width);
      U32 h_pow2 = round_up_to_pow2(height);
      U32 level = 0;
      U32 outy = p->i0;

      if (outy >= height) // wait, we've already processed the whole texture!
         return false;

      do {
         U32 pitch = tex_linear_stride(width) * c->bpp;
         U32 srcpitch = c->pitch[level];

         // copy image data to destination
         U8 *dest = mipstart + (outy >> level) * pitch;
         U8 *src = c->pixels[level];
         for (U32 y=0; y < bheight; y++)
            memcpy(dest + y*pitch, src + y*srcpitch, width * c->bpp);

         // mip offsets are computed from pow2 base size
         mipstart += RR_MAX(w_pow2 >> level, 1) * RR_MAX(h_pow2 >> level, 1) * c->bpp;
         width = RR_MAX(width >> 1, 1);
         height = RR_MAX(height >> 1, 1);
         bheight = RR_MAX(bheight >> 1, 1);
      } while(gdraw_MipmapAddLines(c, ++level));

      // next chunk please!
      p->i0 += p->num_rows;
      p->texture_data = c->pixels[0];
      p->num_rows = c->bheight = RR_MIN(c->bheight, p->i2 - p->i0);
      return true;
   } else
      return false; // non-streaming upload; you got the full image first time!
}

static GDrawTexture * RADLINK gdraw_MakeTextureEnd(GDraw_MakeTexture_ProcessingInfo *p, GDrawStats *stats)
{
   if (p->p1)
      gdraw_MakeTextureMore(p); // submit last piece of data

   RR_UNUSED_VARIABLE(stats);
   return (GDrawTexture *) p->p0;
}

static rrbool RADLINK gdraw_UpdateTextureBegin(GDrawTexture *t, void *unique_id, GDrawStats *stats)
{
   return gdraw_HandleCacheLockStats((GDrawHandle *) t, unique_id, stats);
}

static void RADLINK gdraw_UpdateTextureRect(GDrawTexture *t, void *unique_id, S32 x, S32 y, S32 stride, S32 w, S32 h, U8 *samples, gdraw_texture_format format)
{
   RR_UNUSED_VARIABLE(unique_id);
   GDrawHandle *s = (GDrawHandle *) t;
   U32 bpp = (format == GDRAW_TEXTURE_FORMAT_font) ? 1 : 4;

   // make sure texture is not active. note that we don't implement texture ghosting;
   // this is an actual wait and you can't update a texture you've already used during
   // the current frame. (we only use this path for font cache updates)
   wait_on_fence(s->fence);

   U32 bpl = w * bpp;
   U32 dpitch = tex_linear_stride(sceGxmTextureGetWidth(s->handle.tex.gxm)) * bpp;
   U8 *src = samples;
   U8 *dst = (U8 *)s->raw_ptr + y*dpitch + x*bpp;
   for (S32 row=0; row < h; row++)
      memcpy(dst + row*dpitch, src + row*stride, bpl);
}

static void RADLINK gdraw_UpdateTextureEnd(GDrawTexture *t, void *unique_id, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(unique_id);
   RR_UNUSED_VARIABLE(stats);
   RR_UNUSED_VARIABLE(t);
   // no unlock! (tiled renderer again)
}

static void RADLINK gdraw_FreeTexture(GDrawTexture *tt, void *unique_id, GDrawStats *stats)
{
   GDrawHandle *t = (GDrawHandle *) tt;
   assert(t != nullptr);
   if (t->owner == unique_id || unique_id == nullptr) {
      gdraw_res_kill(t, stats);
   }
}

static rrbool RADLINK gdraw_TryToLockTexture(GDrawTexture *t, void *unique_id, GDrawStats *stats)
{
   return gdraw_HandleCacheLockStats((GDrawHandle *) t, unique_id, stats);
}

static void RADLINK gdraw_DescribeTexture(GDrawTexture *tex, GDraw_Texture_Description *desc)
{
   GDrawHandle *p = (GDrawHandle *) tex;
   desc->width = sceGxmTextureGetWidth(p->handle.tex.gxm);
   desc->height = sceGxmTextureGetHeight(p->handle.tex.gxm);
   desc->size_in_bytes = p->bytes;
}

static void RADLINK gdraw_SetAntialiasTexture(S32 width, U8 *rgba)
{
   if (sceGxmTextureGetData(&gdraw->aa_tex) != nullptr)
      return;

   assert(width <= MAX_AATEX_WIDTH);
   void *ptr = gdraw_arena_alloc(&gdraw->context_arena, width * 4, GDRAW_PSP2_TEXTURE_ALIGNMENT);
   if (!ptr)
      return;

   sceGxmTextureInitLinear(&gdraw->aa_tex, ptr, SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ABGR, width, 1, 1);
   memcpy(ptr, rgba, width * 4);
}

////////////////////////////////////////////////////////////////////////
//
//   Vertex buffer creation/deletion
//

static rrbool RADLINK gdraw_MakeVertexBufferBegin(void *unique_id, gdraw_vformat vformat, S32 vbuf_size, S32 ibuf_size, GDraw_MakeVertexBuffer_ProcessingInfo *p, GDrawStats *stats)
{
   GDrawHandle *vb;
   vb = gdraw_res_alloc_begin(gdraw->vbufcache, vbuf_size + ibuf_size, stats);
   if (!vb)
      return false;

   vb->handle.vbuf.verts = vb->raw_ptr;
   vb->handle.vbuf.inds = (U8 *) vb->raw_ptr + vbuf_size;
   
   p->p0 = vb;
   p->vertex_data = (U8 *)vb->handle.vbuf.verts;
   p->index_data = (U8 *)vb->handle.vbuf.inds;
   p->vertex_data_length = vbuf_size;
   p->index_data_length = ibuf_size;

   gdraw_HandleCacheAllocateEnd(vb, vbuf_size + ibuf_size, unique_id, GDRAW_HANDLE_STATE_locked);
   return true;
}

static rrbool RADLINK gdraw_MakeVertexBufferMore(GDraw_MakeVertexBuffer_ProcessingInfo *p)
{
   RR_BREAK();
   return false;
}

static GDrawVertexBuffer * RADLINK gdraw_MakeVertexBufferEnd(GDraw_MakeVertexBuffer_ProcessingInfo *p, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(stats);
   return (GDrawVertexBuffer *)p->p0;
}

static rrbool RADLINK gdraw_TryLockVertexBuffer(GDrawVertexBuffer *vb, void *unique_id, GDrawStats *stats)
{
   return gdraw_HandleCacheLockStats((GDrawHandle *) vb, unique_id, stats);
}

static void RADLINK gdraw_FreeVertexBuffer(GDrawVertexBuffer *vb, void *unique_id, GDrawStats *stats)
{
   GDrawHandle *h = (GDrawHandle *) vb;
   assert(h != nullptr); // @GDRAW_ASSERT
   if (h->owner == unique_id)
      gdraw_res_kill(h, stats);
}

static void RADLINK gdraw_DescribeVertexBuffer(GDrawVertexBuffer *vbuf, GDraw_VertexBuffer_Description *desc)
{
   GDrawHandle *p = (GDrawHandle *) vbuf;
   desc->size_in_bytes = p->bytes;
}

////////////////////////////////////////////////////////////////////////
//
//   Constant buffer layouts
//

struct VertexVars
{
   F32 world[2][4];
   F32 x_offs[4];
   F32 texgen_s[4];
   F32 texgen_t[4];
   F32 viewproj[4];
};

struct PixelCommonVars
{
   F32 color_mul[4];
   F32 color_add[4];
   F32 focal[4];
};

////////////////////////////////////////////////////////////////////////
//
//   Rendering helpers
//

static void set_gxm_texture(U32 unit, SceGxmTexture *tex, U32 wrap, U32 nearest)
{
   static const U32 addrbits[ASSERT_COUNT(GDRAW_WRAP__count, 4)] = {
#define CLAMPMODE(x) ((SCE_GXM_PDS_DOUTT0_UADDRMODE_##x << SCE_GXM_PDS_DOUTT0_UADDRMODE_SHIFT) | (SCE_GXM_PDS_DOUTT0_VADDRMODE_##x << SCE_GXM_PDS_DOUTT0_VADDRMODE_SHIFT))
      CLAMPMODE(CLAMP),    // GDRAW_WRAP_clamp
      CLAMPMODE(REPEAT),   // GDRAW_WRAP_repeat
      CLAMPMODE(MIRROR),   // GDRAW_WRAP_mirror
      CLAMPMODE(CLAMP),    // GDRAW_WRAP_clamp_to_border (unused in this impl - just use regular clamp)
#undef CLAMPMODE
   };
   static const U32 addrmask = SCE_GXM_PDS_DOUTT0_UADDRMODE_MASK | SCE_GXM_PDS_DOUTT0_VADDRMODE_MASK;

   static const U32 filterbits[2] = {
      // nearest off
      SCE_GXM_PDS_DOUTT0_MINFILTER_MASK | SCE_GXM_PDS_DOUTT0_MAGFILTER_MASK | SCE_GXM_PDS_DOUTT0_MIPFILTER_MASK,
      // nearest on
      SCE_GXM_PDS_DOUTT0_MINFILTER_MASK | SCE_GXM_PDS_DOUTT0_MIPFILTER_MASK,
   };
   static const U32 filtermask = SCE_GXM_PDS_DOUTT0_MINFILTER_MASK | SCE_GXM_PDS_DOUTT0_MAGFILTER_MASK | SCE_GXM_PDS_DOUTT0_MIPFILTER_MASK;

   assert(wrap < GDRAW_WRAP__count);
   assert(nearest < 2);

   SceGxmTexture texv = *tex;
   texv.controlWords[0] = (tex->controlWords[0] & ~(addrmask | filtermask)) | addrbits[wrap] | filterbits[nearest];
   sceGxmSetFragmentTexture(gdraw->gxm, unit, &texv);
}

static void remove_scissor();

static inline void disable_scissor(bool force)
{
   if (gdraw->scissor_state)
      remove_scissor();

   if (force || gdraw->scissor_state != 0) {
      gdraw->scissor_state = 0;
      sceGxmSetRegionClip(gdraw->gxm, SCE_GXM_REGION_CLIP_OUTSIDE, gdraw->cview.x0, gdraw->cview.y0, gdraw->cview.x1 - 1, gdraw->cview.y1 - 1);
   }
}

static void set_viewport_raw(S32 x, S32 y, S32 w, S32 h)
{
   gdraw->cview.x0 = x;
   gdraw->cview.y0 = y;
   gdraw->cview.x1 = x + w;
   gdraw->cview.y1 = y + h;

   // AP - ZOffset/ZScale were set to 0.0/1.0 which is wrong. This fixed the bad poly draw order effect on the models in Skin Select
   sceGxmSetViewport(gdraw->gxm,
                     (F32)x + (F32)w*0.5f, (F32)w *  0.5f,
                     (F32)y + (F32)h*0.5f, (F32)h * -0.5f,
                     0.5f, 0.5f);
}

static void set_projection_raw(S32 x0, S32 x1, S32 y0, S32 y1)
{
   gdraw->projection[0] = 2.0f / (x1-x0);
   gdraw->projection[1] = 2.0f / (y1-y0);
   gdraw->projection[2] = (x1 + x0) / (F32) (x0 - x1);
   gdraw->projection[3] = (y1 + y0) / (F32) (y0 - y1);
}

static void set_viewport()
{
   set_viewport_raw(gdraw->vx, gdraw->vy, gdraw->tw, gdraw->th);
}

static void set_projection()
{
   set_projection_raw(gdraw->tx0, gdraw->tx0 + gdraw->tw, gdraw->ty0 + gdraw->th, gdraw->ty0);
}

static void clear_renderstate()
{
   SceGxmContext *gxm = gdraw->gxm;
   sceGxmSetFrontDepthFunc(gxm, SCE_GXM_DEPTH_FUNC_ALWAYS);
   sceGxmSetFrontDepthWriteEnable(gxm, SCE_GXM_DEPTH_WRITE_DISABLED);
   sceGxmSetFrontStencilFunc(gxm, SCE_GXM_STENCIL_FUNC_ALWAYS, SCE_GXM_STENCIL_OP_KEEP, SCE_GXM_STENCIL_OP_KEEP, SCE_GXM_STENCIL_OP_KEEP, 0, 0);

   // AP - Added this in to reset the clip region to full screen. This fixed the Splash text not appearing and the bad draw order on the 2x2 crafting cursor.
   disable_scissor(true);

   gdraw->z_stencil_key = 0;
}

static void set_common_renderstate()
{
   SceGxmContext *gxm = gdraw->gxm;

   // clear our state caching
   memset(gdraw->active_tex, 0, sizeof(gdraw->active_tex));
   gdraw->scissor_state = 0;
   gdraw->cur_fp = nullptr;
   gdraw->cur_vp = nullptr;
   
   // all the state we won't touch again until we're done rendering
   sceGxmSetCullMode(gxm, SCE_GXM_CULL_NONE);
   sceGxmSetFrontDepthBias(gxm, 0, 0);
   sceGxmSetFrontFragmentProgramEnable(gxm, SCE_GXM_FRAGMENT_PROGRAM_ENABLED);
   sceGxmSetFrontPolygonMode(gxm, SCE_GXM_POLYGON_MODE_TRIANGLE_FILL);
   sceGxmSetFrontVisibilityTestEnable(gxm, SCE_GXM_VISIBILITY_TEST_DISABLED);
   sceGxmSetFrontStencilRef(gxm, 255);
   sceGxmSetTwoSidedEnable(gxm, SCE_GXM_TWO_SIDED_DISABLED);
   sceGxmSetViewportEnable(gxm, SCE_GXM_VIEWPORT_ENABLED);
   sceGxmSetWBufferEnable(gxm, SCE_GXM_WBUFFER_DISABLED);
   sceGxmSetWClampValue(gxm, 0.00001f);
   sceGxmSetWClampEnable(gxm, SCE_GXM_WCLAMP_MODE_ENABLED);

   set_gxm_texture(AATEX_SAMPLER, &gdraw->aa_tex, GDRAW_WRAP_clamp, 0);

   // states we modify during regular rendering
   clear_renderstate();
   set_viewport();
   set_projection();
   disable_scissor(true);
}

static void set_fragment_program(SceGxmFragmentProgram *fp);
static void do_screen_quad(gswf_recti *s, const F32 *tc, F32 z, GDrawStats *stats);

static void render_clear_quad(gswf_recti *r, GDrawStats *stats)
{
   do_screen_quad(r, four_zeros, 1.0f, stats);

   stats->nonzero_flags |= GDRAW_STATS_clears;
   stats->num_clears++;
   stats->cleared_pixels += (r->x1 - r->x0) * (r->y1 - r->y0);
}

static void clear_whole_surf(bool clear_depth, bool clear_stencil, const F32 *clear_color, GDrawStats *stats)
{
   SceGxmContext *gxm = gdraw->gxm;

   gdraw->z_stencil_key = ~0u; // force reset on next draw
   SceGxmStencilOp stencil_op = clear_stencil ? SCE_GXM_STENCIL_OP_ZERO : SCE_GXM_STENCIL_OP_KEEP;

   sceGxmSetFrontDepthFunc(gxm, SCE_GXM_DEPTH_FUNC_ALWAYS);
   sceGxmSetFrontDepthWriteEnable(gxm, clear_depth ? SCE_GXM_DEPTH_WRITE_ENABLED : SCE_GXM_DEPTH_WRITE_DISABLED);
   sceGxmSetFrontStencilFunc(gxm, SCE_GXM_STENCIL_FUNC_ALWAYS, stencil_op, stencil_op, stencil_op, 0xff, 0xff);

   set_fragment_program(gdraw->clear_fp);
   PixelCommonVars *para = (PixelCommonVars *)alloc_and_set_fragment_uniforms(gxm, 0, sizeof(PixelCommonVars));
   if (!para)
      return;
   memset(para, 0, sizeof(*para));

   if (clear_color)
      vst1q_f32(para->color_mul, vld1q_f32(clear_color));
   else
      sceGxmSetFrontFragmentProgramEnable(gxm, SCE_GXM_FRAGMENT_PROGRAM_DISABLED);

   set_viewport_raw(0, 0, gdraw->screen_bounds.x1, gdraw->screen_bounds.y1);
   set_projection_raw(0, gdraw->screen_bounds.x1, gdraw->screen_bounds.y1, 0);
   render_clear_quad(&gdraw->screen_bounds, stats);

   if (!clear_color)
      sceGxmSetFrontFragmentProgramEnable(gxm, SCE_GXM_FRAGMENT_PROGRAM_ENABLED);

   set_viewport();
   set_projection();
}

////////////////////////////////////////////////////////////////////////
//
//   Begin rendering for a frame
//

void gdraw_psp2_SetTileOrigin(S32 x, S32 y)
{
   gdraw->vx = x;
   gdraw->vy = y;
}

void gdraw_psp2_ClearBeforeNextRender(const F32 clear_color_rgba[4])
{
   for (U32 i=0; i < 4; i++)
      gdraw->clear_color_rgba[i] = clear_color_rgba[i];
   gdraw->next_tile_clear = gdraw->clear_color_rgba;
}

static void RADLINK gdraw_SetViewSizeAndWorldScale(S32 w, S32 h, F32 scalex, F32 scaley)
{
   gdraw->fw = w;
   gdraw->fh = h;
   gdraw->tw = w;
   gdraw->th = h;
   gdraw->world_to_pixel[0] = scalex;
   gdraw->world_to_pixel[1] = scaley;
}

// must include anything necessary for texture creation/update
static void RADLINK gdraw_RenderingBegin(void)
{
   assert(gdraw->gxm != nullptr); // call after gdraw_psp2_Begin
   set_common_renderstate();
}

static void RADLINK gdraw_RenderingEnd(void)
{
   clear_renderstate();
}

static void RADLINK gdraw_RenderTileBegin(S32 x0, S32 y0, S32 x1, S32 y1, S32 pad, GDrawStats *stats)
{
   pad = 0; // no reason to ever pad, we don't do filters.

   gdraw->tx0 = x0;
   gdraw->ty0 = y0;
   gdraw->tw = x1-x0;
   gdraw->th = y1-y0;

   gdraw->tx0v = gdraw->tx0 - gdraw->vx;
   gdraw->ty0v = gdraw->ty0 - gdraw->vy;

   // padded region
   gdraw->tx0p = RR_MAX(x0 - pad, 0);
   gdraw->ty0p = RR_MAX(y0 - pad, 0);
   gdraw->tpw = RR_MIN(x1 + pad, gdraw->fw) - gdraw->tx0p;
   gdraw->tph = RR_MIN(y1 + pad, gdraw->fh) - gdraw->ty0p;

   // clear our depth/stencil buffers (and also color if requested)
   clear_whole_surf(true, true, gdraw->next_tile_clear, stats);
   gdraw->next_tile_clear = nullptr;
}

static void RADLINK gdraw_RenderTileEnd(GDrawStats *stats)
{
   // necessary to reset mask bits at end of tile
   // (if we had them set)
   disable_scissor(false);

   // reap once per frame even if there are no allocs
   gdraw_res_reap(gdraw->texturecache, stats);
   gdraw_res_reap(gdraw->vbufcache, stats);
}

void gdraw_psp2_Begin(SceGxmContext *context, const SceGxmColorSurface *color, const SceGxmDepthStencilSurface *depth, gdraw_psp2_dynamic_buffer *dynamic_buf)
{
   U32 xmin, ymin, xmax, ymax;

   assert(gdraw->gxm == nullptr); // may not nest Begin calls

   // need to wait for the buffer to become idle before we can use it!
   gdraw_psp2_WaitForDynamicBufferIdle(dynamic_buf);

   gdraw->gxm = context;
   gdraw->dyn_buf = dynamic_buf;
   gdraw_arena_init(&gdraw->dynamic, dynamic_buf->start, dynamic_buf->size_in_bytes);

   memset(&gdraw->dynamic_stats, 0, sizeof(gdraw->dynamic_stats));

   sceGxmColorSurfaceGetClip(color, &xmin, &ymin, &xmax, &ymax);
   gdraw->screen_bounds.x0 = xmin;
   gdraw->screen_bounds.y0 = ymin;
   gdraw->screen_bounds.x1 = xmax + 1;
   gdraw->screen_bounds.y1 = ymax + 1;

   // If we don't have a depth/stencil surface with the right format,
   // Iggy rendering is not going to come out right.
   if (!depth ||
       !sceGxmDepthStencilSurfaceIsEnabled(depth) ||
       sceGxmDepthStencilSurfaceGetFormat(depth) != SCE_GXM_DEPTH_STENCIL_FORMAT_DF32M_S8) {

      // Why this format?
      // - We need the stencil buffer to support Flash masking operations.
      // - We need the mask bit to perform pixel-accurate scissor testing.
      // There's only one format that satisfies both requirements.
      IggyGDrawSendWarning(nullptr, "Iggy rendering will not work correctly unless a depth/stencil buffer in DF32M_S8 format is provided.");
   }

   // For immediate contexts, we need to flush pending vertex transfers before
   // every draw because we might hit a mid-scene flush. On a deferred
   // context, we need not worry about this happening.
   SceGxmContextType type;
   SceGxmErrorCode err = sceGxmGetContextType(context, &type);
   if (err == SCE_OK && type == SCE_GXM_CONTEXT_TYPE_DEFERRED)
      gdraw->draw_transfer_flush_mask = 0;
   else
      gdraw->draw_transfer_flush_mask = GDRAW_TRANSFER_vertex;
}

SceGxmNotification gdraw_psp2_End()
{
   GDrawStats gdraw_stats = {};
   SceGxmNotification notify;

   assert(gdraw->gxm != nullptr); // please keep Begin / End pairs properly matched

   notify = scene_end_notification();
   gdraw->dyn_buf->sync = gdraw->scene_end_fence.value;
   gdraw->dyn_buf->stats = gdraw->dynamic_stats;

   gdraw_arena_init(&gdraw->dynamic, nullptr, 0);
   gdraw->gxm = nullptr;
   gdraw->dyn_buf = nullptr;

   // NOTE: the stats from these go nowhere. That's a bit unfortunate, but the
   // GDrawStats model is that things can be accounted to something in the
   // display tree, and that's simply not the case with scene-global things
   // like this. With only one Iggy file, a sensible place would be to
   // accumulate stats in the root node. But the user can render multiple Iggys
   // in the same scene, and it's unclear what to do in that case.
   handle_cache_tick(gdraw->texturecache, gdraw->scene_end_fence, &gdraw_stats);
   handle_cache_tick(gdraw->vbufcache, gdraw->scene_end_fence, &gdraw_stats);

   // finally, unlock everything
   gdraw_HandleCacheUnlockAll(gdraw->texturecache);
   gdraw_HandleCacheUnlockAll(gdraw->vbufcache);

   return notify;
}

#define MAX_DEPTH_VALUE (1 << 22)

static void RADLINK gdraw_GetInfo(GDrawInfo *d)
{
   d->num_stencil_bits = 8;
   d->max_id = MAX_DEPTH_VALUE-2;
   // for floating point depth, just use mantissa, e.g. 16-20 bits
   d->max_texture_size = MAX_TEXTURE2D_DIM;
   d->buffer_format = GDRAW_BFORMAT_vbib;
   d->shared_depth_stencil = 1;
   d->always_mipmap = 0;
   d->conditional_nonpow2 = 0;
   d->has_rendertargets = 0;
}

////////////////////////////////////////////////////////////////////////
//
//   Render targets
//

static rrbool RADLINK gdraw_TextureDrawBufferBegin(gswf_recti *region, gdraw_texture_format format, U32 flags, void *owner, GDrawStats *stats)
{
   IggyGDrawSendWarning(nullptr, "GDraw no rendertarget support on PSP2");
   return false;
}

static GDrawTexture *RADLINK gdraw_TextureDrawBufferEnd(GDrawStats *stats)
{
   return nullptr;
}

////////////////////////////////////////////////////////////////////////
//
//   Clear stencil/depth buffers
//

static void RADLINK gdraw_ClearStencilBits(U32 bits)
{
   GDrawStats stats = {};
   clear_whole_surf(false, true, nullptr, &stats);
}

static void RADLINK gdraw_ClearID(void)
{
   GDrawStats stats = {};
   clear_whole_surf(true, false, nullptr, &stats);
}

////////////////////////////////////////////////////////////////////////
//
//   Fragment programs and scissor mask
//

static RADINLINE void set_fragment_program(SceGxmFragmentProgram *fp)
{
   if (gdraw->cur_fp != fp) {
      gdraw->cur_fp = fp;
      sceGxmSetFragmentProgram(gdraw->gxm, fp);
   }
}

static RADINLINE void set_vertex_program(SceGxmVertexProgram *vp)
{
   if (gdraw->cur_vp != vp) {
      gdraw->cur_vp = vp;
      sceGxmSetVertexProgram(gdraw->gxm, vp);
   }
}

static void draw_scissor_region(gswf_recti *r, SceGxmStencilFunc func)
{
   GDraw * RADRESTRICT gd = gdraw;

   // determine tile-aligned rect
   gswf_recti tile_rect;
   tile_rect.x0 = align_down(r->x0, SCE_GXM_TILE_SIZEX);
   tile_rect.y0 = align_down(r->y0, SCE_GXM_TILE_SIZEY);
   tile_rect.x1 = align_up(r->x1, SCE_GXM_TILE_SIZEX);
   tile_rect.y1 = align_up(r->y1, SCE_GXM_TILE_SIZEY);

   // set up vertex positions
   F32 *vpos = (F32 *)alloc_and_set_vertex_uniforms(gd->gxm, 0, 8 * sizeof(F32));
   if (!vpos)
      return;

   vpos[0] = (F32)tile_rect.x0;
   vpos[1] = (F32)r->x0;
   vpos[2] = (F32)r->x1;
   vpos[3] = (F32)tile_rect.x1;
   vpos[4] = (F32)tile_rect.y0;
   vpos[5] = (F32)r->y0;
   vpos[6] = (F32)r->y1;
   vpos[7] = (F32)tile_rect.y1;

   // set our region clip; note gxm bounds are max-inclusive, hence the -1.
   sceGxmSetRegionClip(gd->gxm, SCE_GXM_REGION_CLIP_OUTSIDE,
      tile_rect.x0, tile_rect.y0, tile_rect.x1 - 1, tile_rect.y1 - 1);

   // set up programs and state
   set_vertex_program(gd->mask_vp);
   set_fragment_program(gd->mask_update_fp);

   // draw
   gdraw->z_stencil_key = ~0u; // invalidate z state -> force reset on next draw
   sceGxmSetFrontStencilFunc(gd->gxm, func, SCE_GXM_STENCIL_OP_KEEP, SCE_GXM_STENCIL_OP_KEEP, SCE_GXM_STENCIL_OP_KEEP, 0, 0);
   sceGxmSetViewportEnable(gd->gxm, SCE_GXM_VIEWPORT_DISABLED);
   sceGxmDrawPrecomputed(gd->gxm, &gd->mask_draw);
   sceGxmSetViewportEnable(gd->gxm, SCE_GXM_VIEWPORT_ENABLED);
}

static void remove_scissor()
{
   // re-enable drawing in the exclusion region
   draw_scissor_region(&gdraw->cur_scissor, SCE_GXM_STENCIL_FUNC_ALWAYS);
}

static void materialize_scissor(int x0, int y0, int x1, int y1)
{
   GDraw * RADRESTRICT gd = gdraw;

   // did we have scissor set?
   if (gd->scissor_state) {
      // are we about to set the same scissor again?
      if (gd->cur_scissor.x0 == x0 && gd->cur_scissor.y0 == y0 &&
          gd->cur_scissor.x1 == x1 && gd->cur_scissor.y1 == y1)
         return; // nothing to do!

      remove_scissor();
   }

   // draw the mask: disable drawing outside scissor region
   gd->cur_scissor.x0 = x0;
   gd->cur_scissor.y0 = y0;
   gd->cur_scissor.x1 = x1;
   gd->cur_scissor.y1 = y1;
   draw_scissor_region(&gdraw->cur_scissor, SCE_GXM_STENCIL_FUNC_NEVER);
   gd->scissor_state = 1;
}

////////////////////////////////////////////////////////////////////////
//
//   Set all the render state from GDrawRenderState
//

// converts a depth id into a Z value
static inline F32 depth_from_id(S32 id)
{
   return (1.0f - 1.0f / MAX_DEPTH_VALUE) - id * (1.0f / MAX_DEPTH_VALUE); // = 1 - (id + 1) / MAX_DEPTH_VALUE
}

static bool set_renderstate_full(const GDrawRenderState * RADRESTRICT r, GDrawStats *stats)
{
   static const int canonical_blend[ASSERT_COUNT(GDRAW_BLEND__count, 6)] = {
      GDRAW_CBLEND_none,      // GDRAW_BLEND_none
      GDRAW_CBLEND_alpha,     // GDRAW_BLEND_alpha
      GDRAW_CBLEND_none,      // GDRAW_BLEND_multiply - UNSUPPORTED on PSP2 (only occurs in layer blends which we don't allow)
      GDRAW_CBLEND_add,       // GDRAW_BLEND_add

      GDRAW_CBLEND_none,      // GDRAW_BLEND_filter
      GDRAW_CBLEND_none,      // GDRAW_BLEND_special
   };

   GDraw * RADRESTRICT gd = gdraw;
   SceGxmContext * RADRESTRICT gxm = gd->gxm;

   // we need to handle scissor first, since it might require us to draw things.
   if (r->scissor) {
      S32 xs = gd->tx0v;
      S32 ys = gd->ty0v;

      // clip against viewport
      S32 x0 = RR_MAX(r->scissor_rect.x0 - xs, gd->cview.x0);
      S32 y0 = RR_MAX(r->scissor_rect.y0 - ys, gd->cview.y0);
      S32 x1 = RR_MIN(r->scissor_rect.x1 - xs, gd->cview.x1);
      S32 y1 = RR_MIN(r->scissor_rect.y1 - ys, gd->cview.y1);

      // in case our actual scissor is empty, bail.
      if (x1 <= x0 || y1 <= y0)
         return false;

      materialize_scissor(x0, y0, x1, y1);
   } else if (r->scissor != gd->scissor_state)
      disable_scissor(0);

   // allocate dynamic uniform bufs
   void * dyn_uniform = alloc_dynamic(sizeof(VertexVars) + sizeof(PixelCommonVars), sizeof(U32));
   if (!dyn_uniform)
      return false;

   VertexVars * RADRESTRICT vvars = (VertexVars *)dyn_uniform;
   PixelCommonVars * RADRESTRICT pvars = (PixelCommonVars *)(vvars + 1); // right after VertexVars in dyn_uniform alloc
   sceGxmSetVertexUniformBuffer(gxm, 0, vvars);
   sceGxmSetFragmentUniformBuffer(gxm, 0, pvars);

   // vertex uniforms
   F32 depth = depth_from_id(r->id);
   if (!r->use_world_space)
   {
      gdraw_ObjectSpace(vvars->world[0], r->o2w, depth, 0.0f);
   }
   else
   {
      gdraw_WorldSpace(vvars->world[0], gdraw->world_to_pixel, depth, 0.0f);
   }

   float32x4_t edge = vld1q_f32(r->edge_matrix);
   float32x4_t s0_texgen = vld1q_f32(r->s0_texgen); // always copy, even when unused.
   float32x4_t t0_texgen = vld1q_f32(r->t0_texgen);
   float32x4_t viewproj = vld1q_f32(gd->projection);

   vst1q_f32(vvars->x_offs, edge);
   vst1q_f32(vvars->texgen_s, s0_texgen);
   vst1q_f32(vvars->texgen_t, t0_texgen);
   vst1q_f32(vvars->viewproj, viewproj);

   // fragment uniforms  
   float32x4_t col_mul = vld1q_f32(r->color);
   float32x4_t col_add = vdupq_n_f32(0.0f);
   float32x4_t focal = vld1q_f32(r->focal_point);

   if (r->cxf_add)
      col_add = vmulq_n_f32(vcvtq_f32_s32(vmovl_s16(vld1_s16(r->cxf_add))), 1.0f / 255.0f);

   vst1q_f32(pvars->color_mul, col_mul);
   vst1q_f32(pvars->color_add, col_add);
   vst1q_f32(pvars->focal, focal);

   // set the fragment program
   int tex0mode = r->tex0_mode;
   int cblend_mode = canonical_blend[r->blend_mode];
   if (r->stencil_set)
      cblend_mode = GDRAW_CBLEND_nowrite;

   int additive_mode = 0;
   if (r->cxf_add)
      additive_mode = r->cxf_add[3] ? 2 : 1;

   set_fragment_program(gd->main_fp[tex0mode][additive_mode][cblend_mode]);

   // set textures
   if (tex0mode != GDRAW_TEXTURE_none) {
      if (!r->tex[0]) // this can happen if some allocs fail. just abort in that case.
         return false;

      if (gd->active_tex[0] != r->tex[0]) {
         gd->active_tex[0] = r->tex[0];
         set_gxm_texture(0, ((GDrawHandle *) r->tex[0])->handle.tex.gxm, r->wrap0, r->nearest0);
      }
   }

   // z/stencil mode changed?
   U32 z_stencil_key = r->set_id | (r->test_id << 1) | (r->stencil_test << 16) | (r->stencil_set << 24);
   
   if (z_stencil_key != gd->z_stencil_key) {
      gd->z_stencil_key = z_stencil_key;
      sceGxmSetFrontDepthFunc(gxm, r->test_id ? SCE_GXM_DEPTH_FUNC_LESS : SCE_GXM_DEPTH_FUNC_ALWAYS);
      sceGxmSetFrontDepthWriteEnable(gxm, r->set_id ? SCE_GXM_DEPTH_WRITE_ENABLED : SCE_GXM_DEPTH_WRITE_DISABLED);
      sceGxmSetFrontStencilFunc(gxm,
                                r->stencil_test ? SCE_GXM_STENCIL_FUNC_EQUAL : SCE_GXM_STENCIL_FUNC_ALWAYS,
                                SCE_GXM_STENCIL_OP_KEEP, SCE_GXM_STENCIL_OP_KEEP, SCE_GXM_STENCIL_OP_REPLACE,
                                r->stencil_test, r->stencil_set);
   }

   return true;
}

static RADINLINE bool set_renderstate(const GDrawRenderState * RADRESTRICT r, GDrawStats *stats)
{
   if (!r->identical_state)
      return set_renderstate_full(r, stats);
   else
      return true;
}

////////////////////////////////////////////////////////////////////////
//
//   Draw triangles with a given renderstate
//

static const U32 vfmt_stride_bytes[ASSERT_COUNT(GDRAW_vformat__basic_count, 3)] = {
   8,    // GDRAW_vformat_v2
   16,   // GDRAW_vformat_v2aa
   16,   // GDRAW_vformat_v2tc2
};

#ifdef GDRAW_DEBUG
static GDrawHandle *check_resource(void *ptr)
{
   GDrawHandle *h = (GDrawHandle *)ptr;

   // This is our memory management invariant for tilers.
   assert(h->state == GDRAW_HANDLE_STATE_locked ||
          h->state == GDRAW_HANDLE_STATE_pinned ||
          h->state == GDRAW_HANDLE_STATE_user_owned);
   return h;
}
#else
#define check_resource(ptr) ((GDrawHandle *)(ptr))
#endif

static RADINLINE void fence_resources(void *r1, void *r2=nullptr, void *r3=nullptr)
{
   GDrawFence fence = get_next_fence();
   if (r1) check_resource(r1)->fence = fence;
   if (r2) check_resource(r2)->fence = fence;
   if (r3) check_resource(r3)->fence = fence;
}

static RADINLINE void draw_ind_tris_u16(SceGxmContext *gxm, S32 vfmt, const void *verts, const void *inds, S32 num_inds)
{
   sceGxmSetVertexStream(gxm, 0, verts);
   sceGxmDraw(gxm, SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, inds, num_inds);
}

static void RADLINK gdraw_DrawIndexedTriangles(GDrawRenderState *r, GDrawPrimitive *p, GDrawVertexBuffer *buf, GDrawStats *stats)
{
   SceGxmContext *gxm = gdraw->gxm;
   GDrawHandle *vb = (GDrawHandle *) buf;
   S32 vfmt = p->vertex_format;

   // AP only round the coords for type 2 vertex format (text)
	if( p->vertex_format == 2 )
	{
		r->o2w->trans[0] = (int) r->o2w->trans[0];
		r->o2w->trans[1] = (int) r->o2w->trans[1];
	}

   assert(vfmt < GDRAW_vformat__basic_count);
   U32 stride = vfmt_stride_bytes[vfmt];
   if (!set_renderstate(r, stats))
      return;

   set_vertex_program(gdraw->vp[vfmt]);

   // do we have transfers we need to flush before we draw?
   if ((gdraw->outstanding_transfers & gdraw->draw_transfer_flush_mask) != 0)
      gdraw_gpu_wait_for_transfer_completion();

   if (vb)
      draw_ind_tris_u16(gxm, vfmt, (U8 *)vb->handle.vbuf.verts + (UINTa)p->vertices, (U8 *)vb->handle.vbuf.inds + (UINTa)p->indices, p->num_indices);
   else if (p->indices) {
      U32 vbytes = p->num_vertices * stride;
      U32 ibytes = p->num_indices * sizeof(U16);
      U8 *buf = (U8 *)alloc_dynamic(vbytes + ibytes, sizeof(U32));
      if (!buf)
         return;

      memcpy(buf, p->vertices, vbytes);
      memcpy(buf + vbytes, p->indices, ibytes);
      draw_ind_tris_u16(gxm, vfmt, buf, buf + vbytes, p->num_indices);
   } else { // dynamic quads
      assert(p->num_vertices % 4 == 0);
      U32 num_bytes = (U32)p->num_vertices * stride;

      U8 *buf = (U8 *)alloc_dynamic(num_bytes, sizeof(U32));
      if (!buf)
         return;

      memcpy(buf, p->vertices, num_bytes);

      S32 pos = 0;
      while (pos < p->num_vertices) {
         S32 vert_count = RR_MIN(p->num_vertices - pos, QUAD_IB_COUNT * 4);
         draw_ind_tris_u16(gxm, vfmt, buf + pos*stride, gdraw->quad_ib, (vert_count >> 2) * 6);
         pos += vert_count;
      }
   }

   fence_resources(vb, r->tex[0], r->tex[1]);

   stats->nonzero_flags |= GDRAW_STATS_batches;
   stats->num_batches += 1;
   stats->drawn_indices += p->num_indices;
   stats->drawn_vertices += p->num_vertices;
}

///////////////////////////////////////////////////////////////////////
//
//   Flash 8 filter effects
//

static void do_screen_quad(gswf_recti *s, const F32 *tc, F32 z, GDrawStats *stats)
{
   static const F32 worldv[2][4] = {
      { 1.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, 1.0f, 0.0f, 0.0f },
   };

   set_vertex_program(gdraw->vp[GDRAW_vformat_v2tc2]);

   VertexVars *vvars = (VertexVars *)alloc_and_set_vertex_uniforms(gdraw->gxm, 0, sizeof(VertexVars));
   if (!vvars)
      return;

   float32x4_t world0 = vld1q_f32(worldv[0]);
   float32x4_t world1 = vld1q_f32(worldv[1]);
   float32x4_t zero = vdupq_n_f32(0.0f);
   float32x4_t viewproj = vld1q_f32(gdraw->projection);
   world0 = vsetq_lane_f32(z, world0, 2);
   vst1q_f32(vvars->world[0], world0);
   vst1q_f32(vvars->world[1], world1);
   vst1q_f32(vvars->x_offs, zero);
   vst1q_f32(vvars->texgen_s, zero);
   vst1q_f32(vvars->texgen_t, zero);
   vst1q_f32(vvars->viewproj, viewproj);

   gswf_vertex_xyst * RADRESTRICT v = (gswf_vertex_xyst *)alloc_dynamic(4 * sizeof(gswf_vertex_xyst), 4);
   if (!v)
      return;
   
   F32 px0 = (F32) s->x0, py0 = (F32) s->y0, px1 = (F32) s->x1, py1 = (F32) s->y1;
   v[0].x = px0; v[0].y = py0; v[0].s = tc[0]; v[0].t = tc[1];
   v[1].x = px1; v[1].y = py0; v[1].s = tc[2]; v[1].t = tc[1];
   v[2].x = px1; v[2].y = py1; v[2].s = tc[2]; v[2].t = tc[3];
   v[3].x = px0; v[3].y = py1; v[3].s = tc[0]; v[3].t = tc[3];

   sceGxmSetVertexStream(gdraw->gxm, 0, v);
   sceGxmDraw(gdraw->gxm, SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, gdraw->quad_ib, 6);
}

static void RADLINK gdraw_FilterQuad(GDrawRenderState *r, S32 x0, S32 y0, S32 x1, S32 y1, GDrawStats *stats)
{
   F32 tc[4];
   gswf_recti s;

   // clip to tile boundaries
   s.x0 = RR_MAX(x0, gdraw->tx0p);
   s.y0 = RR_MAX(y0, gdraw->ty0p);
   s.x1 = RR_MIN(x1, gdraw->tx0p + gdraw->tpw);
   s.y1 = RR_MIN(y1, gdraw->ty0p + gdraw->tph);
   if (s.x1 < s.x0 || s.y1 < s.y0)
      return;

   // prepare for drawing
   tc[0] = (s.x0 - gdraw->tx0p) / (F32) gdraw->screen_bounds.x1;
   tc[1] = (s.y0 - gdraw->ty0p) / (F32) gdraw->screen_bounds.y1;
   tc[2] = (s.x1 - gdraw->tx0p) / (F32) gdraw->screen_bounds.x1;
   tc[3] = (s.y1 - gdraw->ty0p) / (F32) gdraw->screen_bounds.y1;

   // actual filter effects and special blends aren't supported on PSP2.
   if (r->blend_mode == GDRAW_BLEND_filter || r->blend_mode == GDRAW_BLEND_special) {
      IggyGDrawSendWarning(nullptr, "GDraw no filter or special blend support on PSP2");
      // just don't do anything.
   } else {
      // just a plain quad.
      if (!set_renderstate(r, stats))
         return;

      do_screen_quad(&s, tc, 0.0f, stats);
      fence_resources(r->tex[0], r->tex[1]);
   }
}

////////////////////////////////////////////////////////////////////////
//
//   Shaders and state initialization
//

#include "gdraw_psp2_shaders.inl"

static bool gxm_check(SceGxmErrorCode err)
{
   if (err != SCE_OK)
      IggyGDrawSendWarning(nullptr, "GXM error");

   return err == SCE_OK;
}

static bool register_shader(SceGxmShaderPatcher *patcher, ShaderCode *shader)
{
   if (!shader->blob)
      return SCE_OK;

   bool ok = gxm_check(sceGxmShaderPatcherRegisterProgram(patcher, (const SceGxmProgram *)shader->blob, &shader->id));
   shader->registered = ok;
   return ok;
}

static void unregister_shader(SceGxmShaderPatcher *patcher, ShaderCode *shader)
{
   if (shader->registered) {
      sceGxmShaderPatcherUnregisterProgram(patcher, shader->id);
      shader->registered = false;
   }
}

static bool register_and_create_vertex_prog(SceGxmVertexProgram **out_prog, SceGxmShaderPatcher *patcher, ShaderCode *shader, U32 attr_bytes)
{
   SceGxmVertexAttribute attr;
   SceGxmVertexStream stream;

   if (!register_shader(patcher, shader))
      return nullptr;

   *out_prog = nullptr;

   if (attr_bytes) {
      attr.streamIndex = 0;
      attr.offset = 0;
      attr.format = SCE_GXM_ATTRIBUTE_FORMAT_UNTYPED;
      attr.componentCount = attr_bytes / sizeof(U32);
      attr.regIndex = 0;

      stream.stride = attr_bytes;
      stream.indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

      return gxm_check(sceGxmShaderPatcherCreateVertexProgram(patcher, shader->id, &attr, 1, &stream, 1, out_prog));
   } else
      return gxm_check(sceGxmShaderPatcherCreateVertexProgram(patcher, shader->id, nullptr, 0, nullptr, 0, out_prog));
}

static void destroy_vertex_prog(SceGxmShaderPatcher *patcher, SceGxmVertexProgram *prog)
{
   if (prog)
      sceGxmShaderPatcherReleaseVertexProgram(patcher, prog);
}

static bool create_fragment_prog(SceGxmFragmentProgram **out_prog, SceGxmShaderPatcher *patcher, ShaderCode *shader, const SceGxmBlendInfo *blend, SceGxmOutputRegisterFormat out_fmt)
{
   *out_prog = nullptr;
   return gxm_check(sceGxmShaderPatcherCreateFragmentProgram(patcher, shader->id, out_fmt, SCE_GXM_MULTISAMPLE_NONE, blend, nullptr, out_prog));
}

static void destroy_fragment_prog(SceGxmShaderPatcher *patcher, SceGxmFragmentProgram *prog)
{
   if (prog)
      sceGxmShaderPatcherReleaseFragmentProgram(patcher, prog);
}

static bool create_all_programs(SceGxmOutputRegisterFormat reg_format)
{
   SceGxmShaderPatcher *patcher = gdraw->patcher;

   // blend states
   static const SceGxmBlendInfo blends[ASSERT_COUNT(GDRAW_CBLEND__count, 4)] = {
      // GDRAW_CBLEND_none
      { SCE_GXM_COLOR_MASK_ALL, SCE_GXM_BLEND_FUNC_ADD, SCE_GXM_BLEND_FUNC_ADD,
        SCE_GXM_BLEND_FACTOR_ONE, SCE_GXM_BLEND_FACTOR_ZERO, 
        SCE_GXM_BLEND_FACTOR_ONE, SCE_GXM_BLEND_FACTOR_ZERO
      },
      // GDRAW_CBLEND_alpha
      { SCE_GXM_COLOR_MASK_ALL, SCE_GXM_BLEND_FUNC_ADD, SCE_GXM_BLEND_FUNC_ADD,
        SCE_GXM_BLEND_FACTOR_ONE, SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, 
        SCE_GXM_BLEND_FACTOR_ONE, SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
      },
      // GDRAW_CBLEND_add
      { SCE_GXM_COLOR_MASK_ALL, SCE_GXM_BLEND_FUNC_ADD, SCE_GXM_BLEND_FUNC_ADD,
        SCE_GXM_BLEND_FACTOR_ONE, SCE_GXM_BLEND_FACTOR_ONE, 
        SCE_GXM_BLEND_FACTOR_ONE, SCE_GXM_BLEND_FACTOR_ONE
      },
      // GDRAW_CBLEND_nowrite
      { SCE_GXM_COLOR_MASK_NONE, SCE_GXM_BLEND_FUNC_ADD, SCE_GXM_BLEND_FUNC_ADD,
        SCE_GXM_BLEND_FACTOR_ONE, SCE_GXM_BLEND_FACTOR_ZERO, 
        SCE_GXM_BLEND_FACTOR_ONE, SCE_GXM_BLEND_FACTOR_ZERO
      },
   };

   // vertex shaders
   for (int i=0; i < GDRAW_vformat__basic_count; i++) {
      if (!register_and_create_vertex_prog(&gdraw->vp[i], patcher, vshader_vspsp2_arr + i, vfmt_stride_bytes[i]))
         return false;
   }

   if (!register_and_create_vertex_prog(&gdraw->mask_vp, patcher, vshader_vspsp2_mask_arr, 0))
      return false;

   // fragment shaders
   for (int i=0; i < GDRAW_TEXTURE__count; i++) {
      for (int j=0; j < 3; j++) {
         ShaderCode *sh = pshader_basic_arr + i*3 + j;
         if (!register_shader(patcher, sh))
            return false;

         for (int k=0; k < GDRAW_CBLEND__count; k++)
            if (!create_fragment_prog(&gdraw->main_fp[i][j][k], patcher, sh, &blends[k], reg_format))
               return false;
      }
   }

   if (!register_shader(patcher, pshader_manual_clear_arr) ||
       !create_fragment_prog(&gdraw->clear_fp, patcher, pshader_manual_clear_arr, nullptr, reg_format))
      return false;

   gdraw->mask_update_fp = nullptr;
   return gxm_check(sceGxmShaderPatcherCreateMaskUpdateFragmentProgram(patcher, &gdraw->mask_update_fp));
}

static void destroy_all_programs()
{
   SceGxmShaderPatcher *patcher = gdraw->patcher;

   // release all programs
   for (int i=0; i < GDRAW_vformat__basic_count; i++)
      destroy_vertex_prog(patcher, gdraw->vp[i]);

   destroy_vertex_prog(patcher, gdraw->mask_vp);

   for (int i=0; i < GDRAW_TEXTURE__count * 3 * GDRAW_CBLEND__count; i++)
      destroy_fragment_prog(patcher, gdraw->main_fp[0][0][i]);

   destroy_fragment_prog(patcher, gdraw->clear_fp);
   sceGxmShaderPatcherReleaseFragmentProgram(patcher, gdraw->mask_update_fp);

   // unregister shaders
   for (int i=0; i < GDRAW_vformat__basic_count; i++)
      unregister_shader(patcher, vshader_vspsp2_arr + i);

   for (int i=0; i < GDRAW_TEXTURE__count*3; i++)
      unregister_shader(patcher, pshader_basic_arr + i);
   unregister_shader(patcher, pshader_manual_clear_arr);
}

typedef struct
{
   S32 num_handles;
   S32 num_bytes;
   void *ptr;
} GDrawResourceLimit;

// Resource limits used by GDraw. Change these using SetResouceLimits!
static GDrawResourceLimit gdraw_limits[GDRAW_PSP2_RESOURCE__count];

static GDrawHandleCache *make_handle_cache(gdraw_psp2_resourcetype type, U32 align, rrbool use_twopool)
{
   S32 num_handles = gdraw_limits[type].num_handles;
   S32 one_pool_bytes = gdraw_limits[type].num_bytes;
   U32 cache_size = sizeof(GDrawHandleCache) + (num_handles - 1) * sizeof(GDrawHandle);
   bool is_vertex = (type == GDRAW_PSP2_RESOURCE_vertexbuffer);
   U32 header_size = num_handles * (is_vertex ? 0 : sizeof(SceGxmTexture));
   GDrawHandleCache *cache;

   if (use_twopool)
      one_pool_bytes = align_down(one_pool_bytes / 2, align);

   if (one_pool_bytes < (S32)align)
      return nullptr;

   cache = (GDrawHandleCache *) IggyGDrawMalloc(cache_size + header_size);
   if (cache) {
      gdraw_HandleCacheInit(cache, num_handles, one_pool_bytes);
      cache->is_vertex = is_vertex;

      // set up resource headers
      void *header_start = (U8 *) cache + cache_size;
      if (!is_vertex) {
         SceGxmTexture *headers = (SceGxmTexture *) header_start;
         for (S32 i=0; i < num_handles; i++)
            cache->handle[i].handle.tex.gxm = &headers[i];
      }

      // set up allocators
      cache->alloc = gfxalloc_create(gdraw_limits[type].ptr, one_pool_bytes, align, num_handles);
      if (!cache->alloc) {
         IggyGDrawFree(cache);
         return nullptr;
      }

      if (use_twopool) {
         cache->alloc_other = gfxalloc_create((U8 *)gdraw_limits[type].ptr + one_pool_bytes, one_pool_bytes, align, num_handles);
         if (!cache->alloc_other) {
            IggyGDrawFree(cache->alloc);
            IggyGDrawFree(cache);
            return nullptr;
         }

         // two dummy copies to make sure we have gpu read/write access
         assert(align >= GPU_MEMCPY_ALIGN);
         U8 *mem_begin = (U8 *)gdraw_limits[type].ptr;
         U8 *mem_near_end = (U8 *)gdraw_limits[type].ptr + 2*one_pool_bytes - GPU_MEMCPY_ALIGN;

         // reads near begin, writes near end
         gdraw_gpu_memcpy(cache, mem_near_end, mem_begin, GPU_MEMCPY_ALIGN);
         // reads near end, writes near begin
         gdraw_gpu_memcpy(cache, mem_begin, mem_near_end, GPU_MEMCPY_ALIGN);
         gdraw_gpu_wait_for_transfer_completion();
      }
   }

   return cache;
}

static void free_handle_cache(GDrawHandleCache *c)
{
   if (c) {
      if (c->alloc) IggyGDrawFree(c->alloc);
      if (c->alloc_other) IggyGDrawFree(c->alloc_other);
      IggyGDrawFree(c);
   }
}

void gdraw_psp2_InitDynamicBuffer(gdraw_psp2_dynamic_buffer *buf, void *ptr, U32 num_bytes)
{
   memset(buf, 0, sizeof(*buf));
   buf->start = ptr;
   buf->size_in_bytes = num_bytes;
}

void gdraw_psp2_WaitForDynamicBufferIdle(gdraw_psp2_dynamic_buffer *buf)
{
   GDrawFence fence;
   fence.value = buf->sync;
   wait_on_fence(fence);
}

int gdraw_psp2_SetResourceMemory(gdraw_psp2_resourcetype type, S32 num_handles, void *ptr, S32 num_bytes)
{
   GDrawStats stats={0};

   assert(type >= GDRAW_PSP2_RESOURCE_texture && type < GDRAW_PSP2_RESOURCE__count);
   assert(num_handles >= 0);
   assert(num_bytes >= 0);

   if (!num_handles) num_handles = 1;

   switch (type) {
   case GDRAW_PSP2_RESOURCE_texture:
      make_pool_aligned(&ptr, &num_bytes, GDRAW_PSP2_TEXTURE_ALIGNMENT);
      break;

   case GDRAW_PSP2_RESOURCE_vertexbuffer:
      make_pool_aligned(&ptr, &num_bytes, GDRAW_PSP2_VERTEXBUFFER_ALIGNMENT);
      break;

   default:
      break;
   }

   gdraw_limits[type].num_handles = num_handles;
   gdraw_limits[type].num_bytes = num_bytes;
   gdraw_limits[type].ptr = ptr;

   // if no gdraw context created, there's nothing to worry about
   if (!gdraw)
      return 1;

   // make sure GPU is done first (assuming we're in a state where we can dispatch commands)
   assert(!is_fence_pending(gdraw->scene_end_fence)); // you may not call this while GPU is still busy with Iggy command buffers!

   if (gdraw->texturecache) gdraw_res_reap(gdraw->texturecache, &stats);
   if (gdraw->vbufcache) gdraw_res_reap(gdraw->vbufcache, &stats);
   // in theory we can now check that the given cache is really empty at this point

   // resize the appropriate pool
   switch (type) {
      case GDRAW_PSP2_RESOURCE_texture:
         free_handle_cache(gdraw->texturecache);
         gdraw->texturecache = make_handle_cache(GDRAW_PSP2_RESOURCE_texture, GDRAW_PSP2_TEXTURE_ALIGNMENT, true);
         return gdraw->texturecache != nullptr;

      case GDRAW_PSP2_RESOURCE_vertexbuffer:
         free_handle_cache(gdraw->vbufcache);
         gdraw->vbufcache = make_handle_cache(GDRAW_PSP2_RESOURCE_vertexbuffer, GDRAW_PSP2_VERTEXBUFFER_ALIGNMENT, true);
         return gdraw->vbufcache != nullptr;

      default:
         return 0;
   }
}

void gdraw_psp2_ResetAllResourceMemory()
{
   gdraw_psp2_SetResourceMemory(GDRAW_PSP2_RESOURCE_texture, 0, nullptr, 0);
   gdraw_psp2_SetResourceMemory(GDRAW_PSP2_RESOURCE_vertexbuffer, 0, nullptr, 0);
}

GDrawFunctions *gdraw_psp2_CreateContext(SceGxmShaderPatcher *shader_patcher, void *context_mem, volatile U32 *notification, SceGxmOutputRegisterFormat reg_format)
{
   // mask index buffer:
   //
   // 0-----------------------3  y0
   // | \5---------------6 /  |  y1
   // |  |               |    |
   // |  |               |    |
   // |  9---------------a    |  y2
   // | /                  \  |
   // c-----------------------f  y3
   //
   // x0 x1              x2   x3
   static const U16 mask_ib_data[5*2] = {
      // tri strip
      0,5, 3,6, 15,10, 12,9, 0,5,
   };

   gdraw = (GDraw *) IggyGDrawMalloc(sizeof(*gdraw));
   if (!gdraw) return nullptr;

   memset(gdraw, 0, sizeof(*gdraw));

   // context shared memory
   gdraw_arena_init(&gdraw->context_arena, context_mem, GDRAW_PSP2_CONTEXT_MEM_SIZE);

   // notifications
   *notification = 0;
   gdraw->fence_label = notification;
   gdraw->next_fence_index = 1;
   gdraw->scene_end_fence.value = 0;

   // shader patcher
   gdraw->patcher = shader_patcher;

   // set up memory for all resource types
   for (int i=0; i < GDRAW_PSP2_RESOURCE__count; i++)
      gdraw_psp2_SetResourceMemory((gdraw_psp2_resourcetype) i, gdraw_limits[i].num_handles, gdraw_limits[i].ptr, gdraw_limits[i].num_bytes);

   // shaders and state
   gdraw->quad_ib = (U16 *)gdraw_arena_alloc(&gdraw->context_arena, QUAD_IB_COUNT * 6 * sizeof(U16), sizeof(U32));
   gdraw->mask_ib = (U16 *)gdraw_arena_alloc(&gdraw->context_arena, sizeof(mask_ib_data), sizeof(U32));

   if (!gdraw->quad_ib || !gdraw->mask_ib || !create_all_programs(reg_format)) {
      gdraw_psp2_DestroyContext();
      return nullptr;
   }

   // init quad index buffer
   for (int i=0; i < QUAD_IB_COUNT; i++) {
      U16 *out_ind = gdraw->quad_ib + i*6;
      U16 base = (U16)(i * 4);

      out_ind[0] = base + 0; out_ind[1] = base + 1; out_ind[2] = base + 2;
      out_ind[3] = base + 0; out_ind[4] = base + 2; out_ind[5] = base + 3;
   }

   // mask draw (can only alloc this here since we need mask_vp)
   gdraw->mask_draw_gpu = gdraw_arena_alloc(&gdraw->context_arena, sceGxmGetPrecomputedDrawSize(gdraw->mask_vp), SCE_GXM_PRECOMPUTED_ALIGNMENT);
   if (!gdraw->mask_draw_gpu) {
      gdraw_psp2_DestroyContext();
      return nullptr;
   }

   memcpy(gdraw->mask_ib, mask_ib_data, sizeof(mask_ib_data));
   sceGxmPrecomputedDrawInit(&gdraw->mask_draw, gdraw->mask_vp, gdraw->mask_draw_gpu);
   sceGxmPrecomputedDrawSetParams(&gdraw->mask_draw, SCE_GXM_PRIMITIVE_TRIANGLE_STRIP, SCE_GXM_INDEX_FORMAT_U16, gdraw->mask_ib, 5*2);

   // API
   gdraw_funcs.SetViewSizeAndWorldScale = gdraw_SetViewSizeAndWorldScale;
   gdraw_funcs.GetInfo = gdraw_GetInfo;

   gdraw_funcs.DescribeTexture = gdraw_DescribeTexture;
   gdraw_funcs.DescribeVertexBuffer = gdraw_DescribeVertexBuffer;

   gdraw_funcs.RenderingBegin = gdraw_RenderingBegin;
   gdraw_funcs.RenderingEnd = gdraw_RenderingEnd;
   gdraw_funcs.RenderTileBegin = gdraw_RenderTileBegin;
   gdraw_funcs.RenderTileEnd = gdraw_RenderTileEnd;

   gdraw_funcs.TextureDrawBufferBegin = gdraw_TextureDrawBufferBegin;
   gdraw_funcs.TextureDrawBufferEnd = gdraw_TextureDrawBufferEnd;

   gdraw_funcs.DrawIndexedTriangles = gdraw_DrawIndexedTriangles;
   gdraw_funcs.FilterQuad = gdraw_FilterQuad;

   gdraw_funcs.SetAntialiasTexture = gdraw_SetAntialiasTexture;

   gdraw_funcs.ClearStencilBits = gdraw_ClearStencilBits;
   gdraw_funcs.ClearID = gdraw_ClearID;

   gdraw_funcs.MakeTextureBegin = gdraw_MakeTextureBegin;
   gdraw_funcs.MakeTextureMore = gdraw_MakeTextureMore;
   gdraw_funcs.MakeTextureEnd = gdraw_MakeTextureEnd;

   gdraw_funcs.UpdateTextureBegin = gdraw_UpdateTextureBegin;
   gdraw_funcs.UpdateTextureRect = gdraw_UpdateTextureRect;
   gdraw_funcs.UpdateTextureEnd = gdraw_UpdateTextureEnd;

   gdraw_funcs.FreeTexture = gdraw_FreeTexture;
   gdraw_funcs.TryToLockTexture = gdraw_TryToLockTexture;

   gdraw_funcs.MakeVertexBufferBegin = gdraw_MakeVertexBufferBegin;
   gdraw_funcs.MakeVertexBufferMore = gdraw_MakeVertexBufferMore;
   gdraw_funcs.MakeVertexBufferEnd = gdraw_MakeVertexBufferEnd;
   gdraw_funcs.TryToLockVertexBuffer = gdraw_TryLockVertexBuffer;
   gdraw_funcs.FreeVertexBuffer = gdraw_FreeVertexBuffer;

   gdraw_funcs.MakeTextureFromResource = (gdraw_make_texture_from_resource *) gdraw_psp2_MakeTextureFromResource;
   gdraw_funcs.FreeTextureFromResource = gdraw_psp2_DestroyTextureFromResource;

   gdraw_funcs.UnlockHandles = gdraw_UnlockHandles;
   gdraw_funcs.SetTextureUniqueID = gdraw_SetTextureUniqueID;
   
   return &gdraw_funcs;
}

void gdraw_psp2_DestroyContext(void)
{
   if (gdraw) {
      GDrawStats stats;
      memset(&stats, 0, sizeof(stats));
      if (gdraw->texturecache) gdraw_res_flush(gdraw->texturecache, &stats);
      if (gdraw->vbufcache) gdraw_res_flush(gdraw->vbufcache, &stats);

      // make sure the GPU is done first
      assert(!is_fence_pending(gdraw->scene_end_fence));

      free_handle_cache(gdraw->texturecache);
      free_handle_cache(gdraw->vbufcache);
      destroy_all_programs();
      IggyGDrawFree(gdraw);
      gdraw = nullptr;
   }
}

void RADLINK gdraw_psp2_BeginCustomDraw(IggyCustomDrawCallbackRegion *region, float matrix[16])
{
   clear_renderstate();
   gdraw_GetObjectSpaceMatrix(matrix, region->o2w, gdraw->projection, 0.0f, 0);
}

void RADLINK gdraw_psp2_CalculateCustomDraw_4J(IggyCustomDrawCallbackRegion * region, F32 mat[16])
{
   gdraw_GetObjectSpaceMatrix(mat, region->o2w, gdraw->projection, 0.0f, 0);
}

void RADLINK gdraw_psp2_EndCustomDraw(IggyCustomDrawCallbackRegion *region)
{
   set_common_renderstate();
}

GDrawTexture * RADLINK gdraw_psp2_MakeTextureFromResource(U8 *file_in_memory, S32 len, IggyFileTexturePSP2 *tex)
{
   SceGxmErrorCode (*init_func)(SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, uint32_t width, uint32_t height, uint32_t mipCount) = nullptr;

   switch (tex->texture.type) {
      case SCE_GXM_TEXTURE_SWIZZLED:            init_func = sceGxmTextureInitSwizzled;          break;
      case SCE_GXM_TEXTURE_LINEAR:              init_func = sceGxmTextureInitLinear;            break;
      case SCE_GXM_TEXTURE_TILED:               init_func = sceGxmTextureInitTiled;             break;
      case SCE_GXM_TEXTURE_SWIZZLED_ARBITRARY:  init_func = sceGxmTextureInitSwizzledArbitrary; break;
   }

   if (!init_func) {
      IggyGDrawSendWarning(nullptr, "Unsupported texture type in MakeTextureFromResource");
      return nullptr;
   }

   SceGxmTexture gxm;
   SceGxmErrorCode err = init_func(&gxm, file_in_memory + tex->file_offset, (SceGxmTextureFormat)tex->texture.format, tex->texture.width, tex->texture.height, tex->texture.mip_count);
   if (err != SCE_OK) {
      IggyGDrawSendWarning(nullptr, "Texture init failed in MakeTextureFromResource (bad data?)");
      return nullptr;
   }

   return gdraw_psp2_WrappedTextureCreate(&gxm);
}

extern void RADLINK gdraw_psp2_DestroyTextureFromResource(GDrawTexture *tex)
{
   gdraw_psp2_WrappedTextureDestroy(tex);
}

