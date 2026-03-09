// gdraw_gl_shared.inl - copyright 2012 RAD Game Tools
//
// This file implements the part of the Iggy graphics driver layer shared between
// GL and GL ES 2 (which is most of it). It heavily depends on a bunch of typedefs,
// #defines and some utility functions that need to be set up correctly for the GL
// version being targeted. It also targets a kind of pseudo-GL 2.0; the platform
// implementation has to set up some #defines and perform extra initialization
// work if we go through extensions instead. This is all a bit ugly, but much
// easier to maintain than the original solution, where we just kept two almost
// identical versions of this code.

/////////////////////////////////////////////////////////////
//
// common code shared by all GDraw implemetations
//

// The native handle type holds resource handles and a coarse description.
typedef union {
   // handle that is a texture
   struct {
      GLuint gl;
      GLuint gl_renderbuf;
      U32 w:24;
      U32 nonpow2:8;
      U32 h:24;
      U32 reserved:8;
   } tex;

   // handle that is a vertex buffer
   struct {
      GLuint base;
      GLuint indices;
   } vbuf;
} GDrawNativeHandle;

#include "gdraw_shared.inl"

// max rendertarget stack depth. this depends on the extent to which you
// use filters and non-standard blend modes, and how nested they are.
#define MAX_RENDER_STACK_DEPTH             8         // Iggy is hardcoded to a limit of 16... probably 1-3 is realistic
#define AATEX_SAMPLER                      3         // sampler that aa_tex gets set in
#define QUAD_IB_COUNT                      4096      // quad index buffer has indices for this many quads

#define ASSERT_COUNT(a,b)                  ((a) == (b) ? (b) : -1)

///////////////////////////////////////////////////////////////////////////////
//
//  debugging/validation
//

static RADINLINE void break_on_err(GLint e)
{
#ifdef _DEBUG
   if (e) {
      RR_BREAK();
   }
#endif
}

static void report_err(GLint e)
{
   break_on_err(e);
   IggyGDrawSendWarning(nullptr, "OpenGL glGetError error");
}

static void compilation_err(const char *msg)
{
   error_msg_platform_specific(msg);
   report_err(GL_INVALID_VALUE);
}

static void eat_gl_err(void)
{
   while (glGetError() != GL_NO_ERROR);
}

static void opengl_check(void)
{
#ifdef _DEBUG
   GLint e = glGetError();
   if (e != GL_NO_ERROR) {
      report_err(e);
      eat_gl_err();
   }
#endif
}

static U32 is_pow2(S32 n)
{
   return ((U32) n & (U32) (n-1)) == 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//  GDraw
//
//  This data structure stores all the data for the GDraw, just to keep
//  it a bit cleaner instead of storing in globals, even though GDraw is
//  a singleton.

// fragment and vertex program

// The mac doesn't use extensions for the functions dealing with programs, and the non-extension versions
// take GLuint instead of GLhandle. The mac defines GDrawGLProgram to GLuint before including gdraw_gl_shared.inl
// to account for this. 
#ifndef GDrawGLProgram
#define GDrawGLProgram GLhandle
#endif

typedef struct ProgramWithCachedVariableLocations
{
   GDrawGLProgram program;
   GLint    vars[2][MAX_VARS];
} ProgramWithCachedVariableLocations;

// render-stack state
typedef struct
{
   GDrawHandle *color_buffer;
   GDrawHandle *stencil_depth;
   S32 base_x, base_y, width, height;
   rrbool cached;
} GDrawFramebufferState;

// texture format description
typedef struct {
   U8 iggyfmt;       // IFT_FORMAT_*
   U8 blkx, blky;    // compressed block size in pixels (for compressed formats)
   U8 blkbytes;      // block bytes
   GLenum intfmt;    // GL internal format
   GLenum fmt;       // GL_TEXTURE_COMPRESSED for compressed formats!
   GLenum type;
} TextureFormatDesc;

static GDrawFunctions gdraw_funcs;

///////////////////////////////////////////////////////////////////////////////
//
//     GDraw data structure
//
//
// This is the primary rendering abstraction, which hides all
// the platform-specific rendering behavior from /G/. It is
// full of platform-specific graphics state, and also general
// graphics state so that it doesn't have to callback into /G/
// to get at that graphics state.

static struct
{
   S32 multisampling;  // number of samples if multisampling (always 0 if no GDRAW_MULTISAMPLING)

   S32 vx,vy;   // viewport width/height in pixels
   S32 fw,fh;   // full width/height of bound rendertarget
   S32 tw,th;   // actual width/height of current tile
   S32 tpw,tph; // width/height of padded version of tile

   // tile origin location (without and with padding)
   rrbool tile_enabled;
   S32 tx0,ty0;
   S32 tx0p,ty0p;

   // if we're in the middle of rendering a blur, certain viewport-related
   // functions have to behave differently, so they check this flag
   rrbool in_blur;

   F32 projection[4]; // scalex, scaley, transx, transy

   // conversion from worldspace to viewspace <0,0>..<w,h> -- no translation or rotation
   F32 world_to_pixel[2];

   // 3d transformation
   F32 xform_3d[3][4];
   rrbool use_3d;

   // render-state stack for 'temporary' rendering
   GDrawFramebufferState frame[MAX_RENDER_STACK_DEPTH];
   GDrawFramebufferState *cur;

   // texture and vertex buffer pools
   GDrawHandleCache *texturecache;
   GDrawHandleCache *vbufcache;

   // GL_EXT_separate_shader_objects isn't sufficiently standard,
   // so we have to bind every vertex shader to every fragment shader

   // raw vertex shaders
   GLuint vert[GDRAW_vformat__count];

   // fragment shaders with vertex shaders
   ProgramWithCachedVariableLocations fprog[GDRAW_TEXTURE__count][3][3]; // [tex0mode][additive][vformat]
   ProgramWithCachedVariableLocations ihud[2];

   // fragment shaders with fixed-function
   ProgramWithCachedVariableLocations exceptional_blend[GDRAW_BLENDSPECIAL__count];
   ProgramWithCachedVariableLocations filter_prog[2][16];
   ProgramWithCachedVariableLocations blur_prog[MAX_TAPS+1];
   ProgramWithCachedVariableLocations colormatrix;
   ProgramWithCachedVariableLocations manual_clear;

   // render targets

   // these two lines must be adjacent because of how rendertargets works
   GDrawHandleCache      rendertargets;
   GDrawHandle           rendertarget_handles[MAX_RENDER_STACK_DEPTH]; // not -1, because we use +1 to initialize

   gswf_recti            rt_valid[MAX_RENDER_STACK_DEPTH + 1];
   GDrawHandle           stencil_depth;

   // size of our render targets
   S32 frametex_width, frametex_height;

   // framebuffer object used for render-to-texture
   GLuint framebuffer_stack_object;

   // framebuffer object used to copy from MSAA renderbuffer to texture
   GLuint framebuffer_copy_to_texture;

   // framebuffer object used for main screen (set to non-0 to do render to texture)
   GLuint main_framebuffer;

   // antialias texture
   GLuint aa_tex;

   // canned quad indices
   GLuint quad_ib;

   // texture formats
   const TextureFormatDesc *tex_formats;

   // caps
   U32 has_conditional_non_power_of_two : 1; // non-power-of-2 supported, but only CLAMP_TO_EDGE and can't have mipmaps
   U32 has_packed_depth_stencil : 1;
   U32 has_depth24 : 1;
   U32 has_mapbuffer : 1;
   U32 has_texture_max_level : 1;

   // fake fence tracking for thrashing detection
   U64 frame_counter;
} *gdraw;

////////////////////////////////////////////////////////////////////////
//
//   General resource management for both textures and vertex buffers
//

// make a texture with reasonable default state
static void make_texture(GLuint tex)
{
   glBindTexture(GL_TEXTURE_2D, tex);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

static void make_rendertarget(GDrawHandle *t, GLuint tex, GLenum int_type, GLenum ext_type, GLenum data_type, S32 w, S32 h, S32 size)
{
   glBindTexture(GL_TEXTURE_2D, tex);
   glTexImage2D(GL_TEXTURE_2D, 0, int_type, w, h, 0, ext_type, data_type, nullptr);
   make_texture(tex);
   glBindTexture(GL_TEXTURE_2D, 0);
}

static void api_free_resource(GDrawHandle *r)
{
   if (r->state == GDRAW_HANDLE_STATE_user_owned)
      return;

   if (!r->cache->is_vertex) {
      glDeleteTextures(1, &r->handle.tex.gl);
      if (r->handle.tex.gl_renderbuf && r->handle.tex.gl_renderbuf != r->handle.tex.gl)
         glDeleteRenderbuffers(1, &r->handle.tex.gl_renderbuf);
   } else {
      glDeleteBuffers(1, &r->handle.vbuf.base);
      glDeleteBuffers(1, &r->handle.vbuf.indices);
   }
   opengl_check();
}

static void RADLINK gdraw_UnlockHandles(GDrawStats *gstats)
{
   // since we're not using fences for this implementation, move all textures off the active list
   // if you're using fences, this is when the fence needs to actually occur
   gdraw_HandleCacheUnlockAll(gdraw->texturecache);
   gdraw_HandleCacheUnlockAll(gdraw->vbufcache);
}

////////////////////////////////////////////////////////////////////////
//
//   Texture creation/updating
//

extern GDrawTexture *gdraw_GLx_(WrappedTextureCreate)(S32 gl_texture_handle, S32 width, S32 height, rrbool has_mipmaps)
{
   GDrawStats stats={0};
   GDrawHandle *p = gdraw_res_alloc_begin(gdraw->texturecache, 0, &stats); // it may need to free one item to give us a handle
   GLint old;

   glGetIntegerv(GL_TEXTURE_BINDING_2D, &old);
   glBindTexture(GL_TEXTURE_2D, gl_texture_handle);
   if (has_mipmaps)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   else
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glBindTexture(GL_TEXTURE_2D, old);

   p->bytes = 0;
   p->handle.tex.gl = gl_texture_handle;
   p->handle.tex.w = width;
   p->handle.tex.h = height;
   p->handle.tex.nonpow2 = !(is_pow2(width) && is_pow2(height));
   gdraw_HandleCacheAllocateEnd(p, 0, nullptr, GDRAW_HANDLE_STATE_user_owned);
   return (GDrawTexture *) p;
}

extern void gdraw_GLx_(WrappedTextureChange)(GDrawTexture *tex, S32 new_gl_texture_handle, S32 new_width, S32 new_height, rrbool has_mipmaps)
{
   GDrawHandle *p = (GDrawHandle *) tex;
   GLint old;

   glGetIntegerv(GL_TEXTURE_BINDING_2D, &old);
   glBindTexture(GL_TEXTURE_2D, new_gl_texture_handle);
   if (has_mipmaps)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   else
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glBindTexture(GL_TEXTURE_2D, old);

   p->handle.tex.gl = new_gl_texture_handle;
   p->handle.tex.w = new_width;
   p->handle.tex.h = new_height;
   p->handle.tex.nonpow2 = !(is_pow2(new_width) && is_pow2(new_height));
}

extern void gdraw_GLx_(WrappedTextureDestroy)(GDrawTexture *tex)
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

static rrbool RADLINK gdraw_MakeTextureBegin(void *owner, S32 width, S32 height, gdraw_texture_format format, U32 flags, GDraw_MakeTexture_ProcessingInfo *p, GDrawStats *gstats)
{
   S32 size=0, asize, stride;
   GDrawHandle *t = nullptr;
   opengl_check();

   stride = width;
   if (format == GDRAW_TEXTURE_FORMAT_rgba32) stride *= 4;
   size = stride*height;

   asize = size;
   if (flags & GDRAW_MAKETEXTURE_FLAGS_mipmap) asize = asize*4/3;

   t = gdraw_res_alloc_begin(gdraw->texturecache, asize, gstats);
   if (!t)
      return IGGY_RESULT_Error_GDraw;

   glGenTextures(1, &t->handle.tex.gl);

   p->texture_data = IggyGDrawMalloc(size);
   if (!p->texture_data) {
      gdraw_HandleCacheAllocateFail(t);
      IggyGDrawSendWarning(nullptr, "GDraw malloc for texture data failed");
      return false;
   }
   
   t->handle.tex.w = width;
   t->handle.tex.h = height;
   t->handle.tex.nonpow2 = !(is_pow2(width) && is_pow2(height));

   p->num_rows = height;
   p->p0 = t;
   p->p1 = owner;
   p->stride_in_bytes = stride;
   p->texture_type = GDRAW_TEXTURE_TYPE_rgba;
   p->i0 = format;
   p->i1 = flags;
   p->i2 = width;
   p->i3 = height;
   p->i4 = asize;
   opengl_check();
   return true;
}

static GDrawTexture * RADLINK gdraw_MakeTextureEnd(GDraw_MakeTexture_ProcessingInfo *p, GDrawStats *stats)
{
   gdraw_texture_format format = (gdraw_texture_format) p->i0;
   S32 flags = p->i1;
   rrbool mipmap = (flags & GDRAW_MAKETEXTURE_FLAGS_mipmap) != 0;
   S32 width = p->i2, height = p->i3;
   GLuint z,e; 
   GDrawHandle *t = (GDrawHandle *) p->p0;

   z = t->handle.tex.gl;
   assert(z != 0);

   make_texture(z);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   if (format == GDRAW_TEXTURE_FORMAT_font)
      glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, p->texture_data);
   else
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, p->texture_data);
   e = glGetError();
   break_on_err(e);

   if (mipmap)
      glGenerateMipmap(GL_TEXTURE_2D);
   if (!e) e = glGetError();

   if (e != 0) {
      gdraw_HandleCacheAllocateFail(t);
      IggyGDrawSendWarning(nullptr, "GDraw OpenGL error creating texture");
      eat_gl_err();
      return nullptr;
   } else {
      gdraw_HandleCacheAllocateEnd(t, p->i4, p->p1, (flags & GDRAW_MAKETEXTURE_FLAGS_never_flush) ? GDRAW_HANDLE_STATE_pinned : GDRAW_HANDLE_STATE_locked);
      stats->nonzero_flags |= GDRAW_STATS_alloc_tex;
      stats->alloc_tex += 1;
      stats->alloc_tex_bytes += p->i4;
   }

   // default wrap mode is clamp to edge
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   if (mipmap)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   else
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   IggyGDrawFree(p->texture_data);

   opengl_check();
   return (GDrawTexture *) t;
}

