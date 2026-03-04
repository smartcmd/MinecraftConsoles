#pragma once
class Player;

class Input
{
public:
	float xa;
    float ya;

    bool wasJumping;
    bool jumping;
    bool sneaking;
	bool sprinting;

	Input();

    virtual void tick(LocalPlayer *player);

private:
    bool lReset;
    bool rReset;
};
