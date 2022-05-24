#include "../tomb3/pch.h"
#include "control.h"
#include "../specific/specific.h"
#include "../specific/smain.h"
#include "../specific/input.h"
#include "demo.h"
#include "gameflow.h"
#include "inventry.h"
#include "savegame.h"
#include "../specific/game.h"
#include "effect2.h"
#include "objects.h"
#include "box.h"
#include "laramisc.h"
#include "hair.h"
#include "sound.h"
#include "camera.h"
#include "effects.h"
#include "../3dsystem/phd_math.h"
#include "../specific/workstubs.h"

long ControlPhase(long nframes, long demo_mode)
{
	ITEM_INFO* item;
	FX_INFO* fx;
	static long framecount = 0;
	long return_val, falloff, r, g, b;
	short item_number, nex, room_number;

	if (nframes > 10)
		nframes = 10;

	for (framecount += nframes; framecount > 0; framecount -= 2)
	{
		if (cdtrack > 0)
			S_CDLoop();

		if (!gameflow.cheatmodecheck_disabled)
			CheckCheatMode();

		if (level_complete)
			return 1;

		S_UpdateInput();

		if (reset_flag)
			return EXIT_TO_TITLE;

		if (demo_mode)
		{
			if (input)
				return gameflow.on_demo_interrupt;

			GetDemoInput();

			if (input == -1)
			{
				input = 0;
				return gameflow.on_demo_end;
			}
		}
		else if (gameflow.noinput_timeout)
		{
			if (input)
				noinput_count = 0;
			else
			{
				noinput_count++;

				if (noinput_count > gameflow.noinput_time)
					return STARTDEMO;
			}
		}

		if (lara.death_count > 300 || lara.death_count > 60 && input || overlay_flag == 2)
		{
			if (demo_mode)
				return gameflow.ondeath_demo_mode;

			if (CurrentLevel == LV_GYM)
				return EXIT_TO_TITLE;

			if (gameflow.ondeath_ingame)
				return  gameflow.ondeath_ingame;

			if (overlay_flag == 2)
			{
				overlay_flag = 1;
				return_val = Display_Inventory(5);

				if (return_val)
					return return_val;
			}
			else
				overlay_flag = 2;
		}

		if ((input & IN_OPTION || input & IN_LOAD || input & IN_SAVE || overlay_flag <= 0) && !lara.death_count && !lara.extra_anim)
		{
			if (overlay_flag > 0)
			{
				if (!gameflow.loadsave_disabled)
				{
					if (input & IN_LOAD)
						overlay_flag = -1;
					else if (input & IN_SAVE)
						overlay_flag = -2;
					else
						overlay_flag = 0;
				}
				else
					overlay_flag = 0;
			}
			else
			{
				if (overlay_flag == -1)
					return_val = Display_Inventory(4);
				else if (overlay_flag == -2)
					return_val = Display_Inventory(3);
				else
					return_val = Display_Inventory(0);

				overlay_flag = 1;

				if (return_val)
				{
					if (Inventory_ExtraData[0] != 1)
						return return_val;

					if (CurrentLevel == LV_GYM)
						return 1;

					CreateSaveGameInfo();
					S_SaveGame(&savegame, sizeof(SAVEGAME_INFO), Inventory_ExtraData[1]);
					S_SaveSettings();
				}
			}
		}

		ClearDynamics();
		item_number = next_item_active;

		while (item_number != NO_ITEM)
		{
			item = &items[item_number];
			nex = item->next_active;

			if (objects[item->object_number].control)
				objects[item->object_number].control(item_number);

			if (nex != NO_ITEM)
			{
				if (item->object_number == ROCKET || item->object_number == GRENADE)
				{
					if (!items[nex].active)
						nex = item_after_projectile;
				}
			}

			item_number = nex;
		}

		item_number = next_fx_active;

		while (item_number != NO_ITEM)
		{
			fx = &effects[item_number];
			nex = fx->next_active;

			if (objects[fx->object_number].control)
				objects[fx->object_number].control(item_number);

			item_number = nex;
		}

		room_number = lara_item->room_number;
		GetFloor(lara_item->pos.x_pos, lara_item->pos.y_pos - 512, lara_item->pos.z_pos, &room_number);

		if (KillEverythingFlag)
			KillEverything();

		if (SmokeCountL)
			SmokeCountL--;

		if (SmokeCountR)
			SmokeCountR--;

		if (SplashCount)
			SplashCount--;

		if (lara.has_fired && !(wibble & 0x7F))
		{
			AlertNearbyGuards(lara_item);
			lara.has_fired = 0;
		}

		if (lara.poisoned >= 16 && !(wibble & 0xFF))
		{
			if (lara.poisoned > 256)
				lara.poisoned = 256;

			lara_item->hit_points -= lara.poisoned >> 4;
			PoisonFlag = 16;
		}

		for (int i = 0; i < 15; i++)
			GotJointPos[i] = 0;

		lara_item->item_flags[0] = 0;
		LaraControl(0);
		HairControl(0);

		if (lara.electric)
		{
			if (lara.electric < 12)
			{
				r = (GetRandomControl() & 7) - lara.electric + 16;
				g = 32 - lara.electric;
				b = 31;
				falloff = (GetRandomControl() & 1) - 2 * lara.electric + 25;
			}
			else
			{
				r = 0;
				g = (GetRandomControl() & 7) + 8;
				b = (GetRandomControl() & 7) + 16;
				falloff = (GetRandomControl() & 3) + 8;
			}

			TriggerDynamic(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, falloff, r, g, b);
			SoundEffect(SFX_LARA_ELECTRIC_LOOP, &lara_item->pos, SFX_DEFAULT);
			SoundEffect(SFX_LARA_ELECTRIC_CRACKLES, &lara_item->pos, SFX_DEFAULT);
		}

		CalculateCamera();
		wibble = (wibble + 4) & 0xFC;

		if (CurrentLevel == LV_ANTARC || CurrentLevel == LV_MINES)
			LaraBreath(lara_item);

		UpdateSparks();
		UpdateSplashes();
		UpdateBats();
		SoundEffects();
		health_bar_timer--;

		if (CurrentLevel != LV_GYM || assault_timer_active)
			savegame.timer++;

		if (assault_penalty_display_timer)
			assault_penalty_display_timer--;

		if (QuadbikeLapTimeDisplayTimer)
			QuadbikeLapTimeDisplayTimer--;
	}

	return 0;
}

