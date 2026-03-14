#include "stdafx.h"
#include "MobSpawnerRenderer.h"
#include "TileEntityRenderDispatcher.h"
#include "EntityRenderDispatcher.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "../Minecraft.World/net.minecraft.world.entity.h"

void MobSpawnerRenderer::render(shared_ptr<TileEntity> _spawner, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<MobSpawnerTileEntity> spawner = dynamic_pointer_cast<MobSpawnerTileEntity>(_spawner);
	render(spawner->getSpawner(), x, y, z, a);
    glPopMatrix();
}

void MobSpawnerRenderer::render(BaseMobSpawner *spawner, double x, double y, double z, float a)
{
	glPushMatrix();
    glTranslatef(static_cast<float>(x) + 0.5f, static_cast<float>(y), static_cast<float>(z) + 0.5f);

    shared_ptr<Entity> e = spawner->getDisplayEntity();
    if (e != nullptr)
	{
        e->setLevel(spawner->getLevel());
        float s = 7 / 16.0f;
        glTranslatef(0, 0.4f, 0);
        glRotatef(static_cast<float>(spawner->oSpin + (spawner->spin - spawner->oSpin) * a) * 10, 0, 1, 0);
        glRotatef(-30, 1, 0, 0);
        glTranslatef(0, -0.4f, 0);
        glScalef(s, s, s);
        e->moveTo(x, y, z, 0, 0);
        EntityRenderDispatcher::instance->render(e, 0, 0, 0, 0, a);
    }
}