static rrbool RADLINK gdraw_UpdateTextureBegin(GDrawTexture *tex, void *unique_id, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(stats);
   return gdraw_HandleCacheLock((GDrawHandle *) tex, unique_id);
}

static void   RADLINK gdraw_UpdateTextureRect(GDrawTexture *tex, void *unique_id, S32 x, S32 y, S32 stride, S32 w, S32 h, U8 *data, gdraw_texture_format format)
{
   glBindTexture(GL_TEXTURE_2D, ((GDrawHandle *) tex)->handle.tex.gl);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   // @TODO: use 'stride'
   glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, (format == GDRAW_TEXTURE_FORMAT_font) ? GL_ALPHA : GL_RGBA, GL_UNSIGNED_BYTE, data);
   opengl_check();
}

static void RADLINK gdraw_UpdateTextureEnd(GDrawTexture *tex, void *unique_id, GDrawStats *stats)
{
   gdraw_HandleCacheUnlock((GDrawHandle *) tex);
}

static void RADLINK gdraw_FreeTexture(GDrawTexture *tt, void *unique_id, GDrawStats *gstats)
{
   GDrawHandle *t = (GDrawHandle *) tt;
   assert(t != nullptr);
   if (t->owner == unique_id || unique_id == nullptr) {
      if (t->cache == &gdraw->rendertargets) {
         gdraw_HandleCacheUnlock(t);
         // cache it by simply not freeing it
         return;
      }

      gdraw_res_free(t, gstats);
   }
}

static rrbool RADLINK gdraw_TryToLockTexture(GDrawTexture *t, void *unique_id, GDrawStats *gstats)
{
   RR_UNUSED_VARIABLE(gstats);
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
   if (!gdraw->aa_tex)
      glGenTextures(1, &gdraw->aa_tex);

   make_texture(gdraw->aa_tex);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
   opengl_check();
}

////////////////////////////////////////////////////////////////////////
//
//   Vertex buffer creation/deletion
//

static rrbool RADLINK gdraw_MakeVertexBufferBegin(void *unique_id, gdraw_vformat vformat, S32 vbuf_size, S32 ibuf_size, GDraw_MakeVertexBuffer_ProcessingInfo *p, GDrawStats *gstats)
{
   GLuint e;
   GDrawHandle *vb;
   opengl_check();
   vb = gdraw_res_alloc_begin(gdraw->vbufcache, vbuf_size + ibuf_size, gstats);
   if (!vb) {
      IggyGDrawSendWarning(nullptr, "GDraw out of vertex buffer memory");
      return false;
   }

   e = glGetError();
   vb->handle.vbuf.base = 0;
   vb->handle.vbuf.indices = 0;
   glGenBuffers(1, &vb->handle.vbuf.base);
   glGenBuffers(1, &vb->handle.vbuf.indices);
   glBindBuffer(GL_ARRAY_BUFFER, vb->handle.vbuf.base);
   glBufferData(GL_ARRAY_BUFFER, vbuf_size, nullptr, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vb->handle.vbuf.indices);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibuf_size, nullptr, GL_STATIC_DRAW);
   if (!e) e = glGetError();
   if (e != GL_NO_ERROR) {
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glDeleteBuffers(1, &vb->handle.vbuf.base);
      glDeleteBuffers(1, &vb->handle.vbuf.indices);
      gdraw_HandleCacheAllocateFail(vb);
      eat_gl_err();
      IggyGDrawSendWarning(nullptr, "GDraw OpenGL vertex buffer creation failed");
      return false;
   }

   p->i0 = vbuf_size;
   p->i1 = ibuf_size;
   p->p0 = vb;
   p->p1 = unique_id;
   
   if (!gdraw->has_mapbuffer) {
      p->vertex_data = IggyGDrawMalloc(vbuf_size);
      p->vertex_data_length = vbuf_size;
      p->index_data = IggyGDrawMalloc(ibuf_size);
      p->index_data_length = ibuf_size;

      // check for out of memory conditions
      if (!p->vertex_data || !p->index_data) {
         if (p->vertex_data)  IggyGDrawFree(p->vertex_data);
         if (p->index_data)   IggyGDrawFree(p->index_data);
         IggyGDrawSendWarning(nullptr, "GDraw malloc for vertex buffer temporary memory failed");
         return false;
      }
   } else {
      p->vertex_data = (U8 *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
      p->vertex_data_length = vbuf_size;

      p->index_data = (U8 *)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
      p->index_data_length = ibuf_size;
   }

   opengl_check();
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
   rrbool ok = true;
   GLuint e;

   if (!gdraw->has_mapbuffer) {
      glBufferData(GL_ARRAY_BUFFER, p->i0, p->vertex_data, GL_STATIC_DRAW);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, p->i1, p->index_data, GL_STATIC_DRAW);
      IggyGDrawFree(p->vertex_data);
      IggyGDrawFree(p->index_data);
   } else {
      if (!glUnmapBuffer(GL_ARRAY_BUFFER)) ok = false;
      if (!glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER)) ok = false;
   }

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   e = glGetError();
   if (!ok || e != GL_NO_ERROR) {
      glDeleteBuffers(1, &vb->handle.vbuf.base);
      glDeleteBuffers(1, &vb->handle.vbuf.indices);
      gdraw_HandleCacheAllocateFail(vb);
      eat_gl_err();
      return nullptr;
   } else
      gdraw_HandleCacheAllocateEnd(vb, p->i0 + p->i1, p->p1, GDRAW_HANDLE_STATE_locked);

   opengl_check();
   return (GDrawVertexBuffer *) vb;
}

static rrbool RADLINK gdraw_TryToLockVertexBuffer(GDrawVertexBuffer *vb, void *unique_id, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(stats);
   return gdraw_HandleCacheLock((GDrawHandle *) vb, unique_id);
}

