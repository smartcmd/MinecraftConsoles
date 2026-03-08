// gdraw.h - author: Sean Barrett - copyright 2009 RAD Game Tools
//
// This is the graphics rendering abstraction that Iggy is implemented
// on top of.

#ifndef __RAD_INCLUDE_GDRAW_H__
#define __RAD_INCLUDE_GDRAW_H__

#include "rrCore.h"

#define IDOC

RADDEFSTART

//idoc(parent,GDrawAPI_Buffers) 

#ifndef IGGY_GDRAW_SHARED_TYPEDEF

   #define IGGY_GDRAW_SHARED_TYPEDEF
   typedef struct GDrawFunctions GDrawFunctions;

   typedef struct GDrawTexture GDrawTexture;

#endif//IGGY_GDRAW_SHARED_TYPEDEF



IDOC typedef struct GDrawVertexBuffer GDrawVertexBuffer;
/* An opaque handle to an internal GDraw vertex buffer. */

//idoc(parent,GDrawAPI_Base)

IDOC typedef struct gswf_recti
{
   S32 x0,y0; // Minimum corner of the rectangle
   S32 x1,y1; // Maximum corner of the rectangle
} gswf_recti;
/* A 2D rectangle with integer coordinates specifying its minimum and maximum corners. */

IDOC typedef struct gswf_rectf
{
   F32 x0,y0; // Minimum corner of the rectangle
   F32 x1,y1; // Maximum corner of the rectangle
} gswf_rectf;
/* A 2D rectangle with floating-point coordinates specifying its minimum and maximum corners. */

IDOC typedef struct gswf_matrix
{
   union {
      F32 m[2][2]; // 2x2 transform matrix 
      struct {
          F32 m00; // Alternate name for m[0][0], for coding convenience
          F32 m01; // Alternate name for m[0][1], for coding convenience
          F32 m10; // Alternate name for m[1][0], for coding convenience
          F32 m11; // Alternate name for m[1][1], for coding convenience
      };
   };
   F32 trans[2]; // 2D translation vector (the affine component of the matrix)
} gswf_matrix;
/* A 2D transform matrix plus a translation offset. */

#define GDRAW_STATS_batches    1
#define GDRAW_STATS_blits      2
#define GDRAW_STATS_alloc_tex  4
#define GDRAW_STATS_frees      8
#define GDRAW_STATS_defrag     16
#define GDRAW_STATS_rendtarg   32
#define GDRAW_STATS_clears     64
IDOC typedef struct GDrawStats
{
   S16 nonzero_flags;  // which of the fields below are non-zero

   U16 num_batches;    // number of batches, e.g. DrawPrim, DrawPrimUP
   U16 num_blits;      // number of blit operations (resolve, msaa resolve, blend readback)
   U16 freed_objects;  // number of cached objects freed
   U16 defrag_objects; // number of cached objects defragmented
   U16 alloc_tex;      // number of textures/buffers allocated
   U16 rendertarget_changes; // number of rendertarget changes
   U16 num_clears;
                                 //0 mod 8

   U32 drawn_indices;  // number of indices drawn (3 times number of triangles)
   U32 drawn_vertices; // number of unique vertices referenced
   U32 num_blit_pixels;// number of pixels in blit operations
   U32 alloc_tex_bytes;// number of bytes in textures/buffers allocated
   U32 freed_bytes;    // number of bytes in freed cached objects
   U32 defrag_bytes;   // number of bytes in defragmented cached objects
   U32 cleared_pixels; // number of pixels cleared by clear operation
   U32 reserved;
                                 //0 mod 8
} GDrawStats;
/* A structure with statistics information to show in resource browser/Telemetry */

////////////////////////////////////////////////////////////
//
// Queries
//
//idoc(parent,GDrawAPI_Queries)

IDOC typedef enum gdraw_bformat
{
   GDRAW_BFORMAT_vbib, // Platform uses vertex and index buffers
   GDRAW_BFORMAT_wii_dlist, // Platform uses Wii-style display lists
   GDRAW_BFORMAT_vbib_single_format, // Platform uses vertex and index buffers, but doesn't support multiple vertex formats in a single VB

   GDRAW_BFORMAT__count,
} gdraw_bformat;
/* Specifies what data format GDraw expects in MakeVertexBuffer_* and DrawIndexedTriangles.

   Most supported platforms prefer Vertex and Index buffers so that's what we use,
   but this format turns out to be somewhat awkward for Wii, so we use the native
   graphics processor display list format on that platform. */

