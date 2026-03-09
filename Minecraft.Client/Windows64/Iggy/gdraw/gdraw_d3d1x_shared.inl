// gdraw_d3d1x_shared.inl - author: Fabian Giesen - copyright 2012 RAD Game Tools
//
// This file implements the part of the Iggy graphics driver layer shared between
// D3D10 and 11 (which is most of it). It heavily depends on a bunch of typedefs,
// #defines and utility functions that need to be set up correctly for the D3D version
// being targeted. This is a bit ugly, but much easier to maintain than the original
// solution, where we just kept two almost identical versions of this code.

// That native handle type holds resource handles and a coarse description.
typedef union {
   // handle that is a texture
   struct {
      ID3D1X(Texture2D) *d3d;
      ID3D1X(ShaderResourceView) *d3d_view;
      ID3D1X(RenderTargetView) *d3d_rtview;
      U32 w, h;
   } tex;

   // handle that is a vertex buffer
   struct {
      ID3D1X(Buffer) *verts;
      ID3D1X(Buffer) *inds;
   } vbuf;
} GDrawNativeHandle;

#define GDRAW_NO_STREAMING_MIPGEN // This renderer doesn't use GDraw-internal mipmap generation
#include "gdraw_shared.inl"

// max rendertarget stack depth. this depends on the extent to which you
// use filters and non-standard blend modes, and how nested they are.
#define MAX_RENDER_STACK_DEPTH             8         // Iggy is hardcoded to a limit of 16... probably 1-3 is realistic
#define AATEX_SAMPLER                      7         // sampler that aa_tex gets set in
#define STENCIL_STATE_CACHE_SIZE           32        // number of distinct stencil states we cache DepthStencilStates for
#define QUAD_IB_COUNT                      2048      // quad index buffer has indices for this many quads

#define ASSERT_COUNT(a,b)                  ((a) == (b) ? (b) : -1)

static GDrawFunctions gdraw_funcs;

// render target state
typedef struct
{
   GDrawHandle *color_buffer;
   S32 base_x, base_y, width, height;
   U32 flags;
   rrbool cached;
} GDrawFramebufferState;

struct ProgramWithCachedVariableLocations
{
   DWORD *bytecode;
   union {
      DWORD size;
      ID3D1X(PixelShader) *pshader;
      ID3D1X(VertexShader) *vshader;
   };
};

