#include "stdafx.h"
// gdraw_ps3gcm.cpp - author: Fabian Giesen - copyright 2010-2011 RAD Game Tools
//
// This implements the Iggy graphics driver layer for GCM.

// GDraw consists of several components that interact fairly loosely with each other;
// e.g. the resource management, drawing and filtering parts are all fairly independent
// of each other. If you want to modify some aspect of GDraw - say the texture allocation
// logic - your best bet is usually to just look for one of the related entry points,
// e.g. MakeTextureBegin, and take it from there. There's a bunch of code in this file,
// but most of it isn't really complicated.
//
// One bit you might be tempted to touch is GDraws state management, to integrate it with
// an existing state caching system. This is *not recommended*; Iggy tends to generate a fairly
// large number of batches, and the existing implementation was designed to keep PPU overhead to a
// minimum while also avoiding redundant state changes and keeping the command buffer small.
// Introducing any extra indirections is likely to cause notably degraded performance. Nonetheless,
// if you want to continue, here's the list of functions that modify RSX state in some way:
// - The video memory defragmentation logic
// - All of the rendering helpers 
// - RenderTile*/TextureDrawBuffer* may change the active rendertarget and depth/stencil surface,
//   as do GCM_NoMoreGDrawThisFrame and the rendertarget management code (see comments there)
// - set_rsx_texture / set_texture
// - set_renderstate and set_renderstate_full. These are the main places where render state changes occur;
//   you should probably start here.
// - DrawIndexedTriangles calls set_vertex_decl which sets the active vertex/index buffers and vertex declaration
// - Most of the functions in the "filter effects" section modify RSX state, mostly pixel shader constants and textures
//
// On fences: GDraw makes use of fences to synchronize texture/vertex buffer updates
// that happen during the middle of a frame (which can happen when GDraw uses a texture
// or vertex buffer that hasn't been used before, or one that has since been freed to make
// space for other resources). This uses its own RSX label so you shouldn't generally notice
// or need to worry about it. It is something to keep in mind when modifying any of the
// resource-management code, though; you need to be careful not to mess up the synchronization
// that's there. The current fence index (stored as gdraw->next_fence_index) is just a 64-bit counter,
// incremented whenever a fence (usually a texture label) is inserted into the command buffer.
// wait_on_fence is then used to make sure that the RSX has passed such a fence whenever GDraw
// is about to start an operation that needs synchronization, e.g. modify or free a texture.
// (Technically, memory only needs to be synchronized when it's being re-allocated, not when
// it's freed, but the only case where GDraw frees textures in usual operation is to make
// space for new allocations, so we don't finesse this). This logic was carefully written (and
// tested) to make sure it works properly even in the presence of counter wraparound.
//
// One final thing to be aware about is RSX pipeline offsets between different type of fences.
// As mentioned above, most fences we insert into the command stream are implemented using
// (fairly light-weight) texture labels. But we do use backend labels during video memory
// defragmentation and to synchronize render-to-texture operations. Backend labels are written
// later in the pipeline than texture labels, causing a race condition in sequences like
//
//   cellGcmSetWriteTextureLabel(label, 0)
//   ...
//   cellGcmSetWriteBackEndLabel(label, 1)
//   ...
//   cellGcmSetWriteTextureLabel(label, 2)
//
// where label might theoretically become first 2 then 1. Needless to say, this would
// be very bad. To ensure this can't happen, we make sure to wait on the last issued
// fence before we insert a backend fence. For defragmentation, this wait is on the PPU
// (we need to wait for rendering to finish before we can shuffle textures or vertex
// buffer around). For render-to-texture operations, we insert a WaitLabel command
// first. This may stall the RSX for a short amount of time, but it's no big deal,
// because if we didn't stall there, we'd still have to wait immediately afterwards
// since the render target change that follows it also causes a pipeline flush. This
// was benchmarked not to cause any notable performance degradation - our first
// implementation used two RSX labels, one only updated with texture labels and the
// other with backend labels, but this variant is somewhat simpler and uses just one
// RSX label, so we switched to it.

#define GDRAW_ASSERTS

#include <string.h>
#include <stdlib.h>
#include <cell/gcm.h>
#include <cell/gcm/gcm_method_data.h>
#include <ppu_asm_intrinsics.h>
#include <math.h>
#include "gdraw.h"
#include "iggy.h"

struct GcmTexture;

#include "gdraw_ps3gcm.h"

typedef union {
   struct {
      GcmTexture *gcm;
      void *gcm_ptr;
   } tex;

   struct {
      void *verts;
      void *inds;
   } vbuf;
} GDrawNativeHandle;

#define GDRAW_MANAGE_MEM
#define GDRAW_DEFRAGMENT
#define GDRAW_BUFFER_RING
#define GDRAW_MIN_FREE_AMOUNT              (64*1024)  // always try to free at least this many bytes when throwing out old textures
#include "gdraw_shared.inl"

// max rendertarget stack depth. this depends on the extent to which you
// use filters and non-standard blend modes, and how nested they are.
#define MAX_RENDER_STACK_DEPTH             8          // Iggy is hardcoded to a limit of 16... probably 1-3 is realistic
#define MAX_SAMPLERS                       3          // max number of texture samplers used
#define AATEX_SAMPLER                      7          // sampler that aa_tex gets set in
#define FENCE_BATCH_INTERVAL               64         // put a fence after every N'th batch

static GDrawFunctions gdraw_funcs;

// render target state
typedef struct
{
   GDrawHandle *color_buffer;
   S32 base_x, base_y, width, height;
   rrbool cached;
} GDrawFramebufferState;

struct ProgramWithCachedVariableLocations
{
   union {
      unsigned char *prog_data;
      CGprogram program;
   };
   union {
      void *ucode;      // used for vertex progs
      CellCgbFragmentProgramConfiguration cfg; // used for fragment progs
   };
   int vars[MAX_VARS]; // it's unsigned in d3d, but we want an 'undefined' value
};

struct GcmTexture
{
   // in hardware register format! (for quick texture switching)
   U32 offset;
   U32 format;
   U32 remap;
   U32 imagerect;
   U32 control3;

   // used internally
   U32 width, height;
   U32 pitch;
   U32 swizzled;
};

#define CHAN_A 0
#define CHAN_R 1
#define CHAN_G 2
#define CHAN_B 3
#define TEXREMAP(chan,out,in) ((CELL_GCM_TEXTURE_REMAP_ ## out << (chan*2 + 8)) | ((CELL_GCM_TEXTURE_REMAP_ ## in << (chan*2))))

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

typedef struct
{
   CellGcmContextData     *gcm;

   // scale factor converting worldspace to viewspace <0,0>..<w,h>
   F32 world_to_pixel[2];
   F32 projection[4];

   // cached state
   int vert_format;        // active vertex format (-1 if unknown)
   U32 scissor_state;      // ~0 if unknown, otherwise 0 or 1
   int blend_mode;         // active blend mode (-1 if unknown)
   U32 stencil_key;        // field built from stencil test flags. 0=no stencil, ~0 is used for "unknown state"
   U32 z_key;              // same for z write/z test

   GDrawTexture *active_tex[MAX_SAMPLERS];
   ProgramWithCachedVariableLocations *cur_fprog;

   // fragment shader base pointers
   ProgramWithCachedVariableLocations *basic_fprog[GDRAW_TEXTURE__count];

   // render targets
   CellGcmSurface main_surface;
   GDrawHandleCache rendertargets;
   GDrawHandle rendertarget_handles[MAX_RENDER_STACK_DEPTH]; // not -1, because we use +1 to initialize
   GcmTexture rendertarget_textures[MAX_RENDER_STACK_DEPTH+1];

   gswf_recti rt_valid[MAX_RENDER_STACK_DEPTH+1]; // valid rect for texture clamping

   // size of our render targets
   S32 frametex_width, frametex_height;

   // viewport setting (in pixels) for current frame
   S32 vx,vy;
   S32 fw,fh; // full width/height of virtual display
   S32 tw,th; // actual width/height of current tile
   S32 tpw,tph; // width/height of padded version of tile

   S32 tx0,ty0;
   S32 tx0p,ty0p;

   rrbool in_blur;
   struct {
      S32 x,y,w,h;   
   } cview; // current viewport

   GcmTexture aa_tex;

   // rsx local mem state
   GDrawArena local_arena; // this is where shaders etc. land
   U32 transfer_mode;      // used for defragmentation

   // render-state stack described above for 'temporary' rendering
   GDrawFramebufferState frame[MAX_RENDER_STACK_DEPTH];
   GDrawFramebufferState *cur;

   // ppu/rsx sync
   U32 fence_label_index;
   volatile U32 *fence_label;
   U64 next_fence_index; // next fence index we're going to write
   U32 fence_batch_counter; // used to write fences every N batches

   // texture and vertex buffer pools
   GDrawHandleCache *texturecache;
   int tex_loc; // CELL_GCM_LOCATION of textures

   GDrawHandleCache *vbufcache;
   U8 *vbuf_base; // used for fast addr2offs
   int vbuf_loc; // CELL_GCM_LOCATION of vertex buffers

   // rendertarget stuff
   GDrawArena rt_arena;
   S32 rt_pitch;  // render target pitch
   int rt_loc; // CELL_GCM_LOCATION of rendertargets

   // dyn vertex buffer
   gdraw_bufring dyn_vb;
   U8 *dynvb_base; // used for fast addr2offs
   int dynvb_loc; // CELL_GCM_LOCATION of dyn vertex buffer

   // fragment shaders
   ProgramWithCachedVariableLocations fprog[GDRAW_TEXTURE__count][3];
   ProgramWithCachedVariableLocations exceptional_blend[GDRAW_BLENDSPECIAL__count];
   ProgramWithCachedVariableLocations filter_prog[2][16];
   ProgramWithCachedVariableLocations blur_prog[MAX_TAPS+1];
   ProgramWithCachedVariableLocations colormatrix;

   // vertex shaders
   ProgramWithCachedVariableLocations vprog[GDRAW_vformat__basic_count];
   U32 vslot[GDRAW_vformat__basic_count];

   // mipmapping
   GDrawMipmapContext mipmap;

   // for bookkeeping
   GDrawFence tile_end_fence;
} GDraw;

#define COLOR_MASK_ALL (CELL_GCM_COLOR_MASK_R | CELL_GCM_COLOR_MASK_G | CELL_GCM_COLOR_MASK_B | CELL_GCM_COLOR_MASK_A)

static GDraw *gdraw;

////////////////////////////////////////////////////////////////////////
//
//   Vertex program registers
//

// These are direct constant register indices
// NOTE: RSX can't set more than 8 vertex constants at once!
#define VVAR_world 0
#define VVAR_count_worldonly 2 // number of constants to update when we only change world matrix
#define VVAR_x_off 2
#define VVAR_color_mul 3
#define VVAR_count_world_and_color 4
#define VVAR_tex_s 4
#define VVAR_tex_t 5
#define VVAR_count 6          // number of vertex program registers to update when we change everything

#define VVAR_viewproj 6       // projection is only updated when it actually changes

struct Vec4
{
   F32 x,y,z,w;
};

struct VertexVars
{
   Vec4 world[2];
   Vec4 x_off;
   Vec4 color_mul;
   Vec4 s0_texgen;
   Vec4 t0_texgen;
   Vec4 proj;
};

////////////////////////////////////////////////////////////////////////
//
//   Fence and command buffer helpers
//

// used to write to the command buffer: gcc won't generate clrldi
// instructions for struct member access, but it will for pointer
// arithmetic.
struct CommandData
{
   U32 w0,w1,w2,w3,w4,w5,w6,w7;
   U32 w8,w9,wa,wb,wc,wd,we,wf;
};

static RADINLINE CommandData *reserve_command(CellGcmContextData * RADRESTRICT gcm, U32 size)
{
   cellGcmReserveMethodSizeInline(gcm, size);
   return (CommandData *) gcm->current;
}

static RADINLINE CommandData *put_command(CellGcmContextData * RADRESTRICT gcm, U32 size)
{
   cellGcmReserveMethodSizeInline(gcm, size);
   CommandData *cmd = (CommandData *) gcm->current;
   gcm->current += size;
   return cmd;
}

static RADINLINE GDrawFence get_next_fence()
{
   GDrawFence fence;
   fence.value = gdraw->next_fence_index;
   return fence;
}

static RADINLINE void flush_delayed_fence_updates()
{
}

static RADINLINE rrbool is_fence_pending(GDrawFence fence)
{
   // if it's older than one full wrap of the fence counter,
   // we know it's retired. (we can't have more than 4 billion
   // fences pending in command buffers with only 256MB of
   // main memory!)
   if (gdraw->next_fence_index - fence.value > 0xffffffffu)
      return false;

   // this is how far the GPU is.
   U32 retired = *gdraw->fence_label;

   // everything between "retired" (exclusive) and "next_fence_index"
   // (inclusive) is pending. everything else is definitely done.
   //
   // we need to be careful about this test since the "fence" value
   // coming in is, for all practical purposes, an arbitrary U32. our
   // fence counter might have wrapped around multiple times since we last
   // used a resource that gets freed, for instance! so if we report a
   // fence ID as pending that's not actually in flight, we might end up
   // with an infinite wait.
   // 
   // this is a bit subtle since it depends on unsigned wraparound for us
   // to do the right thing.

   // number of pending fences (next_fence_index has, by definition, not been written yet)
   U32 num_pending = U32(gdraw->next_fence_index) - retired;

   // position of the current fence in the "list of pending fences", counting
   // from end (i.e. the "youngest" fence that we haven't even issued yet)
   U32 pos_in_pending_list = U32(gdraw->next_fence_index - fence.value);

   return pos_in_pending_list < num_pending;
}

static GDrawFence put_fence()
{
   GDrawFence fence;
   gdraw->fence_batch_counter = FENCE_BATCH_INTERVAL;
   fence.value = gdraw->next_fence_index++;
   cellGcmSetWriteTextureLabelInline(gdraw->gcm, gdraw->fence_label_index, (U32) fence.value);
   return fence;
}