void AnimateItem(ITEM_INFO* item)
{
	ANIM_STRUCT* anim;
	short* command;
	long speed;
	ushort type, num;

	anim = &anims[item->anim_number];
	item->touch_bits = 0;
	item->hit_status = 0;
	item->frame_number++;

	if (anim->number_changes > 0 && GetChange(item, anim))
	{
		anim = &anims[item->anim_number];
		item->current_anim_state = anim->current_anim_state;

		if (item->required_anim_state == item->current_anim_state)
			item->required_anim_state = 0;
	}

	if (item->frame_number > anim->frame_end)
	{
		if (anim->number_commands > 0)
		{
			command = &commands[anim->command_index];

			for (int i = 0; i < anim->number_commands; i++)
			{
				switch (*command++)
				{
				case COMMAND_MOVE_ORIGIN:
					TranslateItem(item, *command, command[1], command[2]);
					command += 3;
					break;

				case COMMAND_JUMP_VELOCITY:
					item->fallspeed = *command++;
					item->speed = *command++;
					item->gravity_status = 1;
					break;

				case COMMAND_DEACTIVATE:

					if (objects[item->object_number].intelligent)
						item->after_death = 1;
					else
						item->after_death = 64;

					item->status = ITEM_DEACTIVATED;
					break;

				case COMMAND_SOUND_FX:
				case COMMAND_EFFECT:
					command += 2;
					break;
				}
			}
		}

		item->anim_number = anim->jump_anim_num;
		item->frame_number = anim->jump_frame_num;
		anim = &anims[item->anim_number];

		if (item->current_anim_state != anim->current_anim_state)
		{
			item->current_anim_state = anim->current_anim_state;
			item->goal_anim_state = anim->current_anim_state;
		}

		if (item->required_anim_state == item->current_anim_state)
			item->required_anim_state = 0;
	}

	if (anim->number_commands > 0)
	{
		command = &commands[anim->command_index];

		for (int i = 0; i < anim->number_commands; i++)
		{
			switch (*command++)
			{
			case COMMAND_MOVE_ORIGIN:
				command += 3;
				break;

			case COMMAND_JUMP_VELOCITY:
				command += 2;
				break;

			case COMMAND_SOUND_FX:

				if (item->frame_number == *command)
				{
					num = command[1] & 0x3FFF;
					type = command[1] & 0xC000;

					if (objects[item->object_number].water_creature)
						SoundEffect(num, &item->pos, SFX_WATER);
					else if (item->room_number == NO_ROOM)
					{
						item->pos.x_pos = lara_item->pos.x_pos;
						item->pos.y_pos = lara_item->pos.y_pos - 762;
						item->pos.z_pos = lara_item->pos.z_pos;

						if (item->object_number == HARPOON_GUN)
							SoundEffect(num, &item->pos, SFX_ALWAYS);
						else
							SoundEffect(num, &item->pos, SFX_DEFAULT);
					}
					else if (room[item->room_number].flags & ROOM_UNDERWATER)
					{
						if (type == SFX_LANDANDWATER || type == SFX_WATERONLY)
							SoundEffect(num, &item->pos, SFX_DEFAULT);
					}
					else if (type == SFX_LANDANDWATER || type == SFX_LANDONLY)
						SoundEffect(num, &item->pos, SFX_DEFAULT);
				}

				command += 2;
				break;

			case COMMAND_EFFECT:

				if (item->frame_number == *command)
				{
					FXType = command[1] & 0xC000;
					num = command[1] & 0x3FFF;
					effect_routines[num](item);
				}
				
				command += 2;
				break;
			}
		}
	}

	if (item->gravity_status)
	{
		item->fallspeed += item->fallspeed >= 128 ? 1 : 6;
		item->pos.y_pos += item->fallspeed;
	}
	else
	{
		speed = anim->velocity;

		if (anim->acceleration)
			speed += anim->acceleration * (item->frame_number - anim->frame_base);

		item->speed = short(speed >> 16);
	}

	item->pos.x_pos += (item->speed * phd_sin(item->pos.y_rot)) >> W2V_SHIFT;
	item->pos.z_pos += (item->speed * phd_cos(item->pos.y_rot)) >> W2V_SHIFT;
}