static void RADLINK gdraw_FreeVertexBuffer(GDrawVertexBuffer *vb, void *unique_id, GDrawStats *stats)
{
   GDrawHandle *h = (GDrawHandle *) vb;
   assert(h != nullptr);
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

static GDrawHandle *get_rendertarget_texture(int width, int height, void *owner, GDrawStats *gstats)
{
   S32 size;
   GDrawHandle *t;
   opengl_check();
   t = gdraw_HandleCacheGetLRU(&gdraw->rendertargets);
   if (t) {
      gdraw_HandleCacheLock(t, (void *) (UINTa) 1);
      return t;
   }

   size = gdraw->frametex_width * gdraw->frametex_height * 4;
   t = gdraw_res_alloc_begin(gdraw->texturecache, size, gstats);
   if (!t) return t;

   glGenTextures(1, &t->handle.tex.gl);
   make_rendertarget(t, t->handle.tex.gl, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, width, height, 4);
   t->handle.tex.w = gdraw->frametex_width;
   t->handle.tex.h = gdraw->frametex_height;
   t->handle.tex.nonpow2 = 1; // assume all rendertargets are non-pow2 for consistency
   gstats->nonzero_flags |= GDRAW_STATS_alloc_tex;
   gstats->alloc_tex += 1;
   gstats->alloc_tex_bytes += size;
   opengl_check();
   gdraw_HandleCacheAllocateEnd(t, size, owner, GDRAW_HANDLE_STATE_locked);

   return t;
}

static GDrawHandle *get_color_rendertarget(GDrawStats *gstats)
{
   S32 size;
   GDrawHandle *t;
   opengl_check();
   t = gdraw_HandleCacheGetLRU(&gdraw->rendertargets);
   if (t) {
      gdraw_HandleCacheLock(t, (void *) (UINTa) 1);
      return t;
   }

   // ran out of RTs, allocate a new one
   size = gdraw->frametex_width * gdraw->frametex_height * 4;
   if (gdraw->rendertargets.bytes_free < size) {
      IggyGDrawSendWarning(nullptr, "GDraw exceeded available rendertarget memory");
      return nullptr;
   }

   t = gdraw_HandleCacheAllocateBegin(&gdraw->rendertargets);
   if (!t) {
      IggyGDrawSendWarning(nullptr, "GDraw exceeded available rendertarget handles");
      return t;
   }

   glGenTextures(1, &t->handle.tex.gl);
   make_rendertarget(t, t->handle.tex.gl, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, gdraw->frametex_width,gdraw->frametex_height, 4);
   t->handle.tex.w = gdraw->frametex_width;
   t->handle.tex.h = gdraw->frametex_height;
   t->handle.tex.nonpow2 = 1; // assume all rendertargets are non-pow2 for consistency

#ifdef GDRAW_MULTISAMPLING
   if (gdraw->multisampling) {
      glGenRenderbuffers(1, &t->handle.tex.gl_renderbuf);
      glBindRenderbuffer(GL_RENDERBUFFER, t->handle.tex.gl_renderbuf);
      glRenderbufferStorageMultisample(GL_RENDERBUFFER, gdraw->multisampling, GL_RGBA, gdraw->frametex_width, gdraw->frametex_height);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);
   }
#endif
   opengl_check();

   gdraw_HandleCacheAllocateEnd(t, size, (void *) (UINTa) 1, GDRAW_HANDLE_STATE_locked);
   gstats->nonzero_flags |= GDRAW_STATS_alloc_tex;
   gstats->alloc_tex += gdraw->multisampling ? 2 : 1;
   gstats->alloc_tex_bytes += (1 + gdraw->multisampling) * size;

   return t;
}

static GDrawHandle *get_depthstencil_renderbuffer(GDrawStats *gstats)
{
   if (!gdraw->stencil_depth.handle.tex.gl) {
      gstats->nonzero_flags |= GDRAW_STATS_alloc_tex;
      gstats->alloc_tex += 1;

#ifdef GDRAW_MULTISAMPLING
      if (gdraw->multisampling) {
         glGenRenderbuffers(1, &gdraw->stencil_depth.handle.tex.gl);
         glBindRenderbuffer(GL_RENDERBUFFER, gdraw->stencil_depth.handle.tex.gl);
         glRenderbufferStorageMultisample(GL_RENDERBUFFER, gdraw->multisampling, GL_DEPTH24_STENCIL8, gdraw->frametex_width, gdraw->frametex_height);

         gstats->alloc_tex_bytes += gdraw->multisampling * 4 * gdraw->frametex_width * gdraw->frametex_height;
      } else {
#endif
         if (gdraw->has_packed_depth_stencil) {
            glGenRenderbuffers(1, &gdraw->stencil_depth.handle.tex.gl);
            glBindRenderbuffer(GL_RENDERBUFFER, gdraw->stencil_depth.handle.tex.gl);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, gdraw->frametex_width, gdraw->frametex_height);

            gdraw->stencil_depth.handle.tex.gl_renderbuf = gdraw->stencil_depth.handle.tex.gl;
         } else {
            // this path is mainly for the iOS simulator
            glGenRenderbuffers(1, &gdraw->stencil_depth.handle.tex.gl);
            glBindRenderbuffer(GL_RENDERBUFFER, gdraw->stencil_depth.handle.tex.gl);
            glRenderbufferStorage(GL_RENDERBUFFER, gdraw->has_depth24 ? GL_DEPTH_COMPONENT24 : GL_DEPTH_COMPONENT16, gdraw->frametex_width, gdraw->frametex_height);

            glGenRenderbuffers(1, &gdraw->stencil_depth.handle.tex.gl_renderbuf);
            glBindRenderbuffer(GL_RENDERBUFFER, gdraw->stencil_depth.handle.tex.gl_renderbuf);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, gdraw->frametex_width, gdraw->frametex_height);
         }

         gstats->alloc_tex_bytes += 4 * gdraw->frametex_width * gdraw->frametex_height;
#ifdef GDRAW_MULTISAMPLING
      }
#endif

      glBindRenderbuffer(GL_RENDERBUFFER, 0);
      opengl_check();
   }
   return &gdraw->stencil_depth;
}

static void flush_rendertargets(GDrawStats *stats)
{
   gdraw_res_flush(&gdraw->rendertargets, stats);

   if (gdraw->stencil_depth.handle.tex.gl_renderbuf &&
       gdraw->stencil_depth.handle.tex.gl_renderbuf != gdraw->stencil_depth.handle.tex.gl) {
      glDeleteRenderbuffers(1, &gdraw->stencil_depth.handle.tex.gl_renderbuf);
      gdraw->stencil_depth.handle.tex.gl_renderbuf = 0;
   }

   if (gdraw->stencil_depth.handle.tex.gl) {
      glDeleteRenderbuffers(1, &gdraw->stencil_depth.handle.tex.gl);
      gdraw->stencil_depth.handle.tex.gl = 0;
   }
   opengl_check();
}

////////////////////////////////////////////////////////////////////////
//
//   Begin rendering for a frame
//

static void lazy_shader(ProgramWithCachedVariableLocations *ptr);

static RADINLINE void use_lazy_shader(ProgramWithCachedVariableLocations *prg)
{
   if (!prg->program)
      lazy_shader(prg); // already does a glUseProgram!
   else
      glUseProgram(prg->program);
}

static void set_viewport(void)
{
   if (gdraw->in_blur) {
      glViewport(0, 0, gdraw->tpw, gdraw->tph);
   } else if (gdraw->cur == gdraw->frame) {
      glViewport(gdraw->vx, gdraw->vy, gdraw->tw, gdraw->th);
   } else if (gdraw->cur->cached) {
      glViewport(0, 0, gdraw->cur->width, gdraw->cur->height);
   } else {
      glViewport(0, 0, gdraw->tpw, gdraw->tph);
      // we need to translate from naive pixel space to align a tile
   }
   opengl_check();
}

static void set_projection_raw(S32 x0, S32 x1, S32 y0, S32 y1)
{
   gdraw->projection[0] = 2.0f / (x1-x0);
   gdraw->projection[1] = 2.0f / (y1-y0);
   gdraw->projection[2] = (x1+x0)/(F32)(x0-x1);
   gdraw->projection[3] = (y1+y0)/(F32)(y0-y1);
}

static void set_projection(void)
{
   if (gdraw->in_blur)
      set_projection_raw(0, gdraw->tpw, gdraw->tph, 0);
   else if (gdraw->cur == gdraw->frame)
      set_projection_raw(gdraw->tx0, gdraw->tx0 + gdraw->tw, gdraw->ty0 + gdraw->th, gdraw->ty0);
   else if (gdraw->cur->cached)
      set_projection_raw(gdraw->cur->base_x, gdraw->cur->base_x + gdraw->cur->width, gdraw->cur->base_y, gdraw->cur->base_y + gdraw->cur->height);
   else
      set_projection_raw(gdraw->tx0p, gdraw->tx0p + gdraw->tpw, gdraw->ty0p + gdraw->tph, gdraw->ty0p);
}

static void clear_renderstate(void)
{
   clear_renderstate_platform_specific();

   // deactivate aa_tex
   glActiveTexture(GL_TEXTURE0 + AATEX_SAMPLER);
   glBindTexture(GL_TEXTURE_2D, 0);

   glColorMask(1,1,1,1);
   glDepthMask(GL_TRUE);

   glDisable(GL_CULL_FACE);
   glDisable(GL_BLEND);
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_STENCIL_TEST);
   glDisable(GL_SCISSOR_TEST);
   glActiveTexture(GL_TEXTURE0);

   glUseProgram(0);
   opengl_check();
}

static void set_common_renderstate(void)
{
   clear_renderstate();

   // activate aa_tex
   glActiveTexture(GL_TEXTURE0 + AATEX_SAMPLER);
   glBindTexture(GL_TEXTURE_2D, gdraw->aa_tex);
   glActiveTexture(GL_TEXTURE0);
}

void gdraw_GLx_(SetTileOrigin)(S32 x, S32 y, U32 framebuffer)
{
   gdraw->vx = x;
   gdraw->vy = y;
   gdraw->main_framebuffer = framebuffer;
}

static void RADLINK gdraw_SetViewSizeAndWorldScale(S32 w, S32 h, F32 scalex, F32 scaley)
{
   memset(gdraw->frame, 0, sizeof(gdraw->frame));
   gdraw->cur = gdraw->frame;
   gdraw->fw = w;
   gdraw->fh = h;
   gdraw->world_to_pixel[0] = scalex;
   gdraw->world_to_pixel[1] = scaley;
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

// must include anything necessary for texture creation/update
static void RADLINK gdraw_RenderingBegin(void)
{
}
static void RADLINK gdraw_RenderingEnd(void)
{
}

static void RADLINK gdraw_RenderTileBegin(S32 x0, S32 y0, S32 x1, S32 y1, S32 pad, GDrawStats *gstats)
{
   opengl_check();

   if (x0 == 0 && y0 == 0 && x1 == gdraw->fw && y1 == gdraw->fh) {
      pad = 0;
      gdraw->tile_enabled = false;
   } else {
      gdraw->tile_enabled = true;
   }

   gdraw->tx0 = x0;
   gdraw->ty0 = y0;
   gdraw->tw = x1-x0;
   gdraw->th = y1-y0;
   gdraw->tpw = gdraw->tw + pad*2;
   gdraw->tph = gdraw->th + pad*2;
   // origin of padded region
   gdraw->tx0p = x0 - pad;
   gdraw->ty0p = y0 - pad;

   if (gdraw->tpw > gdraw->frametex_width || gdraw->tph > gdraw->frametex_height) {
      gdraw->frametex_width  = RR_MAX(gdraw->tpw, gdraw->frametex_width);
      gdraw->frametex_height = RR_MAX(gdraw->tph, gdraw->frametex_height);

      flush_rendertargets(gstats);
   }

   set_viewport();
   set_projection();
   set_common_renderstate();

   glBindFramebuffer(GL_FRAMEBUFFER, gdraw->main_framebuffer);
   opengl_check();
}

static void RADLINK gdraw_RenderTileEnd(GDrawStats *stats)
{
   clear_renderstate();
}

#define MAX_DEPTH_VALUE   (1 << 13)

static void RADLINK gdraw_GetInfo(GDrawInfo *d)
{
   GLint maxtex;

   opengl_check();
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtex);

   d->num_stencil_bits = 8;
   d->max_id = MAX_DEPTH_VALUE-2;
   // for floating point depth, just use mantissa, e.g. 16-20 bits
   d->max_texture_size = maxtex;
   d->buffer_format = GDRAW_BFORMAT_vbib;
   d->shared_depth_stencil = 0;
   d->always_mipmap = 0;
   d->conditional_nonpow2 = gdraw->has_conditional_non_power_of_two;
   opengl_check();
}