IDOC typedef struct GDrawInfo
{
   S32 num_stencil_bits;        // number of (possibly emulated) stencil buffer bits
   U32 max_id;                  // number of unique values that can be easily encoded in zbuffer
   U32 max_texture_size;        // edge length of largest square texture supported by hardware
   U32 buffer_format;           // one of $gdraw_bformat
   rrbool shared_depth_stencil; // does 0'th framebuffer share depth & stencil with others? (on GL it can't?)
   rrbool always_mipmap;        // if GDraw can generate mipmaps nearly for free, then set this flag
   rrbool conditional_nonpow2;  // non-pow2 textures supported, but only using clamp and without mipmaps
   rrbool has_rendertargets;    // if true, then there is no rendertarget stack support
   rrbool no_nonpow2;           // non-pow2 textures aren't supported at all
} GDrawInfo; // must be a multiple of 8
/* $GDrawInfo contains the information that Iggy needs to know about
   what a GDraw implementation supports and what limits it places on
   certain important values. */

IDOC typedef void RADLINK gdraw_get_info(GDrawInfo *d);
/* Iggy queries this at the beginning of rendering to get information
   about the viewport and the device capabilities. */

////////////////////////////////////////////////////////////
//
// Drawing State
//
//idoc(parent,GDrawAPI_DrawingState)

IDOC typedef enum gdraw_blend
{
   GDRAW_BLEND_none, // Directly copy
   GDRAW_BLEND_alpha, // Use the source alpha channel to modulate its contribution
   GDRAW_BLEND_multiply, // Multiply colors componentwise
   GDRAW_BLEND_add, // Add the source and destination together

   GDRAW_BLEND_filter, // Uses a secondary $gdraw_filter specification to determine how to blend
   GDRAW_BLEND_special, // Uses a secondary $gdraw_blendspecial specification to determine how to blend

   GDRAW_BLEND__count,
} gdraw_blend;
/* Identifier indicating the type of blending operation to use when rendering.*/

IDOC typedef enum gdraw_blendspecial
{
   GDRAW_BLENDSPECIAL_layer, // s
   GDRAW_BLENDSPECIAL_multiply, // s*d
   GDRAW_BLENDSPECIAL_screen, // sa*da - (da-d)*(sa-s)
   GDRAW_BLENDSPECIAL_lighten, // max(sa*d,s*da)
   GDRAW_BLENDSPECIAL_darken, // min(sa*d,s*da)
   GDRAW_BLENDSPECIAL_add, // min(d+s,1.0)
   GDRAW_BLENDSPECIAL_subtract, // max(d-s,0.0)
   GDRAW_BLENDSPECIAL_difference, // abs(sa*d-s*da)
   GDRAW_BLENDSPECIAL_invert, // sa*(da-d)
   GDRAW_BLENDSPECIAL_overlay, // d < da/2.0 ? (2.0*s*d) : (sa*da - 2.0*(da-d)*(sa-s))
   GDRAW_BLENDSPECIAL_hardlight, // s < sa/2.0 ? (2.0*s*d) : (sa*da - 2.0*(da-d)*(sa-s))

   // these do extra-special math on the output alpha
   GDRAW_BLENDSPECIAL_erase, // d*(1.0-sa)
   GDRAW_BLENDSPECIAL_alpha_special, // d*sa

   GDRAW_BLENDSPECIAL__count,
} gdraw_blendspecial;
/* Specifies a type of "special" blend mode, which is defined as one
   that has to read from the framebuffer to compute its effect.

   These modes are only used with a 1-to-1 textured quad containing
   the exact output data in premultiplied alpha.  They all need to
   read from the framebuffer to compute their effect, so a GDraw
   implementation will usually need a custom path to handle that.
   Users will not warn in advance whether you're going to need this
   operation, so implementations either need to always render to a
   texture in case it happens, or copy the framebuffer to a texture
   when it does.

   Note that $(gdraw_blendspecial::GDRAW_BLENDSPECIAL_erase) and
   $(gdraw_blendspecial::GDRAW_BLENDSPECIAL_alpha_special) are unique
   among $gdraw_blendspecial modes in that they may not actually need
   to be implemented with the destination input as a texture if
   the destination buffer doesn't have an alpha channel. */