long GetChange(ITEM_INFO* item, ANIM_STRUCT* anim)
{
	CHANGE_STRUCT* change;
	RANGE_STRUCT* range;

	if (item->current_anim_state == item->goal_anim_state)
		return 0;

	change = &changes[anim->change_index];

	for (int i = 0; i < anim->number_changes; i++, change++)
	{
		if (change->goal_anim_state == item->goal_anim_state && change->number_ranges > 0)
		{
			range = &ranges[change->range_index];

			for (int j = 0; j < change->number_ranges; j++, range++)
			{
				if (item->frame_number >= range->start_frame && item->frame_number <= range->end_frame)
				{
					item->anim_number = range->link_anim_num;
					item->frame_number = range->link_frame_num;
					return 1;
				}
			}
		}
	}

	return 0;
}

void TranslateItem(ITEM_INFO* item, long x, long y, long z)
{
	long c, s;

	c = phd_cos(item->pos.y_rot);
	s = phd_sin(item->pos.y_rot);
	item->pos.x_pos += (z * s + x * c) >> W2V_SHIFT;
	item->pos.y_pos += y;
	item->pos.z_pos += (z * c - x * s) >> W2V_SHIFT;
}

FLOOR_INFO* GetFloor(long x, long y, long z, short* room_number)
{
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	long x_floor, y_floor, retval;
	short data;

	r = &room[*room_number];

	do
	{
		x_floor = (z - r->z) >> WALL_SHIFT;
		y_floor = (x - r->x) >> WALL_SHIFT;

		if (x_floor <= 0)
		{
			x_floor = 0;

			if (y_floor < 1)
				y_floor = 1;
			else if (y_floor > r->y_size - 2)
				y_floor = r->y_size - 2;
		}
		else if (x_floor >= r->x_size - 1)
		{
			x_floor = r->x_size - 1;

			if (y_floor < 1)
				y_floor = 1;
			else if (y_floor > r->y_size - 2)
				y_floor = r->y_size - 2;
		}
		else if (y_floor < 0)
			y_floor = 0;
		else if (y_floor >= r->y_size)
			y_floor = r->y_size - 1;

		floor = &r->floor[x_floor + y_floor * r->x_size];
		data = GetDoor(floor);

		if (data != NO_ROOM)
		{
			*room_number = data;
			r = &room[data];
		}

	} while (data != NO_ROOM);

	if (y >= floor->floor << 8)
	{
		do
		{
			if (floor->pit_room == NO_ROOM)
				return floor;

			retval = CheckNoColFloorTriangle(floor, x, z);

			if(retval == 1 || retval == -1 && y < r->minfloor)
				break;

			*room_number = floor->pit_room;
			r = &room[floor->pit_room];
			floor = &r->floor[((z - r->z) >> WALL_SHIFT) + r->x_size * ((x - r->x) >> WALL_SHIFT)];

		} while (y >= floor->floor << 8);
	}
	else if (y < floor->ceiling << 8)
	{
		do
		{
			if (floor->sky_room == NO_ROOM)
				return floor;

			retval = CheckNoColCeilingTriangle(floor, x, z);

			if (retval == 1 || retval == -1 && y >= r->maxceiling)
				break;

			*room_number = floor->sky_room;
			r = &room[floor->sky_room];
			floor = &r->floor[((z - r->z) >> WALL_SHIFT) + r->x_size * ((x - r->x) >> WALL_SHIFT)];

		} while (y < floor->ceiling << 8);
	}

	return floor;
}