////////////////////////////////////////////////////////////////////////
//
//   Enable/disable rendertargets in stack fashion
//

static void clear_with_rect(gswf_recti *region, rrbool clear_depth, GDrawStats *stats);

static void set_render_target_state(void)
{
   GLint h;
#ifdef GDRAW_MULTISAMPLING
   if (gdraw->multisampling) {
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &h);
      h = gdraw->cur->color_buffer ? gdraw->cur->color_buffer->handle.tex.gl_renderbuf : 0;
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_RENDERBUFFER, h);
      h = gdraw->cur->stencil_depth ? gdraw->cur->stencil_depth->handle.tex.gl : 0;
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_RENDERBUFFER, h);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, h);
   } else {
#endif
      h = gdraw->cur->color_buffer ? gdraw->cur->color_buffer->handle.tex.gl : 0;
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, h, 0);
      if (gdraw->cur->stencil_depth) {
         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,   GL_RENDERBUFFER, gdraw->cur->stencil_depth->handle.tex.gl);
         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gdraw->cur->stencil_depth->handle.tex.gl_renderbuf);
      } else {
         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,   GL_RENDERBUFFER, 0);
         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
      }
#ifdef GDRAW_MULTISAMPLING
   }
#endif
   opengl_check();
}

static rrbool RADLINK gdraw_TextureDrawBufferBegin(gswf_recti *region, gdraw_texture_format format, U32 flags, void *owner, GDrawStats *gstats)
{
   GDrawFramebufferState *n = gdraw->cur+1;
   GDrawHandle *t;
   int k;
   if (gdraw->tw == 0 || gdraw->th == 0) {
      IggyGDrawSendWarning(nullptr, "GDraw got a request for an empty rendertarget");
      return false;
   }

   if (n >= &gdraw->frame[MAX_RENDER_STACK_DEPTH]) {
      IggyGDrawSendWarning(nullptr, "GDraw rendertarget nesting exceeded MAX_RENDER_STACK_DEPTH");
      return false;
   }

   if (owner) {
      t = get_rendertarget_texture(region->x1 - region->x0, region->y1 - region->y0, owner, gstats);
      if (!t) {
         IggyGDrawSendWarning(nullptr, "GDraw ran out of rendertargets for cacheAsBItmap");
         return false;
      }
   } else {
      t = get_color_rendertarget(gstats);
      if (!t) {
         IggyGDrawSendWarning(nullptr, "GDraw ran out of rendertargets");
         return false;
      }
   }
   n->color_buffer = t;
   assert(n->color_buffer != nullptr);

   if (n == gdraw->frame+1)
      n->stencil_depth = get_depthstencil_renderbuffer(gstats);
   else
      n->stencil_depth = (n-1)->stencil_depth;
   ++gdraw->cur;

   gdraw->cur->cached = owner != nullptr;
   if (owner) {
      gdraw->cur->base_x = region->x0;
      gdraw->cur->base_y = region->y0;
      gdraw->cur->width  = region->x1 - region->x0;
      gdraw->cur->height = region->y1 - region->y0;
   }

   gstats->nonzero_flags |= GDRAW_STATS_rendtarg;

   glBindFramebuffer(GL_FRAMEBUFFER, gdraw->framebuffer_stack_object);
   set_render_target_state();

   assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

   // viewport for clear (make sure scissor is inside viewport just in case)
   glViewport(0, 0, gdraw->frametex_width, gdraw->frametex_height);

   k = (int) (n->color_buffer - gdraw->rendertargets.handle);
   if (region) {
      S32 ox, oy;
      
      // in a perfect world, we'd only need 1 pixel of border on all sides for
      // bilinear filtering, which would mean pad = 1. however, texture interpolator
      // precision is not that high even on PC parts, and if we only use 1 pixel of
      // padding we will often get some un-filled pixels "creeping in" from the sides.
      // pad = 2 is fine on recent PC parts, but not old PC parts or even fairly new
      // mobile parts, so we play it safe and use 3 pixels which so far gives good
      // results everywhere.
      S32 pad = 3;

      // region.x0,y0 are the top left of the rectangle in display space
      // x,y are the *bottom* left of the rectangle in window space
      S32 h = gdraw->tph;
      S32 xt0, yt0, xt1, yt1;
      S32 x0, y0, x1, y1;

      if (gdraw->in_blur || !gdraw->tile_enabled)
         ox = oy = 0;
      else
         ox = gdraw->tx0, oy = gdraw->ty0;

      // clamp region to tile (in gdraw coords)
      xt0 = RR_MAX(region->x0 - ox, 0);
      yt0 = RR_MAX(region->y0 - oy, 0);
      xt1 = RR_MIN(region->x1 - ox, gdraw->tpw);
      yt1 = RR_MIN(region->y1 - oy, gdraw->tph);

      // but the padding gets clamped to framebuffer coords! also transfer to window space here.
      x0 = RR_MAX(    xt0 - pad, 0);
      y0 = RR_MAX(h - yt1 - pad, 0);
      x1 = RR_MIN(    xt1 + pad, gdraw->frametex_width);
      y1 = RR_MIN(h - yt0 + pad, gdraw->frametex_height);

      if (x1 <= x0 || y1 <= y0) { // region doesn't intersect with current tile
         --gdraw->cur;

         // remove color and stencil buffers
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, 0, 0);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_TEXTURE_2D, 0, 0);
         
         // switch render target back
         if (gdraw->cur == gdraw->frame)
            glBindFramebuffer(GL_FRAMEBUFFER, gdraw->main_framebuffer);
         else
            set_render_target_state();

         set_viewport();
         set_projection();
         opengl_check();

         // free our render target
         gdraw_FreeTexture((GDrawTexture *) n->color_buffer, 0, gstats);

         // note: don't send a warning since this will happen during regular tiled rendering
         return false;
      }

      glEnable(GL_SCISSOR_TEST);
      glScissor(x0, y0, x1 - x0, y1 - y0);
      gdraw->rt_valid[k].x0 = xt0;
      gdraw->rt_valid[k].y0 = yt0;
      gdraw->rt_valid[k].x1 = xt1;
      gdraw->rt_valid[k].y1 = yt1;
      gstats->cleared_pixels += (x1 - x0) * (y1 - y0);
   } else {
      glDisable(GL_SCISSOR_TEST);
      gdraw->rt_valid[k].x0 = 0;
      gdraw->rt_valid[k].y0 = 0;
      gdraw->rt_valid[k].x1 = gdraw->frametex_width;
      gdraw->rt_valid[k].y1 = gdraw->frametex_height;
      gstats->cleared_pixels += gdraw->frametex_width * gdraw->frametex_height;
   }

   gstats->nonzero_flags |= GDRAW_STATS_clears;
   gstats->num_clears += 1;

#ifdef GDRAW_FEWER_CLEARS
   if (region) {
      clear_with_rect(region, n==gdraw->frame+1, gstats);
   } else
#endif // GDRAW_FEWER_CLEARS
   {
      glClearColor(0,0,0,0); // must clear destination alpha
      glClearStencil(0);
      glClearDepth(1);
      glStencilMask(255);
      glDepthMask(GL_TRUE);
      glColorMask(1,1,1,1);
      glDisable(GL_STENCIL_TEST);
      if (n == gdraw->frame+1)
         glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      else
         glClear(GL_COLOR_BUFFER_BIT);
   }

   set_viewport();
   set_projection();

   opengl_check();

   return true;
}


static GDrawTexture *RADLINK gdraw_TextureDrawBufferEnd(GDrawStats *gstats)
{
   GDrawFramebufferState *n =   gdraw->cur;
   GDrawFramebufferState *m = --gdraw->cur;
   if (gdraw->fw == 0 || gdraw->fh == 0) return 0;

   if (n >= &gdraw->frame[MAX_RENDER_STACK_DEPTH])
      return 0; // already returned a warning in Start...()

   assert(m >= gdraw->frame);  // bug in Iggy -- unbalanced

   if (m != gdraw->frame)
      assert(m->color_buffer != nullptr);
   assert(n->color_buffer != nullptr);

   // remove color and stencil buffers
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_RENDERBUFFER, 0);
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_RENDERBUFFER, 0);

#ifdef GDRAW_MULTISAMPLING
   if (gdraw->multisampling) {
      // blit from multisample to texture
      if (n->color_buffer->handle.tex.gl_renderbuf) {
         GLuint res;
         glBindFramebuffer(GL_READ_FRAMEBUFFER, gdraw->framebuffer_copy_to_texture);
         glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
         glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
         glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
         glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
         glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, n->color_buffer->handle.tex.gl_renderbuf);
         glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, n->color_buffer->handle.tex.gl, 0);
         res = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
         glBlitFramebuffer(0,0,gdraw->tpw,gdraw->tph,0,0,gdraw->tpw,gdraw->tph, GL_COLOR_BUFFER_BIT, GL_NEAREST);
         gstats->nonzero_flags |= GDRAW_STATS_blits;
         gstats->num_blits += 1;
         gstats->num_blit_pixels += (gdraw->tpw * gdraw->tph);

         glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
         glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
         glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
      }
   }
#endif

   gstats->nonzero_flags |= GDRAW_STATS_rendtarg;
   gstats->rendertarget_changes += 1;

   // set the new state
   if (m == gdraw->frame) // back to initial framebuffer
      glBindFramebuffer(GL_FRAMEBUFFER, gdraw->main_framebuffer);
   else
      set_render_target_state();

   // reset the viewport if we've reached the root scope
   set_viewport();
   set_projection();

   opengl_check();

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
   glDisable(GL_SCISSOR_TEST);
   glStencilMask(bits);
   glClearStencil(0);
   glClear(GL_STENCIL_BUFFER_BIT);
   opengl_check();
}

// this only happens rarely (hopefully never) if we use the depth buffer,
// so we can just clear the whole thing
static void RADLINK gdraw_ClearID(void)
{
   glDisable(GL_SCISSOR_TEST);
   glClearDepth(1);
   glDepthMask(GL_TRUE);
   glClear(GL_DEPTH_BUFFER_BIT);
   opengl_check();
}


////////////////////////////////////////////////////////////////////////
//
//   Set all the render state from GDrawRenderState
//
// This also is responsible for getting the framebuffer into a texture
// if the read-modify-write blend operation can't be expressed with
// the native blend operators. (E.g. "screen")
//

enum
{
   VVAR_world0    = 0,
   VVAR_world1    = 1,
   VVAR_xoff      = 2,
   VVAR_texgen_s  = 3,
   VVAR_texgen_t  = 4,
   VVAR_viewproj  = 5,
   VVAR_x3d       = 5,
   VVAR_y3d       = 6,
   VVAR_z3d       = 7,
};

// convert an ID request to a value suitable for the depth buffer,
// in homogeneous clip space with w=1 (depth from -1..1)
static float depth_from_id(S32 id)
{
   return 1.0f - 2.0f*(id+1) / (F32) MAX_DEPTH_VALUE;
}

