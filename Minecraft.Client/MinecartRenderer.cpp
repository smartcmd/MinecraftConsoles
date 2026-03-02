#include "stdafx.h"
#include "MinecartRenderer.h"
#include "MinecartModel.h"
#include "..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"

MinecartRenderer::MinecartRenderer()
{
    this->shadowRadius = 0.5f;
    model = new MinecartModel();
}

void MinecartRenderer::render(shared_ptr<Entity> _cart, double x, double y, double z, float rot, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<Minecart> cart = dynamic_pointer_cast<Minecart>(_cart);

    glPushMatrix();
	
	__int64 seed = cart->entityId * 493286711l;
	seed = seed * seed * 4392167121l + seed * 98761;

	float xo = ((((seed >> 16) & 0x7) + 0.5f) / 8.0f - 0.5f) * 0.004f;
	float yo = ((((seed >> 20) & 0x7) + 0.5f) / 8.0f - 0.5f) * 0.004f;
	float zo = ((((seed >> 24) & 0x7) + 0.5f) / 8.0f - 0.5f) * 0.004f;

	glTranslatef(xo, yo, zo);

    double xx = cart->xOld + (cart->x - cart->xOld) * a;
    double yy = cart->yOld + (cart->y - cart->yOld) * a;
    double zz = cart->zOld + (cart->z - cart->zOld) * a;

    double r = 0.3f;

    Vec3 *p = cart->getPos(xx, yy, zz);

    float xRot = cart->xRotO + (cart->xRot - cart->xRotO) * a;

    if (p != NULL)
	{
        Vec3 *p0 = cart->getPosOffs(xx, yy, zz, r);
        Vec3 *p1 = cart->getPosOffs(xx, yy, zz, -r);
        if (p0 == NULL) p0 = p;
        if (p1 == NULL) p1 = p;

        x += p->x - xx;
        y += (p0->y + p1->y) / 2 - yy;
        z += p->z - zz;

        Vec3 *dir = p1->add(-p0->x, -p0->y, -p0->z);
        if (dir->length() == 0)
		{
        }
		else
		{
            dir = dir->normalize();
            rot = (float) (atan2(dir->z, dir->x) * 180 / PI);
            xRot = (float) (atan(dir->y) * 73);
        }
    }
    glTranslatef((float) x, (float) y, (float) z);

    glRotatef(180 - rot, 0, 1, 0);
    glRotatef(-xRot, 0, 0, 1);
    float hurt = cart->getHurtTime() - a;
    float dmg = cart->getDamage() - a;
    if (dmg < 0) dmg = 0;
    if (hurt > 0)
	{
        glRotatef(Mth::sin(hurt) * hurt * dmg / 10 * cart->getHurtDir(), 1, 0, 0);
    }

    if (cart->type != Minecart::RIDEABLE)
	{
		glPushMatrix();
        bindTexture(TN_TERRAIN);	// 4J was L"/terrain.png"
        float ss = 12 / 16.0f;
        glScalef(ss, ss, ss);

		// 4J - changes here brought forward from 1.2.3
		if (cart->type == Minecart::CHEST)
		{
			glTranslatef(0 / 16.0f, 8 / 16.0f, 0 / 16.0f);
			TileRenderer *tr = new TileRenderer();
			tr->renderTile(Tile::chest, 0, cart->getBrightness(a));
			delete tr;
		}
		else if (cart->type == Minecart::FURNACE)
		{
			glTranslatef(0, 6 / 16.0f, 0);
			TileRenderer *tr = new TileRenderer();
			tr->renderTile(Tile::furnace, 0, cart->getBrightness(a));
			delete tr;
		}
		glPopMatrix();
		glColor4f(1, 1, 1, 1);
    }

    bindTexture(TN_ITEM_CART);		// 4J - was L"/item/cart.png"
    glScalef(-1, -1, 1);
    // model.render(0, 0, cart->getLootContent() * 7.1f - 0.1f, 0, 0, 1 /
// 16.0f);
    model->render(cart, 0, 0, -0.1f, 0, 0, 1 / 16.0f, true);
    glPopMatrix();

}