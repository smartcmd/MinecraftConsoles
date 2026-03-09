#pragma once
// gdraw_d3d11.h - author: Fabian Giesen - copyright 2011 RAD Game Tools
//
// Interface for creating a D3D11 GDraw driver.

#define IDOC
//idoc(parent,GDraw_d3d11)

typedef enum gdraw_d3d11_resourcetype
{
   GDRAW_D3D11_RESOURCE_rendertarget,
   GDRAW_D3D11_RESOURCE_texture,
   GDRAW_D3D11_RESOURCE_vertexbuffer,
   GDRAW_D3D11_RESOURCE_dynbuffer,     // Streaming buffer for dynamic vertex/index data (handle count ignored)

   GDRAW_D3D11_RESOURCE__count,
} gdraw_d3d11_resourcetype;

IDOC extern int gdraw_D3D11_SetResourceLimits(gdraw_d3d11_resourcetype type, S32 num_handles, S32 num_bytes);
/* This sets how large the memory pool for a given resource types is, and how many handles
   GDraw should allocate for it. GDraw keeps track of allocations in each pool, and will free
   old resources in a LRU manner to make space if one of the limits is about to be exceeded.

   Returns 1 if value successfully changed, 0 on error.
   You need to call IggyPlayerFlushAll on all active Iggys before you do this to make
   them flush their resources since changing the resource limits invalidates all handles.
   You also need to call IggyFlushInstalledFonts if you have any installed fonts.
*/

IDOC extern GDrawFunctions * gdraw_D3D11_CreateContext(ID3D11Device *dev, ID3D11DeviceContext *ctx, S32 w, S32 h);
/* Creates a GDraw context for rendering using D3D. You need to pass in the D3D device,
   the device context to use for rendering, and the width/height of render target textures.

   The width/height is used solely for sizing internal rendertargets. They will be
   allocated to the larger of this size and the size of any rendered tiles (with padding).
   In other words, you can pass in (0,0) and the rendertargets will be allocated to the
   right size. However, if you draw multiple Iggy files or tiles of different sizes,
   they might first be allocated too small; it's best to pass in the correct size initially
   to avoid unnecessary allocation/deallocation of too-small rendertargets.

   There can only be one D3D GDraw context active at any one time.

   If initialization fails for some reason (the main reason would be an out of memory condition),
   nullptr is returned. Otherwise, you can pass the return value to IggySetGDraw. */

IDOC extern void gdraw_D3D11_DestroyContext(void);
/* Destroys the current GDraw context, if any. */

IDOC extern void gdraw_D3D11_SetErrorHandler(void (__cdecl *error_handler)(HRESULT hr));
/* Sets the GDraw D3D error handler.

   This will get called with the respective D3D error code if GDraw encounters an error
   that it can't handle by itself (e.g. running out of state objects). */

IDOC extern void gdraw_D3D11_SetRendertargetSize(S32 w, S32 h);
/* Changes the current render target size (and recreates all rendertargets if necessary).
   This allows you to shrink the rendertargets if the new needed size is smaller
   than it was previously. As with $gdraw_D3D11_CreateContext, the width and
   height specified here are only minimums; GDraw will reallocate larger rendertargets
   as needed. */

IDOC extern void gdraw_D3D11_SetTileOrigin(ID3D11RenderTargetView *main_rt, ID3D11DepthStencilView *main_ds,
                                      ID3D11ShaderResourceView *non_msaa_rt, S32 x, S32 y);
/* This sets the main rendertarget and matching depth/stencil buffer that GDraw
   should render to and the x/y position of the output location of the top-left
   of the current tile (allowing you to finely-position content, or to do tiled
   rendering).

   If your rendertarget uses multisampling, you also need to specify a shader
   resource view for a non-MSAA rendertarget texture (identically sized to main_rt)
   in non_msaa_rt. This is only used if the Flash content includes non-standard
   blend modes which have to use a special blend shader, so you can leave it nullptr
   if you forbid such content.

   You need to call this before Iggy calls any rendering functions. */