static void set_texture(U32 texunit, GDrawTexture *tex)
{
   glActiveTexture(GL_TEXTURE0 + texunit);
   if (tex == nullptr)
      glBindTexture(GL_TEXTURE_2D, 0);
   else
      glBindTexture(GL_TEXTURE_2D, ((GDrawHandle *) tex)->handle.tex.gl);
}

static void set_world_projection(const int *vvars, const F32 world[2*4])
{
   assert(vvars[VVAR_world0] >= 0 && vvars[VVAR_world1] >= 0 && vvars[VVAR_viewproj] >= 0);
   glUniform4fv(vvars[VVAR_world0], 1, world + 0);
   glUniform4fv(vvars[VVAR_world1], 1, world + 4);
   glUniform4fv(vvars[VVAR_viewproj], 1, gdraw->projection);
}

static void set_3d_projection(const int *vvars, const F32 world[2*4], const F32 xform[3][4])
{
   assert(vvars[VVAR_world0] >= 0 && vvars[VVAR_world1] >= 0);
   glUniform4fv(vvars[VVAR_world0], 1, world + 0);
   glUniform4fv(vvars[VVAR_world1], 1, world + 4);

   assert(vvars[VVAR_x3d] >= 0 && vvars[VVAR_y3d] >= 0 && vvars[VVAR_z3d] >= 0);
   glUniform4fv(vvars[VVAR_x3d], 1, xform[0]);
   glUniform4fv(vvars[VVAR_y3d], 1, xform[1]);
   glUniform4fv(vvars[VVAR_z3d], 1, xform[2]);
}


static int set_render_state(GDrawRenderState *r, S32 vformat, const int **ovvars, GDrawPrimitive *p, GDrawStats *gstats)
{
   static struct gdraw_gl_blendspec {
      GLboolean enable;
      GLenum src;
      GLenum dst;
   } blends[ASSERT_COUNT(GDRAW_BLEND__count, 6)] = {
      { GL_FALSE, GL_ONE,        GL_ZERO                },  // GDRAW_BLEND_none
      { GL_TRUE,  GL_ONE,        GL_ONE_MINUS_SRC_ALPHA },  // GDRAW_BLEND_alpha
      { GL_TRUE,  GL_DST_COLOR,  GL_ONE_MINUS_SRC_ALPHA },  // GDRAW_BLEND_multiply
      { GL_TRUE,  GL_ONE,        GL_ONE                 },  // GDRAW_BLEND_add

      { GL_FALSE, GL_ONE,        GL_ZERO                },  // GDRAW_BLEND_filter
      { GL_FALSE, GL_ONE,        GL_ZERO                },  // GDRAW_BLEND_special
   };

   F32 world[2*4];
   ProgramWithCachedVariableLocations *prg;
   int *fvars, *vvars;
   int blend_mode;

   opengl_check();
   assert((vformat >= 0 && vformat < GDRAW_vformat__basic_count) || vformat == GDRAW_vformat_ihud1);

   if (vformat == GDRAW_vformat_ihud1) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // premultiplied alpha blend mode
      prg = &gdraw->ihud[0];
   } else {
      // apply the major blend mode
      blend_mode = r->blend_mode;
      assert(blend_mode >= 0 && blend_mode < sizeof(blends)/sizeof(*blends));
      if (blends[blend_mode].enable) {
         glEnable(GL_BLEND);
         glBlendFunc(blends[blend_mode].src, blends[blend_mode].dst);
      } else
         glDisable(GL_BLEND);

      // set the fragment program if it wasn't set above
      if (r->blend_mode != GDRAW_BLEND_special) {
         // make sure data has been initialized
         int which = r->tex0_mode, additive = 0;

         if (r->cxf_add) {
            additive = 1;
            if (r->cxf_add[3]) additive = 2;
         }

         prg = &gdraw->fprog[which][additive][vformat];
      } else
         prg = &gdraw->exceptional_blend[r->special_blend];
   }

   use_lazy_shader(prg);
   opengl_check();
   fvars = prg->vars[0];
   vvars = prg->vars[1];

   if (vformat == GDRAW_vformat_ihud1) {
      F32 wv[2][4] = { 1.0f/960,0,0,-1.0, 0,-1.0f/540,0,+1.0 };
      glUniform4fv(vvars[VAR_ihudv_worldview], 2, wv[0]);
      opengl_check();
      glUniform4fv(vvars[VAR_ihudv_material], p->uniform_count, p->uniforms);
      opengl_check();
      glUniform1f(vvars[VAR_ihudv_textmode], p->drawprim_mode ? 0.0f : 1.0f);
      opengl_check();
   } else {

      // set vertex shader constants
      if (!r->use_world_space)
         gdraw_ObjectSpace(world, r->o2w, depth_from_id(r->id), 0.0f);
      else
         gdraw_WorldSpace(world, gdraw->world_to_pixel, depth_from_id(r->id), 0.0f);

   #ifdef FLASH_10
      set_3d_projection(vvars, world, gdraw->xform_3d);
   #else
      set_world_projection(vvars, world);
   #endif

      if (vvars[VVAR_xoff] >= 0)
         glUniform4fv(vvars[VVAR_xoff], 1, r->edge_matrix);

      if (r->texgen0_enabled) {
         assert(vvars[VVAR_texgen_s] >= 0 && vvars[VVAR_texgen_t] >= 0);
         glUniform4fv(vvars[VVAR_texgen_s], 1, r->s0_texgen);
         glUniform4fv(vvars[VVAR_texgen_t], 1, r->t0_texgen);
      }
   }

   // texture stuff
   set_texture(0, r->tex[0]);
   opengl_check();

   if (r->tex[0] && gdraw->has_conditional_non_power_of_two && ((GDrawHandle*) r->tex[0])->handle.tex.nonpow2) {
      // only wrap mode allowed in conditional nonpow2 is clamp; this should
      // have been set when the texture was created, but to be on the safe side...
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   } else switch (r->wrap0) {
      case GDRAW_WRAP_repeat:
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
         break;
      case GDRAW_WRAP_clamp:
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
         break;
      case GDRAW_WRAP_mirror:
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
         break;
   }

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r->nearest0 ? GL_NEAREST : GL_LINEAR);

   // fragment shader constants

   if (fvars[VAR_cmul] >= 0)
      glUniform4f(fvars[VAR_cmul], r->color[0], r->color[1], r->color[2], r->color[3]);
   if (fvars[VAR_cadd] >= 0)
      if (r->cxf_add)
         glUniform4f(fvars[VAR_cadd], r->cxf_add[0]/255.0f, r->cxf_add[1]/255.0f, r->cxf_add[2]/255.0f, r->cxf_add[3]/255.0f);
      else
         glUniform4f(fvars[VAR_cadd], 0,0,0,0);
   if (fvars[VAR_focal] >= 0)
      glUniform4fv(fvars[VAR_focal], 1, r->focal_point);

   glActiveTexture(GL_TEXTURE0);

   // Set pixel operation states

   if (r->scissor) {
      S32 x0,y0,x1,y1;
      // scissor.x0,y0 are the top left of the rectangle in display space
      // x,y are the *bottom* left of the rectangle in window space
      x0 = r->scissor_rect.x0;
      y0 = r->scissor_rect.y1; 
      x1 = r->scissor_rect.x1;
      y1 = r->scissor_rect.y0;
      // convert into tile-relative coordinates
      if (gdraw->tile_enabled) {
         x0 -= gdraw->tx0;
         y0 -= gdraw->ty0;
         x1 -= gdraw->tx0;
         y1 -= gdraw->ty0;
      }
      // convert bottom-most edge to bottom-relative
      y0 = (gdraw->th) - y0;
      y1 = (gdraw->th) - y1;
      if (gdraw->cur == gdraw->frame) {
         // move into viewport space
         x0 += gdraw->vx;
         y0 += gdraw->vy;
         x1 += gdraw->vx;
         y1 += gdraw->vy;
      }
      glScissor(x0,y0,x1-x0,y1-y0);
      glEnable(GL_SCISSOR_TEST);
   } else
      glDisable(GL_SCISSOR_TEST);

   glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
   glStencilMask(r->stencil_set);
   glStencilFunc(GL_EQUAL, 255, r->stencil_test);
   if (r->stencil_set | r->stencil_test)
      glEnable(GL_STENCIL_TEST);
   else
      glDisable(GL_STENCIL_TEST);

   if (r->stencil_set)
      glColorMask(0,0,0,0);
   else
      glColorMask(1,1,1,1);

   if (r->test_id) {
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LESS);
   } else {
      glDisable(GL_DEPTH_TEST);
      glDepthFunc(GL_LESS);
   }

   if (r->set_id)
      glDepthMask(GL_TRUE);
   else
      glDepthMask(GL_FALSE);

   opengl_check();
   if (ovvars)
      *ovvars = vvars;

   return 1;
}

////////////////////////////////////////////////////////////////////////
//
//   Vertex formats
//

static void set_vertex_format(S32 format, F32 *vertices)
{
   switch (format) {
      case GDRAW_vformat_v2:
         glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
         glEnableVertexAttribArray(0);
         break;

      case GDRAW_vformat_v2aa:
         glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, vertices);
         glVertexAttribPointer(1, 4, GL_SHORT, GL_FALSE, 16, vertices+2);
         glEnableVertexAttribArray(0);
         glEnableVertexAttribArray(1);
         break;

      case GDRAW_vformat_v2tc2:
         glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, vertices);
         glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, vertices+2);
         glEnableVertexAttribArray(0);
         glEnableVertexAttribArray(1);
         break;

      case GDRAW_vformat_ihud1:
         glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 20, vertices);
         glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, vertices+2);
         glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 20, vertices+4);
         glEnableVertexAttribArray(0);
         glEnableVertexAttribArray(1);
         glEnableVertexAttribArray(2);
         break;

      default:
         assert(0);
   }
}

static void reset_vertex_format(S32 format)
{
   // we don't use attrib #1 for all formats, but doesn't seem worthwhile to check
   format = format;
   glDisableVertexAttribArray(0);
   glDisableVertexAttribArray(1);
   glDisableVertexAttribArray(2);
}

////////////////////////////////////////////////////////////////////////
//
//   Draw triangles with a given renderstate
//

static void tag_resources(void *r1, void *r2, void *r3)
{
   U64 now = gdraw->frame_counter;
   if (r1) ((GDrawHandle *) r1)->fence.value = now;
   if (r2) ((GDrawHandle *) r2)->fence.value = now;
   if (r3) ((GDrawHandle *) r3)->fence.value = now;
}

static int vformat_stride[] =
{
   2,4,4,5
};

