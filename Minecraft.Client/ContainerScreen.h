#pragma once
#include "AbstractContainerScreen.h"
class Container;

class ContainerScreen : public AbstractContainerScreen
{
private:
	shared_ptr<Container> inventory;
	shared_ptr<Container> container;

	int containerRows;

public:
	ContainerScreen(shared_ptr<Container>inventory, shared_ptr<Container>container);

protected:
	virtual void renderLabels();
	virtual void renderBg(float a);
};