#include "stdafx.h"
#include "Orbis_UIController.h"

// Temp
#include "../Minecraft.h"
#include "../Textures.h"

#define _ENABLEIGGY

ConsoleUIController ui;

void ConsoleUIController::init(S32 w, S32 h)
{
#ifdef _ENABLEIGGY

	// Shared init
	preInit(w,h);

	/* Now that Iggy is ready, we need to create a set of draw
	callbacks for it to use to draw things on the target platform.
	We're using the GNM GDraw implementation that's included with
	the Iggy SDK. To get it set up, we need to provide it with
	memory for the different resource type. GDraw needs memory
	for render targets (color buffers), textures, and vertex buffers.
	There's also command buffers and the staging buffer; we'll get to
	them later.

	We recommend that all resource pools be allocated in Garlic memory
	for performance reasons.

	Note that render target memory is really only used while Iggy is
	rendering; you can share the memory space with other resources that
	aren't in use while Iggy is rendering - for example, post-processing
	render targets or shadow maps.

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
	S32 rt_mem_size = 32*1024*1024; // Render target space
	S32 tex_mem_size = 30*1024*1024; // Texture pool
	S32 vert_mem_size = 2*1024*1024; // Vertex buffer pool

	S32 tex_handles = 200;
	S32 vert_handles = 1000;

	/* SetResourceMemory is the call that actually sets up the location and size
	of a resource pool.  */
	gdraw_orbis_SetResourceMemory(GDRAW_ORBIS_RESOURCE_rendertarget, // Pool type
		0, // Number of handles (unused for render targets)
		RenderManager.MemoryAllocateGPUMem(GDRAW_ORBIS_TEXTURE_ALIGNMENT,rt_mem_size), // Pointer
		rt_mem_size); // Size
	gdraw_orbis_SetResourceMemory(GDRAW_ORBIS_RESOURCE_texture, tex_handles,
		RenderManager.MemoryAllocateGPUMem(GDRAW_ORBIS_TEXTURE_ALIGNMENT,tex_mem_size), tex_mem_size);
	gdraw_orbis_SetResourceMemory(GDRAW_ORBIS_RESOURCE_vertexbuffer, vert_handles,
		RenderManager.MemoryAllocateGPUMem(GDRAW_ORBIS_VERTEXBUFFER_ALIGNMENT,vert_mem_size), vert_mem_size);

	/* There's one more resource-related buffer we need to allocate: the staging
	buffer. The staging buffer is used as a staging area for texture and vertex
	buffer uploads. GDraw doesn't write such data directly to resource memory
	from the CPU because doing so would require us to synchronize GPU and CPU such
	that the CPU never accesses memory that the GPU might currently be using. This
	wastes both CPU and GPU time, and makes it very hard to record command buffers
	(GfxContexts) early in the frame and then kick them off later.

	Instead, GDraw writes all data to the staging buffer, and lets the GPU copy it
	from the there to its final location. While this costs more memory, it avoids
	all need for fine-grained CPU/GPU synchronization and makes it easy to, say,
	have Iggy rendering happen on a second thread.

	The staging buffer is also where per-frame dynamic vertex data and dynamic
	textures (which in the case of Iggy means animated gradients) get allocated
	from. The right way to think of the staging buffer is just as an extra block
	of memory associated with the Iggy command buffer: if you always use the same
	command buffer for Iggy rendering, you only need one staging buffer, if you
	double-buffer commands, you need to use the same scheme for the staging buffer,
	and so forth. Since this example uses only one GfxContext, we also use a single
	staging buffer.

	It's not immediately obvious how big the staging buffer should be; this example
	sizes it so it's big enough to replace all resource pools every frame. This keeps
	things simple but is excessive in practice. $gdraw_orbis_End can report back how
	much of the staging buffer was used every frame, so you can use this as a guideline
	to size it appropriately. If the staging buffer is too small, Iggy will issue a
	warning, and some textures or shapes might pop in a frame late (because they
	couldn't be uploaded in time).
	*/
	staging_buf_size = 12 * 1024 * 1024; // This value determined by experimentation - was (with original comment):  tex_mem_size + vert_mem_size; // This is very generous!
	staging_buf[0] = RenderManager.MemoryAllocateCPUMem(Gnm::kAlignmentOfBufferInBytes,staging_buf_size);
	staging_buf[1] = RenderManager.MemoryAllocateCPUMem(Gnm::kAlignmentOfBufferInBytes,staging_buf_size);
	currentStagingBuf = 0;

	/* Whew! Now that the GDraw memory configuration is set up, we need to initialize
	GDraw. But first, we need to allocate yet another block of video memory for
	GDraw to store its shaders etc. in. Unlike the rest of the memory configuration,
	the work area is reserved to GDraw and can't be freed or relocated afterwards
	without shutting GDraw and all attached Iggys down completely. But it's fairly
	small (64k as of this writing) so this shouldn't be much of a problem. It should
	be allocated in Garlic memory. */
	void *gdraw_context_mem = RenderManager.MemoryAllocateGPUMem(1,GDRAW_ORBIS_CONTEXT_MEM_SIZE);

	/* Finally, we can actually create the GDraw GNM driver and pass it to Iggy. */
	gdraw_funcs = gdraw_orbis_CreateContext(
		w,
		h,
		gdraw_context_mem);
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
	/* Tell GDraw which command buffer and staging buffer to use
	for rendering. GDraw can either render to its own command
	buffer or resume writing to a GfxContext you're already
	using. In this example, we render only Iggy content, so
	we have the command buffer to ourselves. Between this
	call and $gdraw_orbis_End, you are not allowed to write
	to the command buffer ("gfxc" in this case). */
	gdraw_orbis_Begin(RenderManager.GetCurrentBackBufferContext(), staging_buf[currentStagingBuf], staging_buf_size);
	currentStagingBuf ^= 1;

	/* We need to tell GDraw which surface to render to.
	This tutorial just uses our main color and depth buffers,
	but for in-game UI usage, you might want to use another
	render target (like a texture) instead. */
	gdraw_orbis_SetTileOrigin(RenderManager.GetCurrentBackBufferTarget(), RenderManager.GetCurrentBackBufferDepthTarget(), 0, 0);

	renderScenes();

	/* That's it for this frame, so tell GDraw we're done rendering.
	This function can optionally return $gdraw_orbis_staging_stats
	to track staging buffer usage, which is useful information to
	size the buffer appropriately. In this example though, we
	don't bother.

	After this call, you may resume writing to the command buffer
	passed to $gdraw_orbis_Begin ("gfxc" in this example). */

#if 0
	gdraw_orbis_staging_stats stagingStats;
	gdraw_orbis_End(&stagingStats);

	static int throttle = 0;
	static int maxAttempted = 0;
	static int maxSucceeded = 0;

	if( stagingStats.bytes_attempted > maxAttempted ) maxAttempted = stagingStats.bytes_attempted;
	if( stagingStats.bytes_succeeded > maxSucceeded ) maxSucceeded = stagingStats.bytes_succeeded;

	if( ( throttle & 400 ) == 0 )
	{
		app.DebugPrintf("\nIGGY END DRAW\n");
		app.DebugPrintf("   - allocs_attempted = %d\n", stagingStats.allocs_attempted);         // number of allocations attempted from the staging buffer
		app.DebugPrintf("   - allocs_succeeded = %d\n", stagingStats.allocs_succeeded);// number of allocations that succeeded
		app.DebugPrintf("   - bytes_attempted = %d\n", stagingStats.bytes_attempted);// number of bytes attempted to allocate
		app.DebugPrintf("   - bytes_succeeded = %d\n", stagingStats.bytes_succeeded);// number of bytes successfully allocated
		app.DebugPrintf("   -  max bytes_attempted = %d\n", maxAttempted);
		app.DebugPrintf("   -  max bytes_succeeded = %d\n", maxSucceeded);
		app.DebugPrintf("   - largest_bytes_attempted = %d\n", stagingStats.largest_bytes_attempted);// number of bytes in largest attempted alloc
		app.DebugPrintf("   - largest_bytes_succeeded = %d\n", stagingStats.largest_bytes_succeeded);// number of bytes in lagrest successful allo
		app.DebugPrintf("\n\n");
	}
	throttle++;

#else
	gdraw_orbis_End(NULL);
#endif

#endif
}