static GDrawFence put_backend_fence()
{
   // careful with backend labels, they have no defined ordering wrt
   // texture labels in the same command stream! if you use them, make
   // sure there are no races.
   GDrawFence fence;
   gdraw->fence_batch_counter = FENCE_BATCH_INTERVAL;
   fence.value = gdraw->next_fence_index++;
   cellGcmSetWriteBackEndLabel(gdraw->gcm, gdraw->fence_label_index, (U32) fence.value);
   return fence;
}

static void wait_on_fence(GDrawFence fence)
{
   if (is_fence_pending(fence)) {
      GDraw * RADRESTRICT gd = gdraw;
      if (fence.value == gd->next_fence_index) // haven't even written this one yet!
         put_fence();
      cellGcmFlush(gd->gcm);
      IggyWaitOnFence((void *) gd->fence_label, (U32) fence.value);
   }
}

static U32 addr2offs(void *addr)
{
   U32 offs;
   int32_t res = cellGcmAddressToOffset(addr, &offs);
   res = res; // avoid warning in release builds
   assert(res == CELL_OK);
   return offs;
}  

static RADINLINE U32 vbufaddr2offs(GDraw * RADRESTRICT gd, void *addr)
{
   return (U8 *) addr - gd->vbuf_base;
}

static RADINLINE U32 dynvbaddr2offs(GDraw * RADRESTRICT gd, void *addr)
{
   return (U8 *) addr - gd->dynvb_base;
}

////////////////////////////////////////////////////////////////////////
//
//   Texture/video memory defragmentation support code
//

static void gdraw_gpu_memcpy(GDrawHandleCache *c, void *dst, void *src, U32 num_bytes)
{
   RR_UNUSED_VARIABLE(c);
   U32 dstoffs = addr2offs(dst);
   U32 srcoffs = addr2offs(src);
   U32 pos = 0;
   U32 edgelen = 4096;
   U32 blocksize = edgelen*edgelen*4;

   assert((num_bytes & 3) == 0);

   while (pos < num_bytes) {
      // peel off square image transfers of edgelen x edgelen as long as we can
      while (pos + blocksize <= num_bytes) {
         assert(((dstoffs + pos) & (CELL_GCM_SURFACE_LINEAR_ALIGN_OFFSET - 1)) == 0);
         assert(((srcoffs + pos) & (CELL_GCM_SURFACE_LINEAR_ALIGN_OFFSET - 1)) == 0);
         cellGcmSetTransferImage(gdraw->gcm, gdraw->transfer_mode,
            dstoffs + pos, edgelen * 4, 0, 0,
            srcoffs + pos, edgelen * 4, 0, 0,
            edgelen, edgelen, 4);
         pos += blocksize;
      }

      edgelen >>= 1;
      blocksize >>= 2;
      if (edgelen == 32)
         break; // handle the rest (<4k "pixels") using a 1-line transfer
   }

   if (pos < num_bytes) {
      U32 amount = num_bytes - pos;
      assert(((dstoffs + pos) & (CELL_GCM_SURFACE_LINEAR_ALIGN_OFFSET - 1)) == 0);
      assert(((srcoffs + pos) & (CELL_GCM_SURFACE_LINEAR_ALIGN_OFFSET - 1)) == 0);
      cellGcmSetTransferImage(gdraw->gcm, gdraw->transfer_mode,
         dstoffs + pos, amount, 0, 0,
         srcoffs + pos, amount, 0, 0,
         amount >> 2, 1, 4);
   }
}

static void gdraw_defragment_cache(GDrawHandleCache *c, GDrawStats *stats)
{
   GDrawFence fence;
   S32 i;
   int loc;

   if (!gdraw_CanDefragment(c))
      return;

   // gpu needs to finish pending batches before it can start defragmenting
   fence = put_fence();
   cellGcmSetWaitLabel(gdraw->gcm, gdraw->fence_label_index, (U32) fence.value);

   // actual defragmentation...
   loc = c->is_vertex ? gdraw->vbuf_loc : gdraw->tex_loc;
   gdraw->transfer_mode = (loc == CELL_GCM_LOCATION_LOCAL) ? CELL_GCM_TRANSFER_LOCAL_TO_LOCAL : CELL_GCM_TRANSFER_MAIN_TO_MAIN;
   gdraw_DefragmentMain(c, 0, stats);

   // go over all handles and adjustment pointers.
   // pointer adjustment is different for textures than it is for vertex buffers
   if (!c->is_vertex) {
      for (i=0; i < c->max_handles; i++) {
         GDrawHandle *h = &c->handle[i];
         if (gdraw_res_is_managed(h)) {
            h->handle.tex.gcm_ptr = h->raw_ptr;
            h->handle.tex.gcm->offset = CELL_GCM_METHOD_DATA_TEXTURE_OFFSET(addr2offs(h->handle.tex.gcm_ptr));
         }
      }
   } else {
      for (i=0; i < c->max_handles; i++) {
         GDrawHandle *h = &c->handle[i];
         if (gdraw_res_is_managed(h)) {
            SINTa ind_offs = ((U8 *) h->handle.vbuf.inds - (U8 *) h->handle.vbuf.verts);
            h->handle.vbuf.verts = h->raw_ptr;
            h->handle.vbuf.inds = (U8 *) h->raw_ptr + ind_offs;
         }
      }
   }

   // texture pointers have changed, so textures need to be reset
   memset(&gdraw->active_tex, 0, sizeof(gdraw->active_tex));

   // wait for RSX to finish, since we can't safely allocate memory in this cache
   // while data is being moved around.
   // could delay this until the next alloc, but the only place we ever call
   // defragment from is just before an alloc, so there's no point.
   // no backend fence race: we had RSX wait on completion of the last pending
   // texture label before we started this.
   cellGcmSetInvalidateTextureCache(gdraw->gcm, CELL_GCM_INVALIDATE_TEXTURE);
   cellGcmSetInvalidateVertexCache(gdraw->gcm);
   wait_on_fence(put_backend_fence());
}

////////////////////////////////////////////////////////////////////////
//
//   RSX texture swizzling code.
//
//   cellGcmConvertSwizzleFormat is too slow (one callback per pixel!)
//   and RSX swizzle transfers require local mem for the unswizzled and
//   swizzled versions at the same time during the conversion, which
//   is unacceptable for large textures.
//

// NOTE:
//
// RSX texture swizzling uses Morton order inside squares of size
// minor x minor, where minor=min(w,h). If the texture is non-square,
// we might have multiple such squares, which are arranged linearly
// in memory.
//
// There's a nice way to step pixel coordinates given in Morton order
// incrementally (cf. "Morton-order Matrices Deserve Compilers' Support",
// D. S. Wise and J. D. Frens, https://www.cs.indiana.edu/cgi-bin/techreports/TRNNN.cgi?trnum=TR533).
//

