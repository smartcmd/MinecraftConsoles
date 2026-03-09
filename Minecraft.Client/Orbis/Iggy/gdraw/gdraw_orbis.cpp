#include "stdafx.h"
// gdraw_orbis.cpp - author: Fabian Giesen - copyright 2012 RAD Game Tools
//
// This implements the Iggy graphics driver layer for Orbis.

// GDraw consists of several components that interact fairly loosely with each other;
// e.g. the resource management, drawing and filtering parts are all fairly independent
// of each other. If you want to modify some aspect of GDraw - say the texture allocation
// logic - your best bet is usually to just look for one of the related entry points,
// e.g. MakeTextureBegin, and take it from there. There's a bunch of code in this file,
// but most of it isn't really complicated. The bits that are somewhat tricky have a more
// detailed explanation at the top of the relevant section.
 
#include <kernel.h>
#include <gnm.h>
#include <gnmx.h>
#include "iggy.h"
#include "gdraw.h"
#include <math.h>
#include <kernel.h>
#include <xmmintrin.h>

#include "gdraw_orbis.h"

using namespace sce;

typedef union {
   struct {
      Gnm::Texture *gnm;
      void *gnm_ptr;
   } tex;

   struct {
      void *verts;
      void *inds;
   } vbuf;
} GDrawNativeHandle;

#define GDRAW_MANAGE_MEM
#define GDRAW_DEFRAGMENT
#define GDRAW_NO_STREAMING_MIPGEN
#define GDRAW_MIN_FREE_AMOUNT    (64*1024)   // always try to free at least this many bytes when throwing out old textures
#define GDRAW_MAYBE_UNUSED       __attribute__((unused))
#include "gdraw_shared.inl"

// max rendertarget stack depth. this depends on the extent to which you
// use filters and non-standard blend modes, and how nested they are.
#define MAX_RENDER_STACK_DEPTH   8         // Iggy is hardcoded to a limit of 16... probably 1-3 is realistic!
#define MAX_SAMPLERS             3
#define MAX_ATTRS                2         // max number of attrs read by a vertex shader
#define AATEX_SAMPLER            7         // sampler that aa_tex gets set in

#define ASSERT_COUNT(a,b)        ((a) == (b) ? (b) : -1)

#define MAX_TEXTURE2D_DIM        16384     // from GPU walkthrough
#define MAX_AATEX_WIDTH          64

static GDrawFunctions gdraw_funcs;

// render target state
typedef struct
{
   GDrawHandle *color_buffer;
   S32 base_x, base_y, width, height;
   rrbool cached;
   rrbool needs_clear_eliminate;
   U32 clear_col[2];
} GDrawFramebufferState;

struct ShaderCode
{
   void *blob;
   union
   {
      void *desc;
      Gnmx::ShaderCommonData *common;
      Gnmx::PsShader *ps;
      Gnmx::VsShader *vs;
      Gnmx::CsShader *cs;
   };
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
   Gnmx::GfxContext *gfxc;

   // cached state
   U32 scissor_state;      // ~0 if unknown, otherwise 0 or 1
   int blend_mode;         // active blend mode (-1 if unknown)
   int writes_masked;      // are color writes masked or not? (-1 if unknown)
   U32 z_stencil_key;      // field built from z/stencil test flags. 0 = no z/stencil test, ~0 is used for "unknown state"

   GDrawTexture *active_tex[MAX_SAMPLERS];
   ShaderCode *cur_ps;

   // pixel shader base pointers
   ShaderCode *basic_ps[GDRAW_TEXTURE__count];

   // render targets
   Gnm::RenderTarget main_colorbuffer;
   Gnm::DepthRenderTarget main_depthbuffer;
   GDrawHandleCache rendertargets;
   GDrawHandle rendertarget_handles[MAX_RENDER_STACK_DEPTH]; // not -1, because we use +1 to initialize
   Gnm::Texture rendertarget_textures[MAX_RENDER_STACK_DEPTH+1];

   gswf_recti rt_valid[MAX_RENDER_STACK_DEPTH+1]; // valid rect for texture clamping

   // size of our render targets
   S32 frametex_width, frametex_height;

   // viewport setting (in pixels) for the current tile
   S32 vx, vy;
   S32 fw, fh; // full width/height of virtual display
   S32 tw, th; // actual width/height of current tile
   S32 tpw, tph; // width/height of padded version of tile

   S32 tx0, ty0;
   S32 tx0p, ty0p;

   rrbool in_blur;
   struct {
      S32 x0, y0, x1, y1;
   } cview; // current viewport

   Gnm::Texture aa_tex;
   Gnm::Buffer pixel_common_zero_cbuf;
   GDrawArena vidshared_arena; // mainly for shaders
   
   // synchronization
   volatile U64 *label_ptr;
   U64 next_fence_index;
   
   // render target stack described above for 'temporary' rendering
   GDrawFramebufferState frame[MAX_RENDER_STACK_DEPTH];
   GDrawFramebufferState *cur;

   // texture and vertex buffer pools
   GDrawHandleCache *texturecache;
   GDrawHandleCache *vbufcache;

   // render target storage
   Gnm::RenderTarget rt_colorbuffer;
   Gnm::SizeAlign rt_colorbuffer_sa;
   GDrawArena rt_arena;

   // staging buffer
   GDrawArena staging;
   gdraw_orbis_staging_stats staging_stats;

   // upload temp texture
   Gnm::Texture upload_tex;
   
   // precompiled state
   Gnm::Sampler sampler_state[2][GDRAW_WRAP__count]; // [nearest][wrap]
   Gnm::DepthStencilControl depth_stencil_control[2][2][2];  // [set_id][test_id][stencil_enable]
   Gnm::BlendControl blend_control[GDRAW_BLEND__count];
   
   // pixel shaders
   ShaderCode main_ps[GDRAW_TEXTURE__count][3];
   ShaderCode exceptional_blend[GDRAW_BLENDSPECIAL__count];
   ShaderCode filter_ps[2][16];
   ShaderCode blur_ps[MAX_TAPS+1];
   ShaderCode colormatrix;
   ShaderCode clear_ps;

   // compute shaders
   ShaderCode texupload_cs;
   ShaderCode memset_cs;
   ShaderCode defragment_cs;
   ShaderCode mipgen_cs;

   // vertex formats
   struct VFormatDesc {
      U32 stride;
      U32 num_attribs;
      Gnm::DataFormat formats[MAX_ATTRS];
      U32 vb_offs[MAX_ATTRS];
   };
   VFormatDesc vfmt[GDRAW_vformat__count];

   // vertex shader
   ShaderCode vs;

   // for bookkeeping
   GDrawFence tile_end_fence;

