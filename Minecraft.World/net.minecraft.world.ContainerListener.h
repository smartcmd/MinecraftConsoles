#pragma once

class SimpleContainer;

// TODO 4J Stu
// There are 2 classes called ContainerListener. One in net.minecraft.world.inventory and 
// another one in net.minecraft.world .  To avoid clashes I have renamed both and put them in a namespace
// to avoid confusion.

namespace net_minecraft_world
{
	class ContainerListener
	{
		friend class SimpleContainer;
	private:
		virtual void containerChanged(shared_ptr<SimpleContainer> simpleContainer) = 0;
	};
}