void ConsoleUIController::beginIggyCustomDraw4J(IggyCustomDrawCallbackRegion *region, CustomDrawData *customDrawRegion)
{
	PIXBeginNamedEvent(0,"Starting Iggy custom draw\n");

	PIXBeginNamedEvent(0,"Gdraw setup");
	// get the correct object-to-world matrix from GDraw, and set the render state to a normal state
	gdraw_orbis_BeginCustomDraw(region, customDrawRegion->mat);
	PIXEndNamedEvent();
}

CustomDrawData *ConsoleUIController::setupCustomDraw(UIScene *scene, IggyCustomDrawCallbackRegion *region)
{
	CustomDrawData *customDrawRegion = new CustomDrawData();
	customDrawRegion->x0 = region->x0;
	customDrawRegion->x1 = region->x1;
	customDrawRegion->y0 = region->y0;
	customDrawRegion->y1 = region->y1;

	// get the correct object-to-world matrix from GDraw, and set the render state to a normal state   
	gdraw_orbis_BeginCustomDraw(region, customDrawRegion->mat);

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

	gdraw_orbis_CalculateCustomDraw_4J(region, customDrawRegion->mat);

	return customDrawRegion;
}

void ConsoleUIController::endCustomDraw(IggyCustomDrawCallbackRegion *region)
{
	endCustomDrawGameStateAndMatrices();

	gdraw_orbis_EndCustomDraw(region);
}

