#include "stdafx.h"
#include "PistonPieceRenderer.h"
#include "Tesselator.h"
#include "Lighting.h"
#include "TileRenderer.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\PistonPieceEntity.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"

PistonPieceRenderer::PistonPieceRenderer()
{
	tileRenderer = NULL;
}

void PistonPieceRenderer::render(shared_ptr<TileEntity> _entity, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<PistonPieceEntity> entity = dynamic_pointer_cast<PistonPieceEntity>(_entity);

    Tile *tile = Tile::tiles[entity->getId()];
    if (tile != NULL && entity->getProgress(a) <= 1)	// 4J - changed condition from < to <= as our chunk update is async to main thread and so we can have to render these with progress of 1
	{
        Tesselator *t = Tesselator::getInstance();
        bindTexture(TN_TERRAIN);

        Lighting::turnOff();
		glColor4f(1, 1, 1, 1);	// 4J added - this wouldn't be needed in real opengl as the block render has vertex colours and so this isn't use, but our pretend gl always modulates with this

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glDisable(GL_CULL_FACE);

        t->begin();

        t->offset((float) x - entity->x + entity->getXOff(a), (float) y - entity->y + entity->getYOff(a), (float) z - entity->z + entity->getZOff(a));
        t->color(1, 1, 1);
        if (tile == Tile::pistonExtension && entity->getProgress(a) < 0.5f)
		{
            // extending arms may appear through the base block
            tileRenderer->tesselatePistonArmNoCulling(tile, entity->x, entity->y, entity->z, false, entity->getData());
        }
		else if (entity->isSourcePiston() && !entity->isExtending())
		{
            // special case for withdrawing the arm back into the base
            Tile::pistonExtension->setOverrideTopTexture(((PistonBaseTile *) tile)->getPlatformTexture());
            tileRenderer->tesselatePistonArmNoCulling(Tile::pistonExtension, entity->x, entity->y, entity->z, entity->getProgress(a) < 0.5f, entity->getData());
            Tile::pistonExtension->clearOverrideTopTexture();

            t->offset((float) x - entity->x, (float) y - entity->y, (float) z - entity->z);
            tileRenderer->tesselatePistonBaseForceExtended(tile, entity->x, entity->y, entity->z, entity->getData());
        }
		else
		{
            tileRenderer->tesselateInWorldNoCulling(tile, entity->x, entity->y, entity->z, entity->getData(), entity);
        }
        t->offset(0, 0, 0);
        t->end();

        Lighting::turnOn();
    }

}
	
void PistonPieceRenderer::onNewLevel(Level *level)
{
	delete tileRenderer;
	tileRenderer = new TileRenderer(level);
}