long GetWaterHeight(long x, long y, long z, short room_number)
{
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	long x_floor, y_floor;
	short data;

	r = &room[room_number];

	do
	{
		x_floor = (z - r->z) >> WALL_SHIFT;
		y_floor = (x - r->x) >> WALL_SHIFT;

		if (x_floor <= 0)
		{
			x_floor = 0;

			if (y_floor < 1)
				y_floor = 1;
			else if (y_floor > r->y_size - 2)
				y_floor = r->y_size - 2;
		}
		else if (x_floor >= r->x_size - 1)
		{
			x_floor = r->x_size - 1;

			if (y_floor < 1)
				y_floor = 1;
			else if (y_floor > r->y_size - 2)
				y_floor = r->y_size - 2;
		}
		else if (y_floor < 0)
			y_floor = 0;
		else if (y_floor >= r->y_size)
			y_floor = r->y_size - 1;

		floor = &r->floor[x_floor + y_floor * r->x_size];
		data = GetDoor(floor);

		if (data != NO_ROOM)
		{
			room_number = data;
			r = &room[data];
		}

	} while (data != NO_ROOM);

	if (r->flags & (ROOM_UNDERWATER | ROOM_SWAMP))
	{
		while (floor->sky_room != NO_ROOM)
		{
			if (CheckNoColCeilingTriangle(floor, x, z) == 1)
				break;

			r = &room[floor->sky_room];

			if (!(r->flags & (ROOM_UNDERWATER | ROOM_SWAMP)))
				return r->minfloor;

			floor = &r->floor[((z - r->z) >> WALL_SHIFT) + r->x_size * ((x - r->x) >> WALL_SHIFT)];
		}

		return r->maxceiling;
	}
	else
	{
		while (floor->pit_room != NO_ROOM)
		{
			if (CheckNoColFloorTriangle(floor, x, z) == 1)
				break;

			r = &room[floor->pit_room];

			if (r->flags & (ROOM_UNDERWATER | ROOM_SWAMP))
				return r->maxceiling;

			floor = &r->floor[((z - r->z) >> WALL_SHIFT) + r->x_size * ((x - r->x) >> WALL_SHIFT)];
		}
	}

	return NO_HEIGHT;
}

