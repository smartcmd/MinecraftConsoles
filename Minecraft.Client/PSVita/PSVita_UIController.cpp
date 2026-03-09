#include "stdafx.h"
#include "PSVita_UIController.h"

// Temp
#include "..\Minecraft.h"
#include "..\Textures.h"

#define _ENABLEIGGY

ConsoleUIController ui;

void ConsoleUIController::init(S32 w, S32 h)
{
#ifdef _ENABLEIGGY

	// Shared init
	preInit(w,h);

	/* Now that Iggy is ready, we need to create a set of draw
	callbacks for it to use to draw things on the target platform.
	We're using the GXM GDraw implementation that's included with
	the Iggy SDK. To get it set up, we need to provide it with
	memory for the different resource type. GDraw needs memory
	for textures and vertex buffers. There's also the dynamic buffer;
	we'll get to that in a bit.

	We recommend that all resource pools be allocated in uncached memory for
	performance reasons. They also *NEED* to be in GPU-writeable memory;
	passing in memory that is only GPU-readable will cause a fault!
	Typically, it is not necessary to allocate textures in CDRAM; LPDDR will
	do (Iggy UI rendering doesn't need a lot of memory bandwidth for
	textures).

	The texture and vertex buffer pools contain multiple textures and
	vertex buffers. Each object requires a bit of main memory for management
	overhead. We need to specify how many object handles can be in use at
	the same time in any given pool. If Iggy runs out of GDraw handles, it
	won't crash, but instead throw out old cache data in a LRU fashion.
	If the number of handles is too small, this will happen frequently, and
	textures will need to decoded multiple times (or vertex buffers re-generated
	multiple times), which causes a performance hit. */

	// Size of the various memory pools. All these values are intentionally
	// picked to be way larger than necessary for typical Flash files.
	S32 tex_mem_size = 48*1024*1024; // Texture pool
	S32 vert_mem_size = 4*1024*1024; // Vertex buffer pool

	S32 tex_handles = 300;
	S32 vert_handles = 1000;

	/* SetResourceMemory is the call that actually sets up the location and size
	of a resource pool.  */
	gdraw_psp2_SetResourceMemory(GDRAW_PSP2_RESOURCE_texture, tex_handles,
		RenderManager.allocGPUCDMem(tex_mem_size, GDRAW_PSP2_TEXTURE_ALIGNMENT), tex_mem_size);
	gdraw_psp2_SetResourceMemory(GDRAW_PSP2_RESOURCE_vertexbuffer, vert_handles,
		RenderManager.allocGPURWMem(vert_mem_size, GDRAW_PSP2_VERTEXBUFFER_ALIGNMENT), vert_mem_size);

	/* There's one more resource-related buffer we need to allocate: the dynamic
	buffer. This is where GDraw writes dynamic vertex data and uniform buffers.

	The dynamic buffer should be allocated in uncached memory that is mapped
	GPU-readable. GPU writes are not required for this memory.

	In theory you can use only one dynamic buffer, but if you did that, GDraw
	would have to wait for the GPU to finish rendering the previous frame
	before it could start CPU work on the next. This is clearly not a good idea
	from a performance standpoint.

	Hence, we recommend that you use two or three dynamic buffers in a round-robin
	fashion. This example uses as many dynamic buffers as it does back buffers
	(display queue entries), which is a good general rule of thumb.
	*/
	S32 dynamic_buf_size = 1*1024*1024; // Again, very generous!
	m_currentBackBuffer = 0;
	m_dynamicBuffer = new gdraw_psp2_dynamic_buffer[RenderManager.getDisplayBufferCount()];
	for (int i=0; i < RenderManager.getDisplayBufferCount(); i++)
		gdraw_psp2_InitDynamicBuffer(&m_dynamicBuffer[i], RenderManager.allocGPUROMem(dynamic_buf_size, 4), dynamic_buf_size);

	/* Whew! Now that the GDraw memory configuration is set up, we need to initialize
	GDraw. But first, we need to allocate yet another block of video memory for
	GDraw to store some global data in - a few buffers that are shared between
	all Iggys. Unlike the rest of the memory configuration, the work area is reserved
	to GDraw and can't be freed or relocated afterwards without shutting GDraw and all
	attached Iggys down completely. But it's fairly small (16k as of this writing) so
	this shouldn't be much of a problem. It should be allocted in uncached,
	GPU-readable memory. */
	void *gdraw_context_mem = RenderManager.allocGPUROMem(GDRAW_PSP2_CONTEXT_MEM_SIZE, 1);

	/* GDraw also needs a single GPU notification label for its synchronization work.
	Since there's no other rendering in this example, we just use the first label. */
	volatile uint32_t *notify_region = RenderManager.getNotificationRegion(); //sceGxmGetNotificationRegion();

	/* Finally, we can actually create the GDraw GXM driver and pass it to Iggy. */
	gdraw_funcs = gdraw_psp2_CreateContext(
		RenderManager.getGXMShaderPatcher(),
		gdraw_context_mem,
		notify_region + 0,
		SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4);
	IggySetGDraw(gdraw_funcs);

	/* Flash content can have audio embedded.  We'd like to be able
	to play back any audio there is in the Flash that we load,
	but in this tutorial we don't care about processing the sound
	ourselves.  So we call $IggyAudioUseDefault to tell Iggy
	to go ahead and use the default sound driver. */
	//IggyAudioUseDefault();

	// Shared init
	postInit();
#endif
}