// "insert" a 0 bit after each of the 16 low bits of x. used for morton encoding.
static U32 part1by1(U32 x)
{
  x &= 0x0000ffff;                  // x = ---- ---- ---- ---- fedc ba98 7654 3210
  x = (x ^ (x <<  8)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
  x = (x ^ (x <<  4)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
  x = (x ^ (x <<  2)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
  x = (x ^ (x <<  1)) & 0x55555555; // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
  return x;
}

// same for the RSX modified morton encoding. here we need to know which axis we're dealing with.
static U32 rsx_morton_encode(S32 x, U32 axis, U32 minor)
{
   // low bits are morton, high bits are linear
   return (part1by1((U32) x & (minor - 1)) << axis) + ((U32) x & ~(minor - 1)) * minor;
}

#ifdef __SNC__
#define GCCSchedBarrier()
#else
#define GCCSchedBarrier() __asm__ volatile("")
#endif

// update a subrect of a 8bpp texture. w x h pixels are copied from src to
// dest at pixel position (dx,dy). the full destination texture is dw x dh pixels.
static void swizzle_subrect_8bpp(U8 * RADRESTRICT dest, U32 dx, U32 dy, U32 dw, U32 dh, U8 * RADRESTRICT src, U32 srcpitch, U32 w, U32 h)
{
   // determine morton-order stepping constants.
   U32 minor = RR_MIN(dw, dh);
   S64 xinc = (S32) rsx_morton_encode(-1, 0, minor);
   S64 yinc = (S32) rsx_morton_encode(-1, 1, minor);

   // determine start offsets along x/y axis
   U64 outx0 = rsx_morton_encode(dx, 0, minor);
   U64 outy = rsx_morton_encode(dy, 1, minor);

   while (h--) {
      U8 *in = src - 1;
      U8 *out = dest + outy;
      U64 i,outx;

      // copy and swizzle one line
      outx = outx0;
#ifdef __SNC__
      for (i=0; i < w; i++) {
         out[outx] = *++in;
         outx = (outx - xinc) & xinc; // outx++ with bit-interleaving
      }
#else
      // this loop does the same as the above but generates *way* better code on GCC.
      for (i=0; i < w; i++) {
         U64 v,ox;

         v = __lbzu(1, in);
         GCCSchedBarrier();
         ox = outx;
         outx = __subf(xinc, outx);
         GCCSchedBarrier();
         __stbx(v, out, ox);
         outx = __and(outx, xinc);
      }
#endif

      src += srcpitch;
      outy = (outy - yinc) & yinc; // outy++ with bit-interleaving
   }
}

// update a subrect of a 32bpp texture (version for small rects)
static void swizzle_subrect_32bpp_small(U8 * RADRESTRICT dest, U32 dx, U32 dy, U32 dw, U32 dh, U8 * RADRESTRICT src, U32 srcpitch, U32 w, U32 h)
{
   // determine morton-order stepping constants
   U32 minor = RR_MIN(dw, dh);
   S64 xinc = (S32) rsx_morton_encode(-1, 0, minor) * 4; // *4 since we work with byte offsets
   S64 yinc = (S32) rsx_morton_encode(-1, 1, minor) * 4;

   // determine start offsets along x/y axis
   U64 outx0 = rsx_morton_encode(dx, 0, minor) * 4;
   U64 outy = rsx_morton_encode(dy, 1, minor) * 4;

   while (h--) {
      U32 *in = (U32 *) src - 1;
      U8 *out = dest + outy;
      U64 i,outx;

      // copy and swizzle one line
      outx = outx0;
#ifdef __SNC__
      for (i=0; i < w; i++) {
         *((U32 *) (out + outx)) = *++in;
         outx = (outx - xinc) & xinc; // outx++ with bit-interleaving
      }
#else      
      // this loop does the same as the above but generates *way* better code on GCC.
      for (i=0; i < w; i++) {
         U64 v,ox;

         v = __lwzu(4, in);
         GCCSchedBarrier();
         ox = outx;
         outx = __subf(xinc, outx);
         GCCSchedBarrier();
         __stwx(v, out, ox);
         outx = __and(outx, xinc);
      }
#endif

      src += srcpitch;
      outy = (outy - yinc) & yinc; // outy++ with bit-interleaving
   }
}

// update a subrect of a 32 bpp texture (main entry point)
static void swizzle_subrect_32bpp(U8 * RADRESTRICT dest, U32 dx, U32 dy, U32 dw, U32 dh, U8 * RADRESTRICT src, U32 srcpitch, U32 w, U32 h)
{
   U32 minor;
   U32 dx0,dy0,dx1,dy1;
   U32 wa,ha;
   U32 x,y;
   S64 incx,incy; // output increment (per block)
   U64 outx0,outx,outy; // output *offset* in x/y
   U64 a,b,c,d;

   // we have a fast path that updates aligned groups of 8x4 pixels at a time.
   // use that for the bulk of the data, then puzzle the rest together from smaller rects.
   if (w < 8 || h < 4) { // small block, don't bother
      swizzle_subrect_32bpp_small(dest, dx, dy, dw, dh, src, srcpitch, w, h);
      return;
   }

   // calculate the aligned part of the subrect that we process with the fast loop
   // we cut the image up like this:
   // +---------------------+ dy
   // |                     |
   // +---+---------------+-+ dy0
   // |   |               | |
   // |   |               | |
   // +---+---------------+-+ dy1
   // +---------------------+ dy+h
   // dx  dx0           dx1 dx+w
   dx0 = (dx + 7) & ~7;
   dy0 = (dy + 3) & ~3;
   dx1 = (dx + w) & ~7;
   dy1 = (dy + h) & ~3;
   wa = dx1 - dx0;
   ha = dy1 - dy0;

   // if dy wasn't aligned, peel off the first couple of lines
   if (dy < dy0) {
      swizzle_subrect_32bpp_small(dest, dx, dy, dw, dh, src, srcpitch, w, dy0 - dy);
      src += srcpitch * (dy0 - dy);
   }

   // take care of the left/right parts that aren't aligned
   if (dx < dx0)
      swizzle_subrect_32bpp_small(dest, dx, dy0, dw, dh, src, srcpitch, dx0 - dx, ha);

   if (dx1 < dx+w)
      swizzle_subrect_32bpp_small(dest, dx1, dy0, dw, dh, src + (dx1-dx)*4, srcpitch, (dx+w) - dx1, ha);

   // main part: go through image in blocks of 8x4 pixels. (8x4 since that's one full cache line,
   // so we write pixels one cache line at a time)
   minor = RR_MIN(dw, dh);
   incx = (S32) rsx_morton_encode(-8, 0, minor) * 4; // *4 since it's all byte offsets
   incy = (S32) rsx_morton_encode(-4, 1, minor) * 4;
   outx0 = rsx_morton_encode(dx0, 0, minor) * 4;
   outy = rsx_morton_encode(dy0, 1, minor) * 4;

   for (y=0; y < ha/4; ++y) {
      // set up source line pointers for four lines
      void *src0 = (src + (dx0 - dx) * 4 - 8);
      void *src1 = ((U8 *) src0 + srcpitch);
      void *src2 = ((U8 *) src1 + srcpitch);
      void *src3 = ((U8 *) src2 + srcpitch);
      outx = outx0;

      // @TODO prefetches would probably be a good idea here, but need proper test data.
      for (x=0; x < wa/8; ++x) {
         void *out = (void *) (dest + outx + outy);
         outx = (outx - incx) & incx; // advance pointer for next pixel

         // just read 8x4 pixels and write them out in the z-order pattern
         // we can use doubleword loads since even in Z-order, groups of two horizontal
         // pixels don't get reordered. the rest is just the swizzle pattern expanded into offsets.
         a = __ld(0x08, src0);   b = __ld(0x08, src1);   c = __ld(0x10, src0);   d = __ld(0x10, src1);
         __std(a, 0x00, out);    __std(b, 0x08, out);    __std(c, 0x10, out);    __std(d, 0x18, out);
         a = __ld(0x08, src2);   b = __ld(0x08, src3);   c = __ld(0x10, src2);   d = __ld(0x10, src3);
         __std(a, 0x20, out);    __std(b, 0x28, out);    __std(c, 0x30, out);    __std(d, 0x38, out);
         a = __ld(0x18, src0);   b = __ld(0x18, src1);   c = __ldu(0x20, src0);  d = __ldu(0x20, src1);
         __std(a, 0x40, out);    __std(b, 0x48, out);    __std(c, 0x50, out);    __std(d, 0x58, out);
         a = __ld(0x18, src2);   b = __ld(0x18, src3);   c = __ldu(0x20, src2);  d = __ldu(0x20, src3);
         __std(a, 0x60, out);    __std(b, 0x68, out);    __std(c, 0x70, out);    __std(d, 0x78, out);
      }

      src += 4*srcpitch;
      outy = (outy - incy) & incy;
   }

   // and finally, the last few lines
   if (dy1 < dy+h)
      swizzle_subrect_32bpp_small(dest, dx, dy1, dw, dh, src, srcpitch, w, (dy+h) - dy1);
}

static void api_free_resource(GDrawHandle *r)
{
   // we just need to clean up our state cache
   S32 i;
   for (i=0; i < MAX_SAMPLERS; i++)
      if (gdraw->active_tex[i] == (GDrawTexture *) r)
         gdraw->active_tex[i] = nullptr;
}

static void RADLINK gdraw_UnlockHandles(GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(stats);
   gdraw_HandleCacheUnlockAll(gdraw->texturecache);
   gdraw_HandleCacheUnlockAll(gdraw->vbufcache);
}

////////////////////////////////////////////////////////////////////////
//
//   Texture creation/updating/deletion
//

extern GDrawTexture *gdraw_GCM_WrappedTextureCreate(CellGcmTexture *gcm_tex)
{
   GDrawStats stats;
   memset(&stats, 0, sizeof(stats));
   GDrawHandle *p = gdraw_res_alloc_begin(gdraw->texturecache, 0, &stats); // it may need to free one item to give us a handle
   p->handle.tex.gcm_ptr = 0;
   gdraw_HandleCacheAllocateEnd(p, 0, nullptr, GDRAW_HANDLE_STATE_user_owned);
   gdraw_GCM_WrappedTextureChange((GDrawTexture *) p, gcm_tex);
   return (GDrawTexture *) p;
}

extern void gdraw_GCM_WrappedTextureChange(GDrawTexture *tex, CellGcmTexture *gcm_tex)
{
   GDrawHandle *p = (GDrawHandle *) tex;
   GcmTexture *gcm = p->handle.tex.gcm;
   gcm->offset = CELL_GCM_METHOD_DATA_TEXTURE_OFFSET(gcm_tex->offset);
   gcm->format = CELL_GCM_METHOD_DATA_TEXTURE_FORMAT(gcm_tex->location, gcm_tex->cubemap, gcm_tex->dimension, gcm_tex->format, gcm_tex->mipmap);
   gcm->remap = gcm_tex->remap;
   gcm->imagerect = CELL_GCM_METHOD_DATA_TEXTURE_IMAGE_RECT(gcm_tex->height, gcm_tex->width);
   gcm->control3 = CELL_GCM_METHOD_DATA_TEXTURE_CONTROL3(gcm_tex->pitch, gcm_tex->depth);
   gcm->width = gcm_tex->width;
   gcm->height = gcm_tex->height;
   gcm->pitch = gcm_tex->pitch;
   gcm->swizzled = 0; // unused since we never upload to this
}

extern void gdraw_GCM_WrappedTextureDestroy(GDrawTexture *tex)
{
   GDrawStats stats;
   gdraw_res_free((GDrawHandle *) tex, &stats);
}

static void RADLINK gdraw_SetTextureUniqueID(GDrawTexture *tex, void *old_id, void *new_id)
{
   GDrawHandle *p = (GDrawHandle *) tex;
   // if this is still the handle it's thought to be, change the owner;
   // if the owner *doesn't* match, then they're changing a stale handle, so ignore
   if (p->owner == old_id)
      p->owner = new_id;
}

static bool is_texture_swizzled(S32 w, S32 h)
{
   // we swizzle a texture if it's pow2 and not a line texture
   return h > 1 && (w & (w-1)) == 0 && (h & (h-1)) == 0;
}

static rrbool RADLINK gdraw_MakeTextureBegin(void *owner, S32 width, S32 height, gdraw_texture_format gformat, U32 flags, GDraw_MakeTexture_ProcessingInfo *p, GDrawStats *stats)
{
   S32 bytes_pixel = 4;
   GDrawHandle *t = nullptr;
   bool swizzled = false;

   if (width > 4096 || height > 4096) {
      IggyGDrawSendWarning(nullptr, "GDraw %d x %d texture not supported by hardware (dimension size limit 4096)", width, height);
      return false;
   }

   if (gformat == GDRAW_TEXTURE_FORMAT_font)
      bytes_pixel = 1;

   swizzled = is_texture_swizzled(width, height);

   // determine the number of mipmaps to use and the size of the corresponding texture allocation
   // this assumes linear texture memory layout
   S32 pitch = width * bytes_pixel;
   U32 mipmaps = 0;
   S32 size = 0;
   if (!swizzled) // RSX HW bug: linear texture pitch size should be at least 16 bytes
       pitch = RR_MAX(pitch, 16);

   do {
      if (!swizzled)
         size += pitch * (RR_MAX(height >> mipmaps, 1));
      else
         size += RR_MAX(pitch >> mipmaps, bytes_pixel) * RR_MAX(height >> mipmaps, 1);
      mipmaps++;
   } while ((flags & GDRAW_MAKETEXTURE_FLAGS_mipmap) && ((width >> mipmaps) || (height >> mipmaps)));

   // allocate a handle and make room in the cache for this much data
   assert(size != 0);
   t = gdraw_res_alloc_begin(gdraw->texturecache, size, stats);
   if (!t)
      return false;

   t->handle.tex.gcm_ptr = t->raw_ptr;

   GcmTexture *tex = t->handle.tex.gcm;
   U8 format;
   U32 remap;

   if (gformat == GDRAW_TEXTURE_FORMAT_font) {
      format = CELL_GCM_TEXTURE_B8;
      remap = TEXREMAP(CHAN_A, REMAP, FROM_B) | TEXREMAP(CHAN_R, ZERO, FROM_R) | TEXREMAP(CHAN_G, ZERO, FROM_G) | TEXREMAP(CHAN_B, ZERO, FROM_B);
   } else {
      format = CELL_GCM_TEXTURE_A8R8G8B8;
      // remap rgba -> argb
      remap = TEXREMAP(CHAN_A, REMAP, FROM_B) | TEXREMAP(CHAN_R, REMAP, FROM_A) | TEXREMAP(CHAN_G, REMAP, FROM_R) | TEXREMAP(CHAN_B, REMAP, FROM_G);
   }

   format |= (swizzled ? CELL_GCM_TEXTURE_SZ : CELL_GCM_TEXTURE_LN) | CELL_GCM_TEXTURE_NR;
   U8 dimension = (height != 1) ? CELL_GCM_TEXTURE_DIMENSION_2 : CELL_GCM_TEXTURE_DIMENSION_1;
   U8 cubemap = 0;
   U8 depth = 1;
   U8 location = gdraw->tex_loc;

   tex->offset = CELL_GCM_METHOD_DATA_TEXTURE_OFFSET(addr2offs(t->handle.tex.gcm_ptr));
   tex->format = CELL_GCM_METHOD_DATA_TEXTURE_FORMAT(location, cubemap, dimension, format, mipmaps);
   tex->remap = remap;
   tex->imagerect = CELL_GCM_METHOD_DATA_TEXTURE_IMAGE_RECT(height, width);
   tex->control3 = CELL_GCM_METHOD_DATA_TEXTURE_CONTROL3(pitch, depth);
   tex->width = width;
   tex->height = height;
   tex->pitch = pitch;
   tex->swizzled = swizzled;
   
   gdraw_HandleCacheAllocateEnd(t, size, owner, (flags & GDRAW_MAKETEXTURE_FLAGS_never_flush) ? GDRAW_HANDLE_STATE_pinned : GDRAW_HANDLE_STATE_locked);
   stats->nonzero_flags |= GDRAW_STATS_alloc_tex;
   stats->alloc_tex += 1;
   stats->alloc_tex_bytes += size;

   p->texture_type = GDRAW_TEXTURE_TYPE_rgba;
   p->p0 = t;

   if (swizzled || (flags & GDRAW_MAKETEXTURE_FLAGS_mipmap)) {
      rrbool ok;

      assert(p->temp_buffer != nullptr);
      ok = gdraw_MipmapBegin(&gdraw->mipmap, width, height, mipmaps,
         bytes_pixel, p->temp_buffer, p->temp_buffer_bytes);
      assert(ok); // this should never hit unless the temp_buffer is way too small

      p->p1 = &gdraw->mipmap;
      p->texture_data = gdraw->mipmap.pixels[0];
      p->num_rows = gdraw->mipmap.bheight;
      p->stride_in_bytes = gdraw->mipmap.pitch[0];
      p->i0 = 0; // current output y
   } else {
      p->p1 = 0;
      p->texture_data = (U8 *) t->handle.tex.gcm_ptr;
      p->num_rows = height;
      p->stride_in_bytes = t->handle.tex.gcm->pitch;
   }

   return true;
}

static rrbool RADLINK gdraw_MakeTextureMore(GDraw_MakeTexture_ProcessingInfo *p)
{
   GDrawHandle *t = (GDrawHandle *) p->p0;
   
   if (p->p1) {
      GcmTexture *tex = t->handle.tex.gcm;
      GDrawMipmapContext *c = (GDrawMipmapContext *) p->p1;
      U32 pitch = tex->pitch;
      U32 width = tex->width;
      U32 height = tex->height;
      U32 bheight = c->bheight;
      U32 level = 0;
      U32 outy = p->i0;
      U8 *mipstart = (U8 *) t->handle.tex.gcm_ptr;

      if (outy >= tex->height) // wait, we've already processed the whole texture!
         return false;

      do {
         // copy image data to destination
         if (!tex->swizzled) {
            U8 *dest = mipstart + (outy >> level) * pitch;
            U8 *src = c->pixels[level];
            U32 y;

            for (y=0; y < bheight; y++) {
               memcpy(dest, src, width * c->bpp);
               dest += pitch;
               src += c->pitch[level];
            }

            mipstart += pitch * height;
         } else {
            if (c->bpp == 4)
               swizzle_subrect_32bpp(mipstart, 0, outy >> level, width, height, c->pixels[level], c->pitch[level], width, bheight);
            else if (c->bpp == 1)
               swizzle_subrect_8bpp(mipstart, 0, outy >> level, width, height, c->pixels[level], c->pitch[level], width, bheight);
            else {
               assert(c->bpp == 1 || c->bpp == 4);
            }

            mipstart += width * height * c->bpp;
         }

         width = RR_MAX(width >> 1, 1);
         height = RR_MAX(height >> 1, 1);
         bheight = RR_MAX(bheight >> 1, 1);
      } while (gdraw_MipmapAddLines(c, ++level));

      // next chunk please!
      p->i0 += p->num_rows; // increment y
      p->texture_data = c->pixels[0];
      p->num_rows = c->bheight = RR_MIN(c->bheight, tex->height - p->i0);
      return true;
   } else
      return false; // what do you mean, "more"? you got the whole image already!
}

static GDrawTexture * RADLINK gdraw_MakeTextureEnd(GDraw_MakeTexture_ProcessingInfo *p, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(stats);
   if (p->p1)
      gdraw_MakeTextureMore(p); // submit last piece of data using more

   return (GDrawTexture *) p->p0;
}

static rrbool RADLINK gdraw_UpdateTextureBegin(GDrawTexture *t, void *unique_id, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(stats);
   return gdraw_HandleCacheLock((GDrawHandle *) t, unique_id);
}

static void RADLINK gdraw_UpdateTextureRect(GDrawTexture *t, void *unique_id, S32 x, S32 y, S32 stride, S32 w, S32 h, U8 *samples, gdraw_texture_format format)
{
   RR_UNUSED_VARIABLE(unique_id);
   GDrawHandle *s = (GDrawHandle *) t;
   S32 bpp = (format == GDRAW_TEXTURE_FORMAT_font) ? 1 : 4, bpl = bpp * w;
   GcmTexture *tex = s->handle.tex.gcm;
   U8 *texptr = (U8 *) s->handle.tex.gcm_ptr;

   wait_on_fence(s->fence); // make sure it's not active

   if (!tex->swizzled) {
      S32 dpitch = tex->pitch;
      U8 *src = samples;
      U8 *dst = texptr + y * dpitch + x * bpp;
      while (h--) {
         memcpy(dst, src, bpl);
         dst += dpitch;
         src += stride;
      }
   } else {
      if (format == GDRAW_TEXTURE_FORMAT_font)
         swizzle_subrect_8bpp(texptr, x, y, tex->width, tex->height, samples, stride, w, h);
      else
         swizzle_subrect_32bpp(texptr, x, y, tex->width, tex->height, samples, stride, w, h);
   }
}

static void RADLINK gdraw_UpdateTextureEnd(GDrawTexture *t, void *unique_id, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(unique_id);
   RR_UNUSED_VARIABLE(stats);
   gdraw_HandleCacheUnlock((GDrawHandle *) t);
}

static void RADLINK gdraw_FreeTexture(GDrawTexture *tt, void *unique_id, GDrawStats *stats)
{
   GDrawHandle *t = (GDrawHandle *) tt;
   assert(t != nullptr); // @GDRAW_ASSERT
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
   RR_UNUSED_VARIABLE(stats);
   return gdraw_HandleCacheLock((GDrawHandle *) t, unique_id);
}

static void RADLINK gdraw_DescribeTexture(GDrawTexture *tex, GDraw_Texture_Description *desc)
{
   GDrawHandle *p = (GDrawHandle *) tex;
   desc->width = p->handle.tex.gcm->width;
   desc->height = p->handle.tex.gcm->height;
   desc->size_in_bytes = p->bytes;
}

static void RADLINK gdraw_SetAntialiasTexture(S32 width, U8 *rgba)
{
   if (gdraw->aa_tex.offset)
      return;

   S32 pitch = RR_MAX(width * 4, 16); // RSX HW bug: linear textures with pitch<16 cause trouble (we're not swizzled, but play it safe anyway).
   U8 *data = (U8 *)gdraw_arena_alloc(&gdraw->local_arena, pitch, CELL_GCM_TEXTURE_SWIZZLE_ALIGN_OFFSET);
   if (!data)
      return;

   // since it's a line texture, we can safely mark it as swizzled.
   gdraw->aa_tex.offset = CELL_GCM_METHOD_DATA_TEXTURE_OFFSET(addr2offs(data));
   gdraw->aa_tex.format = CELL_GCM_METHOD_DATA_TEXTURE_FORMAT(CELL_GCM_LOCATION_LOCAL, 0, CELL_GCM_TEXTURE_DIMENSION_1,
      CELL_GCM_TEXTURE_A8R8G8B8 | CELL_GCM_TEXTURE_SZ | CELL_GCM_TEXTURE_NR, 1);
   gdraw->aa_tex.remap = TEXREMAP(CHAN_A, REMAP, FROM_B) | TEXREMAP(CHAN_R, REMAP, FROM_A) | TEXREMAP(CHAN_G, REMAP, FROM_R) | TEXREMAP(CHAN_B, REMAP, FROM_G);
   gdraw->aa_tex.imagerect = CELL_GCM_METHOD_DATA_TEXTURE_IMAGE_RECT(1, width);
   gdraw->aa_tex.control3 = CELL_GCM_METHOD_DATA_TEXTURE_CONTROL3(pitch, 1);
   gdraw->aa_tex.width = width;
   gdraw->aa_tex.height = 1;
   gdraw->aa_tex.pitch = pitch;
   memcpy(data, rgba, width * 4);
}

////////////////////////////////////////////////////////////////////////
//
//   Vertex buffer creation/deletion
//

static rrbool RADLINK gdraw_MakeVertexBufferBegin(void *unique_id, gdraw_vformat vformat, S32 vbuf_size, S32 ibuf_size, GDraw_MakeVertexBuffer_ProcessingInfo *p, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(vformat);
   GDrawHandle *vb;
   vb = gdraw_res_alloc_begin(gdraw->vbufcache, vbuf_size + ibuf_size, stats);
   if (!vb)
      return false;

   vb->handle.vbuf.verts = vb->raw_ptr;
   vb->handle.vbuf.inds = (U8 *) vb->raw_ptr + vbuf_size;
   
   p->p0 = vb;
   p->vertex_data = (U8 *) vb->handle.vbuf.verts;
   p->index_data = (U8 *) vb->handle.vbuf.inds;
   p->vertex_data_length = vbuf_size;
   p->index_data_length = ibuf_size;

   gdraw_HandleCacheAllocateEnd(vb, vbuf_size + ibuf_size, unique_id, GDRAW_HANDLE_STATE_locked);

   return true;
}

static rrbool RADLINK gdraw_MakeVertexBufferMore(GDraw_MakeVertexBuffer_ProcessingInfo *p)
{
   RR_UNUSED_VARIABLE(p);
   assert(0);
   return false;
}

static GDrawVertexBuffer * RADLINK gdraw_MakeVertexBufferEnd(GDraw_MakeVertexBuffer_ProcessingInfo *p, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(stats);
   return (GDrawVertexBuffer *) p->p0;
}

static rrbool RADLINK gdraw_TryLockVertexBuffer(GDrawVertexBuffer *vb, void *unique_id, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(stats);
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
   RR_UNUSED_VARIABLE(stats);
   GDrawHandle *t;
   S32 pitch = gdraw->rt_pitch;
   S32 size = pitch * gdraw->frametex_height;

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

   void *ptr = gdraw_arena_alloc(&gdraw->rt_arena, size, 1);
   if (!ptr) {
      IggyGDrawSendWarning(nullptr, "GDraw rendertarget allocation failed: out of rendertarget texture memory");
      gdraw_HandleCacheAllocateFail(t);
      return nullptr;
   }

   t->fence = get_next_fence(); // we're about to start using it immediately, so...
   t->raw_ptr = nullptr;

   t->handle.tex.gcm_ptr = ptr;

   GcmTexture *tex = t->handle.tex.gcm;
   tex->offset = CELL_GCM_METHOD_DATA_TEXTURE_OFFSET(addr2offs(t->handle.tex.gcm_ptr));
   tex->format = CELL_GCM_METHOD_DATA_TEXTURE_FORMAT(gdraw->rt_loc, 0, CELL_GCM_TEXTURE_DIMENSION_2,
      CELL_GCM_TEXTURE_A8R8G8B8 | CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_NR, 1);
   tex->remap = TEXREMAP(CHAN_A, REMAP, FROM_A) | TEXREMAP(CHAN_R, REMAP, FROM_R) | TEXREMAP(CHAN_G, REMAP, FROM_G) | TEXREMAP(CHAN_B, REMAP, FROM_B);
   tex->imagerect = CELL_GCM_METHOD_DATA_TEXTURE_IMAGE_RECT(gdraw->frametex_height, gdraw->frametex_width);
   tex->control3 = CELL_GCM_METHOD_DATA_TEXTURE_CONTROL3(pitch, 1);
   tex->width = gdraw->frametex_width;
   tex->height = gdraw->frametex_height;
   tex->pitch = pitch;

   gdraw_HandleCacheAllocateEnd(t, size, (void *) 1, GDRAW_HANDLE_STATE_locked);
   return t;
}

////////////////////////////////////////////////////////////////////////
//
//   Rendering helpers
//

static void set_rsx_texture(CellGcmContextData * RADRESTRICT gcm, U32 sampler, GcmTexture * RADRESTRICT tex, U32 wrap, U32 nearest)
{
   static const U32 addrmodes[] = {
      // GDRAW_WRAP_clamp
      CELL_GCM_METHOD_DATA_TEXTURE_ADDRESS(CELL_GCM_TEXTURE_CLAMP_TO_EDGE, CELL_GCM_TEXTURE_CLAMP_TO_EDGE,  CELL_GCM_TEXTURE_CLAMP_TO_EDGE, CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL, CELL_GCM_TEXTURE_ZFUNC_NEVER, 0, 0),
      // GDRAW_WRAP_repeat
      CELL_GCM_METHOD_DATA_TEXTURE_ADDRESS(CELL_GCM_TEXTURE_WRAP,          CELL_GCM_TEXTURE_WRAP,           CELL_GCM_TEXTURE_CLAMP_TO_EDGE, CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL, CELL_GCM_TEXTURE_ZFUNC_NEVER, 0, 0),
      // GDRAW_WRAP_mirror
      CELL_GCM_METHOD_DATA_TEXTURE_ADDRESS(CELL_GCM_TEXTURE_MIRROR,        CELL_GCM_TEXTURE_MIRROR,         CELL_GCM_TEXTURE_CLAMP_TO_EDGE, CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL, CELL_GCM_TEXTURE_ZFUNC_NEVER, 0, 0),
   };

   static const U32 filtermodes[] = {
      CELL_GCM_METHOD_DATA_TEXTURE_FILTER(0, CELL_GCM_TEXTURE_LINEAR_LINEAR,  CELL_GCM_TEXTURE_LINEAR,   CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX),
      CELL_GCM_METHOD_DATA_TEXTURE_FILTER(0, CELL_GCM_TEXTURE_LINEAR_LINEAR,  CELL_GCM_TEXTURE_NEAREST,  CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX),
   };

   assert(wrap < sizeof(addrmodes) / sizeof(addrmodes[0]));
   assert(nearest < sizeof(filtermodes) / sizeof(filtermodes[0]));

   CommandData * RADRESTRICT cmd = put_command(gcm, 11);
   cmd->w0 = CELL_GCM_METHOD_HEADER_TEXTURE_OFFSET(sampler, 8);
   cmd->w1 = tex->offset;
   cmd->w2 = tex->format;
   cmd->w3 = addrmodes[wrap];
   cmd->w4 = CELL_GCM_METHOD_DATA_TEXTURE_CONTROL0(1, 0, 12<<8, 0);
   cmd->w5 = tex->remap;
   cmd->w6 = filtermodes[nearest];
   cmd->w7 = tex->imagerect;
   cmd->w8 = CELL_GCM_METHOD_DATA_TEXTURE_BORDER_COLOR(0);
   cmd->w9 = CELL_GCM_METHOD_HEADER_TEXTURE_CONTROL3(sampler, 1);
   cmd->wa = tex->control3;
}

static inline void disable_scissor(bool force)
{
   if (force || gdraw->scissor_state) {
      // set whole viewport as scissor test
      gdraw->scissor_state = 0;
      cellGcmSetScissor(gdraw->gcm, gdraw->cview.x, gdraw->cview.y, gdraw->cview.w, gdraw->cview.h);
   }
}

static void set_viewport_raw(S32 x, S32 y, S32 w, S32 h)
{
   float scale[4] = { w*0.5f, -h*0.5f, 0.5f, 0.0f };
   float offset[4] = { x + scale[0], y - scale[1], 0.5f, 0.0f };
   cellGcmSetViewport(gdraw->gcm, x, y, w, h, 0.0f, 1.0f, scale, offset);

   gdraw->cview.x = x;
   gdraw->cview.y = y;
   gdraw->cview.w = w;
   gdraw->cview.h = h;
   disable_scissor(true);
}

static void set_projection_raw(S32 x0, S32 x1, S32 y0, S32 y1)
{
   gdraw->projection[0] = 2.0f / (x1-x0);
   gdraw->projection[1] = 2.0f / (y1-y0);
   gdraw->projection[2] = (x1 + x0) / (F32) (x0 - x1);
   gdraw->projection[3] = (y1 + y0) / (F32) (y0 - y1);
   cellGcmSetVertexProgramConstants(gdraw->gcm, VVAR_viewproj, 4, gdraw->projection);
}

static void set_viewport(void)
{
   if (gdraw->in_blur) {
      set_viewport_raw(gdraw->cview.x, gdraw->cview.y, gdraw->cview.w, gdraw->cview.h);
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

static void set_projection(void)
{
   if (gdraw->in_blur) return;
   if (gdraw->cur == gdraw->frame) // if the render stack is empty
      set_projection_raw(gdraw->tx0,gdraw->tx0+gdraw->tw,gdraw->ty0+gdraw->th,gdraw->ty0);
   else if (gdraw->cur->cached)
      set_projection_raw(gdraw->cur->base_x, gdraw->cur->base_x + gdraw->cur->width, gdraw->cur->base_y + gdraw->cur->height, gdraw->cur->base_y);
   else
      set_projection_raw(gdraw->tx0p,gdraw->tx0p+gdraw->tpw,gdraw->ty0p+gdraw->tph,gdraw->ty0p);
}

static void set_common_renderstate()
{
   CellGcmContextData *gcm = gdraw->gcm;
   S32 i;

   // all the render states we never change while drawing
   cellGcmSetCullFaceEnable(gcm, CELL_GCM_FALSE);
   cellGcmSetCylindricalWrap(gcm, 0, 0);
   cellGcmSetDepthBoundsTestEnable(gcm, CELL_GCM_FALSE);
   cellGcmSetLogicOpEnable(gcm, CELL_GCM_FALSE);
   cellGcmSetPolygonOffset(gcm, 0.0f, 0.0f);
   cellGcmSetPolygonStippleEnable(gcm, CELL_GCM_FALSE);
   cellGcmSetPolySmoothEnable(gcm, CELL_GCM_FALSE);
   cellGcmSetShadeMode(gcm, CELL_GCM_SMOOTH);
   cellGcmSetUserClipPlaneControl(gcm, CELL_GCM_USER_CLIP_PLANE_DISABLE, CELL_GCM_USER_CLIP_PLANE_DISABLE, CELL_GCM_USER_CLIP_PLANE_DISABLE, CELL_GCM_USER_CLIP_PLANE_DISABLE, CELL_GCM_USER_CLIP_PLANE_DISABLE, CELL_GCM_USER_CLIP_PLANE_DISABLE);
   cellGcmSetStencilOp(gcm, CELL_GCM_KEEP, CELL_GCM_KEEP, CELL_GCM_REPLACE);
   cellGcmSetZcullEnable(gcm, 0, 0);
   cellGcmSetTwoSidedStencilTestEnable(gcm, CELL_GCM_FALSE);
   cellGcmSetTwoSideLightEnable(gcm, CELL_GCM_FALSE);
   cellGcmSetClearDepthStencil(gcm, 0xffffff00);
   cellGcmSetClearColor(gcm, 0);
   cellGcmSetBlendEquation(gcm, CELL_GCM_FUNC_ADD, CELL_GCM_FUNC_ADD);
   cellGcmSetColorMask(gcm, COLOR_MASK_ALL);
   cellGcmSetAlphaTestEnable(gcm, CELL_GCM_FALSE);

   // clear all vertex attr inputs
   for (i=0; i < 16; i++)
      cellGcmSetVertexDataArray(gcm, i, 0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);

   cellGcmSetTransferLocation(gcm,CELL_GCM_LOCATION_LOCAL);

   cellGcmSetVertexDataBase(gcm, 0, 0);

   // load all our vertex programs into their respective slots so we can switch between them
   // by just changing the start slot
   for (i=0; i < GDRAW_vformat__basic_count; i++) {
      ProgramWithCachedVariableLocations *p = &gdraw->vprog[i];
      if (p->program)
         cellGcmSetVertexProgram(gcm, p->program, p->ucode);
   }
   cellGcmSetVertexAttribInputMask(gcm, 0x3); // enable position+attr1 input (all we ever use)

   // initialize all our vertex constants to zero
   void *vconst;
   cellGcmSetVertexProgramConstantsPointer(gcm, 0, VVAR_count, &vconst);
   memset(vconst, 0, 4 * sizeof(F32) * VVAR_count);

   // reset our state caching
   for (i=0; i < MAX_SAMPLERS; i++) {
      gdraw->active_tex[i] = nullptr;
      cellGcmSetTextureControl(gcm, i, CELL_GCM_FALSE, 0, 0, 0);
   }

   assert(gdraw->aa_tex.offset != 0); // if you hit this, your initialization is screwed up.
   set_rsx_texture(gdraw->gcm, AATEX_SAMPLER, &gdraw->aa_tex, GDRAW_WRAP_clamp, 0);

   gdraw->cur_fprog = nullptr;
   gdraw->vert_format = -1;
   gdraw->scissor_state = ~0u;
   gdraw->blend_mode = -1;
   gdraw->stencil_key = ~0u;
   gdraw->z_key = ~0u;
}

static void clear_renderstate(void)
{
   CellGcmContextData *gcm = gdraw->gcm;
   cellGcmSetStencilTestEnable(gcm, CELL_GCM_FALSE);
   cellGcmSetColorMask(gcm, COLOR_MASK_ALL);
   cellGcmSetDepthTestEnable(gcm, CELL_GCM_FALSE);
   cellGcmSetDepthFunc(gcm, CELL_GCM_LESS);
   cellGcmSetDepthMask(gcm, CELL_GCM_FALSE);
   cellGcmSetBlendEnable(gcm, CELL_GCM_FALSE);
   disable_scissor(false);

   gdraw->scissor_state = 0;
   gdraw->blend_mode = GDRAW_BLEND_none;
   gdraw->stencil_key = 0;
   gdraw->z_key = 0;
}

static void set_render_target()
{
   GcmTexture *tex = nullptr;
   S32 i;
   CellGcmSurface surf = gdraw->main_surface;

   if (gdraw->cur->color_buffer)
      tex = gdraw->cur->color_buffer->handle.tex.gcm;

   if (tex) {
      surf.colorLocation[0] = gdraw->rt_loc;
      surf.colorOffset[0] = tex->offset;
      surf.colorPitch[0] = tex->pitch;
      surf.x = 0;
      surf.y = 0;
      surf.width = tex->width;
      surf.height = tex->height;
   }
   
   cellGcmSetSurface(gdraw->gcm, &surf);

   // invalidate current textures (need to reset them to force L1 texture cache flush)
   for (i=0; i < MAX_SAMPLERS; ++i)
      gdraw->active_tex[i] = nullptr;
}

////////////////////////////////////////////////////////////////////////
//
//   Begin rendering for a frame
//

void gdraw_GCM_SetTileOrigin(CellGcmSurface *surf, S32 x, S32 y)
{
   assert(surf->colorFormat == CELL_GCM_SURFACE_A8R8G8B8);
   assert(surf->depthFormat == CELL_GCM_SURFACE_Z24S8);

   gdraw->main_surface = *surf;
   gdraw->vx = x;
   gdraw->vy = y;

   for (int i=1; i<4; i++) {
      gdraw->main_surface.colorLocation[i] = CELL_GCM_LOCATION_LOCAL;
      gdraw->main_surface.colorOffset[i] = 0;
      gdraw->main_surface.colorPitch[i] = 64;
   }
}

static void RADLINK gdraw_SetViewSizeAndWorldScale(S32 w, S32 h, F32 scalex, F32 scaley)
{
   memset(gdraw->frame, 0, sizeof(gdraw->frame));
   gdraw->cur = gdraw->frame;
   gdraw->fw = w;
   gdraw->fh = h;
   gdraw->tw = w;
   gdraw->th = h;
   gdraw->world_to_pixel[0] = scalex;
   gdraw->world_to_pixel[1] = scaley;
   set_viewport();
}

// must include anything necessary for texture creation/update
static void RADLINK gdraw_RenderingBegin(void)
{
   set_common_renderstate();  
}

static void RADLINK gdraw_RenderingEnd(void)
{
   clear_renderstate();
}

static void RADLINK gdraw_RenderTileBegin(S32 x0, S32 y0, S32 x1, S32 y1, S32 pad, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(stats);
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
   // if you hit this assert, the rendertarget_width / rendertarget_height
   // in your gdraw_GCM_MemoryConfig is too small!
   assert(gdraw->tpw <= gdraw->frametex_width && gdraw->tph <= gdraw->frametex_height);

   // need to clear both the requested area of the viewport
   // *and* the area the rendertargets will use... ideally
   // we'd work out the minimal rectangles to clear, or just
   // always clear the whole thing
   set_render_target();
   set_viewport_raw(0, 0, gdraw->tpw, gdraw->tph);
   cellGcmSetStencilMask(gdraw->gcm, 0xff);
   cellGcmSetClearSurface(gdraw->gcm, CELL_GCM_CLEAR_Z | CELL_GCM_CLEAR_S);

   set_viewport();
   set_projection();

   // if the first clear didn't get the actual viewport region, clear it
   if (gdraw->tpw < gdraw->vx+gdraw->tw || gdraw->tph < gdraw->vy+gdraw->th)
      cellGcmSetClearSurface(gdraw->gcm, CELL_GCM_CLEAR_Z | CELL_GCM_CLEAR_S);
}

static void RADLINK gdraw_RenderTileEnd(GDrawStats *stats)
{
   gdraw->tile_end_fence = put_fence();

   // reap once per frame even if there are no allocs
   gdraw_res_reap(gdraw->texturecache, stats);
   gdraw_res_reap(gdraw->vbufcache, stats);
}

void gdraw_GCM_NoMoreGDrawThisFrame(void)
{
   gdraw_HandleCacheTick(gdraw->texturecache, gdraw->tile_end_fence);
   gdraw_HandleCacheTick(gdraw->vbufcache, gdraw->tile_end_fence);
}

#define MAX_DEPTH_VALUE (1 << 13)

static void RADLINK gdraw_GetInfo(GDrawInfo *d)
{
   d->num_stencil_bits = 8;
   d->max_id = MAX_DEPTH_VALUE-2;
   // for floating point depth, just use mantissa, e.g. 16-20 bits
   d->max_texture_size = 4096;
   d->buffer_format = GDRAW_BFORMAT_vbib;
   d->shared_depth_stencil = 1;
   d->always_mipmap = 0;
   d->conditional_nonpow2 = 0;
}

////////////////////////////////////////////////////////////////////////
//
//   Enable/disable rendertargets in stack fashion
//

static rrbool RADLINK gdraw_TextureDrawBufferBegin(gswf_recti *region, gdraw_texture_format format, U32 flags, void *owner, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(format);
   RR_UNUSED_VARIABLE(flags);

   GDrawFramebufferState *n = gdraw->cur+1;
   GDrawHandle *t;
   if (gdraw->tw == 0 || gdraw->th == 0) {
      IggyGDrawSendWarning(nullptr, "GDraw warning: w=0,h=0 rendertarget");
      return false;
   }

   if (n >= &gdraw->frame[MAX_RENDER_STACK_DEPTH]) {
      assert(0);
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
   int k = t - gdraw->rendertargets.handle;

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
      S32 x = RR_MAX(xt0 - pad, 0);
      S32 y = RR_MAX(yt0 - pad, 0);
      S32 w = RR_MIN(xt1 + pad, gdraw->frametex_width) - x;
      S32 h = RR_MIN(yt1 + pad, gdraw->frametex_height) - y;

      if (w <= 0 || h <= 0) { // region doesn't intersect with current tile
         gdraw_FreeTexture((GDrawTexture *) t, 0, stats);
         // note: don't send a warning since this will happen during regular tiled rendering
         return false;
      }

      if (!gdraw->in_blur)
         set_viewport_raw(x, y, w, h); // not strictly necessary (we only use this for a clear), just to avoid GPAD warnings

      cellGcmSetScissor(gdraw->gcm, x, y, w, h);
      gdraw->rt_valid[k].x0 = xt0;
      gdraw->rt_valid[k].y0 = yt0;
      gdraw->rt_valid[k].x1 = xt1;
      gdraw->rt_valid[k].y1 = yt1;
   } else {
      gdraw->rt_valid[k].x0 = 0;
      gdraw->rt_valid[k].y0 = 0;
      gdraw->rt_valid[k].x1 = gdraw->frametex_width;
      gdraw->rt_valid[k].y1 = gdraw->frametex_height;
   }

   // wait for all currently queued draw commands to finish reading textures
   // (in case a draw command is still using this rendertarget as a texture)
   GDrawFence fence = put_fence();
   cellGcmFlush(gdraw->gcm);
   cellGcmSetWaitLabel(gdraw->gcm, gdraw->fence_label_index, (U32) fence.value);

   ++gdraw->cur;
   set_render_target();
   cellGcmSetClearSurface(gdraw->gcm, CELL_GCM_CLEAR_R | CELL_GCM_CLEAR_G | CELL_GCM_CLEAR_B | CELL_GCM_CLEAR_A);
   set_viewport();
   set_projection();

   return true;
}

static GDrawTexture *RADLINK gdraw_TextureDrawBufferEnd(GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(stats);
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

   // wait for render to texture operation to finish
   // can't put down a backend fence directly, there might still be
   // a texture fence pending that completes after the backend fence
   // (can this really happen? not sure, but better safe than sorry).
   // 
   // so: wait for completion of pending texture fence, then put down
   // backend label, then wait on that.

   cellGcmSetWaitLabel(gdraw->gcm, gdraw->fence_label_index, U32(gdraw->next_fence_index - 1));
   GDrawFence fence = put_backend_fence();
   cellGcmFlush(gdraw->gcm);
   cellGcmSetWaitLabel(gdraw->gcm, gdraw->fence_label_index, (U32) fence.value);
   cellGcmSetInvalidateTextureCache(gdraw->gcm, CELL_GCM_INVALIDATE_TEXTURE);
   n->color_buffer->fence = fence;

   // switch back to old rendertarget
   set_render_target();
   set_viewport();
   set_projection();

   return (GDrawTexture *) n->color_buffer;
}

////////////////////////////////////////////////////////////////////////
//
//   Clear stencil/depth buffers
//
// Open question whether we'd be better off finding bounding boxes
// and only clearing those; it depends exactly how fast clearing works.
//

static void RADLINK gdraw_ClearStencilBits(U32 bits)
{
   // @TODO: actually only clear 'bits', not everything
   RR_UNUSED_VARIABLE(bits);
   cellGcmSetStencilMask(gdraw->gcm, 0xff);
   cellGcmSetClearSurface(gdraw->gcm, CELL_GCM_CLEAR_S);
   gdraw->stencil_key = ~0u; // invalid
}

// this only happens rarely (hopefully never) if we use the depth buffer,
// so we can just clear the whole thing
static void RADLINK gdraw_ClearID(void)
{
   cellGcmSetClearSurface(gdraw->gcm, CELL_GCM_CLEAR_Z);
}

////////////////////////////////////////////////////////////////////////
//
//   Fragment shader setting / patching
//

static RADINLINE U64 microcode_endian_swap(U64 v)
{
   U64 lo = v & 0x0000ffff0000ffffull;
   U64 hi = v ^ lo;

   return (lo << 16) | (hi >> 16);
}

static void set_fragment_shader(GDraw * RADRESTRICT gd, ProgramWithCachedVariableLocations *prg)
{
   if (prg != gd->cur_fprog) {
      gd->cur_fprog = prg;
      cellGcmSetFragmentProgramLoad(gd->gcm, &prg->cfg);
   }
}

static void set_fragment_para(GDraw * RADRESTRICT gd, U32 ucode_offs, int para, const void *values, int count)
{
   if (para == -1)
      return;

   gd->cur_fprog = nullptr; // need to re-set shader after patching

   const U64 *inv = (const U64 *) values;
   const int *patch_offs = (const int *) para;
   int offs = *patch_offs;
   void *ptr;

   while (count--) {
      U64 v0 = microcode_endian_swap(*inv++);
      U64 v1 = microcode_endian_swap(*inv++);

      do {
         cellGcmSetInlineTransferPointer(gd->gcm, ucode_offs + offs, 4, &ptr);
         U64 * RADRESTRICT p64 = (U64 *) ptr;
         p64[0] = v0;
         p64[1] = v1;
      } while ((offs = *++patch_offs) != -1);

      offs = *++patch_offs;
   }
}

////////////////////////////////////////////////////////////////////////
//
//   Set all the render state from GDrawRenderState
//
// This also is responsible for getting the framebuffer into a texture
// if the read-modify-write blend operation can't be expressed with
// the native blend operators. (E.g. "screen")
//

static RADINLINE void set_texture(S32 texunit, GDrawTexture *tex)
{
   assert(texunit < MAX_SAMPLERS);
   assert(tex != nullptr);

   if (gdraw->active_tex[texunit] != tex) {
      gdraw->active_tex[texunit] = tex;
      GDrawHandle *h = (GDrawHandle *) tex;
      set_rsx_texture(gdraw->gcm, texunit, h->handle.tex.gcm, GDRAW_WRAP_clamp, 0);
   }
}

#define MAKEBLEND(src,dst) ((src << 16) | src), ((dst << 16) | dst)

static struct gdraw_gcm_blendspec {
   U32 enable;
   U32 src;
   U32 dst;
   U32 _pad;
} blend_states[] = {
   { 0, MAKEBLEND(CELL_GCM_ONE, CELL_GCM_ZERO), 0 },                       // GDRAW_BLEND_none
   { 1, MAKEBLEND(CELL_GCM_ONE, CELL_GCM_ONE_MINUS_SRC_ALPHA), 0 },        // GDRAW_BLEND_alpha
   { 1, MAKEBLEND(CELL_GCM_DST_COLOR, CELL_GCM_ONE_MINUS_SRC_ALPHA), 0 },  // GDRAW_BLEND_multiply
   { 1, MAKEBLEND(CELL_GCM_ONE, CELL_GCM_ONE), 0 },                        // GDRAW_BLEND_add

   { 0, MAKEBLEND(CELL_GCM_ONE, CELL_GCM_ZERO), 0 },                       // GDRAW_BLEND_filter
   { 0, MAKEBLEND(CELL_GCM_ONE, CELL_GCM_ZERO), 0 },                       // GDRAW_BLEND_special
};

#undef MAKEBLEND

// converts a depth id into a Z value
static RADINLINE F64 depth_from_id(S64 id)
{
#ifndef __SNC__
   return (1.0 - 1.0 / (F64) MAX_DEPTH_VALUE) - __fcfid(id) * (1.0 / (F64) MAX_DEPTH_VALUE); // = 1 - (id + 1) / MAX_DEPTH_VALUE
#else
   return (1.0 - 1.0 / (F64) MAX_DEPTH_VALUE) - id * (1.0 / (F64) MAX_DEPTH_VALUE); // = 1 - (id + 1) / MAX_DEPTH_VALUE
#endif
}

#define copy_vec4(d,s) { F32 x = s[0]; F32 y = s[1]; F32 z = s[2]; F32 w = s[3]; d.x = x; d.y = y; d.z = z; d.w = w; }

static void set_renderstate_full(U32 vertex_slot, const GDrawRenderState * RADRESTRICT r, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(stats);
   GDraw * RADRESTRICT gd = gdraw;
   CellGcmContextData * RADRESTRICT gcm = gd->gcm;
   volatile VertexVars * RADRESTRICT vvars;
   ProgramWithCachedVariableLocations *fprog;
#ifndef __SNC__
   volatile S64 depth_id = r->id; // load and sign extend here to avoid an LHS stall
#else
   S64 depth_id = r->id;
#endif

   // prepare commands for vertex program switch + constant upload
   int nconstants = (r->texgen0_enabled ? VVAR_count : VVAR_count_world_and_color) * 4;

   CommandData * RADRESTRICT cmd = put_command(gcm,
      2 +                  // set vertex program start slot
      (2 + nconstants));   // vertex constant loads

   // set vertex program start slot
   cmd->w0 = CELL_GCM_METHOD(CELL_GCM_NV4097_SET_TRANSFORM_PROGRAM_START, 1);
   cmd->w1 = vertex_slot;

   // update vertex shader constants
   cmd->w2 = CELL_GCM_METHOD(CELL_GCM_NV4097_SET_TRANSFORM_CONSTANT_LOAD, nconstants + 1);
   cmd->w3 = 0; // load starting from constant 0
   void *vvars_ptr = &cmd->w4; // vertex constants start here
   vvars = (volatile VertexVars *) vvars_ptr;

   int texgen0 = r->texgen0_enabled;
   int tex0mode = r->tex0_mode;
   int blend_mode = r->blend_mode;

   if (!r->use_world_space)
      gdraw_ObjectSpace(&vvars->world[0].x, r->o2w, depth_from_id(depth_id), 0.0f);
   else
      gdraw_WorldSpace(&vvars->world[0].x, gd->world_to_pixel, depth_from_id(depth_id), 0.0f);

   copy_vec4(vvars->x_off, r->edge_matrix);
   copy_vec4(vvars->color_mul, r->color);

   if (texgen0) {
      copy_vec4(vvars->s0_texgen, r->s0_texgen);
      copy_vec4(vvars->t0_texgen, r->t0_texgen);
   }

   // set the blend mode
   assert(blend_mode != GDRAW_BLEND_filter); // shouldn't come through this path
   assert(blend_mode >= 0 && blend_mode < (int) (sizeof(blend_states)/sizeof(*blend_states)));
   gdraw_gcm_blendspec *blend = &blend_states[blend_mode];
   if (blend_mode != gd->blend_mode) {
      gd->blend_mode = blend_mode;
      cmd = reserve_command(gcm, 4); // must be enough for both cases!

      if (blend->enable) {
         cmd->w0 = CELL_GCM_METHOD(CELL_GCM_NV4097_SET_BLEND_ENABLE, 3);
         cmd->w1 = 1; // enable
         cmd->w2 = blend->src; // src factor
         cmd->w3 = blend->dst; // dst factor
         gcm->current = &cmd->w4;
      } else {
         cmd->w0 = CELL_GCM_METHOD(CELL_GCM_NV4097_SET_BLEND_ENABLE, 1);
         cmd->w1 = 0;
         gcm->current = &cmd->w2;
      }
   }

   // set the fragment program
   if (blend_mode != GDRAW_BLEND_special) {
      fprog = gd->basic_fprog[tex0mode];

      if (r->cxf_add) {
         fprog++; // additive
         if (r->cxf_add[3]) fprog++; // additive alpha
      }
   } else
      fprog = &gd->exceptional_blend[r->special_blend];

   // set textures
   if (tex0mode != GDRAW_TEXTURE_none) {
      if (!r->tex[0]) // this can happen if some allocs fail. the rendered image will be invalid and we don't care.
         return;

      if (gd->active_tex[0] != r->tex[0]) {
         gd->active_tex[0] = r->tex[0];
         set_rsx_texture(gcm, 0, ((GDrawHandle *) r->tex[0])->handle.tex.gcm, r->wrap0, r->nearest0);
      }
   }

   // Set pixel shader and constants
   if (tex0mode == GDRAW_TEXTURE_focal_gradient)
      set_fragment_para(gd, fprog->cfg.offset, fprog->vars[VAR_focal], r->focal_point, 1);

   if (r->cxf_add) {
      float temp[4] = { r->cxf_add[0]/255.0f, r->cxf_add[1]/255.0f, r->cxf_add[2]/255.0f, r->cxf_add[3]/255.0f };
      set_fragment_para(gd, fprog->cfg.offset, fprog->vars[VAR_cadd], temp, 1);
   }

   set_fragment_shader(gd, fprog);

   // Set pixel operation states
   if (r->scissor) {
      int x,y,w,h,xs,ys;
      if (gd->cur == gd->frame) {
         xs = gd->tx0 - gd->vx;
         ys = gd->ty0 - gd->vy;
      } else {
         xs = gd->tx0p;
         ys = gd->ty0p;
      }

      // clip against viewport
      x = RR_MAX(r->scissor_rect.x0 - xs, gd->cview.x);
      y = RR_MAX(r->scissor_rect.y0 - ys, gd->cview.y);
      w = RR_MIN(r->scissor_rect.x1 - xs, gd->cview.x + gd->cview.w) - x;
      h = RR_MIN(r->scissor_rect.y1 - ys, gd->cview.y + gd->cview.h) - y;

      if (w <= 0 || h <= 0) {
         // dummy scissor rect in case our actual scissor is empty
         x = gd->cview.x;
         y = gd->cview.y;
         w = h = 0;
      }

      cellGcmSetScissorInline(gcm, x, y, w, h);
      gd->scissor_state = 1;
   } else if (r->scissor != gd->scissor_state)
      disable_scissor(0);

   // stencil changed?
   U32 stencil_key = r->stencil_test | (r->stencil_set << 8);
   if (stencil_key != gd->stencil_key) {
      gd->stencil_key = stencil_key;

      // it just so happens that all the states we want to set are in a contiguous method index range!
      cmd = put_command(gcm, 7);
      cmd->w0 = CELL_GCM_METHOD(CELL_GCM_NV4097_SET_COLOR_MASK, 6);
      cmd->w1 = r->stencil_set ? 0 : COLOR_MASK_ALL;           // COLOR_MASK
      cmd->w2 = (r->stencil_set | r->stencil_test) != 0;       // STENCIL_TEST_ENABLE
      cmd->w3 = r->stencil_set;                                // STENCIL_MASK
      cmd->w4 = CELL_GCM_EQUAL;                                // STENCIL_FUNC
      cmd->w5 = 0xff;                                          // STENCIL_FUNC_REF
      cmd->w6 = r->stencil_test;                               // STENCIL_FUNC_MASK
   }
   
   // z mode changed?
   U32 z_key = r->set_id | (r->test_id << 1);
   if (z_key != gd->z_key) {
      gd->z_key = z_key;

      cmd = put_command(gcm, 4);
      cmd->w0 = CELL_GCM_METHOD(CELL_GCM_NV4097_SET_DEPTH_FUNC, 3);
      cmd->w1 = r->test_id ? CELL_GCM_LESS : CELL_GCM_EQUAL;   // DEPTH_FUNC
      cmd->w2 = r->set_id;                                     // DEPTH_MASK
      cmd->w3 = r->test_id | r->set_id;                        // DEPTH_TEST_ENABLE
   }
}

static RADINLINE void set_renderstate(U32 vertex_slot, const GDrawRenderState * RADRESTRICT r, GDrawStats *stats)
{
   if (r->identical_state) {
      // may need to switch vertex shader, but that's it - very quick to set up.
      CommandData * RADRESTRICT cmd = put_command(gdraw->gcm, 2);
      cmd->w0 = CELL_GCM_METHOD(CELL_GCM_NV4097_SET_TRANSFORM_PROGRAM_START, 1);
      cmd->w1 = vertex_slot;
   } else
      set_renderstate_full(vertex_slot, r, stats);
}

////////////////////////////////////////////////////////////////////////
//
//   Vertex formats
//

struct gdraw_gcm_vformat_desc {
   int size;
   int tex_offs;
   U32 format0;
   U32 format1;
} vformats[GDRAW_vformat__basic_count] = {
   // sz  toffs   stride0     size0     type0                stride1    size1     type1
   {  8,      0, ( 8 << 8) | (2 << 4) | CELL_GCM_VERTEX_F, ( 0 << 8) | (0 << 4) | CELL_GCM_VERTEX_F    }, // GDRAW_vformat_v2
   { 16,      8, (16 << 8) | (2 << 4) | CELL_GCM_VERTEX_F, (16 << 8) | (4 << 4) | CELL_GCM_VERTEX_S32K }, // GDRAW_vformat_v2aa
   { 16,      8, (16 << 8) | (2 << 4) | CELL_GCM_VERTEX_F, (16 << 8) | (2 << 4) | CELL_GCM_VERTEX_F    }, // GDRAW_vformat_v2tc2
};

static RADINLINE void set_vertex_decl(GDraw * RADRESTRICT gd, CellGcmContextData * RADRESTRICT gcm, int format_index, int location, U32 vertaddr)
{
   assert(format_index >= 0 && format_index < (int) (sizeof(vformats)/sizeof(*vformats)));

   CommandData * RADRESTRICT cmd = reserve_command(gcm, 6); // must be enough for the worst case!
   cmd->w0 = CELL_GCM_METHOD(CELL_GCM_NV4097_SET_VERTEX_DATA_ARRAY_OFFSET, 2);
   cmd->w1 = CELL_GCM_METHOD_DATA_VERTEX_DATA_ARRAY_OFFSET(location, vertaddr + 0); // offset for attr 0
   cmd->w2 = CELL_GCM_METHOD_DATA_VERTEX_DATA_ARRAY_OFFSET(location, vertaddr + 8); // offset for attr 1

   if (format_index != gd->vert_format) {
      gd->vert_format = format_index;
      gdraw_gcm_vformat_desc *fmt = &vformats[format_index];
      U32 fmt0 = fmt->format0;
      U32 fmt1 = fmt->format1;
      
      cmd->w3 = CELL_GCM_METHOD(CELL_GCM_NV4097_SET_VERTEX_DATA_ARRAY_FORMAT, 2);
      cmd->w4 = fmt0;
      cmd->w5 = fmt1;
      gcm->current = &cmd->w6;
   } else
      gcm->current = &cmd->w3;
}

////////////////////////////////////////////////////////////////////////
//
//   Draw triangles with a given renderstate
//

static RADINLINE void fence_resources(GDraw * RADRESTRICT gd, void *r1, void *r2=nullptr, void *r3=nullptr, void *r4=nullptr)
{
   GDrawFence fence;
   fence.value = gd->next_fence_index;

   if (r1) ((GDrawHandle *) r1)->fence = fence;
   if (r2) ((GDrawHandle *) r2)->fence = fence;
   if (r3) ((GDrawHandle *) r3)->fence = fence;
   if (r4) ((GDrawHandle *) r4)->fence = fence;

   if (--gd->fence_batch_counter == 0)
      put_fence();
}

static U32 vprog(S32 vertex_format, GDrawRenderState *r)
{
   RR_UNUSED_VARIABLE(r);
   return gdraw->vslot[vertex_format];
}

static void RADLINK gdraw_DrawIndexedTriangles(GDrawRenderState *r, GDrawPrimitive *p, GDrawVertexBuffer *buf, GDrawStats * RADRESTRICT stats)
{
   GDrawHandle *vb = (GDrawHandle *) buf;
   GDraw * RADRESTRICT gd = gdraw;
   CellGcmContextData * RADRESTRICT gcm = gd->gcm;
   S32 vformat = p->vertex_format;

   set_renderstate(vprog(vformat, r), r, stats);

   if (vb) {
      U32 vertaddr = vbufaddr2offs(gd, vb->handle.vbuf.verts) + (U32) (UINTa) p->vertices;
      U32 indaddr = vbufaddr2offs(gd, vb->handle.vbuf.inds) + (U32) (UINTa) p->indices;
      int loc = gd->vbuf_loc;
      set_vertex_decl(gd, gcm, vformat, loc, vertaddr);
      cellGcmSetDrawIndexArray(gcm, CELL_GCM_PRIMITIVE_TRIANGLES, p->num_indices, CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16, loc, indaddr);
   } else if (p->indices) {
      S32 vertex_size = p->num_vertices * vformats[vformat].size;
      S32 index_size = p->num_indices * 2;
      U8 * RADRESTRICT ring_data = (U8 *) gdraw_bufring_alloc(&gd->dyn_vb, vertex_size + index_size, CELL_GCM_VERTEX_TEXTURE_CACHE_LINE_SIZE);

      if (ring_data) { // normal case: fits inside ring buffer
         memcpy(ring_data, p->vertices, vertex_size);
         memcpy(ring_data + vertex_size, p->indices, index_size);

         cellGcmSetInvalidateVertexCache(gcm);

         U32 vertaddr = dynvbaddr2offs(gd, ring_data);
         U32 indaddr = vertaddr + vertex_size;
         int loc = gd->dynvb_loc;
         set_vertex_decl(gd, gcm, vformat, loc, vertaddr); 
         cellGcmSetDrawIndexArray(gcm, CELL_GCM_PRIMITIVE_TRIANGLES, p->num_indices, CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16, loc, indaddr);
      } else {
         // convert it into a non-indexed triangle list that we can chunk without extra mem
         // this is a fall-back path and, needless to say, it's slow.
         S32 vsize = vformats[p->vertex_format].size;
         S32 tris_per_chunk = gdraw->dyn_vb.seg_size / (3 * vsize);
         S32 verts_per_chunk = tris_per_chunk * 3;
         U16 * RADRESTRICT inds = (U16 *) p->indices;
         S32 pos = 0, i;

         while (pos < p->num_indices) {
            S32 vert_count = RR_MIN(p->num_indices - pos, verts_per_chunk);
            void *ring = gdraw_bufring_alloc(&gd->dyn_vb, vert_count * vsize, CELL_GCM_VERTEX_TEXTURE_CACHE_LINE_SIZE);
            assert(ring != nullptr); // we specifically chunked so this alloc succeeds!

            // prepare for painting...
            cellGcmSetInvalidateVertexCache(gcm);
            set_vertex_decl(gd, gcm, p->vertex_format, gd->dynvb_loc, dynvbaddr2offs(gd, ring));

            // build the triangle list (two versions for the two sizes since it's a small number of bytes being copied)
            if (vsize == 8) {
               ring = (void *) ((U8 *) ring - 8);
               for (i=0; i < vert_count; i++)
                  __stdu(*((U64 *) p->vertices + *inds++), 8, ring);
            } else if (vsize == 16) {
               ring = (void *) ((U8 *) ring - 8);
               for (i=0; i < vert_count; i++) {
                  U64 *src = (U64 *) p->vertices + *inds++ * 2;
                  U64 v0 = __ld(0, src);
                  U64 v1 = __ld(8, src);
                  __std(v0, 8, ring);
                  __stdu(v1, 16, ring);
               }
            } else
               assert(0); // unknown size, shouldn't happen

            // paint this batch
            cellGcmSetDrawArrays(gcm, CELL_GCM_PRIMITIVE_TRIANGLES, 0, vert_count);
            pos += vert_count;
         }
      }
   } else { // dynamic quads
      U32 vsize = vformats[p->vertex_format].size;
      S32 verts_per_chunk = (gdraw->dyn_vb.seg_size / vsize) & ~3;
      S32 pos = 0;

      while (pos < p->num_vertices) {
         S32 vert_count = RR_MIN(p->num_vertices - pos, verts_per_chunk);
         U32 chunk_bytes = vert_count * vsize;
         void *ring = gdraw_bufring_alloc(&gd->dyn_vb, chunk_bytes, CELL_GCM_VERTEX_TEXTURE_CACHE_LINE_SIZE);
         assert(ring != nullptr); // we picked the chunk size so this alloc succeeds!

         memcpy(ring, (U8 *)p->vertices + pos * vsize, chunk_bytes);
         cellGcmSetInvalidateVertexCache(gcm);
         set_vertex_decl(gd, gcm, p->vertex_format, gd->dynvb_loc, dynvbaddr2offs(gd, ring));
         cellGcmSetDrawArrays(gcm, CELL_GCM_PRIMITIVE_QUADS, 0, vert_count);
         pos += vert_count;
      }
   }

   fence_resources(gd, vb, r->tex[0], r->tex[1]);

   {
      // avoid LHS
      S16 oldflags = stats->nonzero_flags;
      S16 oldbatches = stats->num_batches;

      stats->nonzero_flags = oldflags | GDRAW_STATS_batches;
      stats->num_batches = oldbatches + 1;
      stats->drawn_indices += p->num_indices;
      stats->drawn_vertices += p->num_vertices;
   }
}

///////////////////////////////////////////////////////////////////////
//
//   Flash 8 filter effects
//

static void set_pixel_constant(U32 ucode_offs, S32 constant, F32 x, F32 y, F32 z, F32 w)
{
   F32 values[4] = { x,y,z,w };
   set_fragment_para(gdraw, ucode_offs, constant, values, 1);
}

// caller sets up texture coordinates
static void do_screen_quad(gswf_recti *s, F32 *tc, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(stats);

   F32 px0 = (F32) s->x0, py0 = (F32) s->y0, px1 = (F32) s->x1, py1 = (F32) s->y1;
   CellGcmContextData * RADRESTRICT gcm = gdraw->gcm;
   VertexVars * RADRESTRICT vvars;
   volatile F32 * RADRESTRICT vert;
   void *ptr;

   cellGcmSetVertexProgramStartSlot(gcm, gdraw->vslot[GDRAW_vformat_v2tc2]);
   cellGcmSetVertexProgramConstantsPointer(gcm, 0, VVAR_count_worldonly * 4, &ptr);
   vvars = (VertexVars *) ptr;
   gdraw_PixelSpace(&vvars->world[0].x);

   set_vertex_decl(gdraw, gcm, GDRAW_vformat_v2tc2, CELL_GCM_LOCATION_LOCAL, 0);
   cellGcmSetDrawBegin(gcm, CELL_GCM_PRIMITIVE_QUADS);
   cellGcmSetDrawInlineArrayPointer(gcm, 4 * 4, &ptr);
   
   // build vertex data
   vert = (volatile F32 *) ptr;
   vert[ 0] = px0; vert[ 1] = py0; vert[ 2] = tc[0]; vert[ 3] = tc[1];
   vert[ 4] = px1; vert[ 5] = py0; vert[ 6] = tc[2]; vert[ 7] = tc[1];
   vert[ 8] = px1; vert[ 9] = py1; vert[10] = tc[2]; vert[11] = tc[3];
   vert[12] = px0; vert[13] = py1; vert[14] = tc[0]; vert[15] = tc[3];

   cellGcmSetDrawEnd(gcm);
}

static void gdraw_DriverBlurPass(GDrawRenderState *r, int taps,  float *data, gswf_recti *s, float *tc, float height_max, float *clamp, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(height_max);

   ProgramWithCachedVariableLocations *prg = &gdraw->blur_prog[taps];
   U32 ucode_offs = prg->cfg.offset;

   set_texture(0, r->tex[0]);
   set_fragment_para(gdraw, ucode_offs, prg->vars[VAR_blur_tap], data, taps);
   set_fragment_para(gdraw, ucode_offs, prg->vars[VAR_blur_clampv], clamp, 1);
   set_fragment_shader(gdraw, prg);

   do_screen_quad(s, tc, stats);
   fence_resources(gdraw, r->tex[0]);
}

static void gdraw_Colormatrix(GDrawRenderState *r, gswf_recti *s, float *tc, GDrawStats *stats)
{
   ProgramWithCachedVariableLocations *prg = &gdraw->colormatrix;
   U32 ucode_offs = prg->cfg.offset;
   if (!gdraw_TextureDrawBufferBegin(s, GDRAW_TEXTURE_FORMAT_rgba32, GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_color | GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_alpha, 0, stats))
      return;

   set_texture(0, r->tex[0]);
   set_fragment_para(gdraw, ucode_offs, prg->vars[VAR_colormatrix_data], r->shader_data, 5);
   set_fragment_shader(gdraw, prg);

   do_screen_quad(s, tc, stats);
   fence_resources(gdraw, r->tex[0]);
   r->tex[0] = gdraw_TextureDrawBufferEnd(stats);
}

static gswf_recti *get_valid_rect(GDrawTexture *tex)
{
   GDrawHandle *h = (GDrawHandle *) tex;
   S32 n = (S32) (h - gdraw->rendertargets.handle);
   assert(n >= 0 && n <= MAX_RENDER_STACK_DEPTH+1);
   return &gdraw->rt_valid[n];
}

static void set_clamp_constant(U32 ucode_offs, S32 constant, GDrawTexture *tex)
{
   gswf_recti *s = get_valid_rect(tex);
   // when we make the valid data, we make sure there is an extra empty pixel at the border
   set_pixel_constant(ucode_offs, constant,
      (s->x0-0.5f) / gdraw->frametex_width,
      (s->y0-0.5f) / gdraw->frametex_height,
      (s->x1+0.5f) / gdraw->frametex_width,
      (s->y1+0.5f) / gdraw->frametex_height);
}

static void gdraw_Filter(GDrawRenderState *r, gswf_recti *s, float *tc, int isbevel, GDrawStats *stats)
{
   ProgramWithCachedVariableLocations *prg = &gdraw->filter_prog[isbevel][r->filter_mode];
   U32 ucode_offs = prg->cfg.offset;
   if (!gdraw_TextureDrawBufferBegin(s, GDRAW_TEXTURE_FORMAT_rgba32, GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_color | GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_alpha, nullptr, stats))
      return;

   set_texture(0, r->tex[0]);
   set_texture(1, r->tex[1]);
   if (r->tex[2]) set_texture(2, r->tex[2]);

   set_pixel_constant(ucode_offs, prg->vars[VAR_filter_color], r->shader_data[0], r->shader_data[1], r->shader_data[2], r->shader_data[3]);
   set_pixel_constant(ucode_offs, prg->vars[VAR_filter_tc_off], -r->shader_data[4] / (F32)gdraw->frametex_width, -r->shader_data[5] / (F32)gdraw->frametex_height, r->shader_data[6], 0);
   set_pixel_constant(ucode_offs, prg->vars[VAR_filter_color2], r->shader_data[8], r->shader_data[9], r->shader_data[10], r->shader_data[11]);
   set_clamp_constant(ucode_offs, prg->vars[VAR_filter_clamp0], r->tex[0]);
   set_clamp_constant(ucode_offs, prg->vars[VAR_filter_clamp1], r->tex[1]);
   set_fragment_shader(gdraw, prg);

   do_screen_quad(s, tc, stats);
   fence_resources(gdraw, r->tex[0], r->tex[1], r->tex[2]);
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
            set_viewport_raw(0,0,gdraw->tpw,gdraw->tph);
            set_projection_raw(0,gdraw->tpw,gdraw->tph,0);

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
      GDrawHandle *blend_tex = nullptr;
      GDraw *gd = gdraw;

      // for crazy blend modes, we need to read back from the framebuffer
      // and do the blending in the pixel shader. so we need to copy the
      // relevant pixels from our active render target into a texture.
      if (r->blend_mode == GDRAW_BLEND_special &&
          (blend_tex = get_color_rendertarget(stats)) != nullptr) {
         // slightly different logic depending on whether we were rendering
         // to the main color buffer or a render target, because the former
         // has tile origin-based coordinates while the latter don't. also,
         // we don't have a texture for the main framebuffer lying around.
         int bpp = 4;
         GcmTexture *dest = blend_tex->handle.tex.gcm;

         if (gd->cur == gd->frame) {
            CellGcmSurface *src = &gd->main_surface;
            cellGcmSetTransferImage(gd->gcm, CELL_GCM_TRANSFER_LOCAL_TO_LOCAL,
               dest->offset, dest->pitch, gd->tx0 - gd->tx0p, gd->ty0 - gd->ty0p,
               src->colorOffset[0], src->colorPitch[0], src->x + gd->vx, src->y + gd->vy, gd->tw, gd->th, bpp);
         } else {
            GcmTexture *src = gd->cur->color_buffer->handle.tex.gcm;
            cellGcmSetTransferImage(gd->gcm, CELL_GCM_TRANSFER_LOCAL_TO_LOCAL,
               dest->offset, dest->pitch, 0, 0,
               src->offset, src->pitch, 0, 0, gd->tpw, gd->tph, bpp);
         }

         set_texture(1, (GDrawTexture *) blend_tex);
      }

      set_renderstate(vprog(GDRAW_vformat_v2tc2, r), r, stats);
      do_screen_quad(&s, tc, stats);
      fence_resources(gdraw, r->tex[0], r->tex[1]);
      if (blend_tex)
         gdraw_FreeTexture((GDrawTexture *) blend_tex, 0, stats);
   }
}

///////////////////////////////////////////////////////////////////////
//
//   Shaders
//

#include "gdraw_ps3gcm_shaders.inl"

static void create_fragment_program(ProgramWithCachedVariableLocations *p, ProgramWithCachedVariableLocations *src)
{
   *p = *src;

   if (p->program) {
      U32 ucode_size;
      void *ucode_main, *ucode_local;

      cellGcmCgInitProgram(p->program);
      cellGcmCgGetUCode(p->program, &ucode_main, &ucode_size);

      ucode_local = gdraw_arena_alloc(&gdraw->local_arena, ucode_size + 400, CELL_GCM_FRAGMENT_UCODE_LOCAL_ALIGN_OFFSET); // 400 for overfetch
      assert(ucode_local != nullptr); // if this triggers, it's a GDraw bug
      memcpy(ucode_local, ucode_main, ucode_size);

      cellGcmCgGetCgbFragmentProgramConfiguration(p->program, &p->cfg, 0, 1, 0);
      p->cfg.offset = addr2offs(ucode_local);
      p->cfg.attributeInputMask &= ~CELL_GCM_ATTRIB_OUTPUT_MASK_POINTSIZE; // we don't use point sprites
   }
}

static void create_all_shaders()
{
   S32 i;
   U32 slot = 0;

   for (i=0; i < GDRAW_TEXTURE__count*3; ++i)       create_fragment_program(&gdraw->fprog[0][i], pshader_basic_arr + i);
   for (i=0; i < GDRAW_BLENDSPECIAL__count; ++i)    create_fragment_program(&gdraw->exceptional_blend[i], pshader_exceptional_blend_arr + i);
   for (i=0; i < 32; ++i)                           create_fragment_program(&gdraw->filter_prog[0][i], pshader_filter_arr + i);
   for (i=0; i < MAX_TAPS+1; ++i)                   create_fragment_program(&gdraw->blur_prog[i], pshader_blur_arr + i);
   create_fragment_program(&gdraw->colormatrix, pshader_color_matrix_arr);

   for (i=0; i < GDRAW_TEXTURE__count; ++i)         gdraw->basic_fprog[i] = &gdraw->fprog[i][0];

   for (i=0; i < GDRAW_vformat__basic_count; i++) {
      ProgramWithCachedVariableLocations *p = &gdraw->vprog[i];
      U32 ucode_size;

      *p = vshader_vsps3_arr[i];
      if (p->program) {
         cellGcmCgInitProgram(p->program);
         cellGcmCgSetInstructionSlot(p->program, slot);
         cellGcmCgGetUCode(p->program, &p->ucode, &ucode_size);
         gdraw->vslot[i] = slot;
         slot += ucode_size / 16;
      }
   }

   assert(slot <= 512); // that's all the space we have for vertex shaders!
}

////////////////////////////////////////////////////////////////////////
//
//   Create and tear-down the state
//

typedef struct
{
   S32 num_handles;
   S32 num_bytes;
   void *ptr;
} GDrawResourceMem;

static GDrawResourceMem gdraw_mem[GDRAW_GCM_RESOURCE__count];
static void *rt_mem;
static S32 rt_size, rt_width, rt_height, rt_pitch;

static GDrawHandleCache *make_handle_cache(gdraw_gcm_resourcetype type, U32 align)
{
   S32 num_handles = gdraw_mem[type].num_handles;
   S32 num_bytes = gdraw_mem[type].num_bytes;
   rrbool is_vertex = type == GDRAW_GCM_RESOURCE_vertexbuffer;
   U32 cache_size = sizeof(GDrawHandleCache) + (num_handles - 1) * sizeof(GDrawHandle);
   U32 header_size = is_vertex ? 0 : sizeof(GcmTexture) * num_handles;

   if (!num_handles)
      return nullptr;

   GDrawHandleCache *cache = (GDrawHandleCache *) IggyGDrawMalloc(cache_size + header_size);
   if (cache) {
      gdraw_HandleCacheInit(cache, num_handles, num_bytes);
      cache->is_vertex = is_vertex;

      // set up resource headers
      if (type != GDRAW_GCM_RESOURCE_vertexbuffer) {
         GcmTexture *tex = (GcmTexture *) ((U8 *) cache + cache_size);
         S32 i;
         for (i=0; i < num_handles; i++)
            cache->handle[i].handle.tex.gcm = &tex[i];
      }

      // set up our allocator
      cache->alloc = gfxalloc_create(gdraw_mem[type].ptr, num_bytes, align, num_handles);
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

static int get_location(void *ptr)
{
   CellGcmConfig cfg;
   cellGcmGetConfiguration(&cfg);

   U8 *local_start = (U8 *) cfg.localAddress;
   U8 *local_end = local_start + cfg.localSize;
   return (ptr >= local_start && ptr < local_end) ? CELL_GCM_LOCATION_LOCAL : CELL_GCM_LOCATION_MAIN;
}

int gdraw_GCM_SetResourceMemory(gdraw_gcm_resourcetype type, S32 num_handles, void *ptr, S32 num_bytes)
{
   GDrawStats stats;

   assert(type >= GDRAW_GCM_RESOURCE_texture && type < GDRAW_GCM_RESOURCE__count);
   assert(num_handles >= 0);
   assert(num_bytes >= 0);

   if (!num_handles) num_handles = 1;

   switch (type) {
   case GDRAW_GCM_RESOURCE_texture:
      make_pool_aligned(&ptr, &num_bytes, CELL_GCM_TEXTURE_SWIZZLE_ALIGN_OFFSET);
      break;

   case GDRAW_GCM_RESOURCE_vertexbuffer:
   case GDRAW_GCM_RESOURCE_dyn_vertexbuffer:
      make_pool_aligned(&ptr, &num_bytes, CELL_GCM_SURFACE_LINEAR_ALIGN_OFFSET);
      break;

   default: // avoid compiler warning
      break;
   }

   gdraw_mem[type].num_handles = num_handles;
   gdraw_mem[type].num_bytes = num_bytes;
   gdraw_mem[type].ptr = ptr;

   // if no gdraw context created, there's nothing to worry about
   if (!gdraw)
      return 1;

   // wait until GPU is done, then reap everything
   wait_on_fence(gdraw->tile_end_fence);
   memset(&stats, 0, sizeof(stats));
   if (gdraw->texturecache) gdraw_res_reap(gdraw->texturecache, &stats);
   if (gdraw->vbufcache   ) gdraw_res_reap(gdraw->vbufcache, &stats);

   // resize the appropriate pool
   switch (type) {
      case GDRAW_GCM_RESOURCE_texture:
         free_handle_cache(gdraw->texturecache);
         gdraw->texturecache = make_handle_cache(GDRAW_GCM_RESOURCE_texture, CELL_GCM_TEXTURE_SWIZZLE_ALIGN_OFFSET);
         gdraw->tex_loc = get_location(ptr);
         return gdraw->texturecache != nullptr;

      case GDRAW_GCM_RESOURCE_vertexbuffer:
         free_handle_cache(gdraw->vbufcache);
         gdraw->vbufcache = make_handle_cache(GDRAW_GCM_RESOURCE_vertexbuffer, CELL_GCM_SURFACE_LINEAR_ALIGN_OFFSET);
         gdraw->vbuf_base = ptr ? (U8 *) ptr - addr2offs(ptr) : 0;
         gdraw->vbuf_loc = get_location(ptr);
         return gdraw->vbufcache != nullptr;

      case GDRAW_GCM_RESOURCE_dyn_vertexbuffer:
         gdraw_bufring_shutdown(&gdraw->dyn_vb);
         gdraw_bufring_init(&gdraw->dyn_vb, ptr, num_bytes, 2, CELL_GCM_VERTEX_TEXTURE_CACHE_LINE_SIZE);
         gdraw->dynvb_base = ptr ? (U8 *) ptr - addr2offs(ptr) : 0;
         gdraw->dynvb_loc = get_location(ptr);
         return gdraw->dyn_vb.seg_size != 0;

      default:
         return 0;
   }
}

int gdraw_GCM_SetRendertargetMemory(void *ptr, S32 num_bytes, S32 width, S32 height, S32 pitch)
{
   S32 i;
   assert(num_bytes >= 0);

   rt_mem = ptr;
   rt_size = num_bytes;
   rt_width = width;
   rt_height = height;
   rt_pitch = pitch;

   if (!gdraw)
      return 1;

   // make sure RSX is done first
   wait_on_fence(gdraw->tile_end_fence);

   gdraw->frametex_width = width;
   gdraw->frametex_height = height;
   gdraw->rt_pitch = pitch;
   gdraw_arena_init(&gdraw->rt_arena, ptr, rt_size);
   gdraw_arena_reset(&gdraw->rt_arena); // unnecessary, just to avoid warning about unused function
   gdraw->rt_loc = get_location(ptr);
   gdraw_HandleCacheInit(&gdraw->rendertargets, MAX_RENDER_STACK_DEPTH + 1, rt_size);
   for (i=0; i < MAX_RENDER_STACK_DEPTH + 1; ++i)
      gdraw->rendertargets.handle[i].handle.tex.gcm = &gdraw->rendertarget_textures[i];

   return 1;
}

void gdraw_GCM_ResetAllResourceMemory()
{
   gdraw_GCM_SetResourceMemory(GDRAW_GCM_RESOURCE_texture, 0, nullptr, 0);
   gdraw_GCM_SetResourceMemory(GDRAW_GCM_RESOURCE_vertexbuffer, 0, nullptr, 0);
   gdraw_GCM_SetResourceMemory(GDRAW_GCM_RESOURCE_dyn_vertexbuffer, 0, nullptr, 0);
   gdraw_GCM_SetRendertargetMemory(nullptr, 0, 0, 0, 0);
}

GDrawFunctions *gdraw_GCM_CreateContext(CellGcmContextData *gcm, void *local_workmem, U8 rsx_label_index)
{
   S32 i;

   gdraw = (GDraw *) IggyGDrawMalloc(sizeof(*gdraw)); // make sure gdraw struct is PPU cache line aligned
   if (!gdraw) return nullptr;

   memset(gdraw, 0, sizeof(*gdraw));

   // set up local work mem pointers
   gdraw_arena_init(&gdraw->local_arena, local_workmem, GDRAW_GCM_LOCAL_WORKMEM_SIZE);

   // set up fence stuff
   gdraw->fence_label_index = rsx_label_index;
   gdraw->fence_label = cellGcmGetLabelAddress(gdraw->fence_label_index);
   *gdraw->fence_label = 0;
   gdraw->tile_end_fence.value = 0;
   gdraw->next_fence_index = 1;
   gdraw->fence_batch_counter = FENCE_BATCH_INTERVAL;

   // set up memory for all resource types
   for (i = 0; i < GDRAW_GCM_RESOURCE__count; ++i)
      gdraw_GCM_SetResourceMemory((gdraw_gcm_resourcetype) i, gdraw_mem[i].num_handles, gdraw_mem[i].ptr, gdraw_mem[i].num_bytes);

   gdraw_GCM_SetRendertargetMemory(rt_mem, rt_size, rt_width, rt_height, rt_pitch);

   gdraw->gcm = gcm;
   create_all_shaders();

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

   gdraw_funcs.FreeTexture = gdraw_FreeTexture;
   gdraw_funcs.TryToLockTexture = gdraw_TryToLockTexture;

   gdraw_funcs.MakeTextureFromResource = (gdraw_make_texture_from_resource *) gdraw_GCM_MakeTextureFromResource;
   gdraw_funcs.FreeTextureFromResource = gdraw_GCM_DestroyTextureFromResource;

   gdraw_funcs.MakeVertexBufferBegin = gdraw_MakeVertexBufferBegin;
   gdraw_funcs.MakeVertexBufferMore  = gdraw_MakeVertexBufferMore;
   gdraw_funcs.MakeVertexBufferEnd   = gdraw_MakeVertexBufferEnd;
   gdraw_funcs.TryToLockVertexBuffer = gdraw_TryLockVertexBuffer;
   gdraw_funcs.FreeVertexBuffer = gdraw_FreeVertexBuffer;

   gdraw_funcs.UnlockHandles = gdraw_UnlockHandles;
   gdraw_funcs.SetTextureUniqueID = gdraw_SetTextureUniqueID;

   return &gdraw_funcs;
}

void gdraw_GCM_DestroyContext(void)
{
   if (gdraw) {
      GDrawStats stats;
      memset(&stats, 0, sizeof(stats));

      if (gdraw->texturecache) gdraw_res_flush(gdraw->texturecache, &stats);
      if (gdraw->vbufcache)    gdraw_res_flush(gdraw->vbufcache, &stats);

      // make sure RSX is done first
      wait_on_fence(gdraw->tile_end_fence);

      gdraw_bufring_shutdown(&gdraw->dyn_vb);
      free_handle_cache(gdraw->texturecache);
      free_handle_cache(gdraw->vbufcache);
      IggyGDrawFree(gdraw);
      gdraw = nullptr;
   }
}

void RADLINK gdraw_GCM_BeginCustomDraw(IggyCustomDrawCallbackRegion *region, float *matrix)
{
   clear_renderstate();
   gdraw_GetObjectSpaceMatrix(matrix, region->o2w, gdraw->projection, 0.0f, 0);
}

void RADLINK gdraw_GCM_CalculateCustomDraw_4J(IggyCustomDrawCallbackRegion * region, F32 mat[16])
{
   gdraw_GetObjectSpaceMatrix(mat, region->o2w, gdraw->projection, 0.0f, 0);
}

void RADLINK gdraw_GCM_EndCustomDraw(IggyCustomDrawCallbackRegion *region)
{
   RR_UNUSED_VARIABLE(region);
   set_common_renderstate();
   set_projection();
}

GDrawTexture * RADLINK gdraw_GCM_MakeTextureFromResource(U8 *resource_file, S32 len, IggyFileTexturePS3 *texture)
{
   CellGcmTexture *tex = (CellGcmTexture *) (&texture->texture);
   tex->offset = addr2offs(resource_file + texture->file_offset);
   RR_UNUSED_VARIABLE(len);

   // slightly more efficient if we'd done the following at bake time,
   // but doing it here lets us avoid having this knowledge visible
   // in the source to iggyconvert, which lets us avoid having a
   // mechanism for hiding some of that source from non-PS3 customers
   switch (texture->format) {
      case IFT_FORMAT_la_88:
         tex->remap =   CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
                        CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
                        CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
                        CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
                        CELL_GCM_TEXTURE_REMAP_FROM_R << 6 |
                        CELL_GCM_TEXTURE_REMAP_FROM_R << 4 |
                        CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
                        CELL_GCM_TEXTURE_REMAP_FROM_G;
         break;
      case IFT_FORMAT_i_8:
         tex->remap =   CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
                        CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
                        CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
                        CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
                        CELL_GCM_TEXTURE_REMAP_FROM_R << 6 |
                        CELL_GCM_TEXTURE_REMAP_FROM_R << 4 |
                        CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
                        CELL_GCM_TEXTURE_REMAP_FROM_R;
         break;
   }
   return gdraw_GCM_WrappedTextureCreate(tex);
}

void RADLINK gdraw_GCM_DestroyTextureFromResource(GDrawTexture *tex)
{
   gdraw_GCM_WrappedTextureDestroy(tex);
}