// (@OPTIMIZE: the last filter in each chain could be combined with
// the final blend, although only worth doing if the final blend is
// ALPHA/ADD/MULTIPLY--it's usually ALPHA though so worth doing!)
IDOC typedef enum gdraw_filter
{
   GDRAW_FILTER_blur, // Blurs the source image
   GDRAW_FILTER_colormatrix, // Transform RGB pixel values by a matrix
   GDRAW_FILTER_bevel, // Bevels the source image
   GDRAW_FILTER_dropshadow, // Adds a dropshadow underneath the source image

   GDRAW_FILTER__count,
} gdraw_filter;
/* Specifies a type of post-processing graphics filter.

   These modes are only used to implement filter effects, and will
   always be blending from a temporary buffer to another temporary
   buffer with no blending, so in general they should not require
   any additional input.
*/

IDOC typedef enum gdraw_texture
{
   GDRAW_TEXTURE_none,     // No texture applied
   GDRAW_TEXTURE_normal,   // Texture is bitmap or linear gradient
   GDRAW_TEXTURE_alpha,    // Texture is an alpha-only font bitmap
   GDRAW_TEXTURE_radial,   // Texture is a radial gradient
   GDRAW_TEXTURE_focal_gradient, // Texture is a "focal" radial gradient
   GDRAW_TEXTURE_alpha_test,  // Texture is an alpha-only font bitmap, alpha test for alpha >= 0.5

   GDRAW_TEXTURE__count,
} gdraw_texture;
/* Specifies how to apply a texture while rendering. */

IDOC typedef enum gdraw_wrap
{
   GDRAW_WRAP_clamp, // Texture coordinates clamped to edges
   GDRAW_WRAP_repeat, // Texture repeats periodically
   GDRAW_WRAP_mirror, // Repeat periodically, mirror on odd repetititions
   GDRAW_WRAP_clamp_to_border, // only used internally by some GDraws

   GDRAW_WRAP__count,
} gdraw_wrap;
/* Specifies what to do with texture coordinates outside [0,1]. */

typedef struct GDrawRenderState
{
   S32 id;                  // Object "identifier" used for high-quality AA mode
   U32 test_id:1;           // Whether to test zbuffer == id
   U32 set_id:1;            // Whether to set zbuffer == id
   U32 use_world_space:1;   // Whether primitive is defined in object space or world space
   U32 scissor:1;           // Whether rendering will be clipped to $(GDrawRenderState::scissor_rect)
   U32 identical_state:1;   // Whether state is identical to the one used for the previous draw call
   U32 unused:27;
                                       //aligned 0 mod 8

   U8 texgen0_enabled;      // Whether to use texgen for tex0
   U8 tex0_mode;            // One of $gdraw_texture
   U8 wrap0;                // One of $gdraw_wrap
   U8 nearest0;             // Whether to sample texture 0 nearest neighbor

   U8 blend_mode;       // One of $gdraw_blend 
   U8 special_blend;    // One of $gdraw_blendspecial (used only if $(GDrawRenderState::blend_mode) == $(gdraw_blend::GDRAW_BLEND_special)
   U8 filter;           // One of $gdraw_filter (used only if $(GDrawRenderState::blend_mode) == $(gdraw_blend::GDRAW_BLEND_filter)
   U8 filter_mode;      // Used to select the right compositing operation for the $(gdraw_filter::GDRAW_FILTER_bevel) and $(gdraw_filter::GDRAW_FILTER_dropshadow) modes
                                       //aligned 0 mod 8
   U8 stencil_test;     // Only draw if these stencil bits are "set"
   U8 stencil_set;      // "Set" these stencil bits (note that actual implementation initializes stencil to 1, and "set" makes them 0)

   U8 reserved[2];      // Currently unused (used to make padding to 4/8-byte boundary for following pointer explicit)
   S32 blur_passes;     // For filters that include blurring, this is the number of box filter passes to run
                                       //align 0 mod 8

   S16 *cxf_add;        // Color transform addition (discourage additive alpha!)

   GDrawTexture *tex[3]; // One or more textures to apply -- need 3 for gradient dropshadow.
                                       //0 mod 8
   F32 *edge_matrix;     // Screen to object space matrix (for edge antialiasing)
   gswf_matrix *o2w;     // Object-to-world matrix

   // --- Everything below this point must be manually initialized

                                       //0 mod 8
   F32 color[4];         // Color of the object

                                       //0 mod 8
   gswf_recti scissor_rect; // The rectangle to which rendering will be clipped if $(GDrawRenderState::scissor) is set 
                                       //0 mod 8
   // --- Everything below this point might be uninitialized if it's not used for this particular render state

   F32 s0_texgen[4];     // "s" (x) row of texgen matrix
   F32 t0_texgen[4];     // "t" (y) row of texgen matrix
                                       //0 mod 8
   F32 focal_point[4];  // Data used for $(gdraw_texgen_mode::GDRAW_TEXTURE_focal_gradient)
                                       //0 mod 8
   F32 blur_x,blur_y;   // The size of the box filter, where '1' is the identity and 2 adds half a pixel on each side
                                       //0 mod 8
   F32 shader_data[20]; // Various data that depends on filter (e.g. drop shadow direction, color)
} GDrawRenderState;
/* Encapsulation of the entire drawing state that affects a rendering command. */