IDOC extern void gdraw_D3D11_NoMoreGDrawThisFrame(void);
/* Tells GDraw that no more rendering operations will occur this frame. This triggers
   some end-of-frame processing; most importantly, GDraw uses this call as a marker to
   detect thrashing (and react accordingly), so please do not forget to call this
   every frame! (As long as Iggy does any rendering, that is) */

IDOC extern void gdraw_D3D11_PreReset(void);
/* Call this before D3D device Reset(); it will free all default pool resources allocated
   by GDraw. */

IDOC extern void gdraw_D3D11_PostReset(void);
/* Call after D3D device Reset(). */

IDOC extern void RADLINK gdraw_D3D11_BeginCustomDraw_4J(IggyCustomDrawCallbackRegion *Region, F32 mat[16]);
IDOC extern void RADLINK gdraw_D3D11_CalculateCustomDraw_4J(IggyCustomDrawCallbackRegion *Region, F32 mat[16]);
IDOC extern void RADLINK gdraw_D3D11_BeginCustomDraw(IggyCustomDrawCallbackRegion *Region, F32 mat[4][4]);
/* Call at the beginning of Iggy custom draw callback to clear any odd render states GDraw has
   set on the D3D device, and to get the current 2D object-to-world transformation. */

IDOC extern void RADLINK gdraw_D3D11_EndCustomDraw(IggyCustomDrawCallbackRegion *Region);
/* Call at the end of Iggy custom draw callback so GDraw can restore its render states. */

IDOC extern void RADLINK gdraw_D3D11_GetResourceUsageStats(gdraw_d3d11_resourcetype type, S32 *handles_used, S32 *bytes_used);
/* D3D only: Get resource usage stats for last frame.
   This can be used to get an estimate of how much graphics memory got used by GDraw
   during the last frame.

   For the dynbuffer, this always returns 0 in handles_used and the *size of the largest
   single allocation* in bytes_used. It needs to be sized so that this allocation fits;
   make it smaller and it won't work, but if you make it much larger (say more than 2x
   as big), it's just a waste of memory. That said, we still recommend to make it no
   smaller than 64k, and the default is 256k.

   Caveat: This counts the number of bytes that GDraw knows about. 3D hardware usually
   has its own management overhead, alignment requirements, allocation granularity
   and so on. In short, this is not an accurate estimate of how much memory is actually
   used by the GPU - it is a lower bound, though, and makes for a useful ballpark estimate. */

IDOC extern GDrawTexture *gdraw_D3D11_WrappedTextureCreate(ID3D11ShaderResourceView *tex_view);
/* Create a wrapped texture from a shader resource view.
   A wrapped texture can be used to let Iggy draw using the contents of a texture
   you create and manage on your own. For example, you might render to this texture,
   or stream video into it. Wrapped textures take up a handle. They will never be
   freed or otherwise modified by GDraw; nor will GDraw change any reference counts.
   All this is up to the application. */

IDOC extern void gdraw_D3D11_WrappedTextureChange(GDrawTexture *tex, ID3D11ShaderResourceView *tex_view);
/* Switch an existing GDrawTexture * that represents a wrapped texture to use
   a new underlying D3D view. For example, you might internally double-buffer
   a dynamically updated texture. As above, GDraw will leave this texture alone
   and not touch any reference counts. */

IDOC extern void gdraw_D3D11_WrappedTextureDestroy(GDrawTexture *tex);
/* Destroys the GDraw wrapper for a wrapped texture object. This will free up
   a GDraw texture handle but not release the associated D3D texture; that is
   up to you. */

GDrawTexture * RADLINK gdraw_D3D11_MakeTextureFromResource(U8 *resource_file, S32 length, IggyFileTextureRaw *texture);
void RADLINK gdraw_D3D11_DestroyTextureFromResource(GDrawTexture *tex);
// 4J added
extern void RADLINK gdraw_D3D11_setViewport_4J();