static void RADLINK gdraw_DrawIndexedTriangles(GDrawRenderState *r, GDrawPrimitive *p, GDrawVertexBuffer *buf, GDrawStats *gstats)
{
   GDrawHandle *vb = (GDrawHandle *) buf;
   if (vb) {
      glBindBuffer(GL_ARRAY_BUFFER, vb->handle.vbuf.base);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vb->handle.vbuf.indices);
   } else {
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   }

   if (!set_render_state(r,p->vertex_format, nullptr, p, gstats)) return;
   gstats->nonzero_flags |= GDRAW_STATS_batches;
   gstats->num_batches += 1;
   gstats->drawn_indices += p->num_indices;
   gstats->drawn_vertices += p->num_vertices;

   if (vb || p->indices) { // regular path
      set_vertex_format(p->vertex_format, p->vertices);
      glDrawElements(GL_TRIANGLES, p->num_indices, GL_UNSIGNED_SHORT, p->indices);
   } else { // dynamic quads
      S32 pos = 0;
      U32 stride = vformat_stride[p->vertex_format]; // in units of sizeof(F32)
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gdraw->quad_ib);
      assert(p->num_vertices % 4 == 0);

      while (pos < p->num_vertices) {
         S32 vert_count = RR_MIN(p->num_vertices - pos, QUAD_IB_COUNT * 4);
         set_vertex_format(p->vertex_format, p->vertices + pos*stride);
         glDrawElements(GL_TRIANGLES, (vert_count >> 2) * 6, GL_UNSIGNED_SHORT, nullptr);
         pos += vert_count;
      }
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   }
   reset_vertex_format(p->vertex_format);

   if (vb) {
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   }

   opengl_check();
   tag_resources(vb, r->tex[0], r->tex[1]);
}

///////////////////////////////////////////////////////////////////////
//
//   Flash 8 filter effects
//

// caller sets up texture coordinates
static void do_screen_quad(gswf_recti *s, F32 *tc, const int *vvars, GDrawStats *gstats, F32 depth)
{
   F32 px0 = (F32) s->x0, py0 = (F32) s->y0, px1 = (F32) s->x1, py1 = (F32) s->y1;
   F32 s0 = tc[0], t0 = tc[1], s1 = tc[2], t1 = tc[3];
   F32 vert[4][4];
   F32 world[2*4];

   opengl_check();

   vert[0][0] = px0;  vert[0][1] = py0; vert[0][2] = s0; vert[0][3] = t0;
   vert[1][0] = px1;  vert[1][1] = py0; vert[1][2] = s1; vert[1][3] = t0;
   vert[2][0] = px1;  vert[2][1] = py1; vert[2][2] = s1; vert[2][3] = t1;
   vert[3][0] = px0;  vert[3][1] = py1; vert[3][2] = s0; vert[3][3] = t1;

   opengl_check();
   gdraw_PixelSpace(world);
   world[2] = depth;
   set_world_projection(vvars, world);
   opengl_check();

   set_vertex_format(GDRAW_vformat_v2tc2, vert[0]);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   opengl_check();
   glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
   reset_vertex_format(GDRAW_vformat_v2tc2);
   opengl_check();

   gstats->nonzero_flags |= GDRAW_STATS_batches;
   gstats->num_batches    += 1;
   gstats->drawn_vertices += 4;
   gstats->drawn_indices  += 6;

   opengl_check();
}

#ifdef GDRAW_FEWER_CLEARS
static void clear_with_rect(gswf_recti *region, rrbool clear_depth, GDrawStats *gstats)
{
   F32 tc[4] = { 0,0,0,0 };

   use_lazy_shader(&gdraw->manual_clear);
   glUniform4f(gdraw->manual_clear.vars[0][0], 0.0, 0,0,0);

   glDisable(GL_BLEND);

   if (clear_depth) {
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_ALWAYS);
      glDepthMask(GL_TRUE);

      glEnable(GL_STENCIL_TEST);
      glStencilMask(255);
      glStencilOp(GL_REPLACE,GL_REPLACE,GL_REPLACE);
      glStencilFunc(GL_ALWAYS,0,255);
   } else {
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_STENCIL_TEST);
   }

   glColorMask(1,1,1,1);
   glColor4f(0,0,0,0);

   {
      // coordinate system doesn't match, so just draw whole screen, rely on scissor to clip it properly
      gswf_recti foo = { -10000,-10000,10000,10000 };
      do_screen_quad(&foo, tc, gdraw->manual_clear.vars[1], gstats, 1.0f);
   }
}
#endif

static void gdraw_DriverBlurPass(GDrawRenderState *r, int taps, F32 *data, gswf_recti *s, F32 *tc, F32 height_max, F32 *clamp, GDrawStats *gstats)
{
   ProgramWithCachedVariableLocations *prg = &gdraw->blur_prog[taps];
   F32 clampv[4];

   // fix OpenGL t values for rendertargets are from bottom, not top
   tc[1] = height_max - tc[1];
   tc[3] = height_max - tc[3];

   clampv[0] = clamp[0];
   clampv[1] = height_max - clamp[3];
   clampv[2] = clamp[2];
   clampv[3] = height_max - clamp[1];

   use_lazy_shader(prg);
   set_texture(0, r->tex[0]);

   glColorMask(1,1,1,1);
   glDisable(GL_BLEND);
   glDisable(GL_SCISSOR_TEST);

   assert(prg->vars[0][VAR_blur_tap] >= 0);
   glUniform4fv(prg->vars[0][VAR_blur_tap], taps, data);
   glUniform4fv(prg->vars[0][VAR_blur_clampv], 1, clampv);

   do_screen_quad(s, tc, prg->vars[1], gstats, 0);
   tag_resources(r->tex[0],0,0);
}

static void gdraw_Colormatrix(GDrawRenderState *r, gswf_recti *s, float *tc, GDrawStats *gstats)
{
   ProgramWithCachedVariableLocations *prg = &gdraw->colormatrix;
   if (!gdraw_TextureDrawBufferBegin(s, GDRAW_TEXTURE_FORMAT_rgba32, GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_color | GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_alpha, nullptr, gstats))
      return;
   use_lazy_shader(prg);
   set_texture(0, r->tex[0]);
   glUniform4fv(prg->vars[0][VAR_colormatrix_data], 5, r->shader_data);
   do_screen_quad(s, tc, gdraw->colormatrix.vars[1], gstats, 0);
   tag_resources(r->tex[0],0,0);
   r->tex[0] = gdraw_TextureDrawBufferEnd(gstats);
}

static gswf_recti *get_valid_rect(GDrawTexture *tex)
{
   GDrawHandle *h = (GDrawHandle *) tex;
   S32 n = (S32) (h - gdraw->rendertargets.handle);
   assert(n >= 0 && n <= MAX_RENDER_STACK_DEPTH+1);
   return &gdraw->rt_valid[n];
}

static void set_clamp_constant(GLint constant, GDrawTexture *tex)
{
   gswf_recti *s = get_valid_rect(tex);
   // when we make the valid data, we make sure there is an extra empty pixel at the border
   // we also have to convert from GDraw coords to GL coords here.
   glUniform4f(constant,
      (           s->x0-0.5f) / gdraw->frametex_width,
      (gdraw->tph-s->y1-0.5f) / gdraw->frametex_height,
      (           s->x1+0.5f) / gdraw->frametex_width,
      (gdraw->tph-s->y0+0.5f) / gdraw->frametex_height);
}

static void gdraw_Filter(GDrawRenderState *r, gswf_recti *s, float *tc, int isbevel, GDrawStats *gstats)
{
   ProgramWithCachedVariableLocations *prg = &gdraw->filter_prog[isbevel][r->filter_mode];
   if (!gdraw_TextureDrawBufferBegin(s, GDRAW_TEXTURE_FORMAT_rgba32, GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_color | GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_alpha, nullptr, gstats))
      return;
   use_lazy_shader(prg);
   set_texture(0, r->tex[0]);
   set_texture(1, r->tex[1]);
   set_texture(2, r->tex[2]);
   glUniform4fv(prg->vars[0][VAR_filter_color], 1, &r->shader_data[0]);
   glUniform4f(prg->vars[0][VAR_filter_tc_off], -r->shader_data[4] / (F32)gdraw->frametex_width, r->shader_data[5] / (F32)gdraw->frametex_height, r->shader_data[6], 0);
   if (prg->vars[0][VAR_filter_color2] >= 0)
      glUniform4fv(prg->vars[0][VAR_filter_color2], 1, &r->shader_data[8]);
   set_clamp_constant(prg->vars[0][VAR_filter_clamp0], r->tex[0]);
   set_clamp_constant(prg->vars[0][VAR_filter_clamp1], r->tex[1]);
   do_screen_quad(s, tc, prg->vars[1], gstats, 0);
   tag_resources(r->tex[0],0,0);
   r->tex[0] = gdraw_TextureDrawBufferEnd(gstats);
}

static void RADLINK gdraw_FilterQuad(GDrawRenderState *r, S32 x0, S32 y0, S32 x1, S32 y1, GDrawStats *gstats)
{
   F32 tc[4];
   gswf_recti s;

   // clip to tile boundaries
   s.x0 = RR_MAX(x0, gdraw->tx0p);
   s.y0 = RR_MAX(y0, gdraw->ty0p);
   s.x1 = RR_MIN(x1, gdraw->tx0p + gdraw->tpw);
   s.y1 = RR_MIN(y1, gdraw->ty0p + gdraw->tph);
   if (s.x1 <= s.x0 || s.y1 <= s.y0)
      return;

   // if it's a rendertarget, it's inverted from our design because OpenGL is bottom-left 0,0
   // and we have to compensate for scaling
   tc[0] =               (s.x0 - gdraw->tx0p)  / (F32) gdraw->frametex_width;
   tc[1] = (gdraw->tph - (s.y0 + gdraw->ty0p)) / (F32) gdraw->frametex_height;
   tc[2] =               (s.x1 - gdraw->tx0p)  / (F32) gdraw->frametex_width;
   tc[3] = (gdraw->tph - (s.y1 - gdraw->ty0p)) / (F32) gdraw->frametex_height;

   glUseProgram(0);
   set_texture(0, 0);
   set_texture(1, 0);
   set_texture(2, 0);

   glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
   glStencilMask(255);
   glDisable(GL_STENCIL_TEST);
   glColorMask(1,1,1,1);
   glDisable(GL_BLEND);
   glDisable(GL_DEPTH_TEST);
   opengl_check();

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

            // blur passes must override the viewport/ortho projection

            gdraw->in_blur = true; // prevent viewport/projection munging in start/end texture
            set_viewport();
            set_projection();
            gdraw_Blur(&gdraw_funcs, &b,r, &s, &bounds, gstats);

            gdraw->in_blur = false;

            set_viewport();
            set_projection();
            break;
         }

         case GDRAW_FILTER_colormatrix:
            gdraw_Colormatrix(r, &s, tc, gstats);
            break;

         case GDRAW_FILTER_dropshadow:
            gdraw_Filter(r, &s, tc, 0, gstats);
            break;

         case GDRAW_FILTER_bevel:
            gdraw_Filter(r, &s, tc, 1, gstats);
            break;

         default:
            assert(0);
      }
   } else {
      GDrawTexture *blend_tex = nullptr;
      const int *vvars;

      // for crazy blend modes, we need to read back from the framebuffer
      // and do the blending in the pixel shader. we do this with CopyTexSubImage,
      // rather than trying to render-to-texture-all-along, because that's a pain.
      // @TODO: propogate the rectangle down and only copy what we need, like in 360

      if (r->blend_mode == GDRAW_BLEND_special) {
         blend_tex = (GDrawTexture *) get_color_rendertarget(gstats);
         glBindTexture(GL_TEXTURE_2D, ((GDrawHandle *) blend_tex)->handle.tex.gl);
         if (gdraw->cur != gdraw->frame)
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, 0,0,gdraw->tpw,gdraw->tph);
         else
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, gdraw->tx0 - gdraw->tx0p, gdraw->ty0 - gdraw->ty0p, gdraw->vx,gdraw->vy,gdraw->tw,gdraw->th);

         set_texture(1, blend_tex);
      }

      if (!set_render_state(r, GDRAW_vformat_v2tc2, &vvars, nullptr, gstats))
         return;
      do_screen_quad(&s, tc, vvars, gstats, 0);
      tag_resources(r->tex[0],r->tex[1],0);
      if (blend_tex)
         gdraw_FreeTexture(blend_tex, 0, gstats);
   }
}

