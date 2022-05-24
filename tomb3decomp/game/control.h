#pragma once
#include "../global/vars.h"

void inject_control(bool replace);

long ControlPhase(long nframes, long demo_mode);
void AnimateItem(ITEM_INFO* item);
long GetChange(ITEM_INFO* item, ANIM_STRUCT* anim);
void TranslateItem(ITEM_INFO* item, long x, long y, long z);
FLOOR_INFO* GetFloor(long x, long y, long z, short* room_number);
long GetWaterHeight(long x, long y, long z, short room_number);
long GetHeight(FLOOR_INFO* floor, long x, long y, long z);

#define GetDoor	( (short(__cdecl*)(FLOOR_INFO*)) 0x004222B0 )
#define GetCeiling	( (long(__cdecl*)(FLOOR_INFO*, long, long, long)) 0x00421DE0 )
#define TestTriggers	( (void(__cdecl*)(short*, long)) 0x00421460 )
#define TriggerActive	( (long(__cdecl*)(ITEM_INFO*)) 0x00421D80 )
#define LOS	( (long(__cdecl*)(GAME_VECTOR*, GAME_VECTOR*)) 0x00422370 )
#define CheckNoColFloorTriangle	( (long(__cdecl*)(FLOOR_INFO*, long, long)) 0x004231F0 )
#define CheckNoColCeilingTriangle	( (long(__cdecl*)(FLOOR_INFO*, long, long)) 0x004232B0 )
