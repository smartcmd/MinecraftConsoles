#include "stdafx.h"
#include "BoatRenderer.h"
#include "BoatModel.h"
#include "../Minecraft.World/net.minecraft.world.entity.item.h"
#include "../Minecraft.World/Mth.h"

ResourceLocation BoatRenderer::BOAT_LOCATION = ResourceLocation(TN_ITEM_BOAT);

BoatRenderer::BoatRenderer()  : EntityRenderer()
{
    this->shadowRadius = 0.5f;
    model = new BoatModel();
}

void BoatRenderer::render(shared_ptr<Entity> _boat, double x, double y, double z, float rot, float a)
{
	// 4J - original version used generics and thus had an input parameter of type Boat rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
 	shared_ptr<Boat> boat = dynamic_pointer_cast<Boat>(_boat);

    glPushMatrix();

    glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));

    glRotatef(180-rot, 0, 1, 0);
    float hurt = boat->getHurtTime() - a;
    float dmg = boat->getDamage() - a;
    if (dmg<0) dmg = 0;
    if (hurt>0)
	{
        glRotatef(Mth::sin(hurt)*hurt*dmg/10*boat->getHurtDir(), 1, 0, 0);
    }

    float ss = 12/16.0f;
    glScalef(ss, ss, ss);
    glScalef(1/ss, 1/ss, 1/ss);

    bindTexture(boat);
    glScalef(-1, -1, 1);
    model->render(boat, 0, 0, -0.1f, 0, 0, 1 / 16.0f, true);
    glPopMatrix();
}

ResourceLocation *BoatRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &BOAT_LOCATION;
}