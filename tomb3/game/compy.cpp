#include "../tomb3/pch.h"
#include "compy.h"
#include "box.h"
#include "../3dsystem/phd_math.h"
#include "../specific/game.h"
#include "control.h"
#include "effects.h"
#include "objects.h"
#include "lara.h"

static BITE_INFO compy_hit = { 0, 0, 0, 2 };

void InitialiseCompy(short item_number)
{
	InitialiseCreature(item_number);
	compys_attack_lara = 0;
}

void CompyControl(short item_number)
{
	ITEM_INFO* item;
	ITEM_INFO* raptor;
	CREATURE_INFO* compy;
	FLOOR_INFO* floor;
	AI_INFO info;
	long dx, dz;
	short angle, torso, head, dead_raptor, linknum, bits, room_number;

	if (CreatureActive(item_number))
	{
		item = &items[item_number];
		compy = (CREATURE_INFO*)item->data;
		angle = 0;
		head = 0;
		torso = 0;
		dead_raptor = 0;

		if (!item->item_flags[1])
		{
			linknum = room[item->room_number].item_number;

			if (items[linknum].object_number == ANIMATING6)
				item->item_flags[1] = linknum;
			else
			{
				while (linknum != NO_ITEM)
				{
					if (items[linknum].object_number == ANIMATING6)
					{
						item->item_flags[1] = linknum;
						break;
					}

					linknum = items[linknum].next_item;
				}
			}
		}

		dead_raptor = item->item_flags[1];

		if (item->hit_points <= 0)
		{
			if (item->current_anim_state != COMPY_DEATH)
			{
				item->anim_number = objects[item->object_number].anim_index + 6;
				item->current_anim_state = 4;
				item->frame_number = anims[item->anim_number].frame_base;
			}
		}
		else
		{
			CreatureAIInfo(item, &info);

			if (compy->mood == BORED_MOOD && dead_raptor)
			{
				raptor = &items[dead_raptor];
				dx = raptor->pos.x_pos - item->pos.x_pos;
				dz = raptor->pos.z_pos - item->pos.z_pos;
				info.distance = SQUARE(dx) + SQUARE(dz);
				info.angle = short(phd_atan(dz, dx) - item->pos.y_rot);

				if (info.angle > -0x4000 && info.angle < 0x4000)
					info.ahead = 1;
				else
					info.ahead = 0;
			}

			bits = 512 * (item_number & 0x7) - 1792;

			if (!compy_scared_timer && !compys_attack_lara && (info.enemy_facing < 0x3000 && info.enemy_facing > -0x3000 && lara_item->speed > 15 || lara_item->current_anim_state == AS_ROLL || item->hit_status))
			{
				item->item_flags[0] = (bits + 1792) >> 7;
				compy_scared_timer = 280;
			}
			else if (compy_scared_timer)
			{
				if (item->item_flags[0] > 0)
					item->item_flags[0]--;
				else
				{
					compy->mood = ESCAPE_MOOD;
					compy_scared_timer--;
				}

				if (GetRandomControl() < 0x1F && item->timer > 180)
					compys_attack_lara = 1;
			}
			else
				compy->mood = info.zone_number == info.enemy_zone ? ATTACK_MOOD : BORED_MOOD;

			switch (compy->mood)
			{
			case BORED_MOOD:

				if (dead_raptor)
				{
					raptor = &items[dead_raptor];
					compy->target.x = raptor->pos.x_pos;
					compy->target.z = raptor->pos.z_pos;
				}

				break;
			case ATTACK_MOOD:
				compy->target.x = compy->enemy->pos.x_pos + (1024 * phd_sin(bits + item->pos.y_rot) >> W2V_SHIFT);
				compy->target.y = compy->enemy->pos.y_pos;
				compy->target.z = compy->enemy->pos.z_pos + (1024 * phd_cos(bits + item->pos.y_rot) >> W2V_SHIFT);
				break;
			case ESCAPE_MOOD:
			case STALK_MOOD:
				compy->target.x = compy->enemy->pos.x_pos + (1024 * phd_sin(bits + info.angle + 0x8000) >> W2V_SHIFT);
				compy->target.z = compy->enemy->pos.z_pos + (1024 * phd_cos(bits + info.angle + 0x8000) >> W2V_SHIFT);
				room_number = item->room_number;
				floor = GetFloor(compy->target.x, item->pos.y_pos, compy->target.z, &room_number);

				if (abs(boxes[floor->box].height - item->pos.y_pos) > 256)
				{
					compy->mood = BORED_MOOD;
					item->item_flags[0] = (short)compy_scared_timer;
				}

				break;
			}

			angle = CreatureTurn(item, compy->maximum_turn);

			if (info.ahead)
				torso = info.angle;

			head = -(info.angle / 4);
			item->timer++;

			if (item->hit_status && item->timer > 200 && GetRandomControl() < 3100)
				compys_attack_lara = 1;

			switch (item->current_anim_state)
			{
			case COMPY_STOP:
				compy->flags &= 0xFFFE;
				compy->maximum_turn = 0x222;

				if (compy->mood == ATTACK_MOOD)
				{
					if (info.angle && info.distance < 0x718E4)
					{
						if (compys_attack_lara)
						{
							if (GetRandomControl() < 16384)
								item->goal_anim_state = COMPY_ATTACK;
							else
								item->goal_anim_state = COMPY_JUMP;
						}
						else
							item->goal_anim_state = COMPY_STOP;
					}
					else if (info.distance > 0xFF801 - 0x718E4 * compys_attack_lara)
						item->goal_anim_state = COMPY_RUN;
				}
				else if (compy->mood == BORED_MOOD)
				{
					if (info.ahead && info.distance < 0x552AB && dead_raptor)
					{
						if (GetRandomControl() < 16384)
							item->goal_anim_state = COMPY_ATTACK;
						else
							item->goal_anim_state = COMPY_JUMP;
					}
					else if (info.distance > 0x552AB)
						item->goal_anim_state = COMPY_RUN;
				}
				else if (GetRandomControl() < 4096)
					item->goal_anim_state = COMPY_JUMP;
				else
					item->goal_anim_state = COMPY_RUN;

				break;
			case COMPY_RUN:
				compy->flags &= 0xFFFE;
				compy->maximum_turn = 0x71C;

				if (info.angle < 0x3000 && info.angle > -0x3000 && info.distance < 0xFF801 - 0x718E4 * compys_attack_lara)
					item->goal_anim_state = COMPY_STOP;

				break;
			case COMPY_JUMP:
			case COMPY_ATTACK:
				compy->maximum_turn = 0x71C;

				if (!(compy->flags & 0x1))
				{
					if (item->touch_bits & 0x4 && compys_attack_lara)
					{
						compy->flags |= 0x1;
						lara_item->hit_points -= 90;
						lara_item->hit_status = 1;
						CreatureEffect(item, &compy_hit, DoBloodSplat);
					}
					else if (!(compy->flags & 0x1) && info.distance < 0x1C639 && info.ahead && dead_raptor && compy->mood == ATTACK_MOOD)
					{
						compy->flags |= 0x1;
						CreatureEffect(item, &compy_hit, DoBloodSplat);
					}
				}

				break;
			}
		}

		CreatureTilt(item, angle >> 1);
		CreatureJoint(item, 0, torso);
		CreatureJoint(item, 1, head);
		CreatureAnimation(item_number, angle, angle >> 1);
	}
}

void inject_compy(bool replace)
{
	INJECT(0x0041F730, InitialiseCompy, replace);
	INJECT(0x0041F750, CompyControl, replace);
}
