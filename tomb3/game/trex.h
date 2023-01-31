#pragma once
#include "../global/vars.h"

void inject_trex(bool replace);

void DinoControl(short item_number);

enum TREX_STATES
{
	DINO_EMPTY1,
	DINO_STOP,
	DINO_WALK,
	DINO_RUN,
	DINO_EMPTY2,
	DINO_DEATH,
	DINO_ROAR,
	DINO_ATTACK2,
	DINO_KILL,
	DINO_LONGROARSTART,
	DINO_LONGROARMID,
	DINO_LONGROAREND,
	DINO_SNIFFSTART,
	DINO_SNIFFMID,
	DINO_SNIFFEND
};
