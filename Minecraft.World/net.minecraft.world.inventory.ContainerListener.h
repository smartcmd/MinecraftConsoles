#pragma once
using namespace std;
class AbstractContainerMenu;

// 4J Stu
// There are 2 classes called ContainerListener. Once here in net.minecraft.world.inventory and 
// another once in net.minecraft.world .  To avoid clashes I have renamed both and put them in a namespace
// to avoid confusion.

namespace net_minecraft_world_inventory
{
	class ContainerListener
	{
	public:
		virtual void refreshContainer(AbstractContainerMenu *container, vector<shared_ptr<ItemInstance> > *items) = 0;

		virtual void slotChanged(AbstractContainerMenu *container, int slotIndex, shared_ptr<ItemInstance> item) = 0;

		virtual void setContainerData(AbstractContainerMenu *container, int id, int value) = 0;
	};
}