struct DynBuffer
{
   ID3D1X(Buffer) *buffer;
   U32 size;      // size of buffer
   U32 write_pos; // start of most recently allocated chunk
   U32 alloc_pos; // end of most recently allocated chunk (=start of next allocation)
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

typedef struct
{
   ID3D1XDevice *d3d_device;
   ID3D1XContext *d3d_context;

   // fragment shaders
   ProgramWithCachedVariableLocations fprog[GDRAW_TEXTURE__count][3];
   ProgramWithCachedVariableLocations exceptional_blend[GDRAW_BLENDSPECIAL__count];
   ProgramWithCachedVariableLocations filter_prog[2][16];
   ProgramWithCachedVariableLocations blur_prog[MAX_TAPS+1];
   ProgramWithCachedVariableLocations colormatrix;
   ProgramWithCachedVariableLocations clear_ps;

   // vertex input layouts
   ID3D1X(InputLayout) *inlayout[GDRAW_vformat__count];

   // vertex shaders
   ProgramWithCachedVariableLocations vert[GDRAW_vformat__count]; // [format]

   // render targets
   GDrawHandleCache rendertargets;
   GDrawHandle rendertarget_handles[MAX_RENDER_STACK_DEPTH]; // not -1, because we use +1 to initialize

   gswf_recti rt_valid[MAX_RENDER_STACK_DEPTH+1]; // valid rect for texture clamping

   // size of framebuffer-sized texture used for implementing blend modes
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

   F32 projection[4]; // scalex,scaley,transx,transy
   F32 projmat[3][4];
   F32 xform_3d[3][4];
   rrbool use_3d;

   ID3D1X(RenderTargetView) *main_framebuffer;
   ID3D1X(DepthStencilView) *depth_buffer[2]; // 0=main, 1=rendertarget
   ID3D1X(ShaderResourceView) *main_resolve_target;
   rrbool main_msaa; // does main framebuffer have MSAA enabled?

   ID3D1X(Texture2D) *rt_depth_buffer;
   ID3D1X(Texture2D) *aa_tex;
   ID3D1X(ShaderResourceView) *aa_tex_view;
   ID3D1X(Buffer) *quad_ib; // canned quad indices

   // scale factor converting worldspace to viewspace <0,0>..<w,h>
   F32 world_to_pixel[2];

   // state objects
   ID3D1X(RasterizerState) *raster_state[2]; // [msaa]
   ID3D1X(SamplerState) *sampler_state[2][GDRAW_WRAP__count]; // [nearest][wrap]
   ID3D1X(BlendState) *blend_state[GDRAW_BLEND__count];
   ID3D1X(BlendState) *blend_no_color_write;
   ID3D1X(DepthStencilState) *depth_state[2][2]; // [set_id][test_id]

   // stencil state cache
   // SOA so the keys are tightly packed in a few cache lines!
   U32 stencil_cache_key[STENCIL_STATE_CACHE_SIZE];
   ID3D1X(DepthStencilState) *stencil_cache[STENCIL_STATE_CACHE_SIZE];
   U32 stencil_cache_lru[STENCIL_STATE_CACHE_SIZE];
   U32 stencil_cache_now;

   // constant buffers
   ID3D1X(Buffer) *cb_vertex;
   ID3D1X(Buffer) *cb_ps_common;
   ID3D1X(Buffer) *cb_filter;
   ID3D1X(Buffer) *cb_colormatrix;
   ID3D1X(Buffer) *cb_blur;

   // streaming buffers for dynamic vertex/index data
   DynBuffer dyn_vb;
   DynBuffer dyn_ib;

   U32 dyn_maxalloc, last_dyn_maxalloc;
   S32 max_quad_vert_count;

   // cached state
   U32 scissor_state;      // ~0 if unknown, otherwise 0 or 1
   S32 blend_mode;         // -1 if unknown, otherwise GDRAW_BLEND_*

   // render-state stack described above for 'temporary' rendering
   GDrawFramebufferState frame[MAX_RENDER_STACK_DEPTH];
   GDrawFramebufferState *cur;

   // texture and vertex buffer pools
   GDrawHandleCache *texturecache;
   GDrawHandleCache *vbufcache;

   // stat tracking
   rrbool frame_done;
   U64 frame_counter;

   // error handler
   void (__cdecl *error_handler)(HRESULT hr);
} GDraw;

static GDraw *gdraw;

static const F32 four_zeros[4] = { 0 }; // used in several places

////////////////////////////////////////////////////////////////////////
//
//   General resource management for both textures and vertex buffers
//

template<typename T>
static void safe_release(T *&p)
{
   if (p) {
      p->Release();
      p = nullptr;
   }
}

static void report_d3d_error(HRESULT hr, const char *call, const char *context)
{
   if (hr == E_OUTOFMEMORY)
      IggyGDrawSendWarning(nullptr, "GDraw D3D out of memory in %s%s", call, context);
   else
      IggyGDrawSendWarning(nullptr, "GDraw D3D error in %s%s: 0x%08x", call, context, hr);
}

static void unbind_resources(void)
{
   ID3D1XContext *d3d = gdraw->d3d_context;

   // unset active textures and vertex/index buffers,
   // to make sure there are no dangling refs
   static ID3D1X(ShaderResourceView) *no_views[3] = { nullptr };
   ID3D1X(Buffer) *no_vb = nullptr;
   UINT no_offs = 0;

   d3d->PSSetShaderResources(0, 3, no_views);
   d3d->IASetVertexBuffers(0, 1, &no_vb, &no_offs, &no_offs);
   d3d->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
}

static void api_free_resource(GDrawHandle *r)
{
   unbind_resources();
   if (r->state != GDRAW_HANDLE_STATE_user_owned) {
      if (!r->cache->is_vertex) {
         safe_release(r->handle.tex.d3d_view);
         safe_release(r->handle.tex.d3d_rtview);
         safe_release(r->handle.tex.d3d);
      } else {
         safe_release(r->handle.vbuf.verts);
         safe_release(r->handle.vbuf.inds);
      }
   }
}

static void RADLINK gdraw_UnlockHandles(GDrawStats * /*stats*/)
{
   gdraw_HandleCacheUnlockAll(gdraw->texturecache);
   gdraw_HandleCacheUnlockAll(gdraw->vbufcache);
}

////////////////////////////////////////////////////////////////////////
//
//   Dynamic buffer
//

static void *start_write_dyn(DynBuffer *buf, U32 size)
{
   U8 *ptr = nullptr;

   if (size > buf->size) {
      IggyGDrawSendWarning(nullptr, "GDraw dynamic vertex buffer usage of %d bytes in one call larger than buffer size %d", size, buf->size);
      return nullptr;
   }

   // update statistics
   gdraw->dyn_maxalloc = RR_MAX(gdraw->dyn_maxalloc, size);

   // invariant: current alloc_pos is in [0,size]
   assert(buf->alloc_pos <= buf->size);

   // wrap around when less than "size" bytes left in buffer
   buf->write_pos = ((buf->size - buf->alloc_pos) < size) ? 0 : buf->alloc_pos;

   // discard buffer whenever the current write position is 0;
   // done this way so that if a DISCARD Map() were to fail, we would
   // just keep retrying the next time around.
   ptr = static_cast<U8 *>(map_buffer(gdraw->d3d_context, buf->buffer, buf->write_pos == 0));
   if (ptr) {
      ptr += buf->write_pos; // we return pointer to write position in buffer
      buf->alloc_pos = buf->write_pos + size; // bump alloc position
      assert(buf->alloc_pos <= buf->size); // invariant again
   }
   // if map_buffer fails, it will have sent a warning

   return ptr;
}

static U32 end_write_dyn(DynBuffer *buf)
{
   unmap_buffer(gdraw->d3d_context, buf->buffer);
   return buf->write_pos;
}

////////////////////////////////////////////////////////////////////////
//
//   Stencil state cache
//

static void stencil_state_cache_clear()
{
   S32 i;

   for (i=0; i < STENCIL_STATE_CACHE_SIZE; ++i) {
      gdraw->stencil_cache_key[i] = 0;
      safe_release(gdraw->stencil_cache[i]);
      gdraw->stencil_cache_lru[i] = 0;
   }

   gdraw->stencil_cache_now = 0;
}

static ID3D1X(DepthStencilState) *stencil_state_cache_lookup(rrbool set_id, rrbool test_id, U8 read_mask, U8 write_mask)
{
   D3D1X_(DEPTH_STENCIL_DESC) desc;
   S32 i, best = 0;
   U32 key = (set_id << 1) | test_id | (read_mask << 8) | (write_mask << 16);
   U32 now, age, highest_age;
   HRESULT hr;

   // for LRU
   now = ++gdraw->stencil_cache_now;

   // do we have this in the cache?
   for (i=0; i < STENCIL_STATE_CACHE_SIZE; ++i) {
      if (gdraw->stencil_cache_key[i] == key) {
         gdraw->stencil_cache_lru[i] = now;
         return gdraw->stencil_cache[i];
      }
   }

   // not in the cache, find the best slot to replace it with (LRU)
   highest_age = 0;
   for (i=0; i < STENCIL_STATE_CACHE_SIZE; ++i) {
      if (!gdraw->stencil_cache[i]) { // unused slot!
         best = i;
         break;
      }

      age = now - gdraw->stencil_cache_lru[i];
      if (age > highest_age) {
         highest_age = age;
         best = i;
      }
   }

   // release old depth/stencil state at that position and create new one
   safe_release(gdraw->stencil_cache[best]);

   gdraw->depth_state[set_id][test_id]->GetDesc(&desc); // reference state
   desc.StencilEnable = TRUE;
   desc.StencilReadMask = read_mask;
   desc.StencilWriteMask = write_mask;
   desc.FrontFace.StencilFailOp = D3D1X_(STENCIL_OP_KEEP);
   desc.FrontFace.StencilDepthFailOp = D3D1X_(STENCIL_OP_KEEP);
   desc.FrontFace.StencilPassOp = D3D1X_(STENCIL_OP_REPLACE);
   desc.FrontFace.StencilFunc = D3D1X_(COMPARISON_EQUAL);
   desc.BackFace.StencilFailOp = D3D1X_(STENCIL_OP_KEEP);
   desc.BackFace.StencilDepthFailOp = D3D1X_(STENCIL_OP_KEEP);
   desc.BackFace.StencilPassOp = D3D1X_(STENCIL_OP_REPLACE);
   desc.BackFace.StencilFunc = D3D1X_(COMPARISON_EQUAL);

   hr = gdraw->d3d_device->CreateDepthStencilState(&desc, &gdraw->stencil_cache[best]);
   if (FAILED(hr))
      report_d3d_error(hr, "CreateDepthStencilState", "");

   gdraw->stencil_cache_key[best] = key;
   gdraw->stencil_cache_lru[best] = now;
   return gdraw->stencil_cache[best];
}

////////////////////////////////////////////////////////////////////////
//
//   Texture creation/updating/deletion
//

extern GDrawTexture *gdraw_D3D1X_(WrappedTextureCreate)(ID3D1X(ShaderResourceView) *tex_view)
{
   GDrawStats stats={0};
   GDrawHandle *p = gdraw_res_alloc_begin(gdraw->texturecache, 0, &stats); // it may need to free one item to give us a handle
   p->handle.tex.d3d = nullptr;
   p->handle.tex.d3d_view = tex_view;
   p->handle.tex.d3d_rtview = nullptr;
   p->handle.tex.w = 1;
   p->handle.tex.h = 1;
   gdraw_HandleCacheAllocateEnd(p, 0, nullptr, GDRAW_HANDLE_STATE_user_owned);
   return (GDrawTexture *) p;
}

extern void gdraw_D3D1X_(WrappedTextureChange)(GDrawTexture *tex, ID3D1X(ShaderResourceView) *tex_view)
{
   GDrawHandle *p = (GDrawHandle *) tex;
   p->handle.tex.d3d = nullptr;
   p->handle.tex.d3d_view = tex_view;
}

extern void gdraw_D3D1X_(WrappedTextureDestroy)(GDrawTexture *tex)
{
   GDrawStats stats={0};
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


static rrbool RADLINK gdraw_MakeTextureBegin(void *owner, S32 width, S32 height, gdraw_texture_format format, U32 flags, GDraw_MakeTexture_ProcessingInfo *p, GDrawStats *stats)
{
   GDrawHandle *t = nullptr;
   DXGI_FORMAT dxgi_fmt;
   S32 bpp, size = 0, nmips = 0;

   if (width >= 16384 || height >= 16384) {
      IggyGDrawSendWarning(nullptr, "GDraw texture size too large (%d x %d), dimension limit is 16384", width, height);
      return false;
   }

   if (format == GDRAW_TEXTURE_FORMAT_rgba32) {
      dxgi_fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
      bpp = 4;
   } else {
      dxgi_fmt = DXGI_FORMAT_R8_UNORM;
      bpp = 1;
   }

   // compute estimated size of texture in video memory
   do {
      size += RR_MAX(width >> nmips, 1) * RR_MAX(height >> nmips, 1) * bpp;
      ++nmips;
   } while ((flags & GDRAW_MAKETEXTURE_FLAGS_mipmap) && ((width >> nmips) || (height >> nmips)));

   // try to allocate memory for the client to write to
   p->texture_data = (U8 *) IggyGDrawMalloc(size);
   if (!p->texture_data) {
      IggyGDrawSendWarning(nullptr, "GDraw out of memory to store texture data to pass to D3D for %d x %d texture", width, height);
      return false;
   }

   // allocate a handle and make room in the cache for this much data
   t = gdraw_res_alloc_begin(gdraw->texturecache, size, stats);
   if (!t) {
      IggyGDrawFree(p->texture_data);
      return false;
   }

   t->handle.tex.w = width;
   t->handle.tex.h = height;
   t->handle.tex.d3d = nullptr;
   t->handle.tex.d3d_view = nullptr;
   t->handle.tex.d3d_rtview = nullptr;

   p->texture_type = GDRAW_TEXTURE_TYPE_rgba;
   p->p0 = t;
   p->p1 = owner;
   p->i0 = width;
   p->i1 = height;
   p->i2 = flags;
   p->i3 = dxgi_fmt;
   p->i4 = size;
   p->i5 = nmips;
   p->i6 = bpp;

   p->stride_in_bytes = width * bpp;
   p->num_rows = height;

   return true;
}

static rrbool RADLINK gdraw_MakeTextureMore(GDraw_MakeTexture_ProcessingInfo * /*p*/)
{
   return false;
}

static GDrawTexture * RADLINK gdraw_MakeTextureEnd(GDraw_MakeTexture_ProcessingInfo *p, GDrawStats *stats)
{
   GDrawHandle *t = static_cast<GDrawHandle *>(p->p0);
   D3D1X_(SUBRESOURCE_DATA) mipdata[24];
   S32 i, w, h, nmips, bpp;
   HRESULT hr = S_OK;
   const char *failed_call;
   U8 *ptr;

   // generate mip maps and set up descriptors for them
   assert(p->i5 <= 24);
   ptr = p->texture_data;
   w = p->i0;
   h = p->i1;
   nmips = p->i5;
   bpp = p->i6;

   for (i=0; i < nmips; ++i) {
      mipdata[i].pSysMem = ptr;
      mipdata[i].SysMemPitch = RR_MAX(w >> i, 1) * bpp;
      mipdata[i].SysMemSlicePitch = 0;
      ptr += mipdata[i].SysMemPitch * RR_MAX(h >> i, 1);

      // create mip data by downsampling
      if (i)
         gdraw_Downsample((U8 *) mipdata[i].pSysMem, mipdata[i].SysMemPitch, w >> i, h >> i,
            (U8 *) mipdata[i-1].pSysMem, mipdata[i-1].SysMemPitch, bpp);
   }

   // actually create texture
   D3D1X_(TEXTURE2D_DESC) desc = { static_cast<U32>(w), static_cast<U32>(h), static_cast<U32>(nmips), 1, static_cast<DXGI_FORMAT>(p->i3), { 1, 0 },
      (p->i2 & GDRAW_MAKETEXTURE_FLAGS_updatable) ? D3D1X_(USAGE_DEFAULT) : D3D1X_(USAGE_IMMUTABLE),
      D3D1X_(BIND_SHADER_RESOURCE), 0, 0 };

   failed_call = "CreateTexture2D";
   hr = gdraw->d3d_device->CreateTexture2D(&desc, mipdata, &t->handle.tex.d3d);
   if (FAILED(hr)) goto done;

   // and create a corresponding shader resource view
   failed_call = "CreateShaderResourceView";
   hr = gdraw->d3d_device->CreateShaderResourceView(t->handle.tex.d3d, nullptr, &t->handle.tex.d3d_view);

done:
   if (!FAILED(hr)) {
      gdraw_HandleCacheAllocateEnd(t, p->i4, p->p1, (p->i2 & GDRAW_MAKETEXTURE_FLAGS_never_flush) ? GDRAW_HANDLE_STATE_pinned : GDRAW_HANDLE_STATE_locked);
      stats->nonzero_flags |= GDRAW_STATS_alloc_tex;
      stats->alloc_tex += 1;
      stats->alloc_tex_bytes += p->i4;
   } else {
      safe_release(t->handle.tex.d3d);
      safe_release(t->handle.tex.d3d_view);

      gdraw_HandleCacheAllocateFail(t);
      t = nullptr;
      report_d3d_error(hr, failed_call, " while creating texture");
   }

   IggyGDrawFree(p->texture_data);
   return (GDrawTexture *) t;
}

static rrbool RADLINK gdraw_UpdateTextureBegin(GDrawTexture *t, void *unique_id, GDrawStats * /*stats*/)
{
   return gdraw_HandleCacheLock((GDrawHandle *) t, unique_id);
}

static void RADLINK gdraw_UpdateTextureRect(GDrawTexture *t, void * /*unique_id*/, S32 x, S32 y, S32 stride, S32 w, S32 h, U8 *samples, gdraw_texture_format /*format*/)
{
   GDrawHandle *s = (GDrawHandle *) t;
   D3D1X_(BOX) box = { static_cast<U32>(x), static_cast<U32>(y), 0U, static_cast<U32>(x + w), static_cast<U32>(y + h), 1U };

   gdraw->d3d_context->UpdateSubresource(s->handle.tex.d3d, 0, &box, samples, stride, 0);
}

static void RADLINK gdraw_UpdateTextureEnd(GDrawTexture *t, void * /*unique_id*/, GDrawStats * /*stats*/)
{
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

      gdraw_res_free(t, stats);
   }
}

static rrbool RADLINK gdraw_TryToLockTexture(GDrawTexture *t, void *unique_id, GDrawStats * /*stats*/)
{
   return gdraw_HandleCacheLock((GDrawHandle *) t, unique_id);
}

static void RADLINK gdraw_DescribeTexture(GDrawTexture *tex, GDraw_Texture_Description *desc)
{
   GDrawHandle *p = (GDrawHandle *) tex;
   desc->width = p->handle.tex.w;
   desc->height = p->handle.tex.h;
   desc->size_in_bytes = p->bytes;
}

static void RADLINK gdraw_SetAntialiasTexture(S32 width, U8 *rgba)
{
   HRESULT hr;

   safe_release(gdraw->aa_tex_view);
   safe_release(gdraw->aa_tex);

   D3D1X_(TEXTURE2D_DESC) desc = { static_cast<U32>(width), 1U, 1U, 1U, DXGI_FORMAT_R8G8B8A8_UNORM, { 1, 0 }, D3D1X_(USAGE_IMMUTABLE), D3D1X_(BIND_SHADER_RESOURCE), 0U, 0U };
   D3D1X_(SUBRESOURCE_DATA) data = { rgba, static_cast<U32>(width) * 4U, 0U };

   hr = gdraw->d3d_device->CreateTexture2D(&desc, &data, &gdraw->aa_tex);
   if (FAILED(hr)) {
      report_d3d_error(hr, "CreateTexture2D", "");
      return;
   }

   hr = gdraw->d3d_device->CreateShaderResourceView(gdraw->aa_tex, nullptr, &gdraw->aa_tex_view);
   if (FAILED(hr)) {
      report_d3d_error(hr, "CreateShaderResourceView", " while creating texture");
      safe_release(gdraw->aa_tex);
      return;
   }
}

////////////////////////////////////////////////////////////////////////
//
//   Vertex buffer creation/deletion
//

static rrbool RADLINK gdraw_MakeVertexBufferBegin(void *unique_id, gdraw_vformat /*vformat*/, S32 vbuf_size, S32 ibuf_size, GDraw_MakeVertexBuffer_ProcessingInfo *p, GDrawStats *stats)
{
   // prepare staging buffers for the app to put data into
   p->vertex_data = (U8 *) IggyGDrawMalloc(vbuf_size);
   p->index_data = (U8 *) IggyGDrawMalloc(ibuf_size);
   if (p->vertex_data && p->index_data) {
      GDrawHandle *vb = gdraw_res_alloc_begin(gdraw->vbufcache, vbuf_size + ibuf_size, stats);
      if (vb) {
         vb->handle.vbuf.verts = nullptr;
         vb->handle.vbuf.inds = nullptr;

         p->vertex_data_length = vbuf_size;
         p->index_data_length = ibuf_size;
         p->p0 = vb;
         p->p1 = unique_id;
         return true;
      }
   }

   if (p->vertex_data)
      IggyGDrawFree(p->vertex_data);
   if (p->index_data)
      IggyGDrawFree(p->index_data);

   return false;
}

static rrbool RADLINK gdraw_MakeVertexBufferMore(GDraw_MakeVertexBuffer_ProcessingInfo * /*p*/)
{
   assert(0);
   return false;
}

static GDrawVertexBuffer * RADLINK gdraw_MakeVertexBufferEnd(GDraw_MakeVertexBuffer_ProcessingInfo *p, GDrawStats * /*stats*/)
{
   GDrawHandle *vb = static_cast<GDrawHandle *>(p->p0);

   HRESULT hr;
   D3D1X_(BUFFER_DESC) vbdesc = { static_cast<U32>(p->vertex_data_length), D3D1X_(USAGE_IMMUTABLE), D3D1X_(BIND_VERTEX_BUFFER), 0U, 0U };
   D3D1X_(SUBRESOURCE_DATA) vbdata = { p->vertex_data, 0, 0 };
   
   D3D1X_(BUFFER_DESC) ibdesc = { static_cast<U32>(p->index_data_length), D3D1X_(USAGE_IMMUTABLE), D3D1X_(BIND_INDEX_BUFFER), 0U, 0U };
   D3D1X_(SUBRESOURCE_DATA) ibdata = { p->index_data, 0, 0 };

   hr = gdraw->d3d_device->CreateBuffer(&vbdesc, &vbdata, &vb->handle.vbuf.verts);
   if (!FAILED(hr))
      hr = gdraw->d3d_device->CreateBuffer(&ibdesc, &ibdata, &vb->handle.vbuf.inds);

   if (FAILED(hr)) {
      safe_release(vb->handle.vbuf.verts);
      safe_release(vb->handle.vbuf.inds);

      gdraw_HandleCacheAllocateFail(vb);
      vb = nullptr;

      report_d3d_error(hr, "CreateBuffer", " creating vertex buffer");
   } else {
      gdraw_HandleCacheAllocateEnd(vb, p->vertex_data_length + p->index_data_length, p->p1, GDRAW_HANDLE_STATE_locked);
   }

   IggyGDrawFree(p->vertex_data);
   IggyGDrawFree(p->index_data);

   return (GDrawVertexBuffer *) vb;
}

static rrbool RADLINK gdraw_TryLockVertexBuffer(GDrawVertexBuffer *vb, void *unique_id, GDrawStats * /*stats*/)
{
   return gdraw_HandleCacheLock((GDrawHandle *) vb, unique_id);
}

static void RADLINK gdraw_FreeVertexBuffer(GDrawVertexBuffer *vb, void *unique_id, GDrawStats *stats)
{
   GDrawHandle *h = (GDrawHandle *) vb;
   assert(h != nullptr); // @GDRAW_ASSERT
   if (h->owner == unique_id)
      gdraw_res_free(h, stats);
}

static void RADLINK gdraw_DescribeVertexBuffer(GDrawVertexBuffer *vbuf, GDraw_VertexBuffer_Description *desc)
{
   GDrawHandle *p = (GDrawHandle *) vbuf;
   desc->size_in_bytes = p->bytes;
}

////////////////////////////////////////////////////////////////////////
//
//   Create/free (or cache) render targets
//

static GDrawHandle *get_color_rendertarget(GDrawStats *stats)
{
   const char *failed_call;

   // try to recycle LRU rendertarget
   GDrawHandle *t = gdraw_HandleCacheGetLRU(&gdraw->rendertargets);
   if (t) {
      gdraw_HandleCacheLock(t, (void *) 1);
      return t;
   }

   // ran out of RTs, allocate a new one
   S32 size = gdraw->frametex_width * gdraw->frametex_height * 4;
   if (gdraw->rendertargets.bytes_free < size) {
      IggyGDrawSendWarning(nullptr, "GDraw rendertarget allocation failed: hit size limit of %d bytes", gdraw->rendertargets.total_bytes);
      return nullptr;
   }

   t = gdraw_HandleCacheAllocateBegin(&gdraw->rendertargets);
   if (!t) {
      IggyGDrawSendWarning(nullptr, "GDraw rendertarget allocation failed: hit handle limit");
      return t;
   }

   D3D1X_(TEXTURE2D_DESC) desc = { static_cast<U32>(gdraw->frametex_width), static_cast<U32>(gdraw->frametex_height), 1U, 1U, DXGI_FORMAT_R8G8B8A8_UNORM, { 1, 0 },
      D3D1X_(USAGE_DEFAULT), D3D1X_(BIND_SHADER_RESOURCE) | D3D1X_(BIND_RENDER_TARGET), 0U, 0U };

   t->handle.tex.d3d = nullptr;
   t->handle.tex.d3d_view = nullptr;
   t->handle.tex.d3d_rtview = nullptr;

   HRESULT hr = gdraw->d3d_device->CreateTexture2D(&desc, nullptr, &t->handle.tex.d3d);
   failed_call = "CreateTexture2D";
   if (!FAILED(hr)) {
      hr = gdraw->d3d_device->CreateShaderResourceView(t->handle.tex.d3d, nullptr, &t->handle.tex.d3d_view);
      failed_call = "CreateTexture2D";
   }
   if (!FAILED(hr)) {
      hr = gdraw->d3d_device->CreateRenderTargetView(t->handle.tex.d3d, nullptr, &t->handle.tex.d3d_rtview);
      failed_call = "CreateRenderTargetView";
   }

   if (FAILED(hr)) {
      safe_release(t->handle.tex.d3d);
      safe_release(t->handle.tex.d3d_view);
      safe_release(t->handle.tex.d3d_rtview);
      gdraw_HandleCacheAllocateFail(t);

      report_d3d_error(hr, failed_call, " creating rendertarget");

      return nullptr;
   }

   gdraw_HandleCacheAllocateEnd(t, size, (void *) 1, GDRAW_HANDLE_STATE_locked);
   stats->nonzero_flags |= GDRAW_STATS_alloc_tex;
   stats->alloc_tex += 1;
   stats->alloc_tex_bytes += size;

   return t;
}

static ID3D1X(DepthStencilView) *get_rendertarget_depthbuffer(GDrawStats *stats)
{
   if (!gdraw->depth_buffer[1]) {
      const char *failed_call;
      assert(!gdraw->rt_depth_buffer);

      D3D1X_(TEXTURE2D_DESC) desc = { static_cast<U32>(gdraw->frametex_width), static_cast<U32>(gdraw->frametex_height), 1U, 1U, DXGI_FORMAT_D24_UNORM_S8_UINT, { 1, 0 },
         D3D1X_(USAGE_DEFAULT), D3D1X_(BIND_DEPTH_STENCIL), 0U, 0U };

      HRESULT hr = gdraw->d3d_device->CreateTexture2D(&desc, nullptr, &gdraw->rt_depth_buffer);
      failed_call = "CreateTexture2D";
      if (!FAILED(hr)) {
         hr = gdraw->d3d_device->CreateDepthStencilView(gdraw->rt_depth_buffer, nullptr, &gdraw->depth_buffer[1]);
         failed_call = "CreateDepthStencilView while creating rendertarget";
      }

      if (FAILED(hr)) {
         report_d3d_error(hr, failed_call, "");
         safe_release(gdraw->rt_depth_buffer);
         safe_release(gdraw->depth_buffer[1]);
      } else {
         stats->nonzero_flags |= GDRAW_STATS_alloc_tex;
         stats->alloc_tex += 1;
         stats->alloc_tex_bytes += gdraw->frametex_width * gdraw->frametex_height * 4;

         gdraw->d3d_context->ClearDepthStencilView(gdraw->depth_buffer[1], D3D1X_(CLEAR_DEPTH) | D3D1X_(CLEAR_STENCIL), 1.0f, 0);
      }
   }

   return gdraw->depth_buffer[1];
}

static void flush_rendertargets(GDrawStats *stats)
{
   gdraw_res_flush(&gdraw->rendertargets, stats);

   safe_release(gdraw->depth_buffer[1]);
   safe_release(gdraw->rt_depth_buffer);
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
   F32 x3d[4];
   F32 y3d[4];
   F32 w3d[4];
};

struct PixelCommonVars
{
   F32 color_mul[4];
   F32 color_add[4];
   F32 focal[4];
   F32 rescale1[4];
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

static void disable_scissor(int force)
{
   if (force || gdraw->scissor_state) {
      // disable scissor by setting whole viewport as scissor rect
      S32 x = gdraw->cview.x;
      S32 y = gdraw->cview.y;
      D3D1X_(RECT) r = { x, y, x + gdraw->cview.w, y + gdraw->cview.h };

      gdraw->d3d_context->RSSetScissorRects(1, &r);
      gdraw->scissor_state = 0;
   }
}

static void set_viewport_raw(S32 x, S32 y, S32 w, S32 h)
{
   D3D1X_(VIEWPORT) vp = { static_cast<ViewCoord>(x), static_cast<ViewCoord>(y), static_cast<ViewCoord>(w), static_cast<ViewCoord>(h), 0.0f, 1.0f };
   gdraw->d3d_context->RSSetViewports(1, &vp);
   gdraw->cview.x = x;
   gdraw->cview.y = y;
   gdraw->cview.w = w;
   gdraw->cview.h = h;
   
   disable_scissor(1);
}

static void set_projection_base(void)
{
   //    x3d = < viewproj.x, 0, 0, 0 >
   //    y3d = < 0, viewproj.y, 0, 0 >
   //    w3d = < viewproj.z, viewproj.w, 1.0, 1.0 >

   memset(gdraw->projmat[0], 0, sizeof(gdraw->projmat));
   gdraw->projmat[0][0] = gdraw->projection[0];
   gdraw->projmat[1][1] = gdraw->projection[1];
   gdraw->projmat[2][0] = gdraw->projection[2];
   gdraw->projmat[2][1] = gdraw->projection[3];

   gdraw->projmat[2][2] = 1.0;
   gdraw->projmat[2][3] = 1.0;
}

static void set_projection_raw(S32 x0, S32 x1, S32 y0, S32 y1)
{
   gdraw->projection[0] = 2.0f / (x1-x0);
   gdraw->projection[1] = 2.0f / (y1-y0);
   gdraw->projection[2] = (x1+x0)/static_cast<F32>(x0 - x1);
   gdraw->projection[3] = (y1+y0)/static_cast<F32>(y0 - y1);

   set_projection_base();
}


static void set_viewport(void)
{
   if (gdraw->in_blur) {
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

static void set_projection(void)
{
   if (gdraw->in_blur) return;
   if (gdraw->cur == gdraw->frame) // if the render stack is empty
      set_projection_raw(gdraw->tx0, gdraw->tx0+gdraw->tw, gdraw->ty0+gdraw->th, gdraw->ty0);
   else if (gdraw->cur->cached)
      set_projection_raw(gdraw->cur->base_x, gdraw->cur->base_x+gdraw->cur->width, gdraw->cur->base_y, gdraw->cur->base_y+gdraw->cur->height);
   else
      set_projection_raw(gdraw->tx0p, gdraw->tx0p+gdraw->tpw, gdraw->ty0p+gdraw->tph, gdraw->ty0p);
}

static void clear_renderstate(void)
{
   gdraw->d3d_context->ClearState();
}

static void set_common_renderstate()
{
   ID3D1XContext *d3d = gdraw->d3d_context;
   S32 i;

   clear_renderstate();

   // all the render states we never change while drawing
   d3d->IASetPrimitiveTopology(D3D1X_(PRIMITIVE_TOPOLOGY_TRIANGLELIST));

   d3d->PSSetShaderResources(7, 1, &gdraw->aa_tex_view);
   d3d->PSSetSamplers(7, 1, &gdraw->sampler_state[0][GDRAW_WRAP_clamp]);

   // set a well-defined default sampler for all PS textures we use
   for (i=0; i < 3; ++i)
      d3d->PSSetSamplers(i, 1, &gdraw->sampler_state[0][GDRAW_WRAP_clamp]);

   // reset our state caching
   gdraw->scissor_state = ~0u;
   gdraw->blend_mode = -1;
}

static void manual_clear(gswf_recti *r, GDrawStats *stats);
static void set_render_target(GDrawStats *stats);

////////////////////////////////////////////////////////////////////////
//
//   Begin/end rendering of a tile and per-frame processing
//

void gdraw_D3D1X_(SetRendertargetSize)(S32 w, S32 h)
{
   if (gdraw && (w != gdraw->frametex_width || h != gdraw->frametex_height)) {
      GDrawStats stats = { 0 };
      gdraw->frametex_width = w;
      gdraw->frametex_height = h;
      flush_rendertargets(&stats);
   }
}

void gdraw_D3D1X_(SetTileOrigin)(ID3D1X(RenderTargetView) *main_rt, ID3D1X(DepthStencilView) *main_ds, ID3D1X(ShaderResourceView) *non_msaa_rt, S32 x, S32 y)
{

    if (!gdraw) return; // AAR - saftey check because windows calls resize early

   D3D1X_(RENDER_TARGET_VIEW_DESC) desc;

   if (gdraw->frame_done) {
      ++gdraw->frame_counter;
      gdraw->frame_done = false;
   }

   main_rt->GetDesc(&desc);

   gdraw->main_framebuffer = main_rt;
   gdraw->main_resolve_target = non_msaa_rt;
   gdraw->main_msaa = (desc.ViewDimension == D3D1X_(RTV_DIMENSION_TEXTURE2DMS));
   gdraw->depth_buffer[0] = main_ds;

   gdraw->vx = x;
   gdraw->vy = y;
}

static void RADLINK gdraw_SetViewSizeAndWorldScale(S32 w, S32 h, F32 scalex, F32 scaley)
{
   static S32 s_lastW = 0, s_lastH = 0;
   static F32 s_lastSx = 0, s_lastSy = 0;
   if (w != s_lastW || h != s_lastH || scalex != s_lastSx || scaley != s_lastSy) {
      app.DebugPrintf("[GDRAW] SetViewSize: fw=%d fh=%d scale=%.6f,%.6f frametex=%dx%d vx=%d vy=%d\n",
         w, h, scalex, scaley, gdraw->frametex_width, gdraw->frametex_height, gdraw->vx, gdraw->vy);
      s_lastW = w; s_lastH = h; s_lastSx = scalex; s_lastSy = scaley;
   }
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
}
static void RADLINK gdraw_RenderingEnd(void)
{
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

   // make sure our rendertargets are large enough to contain the tile
   if (gdraw->tpw > gdraw->frametex_width || gdraw->tph > gdraw->frametex_height) {
      gdraw->frametex_width  = RR_MAX(gdraw->tpw, gdraw->frametex_width);
      gdraw->frametex_height = RR_MAX(gdraw->tph, gdraw->frametex_height);

      flush_rendertargets(stats);
   }
   assert(gdraw->tpw <= gdraw->frametex_width && gdraw->tph <= gdraw->frametex_height);

   // set up rendertargets we'll use
   set_common_renderstate();
   gdraw->d3d_context->ClearDepthStencilView(gdraw->depth_buffer[0], D3D1X_(CLEAR_DEPTH) | D3D1X_(CLEAR_STENCIL), 1.0f, 0);
   if (gdraw->depth_buffer[1])
      gdraw->d3d_context->ClearDepthStencilView(gdraw->depth_buffer[1], D3D1X_(CLEAR_DEPTH) | D3D1X_(CLEAR_STENCIL), 1.0f, 0);

   set_projection();
   set_viewport();
   set_render_target(stats);
}

static void RADLINK gdraw_RenderTileEnd(GDrawStats * /*stats*/)
{
}

void gdraw_D3D1X_(NoMoreGDrawThisFrame)(void)
{
   clear_renderstate();
   gdraw->frame_done = true;

   gdraw->last_dyn_maxalloc = gdraw->dyn_maxalloc;
   gdraw->dyn_maxalloc = 0;

   // reset dynamic buffer alloc position so they get DISCARDed
   // next time around.
   gdraw->dyn_vb.alloc_pos = 0;
   gdraw->dyn_ib.alloc_pos = 0;

   GDrawFence now = { gdraw->frame_counter };
   gdraw_HandleCacheTick(gdraw->texturecache, now);
   gdraw_HandleCacheTick(gdraw->vbufcache, now);
}

#define MAX_DEPTH_VALUE   (1 << 13)

static void RADLINK gdraw_GetInfo(GDrawInfo *d)
{
   d->num_stencil_bits = 8;
   d->max_id = MAX_DEPTH_VALUE-2;
   // for floating point depth, just use mantissa, e.g. 16-20 bits
   d->buffer_format = GDRAW_BFORMAT_vbib;
   d->shared_depth_stencil = 1;
   d->always_mipmap = 1;
#ifndef GDRAW_D3D11_LEVEL9
   d->max_texture_size = 8192;
   d->conditional_nonpow2 = 0;
#else
   d->max_texture_size = 2048;
   d->conditional_nonpow2 = 1;
#endif
}

////////////////////////////////////////////////////////////////////////
//
//   Enable/disable rendertargets in stack fashion
//

static ID3D1X(RenderTargetView) *get_active_render_target()
{
   if (gdraw->cur->color_buffer) {
      unbind_resources(); // to make sure this RT isn't accidentally set as a texture (avoid D3D warnings)
      return gdraw->cur->color_buffer->handle.tex.d3d_rtview;
   } else
      return gdraw->main_framebuffer;
}

static void set_render_target(GDrawStats *stats)
{
   ID3D1X(RenderTargetView) *target = get_active_render_target();
   if (target == gdraw->main_framebuffer) {
      gdraw->d3d_context->OMSetRenderTargets(1, &target, gdraw->depth_buffer[0]);
      gdraw->d3d_context->RSSetState(gdraw->raster_state[gdraw->main_msaa]);
   } else {
      ID3D1X(DepthStencilView) *depth = nullptr;
      if (gdraw->cur->flags & (GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_id | GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_stencil))
         depth = get_rendertarget_depthbuffer(stats);

      gdraw->d3d_context->OMSetRenderTargets(1, &target, depth);
      gdraw->d3d_context->RSSetState(gdraw->raster_state[0]);
   }

   stats->nonzero_flags |= GDRAW_STATS_rendtarg;
   stats->rendertarget_changes += 1;
}

static rrbool RADLINK gdraw_TextureDrawBufferBegin(gswf_recti *region, gdraw_texture_format /*format*/, U32 flags, void *owner, GDrawStats *stats)
{
   GDrawFramebufferState *n = gdraw->cur+1;
   GDrawHandle *t = nullptr;
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
      // nyi
   } else {
      t = get_color_rendertarget(stats);
      if (!t)
         return false;
   }

   n->flags = flags;
   n->color_buffer = t;
   assert(n->color_buffer != nullptr); // @GDRAW_ASSERT

   ++gdraw->cur;
   gdraw->cur->cached = owner != nullptr;
   if (owner) {
      gdraw->cur->base_x = region->x0;
      gdraw->cur->base_y = region->y0;
      gdraw->cur->width  = region->x1 - region->x0;
      gdraw->cur->height = region->y1 - region->y0;
   }

   set_render_target(stats);
   assert(gdraw->frametex_width >= gdraw->tw && gdraw->frametex_height >= gdraw->th); // @GDRAW_ASSERT

   S32 k = static_cast<S32>(t - gdraw->rendertargets.handle);

   if (region) {
      gswf_recti r;
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
      r.x0 = RR_MAX(xt0 - pad, 0);
      r.y0 = RR_MAX(yt0 - pad, 0);
      r.x1 = RR_MIN(xt1 + pad, gdraw->frametex_width);
      r.y1 = RR_MIN(yt1 + pad, gdraw->frametex_height);

      if (r.x1 <= r.x0 || r.y1 <= r.y0) { // region doesn't intersect with current tile
         --gdraw->cur;
         gdraw_FreeTexture((GDrawTexture *) t, nullptr, stats);
         // note: don't send a warning since this will happen during regular tiled rendering
         return false;
      }

      manual_clear(&r, stats);

      gdraw->rt_valid[k].x0 = xt0;
      gdraw->rt_valid[k].y0 = yt0;
      gdraw->rt_valid[k].x1 = xt1;
      gdraw->rt_valid[k].y1 = yt1;
   } else {
      gdraw->d3d_context->ClearRenderTargetView(gdraw->cur->color_buffer->handle.tex.d3d_rtview, four_zeros);
      gdraw->rt_valid[k].x0 = 0;
      gdraw->rt_valid[k].y0 = 0;
      gdraw->rt_valid[k].x1 = gdraw->frametex_width;
      gdraw->rt_valid[k].y1 = gdraw->frametex_height;
   }

   if (!gdraw->in_blur) {
      set_viewport();
      set_projection();
   } else {
      set_viewport_raw(0, 0, gdraw->tpw, gdraw->tph);
      set_projection_raw(0, gdraw->tpw, gdraw->tph, 0);
   }

   return true;
}

static GDrawTexture *RADLINK gdraw_TextureDrawBufferEnd(GDrawStats *stats)
{
   GDrawFramebufferState *n =   gdraw->cur;
   GDrawFramebufferState *m = --gdraw->cur;
   if (gdraw->tw == 0 || gdraw->th == 0) return nullptr;

   if (n >= &gdraw->frame[MAX_RENDER_STACK_DEPTH])
      return nullptr; // already returned a warning in Begin

   assert(m >= gdraw->frame);  // bug in Iggy -- unbalanced

   if (m != gdraw->frame) {
      assert(m->color_buffer != nullptr); // @GDRAW_ASSERT
   }
   assert(n->color_buffer != nullptr); // @GDRAW_ASSERT

   // switch back to old render target
   set_render_target(stats);

   // if we're at the root, set the viewport back
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

static void RADLINK gdraw_ClearStencilBits(U32 /*bits*/)
{
   gdraw->d3d_context->ClearDepthStencilView(gdraw->depth_buffer[0], D3D1X_(CLEAR_STENCIL), 1.0f, 0);
   if (gdraw->depth_buffer[1])
      gdraw->d3d_context->ClearDepthStencilView(gdraw->depth_buffer[1], D3D1X_(CLEAR_STENCIL), 1.0f, 0);
}

// this only happens rarely (hopefully never) if we use the depth buffer,
// so we can just clear the whole thing
static void RADLINK gdraw_ClearID(void)
{
   gdraw->d3d_context->ClearDepthStencilView(gdraw->depth_buffer[0], D3D1X_(CLEAR_DEPTH), 1.0f, 0);
   if (gdraw->depth_buffer[1])
      gdraw->d3d_context->ClearDepthStencilView(gdraw->depth_buffer[1], D3D1X_(CLEAR_DEPTH), 1.0f, 0);
}

////////////////////////////////////////////////////////////////////////
//
//   Set all the render state from GDrawRenderState
//
// This also is responsible for getting the framebuffer into a texture
// if the read-modify-write blend operation can't be expressed with
// the native blend operators. (E.g. "screen")
//

// convert an ID request to a value suitable for the depth buffer,
// assuming the depth buffer has been mappped to 0..1
static F32 depth_from_id(S32 id)
{
   return 1.0f - (static_cast<F32>(id) + 1.0f) / MAX_DEPTH_VALUE;
}

static void set_texture(S32 texunit, GDrawTexture *tex, rrbool nearest, S32 wrap)
{
   ID3D1XContext *d3d = gdraw->d3d_context;

   if (tex == nullptr) {
      ID3D1X(ShaderResourceView) *notex = nullptr;
      d3d->PSSetShaderResources(texunit, 1, &notex);
   } else {
      GDrawHandle *h = (GDrawHandle *) tex;
      d3d->PSSetShaderResources(texunit, 1, &h->handle.tex.d3d_view);
      d3d->PSSetSamplers(texunit, 1, &gdraw->sampler_state[nearest][wrap]);
   }
}

static void RADLINK gdraw_Set3DTransform(F32 *mat)
{
   if (mat == nullptr)
      gdraw->use_3d = 0;
   else {
      gdraw->use_3d = 1;
      memcpy(gdraw->xform_3d, mat, sizeof(gdraw->xform_3d));
   }
}

static int set_renderstate_full(S32 vertex_format, GDrawRenderState *r, GDrawStats * /* stats */, const F32 *rescale1)
{
   ID3D1XContext *d3d = gdraw->d3d_context;

   // set vertex shader
   set_vertex_shader(d3d, gdraw->vert[vertex_format].vshader);
   
   // set vertex shader constants
   if (VertexVars *vvars = static_cast<VertexVars *>(map_buffer(gdraw->d3d_context, gdraw->cb_vertex, true))) {
      F32 depth = depth_from_id(r->id);
      if (!r->use_world_space)
         gdraw_ObjectSpace(vvars->world[0], r->o2w, depth, 0.0f);
      else
         gdraw_WorldSpace(vvars->world[0], gdraw->world_to_pixel, depth, 0.0f);

      memcpy(&vvars->x_off, r->edge_matrix, 4*sizeof(F32));

      if (r->texgen0_enabled) {
         memcpy(&vvars->texgen_s, r->s0_texgen, 4*sizeof(F32));
         memcpy(&vvars->texgen_t, r->t0_texgen, 4*sizeof(F32));
      }

      if (gdraw->use_3d)
         memcpy(vvars->x3d, gdraw->xform_3d, 12*sizeof(F32));
      else
         memcpy(vvars->x3d, gdraw->projmat, 12*sizeof(F32));

      unmap_buffer(gdraw->d3d_context, gdraw->cb_vertex);

      d3d->VSSetConstantBuffers(0, 1, &gdraw->cb_vertex);
   }

   // set the blend mode
   int blend_mode = r->blend_mode;
   if (blend_mode != gdraw->blend_mode) {
      gdraw->blend_mode = blend_mode;
      d3d->OMSetBlendState(gdraw->blend_state[blend_mode], four_zeros, ~0u);
   }

   // set the fragment program
   if (blend_mode != GDRAW_BLEND_special) {
      int which = r->tex0_mode;
      assert(which >= 0 && which < sizeof(gdraw->fprog) / sizeof(*gdraw->fprog));

      int additive = 0;
      if (r->cxf_add) {
         additive = 1;
         if (r->cxf_add[3]) additive = 2;
      }

      ID3D1X(PixelShader) *program = gdraw->fprog[which][additive].pshader;
      if (r->stencil_set) {
         // in stencil set mode, prefer not doing any shading at all
         // but if alpha test is on, we need to make an exception

#ifndef GDRAW_D3D11_LEVEL9 // level9 can't do nullptr PS it seems
         if (which != GDRAW_TEXTURE_alpha_test)
            program = nullptr;
         else
#endif
         {
            gdraw->blend_mode = -1;
            d3d->OMSetBlendState(gdraw->blend_no_color_write, four_zeros, ~0u);
         }
      }

      set_pixel_shader(d3d, program);
   } else
      set_pixel_shader(d3d, gdraw->exceptional_blend[r->special_blend].pshader);

   set_texture(0, r->tex[0], r->nearest0, r->wrap0);

   // pixel shader constants
   if (PixelCommonVars *pvars = static_cast<PixelCommonVars *>(map_buffer(gdraw->d3d_context, gdraw->cb_ps_common, true))) {
      memcpy(pvars->color_mul, r->color, 4*sizeof(float));

      if (r->cxf_add) {
         pvars->color_add[0] = r->cxf_add[0] / 255.0f;
         pvars->color_add[1] = r->cxf_add[1] / 255.0f;
         pvars->color_add[2] = r->cxf_add[2] / 255.0f;
         pvars->color_add[3] = r->cxf_add[3] / 255.0f;
      } else
         pvars->color_add[0] = pvars->color_add[1] = pvars->color_add[2] = pvars->color_add[3] = 0.0f;

      if (r->tex0_mode == GDRAW_TEXTURE_focal_gradient) memcpy(pvars->focal, r->focal_point, 4*sizeof(float));
      if (r->blend_mode == GDRAW_BLEND_special) memcpy(pvars->rescale1, rescale1, 4*sizeof(float));
      unmap_buffer(gdraw->d3d_context, gdraw->cb_ps_common);
      d3d->PSSetConstantBuffers(0, 1, &gdraw->cb_ps_common);
   }

   // Set pixel operation states
   if (r->scissor) {
      D3D1X_(RECT) s;
      gdraw->scissor_state = 1;
      if (gdraw->cur == gdraw->frame) {
         s.left = r->scissor_rect.x0 + gdraw->vx - gdraw->tx0;
         s.top  = r->scissor_rect.y0 + gdraw->vy - gdraw->ty0;
         s.right = r->scissor_rect.x1 + gdraw->vx - gdraw->tx0;
         s.bottom = r->scissor_rect.y1 + gdraw->vy - gdraw->ty0;
      } else {
         s.left = r->scissor_rect.x0 - gdraw->tx0p;
         s.top  = r->scissor_rect.y0 - gdraw->ty0p;
         s.right = r->scissor_rect.x1 - gdraw->tx0p;
         s.bottom = r->scissor_rect.y1 - gdraw->ty0p;
      }
      d3d->RSSetScissorRects(1, &s);
   } else if (r->scissor != gdraw->scissor_state)
      disable_scissor(0);

   if (r->stencil_set | r->stencil_test)
      d3d->OMSetDepthStencilState(stencil_state_cache_lookup(r->set_id, r->test_id, r->stencil_test, r->stencil_set), 255);
   else
      d3d->OMSetDepthStencilState(gdraw->depth_state[r->set_id][r->test_id], 0);

   return 1;
}

static RADINLINE int set_renderstate(S32 vertex_format, GDrawRenderState *r, GDrawStats *stats)
{
   static const F32 unit_rescale[4] = { 1.0f, 1.0f, 0.0f, 0.0f };
   if (r->identical_state) {
      // fast path: only need to change vertex shader, other state is the same
      set_vertex_shader(gdraw->d3d_context, gdraw->vert[vertex_format].vshader);
      return 1;
   } else
      return set_renderstate_full(vertex_format, r, stats, unit_rescale);
}

////////////////////////////////////////////////////////////////////////
//
//   Vertex formats
//

static D3D1X_(INPUT_ELEMENT_DESC) vformat_v2[] = {
   { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D1X_(INPUT_PER_VERTEX_DATA), 0 },
};

static D3D1X_(INPUT_ELEMENT_DESC) vformat_v2aa[] = {
   { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,      0, 0, D3D1X_(INPUT_PER_VERTEX_DATA), 0 },
   { "TEXCOORD", 0, DXGI_FORMAT_R16G16B16A16_SINT, 0, 8, D3D1X_(INPUT_PER_VERTEX_DATA), 0 },
};

static D3D1X_(INPUT_ELEMENT_DESC) vformat_v2tc2[] = {
   { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,      0, 0, D3D1X_(INPUT_PER_VERTEX_DATA), 0 },
   { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,      0, 8, D3D1X_(INPUT_PER_VERTEX_DATA), 0 },
};

static struct gdraw_vertex_format_desc {
   D3D1X_(INPUT_ELEMENT_DESC) *desc;
   U32 nelem;
} vformats[ASSERT_COUNT(GDRAW_vformat__basic_count, 3)] = {
   vformat_v2,    1,    // GDRAW_vformat_v2
   vformat_v2aa,  2,    // GDRAW_vformat_v2aa
   vformat_v2tc2, 2,    // GDRAW_vforamt_v2tc2
};

static int vertsize[GDRAW_vformat__basic_count] = {
   8,    // GDRAW_vformat_v2
   16,   // GDRAW_vformat_v2aa
   16,   // GDRAW_vformat_v2tc2
};

////////////////////////////////////////////////////////////////////////
//
//   Draw triangles with a given renderstate
//

static void tag_resources(void *r1, void *r2=nullptr, void *r3=nullptr, void *r4=nullptr)
{
   U64 now = gdraw->frame_counter;
   if (r1) static_cast<GDrawHandle *>(r1)->fence.value = now;
   if (r2) static_cast<GDrawHandle *>(r2)->fence.value = now;
   if (r3) static_cast<GDrawHandle *>(r3)->fence.value = now;
   if (r4) static_cast<GDrawHandle *>(r4)->fence.value = now;
}

static void RADLINK gdraw_DrawIndexedTriangles(GDrawRenderState *r, GDrawPrimitive *p, GDrawVertexBuffer *buf, GDrawStats *stats)
{
   ID3D1XContext *d3d = gdraw->d3d_context;
   GDrawHandle *vb = (GDrawHandle *) buf;
   int vfmt = p->vertex_format;
   assert(vfmt >= 0 && vfmt < GDRAW_vformat__count);

   if (!set_renderstate(vfmt, r, stats))
      return;

   UINT stride = vertsize[vfmt];
   d3d->IASetInputLayout(gdraw->inlayout[vfmt]);

   if (vb) {
      UINT offs = static_cast<UINT>((UINTa)p->vertices);

      d3d->IASetVertexBuffers(0, 1, &vb->handle.vbuf.verts, &stride, &offs);
      d3d->IASetIndexBuffer(vb->handle.vbuf.inds, DXGI_FORMAT_R16_UINT, static_cast<UINT>((UINTa)p->indices));
      d3d->DrawIndexed(p->num_indices, 0, 0);
   } else if (p->indices) {
      U32 vbytes = p->num_vertices * stride;
      U32 ibytes = p->num_indices * 2;
      
      if (void *vbptr = start_write_dyn(&gdraw->dyn_vb, vbytes)) {
         memcpy(vbptr, p->vertices, vbytes);
         UINT vboffs = end_write_dyn(&gdraw->dyn_vb);

         if (void *ibptr = start_write_dyn(&gdraw->dyn_ib, ibytes)) {
            memcpy(ibptr, p->indices, ibytes);
            UINT iboffs = end_write_dyn(&gdraw->dyn_ib);

            d3d->IASetVertexBuffers(0, 1, &gdraw->dyn_vb.buffer, &stride, &vboffs);
            d3d->IASetIndexBuffer(gdraw->dyn_ib.buffer, DXGI_FORMAT_R16_UINT, iboffs);
            d3d->DrawIndexed(p->num_indices, 0, 0);
         }
      }
  } else { // dynamic quads
      assert(p->num_vertices % 4 == 0);
      d3d->IASetIndexBuffer(gdraw->quad_ib, DXGI_FORMAT_R16_UINT, 0);

      if (gdraw->max_quad_vert_count) {
         S32 pos = 0;
         while (pos < p->num_vertices) {
            S32 vert_count = RR_MIN(p->num_vertices - pos, gdraw->max_quad_vert_count);
            UINT chunk_bytes = vert_count * stride;

            if (void *vbptr = start_write_dyn(&gdraw->dyn_vb, chunk_bytes)) {
               memcpy(vbptr, (U8 *)p->vertices + pos*stride, chunk_bytes);
               UINT offs = end_write_dyn(&gdraw->dyn_vb);

               d3d->IASetVertexBuffers(0, 1, &gdraw->dyn_vb.buffer, &stride, &offs);
               d3d->DrawIndexed((vert_count >> 2) * 6, 0, 0);
            }
            pos += vert_count;
         }
      }
   }

   tag_resources(vb, r->tex[0], r->tex[1]);

   stats->nonzero_flags |= GDRAW_STATS_batches;
   stats->num_batches += 1;
   stats->drawn_indices += p->num_indices;
   stats->drawn_vertices += p->num_vertices;
}

///////////////////////////////////////////////////////////////////////
//
//   Flash 8 filter effects
//

static void *start_ps_constants(ID3D1X(Buffer) *buffer)
{
   return map_buffer(gdraw->d3d_context, buffer, true);
}

static void end_ps_constants(ID3D1X(Buffer) *buffer)
{
   unmap_buffer(gdraw->d3d_context, buffer);
   gdraw->d3d_context->PSSetConstantBuffers(1, 1, &buffer);
}

static void set_pixel_constant(F32 *constant, F32 x, F32 y, F32 z, F32 w)
{
   constant[0] = x;
   constant[1] = y;
   constant[2] = z;
   constant[3] = w;
}

// caller sets up texture coordinates
static void do_screen_quad(gswf_recti *s, const F32 *tc, GDrawStats *stats)
{
   ID3D1XContext *d3d = gdraw->d3d_context;
   F32 px0 = static_cast<F32>(s->x0), py0 = static_cast<F32>(s->y0), px1 = static_cast<F32>(s->x1), py1 = static_cast<F32>(s->y1);

   // generate vertex data
   gswf_vertex_xyst *vert = static_cast<gswf_vertex_xyst *>(start_write_dyn(&gdraw->dyn_vb, 4 * sizeof(gswf_vertex_xyst)));
   if (!vert)
      return;

   vert[0].x = px0; vert[0].y = py0; vert[0].s = tc[0]; vert[0].t = tc[1];
   vert[1].x = px1; vert[1].y = py0; vert[1].s = tc[2]; vert[1].t = tc[1];
   vert[2].x = px0; vert[2].y = py1; vert[2].s = tc[0]; vert[2].t = tc[3];
   vert[3].x = px1; vert[3].y = py1; vert[3].s = tc[2]; vert[3].t = tc[3];
   UINT offs = end_write_dyn(&gdraw->dyn_vb);
   UINT stride = sizeof(gswf_vertex_xyst);

   if (VertexVars *vvars = static_cast<VertexVars *>(map_buffer(gdraw->d3d_context, gdraw->cb_vertex, true))) {
      gdraw_PixelSpace(vvars->world[0]);
      memcpy(vvars->x3d, gdraw->projmat, 12*sizeof(F32));
      unmap_buffer(gdraw->d3d_context, gdraw->cb_vertex);
      d3d->VSSetConstantBuffers(0, 1, &gdraw->cb_vertex);

      set_vertex_shader(d3d, gdraw->vert[GDRAW_vformat_v2tc2].vshader);

      d3d->IASetInputLayout(gdraw->inlayout[GDRAW_vformat_v2tc2]);
      d3d->IASetVertexBuffers(0, 1, &gdraw->dyn_vb.buffer, &stride, &offs);
      d3d->IASetPrimitiveTopology(D3D1X_(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP));
      d3d->Draw(4, 0);
      d3d->IASetPrimitiveTopology(D3D1X_(PRIMITIVE_TOPOLOGY_TRIANGLELIST));

      stats->nonzero_flags |= GDRAW_STATS_batches;
      stats->num_batches += 1;
      stats->drawn_indices += 6;
      stats->drawn_vertices += 4;
   }
}

static void manual_clear(gswf_recti *r, GDrawStats *stats)
{
   ID3D1XContext *d3d = gdraw->d3d_context;

   // go to known render state
   d3d->OMSetBlendState(gdraw->blend_state[GDRAW_BLEND_none], four_zeros, ~0u);
   d3d->OMSetDepthStencilState(gdraw->depth_state[0][0], 0);
   gdraw->blend_mode = GDRAW_BLEND_none;

   set_viewport_raw(0, 0, gdraw->frametex_width, gdraw->frametex_height);
   set_projection_raw(0, gdraw->frametex_width, gdraw->frametex_height, 0);
   set_pixel_shader(d3d, gdraw->clear_ps.pshader);

   if (PixelCommonVars *pvars = static_cast<PixelCommonVars *>(map_buffer(gdraw->d3d_context, gdraw->cb_ps_common, true))) {
      memset(pvars, 0, sizeof(*pvars));
      unmap_buffer(gdraw->d3d_context, gdraw->cb_ps_common);
      d3d->PSSetConstantBuffers(0, 1, &gdraw->cb_ps_common);

      do_screen_quad(r, four_zeros, stats);
   }
}

static void gdraw_DriverBlurPass(GDrawRenderState *r, int taps,  float *data, gswf_recti *s, float *tc, float /*height_max*/, float *clamp, GDrawStats *gstats)
{
   set_texture(0, r->tex[0], false, GDRAW_WRAP_clamp);

   set_pixel_shader(gdraw->d3d_context, gdraw->blur_prog[taps].pshader);
   PixelParaBlur *para = static_cast<PixelParaBlur *>(start_ps_constants(gdraw->cb_blur));
   memcpy(para->clamp, clamp, 4 * sizeof(float));
   memcpy(para->tap, data, taps * 4 * sizeof(float));
   end_ps_constants(gdraw->cb_blur);

   do_screen_quad(s, tc, gstats);
   tag_resources(r->tex[0]);
}

static void gdraw_Colormatrix(GDrawRenderState *r, gswf_recti *s, float *tc, GDrawStats *stats)
{
   if (!gdraw_TextureDrawBufferBegin(s, GDRAW_TEXTURE_FORMAT_rgba32, GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_color | GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_alpha, nullptr, stats))
      return;

   set_texture(0, r->tex[0], false, GDRAW_WRAP_clamp);
   set_pixel_shader(gdraw->d3d_context, gdraw->colormatrix.pshader);

   PixelParaColorMatrix *para = static_cast<PixelParaColorMatrix *>(start_ps_constants(gdraw->cb_colormatrix));
   memcpy(para->data, r->shader_data, 5 * 4 * sizeof(float));
   end_ps_constants(gdraw->cb_colormatrix);

   do_screen_quad(s, tc, stats);
   tag_resources(r->tex[0]);
   r->tex[0] = gdraw_TextureDrawBufferEnd(stats);
}

static gswf_recti *get_valid_rect(GDrawTexture *tex)
{
   GDrawHandle *h = (GDrawHandle *) tex;
   S32 n = static_cast<S32>(h - gdraw->rendertargets.handle);
   assert(n >= 0 && n <= MAX_RENDER_STACK_DEPTH+1);
   return &gdraw->rt_valid[n];
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

   set_texture(0, r->tex[0], false, GDRAW_WRAP_clamp);
   set_texture(1, r->tex[1], false, GDRAW_WRAP_clamp);
   set_texture(2, r->tex[2], false, GDRAW_WRAP_clamp);
   set_pixel_shader(gdraw->d3d_context, gdraw->filter_prog[isbevel][r->filter_mode].pshader);

   PixelParaFilter *para = static_cast<PixelParaFilter *>(start_ps_constants(gdraw->cb_filter));
   set_clamp_constant(para->clamp0, r->tex[0]);
   set_clamp_constant(para->clamp1, r->tex[1]);
   set_pixel_constant(para->color, r->shader_data[0], r->shader_data[1], r->shader_data[2], r->shader_data[3]);
   set_pixel_constant(para->color2, r->shader_data[8], r->shader_data[9], r->shader_data[10], r->shader_data[11]);
   set_pixel_constant(para->tc_off, -r->shader_data[4] / static_cast<F32>(gdraw->frametex_width), -r->shader_data[5] / static_cast<F32>(gdraw->frametex_height), r->shader_data[6], 0);
   end_ps_constants(gdraw->cb_filter);

   do_screen_quad(s, tc, stats);
   tag_resources(r->tex[0], r->tex[1], r->tex[2]);
   r->tex[0] = gdraw_TextureDrawBufferEnd(stats);
}

static void RADLINK gdraw_FilterQuad(GDrawRenderState *r, S32 x0, S32 y0, S32 x1, S32 y1, GDrawStats *stats)
{
   ID3D1XContext *d3d = gdraw->d3d_context;
   F32 tc[4];
   gswf_recti s;

   // clip to tile boundaries
   s.x0 = RR_MAX(x0, gdraw->tx0p);
   s.y0 = RR_MAX(y0, gdraw->ty0p);
   s.x1 = RR_MIN(x1, gdraw->tx0p + gdraw->tpw);
   s.y1 = RR_MIN(y1, gdraw->ty0p + gdraw->tph);
   if (s.x1 < s.x0 || s.y1 < s.y0)
      return;

   tc[0] = (s.x0 - gdraw->tx0p) / static_cast<F32>(gdraw->frametex_width);
   tc[1] = (s.y0 - gdraw->ty0p) / static_cast<F32>(gdraw->frametex_height);
   tc[2] = (s.x1 - gdraw->tx0p) / static_cast<F32>(gdraw->frametex_width);
   tc[3] = (s.y1 - gdraw->ty0p) / static_cast<F32>(gdraw->frametex_height);

   // clear to known render state
   d3d->OMSetBlendState(gdraw->blend_state[GDRAW_BLEND_none], four_zeros, ~0u);
   d3d->OMSetDepthStencilState(gdraw->depth_state[0][0], 0);
   disable_scissor(0);
   gdraw->blend_mode = GDRAW_BLEND_none;

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

            // do the blur
            gdraw_Blur(&gdraw_funcs, &b, r, &s, &bounds, stats);

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

      // for crazy blend modes, we need to read back from the framebuffer
      // and do the blending in the pixel shader. we do this with copies
      // rather than trying to render-to-texture-all-along, because we want
      // to be able to render over the user's existing framebuffer, which might
      // not be a texture. note that this isn't optimal when MSAA is on!
      F32 rescale1[4] = { 1.0f, 1.0f, 0.0f, 0.0f };
      if (r->blend_mode == GDRAW_BLEND_special) {
         ID3D1XContext *d3d = gdraw->d3d_context;
         ID3D1X(Resource) *cur_rt_rsrc;
         get_active_render_target()->GetResource(&cur_rt_rsrc);

         if (gdraw->cur == gdraw->frame && gdraw->main_msaa) {
            // source surface is main framebuffer and it uses MSAA. just resolve it first.
            D3D1X_(SHADER_RESOURCE_VIEW_DESC) desc;
            D3D1X_(TEXTURE2D_DESC) texdesc;
            ID3D1X(Texture2D) *resolve_tex;

            gdraw->main_resolve_target->GetDesc(&desc);
            gdraw->main_resolve_target->GetResource((ID3D1X(Resource) **) &resolve_tex);
            resolve_tex->GetDesc(&texdesc);
            d3d->ResolveSubresource(resolve_tex, 0, cur_rt_rsrc, 0, desc.Format);
            resolve_tex->Release();

            stats->nonzero_flags |= GDRAW_STATS_blits;
            stats->num_blits += 1;
            stats->num_blit_pixels += texdesc.Width * texdesc.Height;

            d3d->PSSetShaderResources(1, 1, &gdraw->main_resolve_target);
            d3d->PSSetSamplers(1, 1, &gdraw->sampler_state[0][GDRAW_WRAP_clamp]);

            // calculate texture coordinate remapping
            rescale1[0] = gdraw->frametex_width / static_cast<F32>(texdesc.Width);
            rescale1[1] = gdraw->frametex_height / static_cast<F32>(texdesc.Height);
            rescale1[2] = (gdraw->vx - gdraw->tx0 + gdraw->tx0p) / static_cast<F32>(texdesc.Width);
            rescale1[3] = (gdraw->vy - gdraw->ty0 + gdraw->ty0p) / static_cast<F32>(texdesc.Height);
         } else {
            D3D1X_(BOX) box = { 0,0,0,0,0,1 };
            S32 dx = 0, dy = 0;
            blend_tex = get_color_rendertarget(stats);

            if (gdraw->cur != gdraw->frame)
               box.right=gdraw->tpw, box.bottom=gdraw->tph;
            else {
               box.left=gdraw->vx, box.top=gdraw->vy, box.right=gdraw->vx+gdraw->tw, box.bottom=gdraw->vy+gdraw->th;
               dx = gdraw->tx0 - gdraw->tx0p;
               dy = gdraw->ty0 - gdraw->ty0p;
            }

            d3d->CopySubresourceRegion(blend_tex->handle.tex.d3d, 0, dx, dy, 0,
               cur_rt_rsrc, 0, &box);

            stats->nonzero_flags |= GDRAW_STATS_blits;
            stats->num_blits += 1;
            stats->num_blit_pixels += (box.right - box.left) * (box.bottom - box.top);

            set_texture(1, (GDrawTexture *) blend_tex, false, GDRAW_WRAP_clamp);
         }
         
         cur_rt_rsrc->Release();
      }

      if (!set_renderstate_full(GDRAW_vformat_v2tc2, r, stats, rescale1))
         return;

      do_screen_quad(&s, tc, stats);
      tag_resources(r->tex[0], r->tex[1]);
      if (blend_tex)
         gdraw_FreeTexture((GDrawTexture *) blend_tex, nullptr, stats);
   }
}

///////////////////////////////////////////////////////////////////////
//
//   Shaders and state
//

#include GDRAW_SHADER_FILE

static void destroy_shader(ProgramWithCachedVariableLocations *p)
{
   if (p->pshader) {
      p->pshader->Release();
      p->pshader = nullptr;
   }
}

static ID3D1X(Buffer) *create_dynamic_buffer(U32 size, U32 bind)
{
   D3D1X_(BUFFER_DESC) desc = { size, D3D1X_(USAGE_DYNAMIC), bind, D3D1X_(CPU_ACCESS_WRITE), 0 };
   ID3D1X(Buffer) *buf = nullptr;
   HRESULT hr = gdraw->d3d_device->CreateBuffer(&desc, nullptr, &buf);
   if (FAILED(hr)) {
      report_d3d_error(hr, "CreateBuffer", " creating dynamic vertex buffer");
      buf = nullptr;
   }
   return buf;
}

static void init_dyn_buffer(DynBuffer *buf, U32 size, U32 bind)
{
   buf->buffer = create_dynamic_buffer(size, bind);
   buf->size = size;
   buf->write_pos = 0;
   buf->alloc_pos = 0;
}

// These two functions are implemented by the D3D10- respectively D3D11-specific part.
static void create_pixel_shader(ProgramWithCachedVariableLocations *p, ProgramWithCachedVariableLocations *src);
static void create_vertex_shader(ProgramWithCachedVariableLocations *p, ProgramWithCachedVariableLocations *src);

static void create_all_shaders_and_state(void)
{
   ID3D1X(Device) *d3d = gdraw->d3d_device;
   HRESULT hr;
   S32 i, j;

   for (i=0; i < GDRAW_TEXTURE__count*3; ++i)       create_pixel_shader(&gdraw->fprog[0][i], pshader_basic_arr + i);
   for (i=0; i < GDRAW_BLENDSPECIAL__count; ++i)    create_pixel_shader(&gdraw->exceptional_blend[i], pshader_exceptional_blend_arr + i);
   for (i=0; i < 32; ++i)                           create_pixel_shader(&gdraw->filter_prog[0][i], pshader_filter_arr + i);
   for (i=0; i < MAX_TAPS+1; ++i)                   create_pixel_shader(&gdraw->blur_prog[i], pshader_blur_arr + i);
   create_pixel_shader(&gdraw->colormatrix, pshader_color_matrix_arr);
   create_pixel_shader(&gdraw->clear_ps, pshader_manual_clear_arr);

   for (i=0; i < GDRAW_vformat__basic_count; i++) {
      ProgramWithCachedVariableLocations *vsh = vshader_vsd3d10_arr + i;

      create_vertex_shader(&gdraw->vert[i], vsh);
      HRESULT hr = d3d->CreateInputLayout(vformats[i].desc, vformats[i].nelem, vsh->bytecode, vsh->size, &gdraw->inlayout[i]);
      if (FAILED(hr)) {
         report_d3d_error(hr, "CreateInputLayout", "");
         gdraw->inlayout[i] = nullptr;
      }
   }

   // create rasterizer state setups
   for (i=0; i < 2; ++i) {
      D3D1X_(RASTERIZER_DESC) raster_desc = { D3D1X_(FILL_SOLID), D3D1X_(CULL_NONE), FALSE, 0, 0.0f, 0.0f, TRUE, TRUE, FALSE, FALSE };
      raster_desc.MultisampleEnable = i;
      hr = d3d->CreateRasterizerState(&raster_desc, &gdraw->raster_state[i]);
      if (FAILED(hr)) {
         report_d3d_error(hr, "CreateRasterizerState", "");
         return;
      }
   }

   // create sampler state setups
   static const D3D1X_(TEXTURE_ADDRESS_MODE) addrmode[ASSERT_COUNT(GDRAW_WRAP__count, 4)] = {
      D3D1X_(TEXTURE_ADDRESS_CLAMP),  // GDRAW_WRAP_clamp
      D3D1X_(TEXTURE_ADDRESS_WRAP),   // GDRAW_WRAP_repeat
      D3D1X_(TEXTURE_ADDRESS_MIRROR), // GDRAW_WRAP_mirror
      D3D1X_(TEXTURE_ADDRESS_CLAMP),  // GDRAW_WRAP_clamp_to_border (unused for this renderer)
   };

   for (i=0; i < 2; ++i) {
      for (j=0; j < GDRAW_WRAP__count; ++j) {
         D3D1X_(SAMPLER_DESC) sampler_desc;
         memset(&sampler_desc, 0, sizeof(sampler_desc));
         sampler_desc.Filter = i ? D3D1X_(FILTER_MIN_LINEAR_MAG_MIP_POINT) : D3D1X_(FILTER_MIN_MAG_MIP_LINEAR);
         sampler_desc.AddressU = addrmode[j];
         sampler_desc.AddressV = addrmode[j];
         sampler_desc.AddressW = D3D1X_(TEXTURE_ADDRESS_CLAMP);
         sampler_desc.MaxAnisotropy = 1;
         sampler_desc.MaxLOD = D3D1X_(FLOAT32_MAX);
         hr = d3d->CreateSamplerState(&sampler_desc, &gdraw->sampler_state[i][j]);
         if (FAILED(hr)) {
            report_d3d_error(hr, "CreateSamplerState", "");
            return;
         }
      }
   }

   // create blend stage setups
   static struct blendspec {
      BOOL blend;
      D3D1X_(BLEND) src;
      D3D1X_(BLEND) dst;
   } blends[ASSERT_COUNT(GDRAW_BLEND__count, 6)] = {
      FALSE,   D3D1X_(BLEND_ONE),        D3D1X_(BLEND_ZERO),          // GDRAW_BLEND_none
      TRUE,    D3D1X_(BLEND_ONE),        D3D1X_(BLEND_INV_SRC_ALPHA), // GDRAW_BLEND_alpha
      TRUE,    D3D1X_(BLEND_DEST_COLOR), D3D1X_(BLEND_INV_SRC_ALPHA), // GDRAW_BLEND_multiply
      TRUE,    D3D1X_(BLEND_ONE),        D3D1X_(BLEND_ONE),           // GDRAW_BLEND_add

      FALSE,   D3D1X_(BLEND_ONE),        D3D1X_(BLEND_ZERO),          // GDRAW_BLEND_filter
      FALSE,   D3D1X_(BLEND_ONE),        D3D1X_(BLEND_ZERO),          // GDRAW_BLEND_special
   };

   for (i=0; i < GDRAW_BLEND__count; ++i) {
      gdraw->blend_state[i] = create_blend_state(d3d, blends[i].blend, blends[i].src, blends[i].dst);
      if (!gdraw->blend_state[i])
         return;
   }

   D3D1X_(BLEND_DESC) blend_desc;
   memset(&blend_desc, 0, sizeof(blend_desc));
   hr = d3d->CreateBlendState(&blend_desc, &gdraw->blend_no_color_write);
   if (FAILED(hr)) {
      report_d3d_error(hr, "CreateBlendState", "");
      return;
   }

   // create depth/stencil setups
   for (i=0; i < 2; ++i) {
      for (j=0; j < 2; ++j) {
         D3D1X_(DEPTH_STENCIL_DESC) depth_desc;
         memset(&depth_desc, 0, sizeof(depth_desc));

         depth_desc.DepthEnable = (i || j);
         depth_desc.DepthWriteMask = i ? D3D1X_(DEPTH_WRITE_MASK_ALL) : D3D1X_(DEPTH_WRITE_MASK_ZERO);
         depth_desc.DepthFunc = j ? D3D1X_(COMPARISON_LESS) : D3D1X_(COMPARISON_ALWAYS);
         depth_desc.StencilEnable = FALSE;

         hr = d3d->CreateDepthStencilState(&depth_desc, &gdraw->depth_state[i][j]);
         if (FAILED(hr)) {
            report_d3d_error(hr, "CreateDepthStencilState", "");
            return;
         }
      }
   }

   // constant buffers
   gdraw->cb_vertex = create_dynamic_buffer(sizeof(VertexVars), D3D1X_(BIND_CONSTANT_BUFFER));
   gdraw->cb_ps_common = create_dynamic_buffer(sizeof(PixelCommonVars), D3D1X_(BIND_CONSTANT_BUFFER));
   gdraw->cb_filter = create_dynamic_buffer(sizeof(PixelParaFilter), D3D1X_(BIND_CONSTANT_BUFFER));
   gdraw->cb_colormatrix = create_dynamic_buffer(sizeof(PixelParaColorMatrix), D3D1X_(BIND_CONSTANT_BUFFER));
   gdraw->cb_blur = create_dynamic_buffer(sizeof(PixelParaBlur), D3D1X_(BIND_CONSTANT_BUFFER));
   
   // quad index buffer
   assert(QUAD_IB_COUNT * 4 < 65535); // can't use more; we have 16-bit index buffers and 0xffff = primitive cut index
   U16 *inds = (U16 *) IggyGDrawMalloc(QUAD_IB_COUNT * 6 * sizeof(U16));
   if (inds) {
      D3D1X_(BUFFER_DESC) bufdesc = { };
      D3D1X_(SUBRESOURCE_DATA) data = { inds, 0, 0 };

      bufdesc.ByteWidth = QUAD_IB_COUNT * 6 * sizeof(U16);
      bufdesc.Usage = D3D1X_(USAGE_IMMUTABLE);
      bufdesc.BindFlags = D3D1X_(BIND_INDEX_BUFFER);

      for (U16 i=0; i < QUAD_IB_COUNT; i++) {
         inds[i*6 + 0] = i*4 + 0;
         inds[i*6 + 1] = i*4 + 1;
         inds[i*6 + 2] = i*4 + 2;
         inds[i*6 + 3] = i*4 + 0;
         inds[i*6 + 4] = i*4 + 2;
         inds[i*6 + 5] = i*4 + 3;
      }

      hr = gdraw->d3d_device->CreateBuffer(&bufdesc, &data, &gdraw->quad_ib);
      if (FAILED(hr)) {
         report_d3d_error(hr, "CreateBuffer", " for constants");
         gdraw->quad_ib = nullptr;
      }
      IggyGDrawFree(inds);
   } else
      gdraw->quad_ib = nullptr;
}

static void destroy_all_shaders_and_state()
{
   S32 i;

   for (i=0; i < GDRAW_TEXTURE__count*3; ++i)       destroy_shader(&gdraw->fprog[0][i]);
   for (i=0; i < GDRAW_BLENDSPECIAL__count; ++i)    destroy_shader(&gdraw->exceptional_blend[i]);
   for (i=0; i < 32; ++i)                           destroy_shader(&gdraw->filter_prog[0][i]);
   for (i=0; i < MAX_TAPS+1; ++i)                   destroy_shader(&gdraw->blur_prog[i]);
   destroy_shader(&gdraw->colormatrix);
   destroy_shader(&gdraw->clear_ps);

   for (i=0; i < GDRAW_vformat__basic_count; i++) {
      safe_release(gdraw->inlayout[i]);
      destroy_shader(&gdraw->vert[i]);
   }

   for (i=0; i < 2; ++i)                     safe_release(gdraw->raster_state[i]);
   for (i=0; i < GDRAW_WRAP__count*2; ++i)   safe_release(gdraw->sampler_state[0][i]);
   for (i=0; i < GDRAW_BLEND__count; ++i)    safe_release(gdraw->blend_state[i]);
   for (i=0; i < 2*2; ++i)                   safe_release(gdraw->depth_state[0][i]);

   safe_release(gdraw->blend_no_color_write);

   safe_release(gdraw->cb_vertex);
   safe_release(gdraw->cb_ps_common);
   safe_release(gdraw->cb_filter);
   safe_release(gdraw->cb_colormatrix);
   safe_release(gdraw->cb_blur);

   safe_release(gdraw->quad_ib);
}

////////////////////////////////////////////////////////////////////////
//
//   Create and tear-down the state
//

typedef struct
{
   S32 num_handles;
   S32 num_bytes;
} GDrawResourceLimit;

// These are the defaults limits used by GDraw unless the user specifies something else.
static GDrawResourceLimit gdraw_limits[GDRAW_D3D1X_(RESOURCE__count)] = {
   MAX_RENDER_STACK_DEPTH + 1, 16*1024*1024,  // RESOURCE_rendertarget
    500,                       16*1024*1024,  // RESOURCE_texture
   1000,                        2*1024*1024,  // RESOURCE_vertexbuffer
      0,                           256*1024,  // RESOURCE_dynbuffer
};

static GDrawHandleCache *make_handle_cache(gdraw_resourcetype type)
{
   S32 num_handles = gdraw_limits[type].num_handles;
   S32 num_bytes = gdraw_limits[type].num_bytes;
   GDrawHandleCache *cache = (GDrawHandleCache *) IggyGDrawMalloc(sizeof(GDrawHandleCache) + (num_handles - 1) * sizeof(GDrawHandle));
   if (cache) {
      gdraw_HandleCacheInit(cache, num_handles, num_bytes);
      cache->is_vertex = (type == GDRAW_D3D1X_(RESOURCE_vertexbuffer));
   }

   return cache;
}

static void free_gdraw()
{
   if (!gdraw) return;
   if (gdraw->texturecache) IggyGDrawFree(gdraw->texturecache);
   if (gdraw->vbufcache) IggyGDrawFree(gdraw->vbufcache);
   IggyGDrawFree(gdraw);
   gdraw = nullptr;
}

static bool alloc_dynbuffer(U32 size)
{
   // specified input size is vertex buffer size. determine sensible size for the
   // corresponding index buffer. iggy always uses 16-bit indices and has three
   // primary types of geometry it sends:
   //
   // 1. filled polygons. these are triangulated simple polygons and thus have
   //    roughly as many triangles as they have vertices. they use either 8- or
   //    16-byte vertex formats; this makes a worst case of 6 bytes of indices
   //    for every 8 bytes of vertex data.
   // 2. strokes and edge antialiasing. they use a 16-byte vertex format and
   //    worst-case write a "double quadstrip" which has 4 triangles for every
   //    3 vertices, which means 24 bytes of index data for every 48 bytes
   //    of vertex data.
   // 3. textured quads. they use a 16-byte vertex format, have exactly 2
   //    triangles for every 4 vertices, and use either a static index buffer
   //    (quad_ib) or a single triangle strip, so for our purposes they need no
   //    space to store indices at all.
   //
   // 1) argues for allocating index buffers at 3/4 the size of the corresponding
   // vertex buffer, while 2) and 3) need 1/2 the size of the vertex buffer or less.
   // 2) and 3) are the most common types of vertex data, while 1) is used only for
   // morphed shapes and in certain cases when the RESOURCE_vertexbuffer pool is full.
   // we just play it safe anyway and make sure we size the IB large enough to cover
   // the worst case for 1). this is conservative, but it probably doesn't matter much.
   U32 ibsize = (size * 3) / 4;

   init_dyn_buffer(&gdraw->dyn_vb, size, D3D1X_(BIND_VERTEX_BUFFER));
   init_dyn_buffer(&gdraw->dyn_ib, ibsize, D3D1X_(BIND_INDEX_BUFFER));

   gdraw->max_quad_vert_count = RR_MIN(size / sizeof(gswf_vertex_xyst), QUAD_IB_COUNT * 4);
   gdraw->max_quad_vert_count &= ~3; // must be multiple of four

   return gdraw->dyn_vb.buffer != nullptr && gdraw->dyn_ib.buffer != nullptr;
}

int gdraw_D3D1X_(SetResourceLimits)(gdraw_resourcetype type, S32 num_handles, S32 num_bytes)
{
   GDrawStats stats={0};

   if (type == GDRAW_D3D1X_(RESOURCE_rendertarget)) // RT count is small and space is preallocated
      num_handles = MAX_RENDER_STACK_DEPTH + 1;

   assert(type >= GDRAW_D3D1X_(RESOURCE_rendertarget) && type < GDRAW_D3D1X_(RESOURCE__count));
   assert(num_handles >= 0);
   assert(num_bytes >= 0);

   // nothing to do if the values are unchanged
   if (gdraw_limits[type].num_handles == num_handles &&
      gdraw_limits[type].num_bytes == num_bytes)
      return 1;

   gdraw_limits[type].num_handles = num_handles;
   gdraw_limits[type].num_bytes = num_bytes;

   // if no gdraw context created, there's nothing to worry about
   if (!gdraw)
      return 1;

   // resize the appropriate pool
   switch (type) {
      case GDRAW_D3D1X_(RESOURCE_rendertarget):
         flush_rendertargets(&stats);
         gdraw_HandleCacheInit(&gdraw->rendertargets, num_handles, num_bytes);
         return 1;
         
      case GDRAW_D3D1X_(RESOURCE_texture):
         if (gdraw->texturecache) {
            gdraw_res_flush(gdraw->texturecache, &stats);
            IggyGDrawFree(gdraw->texturecache);
         }
         gdraw->texturecache = make_handle_cache(GDRAW_D3D1X_(RESOURCE_texture));
         return gdraw->texturecache != nullptr;

      case GDRAW_D3D1X_(RESOURCE_vertexbuffer):
         if (gdraw->vbufcache) {
            gdraw_res_flush(gdraw->vbufcache, &stats);
            IggyGDrawFree(gdraw->vbufcache);
         }
         gdraw->vbufcache = make_handle_cache(GDRAW_D3D1X_(RESOURCE_vertexbuffer));
         return gdraw->vbufcache != nullptr;

      case GDRAW_D3D1X_(RESOURCE_dynbuffer):
         unbind_resources();
         safe_release(gdraw->dyn_vb.buffer);
         safe_release(gdraw->dyn_ib.buffer);
         return alloc_dynbuffer(num_bytes);

      default:
         return 0;
   }
}

static GDrawFunctions *create_context(ID3D1XDevice *dev, ID3D1XContext *ctx, S32 w, S32 h)
{
   gdraw = (GDraw *) IggyGDrawMalloc(sizeof(*gdraw));
   if (!gdraw) return nullptr;

   memset(gdraw, 0, sizeof(*gdraw));

   gdraw->frametex_width = w;
   gdraw->frametex_height = h;
   gdraw->d3d_device = dev;
   gdraw->d3d_context = ctx;

   gdraw->texturecache = make_handle_cache(GDRAW_D3D1X_(RESOURCE_texture));
   gdraw->vbufcache = make_handle_cache(GDRAW_D3D1X_(RESOURCE_vertexbuffer));
   gdraw_HandleCacheInit(&gdraw->rendertargets, gdraw_limits[GDRAW_D3D1X_(RESOURCE_rendertarget)].num_handles, gdraw_limits[GDRAW_D3D1X_(RESOURCE_rendertarget)].num_bytes);

   if (!gdraw->texturecache || !gdraw->vbufcache || !alloc_dynbuffer(gdraw_limits[GDRAW_D3D1X_(RESOURCE_dynbuffer)].num_bytes)) {
      free_gdraw();
      return nullptr;
   }

   create_all_shaders_and_state();

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

   gdraw_funcs.MakeTextureFromResource = (gdraw_make_texture_from_resource *) gdraw_D3D1X_(MakeTextureFromResource);
   gdraw_funcs.FreeTextureFromResource = gdraw_D3D1X_(DestroyTextureFromResource);

   gdraw_funcs.MakeVertexBufferBegin = gdraw_MakeVertexBufferBegin;
   gdraw_funcs.MakeVertexBufferMore  = gdraw_MakeVertexBufferMore;
   gdraw_funcs.MakeVertexBufferEnd   = gdraw_MakeVertexBufferEnd;
   gdraw_funcs.TryToLockVertexBuffer = gdraw_TryLockVertexBuffer;
   gdraw_funcs.FreeVertexBuffer = gdraw_FreeVertexBuffer;

   gdraw_funcs.UnlockHandles = gdraw_UnlockHandles;
   gdraw_funcs.SetTextureUniqueID = gdraw_SetTextureUniqueID;

   gdraw_funcs.Set3DTransform = gdraw_Set3DTransform;

   return &gdraw_funcs;
}

void gdraw_D3D1X_(DestroyContext)(void)
{
   if (gdraw && gdraw->d3d_device) {
      GDrawStats stats={0};
      clear_renderstate();
      stencil_state_cache_clear();
      destroy_all_shaders_and_state();
      safe_release(gdraw->aa_tex);
      safe_release(gdraw->aa_tex_view);
      safe_release(gdraw->dyn_vb.buffer);
      safe_release(gdraw->dyn_ib.buffer);

      flush_rendertargets(&stats);
      if (gdraw->texturecache)   gdraw_res_flush(gdraw->texturecache, &stats);
      if (gdraw->vbufcache)      gdraw_res_flush(gdraw->vbufcache, &stats);

      gdraw->d3d_device = nullptr;
   }

   free_gdraw();
}

void gdraw_D3D1X_(SetErrorHandler)(void (__cdecl *error_handler)(HRESULT hr))
{
   if (gdraw)
      gdraw->error_handler = error_handler;
}

void gdraw_D3D1X_(PreReset)(void)
{
   if (!gdraw) return;

   GDrawStats stats={0};
   flush_rendertargets(&stats);

   // we may end up resizing the frame buffer
   gdraw->frametex_width = 0;
   gdraw->frametex_height = 0;
}

void gdraw_D3D1X_(PostReset)(void)
{
   // maybe re-create rendertargets right now?
}

void RADLINK gdraw_D3D1X_(BeginCustomDraw)(IggyCustomDrawCallbackRegion * region, F32 mat[4][4])
{
   clear_renderstate();
   gdraw_GetObjectSpaceMatrix(mat[0], region->o2w, gdraw->projection, 0, 0);
}

void RADLINK gdraw_D3D1X_(BeginCustomDraw_4J)(IggyCustomDrawCallbackRegion * region, F32 mat[16])
{
   clear_renderstate();
   gdraw_GetObjectSpaceMatrix(mat, region->o2w, gdraw->projection, 0, 0);
}

void RADLINK gdraw_D3D1X_(CalculateCustomDraw_4J)(IggyCustomDrawCallbackRegion * region, F32 mat[16])
{
   gdraw_GetObjectSpaceMatrix(mat, region->o2w, gdraw->projection, 0, 0);
}

void RADLINK gdraw_D3D1X_(EndCustomDraw)(IggyCustomDrawCallbackRegion * /*region*/)
{
   GDrawStats stats={};
   set_common_renderstate();
   set_viewport();
   set_render_target(&stats);
}

void RADLINK gdraw_D3D1X_(GetResourceUsageStats)(gdraw_resourcetype type, S32 *handles_used, S32 *bytes_used)
{
   GDrawHandleCache *cache;

   switch (type) {
      case GDRAW_D3D1X_(RESOURCE_rendertarget):    cache = &gdraw->rendertargets; break;
      case GDRAW_D3D1X_(RESOURCE_texture):         cache = gdraw->texturecache; break;
      case GDRAW_D3D1X_(RESOURCE_vertexbuffer):    cache = gdraw->vbufcache; break;
      case GDRAW_D3D1X_(RESOURCE_dynbuffer):       *handles_used = 0; *bytes_used = gdraw->last_dyn_maxalloc; return;
      default:                                     cache = nullptr; break;
   }

   *handles_used = *bytes_used = 0;

   if (cache) {
      S32 i;
      U64 frame = gdraw->frame_counter;

      for (i=0; i < cache->max_handles; ++i)
         if (cache->handle[i].bytes && cache->handle[i].owner && cache->handle[i].fence.value == frame) {
            *handles_used += 1;
            *bytes_used += cache->handle[i].bytes;
         }
   }
}

static S32 num_pixels(S32 w, S32 h, S32 mipmaps)
{
   S32 k, pixels=0;
   for (k=0; k < mipmaps; ++k) {
      pixels += w*h*2;
      w = (w>>1); w += !w;
      h = (h>>1); h += !h;
   }
   return pixels;
}

GDrawTexture * RADLINK gdraw_D3D1X_(MakeTextureFromResource)(U8 *resource_file, S32 /*len*/, IggyFileTextureRaw *texture)
{
   const char *failed_call="";
   U8 *free_data = nullptr;
   GDrawTexture *t=nullptr;
   S32 width, height, mipmaps, size, blk;
   ID3D1X(Texture2D) *tex=nullptr;
   ID3D1X(ShaderResourceView) *view=nullptr;

   DXGI_FORMAT d3dfmt;
   D3D1X_(SUBRESOURCE_DATA) mipdata[24] = { nullptr };
   S32 k;

   HRESULT hr = S_OK;

   width = texture->w;
   height = texture->h;
   mipmaps = texture->mipmaps;
   blk = 1;

   D3D1X_(TEXTURE2D_DESC) desc = { static_cast<U32>(width), static_cast<U32>(height), static_cast<U32>(mipmaps), 1U, DXGI_FORMAT_UNKNOWN, { 1, 0 },
      D3D1X_(USAGE_IMMUTABLE), D3D1X_(BIND_SHADER_RESOURCE), 0U, 0U };

   bool done = false;

   switch (texture->format) {
      case IFT_FORMAT_rgba_8888   : size= 4; d3dfmt = DXGI_FORMAT_R8G8B8A8_UNORM; break;
      case IFT_FORMAT_DXT1        : size= 8; d3dfmt = DXGI_FORMAT_BC1_UNORM; blk = 4; break;
      case IFT_FORMAT_DXT3        : size=16; d3dfmt = DXGI_FORMAT_BC2_UNORM; blk = 4; break;
      case IFT_FORMAT_DXT5        : size=16; d3dfmt = DXGI_FORMAT_BC3_UNORM; blk = 4; break;
      default: {
         IggyGDrawSendWarning(nullptr, "GDraw .iggytex raw texture format %d not supported by hardware", texture->format);
         done = true;
      }
   }

   if (!done) {
      desc.Format = d3dfmt;
      
      U8 *data = resource_file + texture->file_offset;
      
      if (texture->format == IFT_FORMAT_i_8 || texture->format == IFT_FORMAT_i_4) {
         // convert from intensity to luma+alpha
         S32 i;
         S32 total_size = 2 * num_pixels(width,height,mipmaps);
      
         free_data = (U8 *) IggyGDrawMalloc(total_size);
         if (!free_data) {
            IggyGDrawSendWarning(nullptr, "GDraw out of memory to store texture data to pass to D3D for %d x %d texture", width, height);
            done = true;
         } else {
            U8 *cur = free_data;
            
            for (k=0; k < mipmaps; ++k) {
               S32 w = RR_MAX(width >> k, 1);
               S32 h = RR_MAX(height >> k, 1);
               for (i=0; i < w*h; ++i) {
                  cur[0] = cur[1] = *data++;
                  cur += 2;
               }
            }
            data = free_data;
         }
      }
      
      if (!done) {
         for (k=0; k < mipmaps; ++k) {
            S32 w = RR_MAX(width >> k, 1);
            S32 h = RR_MAX(height >> k, 1);
            S32 blkw = (w + blk-1) / blk;
            S32 blkh = (h + blk-1) / blk;
         
            mipdata[k].pSysMem     = data;
            mipdata[k].SysMemPitch = blkw * size;
            data += blkw * blkh * size;
         }
         
         failed_call = "CreateTexture2D";
         hr = gdraw->d3d_device->CreateTexture2D(&desc, mipdata, &tex);
         if (!FAILED(hr)) {
             failed_call = "CreateShaderResourceView for texture creation";
             hr = gdraw->d3d_device->CreateShaderResourceView(tex, nullptr, &view);
             if (!FAILED(hr))
                 t = gdraw_D3D1X_(WrappedTextureCreate)(view);
         }
      }
   }

   if (FAILED(hr)) {
      report_d3d_error(hr, failed_call, "");
   }

   if (free_data)
      IggyGDrawFree(free_data);

   if (!t) {
      if (view)
         view->Release();
      if (tex)
         tex->Release();
   } else {
      reinterpret_cast<GDrawHandle *>(t)->handle.tex.d3d = tex;
   }
   return t;
}

void RADLINK gdraw_D3D1X_(DestroyTextureFromResource)(GDrawTexture *tex)
{
   GDrawHandle *h = reinterpret_cast<GDrawHandle *>(tex);
   safe_release(h->handle.tex.d3d_view);
   safe_release(h->handle.tex.d3d);
   gdraw_D3D1X_(WrappedTextureDestroy)(tex);
}

