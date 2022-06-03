#pragma once
#include "../global/vars.h"

#define GF_ModifyInventory	( (void(__cdecl*)(long, long)) 0x004328D0 )

enum game_levels
{
	LV_GYM,
	LV_FIRSTLEVEL,
	LV_JUNGLE = 1,
	LV_TEMPLE,
	LV_QUADBIKE,
	LV_INDIABOSS,
	LV_SHORE,
	LV_CRASH,
	LV_RAPIDS,
	LV_PACBOSS,
	LV_ROOFTOPS,
	LV_SEWER,
	LV_TOWER,
	LV_OFFICE,
	LV_DESERT,
	LV_COMPOUND,
	LV_AREA51,
	LV_ANTARC,
	LV_MINES,
	LV_CITY,
	LV_CHAMBER,
	LV_STPAULS
};