void ConsoleUIController::setTileOrigin(S32 xPos, S32 yPos)
{
	gdraw_orbis_SetTileOrigin(RenderManager.GetCurrentBackBufferTarget(), RenderManager.GetCurrentBackBufferDepthTarget(), xPos, yPos);
}

GDrawTexture *ConsoleUIController::getSubstitutionTexture(int textureId)
{
	/* Create a wrapped texture from a shader resource view.
	A wrapped texture can be used to let Iggy draw using the contents of a texture
	you create and manage on your own. For example, you might render to this texture,
	or stream video into it. Wrapped textures take up a handle. They will never be
	freed or otherwise modified by GDraw; nor will GDraw change any reference counts.
	All this is up to the application. */

	sce::Gnm::Texture *tex = RenderManager.TextureGetTexture(textureId);
	GDrawTexture *gdrawTex = gdraw_orbis_WrappedTextureCreate(tex);
	return gdrawTex;

	return NULL;
}

void ConsoleUIController::destroySubstitutionTexture(void *destroyCallBackData, GDrawTexture *handle)
{
	/* Destroys the GDraw wrapper for a wrapped texture object. This will free up
	a GDraw texture handle but not release the associated D3D texture; that is
	up to you. */
	gdraw_orbis_WrappedTextureDestroy(handle);
}

void ConsoleUIController::shutdown()
{
#ifdef _ENABLEIGGY
	/* Destroy the GDraw context. This frees all resources, shaders etc.
	allocated by GDraw. Note this is only safe to call after all
	active Iggy player have been destroyed! */
	gdraw_orbis_DestroyContext();
#endif
}


void ConsoleUIController::handleUnlockFullVersionCallback()
{
	for(unsigned int i = 0; i < eUIGroup_COUNT; ++i)
	{
		ui.m_groups[i]->handleUnlockFullVersion();
	}
}

