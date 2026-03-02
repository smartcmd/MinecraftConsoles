#include "stdafx.h"
#include "SheepRenderer.h"
#include "MultiPlayerLocalPlayer.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"

SheepRenderer::SheepRenderer(Model *model, Model *armor, float shadow) : MobRenderer(model, shadow)
{
	setArmor(armor);
}

int SheepRenderer::prepareArmor(shared_ptr<Mob> _sheep, int layer, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<Sheep> sheep = dynamic_pointer_cast<Sheep>(_sheep);

    if (layer == 0 && !sheep->isSheared() &&
		!sheep->isInvisibleTo(Minecraft::GetInstance()->player)) // 4J-JEV: Todo, merge with java fix (for invisible sheep armour) in '1.7.5'.
	{
		MemSect(31);
		bindTexture(TN_MOB_SHEEP_FUR);	// 4J was L"/mob/sheep_fur.png"
		MemSect(0);
		// 4J - change brought forward from 1.8.2
        float brightness = SharedConstants::TEXTURE_LIGHTING ? 1.0f : sheep->getBrightness(a);
        int color = sheep->getColor();
        glColor3f(brightness * Sheep::COLOR[color][0], brightness * Sheep::COLOR[color][1], brightness * Sheep::COLOR[color][2]);
        return 1;
    }
    return -1;
}

void SheepRenderer::render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a)
{
	MobRenderer::render(mob, x, y, z, rot, a);
} 
