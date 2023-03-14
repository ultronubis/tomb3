#pragma once
#include "../global/vars.h"

void inject_lizman(bool replace);

void LizManControl(short item_number);

enum LIZMAN_STATES
{
	LIZMAN_EMPTY,
	LIZMAN_STOP,
	LIZMAN_WALK,
	LIZMAN_PUNCH2,
	LIZMAN_AIM2,
	LIZMAN_WAIT,
	LIZMAN_AIM1,
	LIZMAN_AIM0,
	LIZMAN_PUNCH1,
	LIZMAN_PUNCH0,
	LIZMAN_RUN,
	LIZMAN_DEATH,
	LIZMAN_CLIMB3,
	LIZMAN_CLIMB1,
	LIZMAN_CLIMB2,
	LIZMAN_FALL3
};
