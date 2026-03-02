#pragma once

class OwnableEntity
{
public:
	virtual wstring getOwnerUUID() = 0;
	virtual shared_ptr<Entity> getOwner() = 0;
};