long GetHeight(FLOOR_INFO* floor, long x, long y, long z)
{
	ROOM_INFO* r;
	ITEM_INFO* item;
	short* data;
	long h, h2, xoff, yoff, ch1, ch2;
	short type, trigger, tilts, t0, t1, t2, t3, dx, dz, hadj;

	OnObject = 0;
	height_type = WALL;

	while (floor->pit_room != NO_ROOM)
	{
		if (CheckNoColFloorTriangle(floor, x, z) == 1)
			break;

		r = &room[floor->pit_room];
		floor = &r->floor[((z - r->z) >> WALL_SHIFT) + r->x_size * ((x - r->x) >> WALL_SHIFT)];
	}

	h = floor->floor << 8;

	if (h == NO_HEIGHT)
		return h;

	if (GF_NoFloor && GF_NoFloor == h)
		h = 0x4000;

	h2 = h;
	trigger_index = 0;

	if (!floor->index)
		return h;

	tiltxoff = 0;
	tiltyoff = 0;
	data = &floor_data[floor->index];

	do
	{
		type = *data++;

		switch (type & 0x1F)
		{
		case DOOR_TYPE:
		case ROOF_TYPE:
		case SPLIT3:
		case SPLIT4:
		case NOCOLC1T:
		case NOCOLC1B:
		case NOCOLC2T:
		case NOCOLC2B:
			data++;
			break;

		case TILT_TYPE:
			xoff = *data >> 8;
			yoff = (char)*data;
			tiltxoff = xoff;
			tiltyoff = yoff;

			if (!chunky_flag || (ABS(xoff) <= 2 && ABS(yoff) <= 2))
			{
				if (ABS(xoff) > 2 || ABS(yoff) > 2)
					height_type = BIG_SLOPE;
				else
					height_type = SMALL_SLOPE;

				if (xoff < 0)
					h -= xoff * (z & (WALL_SIZE - 1)) >> 2;
				else
					h += xoff * ((-1 - (ushort)z) & (WALL_SIZE - 1)) >> 2;

				if (yoff < 0)
					h -= (yoff * (x & (WALL_SIZE - 1))) >> 2;
				else
					h += (yoff * ((-1 - (ushort)x) & (WALL_SIZE - 1))) >> 2;
			}

			data++;
			break;

		case TRIGGER_TYPE:

			if (!trigger_index)
				trigger_index = data - 1;

			data++;

			do
			{
				trigger = *data++;

				if (((trigger & 0x3C00) != (TO_OBJECT << 1)) && ((trigger & 0x3C00) == (TO_CAMERA << 10)))
				{
					trigger = *data++;
					continue;
				}

				item = &items[trigger & 0x3FF];

				if (objects[item->object_number].floor)
					objects[item->object_number].floor(item, x, y, z, &h);

			} while (!(trigger & 0x8000));

			break;

		case LAVA_TYPE:
			trigger_index = data - 1;
			break;

		case CLIMB_TYPE:
		case MONKEY_TYPE:
		case MINEL_TYPE:
		case MINER_TYPE:

			if (!trigger_index)
				trigger_index = data - 1;

			break;

		case SPLIT1:
		case SPLIT2:
		case NOCOLF1T:
		case NOCOLF1B:
		case NOCOLF2T:
		case NOCOLF2B:
			tilts = *data;
			t0 = tilts & 15;
			t1 = (tilts >> 4) & 15;
			t2 = (tilts >> 8) & 15;
			t3 = (tilts >> 12) & 15;
			dx = x & (WALL_SIZE - 1);
			dz = z & (WALL_SIZE - 1);
			xoff = 0;
			yoff = 0;
			height_type = SPLIT_TRI;

			if ((type & 0x1F) != SPLIT1 && (type & 0x1F) != NOCOLF1T && (type & 0x1F) != NOCOLF1B)
			{
				if (dx <= dz)
				{
					hadj = (type >> 10) & 0x1F;

					if (hadj & 0x10)
						hadj |= 0xFFF0;

					h += hadj << 8;
					xoff = t2 - t1;
					yoff = t3 - t2;
				}
				else
				{
					hadj = (type >> 5) & 0x1F;

					if (hadj & 0x10)
						hadj |= 0xFFF0;

					h += hadj << 8;
					xoff = t3 - t0;
					yoff = t0 - t1;
				}
			}
			else
			{
				if (dx <= (WALL_SIZE - dz))
				{
					hadj = (type >> 10) & 0x1F;

					if (hadj & 0x10)
						hadj |= 0xFFF0;

					h += hadj << 8;
					xoff = t2 - t1;
					yoff = t0 - t1;
				}
				else
				{
					hadj = (type >> 5) & 0x1F;

					if (hadj & 0x10)
						hadj |= 0xFFF0;

					h += hadj << 8;
					xoff = t3 - t0;
					yoff = t3 - t2;
				}
			}

			tiltxoff = xoff;
			tiltyoff = yoff;

			if (chunky_flag)
			{
				hadj = (hadj >> 10) & 0x1F;

				if (hadj & 0x10)
					hadj |= 0xFFF0;

				ch1 = h2;
				ch2 = ch1;
				ch1 += hadj << 8;

				hadj = (hadj >> 5) & 0x1F;

				if (hadj & 0x10)
					hadj |= 0xFFF0;

				ch2 += hadj << 8;

				if (ch1 < ch2)
					h = ch1;
				else
					h = ch2;
			}
			else
			{
				if (ABS(xoff) > 2 || ABS(yoff) > 2)
					height_type = DIAGONAL;
				else if (height_type != SPLIT_TRI)
					height_type = SMALL_SLOPE;

				if (xoff < 0)
					h -= xoff * (z & (WALL_SIZE - 1)) >> 2;
				else
					h += xoff * ((-1 - (ushort)z) & (WALL_SIZE - 1)) >> 2;

				if (yoff < 0)
					h -= yoff * (x & (WALL_SIZE - 1)) >> 2;
				else
					h += yoff * ((-1 - (ushort)x) & (WALL_SIZE - 1)) >> 2;
			}

			data++;
			break;

		default:
			S_ExitSystem("GetHeight(): Unknown type");
		}

	} while (!(type & 0x8000));

	return h;
}

void inject_control(bool replace)
{
	INJECT(0x0041FFA0, ControlPhase, replace);
	INJECT(0x00420590, AnimateItem, replace);
	INJECT(0x00420970, GetChange, replace);
	INJECT(0x00420A20, TranslateItem, replace);
	INJECT(0x00420A80, GetFloor, replace);
	INJECT(0x00420C70, GetWaterHeight, replace);
	INJECT(0x00420E10, GetHeight, replace);
}
