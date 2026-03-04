#include "stdafx.h"
#include "PS3_UIController.h"
#include "Common/UI/UIController.h"
#include <cell/gcm.h>

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
      callbacks for it to use to draw things with the platform
      hardware (in this case, via GCM).  To do this, we're building
      this tutorial with the GCM GDraw implementaton that's included
      with the Iggy SDK. To get it set up, we first need to set up
      its memory configuration, then we create a context, and finally
      we pass that context to Iggy so it will use it to render Flash
      content.
      First, we need to set up the GDraw memory configuration. GDraw
      needs memory areas for various types of resources (such as textures,
      and vertex buffers). Some of them, such as render targets, need to be
      allocated in special types of memory to get optimal performance. In this
      example, we allocate everything except for the dynamic vertex buffer
      memory in local video memory. 
      Rendertargets deserve a special mention: They need to be allocated in a
      tile region for optimum performance, which places some requirements on
      their alignment and size. The size you specify here must be big enough to
      cover the largest rendertarget you'll ever need for one Iggy rendering
      call. In our case, we render the whole screen at once, so that's the
      size we pass. */

//    // Set up rendertarget memory.
    S32 rt_mem_size = 16*1024*1024; // 16MB of render target memory.
    void *rt_mem = RenderManager.allocVRAM(rt_mem_size, CELL_GCM_TILE_ALIGN_OFFSET);
    S32 rt_pitch = cellGcmGetTiledPitchSize(w * 4);
    gdraw_GCM_SetRendertargetMemory(rt_mem, rt_mem_size, w, h, rt_pitch);

   // Also configure a tile region for it.
	RenderManager.setTileInfo(rt_mem, rt_pitch, CELL_GCM_COMPMODE_C32_2X1);

   // Texture and vertex buffer memory work in a similar fashion.
   S32 tex_mem_size = 24*1024*1024;
   S32 vert_mem_size = 2*1024*1024;

   gdraw_GCM_SetResourceMemory(GDRAW_GCM_RESOURCE_texture, 200, RenderManager.allocVRAM(tex_mem_size, 128), tex_mem_size);
   gdraw_GCM_SetResourceMemory(GDRAW_GCM_RESOURCE_vertexbuffer, 1000, RenderManager.allocVRAM(vert_mem_size, 128), vert_mem_size);

   // In this example, we allocate dynamic vertex buffer memory from main memory,
   // which is generally faster.
   S32 dyn_vert_size = 128*1024;
   gdraw_GCM_SetResourceMemory(GDRAW_GCM_RESOURCE_dyn_vertexbuffer, 0, RenderManager.allocIOMem(dyn_vert_size, 128), dyn_vert_size);

// Whew! Now that the GDraw memory configuration is set up, we need to initialize
// GDraw. But first, we need to allocate yet another block of video memory for
// GDraw to store its shaders etc. in (this is the last alloc!). Unlike the
// rest of the memory configuration, the work area is reserved to GDraw and can't
// be freed or relocated afterwards without shutting GDraw and all attached Iggys
// down completely. But it's fairly small (64k as of this writing) so this shouldn't
// be much of a problem. 
   void *gdraw_work_mem = RenderManager.allocVRAM(GDRAW_GCM_LOCAL_WORKMEM_SIZE, 128);

   /* Finally, we can actually create the GDraw GCM driver and pass it to Iggy. */
   gdraw_funcs = gdraw_GCM_CreateContext(
      gCellGcmCurrentContext, // The default GCM context provided by libgcm.
      gdraw_work_mem,         // The work area in local memory
      64);                    // RSX Label index to use for fences
   IggySetGDraw(gdraw_funcs);

	// Initialize audio
	// TODO: 4J Stu - Currently Iggy crashes if I have audio enabled. Disabling for now.
	//IggyAudioUseDefault();

	// Shared init
	postInit();
#endif
}

void ConsoleUIController::render()
{
#ifdef _ENABLEIGGY
// We need to tell GDraw which surface to render to.
// This tutorial just uses the backbuffer, but for in-game
// UI usage, you might want to use another rendertarget (like
// a texture) instead. 
	gdraw_GCM_SetTileOrigin(RenderManager.getCurrentBackBufferSurface(), 0, 0);
 	renderScenes();

// Finally we're ready to display the frame.  First we
// call GDraw to let it know we're done rendering, so
// it can do any finalization it needs to do.  Then we
// initiate the GCM buffer-flip procedure. 
      gdraw_GCM_NoMoreGDrawThisFrame();
#endif
}

CustomDrawData *ConsoleUIController::setupCustomDraw(UIScene *scene, IggyCustomDrawCallbackRegion *region)
{
	CustomDrawData *customDrawRegion = new CustomDrawData();
	customDrawRegion->x0 = region->x0;
	customDrawRegion->x1 = region->x1;
	customDrawRegion->y0 = region->y0;
	customDrawRegion->y1 = region->y1;

	// get the correct object-to-world matrix from GDraw, and set the render state to a normal state   
	gdraw_GCM_BeginCustomDraw(region, customDrawRegion->mat);

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

	gdraw_GCM_CalculateCustomDraw_4J(region, customDrawRegion->mat);

	return customDrawRegion;
}

void ConsoleUIController::endCustomDraw(IggyCustomDrawCallbackRegion *region)
{
	endCustomDrawGameStateAndMatrices();

	gdraw_GCM_EndCustomDraw(region);
}

void ConsoleUIController::setTileOrigin(S32 xPos, S32 yPos)
{
	gdraw_GCM_SetTileOrigin(RenderManager.getCurrentBackBufferSurface(), xPos, yPos);
}

GDrawTexture *ConsoleUIController::getSubstitutionTexture(int textureId)
{
	/* Create a wrapped texture from a shader resource view.
	A wrapped texture can be used to let Iggy draw using the contents of a texture
	you create and manage on your own. For example, you might render to this texture,
	or stream video into it. Wrapped textures take up a handle. They will never be
	freed or otherwise modified by GDraw; nor will GDraw change any reference counts.
	All this is up to the application. */
	CellGcmTexture *tex = RenderManager.TextureGetTexture(textureId);
	GDrawTexture *gdrawTex = gdraw_GCM_WrappedTextureCreate(tex);
	return gdrawTex;
}

void ConsoleUIController::destroySubstitutionTexture(void *destroyCallBackData, GDrawTexture *handle)
{
	/* Destroys the GDraw wrapper for a wrapped texture object. This will free up
	a GDraw texture handle but not release the associated D3D texture; that is
	up to you. */
	gdraw_GCM_WrappedTextureDestroy(handle);
}

void ConsoleUIController::shutdown()
{
#ifdef _ENABLEIGGY
	/* Destroy the GDraw context. This frees all resources, shaders etc.
	allocated by GDraw. Note this is only safe to call after all
	active Iggy player have been destroyed! */
	gdraw_GCM_DestroyContext();
#endif
}

void ConsoleUIController::beginIggyCustomDraw4J(IggyCustomDrawCallbackRegion *region, CustomDrawData *customDrawRegion)
{
	PIXBeginNamedEvent(0,"Starting Iggy custom draw\n");

	PIXBeginNamedEvent(0,"Gdraw setup");
	// get the correct object-to-world matrix from GDraw, and set the render state to a normal state
	gdraw_GCM_BeginCustomDraw(region, customDrawRegion->mat);
	PIXEndNamedEvent();
}


void ConsoleUIController::handleUnlockFullVersionCallback()
{
	for(unsigned int i = 0; i < eUIGroup_COUNT; ++i)
	{
		ui.m_groups[i]->handleUnlockFullVersion();
	}
}