IDOC typedef void RADLINK gdraw_set_view_size_and_world_scale(S32 w, S32 h, F32 x_world_to_pixel, F32 y_world_to_pixel);
/* Sets the size of the rendering viewport and the world to pixel scaling.

   Iggy calls this function with the full size that the viewport would
   be if it were rendered untiled, even if it will eventually be
   rendered as a collection of smaller tiles.
   
   The world scale is used to compensate non-square pixel aspect ratios
   when rendering wide lines. Both scale factors are 1 unless Iggy is
   running on a display with non-square pixels. */

typedef void RADLINK gdraw_set_3d_transform(F32 *mat); /* mat[3][4] */

IDOC typedef void RADLINK gdraw_render_tile_begin(S32 tx0, S32 ty0, S32 tx1, S32 ty1, S32 pad, GDrawStats *stats);
/* Begins rendering of a sub-region of the rendered image. */

IDOC typedef void RADLINK gdraw_render_tile_end(GDrawStats *stats);
/* Ends rendering of a sub-region of the rendered image. */

IDOC typedef void RADLINK gdraw_rendering_begin(void);
/* Begins rendering; takes control of the graphics API. */

IDOC typedef void RADLINK gdraw_rendering_end(void);
/* Ends rendering; gives up control of the graphics API.  */


////////////////////////////////////////////////////////////
//
// Drawing
//
//idoc(parent,GDrawAPI_Drawing)

IDOC typedef void RADLINK gdraw_clear_stencil_bits(U32 bits);
/* Clears the 'bits' parts of the stencil value in the entire framebuffer to the default value. */

IDOC typedef void RADLINK gdraw_clear_id(void);
/* Clears the 'id' buffer, which is typically the z-buffer but can also be the stencil buffer. */

IDOC typedef void RADLINK gdraw_filter_quad(GDrawRenderState *r, S32 x0, S32 y0, S32 x1, S32 y1, GDrawStats *stats);
/* Draws a special quad in viewport-relative pixel space.

   May be normal, may be displaced by filters, etc. and require multiple passes,
   may apply special blending (and require extra resolves/rendertargets)
   for filter/blend.,

   The x0,y0,x1,y1 always describes the "input" box. */

IDOC typedef struct GDrawPrimitive
{
   F32 *vertices; // Pointer to an array of $gswf_vertex_xy, $gswf_vertex_xyst, or $gswf_vertex_xyoffs
   U16 *indices; // Pointer to an array of 16-bit indices into $(GDrawPrimitive::vertices)

   S32 num_vertices; // Count of elements in $(GDrawPrimitive::vertices)
   S32 num_indices; // Count of elements in $(GDrawPrimitive::indices)

   S32 vertex_format; // One of $gdraw_vformat, specifying the type of element in $(GDrawPrimitive::vertices)

   U32  uniform_count;
   F32 *uniforms;

   U8  drawprim_mode;
} GDrawPrimitive;
/* Specifies the vertex and index data necessary to draw a batch of graphics primitives. */

IDOC typedef void RADLINK gdraw_draw_indexed_triangles(GDrawRenderState *r, GDrawPrimitive *prim, GDrawVertexBuffer *buf, GDrawStats *stats);
/* Draws a collection of indexed triangles, ignoring special filters or blend modes.

   If buf is NULL, then the pointers in 'prim' are machine pointers, and
   you need to make a copy of the data (note currently all triangles
   implementing strokes (wide lines) go this path).
   
   If buf is non-NULL, then use the appropriate vertex buffer, and the
   pointers in prim are actually offsets from the beginning of the
   vertex buffer -- i.e. offset = (char*) prim->whatever - (char*) NULL;
   (note there are separate spaces for vertices and indices; e.g. the
   first mesh in a given vertex buffer will normally have a 0 offset
   for the vertices and a 0 offset for the indices)
*/