void gdraw_GLx_(NoMoreGDrawThisFrame)(void)
{
   clear_renderstate();
   ++gdraw->frame_counter;
}

void gdraw_GLx_(BeginCustomDraw)(IggyCustomDrawCallbackRegion *region, F32 *matrix)
{
   clear_renderstate();
   gdraw_GetObjectSpaceMatrix(matrix, region->o2w, gdraw->projection, depth_from_id(0), 1);
}

void gdraw_GLx_(EndCustomDraw)(IggyCustomDrawCallbackRegion *region)
{
   set_common_renderstate();
}


///////////////////////////////////////////////////////////////////////
//
//   Vertex and Fragment program initialization
//

#include GDRAW_SHADERS

static void make_vars(GDrawGLProgram prog, S32 vars[2][8], char **varn)
{
   if (prog) {
      char **varn2 = (varn == pshader_general2_vars ? vshader_vsglihud_vars : vshader_vsgl_vars);
      S32 k;
      for (k=0; varn[k]; ++k)
         if (varn[k][0])
            vars[0][k] = glGetUniformLocation(prog, varn[k]);
         else
            vars[0][k] = -1;

      for (k=0; varn2[k]; ++k)
         if (varn2[k][0])
            vars[1][k] = glGetUniformLocation(prog, varn2[k]);
         else
            vars[1][k] = -1;

      if (vars[0][0] >= 0)
         assert(vars[0][0] != vars[0][1]);
   }
}

static void make_fragment_program(ProgramWithCachedVariableLocations *p, int num_strings, char **strings, char **varn)
{
   S32 i;
   GLint res;
   GDrawGLProgram shad;
   opengl_check();
   for (i=0; i < MAX_VARS; ++i) {
      p->vars[0][i] = -1;
      p->vars[1][i] = -1;
   }

   shad = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(shad, num_strings, (const GLchar **)strings, nullptr);
   glCompileShader(shad);
   glGetShaderiv(shad, GL_COMPILE_STATUS, &res);
   if (!res) {
      char errors[512];
      glGetShaderInfoLog(shad, sizeof(errors)-2, &res, errors);
      compilation_err(errors);
      p->program = 0;
   } else {
      S32 vert = GDRAW_vformat_v2tc2;
      if (p >= &gdraw->fprog[0][0][0] && p < &gdraw->fprog[GDRAW_TEXTURE__count][0][0]) {
         // for basic rendering shaders, we have three versions corresponding to the
         // three vertex formats we support.
         S32 n = (S32) (p - gdraw->fprog[0][0]);
         vert = n % 3;
      }

      if (p == &gdraw->ihud[0])
         vert = GDRAW_vformat_ihud1;

      opengl_check();
      p->program = glCreateProgram();
      glAttachShader(p->program, shad);
      glAttachShader(p->program, gdraw->vert[vert]);
      opengl_check();

      if (vert == GDRAW_vformat_ihud1) {
         glBindAttribLocation(p->program, 0, "position");
         glBindAttribLocation(p->program, 1, "texcoord");
         glBindAttribLocation(p->program, 2, "material_index");
      } else {
         glBindAttribLocation(p->program, 0, "position");
         glBindAttribLocation(p->program, 1, "in_attr");
      }

      glLinkProgram(p->program);
      glGetProgramiv(p->program, GL_LINK_STATUS, &res);
      if (!res) {
         char errors[512];
         glGetProgramiv(p->program, GL_INFO_LOG_LENGTH, &res);
         glGetProgramInfoLog(p->program, sizeof(errors)-2, &res, errors);
         compilation_err(errors);
         glDeleteShader(shad);
         glDeleteProgram(p->program);
         p->program = 0;
      } else
         make_vars(p->program, p->vars, varn);
   }
   opengl_check();
   glUseProgram(p->program); // now activate the program
   opengl_check();
}

static void make_vertex_program(GLuint *vprog, int num_strings, char **strings)
{
   GLint res;
   GDrawGLProgram shad;
   opengl_check();

   if(strings[0])
   {
      shad = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(shad, num_strings, (const GLchar **)strings, nullptr);
      glCompileShader(shad);
      glGetShaderiv(shad, GL_COMPILE_STATUS, &res);
      if (!res) {
         char errors[512];
         glGetShaderInfoLog(shad, sizeof(errors)-2, &res, errors);
         compilation_err(errors);
         glDeleteShader(shad);
         shad = 0;
      }
      opengl_check();
      *vprog = shad;
   }
   else
   {
      *vprog = 0;
   }
}

static void bind_sampler(ProgramWithCachedVariableLocations *prog, int varidx, int sampleridx)
{
   int var = prog->vars[0][varidx];
   if (var >= 0)
      glUniform1i(var, sampleridx);
}

static void make_vertex_programs(void)
{
   int type;
   for (type=0; type < GDRAW_vformat__basic_count; type++)
      make_vertex_program(&gdraw->vert[type], NUMFRAGMENTS_vshader_vsgl, vshader_vsgl(type));
   type = GDRAW_vformat_ihud1;
   make_vertex_program(&gdraw->vert[type], NUMFRAGMENTS_vshader_vsglihud, vshader_vsglihud());
}

static void lazy_shader(ProgramWithCachedVariableLocations *ptr)
{
   if (ptr >= &gdraw->fprog[0][0][0] && ptr < &gdraw->fprog[GDRAW_TEXTURE__count][0][0]) {
      S32 n = (S32) (ptr - gdraw->fprog[0][0]);
      n /= 3;

      make_fragment_program(ptr, NUMFRAGMENTS_pshader_basic, pshader_basic_arr[n], pshader_basic_vars);
      bind_sampler(ptr, VAR_tex0, 0);
      bind_sampler(ptr, VAR_tex1, AATEX_SAMPLER);
      return;
   }

   if (ptr >= &gdraw->exceptional_blend[0] && ptr < &gdraw->exceptional_blend[GDRAW_BLENDSPECIAL__count]) {
      S32 n = (S32) (ptr - gdraw->exceptional_blend);
      make_fragment_program(ptr, NUMFRAGMENTS_pshader_exceptional_blend, pshader_exceptional_blend_arr[n], pshader_exceptional_blend_vars);
      bind_sampler(ptr, VAR_tex0, 0);
      bind_sampler(ptr, VAR_tex1, 1);
      return;
   }

   if (ptr >= &gdraw->filter_prog[0][0] && ptr <= &gdraw->filter_prog[1][15]) {
      S32 n = (S32) (ptr - gdraw->filter_prog[0]);
      make_fragment_program(ptr, NUMFRAGMENTS_pshader_filter, pshader_filter_arr[n], pshader_filter_vars);
      bind_sampler(ptr, VAR_filter_tex0, 0);
      bind_sampler(ptr, VAR_filter_tex1, 1);
      bind_sampler(ptr, VAR_filter_tex2, 2);
      return;
   }

   if (ptr >= &gdraw->blur_prog[0] && ptr <= &gdraw->blur_prog[MAX_TAPS]) {
      S32 n = (S32) (ptr - gdraw->blur_prog);
      make_fragment_program(ptr, NUMFRAGMENTS_pshader_blur, pshader_blur_arr[n], pshader_blur_vars);
      bind_sampler(ptr, VAR_blur_tex0, 0);
      return;
   }

   if (ptr == &gdraw->colormatrix) {
      make_fragment_program(ptr, NUMFRAGMENTS_pshader_color_matrix, pshader_color_matrix_arr[0], pshader_color_matrix_vars);
      bind_sampler(ptr, VAR_colormatrix_tex0, 0);
      return;
   }

   if (ptr == &gdraw->manual_clear) {
      make_fragment_program(ptr, NUMFRAGMENTS_pshader_manual_clear, pshader_manual_clear_arr[0], pshader_manual_clear_vars);
      return;
   }

   if (ptr == &gdraw->ihud[0]) {
      make_fragment_program(ptr, NUMFRAGMENTS_pshader_general2, pshader_general2_arr[0], pshader_general2_vars);
      bind_sampler(ptr, VAR_tex0, 0);
      return;
   }

   RR_BREAK();
}

