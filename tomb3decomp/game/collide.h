#pragma once
#include "../global/vars.h"

void inject_collide(bool replace);

void ShiftItem(ITEM_INFO* item, COLL_INFO* coll);
void ObjectCollision(short item_num, ITEM_INFO* l, COLL_INFO* coll);
void ObjectCollisionSub(short item_num, ITEM_INFO* l, COLL_INFO* coll);

#define GetCollisionInfo	( (void(__cdecl*)(COLL_INFO*, long, long, long, short, long)) 0x0041D500 )
#define LaraBaddieCollision	( (void(__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x0041E8D0 )
#define UpdateLaraRoom	( (void(__cdecl*)(ITEM_INFO*, long)) 0x0041E6D0 )
#define TestBoundsCollide	( (long(__cdecl*)(ITEM_INFO*, ITEM_INFO*, long)) 0x0041F0E0 )
#define ItemPushLara	( (void(__cdecl*)(ITEM_INFO*, ITEM_INFO*, COLL_INFO*, long, long)) 0x0041EDA0 )
#define TestLaraPosition	( (long(__cdecl*)(short*, ITEM_INFO*, ITEM_INFO*)) 0x0041F1B0 )
#define MoveLaraPosition	( (long(__cdecl*)(PHD_VECTOR*, ITEM_INFO*, ITEM_INFO*)) 0x0041F430 )
#define AlignLaraPosition	( (void(__cdecl*)(PHD_VECTOR*, ITEM_INFO*, ITEM_INFO*)) 0x0041F2F0 )