IDOC typedef void RADLINK gdraw_set_antialias_texture(S32 width, U8 *rgba);
/* Specifies the 1D texture data to be used for the antialiasing gradients.

   'rgba' specifies the pixel values in rgba byte order. This will only be called
   once during initialization. */

////////////////////////////////////////////////////////////
//
// Texture and Vertex Buffers
//
//idoc(parent,GDrawAPI_Buffers)

IDOC typedef enum gdraw_texture_format
{
   // Platform-independent formats
   GDRAW_TEXTURE_FORMAT_rgba32,  // 32bpp RGBA data in platform-preferred byte order (returned by $gdraw_make_texture_begin as $gdraw_texture_type)
   GDRAW_TEXTURE_FORMAT_font,    // Alpha-only data with at least 4 bits/pixel. Data is submitted as 8 bits/pixel, conversion (if necessary) done by GDraw.

   // First platform-specific format index (for reference)
   GDRAW_TEXTURE_FORMAT__platform = 16,

   // In the future, we will support platform-specific formats and add them to this list.
} gdraw_texture_format;
/* Describes the format of a texture submitted to GDraw. */

IDOC typedef enum gdraw_texture_type
{
   GDRAW_TEXTURE_TYPE_rgba, // Raw 4-channel packed texels, in OpenGL-standard order
   GDRAW_TEXTURE_TYPE_bgra, // Raw 4-channel packed texels, in Direct3D-standard order
   GDRAW_TEXTURE_TYPE_argb, // Raw 4-channel packed texels, in Flash native order

   GDRAW_TEXTURE_TYPE__count,
} gdraw_texture_type;
/* Describes the channel layout of a RGBA texture submitted to GDraw. */

IDOC typedef struct GDraw_MakeTexture_ProcessingInfo
{
   U8   *texture_data; // Pointer to the texture image bits
   S32   num_rows; // Number of rows to upload in the current chunk
   S32   stride_in_bytes;  // Distance between a given pixel and the first pixel in the next row
   S32   texture_type; // One of $gdraw_texture_type

   U32   temp_buffer_bytes; // Size of temp buffer in bytes
   U8   *temp_buffer; // Temp buffer for GDraw to work in (used during mipmap creation)
  
   void *p0,*p1,*p2,*p3,*p4,*p5,*p6,*p7; // Pointers for GDraw to store data across "passes" (never touched by Iggy)
   U32   i0, i1, i2, i3, i4, i5, i6, i7; // Integers for GDraw to store data across "passes" (never touched by Iggy)
} GDraw_MakeTexture_ProcessingInfo;
/* $GDraw_MakeTexture_ProcessingInfo is used when building a texture. */

IDOC typedef struct GDraw_Texture_Description {
   S32   width; // Width of the texture in pixels
   S32   height; // Height of the texture in pixels
   U32   size_in_bytes; // Size of the texture in bytes
} GDraw_Texture_Description;
/* $GDraw_Texture_Description contains information about a texture. */

IDOC typedef U32 gdraw_maketexture_flags;
#define GDRAW_MAKETEXTURE_FLAGS_mipmap             1 IDOC // Generates mip-maps for the texture
#define GDRAW_MAKETEXTURE_FLAGS_updatable          2 IDOC // Set if the texture might be updated subsequent to its initial submission
#define GDRAW_MAKETEXTURE_FLAGS_never_flush        4 IDOC // Set to request that the texture never be flushed from the GDraw cache

/* Flags that control the submission and management of GDraw textures. */

IDOC typedef void RADLINK gdraw_set_texture_unique_id(GDrawTexture *tex, void *old_unique_id, void *new_unique_id);
/* Changes unique id of a texture, only used for TextureSubstitution */

IDOC typedef rrbool RADLINK gdraw_make_texture_begin(void *unique_id,
                                                   S32 width, S32 height, gdraw_texture_format format, gdraw_maketexture_flags flags,
                                                   GDraw_MakeTexture_ProcessingInfo *output_info, GDrawStats *stats);
/* Begins specifying a new texture.

   $:unique_id Unique value specified by Iggy that you can use to identify a reference to the same texture even if its handle has been discarded
   $:return Error code if there was a problem, IGGY_RESULT_OK otherwise
*/

IDOC typedef rrbool RADLINK gdraw_make_texture_more(GDraw_MakeTexture_ProcessingInfo *info);
/* Continues specifying a new texture.

   $:info The same handle initially passed to $gdraw_make_texture_begin
   $:return True if specification can continue, false if specification must be aborted
*/