static rrbool make_quad_indices(void)
{
   int size = QUAD_IB_COUNT * 6 * sizeof(GLushort);
   GLushort *inds = IggyGDrawMalloc(size);
   int i, e;

   if (!inds)
      return 0;

   // make quad inds
   for (i=0; i < QUAD_IB_COUNT; i++) {
      inds[i*6 + 0] = (GLushort) (i*4 + 0);
      inds[i*6 + 1] = (GLushort) (i*4 + 1);
      inds[i*6 + 2] = (GLushort) (i*4 + 2);
      inds[i*6 + 3] = (GLushort) (i*4 + 0);
      inds[i*6 + 4] = (GLushort) (i*4 + 2);
      inds[i*6 + 5] = (GLushort) (i*4 + 3);
   }

   glGenBuffers(1, &gdraw->quad_ib);
   glBindBuffer(GL_ARRAY_BUFFER, gdraw->quad_ib);
   glBufferData(GL_ARRAY_BUFFER, size, inds, GL_STATIC_DRAW);
   IggyGDrawFree(inds);
   e = glGetError();
   if (e != GL_NO_ERROR) {
      eat_gl_err();
      return 0;
   }

   return 1;
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
static GDrawResourceLimit gdraw_limits[GDRAW_GLx_(RESOURCE__count)] = {
   MAX_RENDER_STACK_DEPTH + 1, 16*1024*1024,  // GDRAW_GLx_RESOURCE_rendertarget
    500,                       20*1024*1024,  // GDRAW_GLx_RESOURCE_texture
   1000,                        2*1024*1024,  // GDRAW_GLx_RESOURCE_vertexbuffer
};

static GDrawHandleCache *make_handle_cache(gdraw_resourcetype type)
{
   S32 num_handles = gdraw_limits[type].num_handles;
   S32 num_bytes = gdraw_limits[type].num_bytes;
   GDrawHandleCache *cache = (GDrawHandleCache *) IggyGDrawMalloc(sizeof(GDrawHandleCache) + (num_handles - 1) * sizeof(GDrawHandle));
   if (cache) {
      gdraw_HandleCacheInit(cache, num_handles, num_bytes);
      cache->is_vertex = (type == GDRAW_GLx_(RESOURCE_vertexbuffer));
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

int gdraw_GLx_(SetResourceLimits)(gdraw_resourcetype type, S32 num_handles, S32 num_bytes)
{
   GDrawStats stats={0};

   if (type == GDRAW_GLx_(RESOURCE_rendertarget)) // RT count is small and space is preallocated
      num_handles = MAX_RENDER_STACK_DEPTH + 1;

   assert(type >= GDRAW_GLx_(RESOURCE_rendertarget) && type < GDRAW_GLx_(RESOURCE__count));
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
      case GDRAW_GLx_(RESOURCE_rendertarget):
         flush_rendertargets(&stats);
         gdraw_HandleCacheInit(&gdraw->rendertargets, num_handles, num_bytes);
         return 1;
         
      case GDRAW_GLx_(RESOURCE_texture):
         if (gdraw->texturecache) {
            gdraw_res_flush(gdraw->texturecache, &stats);
            IggyGDrawFree(gdraw->texturecache);
         }
         gdraw->texturecache = make_handle_cache(GDRAW_GLx_(RESOURCE_texture));
         return gdraw->texturecache != nullptr;

      case GDRAW_GLx_(RESOURCE_vertexbuffer):
         if (gdraw->vbufcache) {
            gdraw_res_flush(gdraw->vbufcache, &stats);
            IggyGDrawFree(gdraw->vbufcache);
         }
         gdraw->vbufcache = make_handle_cache(GDRAW_GLx_(RESOURCE_vertexbuffer));
         return gdraw->vbufcache != nullptr;

      default:
         return 0;
   }
}

GDrawTexture * RADLINK gdraw_GLx_(MakeTextureFromResource)(U8 *resource_file, S32 len, IggyFileTextureRaw *texture)
{
   int i, offset, mips;
   const TextureFormatDesc *fmt;
   GDrawTexture *tex;
   GLuint gl_texture_handle;

   // look up the texture format
   fmt = gdraw->tex_formats;
   while (fmt->iggyfmt != texture->format && fmt->blkbytes)
      fmt++;
   if (!fmt->blkbytes) // end of list - i.e. format not supported
      return nullptr;

   // prepare texture
   glGenTextures(1, &gl_texture_handle);
   if (gl_texture_handle == 0)
      return nullptr;

   opengl_check();
   make_texture(gl_texture_handle);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   offset = texture->file_offset;
   mips = RR_MAX(texture->mipmaps, 1);

   // disable mipmaps if non-pow-2 is unsupported
   if (gdraw->has_conditional_non_power_of_two)
      if (!is_pow2(texture->w) || !is_pow2(texture->h))
         mips = 1;

   // disable mipmaps if chain is incomplete and GL_TEXTURE_MAX_LEVEL is unsupported
   if (!gdraw->has_texture_max_level && mips > 1) {
      int lastmip = mips-1;
      if ((texture->w >> lastmip) > 1 || (texture->h >> lastmip) > 1)
         mips = 1;
   }

   for (i=0; i < mips; i++) {
      U8 *data = resource_file + offset;
      int w = RR_MAX(texture->w >> i, 1);
      int h = RR_MAX(texture->h >> i, 1);
      int j;

      if (texture->format == IFT_FORMAT_rgba_4444_LE) {
         for (j=0; j < w * h; ++j) {
            unsigned short x = * (unsigned short *) (data + j*2);
            x = ((x>>12) & 0xf) | ((x<<4) & 0xfff0);
            * (unsigned short *) (data + j*2) = x;
         }
      }
      if (texture->format == IFT_FORMAT_rgba_5551_LE) {
         for (j=0; j < w * h; ++j) {
            unsigned short x = * (unsigned short *) (data + j*2);
            x = (x >> 15) | (x << 1);
            * (unsigned short *) (data + j*2) = x;
         }
      }

      if (fmt->fmt != 0) {
         glTexImage2D(GL_TEXTURE_2D, i, fmt->intfmt, w, h, 0, fmt->fmt, fmt->type, data);
         offset += w * h * fmt->blkbytes;
      } else {
         int size = ((w + fmt->blkx-1) / fmt->blkx) * ((h + fmt->blky-1) / fmt->blky) * fmt->blkbytes;
         glCompressedTexImage2D(GL_TEXTURE_2D, i, fmt->intfmt, w, h, 0, size, data);
         offset += size;
      }

      opengl_check();
   }

   if (gdraw->has_texture_max_level)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mips-1);

   tex = gdraw_GLx_(WrappedTextureCreate)(gl_texture_handle, texture->w, texture->h, mips > 1);
   if (tex == nullptr)
      glDeleteTextures(1, &gl_texture_handle);
   opengl_check();
   return tex;
}

void RADLINK gdraw_GLx_(DestroyTextureFromResource)(GDrawTexture *tex)
{
   if (tex)
      gdraw_GLx_(WrappedTextureDestroy)(tex);
}

static rrbool hasext(const char *exts, const char *which)
{
   const char *where;
   size_t len;

#ifdef GDRAW_USE_glGetStringi
   if (exts == nullptr) {
      GLint i, num_exts;
      glGetIntegerv(GL_NUM_EXTENSIONS, &num_exts);
      for (i=0; i < num_exts; ++i)
         if (0==strcmp(which, (char const *) glGetStringi(GL_EXTENSIONS, i)))
            return 1;
      return 0;
   }
#endif

   where = exts;
   len = strlen(which);

   for(;;) {
      where = strstr(where, which);
      if (where == nullptr)
         return false;

      if (   (where == exts || *(where - 1) == ' ')   // starts with terminator
          && (where[len] == ' ' || where[len] == 0))  // ends with terminator
         return true;
      where += len;
   }
}

static GDrawFunctions *create_context(S32 w, S32 h)
{
   gdraw = IggyGDrawMalloc(sizeof(*gdraw));
   if (!gdraw) return nullptr;

   memset(gdraw, 0, sizeof(*gdraw));

   gdraw->texturecache = make_handle_cache(GDRAW_GLx_(RESOURCE_texture));
   gdraw->vbufcache = make_handle_cache(GDRAW_GLx_(RESOURCE_vertexbuffer));
   gdraw_HandleCacheInit(&gdraw->rendertargets, gdraw_limits[GDRAW_GLx_(RESOURCE_rendertarget)].num_handles, gdraw_limits[GDRAW_GLx_(RESOURCE_rendertarget)].num_bytes);

   if (!gdraw->texturecache || !gdraw->vbufcache || !make_quad_indices()) {
      free_gdraw();
      return nullptr;
   }

   opengl_check();

   gdraw->frametex_width = w;
   gdraw->frametex_height = h;
   gdraw->frame->cached = false;

   // if the globals have already been initialized, this has no effect;
   // otherwise it initializes them with no global texture storage and the
   // default global rendertarget storage

   glGenFramebuffers(1, &gdraw->framebuffer_stack_object);
   glGenFramebuffers(1, &gdraw->framebuffer_copy_to_texture);
   opengl_check();

   make_vertex_programs();
   // fragment shaders are created lazily

   gdraw_funcs.SetViewSizeAndWorldScale = gdraw_SetViewSizeAndWorldScale;
   gdraw_funcs.RenderingBegin  = gdraw_RenderingBegin;
   gdraw_funcs.RenderingEnd    = gdraw_RenderingEnd;
   gdraw_funcs.RenderTileBegin = gdraw_RenderTileBegin;
   gdraw_funcs.RenderTileEnd   = gdraw_RenderTileEnd;
   gdraw_funcs.GetInfo         = gdraw_GetInfo;
   gdraw_funcs.DescribeTexture = gdraw_DescribeTexture;
   gdraw_funcs.DescribeVertexBuffer = gdraw_DescribeVertexBuffer;

   gdraw_funcs.TextureDrawBufferBegin = gdraw_TextureDrawBufferBegin;
   gdraw_funcs.TextureDrawBufferEnd = gdraw_TextureDrawBufferEnd;

   gdraw_funcs.DrawIndexedTriangles = gdraw_DrawIndexedTriangles;
   gdraw_funcs.FilterQuad = gdraw_FilterQuad;

   gdraw_funcs.SetAntialiasTexture = gdraw_SetAntialiasTexture;

   gdraw_funcs.ClearStencilBits = gdraw_ClearStencilBits;
   gdraw_funcs.ClearID = gdraw_ClearID;

   gdraw_funcs.MakeTextureBegin = gdraw_MakeTextureBegin;
   gdraw_funcs.MakeTextureMore = nullptr;
   gdraw_funcs.MakeTextureEnd = gdraw_MakeTextureEnd;

   gdraw_funcs.UpdateTextureRect = gdraw_UpdateTextureRect;
   gdraw_funcs.UpdateTextureBegin = gdraw_UpdateTextureBegin;
   gdraw_funcs.UpdateTextureEnd = gdraw_UpdateTextureEnd;
   gdraw_funcs.FreeTexture = gdraw_FreeTexture;
   gdraw_funcs.TryToLockTexture = gdraw_TryToLockTexture;

   gdraw_funcs.MakeVertexBufferBegin = gdraw_MakeVertexBufferBegin;
   gdraw_funcs.MakeVertexBufferMore  = gdraw_MakeVertexBufferMore;
   gdraw_funcs.MakeVertexBufferEnd   = gdraw_MakeVertexBufferEnd;
   gdraw_funcs.TryToLockVertexBuffer = gdraw_TryToLockVertexBuffer;
   gdraw_funcs.FreeVertexBuffer = gdraw_FreeVertexBuffer;

   gdraw_funcs.UnlockHandles = gdraw_UnlockHandles;
   gdraw_funcs.SetTextureUniqueID = gdraw_SetTextureUniqueID;

   gdraw_funcs.MakeTextureFromResource = (gdraw_make_texture_from_resource *) gdraw_GLx_(MakeTextureFromResource);
   gdraw_funcs.FreeTextureFromResource = gdraw_GLx_(DestroyTextureFromResource);

   gdraw_funcs.Set3DTransform = gdraw_Set3DTransform;

   return &gdraw_funcs;
}

void gdraw_GLx_(DestroyContext)(void)
{
   if (gdraw)
   {
      GDrawStats stats={0};
      if (gdraw->texturecache)   gdraw_res_flush(gdraw->texturecache, &stats);
      if (gdraw->vbufcache)      gdraw_res_flush(gdraw->vbufcache, &stats);
      flush_rendertargets(&stats);

      if (gdraw->aa_tex)
         glDeleteTextures(1, &gdraw->aa_tex);

      if (gdraw->quad_ib)
         glDeleteBuffers(1, &gdraw->quad_ib);
   }

   opengl_check();
   free_gdraw();
}