   // antialias texture upload cache
   bool aatex_new;
   U8 aatex_data[MAX_AATEX_WIDTH * 4];
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

static RADINLINE rrbool is_fence_pending(GDrawFence fence)
{
   return gdraw->label_ptr[0] < fence.value;
}

static GDrawFence put_fence()
{
   GDrawFence fence = { gdraw->next_fence_index++ };
   gdraw->gfxc->writeImmediateAtEndOfPipe(Gnm::kEopFlushCbDbCaches, (void *)gdraw->label_ptr, fence.value, Gnm::kCacheActionNone);
   return fence;
}

static void *insert_cb_label();

static void wait_on_fence(GDrawFence fence)
{
   // we don't actually wait here, current Orbis GDraw memory management is
   // non-blocking by design. but the resource manager emits these when it's
   // about to free (and then reuse) a resource that was previously in use,
   // so when we get the call, make sure to finish shading first, because
   // the following commands are likely to stomp over (part of) the resource
   // pools.
   if (is_fence_pending(fence)) {
      void *label = insert_cb_label();
      gdraw->gfxc->writeAtEndOfShader(Gnm::kEosPsDone, label, 1);
      gdraw->gfxc->waitOnAddress(label, ~0, Gnm::kWaitCompareFuncEqual, 1);
   }
}

extern "C" void gdraw_ps4_wait(U64)
{
   // stub for Iggy - ignored.
}

static void *insert_cb_label()
{
   U64 *label = (U64 *) gdraw->gfxc->allocateFromCommandBuffer(sizeof(U64), Gnm::kEmbeddedDataAlignment8);
   *label = 0;
   return label;
}

// compute->compute sync (just wait for previous dispatch to finish)
static void compute_to_compute_sync()
{
   Gnmx::GfxContext *gfxc = gdraw->gfxc;

   void *label = insert_cb_label();
   gfxc->writeAtEndOfShader(Gnm::kEosCsDone, label, 1);
   gfxc->waitOnAddress(label, ~0, Gnm::kWaitCompareFuncEqual, 1);
}

// compute->graphics sync
static void compute_to_graphics_sync()
{
   compute_to_compute_sync();
   // compute writes made it to L2 but not all CU L1s, so need to wipe L1 before we go on.
   gdraw->gfxc->flushShaderCachesAndWait(Gnm::kCacheActionInvalidateL1, 0, Gnm::kStallCommandBufferParserDisable);
}

// render-to-texture sync
static RADINLINE void rtt_sync(void *base, U32 size256)
{
   UINTa addr = (UINTa) base;
   assert((addr & 0xff) == 0);
   U32 base256 = (U32) (addr >> 8);
   gdraw->gfxc->waitForGraphicsWrites(base256, size256,
       Gnm::kWaitTargetSlotCb0,
       Gnm::kCacheActionWriteBackAndInvalidateL1andL2,
       Gnm::kExtendedCacheActionFlushAndInvalidateCbCache,
       Gnm::kStallCommandBufferParserDisable);
}

////////////////////////////////////////////////////////////////////////
//
//   Texture/vertex memory defragmentation support code
//

static void gdraw_gpu_memcpy(GDrawHandleCache *c, void *dst, void *src, U32 num_bytes)
{
   Gnmx::GfxContext *gfxc = gdraw->gfxc;
   Gnm::Buffer src_buf, dst_buf;
   U32 num_16b = (num_bytes + 15) / 16;

   src_buf.initAsDataBuffer(src, Gnm::kDataFormatR32G32B32A32Uint, num_16b);
   dst_buf.initAsDataBuffer(dst, Gnm::kDataFormatR32G32B32A32Uint, num_16b);
   src_buf.setResourceMemoryType(Gnm::kResourceMemoryTypeGC);
   dst_buf.setResourceMemoryType(Gnm::kResourceMemoryTypeGC);

   gfxc->setBuffers(Gnm::kShaderStageCs, 0, 1, &src_buf);
   gfxc->setRwBuffers(Gnm::kShaderStageCs, 0, 1, &dst_buf);
   gfxc->dispatch(1, 1, 1);

   // need to sync before the next one can start (because of potential overlaps)
   // no need to flush L1 because defragment doesn't read any data it just wrote.
   compute_to_compute_sync();
}

static void gdraw_defragment_cache(GDrawHandleCache *c, GDrawStats *stats)
{
   if (!gdraw_CanDefragment(c))
      return;

   gdraw->gfxc->setShaderType(Gnm::kShaderTypeCompute);
   gdraw->gfxc->setCsShader(gdraw->defragment_cs.cs);

   // actual defragmentation...
   gdraw_DefragmentMain(c, GDRAW_DEFRAGMENT_may_overlap, stats);

   // go over all handles and adjust pointers.
   // pointer adjustment is different between textures and vertex buffers
   if (!c->is_vertex) {
      for (S32 i=0; i < c->max_handles; i++) {
         GDrawHandle *h = &c->handle[i];
         if (gdraw_res_is_managed(h)) {
            Gnm::Texture *tex = h->handle.tex.gnm;
            tex->setBaseAddress(h->raw_ptr);
         }
      }
   } else {
      for (S32 i=0; i < c->max_handles; i++) {
         GDrawHandle *h = &c->handle[i];
         if (gdraw_res_is_managed(h)) {
            SINTa index_offs = (U8 *)h->handle.vbuf.inds - (U8 *)h->handle.vbuf.verts;
            h->handle.vbuf.verts = h->raw_ptr;
            h->handle.vbuf.inds = (U8 *)h->raw_ptr + index_offs;
         }
      }
   }

   // synchronize
   compute_to_graphics_sync();
   gdraw->gfxc->setCsShader(nullptr);
   gdraw->gfxc->setShaderType(Gnm::kShaderTypeGraphics);
   
   // don't need to wait till GPU is done since we never access GPU memory from the
   // CPU, we always go through the staging buffer.
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
   gdraw_HandleCacheUnlockAll(gdraw->texturecache);
   gdraw_HandleCacheUnlockAll(gdraw->vbufcache);
}

////////////////////////////////////////////////////////////////////////
//
//   Various helpers
//

struct TexUploadPara
{
   U32 dest_pos[2];
   U32 size[2];
};

struct MipGenPara
{
   U32 sizeMinus1[2];
};

static bool can_staging_satisfy(U32 size, U32 align)
{
   // determine aligned start addr
   UINTa start_addr = ((UINTa) gdraw->staging.current + align-1) & ~((UINTa) align - 1);
   U8 *ptr = (U8 *) start_addr;
   return ptr + size <= gdraw->staging.end;
}

static void track_staging_alloc_attempt(U32 size, U32 align)
{
   // TODO: include alignment bytes in tracking!
   gdraw->staging_stats.allocs_attempted++;
   gdraw->staging_stats.bytes_attempted += size;
   gdraw->staging_stats.largest_bytes_attempted = RR_MAX(gdraw->staging_stats.largest_bytes_attempted, size);
}

static void track_staging_alloc_failed()
{
   if (gdraw->staging_stats.allocs_attempted == gdraw->staging_stats.allocs_succeeded + 1) { // warn the first time we run out of mem
      IggyGDrawSendWarning(nullptr, "GDraw out of staging memory");
   }
}

static void *alloc_staging(U32 size, U32 align)
{
   track_staging_alloc_attempt(size, align);

   void *ptr = gdraw_arena_alloc(&gdraw->staging, size, align);
   if (ptr) {
      gdraw->staging_stats.allocs_succeeded++;
      gdraw->staging_stats.bytes_succeeded += size;
      gdraw->staging_stats.largest_bytes_succeeded = RR_MAX(gdraw->staging_stats.largest_bytes_succeeded, size);
   } else
      track_staging_alloc_failed();

   return ptr;
}

static void *embed_const_buffer_impl(Gnm::ShaderStage stage, U32 index, U32 size)
{
   Gnm::Buffer buf;
   void *ptr = gdraw->gfxc->allocateFromCommandBuffer(size, Gnm::kEmbeddedDataAlignment4);
   buf.initAsConstantBuffer(ptr, size);
   gdraw->gfxc->setConstantBuffers(stage, index, 1, &buf);
   return ptr;
}

template<typename T>
static RADINLINE T *embed_const_buffer(Gnm::ShaderStage stage, U32 index)
{
   return (T *) embed_const_buffer_impl(stage, index, sizeof(T));
}

static void upload_begin()
{
   gdraw->gfxc->setShaderType(Gnm::kShaderTypeCompute);
   gdraw->gfxc->setCsShader(gdraw->texupload_cs.cs);
}

static void upload_end()
{
   // synchronize
   compute_to_graphics_sync();
   gdraw->gfxc->setShaderType(Gnm::kShaderTypeGraphics);
}

static void upload_dispatch(const Gnm::Texture &dst_tex, const Gnm::Texture &src_tex, U32 dst_x, U32 dst_y, U32 ncols, U32 nrows)
{
   auto para = embed_const_buffer<TexUploadPara>(Gnm::kShaderStageCs, 0);
   para->dest_pos[0] = dst_x;
   para->dest_pos[1] = dst_y;
   para->size[0] = ncols;
   para->size[1] = nrows;

   Gnmx::GfxContext *gfxc = gdraw->gfxc;
   gfxc->setTextures(Gnm::kShaderStageCs, 0, 1, &src_tex);
   gfxc->setRwTextures(Gnm::kShaderStageCs, 0, 1, &dst_tex);
   gfxc->dispatch((ncols + 7) / 8, (nrows + 7) / 8, 1);
}

static void upload_tex_subrect(Gnm::Texture *dst, U32 dst_mip, U32 dst_x, U32 dst_y, const void *src, U32 pitch, U32 ncols, U32 nrows, U32 bpp)
{
   // set up texture descriptions
   Gnm::Texture dst_tex = *dst;
   dst_tex.setMipLevelRange(dst_mip, dst_mip);
   dst_tex.setResourceMemoryType(Gnm::kResourceMemoryTypeGC);

   Gnm::Texture src_tex;
   Gnm::SizeAlign sa = src_tex.initAs2d(ncols, nrows, 1, dst_tex.getDataFormat(), Gnm::kTileModeDisplay_LinearAligned, Gnm::kNumSamples1);

   // alloc staging buffer for source texture
   U8 *staging = (U8 *) alloc_staging(sa.m_size, sa.m_align);
   if (!staging) // if out of staging mem, bail
      return;

   src_tex.setBaseAddress(staging);
   src_tex.setResourceMemoryType(Gnm::kResourceMemoryTypeGC);

   // copy source data into staging buffer
   U32 staging_pitch_bytes = src_tex.getPitch() * bpp;
   U32 copy_bytes = ncols * bpp;
   for (U32 y=0; y < nrows; y++)
      memcpy(staging + y * staging_pitch_bytes, (const U8 *) src + y * pitch, copy_bytes);

   upload_dispatch(dst_tex, src_tex, dst_x, dst_y, ncols, nrows);
}

static void gpu_compute_memset(void *ptr, U32 value, U32 size_in_bytes)
{
   assert((size_in_bytes & 3) == 0);

   Gnmx::GfxContext *gfxc = gdraw->gfxc;
   gfxc->setShaderType(Gnm::kShaderTypeCompute);
   gfxc->setCsShader(gdraw->memset_cs.cs);

   auto para = embed_const_buffer<U32>(Gnm::kShaderStageCs, 0);
   *para = value;

   // we tag resources as SC (system coherent); i.e. no L1, write to L2,
   // tag as volatile so we can flush it cheaply.

   Gnm::Buffer buf;
   if (size_in_bytes >= 16) {
      U32 nelem = size_in_bytes / 16;

      buf.initAsDataBuffer(ptr, Gnm::kDataFormatR32G32B32A32Uint, nelem);
      buf.setResourceMemoryType(Gnm::kResourceMemoryTypeSC);
      gfxc->setRwBuffers(Gnm::kShaderStageCs, 0, 1, &buf);
      gfxc->dispatch((nelem + 255) / 256, 1, 1);
   }

   if (size_in_bytes & 0xf) {
      buf.initAsDataBuffer((U8 *)ptr + (size_in_bytes & ~0xf), Gnm::kDataFormatR32Uint, (size_in_bytes & 0xf) / 4);
      buf.setResourceMemoryType(Gnm::kResourceMemoryTypeSC);
      gfxc->setRwBuffers(Gnm::kShaderStageCs, 0, 1, &buf);
      gfxc->dispatch(1, 1, 1);
   }

   compute_to_compute_sync();

   // write back L2 volatile here to make sure the values reach memory.
   // this is all necessary because this function is used to clear HTile
   // buffers which are written by the CB/DB blocks and not accessed
   // through the regular caches.
   gfxc->flushShaderCachesAndWait(Gnm::kCacheActionWriteBackL2Volatile, 0, Gnm::kStallCommandBufferParserDisable);
   gfxc->setShaderType(Gnm::kShaderTypeGraphics);
   gfxc->setCsShader(nullptr);
}

////////////////////////////////////////////////////////////////////////
//
//   Texture creation/updating/deletion
//

GDrawTexture * RADLINK gdraw_orbis_WrappedTextureCreate(Gnm::Texture *tex)
{
   GDrawStats stats = {};
   GDrawHandle *p = gdraw_res_alloc_begin(gdraw->texturecache, 0, &stats);
   p->handle.tex.gnm_ptr = nullptr;
   gdraw_HandleCacheAllocateEnd(p, 0, nullptr, GDRAW_HANDLE_STATE_user_owned);
   gdraw_orbis_WrappedTextureChange((GDrawTexture *) p, tex);
   return (GDrawTexture *) p;
}

void RADLINK gdraw_orbis_WrappedTextureChange(GDrawTexture *handle, Gnm::Texture *tex)
{
   GDrawHandle *p = (GDrawHandle *) handle;
   *p->handle.tex.gnm = *tex;
}

void RADLINK gdraw_orbis_WrappedTextureDestroy(GDrawTexture *handle)
{
	// 4J Changed
	sce::Gnmx::GfxContext *oldContext = gdraw->gfxc;
	gdraw->gfxc = RenderManager.GetCurrentBackBufferContext();
	GDrawStats stats = {};
	gdraw_res_free((GDrawHandle *) handle, &stats);
	gdraw->gfxc = oldContext;
}

static void RADLINK gdraw_SetTextureUniqueID(GDrawTexture *tex, void *old_id, void *new_id)
{
   GDrawHandle *p = (GDrawHandle *) tex;
   // if this is still the handle it's thought to be, change the owner;
   // if the owner *doesn't* match, then they're changing a stale handle, so ignore
   if (p->owner == old_id)
      p->owner = new_id;
}

static rrbool RADLINK gdraw_MakeTextureBegin(void *owner, S32 width, S32 height, gdraw_texture_format gformat, U32 flags, GDraw_MakeTexture_ProcessingInfo *p, GDrawStats *stats)
{
   S32 bytes_pixel = 4;
   GDrawHandle *t = nullptr;
   Gnm::Texture gt;
   Gnm::SizeAlign sa;

   Gnm::DataFormat format = Gnm::kDataFormatR8G8B8A8Unorm;
   if (width > MAX_TEXTURE2D_DIM || height > MAX_TEXTURE2D_DIM) {
      IggyGDrawSendWarning(nullptr, "GDraw %d x %d texture not supported by hardware (dimension limit %d)", width, height, MAX_TEXTURE2D_DIM);
      return false;
   }

   if (gformat == GDRAW_TEXTURE_FORMAT_font) {
      format = Gnm::kDataFormatR8Unorm;
      bytes_pixel = 1;
   }

   // don't do mipmaps for gradients!
   if (height == 1)
      flags &= ~GDRAW_MAKETEXTURE_FLAGS_mipmap;

   // determine the number of mipmaps to use and size of resulting surface
   U32 mipmaps = 0;
   do
      mipmaps++;
   while ((flags & GDRAW_MAKETEXTURE_FLAGS_mipmap) && ((width >> mipmaps) || (height >> mipmaps)));

   sa = gt.initAs2d(width, height, mipmaps, format, (height == 1) ? Gnm::kTileModeDisplay_LinearAligned : Gnm::kTileModeThin_1dThin, Gnm::kNumSamples1);

   if (gformat == GDRAW_TEXTURE_FORMAT_font) // we want an A8 not R8 texture!
      gt.setChannelOrder(Gnm::kTextureChannelX, Gnm::kTextureChannelX, Gnm::kTextureChannelX, Gnm::kTextureChannelX);

   // Make sure we actually satisfy alignment requirements
   assert(sa.m_align <= GDRAW_ORBIS_TEXTURE_ALIGNMENT);

   // Determine space requirements for the upload texture and check if there's enough space
   // do this before gdraw_res_alloc_begin so we don't start freeing resources to make space
   // only to later discover that we can't proceed due to lack of staging mem anyway.
   Gnm::SizeAlign sa_up = gdraw->upload_tex.initAs2d(width, height, 1, format, Gnm::kTileModeDisplay_LinearAligned, Gnm::kNumSamples1);
   if (!can_staging_satisfy(sa_up.m_size, sa_up.m_align)) {
      track_staging_alloc_attempt(sa_up.m_size, sa_up.m_align);
      track_staging_alloc_failed();
      return false;
   }

   // allocate a handle and make room in the cache for this much data
   U32 size = sa.m_size;
   t = gdraw_res_alloc_begin(gdraw->texturecache, size, stats);
   if (!t)
      return false;

   t->handle.tex.gnm_ptr = t->raw_ptr;
   gt.setBaseAddress(t->raw_ptr);
   *t->handle.tex.gnm = gt;

   // allocate staging texture (we checked that there was enough space earlier)
   void *upload_ptr = alloc_staging(sa_up.m_size, sa_up.m_align);
   if (!upload_ptr) {
      // not supposed to happen - we checked there was enough space earlier!
      // but if we ever get here, be sure to handle it properly anyway.
      assert(0);
      gdraw_HandleCacheAllocateFail(t);
      return false;
   }
   gdraw->upload_tex.setBaseAddress(upload_ptr);

   gdraw_HandleCacheAllocateEnd(t, size, owner, (flags & GDRAW_MAKETEXTURE_FLAGS_never_flush) ? GDRAW_HANDLE_STATE_pinned : GDRAW_HANDLE_STATE_locked);
   stats->nonzero_flags |= GDRAW_STATS_alloc_tex;
   stats->alloc_tex += 1;
   stats->alloc_tex_bytes += size;

   p->texture_type = GDRAW_TEXTURE_TYPE_rgba;
   p->p0 = t;
   p->texture_data = (U8 *) upload_ptr;
   p->num_rows = height; // just send the whole texture at once
   p->stride_in_bytes = gdraw->upload_tex.getPitch() * bytes_pixel;

   return true;
}

static rrbool RADLINK gdraw_MakeTextureMore(GDraw_MakeTexture_ProcessingInfo *p)
{
   return false; // we always let the user write the full texture on the first try
}

static GDrawTexture * RADLINK gdraw_MakeTextureEnd(GDraw_MakeTexture_ProcessingInfo *p, GDrawStats *stats)
{
   GDrawHandle *t = (GDrawHandle *) p->p0;
   Gnm::Texture *gnm_tex = t->handle.tex.gnm;
   Gnmx::GfxContext *gfxc = gdraw->gfxc;
   U32 width = gnm_tex->getWidth();
   U32 height = gnm_tex->getHeight();

   // upload the mip data
   upload_begin();

   Gnm::Texture dst_tex = *gnm_tex;
   dst_tex.setResourceMemoryType(Gnm::kResourceMemoryTypeGC);
   Gnm::Texture src_tex = dst_tex;
   dst_tex.setMipLevelRange(0, 0);
   upload_dispatch(dst_tex, gdraw->upload_tex, 0, 0, width, height);

   upload_end();

   // compute the mip maps
   gfxc->setShaderType(Gnm::kShaderTypeCompute);
   gfxc->setCsShader(gdraw->mipgen_cs.cs);

   for (U32 mip=1; mip <= gnm_tex->getLastMipLevel(); mip++) {
      U32 mipw = RR_MAX(gnm_tex->getWidth() >> mip, 1);
      U32 miph = RR_MAX(gnm_tex->getHeight() >> mip, 1);

      src_tex.setMipLevelRange(mip - 1, mip - 1);
      dst_tex.setMipLevelRange(mip, mip);

      auto para = embed_const_buffer<MipGenPara>(Gnm::kShaderStageCs, 0);
      para->sizeMinus1[0] = RR_MAX(gnm_tex->getWidth() >> (mip - 1), 1) - 1;
      para->sizeMinus1[1] = RR_MAX(gnm_tex->getHeight() >> (mip - 1), 1) - 1;

      gfxc->setTextures(Gnm::kShaderStageCs, 0, 1, &src_tex);
      gfxc->setRwTextures(Gnm::kShaderStageCs, 0, 1, &dst_tex);
      gfxc->dispatch((mipw + 7) / 8, (miph + 7) / 8, 1);
      if (mip < gnm_tex->getLastMipLevel())
         compute_to_compute_sync();
      else
         compute_to_graphics_sync();
   }

   gfxc->setShaderType(Gnm::kShaderTypeGraphics);

   return (GDrawTexture *) p->p0;
}

static rrbool RADLINK gdraw_UpdateTextureBegin(GDrawTexture *t, void *unique_id, GDrawStats *stats)
{
   if (gdraw_HandleCacheLock((GDrawHandle *) t, unique_id)) {
      upload_begin();
      return true;
   } else
      return false;
}

static void RADLINK gdraw_UpdateTextureRect(GDrawTexture *t, void *unique_id, S32 x, S32 y, S32 stride, S32 w, S32 h, U8 *samples, gdraw_texture_format format)
{
   GDrawHandle *s = (GDrawHandle *) t;
   Gnm::Texture *tex = s->handle.tex.gnm;
   U32 bpp = (format == GDRAW_TEXTURE_FORMAT_font) ? 1 : 4;

   upload_tex_subrect(tex, 0, x, y, samples, stride, w, h, bpp);
}

static void RADLINK gdraw_UpdateTextureEnd(GDrawTexture *t, void *unique_id, GDrawStats *stats)
{
   GDrawHandle *s = (GDrawHandle *) t;
   upload_end();
   gdraw_HandleCacheUnlock(s);
}

static void RADLINK gdraw_FreeTexture(GDrawTexture *tt, void *unique_id, GDrawStats *stats)
{
   GDrawHandle *t = (GDrawHandle *) tt;
   assert(t != nullptr);
   if (t->owner == unique_id || unique_id == nullptr) {
      if (t->cache == &gdraw->rendertargets) {
         gdraw_HandleCacheUnlock(t);
         // cache it by simply not freeing it
         return;
      }

      gdraw_res_kill(t, stats);
   }
}

static rrbool RADLINK gdraw_TryToLockTexture(GDrawTexture *t, void *unique_id, GDrawStats *stats)
{
   return gdraw_HandleCacheLock((GDrawHandle *) t, unique_id);
}

static void RADLINK gdraw_DescribeTexture(GDrawTexture *tex, GDraw_Texture_Description *desc)
{
   GDrawHandle *p = (GDrawHandle *) tex;
   desc->width = p->handle.tex.gnm->getWidth();
   desc->height = p->handle.tex.gnm->getHeight();
   desc->size_in_bytes = p->bytes;
}

static void antialias_tex_upload()
{
   if (!gdraw->aatex_new || !gdraw->gfxc)
      return;

   U32 width = gdraw->aa_tex.getWidth();

   upload_begin();
   upload_tex_subrect(&gdraw->aa_tex, 0, 0, 0, gdraw->aatex_data, width*4, width, 1, 4);
   upload_end();
   gdraw->aatex_new = false;
}

static void RADLINK gdraw_SetAntialiasTexture(S32 width, U8 *rgba)
{
   if (gdraw->aa_tex.isTexture())
      return;

   Gnm::SizeAlign sa = gdraw->aa_tex.initAs2d(width, 1, 1, Gnm::kDataFormatR8G8B8A8Unorm, Gnm::kTileModeDisplay_LinearAligned, Gnm::kNumSamples1);
   void *ptr = gdraw_arena_alloc(&gdraw->vidshared_arena, sa.m_size, sa.m_align);
   if (!ptr)
      return;
   gdraw->aa_tex.setBaseAddress(ptr);

   assert(width <= MAX_AATEX_WIDTH);
   memcpy(gdraw->aatex_data, rgba, width*4);
   gdraw->aatex_new = true;

   antialias_tex_upload();
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
   p->vertex_data_length = vbuf_size;
   p->index_data_length = ibuf_size;

   // need to go through staging buffer for uploads
   p->p1 = alloc_staging(vbuf_size + ibuf_size, Gnm::kAlignmentOfBufferInBytes);
   if (!p->p1) {
      gdraw_HandleCacheAllocateFail(vb);
      return false;
   }

   p->vertex_data = (U8 *) p->p1;
   p->index_data = (U8 *) p->p1 + vbuf_size;
   p->i0 = vbuf_size + ibuf_size;

   gdraw_HandleCacheAllocateEnd(vb, vbuf_size + ibuf_size, unique_id, GDRAW_HANDLE_STATE_locked);
   return true;
}

static rrbool RADLINK gdraw_MakeVertexBufferMore(GDraw_MakeVertexBuffer_ProcessingInfo *p)
{
   assert(0);
   return false;
}

static GDrawVertexBuffer * RADLINK gdraw_MakeVertexBufferEnd(GDraw_MakeVertexBuffer_ProcessingInfo *p, GDrawStats *stats)
{
   GDrawHandle *vb = (GDrawHandle *) p->p0;

   // DMA from staging buffer to actual target address.
   gdraw->gfxc->copyData(vb->raw_ptr, p->p1, p->i0, Gnm::kDmaDataBlockingEnable);

   // Flush shader L1 & L2 so we can safely use the updated VB
   // need to stall parsing of the command buffer because if the next
   // command is drawing this vertex buffer, PS4 gpu will prefetch
   // the index data, and since we haven't flushed yet, it can fetch
   // bogus index data
   gdraw->gfxc->flushShaderCachesAndWait(Gnm::kCacheActionWriteBackAndInvalidateL1andL2, 0, Gnm::kStallCommandBufferParserEnable);

   return (GDrawVertexBuffer *) vb;
}

static rrbool RADLINK gdraw_TryLockVertexBuffer(GDrawVertexBuffer *vb, void *unique_id, GDrawStats *stats)
{
   return gdraw_HandleCacheLock((GDrawHandle *) vb, unique_id);
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
//   Create/free (or cache) framebuffer-sized textures
//

static GDrawHandle *get_color_rendertarget(GDrawStats *stats)
{
   GDrawHandle *t;

   t = gdraw_HandleCacheGetLRU(&gdraw->rendertargets);
   if (t) {
      gdraw_HandleCacheLock(t, (void *) 1);
      return t;
   }

   t = gdraw_HandleCacheAllocateBegin(&gdraw->rendertargets);
   if (!t) {
      IggyGDrawSendWarning(nullptr, "GDraw rendertarget allocation failed: hit handle limit");
      return t;
   }

   U8 *ptr = (U8 *)gdraw_arena_alloc(&gdraw->rt_arena, gdraw->rt_colorbuffer_sa.m_size, gdraw->rt_colorbuffer_sa.m_align);
   if (!ptr) {
      IggyGDrawSendWarning(nullptr, "GDraw rendertarget allocation failed: out of rendertarget texture memory");
      gdraw_HandleCacheAllocateFail(t);
      return nullptr;
   }

   t->fence = get_next_fence();
   t->raw_ptr = nullptr;

   t->handle.tex.gnm_ptr = ptr;
   t->handle.tex.gnm->initFromRenderTarget(&gdraw->rt_colorbuffer, false);
   t->handle.tex.gnm->setBaseAddress(ptr);

   gdraw_HandleCacheAllocateEnd(t, gdraw->rt_colorbuffer_sa.m_size, (void *) 1, GDRAW_HANDLE_STATE_locked);
   return t;
}

////////////////////////////////////////////////////////////////////////
//
//   Constant buffer layouts
//

struct VertexVars
{
   F32 world[2][4];
   F32 x_off[4];
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

struct PixelParaFilter
{
   F32 clamp0[4], clamp1[4];
   F32 color[4], color2[4];
   F32 tc_off[4];
};

struct PixelParaBlur
{
   F32 clamp[4];
   F32 tap[9][4];
};

struct PixelParaColorMatrix
{
   F32 data[5][4];
};

////////////////////////////////////////////////////////////////////////
//
//   Rendering helpers
//

static void set_gnm_texture(U32 unit, Gnm::Texture *tex, U32 wrap, U32 nearest)
{
   assert(wrap < GDRAW_WRAP__count);
   assert(nearest < 2);

   Gnmx::GfxContext *gfxc = gdraw->gfxc;
   gfxc->setSamplers(Gnm::kShaderStagePs, unit, 1, &gdraw->sampler_state[nearest][wrap]);
   gfxc->setTextures(Gnm::kShaderStagePs, unit, 1, tex);
}

static inline void disable_scissor(bool force)
{
   if (force || gdraw->scissor_state) {
      // set whole viewport as scissor rect
      gdraw->scissor_state = 0;
      gdraw->gfxc->setScreenScissor(gdraw->cview.x0, gdraw->cview.y0, gdraw->cview.x1, gdraw->cview.y1);
   }
}

static void set_viewport_raw(S32 x, S32 y, S32 w, S32 h)
{
   // check against hardware limits
   assert(w >= 0 && w <= 16384);
   assert(h >= 0 && h <= 16384);

   gdraw->cview.x0 = x;
   gdraw->cview.y0 = y;
   gdraw->cview.x1 = x + w;
   gdraw->cview.y1 = y + h;

   F32 scale[3] = { (F32)w * 0.5f, -(F32)h * 0.5f, 1.0f };
   F32 offset[3] = { (F32)x + (F32)w * 0.5f, (F32)y + (F32)h * 0.5f, 0.0f };
   gdraw->gfxc->setViewport(0, 0.0f, 1.0f, scale, offset);
   disable_scissor(true);
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
   if (gdraw->in_blur) { // blur needs special setup
      set_viewport_raw(0, 0, gdraw->tpw, gdraw->tph);
      return;
   }

   if (gdraw->cur == gdraw->frame) // if the rendering stack is empty
      // render a tile-sized region to the user-request tile location
      set_viewport_raw(gdraw->vx, gdraw->vy, gdraw->tw, gdraw->th);
   else if (gdraw->cur->cached)
      set_viewport_raw(0, 0, gdraw->cur->width, gdraw->cur->height);
   else
      // if on the render stack, draw a padded-tile-sized region at the origin
      set_viewport_raw(0, 0, gdraw->tpw, gdraw->tph);
}

static void set_projection()
{
   if (gdraw->in_blur) { // blur needs special setup
      set_projection_raw(0, gdraw->tpw, gdraw->tph, 0);
      return;
   }

   if (gdraw->cur == gdraw->frame) // if the render stack is empty
      set_projection_raw(gdraw->tx0,gdraw->tx0+gdraw->tw,gdraw->ty0+gdraw->th,gdraw->ty0);
   else if (gdraw->cur->cached)
      set_projection_raw(gdraw->cur->base_x, gdraw->cur->base_x + gdraw->cur->width, gdraw->cur->base_y + gdraw->cur->height, gdraw->cur->base_y);
   else
      set_projection_raw(gdraw->tx0p,gdraw->tx0p+gdraw->tpw,gdraw->ty0p+gdraw->tph,gdraw->ty0p);
}

static void set_render_target()
{
   Gnmx::GfxContext *gfxc = gdraw->gfxc;
   if (GDrawHandle *color = gdraw->cur->color_buffer) {
      gdraw->rt_colorbuffer.setBaseAddress256ByteBlocks(color->handle.tex.gnm->getBaseAddress256ByteBlocks());
      gfxc->setRenderTarget(0, &gdraw->rt_colorbuffer);
   } else
      gfxc->setRenderTarget(0, &gdraw->main_colorbuffer);

   gfxc->setDepthRenderTarget(&gdraw->main_depthbuffer);
   gfxc->setCmaskClearColor(0, gdraw->cur->clear_col);
}

static void clear_renderstate()
{
   Gnmx::GfxContext *gfxc = gdraw->gfxc;
   gfxc->setDepthStencilControl(gdraw->depth_stencil_control[0][0][0]);
   gfxc->setRenderTargetMask(0xf);
   gfxc->setBlendControl(0, gdraw->blend_control[GDRAW_BLEND_none]);

   disable_scissor(false);
   gdraw->z_stencil_key = 0;
   gdraw->writes_masked = 0;
   gdraw->blend_mode = GDRAW_BLEND_none;
}

static void set_common_renderstate()
{
   Gnmx::GfxContext *gfxc = gdraw->gfxc;

   // clear our state caching
   memset(gdraw->active_tex, 0, sizeof(gdraw->active_tex));
   gdraw->cur_ps = nullptr;
   gdraw->scissor_state = ~0u;
   gdraw->blend_mode = -1;
   
   // all the state we won't touch again until we're done rendering
   Gnm::ClipControl clip_control;
   clip_control.init();
   clip_control.setClipSpace(Gnm::kClipControlClipSpaceDX);
   gfxc->setClipControl(clip_control);
   gfxc->setShaderType(Gnm::kShaderTypeGraphics);
   gfxc->setIndexSize(Gnm::kIndexSize16);
   gfxc->setStencilClearValue(0);
   gfxc->setDepthClearValue(1.0f);
   gfxc->setVsShader(gdraw->vs.vs, 0, (void*)0);

   Gnm::StencilOpControl stencil_op;
   stencil_op.init();
   stencil_op.setStencilOps(Gnm::kStencilOpKeep, Gnm::kStencilOpReplaceTest, Gnm::kStencilOpKeep);
   gfxc->setStencilOpControl(stencil_op);

   Gnm::ViewportTransformControl vt_control;
   vt_control.init();
   vt_control.setPassThroughEnable(false);
   gfxc->setViewportTransformControl(vt_control);

   // set up guard band and hardware screen offset once
   // we know ahead of time which viewports we're going to set:
   // * our viewport top/left corner is always >= (0,0)
   // * viewport bottom/right is <= size of the largest render target
   S32 min_x = 0;
   S32 min_y = 0;
   S32 max_x = RR_MAX(gdraw->main_colorbuffer.getWidth(), gdraw->rt_colorbuffer.getWidth());
   S32 max_y = RR_MAX(gdraw->main_colorbuffer.getHeight(), gdraw->rt_colorbuffer.getHeight());

   F32 offs_x = (F32) (min_x + max_x) * 0.5f;
   F32 offs_y = (F32) (min_y + max_y) * 0.5f;
   F32 abs_scale_x = (F32) (max_x - min_x) * 0.5f;
   F32 abs_scale_y = (F32) (max_y - min_y) * 0.5f;

   // set up guard band offset so we're centered around our viewport region
   // hardware offset must be a multiple of 16 pixels
   S32 hw_offset_x = (S32)floorf(offs_x/16.0f + 0.5f) * 16;
   S32 hw_offset_y = (S32)floorf(offs_y/16.0f + 0.5f) * 16;
   gfxc->setHardwareScreenOffset(hw_offset_x >> 4, hw_offset_y >> 4);

   // set up guard band clip and discard distances
   // NB both the values for hw_min and hw_max are slightly smaller than the actual min/max
   // (by about 1/256th) to keep a bit of a safety margin for FP round-off error
   F32 hw_min = -(F32)(0xff<<16) / (F32)(1<<8);
   F32 hw_max =  (F32)(0xff<<16) / (F32)(1<<8);
   F32 gb_max_x = RR_MIN(hw_max - abs_scale_x - offs_x + hw_offset_x, -abs_scale_x + offs_x - hw_offset_x - hw_min);
   F32 gb_max_y = RR_MIN(hw_max - abs_scale_y - offs_y + hw_offset_y, -abs_scale_y + offs_y - hw_offset_y - hw_min);
   gfxc->setGuardBandClip(gb_max_x / abs_scale_x, gb_max_y / abs_scale_y);
   gfxc->setGuardBandDiscard(1.0f, 1.0f);

   assert(gdraw->aa_tex.isTexture()); // if this triggers, your initialization sequence is wrong.
   set_gnm_texture(AATEX_SAMPLER, &gdraw->aa_tex, GDRAW_WRAP_clamp, 0);

   // states we modify during regular rendering
   clear_renderstate();
   set_render_target();
   set_viewport();
   set_projection();
}

static void set_pixel_shader(ShaderCode *ps);
static void do_screen_quad(gswf_recti *s, const F32 *tc, GDrawStats *stats);

static void render_clear_quad(gswf_recti *r, GDrawStats *stats)
{
   set_pixel_shader(&gdraw->clear_ps);
   do_screen_quad(r, four_zeros, stats);

   stats->nonzero_flags |= GDRAW_STATS_clears;
   stats->num_clears++;
   stats->cleared_pixels += (r->x1 - r->x0) * (r->y1 - r->y0);
}

static void manual_clear_color(S32 x, S32 y, S32 w, S32 h, GDrawStats *stats)
{
   clear_renderstate();
   set_viewport_raw(0, 0, gdraw->frametex_width, gdraw->frametex_height);
   set_projection_raw(0, gdraw->frametex_width, gdraw->frametex_height, 0);

   gswf_recti r = { x, y, x+w, y+h };
   gdraw->gfxc->setConstantBuffers(Gnm::kShaderStagePs, 0, 1, &gdraw->pixel_common_zero_cbuf);
   render_clear_quad(&r, stats);
}

static void clear_whole_zs(bool clear_depth, bool clear_stencil, GDrawStats *stats)
{
   Gnm::DepthRenderTarget &depthbuf = gdraw->main_depthbuffer;
   
   // to clear both depth and stencil, we can just set up the metadata in HTile with a
   // compute shader if a) there's a HTile to begin with and b) it contains stencil metadata.
   // if no stencil info in HTile, clearing HTile manually is a net perf loss.
   if (clear_depth && clear_stencil && depthbuf.getHtileAccelerationEnable() && !depthbuf.getHtileStencilDisable()) {
      gdraw->gfxc->triggerEvent(Gnm::kEventTypeFlushAndInvalidateDbMeta);
      gpu_compute_memset(depthbuf.getHtileAddress(), 0xfffc00f0, depthbuf.getHtileSizeInBytes());
   } else {
      S32 w = gdraw->main_depthbuffer.getWidth();
      S32 h = gdraw->main_depthbuffer.getHeight();

      Gnmx::GfxContext *gfxc = gdraw->gfxc;
      Gnm::DbRenderControl db_control;
      db_control.init();
      db_control.setDepthClearEnable(clear_depth);
      db_control.setStencilClearEnable(clear_stencil);
      gfxc->setDbRenderControl(db_control);

      Gnm::DepthStencilControl ds_control;
      ds_control.init();
      ds_control.setDepthControl(clear_depth ? Gnm::kDepthControlZWriteEnable : Gnm::kDepthControlZWriteDisable, Gnm::kCompareFuncAlways);
      ds_control.setStencilFunction(Gnm::kCompareFuncAlways);
      ds_control.setDepthEnable(clear_depth);
      ds_control.setStencilEnable(clear_stencil);
      gfxc->setDepthStencilControl(ds_control);

      Gnm::StencilControl st_control;
      st_control.m_testVal = 255;
      st_control.m_mask = 255;
      st_control.m_writeMask = 255;
      st_control.m_opVal = 0;
      gfxc->setStencil(st_control);

      set_viewport_raw(0, 0, w, h);
      set_projection_raw(0, w, h, 0);
      gfxc->setRenderTargetMask(0);

      gswf_recti r = { 0, 0, w, h };
      gfxc->setConstantBuffers(Gnm::kShaderStagePs, 0, 1, &gdraw->pixel_common_zero_cbuf);
      render_clear_quad(&r, stats);

      db_control.init();
      gfxc->setDbRenderControl(db_control);
      clear_renderstate();
      set_viewport();
      set_projection();
   }
}

static void eliminate_fast_clear()
{
   if (!gdraw->cur->needs_clear_eliminate)
      return;

   clear_renderstate();

   Gnmx::GfxContext *gfxc = gdraw->gfxc;
   gfxc->triggerEvent(Gnm::kEventTypeFlushAndInvalidateCbMeta); // flush CMask data

   gswf_recti r;
   if (gdraw->cur != gdraw->frame) {
      r.x0 = gdraw->cur->base_x;
      r.y0 = gdraw->cur->base_y;
      r.x1 = r.x0 + gdraw->cur->width;
      r.y1 = r.y0 + gdraw->cur->height;
   } else {
      r.x0 = 0;
      r.y0 = 0;
      r.x1 = gdraw->main_colorbuffer.getWidth();
      r.y1 = gdraw->main_colorbuffer.getHeight();
   }

   gfxc->setCbControl(Gnm::kCbModeEliminateFastClear, Gnm::kRasterOpSrcCopy);
   gfxc->setPsShader(nullptr);
   set_viewport_raw(r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);
   set_projection_raw(r.x0, r.x1, r.y1, r.y0);
   GDrawStats stats = {}; // we already counted these clears once, so don't add to main stats
   gfxc->setConstantBuffers(Gnm::kShaderStagePs, 0, 1, &gdraw->pixel_common_zero_cbuf);
   render_clear_quad(&r, &stats);

   void *label = insert_cb_label();
   gfxc->writeImmediateAtEndOfPipe(Gnm::kEopFlushCbDbCaches, label, 1, Gnm::kCacheActionNone);
   gfxc->waitOnAddress(label, ~0u, Gnm::kWaitCompareFuncEqual, 1);

   gfxc->setCbControl(Gnm::kCbModeNormal, Gnm::kRasterOpSrcCopy);
   set_viewport();
   set_projection();

   gdraw->cur_ps = nullptr;
   gdraw->cur->needs_clear_eliminate = false;
}

////////////////////////////////////////////////////////////////////////
//
//   Begin rendering for a frame
//

void gdraw_orbis_SetTileOrigin(Gnm::RenderTarget *color, Gnm::DepthRenderTarget *depth, S32 x, S32 y)
{
   gdraw->main_colorbuffer = *color;
   gdraw->main_depthbuffer = *depth;
   gdraw->vx = x;
   gdraw->vy = y;
}

static inline U32 pack8(F32 v)
{
   if (v < 0.0f) v = 0.0f;
   if (v > 1.0f) v = 1.0f;
   return (U32) (S32) (v * 255.0f + 0.5f);
}

static inline U32 pack_color_8888(F32 x, F32 y, F32 z, F32 w)
{
   return (pack8(x) << 0) | (pack8(y) << 8) | (pack8(z) << 16) | (pack8(w) << 24);
}

void gdraw_orbis_ClearWholeRenderTarget(const F32 clear_color_rgba[4])
{
   assert(gdraw->gfxc != nullptr); // call after gdraw_orbis_Begin

   gdraw->cur = gdraw->frame;
   set_common_renderstate();
   clear_renderstate();

   if (gdraw->main_colorbuffer.getCmaskFastClearEnable()) {
      Gnmx::GfxContext *gfxc = gdraw->gfxc;

      // CB flush before
      gfxc->triggerEvent(Gnm::kEventTypeFlushAndInvalidateCbPixelData);

      // Clear Cmask
      gfxc->fillData(gdraw->main_colorbuffer.getCmaskAddress(), 0, gdraw->main_colorbuffer.getCmaskSizeInBytes(), Gnm::kDmaDataBlockingEnable);

      // CB flush after
      void *label = insert_cb_label();
      gfxc->writeImmediateAtEndOfPipe(Gnm::kEopFlushCbDbCaches, label, 1, Gnm::kCacheActionNone);
      gfxc->waitOnAddress(label, ~0u, Gnm::kWaitCompareFuncEqual, 1);

      // Set Cmask clear color
      Gnm::DataFormat fmt = gdraw->main_colorbuffer.getDataFormat();
      if (fmt.m_asInt == Gnm::kDataFormatB8G8R8A8Unorm.m_asInt || fmt.m_asInt == Gnm::kDataFormatB8G8R8X8Unorm.m_asInt) {
         gdraw->cur->clear_col[0] = pack_color_8888(clear_color_rgba[2], clear_color_rgba[1], clear_color_rgba[0], clear_color_rgba[3]);
         gdraw->cur->clear_col[1] = 0;
      } else if (fmt.m_asInt == Gnm::kDataFormatR8G8B8A8Unorm.m_asInt) {
         gdraw->cur->clear_col[0] = pack_color_8888(clear_color_rgba[0], clear_color_rgba[1], clear_color_rgba[2], clear_color_rgba[3]);
      } else
         assert(0); // unsupported color format!

      gfxc->setCmaskClearColor(0, gdraw->cur->clear_col);
      gdraw->cur->needs_clear_eliminate = true;
   } else {
      auto para = embed_const_buffer<PixelCommonVars>(Gnm::kShaderStagePs, 0);
      memset(para, 0, sizeof(*para));
      for (U32 i=0; i < 4; i++)
         para->color_mul[i] = clear_color_rgba[i];

      GDrawStats stats = {};
      gswf_recti r = { 0, 0, (S32) gdraw->main_colorbuffer.getWidth(), (S32) gdraw->main_colorbuffer.getHeight() };
      set_viewport_raw(0, 0, r.x1, r.y1);
      set_projection_raw(0, r.x1, r.y1, 0);
      render_clear_quad(&r, &stats);
   }
}

static void RADLINK gdraw_SetViewSizeAndWorldScale(S32 w, S32 h, F32 scalex, F32 scaley)
{
   gdraw->cur = gdraw->frame;
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
   assert(gdraw->gfxc != nullptr); // call after gdraw_orbis_Begin

   // unbind all shaders
   Gnmx::GfxContext *gfxc = gdraw->gfxc;
   gfxc->setVsShader(nullptr, 0, (void*)0);
   gfxc->setPsShader(nullptr);
   gfxc->setCsShader(nullptr);
   gfxc->setLsHsShaders(nullptr, 0, (void*)0, nullptr, 0);
   gfxc->setEsShader(nullptr, 0, (void *) 0);
   gfxc->setGsVsShaders(nullptr);

   set_common_renderstate();
}

static void RADLINK gdraw_RenderingEnd(void)
{
   clear_renderstate();
}

static void RADLINK gdraw_RenderTileBegin(S32 x0, S32 y0, S32 x1, S32 y1, S32 pad, GDrawStats *stats)
{
   if (x0 == 0 && y0 == 0 && x1 == gdraw->fw && y1 == gdraw->fh)
      pad = 0;

   gdraw->tx0 = x0;
   gdraw->ty0 = y0;
   gdraw->tw = x1-x0;
   gdraw->th = y1-y0;

   // padded region
   gdraw->tx0p = RR_MAX(x0 - pad, 0);
   gdraw->ty0p = RR_MAX(y0 - pad, 0);
   gdraw->tpw = RR_MIN(x1 + pad, gdraw->fw) - gdraw->tx0p;
   gdraw->tph = RR_MIN(y1 + pad, gdraw->fh) - gdraw->ty0p;

   // check if this fits inside our rendertarget buffers
   assert(gdraw->tpw <= gdraw->frametex_width && gdraw->tph <= gdraw->frametex_height);

   gdraw->frame->base_x = gdraw->tx0p;
   gdraw->frame->base_y = gdraw->ty0p;
   gdraw->frame->width = gdraw->tpw;
   gdraw->frame->height = gdraw->tph;

   // clear our depth/stencil buffers
   clear_whole_zs(true, true, stats);
}

static void RADLINK gdraw_RenderTileEnd(GDrawStats *stats)
{
   gdraw->tile_end_fence = put_fence();

   // reap once per frame even if there are no allocs
   gdraw_res_reap(gdraw->texturecache, stats);
   gdraw_res_reap(gdraw->vbufcache, stats);
}

GDRAW_MAYBE_UNUSED static bool mem_is_direct_and_write_combined_or_cached(const void *mem)
{
   SceKernelVirtualQueryInfo info;
   if (sceKernelVirtualQuery(mem, 0, &info, sizeof(info)) != 0)
      return false; // if this errors, mem is likely not even mapped!

   if (!info.isDirectMemory)
      return false;

   return true;
}

void gdraw_orbis_Begin(sce::Gnmx::GfxContext *context, void *staging_buffer, U32 staging_buf_bytes)
{
   assert(gdraw->gfxc == nullptr); // may not nest Begin calls

   // make sure that the memory setup is sensible.
   // if any of these asserts fire, please relocate your command buffers
   // and staging buffers to direct memory that is either cached or
   // write-combined!
   assert(mem_is_direct_and_write_combined_or_cached(context->m_dcb.m_cmdptr));
   assert(mem_is_direct_and_write_combined_or_cached(context->m_ccb.m_cmdptr));
   assert(mem_is_direct_and_write_combined_or_cached(staging_buffer));

   gdraw->gfxc = context;
   gdraw_arena_init(&gdraw->staging, staging_buffer, staging_buf_bytes);
   memset(&gdraw->staging_stats, 0, sizeof(gdraw->staging_stats));

   context->initializeToDefaultContextState();
   antialias_tex_upload();
}

void gdraw_orbis_End(gdraw_orbis_staging_stats *stats)
{
   assert(gdraw->gfxc != nullptr); // please keep Begin / End pairs properly matched

   gdraw_HandleCacheTick(gdraw->texturecache, gdraw->tile_end_fence);
   gdraw_HandleCacheTick(gdraw->vbufcache, gdraw->tile_end_fence);

   gdraw_arena_init(&gdraw->staging, nullptr, 0);
   gdraw->gfxc = nullptr;

   if (stats)
      *stats = gdraw->staging_stats;
}

void gdraw_orbis_EliminateFastClears(void)
{
   assert(gdraw->gfxc != nullptr); // call between gdraw_orbis_Begin and gdraw_orbis_End

   eliminate_fast_clear();
}

#define MAX_DEPTH_VALUE (1 << 14)

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
}

////////////////////////////////////////////////////////////////////////
//
//   Render targets
//

static rrbool RADLINK gdraw_TextureDrawBufferBegin(gswf_recti *region, gdraw_texture_format format, U32 flags, void *owner, GDrawStats *stats)
{
   GDrawFramebufferState *n = gdraw->cur+1;
   GDrawHandle *t;
   if (gdraw->tw == 0 || gdraw->th == 0) {
      IggyGDrawSendWarning(nullptr, "GDraw warning: w=0,h=0 rendertarget");
      return false;
   }

   if (n >= &gdraw->frame[MAX_RENDER_STACK_DEPTH]) {
      IggyGDrawSendWarning(nullptr, "GDraw rendertarget nesting exceeds MAX_RENDER_STACK_DEPTH");
      return false;
   }

   if (owner) {
      // @TODO implement
      t = nullptr;
      assert(0); // nyi
   } else {
      t = get_color_rendertarget(stats);
      if (!t)
         return false;
   }

   n->color_buffer = t;
   assert(n->color_buffer != nullptr); // @GDRAW_ASSERT

   n->cached = owner != nullptr;
   if (owner) {
      n->base_x = region->x0;
      n->base_y = region->y0;
      n->width  = region->x1 - region->x0;
      n->height = region->y1 - region->y0;
   }

   assert(gdraw->frametex_width >= gdraw->tw && gdraw->frametex_height >= gdraw->th); // @GDRAW_ASSERT
   int k = n->color_buffer - gdraw->rendertargets.handle;
   S32 x, y, w, h;

   if (region) {
      S32 ox, oy, pad = 2; // 2 pixels of border on all sides
      // 1 pixel turns out to be not quite enough with the interpolator precision we get.

      if (gdraw->in_blur)
         ox = oy = 0;
      else
         ox = gdraw->tx0p, oy = gdraw->ty0p;

      // clamp region to tile
      S32 xt0 = RR_MAX(region->x0 - ox, 0);
      S32 yt0 = RR_MAX(region->y0 - oy, 0);
      S32 xt1 = RR_MIN(region->x1 - ox, gdraw->tpw);
      S32 yt1 = RR_MIN(region->y1 - oy, gdraw->tph);

      // but the padding needs to clamp to render target bounds
      x = RR_MAX(xt0 - pad, 0);
      y = RR_MAX(yt0 - pad, 0);
      w = RR_MIN(xt1 + pad, gdraw->frametex_width) - x;
      h = RR_MIN(yt1 + pad, gdraw->frametex_height) - y;

      if (w <= 0 || h <= 0) { // region doesn't intersect with current tile
         gdraw_FreeTexture((GDrawTexture *) n->color_buffer, 0, stats);
         // note: don't send a warning since this will happen during regular tiled rendering
         return false;
      }

      gdraw->rt_valid[k].x0 = xt0;
      gdraw->rt_valid[k].y0 = yt0;
      gdraw->rt_valid[k].x1 = xt1;
      gdraw->rt_valid[k].y1 = yt1;
   } else {
      x = 0;
      y = 0;
      w = gdraw->frametex_width;
      h = gdraw->frametex_height;

      gdraw->rt_valid[k].x0 = 0;
      gdraw->rt_valid[k].y0 = 0;
      gdraw->rt_valid[k].x1 = w;
      gdraw->rt_valid[k].y1 = h;
   }

   stats->nonzero_flags |= GDRAW_STATS_rendtarg;
   stats->rendertarget_changes++;

   ++gdraw->cur;
   gdraw->rt_colorbuffer.setBaseAddress(n->color_buffer->handle.tex.gnm_ptr);
   set_render_target();
   manual_clear_color(x, y, w, h, stats);
   set_viewport();
   set_projection();

   return true;
}

static GDrawTexture *RADLINK gdraw_TextureDrawBufferEnd(GDrawStats *stats)
{
   GDrawFramebufferState *n =   gdraw->cur;
   GDrawFramebufferState *m = --gdraw->cur;
   if (gdraw->tw == 0 || gdraw->th == 0) return 0;

   if (n >= &gdraw->frame[MAX_RENDER_STACK_DEPTH])
      return 0; // already returned a warning in Start...()

   assert(m >= gdraw->frame);  // bug in Iggy -- unbalanced

   if (m != gdraw->frame) {
      assert(m->color_buffer != nullptr); // @GDRAW_ASSERT
   }
   assert(n->color_buffer != nullptr); // @GDRAW_ASSERT

   // sync on draw completion for this render target
   rtt_sync(n->color_buffer->handle.tex.gnm_ptr, gdraw->rt_colorbuffer_sa.m_size >> 8);
   n->color_buffer->fence = get_next_fence();

   // switch back to old rendertarget
   set_render_target();
   set_viewport();
   set_projection();

   stats->nonzero_flags |= GDRAW_STATS_rendtarg;
   stats->rendertarget_changes++;

   return (GDrawTexture *) n->color_buffer;
}

////////////////////////////////////////////////////////////////////////
//
//   Clear stencil/depth buffers
//

static void RADLINK gdraw_ClearStencilBits(U32 bits)
{
   GDrawStats stats = {};
   clear_whole_zs(false, true, &stats);
}

static void RADLINK gdraw_ClearID(void)
{
   GDrawStats stats = {};
   clear_whole_zs(true, false, &stats);
}

////////////////////////////////////////////////////////////////////////
//
//   Set all the render state from GDrawRenderState
//

static RADINLINE void set_texture(U32 texunit, GDrawTexture *tex)
{
   assert(texunit < MAX_SAMPLERS);
   assert(tex != nullptr);

   if (gdraw->active_tex[texunit] != tex) {
      gdraw->active_tex[texunit] = tex;
      GDrawHandle *h = (GDrawHandle *) tex;
      set_gnm_texture(texunit, h->handle.tex.gnm, GDRAW_WRAP_clamp, 0);
   }
}

static RADINLINE void set_pixel_shader(ShaderCode *ps)
{
   if (gdraw->cur_ps != ps) {
      gdraw->cur_ps = ps;
      gdraw->gfxc->setPsShader(ps->ps);
   }
}

// converts a depth id into a Z value
static inline F32 depth_from_id(S32 id)
{
   return (1.0f - 2.0f / MAX_DEPTH_VALUE) - id * (2.0f / MAX_DEPTH_VALUE); // = 1 - 2 * (id + 1) / MAX_DEPTH_VALUE
}

static void set_renderstate_full(const GDrawRenderState * RADRESTRICT r, GDrawStats *stats)
{
   GDraw * RADRESTRICT gd = gdraw;
   Gnmx::GfxContext * RADRESTRICT gfxc = gd->gfxc;
   F32 depth = depth_from_id(r->id);

   // set vertex shader constants
   VertexVars * RADRESTRICT vvars = embed_const_buffer<VertexVars>(Gnm::kShaderStageVs, 0);
   if (!r->use_world_space)
      gdraw_ObjectSpace(vvars->world[0], r->o2w, depth, 0.0f);
   else
      gdraw_WorldSpace(vvars->world[0], gdraw->world_to_pixel, depth, 0.0f);

   __m128 edge = _mm_loadu_ps(r->edge_matrix);
   __m128 s0_texgen = _mm_setzero_ps();
   __m128 t0_texgen = _mm_setzero_ps();
   __m128 viewproj = _mm_load_ps(gd->projection);
   if (r->texgen0_enabled) {
      s0_texgen = _mm_loadu_ps(&r->s0_texgen[0]);
      t0_texgen = _mm_loadu_ps(&r->t0_texgen[0]);
   }

   _mm_storeu_ps(&vvars->x_off[0], edge);
   _mm_storeu_ps(&vvars->texgen_s[0], s0_texgen);
   _mm_storeu_ps(&vvars->texgen_t[0], t0_texgen);
   _mm_storeu_ps(&vvars->viewproj[0], viewproj);

   // set the blend mode
   int tex0mode = r->tex0_mode;
   int blend_mode = r->blend_mode;
   if (blend_mode != gd->blend_mode) {
      gd->blend_mode = blend_mode;
      gfxc->setBlendControl(0, gd->blend_control[blend_mode]);
   }

   // color channel write mask: stencil set mode doesn't write color
   if (r->stencil_set != gd->writes_masked) {
      gd->writes_masked = r->stencil_set;
      gfxc->setRenderTargetMask(r->stencil_set ? 0 : 0xf);
   }

   // set the pixel shader
   if (blend_mode != GDRAW_BLEND_special) {
      assert(tex0mode >= 0 && tex0mode < sizeof(gd->main_ps) / sizeof(*gd->main_ps));
      ShaderCode *ps = gd->basic_ps[tex0mode];

      if (r->cxf_add) {
         ps++;
         if (r->cxf_add[3]) ps++;
      }

      set_pixel_shader(ps);
   } else // special blends have a special pixel shader.
      set_pixel_shader(&gd->exceptional_blend[r->special_blend]);

   // set textures
   if (tex0mode != GDRAW_TEXTURE_none) {
      if (!r->tex[0]) // this can happen if some allocs fail. just abort in that case.
         return;

      if (gd->active_tex[0] != r->tex[0]) {
         gd->active_tex[0] = r->tex[0];
         set_gnm_texture(0, ((GDrawHandle *) r->tex[0])->handle.tex.gnm, r->wrap0, r->nearest0);
      }
   }

   // pixel shader constants
   PixelCommonVars * RADRESTRICT pvars = embed_const_buffer<PixelCommonVars>(Gnm::kShaderStagePs, 0);
   __m128 col_mul = _mm_loadu_ps(r->color);
   __m128 col_add = _mm_setzero_ps();
   __m128 focal = _mm_loadu_ps(r->focal_point);
   if (r->cxf_add) {
      const float scalef = 1.0f / 255.0f;
      col_add = _mm_setr_ps((F32) r->cxf_add[0] * scalef, (F32) r->cxf_add[1] * scalef, (F32) r->cxf_add[2] * scalef, (F32) r->cxf_add[3] * scalef);
   }

   _mm_storeu_ps(&pvars->color_mul[0], col_mul);
   _mm_storeu_ps(&pvars->color_add[0], col_add);
   _mm_storeu_ps(&pvars->focal[0], focal);

   // set scissor
   if (r->scissor) {
      S32 xs,ys;
      if (gd->cur == gd->frame) {
         xs = gd->tx0 - gd->vx;
         ys = gd->ty0 - gd->vy;
      } else {
         xs = gd->tx0p;
         ys = gd->ty0p;
      }

      // clip against viewport
      S32 x0 = RR_MAX(r->scissor_rect.x0 - xs, gd->cview.x0);
      S32 y0 = RR_MAX(r->scissor_rect.y0 - ys, gd->cview.y0);
      S32 x1 = RR_MIN(r->scissor_rect.x1 - xs, gd->cview.x1);
      S32 y1 = RR_MIN(r->scissor_rect.y1 - ys, gd->cview.y1);

      if (x1 <= x0 || y1 <= y0) {
         // dummy scissor rect in case our actual scissor is empty
         x0 = x1 = gd->cview.x0;
         y0 = y1 = gd->cview.y0;
      }

      gfxc->setScreenScissor(x0, y0, x1, y1);
      gd->scissor_state = 1;  
   } else if (r->scissor != gd->scissor_state)
      disable_scissor(0);
   
   // z/stencil mode changed?
   U32 z_stencil_key = r->set_id | (r->test_id << 1) | (r->stencil_test << 16) | (r->stencil_set << 24);
   
   if (z_stencil_key != gd->z_stencil_key) {
      gd->z_stencil_key = z_stencil_key;
      if (r->stencil_test | r->stencil_set) {
         Gnm::StencilControl ctl;
         ctl.m_testVal = 255;
         ctl.m_mask = r->stencil_test;
         ctl.m_writeMask = r->stencil_set;
         ctl.m_opVal = 255;

         gfxc->setDepthStencilControl(gd->depth_stencil_control[r->set_id][r->test_id][1]);
         gfxc->setStencil(ctl);
      } else
         gfxc->setDepthStencilControl(gd->depth_stencil_control[r->set_id][r->test_id][0]);
   }
}

static RADINLINE void set_renderstate(const GDrawRenderState * RADRESTRICT r, GDrawStats *stats)
{
   if (!r->identical_state)
      set_renderstate_full(r, stats);
}

////////////////////////////////////////////////////////////////////////
//
//   Draw triangles with a given renderstate
//

static RADINLINE const GDraw::VFormatDesc *get_vertex_fmt(S32 vfmt)
{
   assert(vfmt >= 0 && vfmt < GDRAW_vformat__count);
   return &gdraw->vfmt[vfmt];
}

static void set_vertex_buffer(const GDraw::VFormatDesc *fmtdesc, void *ptr, U32 num_verts)
{
   Gnm::Buffer bufs[MAX_ATTRS];

   for (U32 i=0; i < fmtdesc->num_attribs; i++)
      bufs[i].initAsVertexBuffer((U8 *)ptr + fmtdesc->vb_offs[i], fmtdesc->formats[i], fmtdesc->stride, num_verts);
   gdraw->gfxc->setBuffers(Gnm::kShaderStageVs, 0, fmtdesc->num_attribs, bufs);
}

static RADINLINE void fence_resources(void *r1, void *r2=nullptr, void *r3=nullptr, void *r4=nullptr)
{
   GDrawFence fence = get_next_fence();
   if (r1) ((GDrawHandle *) r1)->fence = fence;
   if (r2) ((GDrawHandle *) r2)->fence = fence;
   if (r3) ((GDrawHandle *) r3)->fence = fence;
   if (r4) ((GDrawHandle *) r4)->fence = fence;
}

static void RADLINK gdraw_DrawIndexedTriangles(GDrawRenderState *r, GDrawPrimitive *p, GDrawVertexBuffer *buf, GDrawStats *stats)
{
   Gnmx::GfxContext * RADRESTRICT gfxc = gdraw->gfxc;
   GDrawHandle *vb = (GDrawHandle *) buf;
   const GDraw::VFormatDesc * RADRESTRICT vfmt = get_vertex_fmt(p->vertex_format);

   set_renderstate(r, stats);

   if (vb) {
      set_vertex_buffer(vfmt, (U8 *) vb->handle.vbuf.verts + (UINTa) p->vertices, p->num_vertices);
      gfxc->setPrimitiveType(Gnm::kPrimitiveTypeTriList);
      gfxc->drawIndex(p->num_indices, (U8 *)vb->handle.vbuf.inds + (UINTa) p->indices);
   } else if (p->indices) {
      U32 vbytes = p->num_vertices * vfmt->stride;
      U32 ibytes = p->num_indices * 2;

      gfxc->setPrimitiveType(Gnm::kPrimitiveTypeTriList);
      U8 *buf = (U8 *) alloc_staging(vbytes + ibytes, Gnm::kAlignmentOfBufferInBytes);
      if (!buf)
         return;

      memcpy(buf, p->vertices, vbytes);
      memcpy(buf + vbytes, p->indices, ibytes);
      set_vertex_buffer(vfmt, buf, p->num_vertices);
      gfxc->drawIndex(p->num_indices, buf + vbytes);
   } else { // dynamic quads
      assert(p->num_vertices % 4 == 0);
      U32 stride = vfmt->stride;
      U32 num_bytes = (U32)p->num_vertices * stride;

      gfxc->setPrimitiveType(Gnm::kPrimitiveTypeQuadList);
      U8 *buf = (U8 *) alloc_staging(num_bytes, Gnm::kAlignmentOfBufferInBytes);
      if (!buf)
         return;

      memcpy(buf, p->vertices, num_bytes);
      set_vertex_buffer(vfmt, buf, p->num_vertices);
      gfxc->drawIndexAuto(p->num_vertices);
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

static void do_screen_quad(gswf_recti *s, const F32 *tc, GDrawStats *stats)
{
   VertexVars *vvars = embed_const_buffer<VertexVars>(Gnm::kShaderStageVs, 0);
   __m128 world0 = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
   __m128 world1 = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
   __m128 zero = _mm_setzero_ps();
   __m128 viewproj = _mm_load_ps(gdraw->projection);
   _mm_storeu_ps(vvars->world[0], world0);
   _mm_storeu_ps(vvars->world[1], world1);
   _mm_storeu_ps(vvars->x_off, zero);
   _mm_storeu_ps(vvars->texgen_s, zero);
   _mm_storeu_ps(vvars->texgen_t, zero);
   _mm_storeu_ps(vvars->viewproj, viewproj);

   gswf_vertex_xyst * RADRESTRICT v = (gswf_vertex_xyst *) alloc_staging(3 * sizeof(gswf_vertex_xyst), Gnm::kAlignmentOfBufferInBytes);
   if (!v)
      return;
   
   F32 px0 = (F32) s->x0, py0 = (F32) s->y0, px1 = (F32) s->x1, py1 = (F32) s->y1;
   v[0].x = px0; v[0].y = py0; v[0].s = tc[0]; v[0].t = tc[1];
   v[1].x = px1; v[1].y = py0; v[1].s = tc[2]; v[1].t = tc[1];
   v[2].x = px0; v[2].y = py1; v[2].s = tc[0]; v[2].t = tc[3];

   set_vertex_buffer(&gdraw->vfmt[GDRAW_vformat_v2tc2], v, 3);
   gdraw->gfxc->setPrimitiveType(Gnm::kPrimitiveTypeRectList);
   gdraw->gfxc->drawIndexAuto(3);
}

static void gdraw_DriverBlurPass(GDrawRenderState *r, int taps,  float *data, gswf_recti *s, float *tc, float /*height_max*/, float *clamp, GDrawStats *gstats)
{
   set_texture(0, r->tex[0]);
   set_pixel_shader(&gdraw->blur_ps[taps]);
   auto para = embed_const_buffer<PixelParaBlur>(Gnm::kShaderStagePs, 1);
   memcpy(para->clamp, clamp, 4 * sizeof(float));
   memcpy(para->tap, data, taps * 4 * sizeof(float));

   do_screen_quad(s, tc, gstats);
   fence_resources(r->tex[0]);
}

static void gdraw_Colormatrix(GDrawRenderState *r, gswf_recti *s, float *tc, GDrawStats *stats)
{
   if (!gdraw_TextureDrawBufferBegin(s, GDRAW_TEXTURE_FORMAT_rgba32, GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_color | GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_alpha, 0, stats))
      return;

   set_texture(0, r->tex[0]);
   set_pixel_shader(&gdraw->colormatrix);
   auto para = embed_const_buffer<PixelParaColorMatrix>(Gnm::kShaderStagePs, 1);
   memcpy(para->data, r->shader_data, 5 * 4 * sizeof(float));

   do_screen_quad(s, tc, stats);
   fence_resources(r->tex[0]);
   r->tex[0] = gdraw_TextureDrawBufferEnd(stats);
}

static gswf_recti *get_valid_rect(GDrawTexture *tex)
{
   GDrawHandle *h = (GDrawHandle *) tex;
   S32 n = (S32) (h - gdraw->rendertargets.handle);
   assert(n >= 0 && n <= MAX_RENDER_STACK_DEPTH+1);
   return &gdraw->rt_valid[n];
}

static void set_pixel_constant(F32 *constant, F32 x, F32 y, F32 z, F32 w)
{
   constant[0] = x;
   constant[1] = y;
   constant[2] = z;
   constant[3] = w;
}

static void set_clamp_constant(F32 *constant, GDrawTexture *tex)
{
   gswf_recti *s = get_valid_rect(tex);
   // when we make the valid data, we make sure there is an extra empty pixel at the border
   set_pixel_constant(constant,
      (s->x0-0.5f) / gdraw->frametex_width,
      (s->y0-0.5f) / gdraw->frametex_height,
      (s->x1+0.5f) / gdraw->frametex_width,
      (s->y1+0.5f) / gdraw->frametex_height);
}

static void gdraw_Filter(GDrawRenderState *r, gswf_recti *s, float *tc, int isbevel, GDrawStats *stats)
{
   if (!gdraw_TextureDrawBufferBegin(s, GDRAW_TEXTURE_FORMAT_rgba32, GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_color | GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_alpha, nullptr, stats))
      return;

   set_texture(0, r->tex[0]);
   set_texture(1, r->tex[1]);
   if (r->tex[2]) set_texture(2, r->tex[2]);
   set_pixel_shader(&gdraw->filter_ps[isbevel][r->filter_mode]);

   auto para = embed_const_buffer<PixelParaFilter>(Gnm::kShaderStagePs, 1);
   set_clamp_constant(para->clamp0, r->tex[0]);
   set_clamp_constant(para->clamp1, r->tex[1]);
   set_pixel_constant(para->color, r->shader_data[0], r->shader_data[1], r->shader_data[2], r->shader_data[3]);
   set_pixel_constant(para->color2, r->shader_data[8], r->shader_data[9], r->shader_data[10], r->shader_data[11]);
   set_pixel_constant(para->tc_off, -r->shader_data[4] / (F32)gdraw->frametex_width, -r->shader_data[5] / (F32)gdraw->frametex_height, r->shader_data[6], 0);

   do_screen_quad(s, tc, stats);
   fence_resources(r->tex[0], r->tex[1], r->tex[2]);
   r->tex[0] = gdraw_TextureDrawBufferEnd(stats);
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
   tc[0] = (s.x0 - gdraw->tx0p) / (F32) gdraw->frametex_width;    
   tc[1] = (s.y0 - gdraw->ty0p) / (F32) gdraw->frametex_height; 
   tc[2] = (s.x1 - gdraw->tx0p) / (F32) gdraw->frametex_width;    
   tc[3] = (s.y1 - gdraw->ty0p) / (F32) gdraw->frametex_height;

   clear_renderstate();

   if (r->blend_mode == GDRAW_BLEND_filter) {
      switch (r->filter) {
         case GDRAW_FILTER_blur: {
            GDrawBlurInfo b;
            gswf_recti bounds = *get_valid_rect(r->tex[0]);
            gdraw_ShiftRect(&s, &s, -gdraw->tx0p, -gdraw->ty0p); // blur uses physical rendertarget coordinates

            b.BlurPass = gdraw_DriverBlurPass;
            b.w = gdraw->tpw;
            b.h = gdraw->tph;
            b.frametex_width = gdraw->frametex_width;
            b.frametex_height = gdraw->frametex_height;

            // blur needs to draw with multiple passes, so set up special state
            gdraw->in_blur = true;
            set_viewport();
            set_projection();

            // do the blur
            gdraw_Blur(&gdraw_funcs, &b,r, &s, &bounds, stats);

            // restore the normal state
            gdraw->in_blur = false;
            set_viewport();
            set_projection();
            break;
         }

         case GDRAW_FILTER_colormatrix:
            gdraw_Colormatrix(r, &s, tc, stats);
            break;

         case GDRAW_FILTER_dropshadow:
            gdraw_Filter(r, &s, tc, 0, stats);
            break;

         case GDRAW_FILTER_bevel:
            gdraw_Filter(r, &s, tc, 1, stats);
            break;

         default:
            assert(0);
      }
   } else {
      // for crazy blend modes, we need to read back from the framebuffer
      // and do the blending in the pixel shader. because we render as
      // a RectList, no quad is ever shaded twice, so we can do this safely
      // while texturing from the render target that we're writing to.
      if (r->blend_mode == GDRAW_BLEND_special) {
         // before we texture from this RT, we need to eliminate fast clears.
         eliminate_fast_clear();

         // input texture. slightly different logic between the main render
         // target and our scratch render targets, because they might have
         // different dimensions.
         Gnm::Texture rt_tex;

         if (gdraw->cur == gdraw->frame) {
            rt_tex.initFromRenderTarget(&gdraw->main_colorbuffer, false);
            rtt_sync(rt_tex.getBaseAddress(), gdraw->main_colorbuffer.getSizeInBytes() >> 8);
         } else {
            rt_tex = *gdraw->cur->color_buffer->handle.tex.gnm;
            rtt_sync(rt_tex.getBaseAddress(), gdraw->rt_colorbuffer_sa.m_size >> 8);
         }

         set_gnm_texture(1, &rt_tex, GDRAW_WRAP_clamp, 0);
      }

      set_renderstate(r, stats);
      do_screen_quad(&s, tc, stats);
      fence_resources(r->tex[0], r->tex[1]);
   }
}

////////////////////////////////////////////////////////////////////////
//
//   Shaders and state initialization
//

#include "gdraw_orbis_shaders.inl"

static void init_shader(ShaderCode *shader, const ShaderCode *src)
{
   *shader = *src;
   if (!shader->blob)
      return;

   Gnmx::ShaderFileHeader *shdr = (Gnmx::ShaderFileHeader *) shader->blob;
   shader->desc = shdr + 1;

   // grab gpu code and copy to arena
   const void *shader_code = (const U8 *) shader->desc + shdr->m_shaderHeaderSizeInDW * 4;
   void *gpu_ptr = gdraw_arena_alloc(&gdraw->vidshared_arena, shader->common->m_shaderSize, Gnm::kAlignmentOfShaderInBytes);
   memcpy(gpu_ptr, shader_code, shader->common->m_shaderSize);

   // patch the shader
   switch (shdr->m_type) {
   case Gnmx::kVertexShader:  shader->vs->m_vsStageRegisters.m_spiShaderPgmHiVs = ~0u; shader->vs->patchShaderGpuAddress(gpu_ptr); break;
   case Gnmx::kPixelShader:   shader->ps->m_psStageRegisters.m_spiShaderPgmHiPs = ~0u; shader->ps->patchShaderGpuAddress(gpu_ptr); break;
   case Gnmx::kComputeShader: shader->cs->m_csStageRegisters.m_computePgmHi = ~0u;     shader->cs->patchShaderGpuAddress(gpu_ptr); break;
   default:                   assert(0);
   }
}

static void create_all_shaders_and_state()
{
   // sampler state
   static const Gnm::WrapMode addrmodes[ASSERT_COUNT(GDRAW_WRAP__count, 4)] = {
      Gnm::kWrapModeClampLastTexel,       // GDRAW_WRAP_clamp
      Gnm::kWrapModeWrap,                 // GDRAW_WRAP_repeat
      Gnm::kWrapModeMirror,               // GDRAW_WRAP_mirror
      Gnm::kWrapModeClampBorder,          // GDRAW_WRAP_clamp_to_border
   };

   for (int nearest=0; nearest < 2; nearest++)
      for (int addr=0; addr < GDRAW_WRAP__count; addr++) {
         Gnm::Sampler *smp = &gdraw->sampler_state[nearest][addr];
         smp->init();
         smp->setWrapMode(addrmodes[addr], addrmodes[addr], addrmodes[addr]);
         smp->setBorderColor(Gnm::kBorderColorTransBlack);
         smp->setXyFilterMode(nearest ? Gnm::kFilterModePoint : Gnm::kFilterModeBilinear, Gnm::kFilterModeBilinear);
         smp->setMipFilterMode(Gnm::kMipFilterModeLinear);
      }

   // depth/stencil state
   for (int set_id=0; set_id < 2; set_id++)
      for (int test_id=0; test_id < 2; test_id++)
         for (int stencil_enable=0; stencil_enable < 2; stencil_enable++) {
            Gnm::DepthStencilControl *ctl = &gdraw->depth_stencil_control[set_id][test_id][stencil_enable];
            ctl->init();
            ctl->setDepthEnable(set_id || test_id);
            ctl->setDepthControl(set_id ? Gnm::kDepthControlZWriteEnable : Gnm::kDepthControlZWriteDisable, test_id ? Gnm::kCompareFuncLess : Gnm::kCompareFuncAlways);
            ctl->setStencilEnable(stencil_enable != 0);
            ctl->setStencilFunction(Gnm::kCompareFuncEqual);
         }

   // blend state
   static const struct {
      bool enable;
      Gnm::BlendMultiplier src;
      Gnm::BlendMultiplier dst;
   } blend_states[ASSERT_COUNT(GDRAW_BLEND__count, 6)] = {
      { false, Gnm::kBlendMultiplierOne,       Gnm::kBlendMultiplierZero },             // GDRAW_BLEND_none
      { true,  Gnm::kBlendMultiplierOne,       Gnm::kBlendMultiplierOneMinusSrcAlpha }, // GDRAW_BLEND_alpha
      { true,  Gnm::kBlendMultiplierDestColor, Gnm::kBlendMultiplierOneMinusSrcAlpha }, // GDRAW_BLEND_multiply
      { true,  Gnm::kBlendMultiplierOne,       Gnm::kBlendMultiplierOne },              // GDRAW_BLEND_add

      { false, Gnm::kBlendMultiplierOne,       Gnm::kBlendMultiplierZero },             // GDRAW_BLEND_filter
      { false, Gnm::kBlendMultiplierOne,       Gnm::kBlendMultiplierZero },             // GDRAW_BLEND_special
   };
   for (int mode = 0; mode < GDRAW_BLEND__count; mode++) {
      Gnm::BlendControl *ctl = &gdraw->blend_control[mode];
      ctl->init();
      ctl->setBlendEnable(blend_states[mode].enable);
      ctl->setSeparateAlphaEnable(false);
      ctl->setColorEquation(blend_states[mode].src, Gnm::kBlendFuncAdd, blend_states[mode].dst);
   }

   // vertex shader
   init_shader(&gdraw->vs, vshader_vsps4_arr);

   // pixel shaders
   for (int i=0; i < GDRAW_TEXTURE__count*3; i++)     init_shader(&gdraw->main_ps[0][i], pshader_basic_arr + i);
   for (int i=0; i < GDRAW_BLENDSPECIAL__count; i++)  init_shader(&gdraw->exceptional_blend[i], pshader_exceptional_blend_arr + i);
   for (int i=0; i < 32; i++)                         init_shader(&gdraw->filter_ps[0][i], pshader_filter_arr + i);
   for (int i=0; i <= MAX_TAPS; i++)                  init_shader(&gdraw->blur_ps[i], pshader_blur_arr + i);
   init_shader(&gdraw->colormatrix, pshader_color_matrix_arr);
   init_shader(&gdraw->clear_ps, pshader_manual_clear_arr);

   for (int i=0; i < GDRAW_TEXTURE__count; i++)
      gdraw->basic_ps[i] = &gdraw->main_ps[i][0];

   // compute shaders
   init_shader(&gdraw->texupload_cs, cshader_tex_upload_arr);
   init_shader(&gdraw->memset_cs, cshader_memset_arr);
   init_shader(&gdraw->defragment_cs, cshader_defragment_arr);
   init_shader(&gdraw->mipgen_cs, cshader_mipgen_arr);

   // vertex formats
   struct VAttrDesc
   {
      U32 offset;
      Gnm::DataFormat fmt;
   };

   static const struct VFmtDesc {
      U32 stride;
      U32 num_attribs;
      VAttrDesc attribs[MAX_ATTRS];
   } vformats[ASSERT_COUNT(GDRAW_vformat__basic_count, 3)] = {
      // GDRAW_vformat_v2
      { 8, 2,  {
         { 0, {{{ Gnm::kSurfaceFormat32_32,         Gnm::kBufferChannelTypeFloat,   Gnm::kBufferChannelX,  Gnm::kBufferChannelY,   Gnm::kBufferChannelConstant0,  Gnm::kBufferChannelConstant1 }}} },
         { 4, {{{ Gnm::kSurfaceFormat8_8_8_8,       Gnm::kBufferChannelTypeUNorm,   Gnm::kBufferChannelConstant0, Gnm::kBufferChannelConstant0,  Gnm::kBufferChannelConstant1,  Gnm::kBufferChannelConstant1 }}} },
      } },
      // GDRAW_vformat_v2aa
      { 16, 2,  {
         { 0, {{{ Gnm::kSurfaceFormat32_32,         Gnm::kBufferChannelTypeFloat,   Gnm::kBufferChannelX,  Gnm::kBufferChannelY,   Gnm::kBufferChannelConstant0,  Gnm::kBufferChannelConstant1 }}} },
         { 8, {{{ Gnm::kSurfaceFormat16_16_16_16,   Gnm::kBufferChannelTypeSScaled, Gnm::kBufferChannelX,  Gnm::kBufferChannelY,   Gnm::kBufferChannelZ,   Gnm::kBufferChannelConstant0 }}} },
      } },
      // GDRAW_vformat_v2tc2
      { 16, 2,  {
         { 0, {{{ Gnm::kSurfaceFormat32_32,         Gnm::kBufferChannelTypeFloat,   Gnm::kBufferChannelX,  Gnm::kBufferChannelY,   Gnm::kBufferChannelConstant0,  Gnm::kBufferChannelConstant1 }}} },
         { 8, {{{ Gnm::kSurfaceFormat32_32,         Gnm::kBufferChannelTypeFloat,   Gnm::kBufferChannelX,  Gnm::kBufferChannelY,   Gnm::kBufferChannelConstant0,  Gnm::kBufferChannelConstant1 }}} },
      } },
   };

   for (int i=0; i < GDRAW_vformat__basic_count; i++) {
      gdraw->vfmt[i].stride = vformats[i].stride;
      gdraw->vfmt[i].num_attribs = vformats[i].num_attribs;
      for (U32 j=0; j < vformats[i].num_attribs; j++) {
         const VAttrDesc *desc = &vformats[i].attribs[j];
         gdraw->vfmt[i].formats[j] = desc->fmt;
         gdraw->vfmt[i].vb_offs[j] = desc->offset;
      }
   }

   // zero "pixel common" constant buffer
   PixelCommonVars *pvars = (PixelCommonVars *) gdraw_arena_alloc(&gdraw->vidshared_arena, sizeof(PixelCommonVars), Gnm::kAlignmentOfBufferInBytes);
   memset(pvars, 0, sizeof(*pvars));
   gdraw->pixel_common_zero_cbuf.initAsConstantBuffer(pvars, sizeof(*pvars));
}

typedef struct
{
   S32 num_handles;
   S32 num_bytes;
   void *ptr;
} GDrawResourceLimit;

// Resource limits used by GDraw. Change these using SetResouceLimits!
static GDrawResourceLimit gdraw_limits[GDRAW_ORBIS_RESOURCE__count];

static GDrawHandleCache *make_handle_cache(gdraw_orbis_resourcetype type, U32 align)
{
   S32 num_handles = gdraw_limits[type].num_handles;
   S32 num_bytes = gdraw_limits[type].num_bytes;
   U32 cache_size = sizeof(GDrawHandleCache) + (num_handles - 1) * sizeof(GDrawHandle);
   bool is_vertex = (type == GDRAW_ORBIS_RESOURCE_vertexbuffer);
   U32 header_size = num_handles * (is_vertex ? 0 : sizeof(Gnm::Texture));

   GDrawHandleCache *cache = (GDrawHandleCache *) IggyGDrawMalloc(cache_size + header_size);
   if (cache) {
      gdraw_HandleCacheInit(cache, num_handles, num_bytes);
      cache->is_vertex = is_vertex;

      // set up resource headers
      void *header_start = (U8 *) cache + cache_size;
      if (!is_vertex) {
         Gnm::Texture *headers = (Gnm::Texture *) header_start;
         for (S32 i=0; i < num_handles; i++)
            cache->handle[i].handle.tex.gnm = &headers[i];
      }

      // set up allocator
      cache->alloc = gfxalloc_create(gdraw_limits[type].ptr, num_bytes, align, num_handles);
      if (!cache->alloc) {
         IggyGDrawFree(cache);
         cache = nullptr;
      }
   }

   return cache;
}

static void free_handle_cache(GDrawHandleCache *c)
{
   if (c) {
      if (c->alloc) IggyGDrawFree(c->alloc);
      IggyGDrawFree(c);
   }
}


int gdraw_orbis_SetResourceMemory(gdraw_orbis_resourcetype type, S32 num_handles, void *ptr, S32 num_bytes)
{
   GDrawStats stats={0};

   assert(type >= GDRAW_ORBIS_RESOURCE_rendertarget && type < GDRAW_ORBIS_RESOURCE__count);
   assert(num_handles >= 0);
   assert(num_bytes >= 0);

   if (!num_handles) num_handles = 1;

   switch (type) {
   case GDRAW_ORBIS_RESOURCE_texture:
      make_pool_aligned(&ptr, &num_bytes, GDRAW_ORBIS_TEXTURE_ALIGNMENT);
      break;

   case GDRAW_ORBIS_RESOURCE_vertexbuffer:
      make_pool_aligned(&ptr, &num_bytes, Gnm::kAlignmentOfBufferInBytes);
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
   assert(!is_fence_pending(gdraw->tile_end_fence)); // you may not call this while GPU is still busy with Iggy command buffers!

   if (gdraw->texturecache) gdraw_res_reap(gdraw->texturecache, &stats);
   if (gdraw->vbufcache) gdraw_res_reap(gdraw->vbufcache, &stats);
   // in theory we can now check that the given cache is really empty at this point

   // resize the appropriate pool
   switch (type) {
      case GDRAW_ORBIS_RESOURCE_rendertarget:
         gdraw_HandleCacheInit(&gdraw->rendertargets, MAX_RENDER_STACK_DEPTH + 1, num_bytes);
         for (int i=0; i < MAX_RENDER_STACK_DEPTH + 1; i++)
            gdraw->rendertargets.handle[i].handle.tex.gnm = &gdraw->rendertarget_textures[i];
         gdraw_arena_init(&gdraw->rt_arena, ptr, num_bytes);
         return 1;

      case GDRAW_ORBIS_RESOURCE_texture:
         free_handle_cache(gdraw->texturecache);
         gdraw->texturecache = make_handle_cache(GDRAW_ORBIS_RESOURCE_texture, GDRAW_ORBIS_TEXTURE_ALIGNMENT);
         return gdraw->texturecache != nullptr;

      case GDRAW_ORBIS_RESOURCE_vertexbuffer:
         free_handle_cache(gdraw->vbufcache);
         gdraw->vbufcache = make_handle_cache(GDRAW_ORBIS_RESOURCE_vertexbuffer, GDRAW_ORBIS_VERTEXBUFFER_ALIGNMENT);
         return gdraw->vbufcache != nullptr;

      default:
         return 0;
   }
}

void gdraw_orbis_ResetAllResourceMemory()
{
   gdraw_orbis_SetResourceMemory(GDRAW_ORBIS_RESOURCE_rendertarget, 0, nullptr, 0);
   gdraw_orbis_SetResourceMemory(GDRAW_ORBIS_RESOURCE_texture, 0, nullptr, 0);
   gdraw_orbis_SetResourceMemory(GDRAW_ORBIS_RESOURCE_vertexbuffer, 0, nullptr, 0);
}

GDrawFunctions *gdraw_orbis_CreateContext(S32 w, S32 h, void *context_shared_mem)
{
   U32 cpram_shadow_size = Gnmx::ConstantUpdateEngine::computeCpRamShadowSize();

   gdraw = (GDraw *) IggyGDrawMalloc(sizeof(*gdraw) + cpram_shadow_size);
   if (!gdraw) return nullptr;

   memset(gdraw, 0, sizeof(*gdraw));

   // context shared memory
   gdraw_arena_init(&gdraw->vidshared_arena, context_shared_mem, GDRAW_ORBIS_CONTEXT_MEM_SIZE);

   // labels
   gdraw->label_ptr = (volatile U64 *) gdraw_arena_alloc(&gdraw->vidshared_arena, sizeof(U64), sizeof(U64));
   *gdraw->label_ptr = 0;
   gdraw->next_fence_index = 1;
   gdraw->tile_end_fence.value = 0;

   // set up memory for all resource types
   for (int i=0; i < GDRAW_ORBIS_RESOURCE__count; i++)
      gdraw_orbis_SetResourceMemory((gdraw_orbis_resourcetype) i, gdraw_limits[i].num_handles, gdraw_limits[i].ptr, gdraw_limits[i].num_bytes);

   // initialize render target texture desc
   gdraw->frametex_width = w;
   gdraw->frametex_height = h;
   Gnm::DataFormat rtFormat = Gnm::kDataFormatR8G8B8A8Unorm;
   Gnm::TileMode tileMode;
   GpuAddress::computeSurfaceTileMode(&tileMode, GpuAddress::kSurfaceTypeRwTextureFlat, rtFormat, 1);
   gdraw->rt_colorbuffer_sa = gdraw->rt_colorbuffer.init(gdraw->frametex_width, gdraw->frametex_height, 1, rtFormat, tileMode, Gnm::kNumSamples1, Gnm::kNumFragments1, nullptr, nullptr);
   gdraw->rt_colorbuffer.setCmaskFastClearEnable(false);

   // shaders and state
   create_all_shaders_and_state();

   // API
   gdraw_funcs.SetViewSizeAndWorldScale = gdraw_SetViewSizeAndWorldScale;
   gdraw_funcs.GetInfo       = gdraw_GetInfo;

   gdraw_funcs.DescribeTexture = gdraw_DescribeTexture;
   gdraw_funcs.DescribeVertexBuffer = gdraw_DescribeVertexBuffer;

   gdraw_funcs.RenderingBegin  = gdraw_RenderingBegin;
   gdraw_funcs.RenderingEnd    = gdraw_RenderingEnd;
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
   gdraw_funcs.MakeTextureMore  = gdraw_MakeTextureMore;
   gdraw_funcs.MakeTextureEnd   = gdraw_MakeTextureEnd;

   gdraw_funcs.UpdateTextureBegin = gdraw_UpdateTextureBegin;
   gdraw_funcs.UpdateTextureRect  = gdraw_UpdateTextureRect;
   gdraw_funcs.UpdateTextureEnd   = gdraw_UpdateTextureEnd;

   gdraw_funcs.FreeTexture     = gdraw_FreeTexture;
   gdraw_funcs.TryToLockTexture = gdraw_TryToLockTexture;

   gdraw_funcs.MakeVertexBufferBegin = gdraw_MakeVertexBufferBegin;
   gdraw_funcs.MakeVertexBufferMore  = gdraw_MakeVertexBufferMore;
   gdraw_funcs.MakeVertexBufferEnd   = gdraw_MakeVertexBufferEnd;
   gdraw_funcs.TryToLockVertexBuffer  = gdraw_TryLockVertexBuffer;
   gdraw_funcs.FreeVertexBuffer       = gdraw_FreeVertexBuffer;

   gdraw_funcs.MakeTextureFromResource = (gdraw_make_texture_from_resource *) gdraw_orbis_MakeTextureFromResource;
   gdraw_funcs.FreeTextureFromResource = gdraw_orbis_DestroyTextureFromResource;

   gdraw_funcs.UnlockHandles = gdraw_UnlockHandles;
   gdraw_funcs.SetTextureUniqueID = gdraw_SetTextureUniqueID;
   
   return &gdraw_funcs;
}

void gdraw_orbis_DestroyContext(void)
{
   if (gdraw) {
      GDrawStats stats;
      memset(&stats, 0, sizeof(stats));
      if (gdraw->texturecache) gdraw_res_flush(gdraw->texturecache, &stats);
      if (gdraw->vbufcache) gdraw_res_flush(gdraw->vbufcache, &stats);

      // make sure the GPU is done first
      assert(!is_fence_pending(gdraw->tile_end_fence));

      free_handle_cache(gdraw->texturecache);
      free_handle_cache(gdraw->vbufcache);
      IggyGDrawFree(gdraw);
      gdraw = nullptr;
   }
}

void RADLINK gdraw_orbis_BeginCustomDraw(IggyCustomDrawCallbackRegion *region, float matrix[16])
{
   clear_renderstate();
   gdraw_GetObjectSpaceMatrix(matrix, region->o2w, gdraw->projection, 0.0f, 0);
}

void RADLINK gdraw_orbis_CalculateCustomDraw_4J(IggyCustomDrawCallbackRegion * region, F32 mat[16])
{
   gdraw_GetObjectSpaceMatrix(mat, region->o2w, gdraw->projection, 0.0f, 0);
}

void RADLINK gdraw_orbis_EndCustomDraw(IggyCustomDrawCallbackRegion *region)
{
   set_common_renderstate();
}

GDrawTexture * RADLINK gdraw_orbis_MakeTextureFromResource(U8 *file_in_memory, S32 len, IggyFileTexturePS4 *tex)
{
   Gnm::Texture *texture = (Gnm::Texture *) &tex->texture;
   texture->setBaseAddress(file_in_memory + tex->file_offset);
   texture->m_regs[7] = 0;
   switch (tex->format) {
      case IFT_FORMAT_la_88: texture->setChannelOrder(Gnm::kTextureChannelX, Gnm::kTextureChannelX, Gnm::kTextureChannelX, Gnm::kTextureChannelY); break;
      case IFT_FORMAT_i_8:   texture->setChannelOrder(Gnm::kTextureChannelX, Gnm::kTextureChannelX, Gnm::kTextureChannelX, Gnm::kTextureChannelX); break;
      case IFT_FORMAT_l_8:   texture->setChannelOrder(Gnm::kTextureChannelX, Gnm::kTextureChannelX, Gnm::kTextureChannelX, Gnm::kTextureChannelConstant1); break;
   }
   return gdraw_orbis_WrappedTextureCreate(texture);
}

extern void RADLINK gdraw_orbis_DestroyTextureFromResource(GDrawTexture *tex)
{
   gdraw_orbis_WrappedTextureDestroy(tex);
}


// 4J added - copy of  set_viewport_raw that sets an opengl style z-range rather than the direct-x range used in set_viewport_raw
static void set_viewport_raw_4J(S32 x, S32 y, S32 w, S32 h)
{
   // check against hardware limits
   assert(w >= 0 && w <= 16384);
   assert(h >= 0 && h <= 16384);

   gdraw->cview.x0 = x;
   gdraw->cview.y0 = y;
   gdraw->cview.x1 = x + w;
   gdraw->cview.y1 = y + h;

   F32 scale[3] = { (F32)w * 0.5f, -(F32)h * 0.5f, 0.5f };
   F32 offset[3] = { (F32)x + (F32)w * 0.5f, (F32)y + (F32)h * 0.5f, 0.5f };
   gdraw->gfxc->setViewport(0, 0.0f, 1.0f, scale, offset);
   disable_scissor(true);
}

// 4J added - copy of setViewport, that sets the current viewport but with an opengl-style z-range rather than the direct-x range that Iggy uses internally
// on PS4. We need this to set up a viewport to match Iggy when doing custom rendering
void gdraw_orbis_setViewport_4J()
{
   if (gdraw->in_blur) { // blur needs special setup
      set_viewport_raw_4J(0, 0, gdraw->tpw, gdraw->tph);
      return;
   }

   if (gdraw->cur == gdraw->frame) // if the rendering stack is empty
      // render a tile-sized region to the user-request tile location
      set_viewport_raw_4J(gdraw->vx, gdraw->vy, gdraw->tw, gdraw->th);
   else if (gdraw->cur->cached)
      set_viewport_raw_4J(0, 0, gdraw->cur->width, gdraw->cur->height);
   else
      // if on the render stack, draw a padded-tile-sized region at the origin
      set_viewport_raw_4J(0, 0, gdraw->tpw, gdraw->tph);
}