IDOC typedef GDrawTexture * RADLINK gdraw_make_texture_end(GDraw_MakeTexture_ProcessingInfo *info, GDrawStats *stats);
/* Ends specification of a new texture.
   
   $:info The same handle initially passed to $gdraw_make_texture_begin
   $:return Handle for the newly created texture, or NULL if an error occured
*/

IDOC typedef rrbool RADLINK gdraw_update_texture_begin(GDrawTexture *tex, void *unique_id, GDrawStats *stats);
/* Begins updating a previously submitted texture.

   $:unique_id Must be the same value initially passed to $gdraw_make_texture_begin
   $:return True on success, false otherwise and the texture must be recreated
*/

IDOC typedef void RADLINK gdraw_update_texture_rect(GDrawTexture *tex, void *unique_id, S32 x, S32 y, S32 stride, S32 w, S32 h, U8 *data, gdraw_texture_format format);
/* Updates a rectangle in a previously submitted texture.
   
   $:format Must be the $gdraw_texture_format that was originally passed to $gdraw_make_texture_begin for this texture.
*/

IDOC typedef void RADLINK gdraw_update_texture_end(GDrawTexture *tex, void *unique_id, GDrawStats *stats);
/* Ends an update to a previously submitted texture.

   $:unique_id Must be the same value initially passed to $gdraw_make_texture_begin (and hence $gdraw_update_texture_begin)
*/

IDOC typedef void RADLINK gdraw_describe_texture(GDrawTexture *tex, GDraw_Texture_Description *desc);
/* Returns a texture description for a given GDraw texture. */

IDOC typedef GDrawTexture * RADLINK gdraw_make_texture_from_resource(U8 *resource_file, S32 file_len, void *texture);
/* Loads a texture from a resource file and returns a wrapped pointer. */

IDOC typedef void RADLINK gdraw_free_texture_from_resource(GDrawTexture *tex);
/* Frees a texture created with gdraw_make_texture_from_resource. */


IDOC typedef struct gswf_vertex_xy
{
   F32 x,y; // Position of the vertex
} gswf_vertex_xy;
/* A 2D point with floating-point position. */

IDOC typedef struct gswf_vertex_xyoffs
{
   F32 x,y; // Position of the vertex

   S16 aa; // Stroke/aa texcoord
   S16 dx, dy; // Vector offset from the position, used for anti-aliasing (signed 11.5 fixed point)
   S16 unused;
} gswf_vertex_xyoffs;
/* A 2D point with floating-point position, additional integer parameter, and integer anti-aliasing offset vector. */

IDOC typedef struct gswf_vertex_xyst
{
   F32 x,y; // Position of the vertex
   F32 s,t; // Explicit texture coordinates for rectangles
} gswf_vertex_xyst;
/* A 2D point with floating-point position and texture coordinates. */

typedef int gdraw_verify_size_xy    [sizeof(gswf_vertex_xy    ) ==  8 ? 1 : -1];
typedef int gdraw_verify_size_xyoffs[sizeof(gswf_vertex_xyoffs) == 16 ? 1 : -1];
typedef int gdraw_verify_size_xyst  [sizeof(gswf_vertex_xyst  ) == 16 ? 1 : -1];
  
IDOC typedef enum gdraw_vformat
{
   GDRAW_vformat_v2,    // Indicates vertices of type $gswf_vertex_xy (8 bytes per vertex)
   GDRAW_vformat_v2aa,  // Indicates vertices of type $gswf_vertex_xyoffs (16 bytes per vertex)
   GDRAW_vformat_v2tc2, // Indicates vertices of type $gswf_vertex_xyst (16 bytes per vertex)

   GDRAW_vformat__basic_count,
   GDRAW_vformat_ihud1 = GDRAW_vformat__basic_count, // primary format for ihud, currently v2tc2mat4 (20 bytes per vertex)

   GDRAW_vformat__count,
   GDRAW_vformat_mixed, // Special value that denotes a VB containing data in multiple vertex formats. Never used when drawing!
} gdraw_vformat;
/* Identifies one of the vertex data types. */

IDOC typedef struct GDraw_MakeVertexBuffer_ProcessingInfo
{
   U8  *vertex_data;         // location to write vertex data
   U8  *index_data;          // location to write index data

   S32  vertex_data_length;  // size of buffer to write vertex data
   S32  index_data_length;   // size of buffer to write index data

   void *p0,*p1,*p2,*p3,*p4,*p5,*p6,*p7; // Pointers for GDraw to store data across "passes" (never touched by Iggy)
   U32   i0, i1, i2, i3, i4, i5, i6, i7; // Integers for GDraw to store data across "passes" (never touched by Iggy)
} GDraw_MakeVertexBuffer_ProcessingInfo;
/* $GDraw_MakeVertexBuffer_ProcessingInfo is used when building a vertex buffer. */