void ConsoleUIController::render()
{
#ifdef _ENABLEIGGY
	/* Tell GDraw that we're about to start Iggy rendering.
	There may only be one pair of $gdraw_psp2_Begin / $gdraw_psp2_End
	per scene. Pass the same color surface and depth/stencil surface
	you passed to sceGxmBeginScene.

	You can do other rendering before you call $gdraw_psp2_Begin.
	*/

	gdraw_psp2_Begin(RenderManager.getGXMContext(), RenderManager.getCurrentDisplaySurface(), RenderManager.getCurrentDepthSurface(), &m_dynamicBuffer[m_currentBackBuffer]);
	++m_currentBackBuffer;
	m_currentBackBuffer %= RenderManager.getDisplayBufferCount();

	/* We need to tell GDraw which surface to render to.
	This tutorial just uses our main color and depth buffers,
	but for in-game UI usage, you might want to use another
	render target (like a texture) instead. */
	gdraw_psp2_SetTileOrigin(0, 0);

	renderScenes();

	/* That's it with the Iggy rendering for this scene, so tell
	GDraw we're done rendering. $gdraw_psp2_End returns a notification
	that you have to pass as fragment notification to sceGxmEndScene.
	This is *crucial* or rendering artifacts will occur in certain
	cases!

	After this call, you may resume other (non-Iggy) rendering for this
	scene. */
	SceGxmNotification notify = gdraw_psp2_End();

	/* End the GXM scene. Note that we pass "notify" (returned from
	$gdraw_psp2_End) as the fragment notification. */
	//sceGxmEndScene(gxm, nullptr, &notify);
	RenderManager.setFragmentNotification(notify);
#endif
}

void ConsoleUIController::beginIggyCustomDraw4J(IggyCustomDrawCallbackRegion *region, CustomDrawData *customDrawRegion)
{
	// get the correct object-to-world matrix from GDraw, and set the render state to a normal state
	gdraw_psp2_BeginCustomDraw(region, customDrawRegion->mat);
}

CustomDrawData *ConsoleUIController::setupCustomDraw(UIScene *scene, IggyCustomDrawCallbackRegion *region)
{
	CustomDrawData *customDrawRegion = new CustomDrawData();
	customDrawRegion->x0 = region->x0;
	customDrawRegion->x1 = region->x1;
	customDrawRegion->y0 = region->y0;
	customDrawRegion->y1 = region->y1;

	// get the correct object-to-world matrix from GDraw, and set the render state to a normal state   
	gdraw_psp2_BeginCustomDraw(region, customDrawRegion->mat);

	setupCustomDrawGameStateAndMatrices(scene, customDrawRegion);

	return customDrawRegion;
}

CustomDrawData *ConsoleUIController::calculateCustomDraw(IggyCustomDrawCallbackRegion *region)
{
	CustomDrawData *customDrawRegion = new CustomDrawData();
	customDrawRegion->x0 = region->x0;
	customDrawRegion->x1 = region->x1;
	customDrawRegion->y0 = region->y0;
	customDrawRegion->y1 = region->y1;

	gdraw_psp2_CalculateCustomDraw_4J(region, customDrawRegion->mat);

	return customDrawRegion;
}

void ConsoleUIController::endCustomDraw(IggyCustomDrawCallbackRegion *region)
{
	endCustomDrawGameStateAndMatrices();

	gdraw_psp2_EndCustomDraw(region);
}

void ConsoleUIController::setTileOrigin(S32 xPos, S32 yPos)
{
	gdraw_psp2_SetTileOrigin(xPos, yPos);
}

GDrawTexture *ConsoleUIController::getSubstitutionTexture(int textureId)
{
	/* Create a wrapped texture from a shader resource view.
	A wrapped texture can be used to let Iggy draw using the contents of a texture
	you create and manage on your own. For example, you might render to this texture,
	or stream video into it. Wrapped textures take up a handle. They will never be
	freed or otherwise modified by GDraw; nor will GDraw change any reference counts.
	All this is up to the application. */

	SceGxmTexture *tex = RenderManager.TextureGetTexture(textureId);
	GDrawTexture *gdrawTex = gdraw_psp2_WrappedTextureCreate(tex);
	return gdrawTex;
}

void ConsoleUIController::destroySubstitutionTexture(void *destroyCallBackData, GDrawTexture *handle)
{
	/* Destroys the GDraw wrapper for a wrapped texture object. This will free up
	a GDraw texture handle but not release the associated D3D texture; that is
	up to you. */
	gdraw_psp2_WrappedTextureDestroy(handle);
}

void ConsoleUIController::shutdown()
{
#ifdef _ENABLEIGGY
	/* Destroy the GDraw context. This frees all resources, shaders etc.
	allocated by GDraw. Note this is only safe to call after all
	active Iggy player have been destroyed! */
	gdraw_psp2_DestroyContext();
#endif
}


void ConsoleUIController::handleUnlockFullVersionCallback()
{
	for(unsigned int i = 0; i < eUIGroup_COUNT; ++i)
	{
		ui.m_groups[i]->handleUnlockFullVersion();
	}
}