IDOC typedef struct GDraw_VertexBuffer_Description {
   S32  size_in_bytes; // Size of the vertex buffer in bytes
} GDraw_VertexBuffer_Description;
/* $GDraw_VertexBuffer_Description contains information about a vertex buffer. */

IDOC typedef rrbool RADLINK gdraw_make_vertex_buffer_begin(void *unique_id, gdraw_vformat vformat, S32 vdata_len_in_bytes, S32 idata_len_in_bytes, GDraw_MakeVertexBuffer_ProcessingInfo *info, GDrawStats *stats);
/* Begins specifying a new vertex buffer.

   $:unique_id Unique value that identifies this texture, across potentially multiple flushes and re-creations of its $GDrawTexture handle in GDraw
   $:vformat One of $gdraw_vformat, denoting the format of the vertex data submitted
   $:return false if there was a problem, true if ok
*/

IDOC typedef rrbool RADLINK gdraw_make_vertex_buffer_more(GDraw_MakeVertexBuffer_ProcessingInfo *info);
/* Continues specifying a new vertex buffer.

   $:info The same handle initially passed to $gdraw_make_vertex_buffer_begin
   $:return True if specification can continue, false if specification must be aborted
*/   

IDOC typedef GDrawVertexBuffer * RADLINK gdraw_make_vertex_buffer_end(GDraw_MakeVertexBuffer_ProcessingInfo *info, GDrawStats *stats);
/* Ends specification of a new vertex buffer.

   $:info The same handle initially passed to $gdraw_make_texture_begin
   $:return Handle for the newly created vertex buffer
*/

IDOC typedef void RADLINK gdraw_describe_vertex_buffer(GDrawVertexBuffer *buffer, GDraw_VertexBuffer_Description *desc);
/* Returns a description for a given GDrawVertexBuffer */


IDOC typedef rrbool RADLINK gdraw_try_to_lock_texture(GDrawTexture *tex, void *unique_id, GDrawStats *stats);
/* Tells GDraw that a $GDrawTexture is going to be referenced.

   $:unique_id Must be the same value initially passed to $gdraw_make_texture_begin
*/

IDOC typedef rrbool RADLINK gdraw_try_to_lock_vertex_buffer(GDrawVertexBuffer *vb, void *unique_id, GDrawStats *stats);
/* Tells GDraw that a $GDrawVertexBuffer is going to be referenced.

   $:unique_id Must be the same value initially passed to $gdraw_make_vertex_buffer_begin
*/

IDOC typedef void RADLINK gdraw_unlock_handles(GDrawStats *stats);
/* Indicates that the user of GDraw will not try to reference anything without locking it again.

   Note that although a call to $gdraw_unlock_handles indicates that
   all $GDrawTexture and $GDrawVertexBuffer handles that have had a
   "unique_id" specified will no longer be referenced by the user of
   GDraw, it does not affect those $GDrawTexture handles that were
   created by $gdraw_start_texture_draw_buffer with a unique_id of 0.
*/

IDOC typedef void RADLINK gdraw_free_vertex_buffer(GDrawVertexBuffer *vb, void *unique_id, GDrawStats *stats);
/* Free a vertex buffer and invalidate the handle

   $:unique_id Must be the same value initially passed to $gdraw_make_vertex_buffer_begin
*/

IDOC typedef void RADLINK gdraw_free_texture(GDrawTexture *t, void *unique_id, GDrawStats *stats);
/* Free a texture and invalidate the handle.

   $:unique_id Must be the same value initially passed to $gdraw_make_texture_begin, or 0 for a texture created by $gdraw_end_texture_draw_buffer
*/

////////////////////////////////////////////////////////////
//
// Render targets
//
//idoc(parent,GDrawAPI_Targets)

IDOC typedef U32 gdraw_texturedrawbuffer_flags;
#define GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_color    1 IDOC // Tells GDraw that you will need the color channel when rendering a texture
#define GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_alpha    2 IDOC // Tells GDraw that you will need the alpha channel when rendering a texture
#define GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_stencil  4 IDOC // Tells GDraw that you will need the stencil channel when rendering a texture
#define GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_id       8 IDOC // Tells GDraw that you will need the id channel when rendering a texture

/* Flags that control rendering to a texture. */

IDOC typedef rrbool RADLINK gdraw_texture_draw_buffer_begin(gswf_recti *region, gdraw_texture_format format, gdraw_texturedrawbuffer_flags flags, void *unique_id, GDrawStats *stats);
/* Starts rendering all GDraw commands to a new texture.

   Creates a rendertarget with destination alpha, initializes to all 0s and prepares to render into it
*/


IDOC typedef GDrawTexture * RADLINK gdraw_texture_draw_buffer_end(GDrawStats *stats);
/* Ends rendering GDraw commands to a texture, and returns the texture created.

   You can get the size of the resulting texture with $gdraw_query_texture_size.
*/

////////////////////////////////////////////////////////////
//
// Masking
//
//idoc(parent,GDrawAPI_Masking)

IDOC typedef void RADLINK gdraw_draw_mask_begin(gswf_recti *region, S32 mask_bit, GDrawStats *stats);
/* Start a masking operation on the given region for the specified mask bit.

   For most drivers, no special preparation is necessary to start masking, so this is a no-op.
*/

IDOC typedef void RADLINK gdraw_draw_mask_end(gswf_recti *region, S32 mask_bit, GDrawStats *stats);
/* End a masking operation on the given region for the specified mask bit.

   For most drivers, no special preparation is necessary to end masking, so this is a no-op.
*/

////////////////////////////////////////////////////////////
//
// GDraw API Function table
//
//idoc(parent,GDrawAPI_Base)

IDOC struct GDrawFunctions
{
    // queries
    gdraw_get_info *GetInfo;

    // drawing state
    gdraw_set_view_size_and_world_scale   * SetViewSizeAndWorldScale;
    gdraw_render_tile_begin               * RenderTileBegin;
    gdraw_render_tile_end                 * RenderTileEnd;
    gdraw_set_antialias_texture           * SetAntialiasTexture;

    // drawing
    gdraw_clear_stencil_bits              * ClearStencilBits;
    gdraw_clear_id                        * ClearID;
    gdraw_filter_quad                     * FilterQuad;
    gdraw_draw_indexed_triangles          * DrawIndexedTriangles;
    gdraw_make_texture_begin              * MakeTextureBegin;
    gdraw_make_texture_more               * MakeTextureMore;
    gdraw_make_texture_end                * MakeTextureEnd;
    gdraw_make_vertex_buffer_begin        * MakeVertexBufferBegin;
    gdraw_make_vertex_buffer_more         * MakeVertexBufferMore;
    gdraw_make_vertex_buffer_end          * MakeVertexBufferEnd;
    gdraw_try_to_lock_texture             * TryToLockTexture;
    gdraw_try_to_lock_vertex_buffer       * TryToLockVertexBuffer;
    gdraw_unlock_handles                  * UnlockHandles;
    gdraw_free_texture                    * FreeTexture;
    gdraw_free_vertex_buffer              * FreeVertexBuffer;
    gdraw_update_texture_begin            * UpdateTextureBegin;
    gdraw_update_texture_rect             * UpdateTextureRect;
    gdraw_update_texture_end              * UpdateTextureEnd;

    // rendertargets
    gdraw_texture_draw_buffer_begin       * TextureDrawBufferBegin;
    gdraw_texture_draw_buffer_end         * TextureDrawBufferEnd;

    gdraw_describe_texture                * DescribeTexture;
    gdraw_describe_vertex_buffer          * DescribeVertexBuffer;

    // new functions are always added at the end, so these have no structure
    gdraw_set_texture_unique_id           * SetTextureUniqueID;

    gdraw_draw_mask_begin                 * DrawMaskBegin;
    gdraw_draw_mask_end                   * DrawMaskEnd;

    gdraw_rendering_begin                 * RenderingBegin;
    gdraw_rendering_end                   * RenderingEnd;

    gdraw_make_texture_from_resource      * MakeTextureFromResource;
    gdraw_free_texture_from_resource      * FreeTextureFromResource;

    gdraw_set_3d_transform                * Set3DTransform;
};
/* The function interface called by Iggy to render graphics on all
   platforms.

   So that Iggy can integrate with the widest possible variety of
   rendering scenarios, all of its renderer-specific drawing calls
   go through this table of function pointers.  This allows you
   to dynamically configure which of RAD's supplied drawing layers
   you wish to use, or to integrate it directly into your own
   renderer by implementing your own versions of the drawing
   functions Iggy requires.
*/

RADDEFEND

#endif
