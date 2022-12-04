#include "../tomb3/pch.h"
#include "collide.h"
#include "sphere.h"
#include "control.h"
#include "../3dsystem/phd_math.h"
#include "items.h"
#include "objects.h"
#include "draw.h"
#include "../3dsystem/3d_gen.h"

void ShiftItem(ITEM_INFO* item, COLL_INFO* coll)
{
	item->pos.x_pos += coll->shift.x;
	item->pos.y_pos += coll->shift.y;
	item->pos.z_pos += coll->shift.z;
	coll->shift.z = 0;
	coll->shift.y = 0;
	coll->shift.x = 0;
}

void ObjectCollision(short item_num, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item;

	item = &items[item_num];

	if (TestBoundsCollide(item, l, coll->radius) && TestCollision(item, l) && coll->enable_baddie_push)
		ItemPushLara(item, l, coll, 0, 1);
}

void ObjectCollisionSub(short item_num, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item;

	item = &items[item_num];

	if (TestBoundsCollide(item, l, coll->radius) && TestCollision(item, l))
		ItemPushLara(item, l, coll, 0, 0);
}

void GetNewRoom(long x, long y, long z, short room_number)
{
	long r;

	GetFloor(x, y, z, &room_number);

	for (r = 0; r < number_draw_rooms; r++)
	{
		if (draw_rooms[r] == room_number)
			break;
	}

	if (r == number_draw_rooms)
	{
		draw_rooms[number_draw_rooms] = room_number;
		number_draw_rooms++;
	}
}

void GetNearByRooms(long x, long y, long z, long r, long h, short room_number)
{
	draw_rooms[0] = room_number;
	number_draw_rooms = 1;
	GetNewRoom(r + x, y, r + z, room_number);
	GetNewRoom(x - r, y, r + z, room_number);
	GetNewRoom(r + x, y, z - r, room_number);
	GetNewRoom(x - r, y, z - r, room_number);
	GetNewRoom(r + x, y - h, r + z, room_number);
	GetNewRoom(x - r, y - h, r + z, room_number);
	GetNewRoom(r + x, y - h, z - r, room_number);
	GetNewRoom(x - r, y - h, z - r, room_number);
}

short GetTiltType(FLOOR_INFO* floor, long x, long y, long z)
{
	ROOM_INFO* r;
	short* data;
	short type, t0, t1, t2, t3, tilt, x2, z2, x3, y2;

	while (floor->pit_room != 255)
	{
		if (CheckNoColFloorTriangle(floor, x, z) == 1)
			break;

		r = &room[floor->pit_room];
		floor = &r->floor[((z - r->z) >> 10) + (((x - r->x) >> 10) * r->x_size)];
	}

	if (y + 512 < floor->floor << 8)
		return 0;

	if (floor->index)
	{
		data = &floor_data[floor->index];
		type = data[0] & 0x1F;

		if (type == TILT_TYPE)
			return data[1];

		if (type == SPLIT1 || type == SPLIT2 || type == NOCOLF1T || type == NOCOLF2T || type == NOCOLF1B || type == NOCOLF2B)
		{
			tilt = data[1];
			t0 = tilt & 0xF;
			t1 = (tilt >> 4) & 0xF;
			t2 = (tilt >> 8) & 0xF;
			t3 = (tilt >> 12) & 0xF;
			x2 = x & 0x3FF;
			z2 = z & 0x3FF;
			type = type & 0x1F;

			if (type == SPLIT1 || type == NOCOLF1T || type == NOCOLF1B)
			{
				if (x2 > 1024 - z2)
				{
					x3 = t3 - t0;
					y2 = t3 - t2;
				}
				else
				{
					x3 = t2 - t1;
					y2 = t0 - t1;
				}
			}
			else if (x2 > z2)
			{
				x3 = t3 - t0;
				y2 = t0 - t1;
			}
			else
			{
				x3 = t2 - t1;
				y2 = t3 - t2;
			}

			return ((x3 << 8) | (y2 & 0xFF));
		}
	}

	return 0;
}

long CollideStaticObjects(COLL_INFO* coll, long x, long y, long z, short room_number, long hite)
{
	ROOM_INFO* r;
	MESH_INFO* mesh;
	STATIC_INFO* sinfo;
	long lxmin, lxmax, lymin, lymax, lzmin, lzmax;
	long xmin, xmax, ymin, ymax, zmin, zmax;
	long ls, rs, xs, zs;

	coll->hit_static = 0;
	lxmin = x - coll->radius;
	lxmax = x + coll->radius;
	lymin = y - hite;
	lymax = y;
	lzmin = z - coll->radius;
	lzmax = z + coll->radius;
	GetNearByRooms(x, y, z, coll->radius + 50, hite + 50, room_number);

	for (int i = 0; i < number_draw_rooms; i++)
	{
		r = &room[draw_rooms[i]];
		mesh = r->mesh;

		for (int j = r->num_meshes; j > 0; j--, mesh++)
		{
			sinfo = &static_objects[mesh->static_number];

			if (sinfo->flags & 1)
				continue;

			ymin = mesh->y + sinfo->y_minc;
			ymax = mesh->y + sinfo->y_maxc;

			if (mesh->y_rot == -0x8000)
			{
				xmin = mesh->x - sinfo->x_maxc;
				xmax = mesh->x - sinfo->x_minc;
				zmin = mesh->z - sinfo->z_maxc;
				zmax = mesh->z - sinfo->z_minc;
			}
			else if (mesh->y_rot == -0x4000)
			{
				xmin = mesh->x - sinfo->z_maxc;
				xmax = mesh->x - sinfo->z_minc;
				zmin = mesh->z + sinfo->x_minc;
				zmax = mesh->z + sinfo->x_maxc;
			}
			else if (mesh->y_rot == 0x4000)
			{
				xmin = mesh->x + sinfo->z_minc;
				xmax = mesh->x + sinfo->z_maxc;
				zmin = mesh->z - sinfo->x_maxc;
				zmax = mesh->z - sinfo->x_minc;
			}
			else
			{
				xmin = mesh->x + sinfo->x_minc;
				xmax = mesh->x + sinfo->x_maxc;
				zmin = mesh->z + sinfo->z_minc;
				zmax = mesh->z + sinfo->z_maxc;
			}

			if (lxmax <= xmin || lxmin >= xmax || lymax <= ymin || lymin >= ymax || lzmax <= zmin || lzmin >= zmax)
				continue;

			ls = lxmax - xmin;
			rs = xmax - lxmin;
			xs = ls >= rs ? rs : -ls;

			ls = lzmax - zmin;
			rs = zmax - lzmin;
			zs = ls >= rs ? rs : -ls;

			switch (coll->quadrant)
			{
			case NORTH:

				if (xs > coll->radius || xs < -coll->radius)
				{
					coll->shift.x = coll->old.x - x;
					coll->shift.z = zs;
					coll->coll_type = CT_FRONT;
				}
				else if (xs > 0)
				{
					coll->shift.x = xs;
					coll->shift.z = 0;
					coll->coll_type = CT_LEFT;
				}
				else if (xs < 0)
				{
					coll->shift.x = xs;
					coll->shift.z = 0;
					coll->coll_type = CT_RIGHT;
				}

				break;

			case EAST:

				if (zs > coll->radius || zs < -coll->radius)
				{
					coll->shift.x = xs;
					coll->shift.z = coll->old.z - z;
					coll->coll_type = CT_FRONT;
				}
				else if (zs > 0)
				{
					coll->shift.x = 0;
					coll->shift.z = zs;
					coll->coll_type = CT_RIGHT;
				}
				else if (zs < 0)
				{
					coll->shift.x = 0;
					coll->shift.z = zs;
					coll->coll_type = CT_LEFT;
				}

				break;

			case SOUTH:

				if (xs > coll->radius || xs < -coll->radius)
				{
					coll->shift.x = coll->old.x - x;
					coll->shift.z = zs;
					coll->coll_type = CT_FRONT;
				}
				else if (xs > 0)
				{
					coll->shift.x = xs;
					coll->shift.z = 0;
					coll->coll_type = CT_RIGHT;
				}
				else if (xs < 0)
				{
					coll->shift.x = xs;
					coll->shift.z = 0;
					coll->coll_type = CT_LEFT;
				}

				break;

			case WEST:

				if (zs > coll->radius || zs < -coll->radius)
				{
					coll->shift.x = xs;
					coll->shift.z = coll->old.z - z;
					coll->coll_type = CT_FRONT;
				}
				else if (zs > 0)
				{
					coll->shift.x = 0;
					coll->shift.z = zs;
					coll->coll_type = CT_LEFT;
				}
				else if (zs < 0)
				{
					coll->shift.x = 0;
					coll->shift.z = zs;
					coll->coll_type = CT_RIGHT;
				}

				break;
			}

			coll->hit_static = 1;
			return 1;
		}
	}

	return 0;
}

long FindGridShift(long src, long dst)
{
	long srcw, dstw;

	srcw = src >> WALL_SHIFT;
	dstw = dst >> WALL_SHIFT;

	if (srcw == dstw)
		return 0;

	src &= WALL_SIZE - 1;

	if (dstw > srcw)
		return (WALL_SIZE + 1) - src;
	else
		return -1 - src;
}

void GetCollisionInfo(COLL_INFO* coll, long x, long y, long z, short room_number, long hite)
{
	FLOOR_INFO* floor;
	static long xfront, zfront;
	long yT, h, c, tx, tz;
	long ang, xright, xleft, zright, zleft, xright2, xleft2, zright2, zleft2, hit_left, hit_right;
	short room_num, room_num2, tilt;

	coll->coll_type = CT_NONE;
	coll->shift.x = 0;
	coll->shift.y = 0;
	coll->shift.z = 0;
	coll->quadrant = ushort(coll->facing + 0x2000) / 0x4000;

	ang = abs(lara_item->pos.y_rot - coll->facing) > 0x7000 ? 0x3000 : 0x4000;
	xright2 = (250 * phd_sin(coll->facing + ang)) >> W2V_SHIFT;
	xleft2 = (250 * phd_sin(coll->facing - ang)) >> W2V_SHIFT;
	zright2 = (250 * phd_cos(coll->facing + ang)) >> W2V_SHIFT;
	zleft2 = (250 * phd_cos(coll->facing - ang)) >> W2V_SHIFT;
	hit_left = 0;
	hit_right = 0;

	yT = y - hite - 160;
	room_num = room_number;
	floor = GetFloor(x, yT, z, &room_num);
	h = GetHeight(floor, x, yT, z);

	if (h != NO_HEIGHT)
		h -= y;

	c = GetCeiling(floor, x, yT, z);

	if (c != NO_HEIGHT)
		c -= y - hite;

	coll->mid_floor = h;
	coll->mid_ceiling = c;
	coll->mid_type = height_type;
	coll->trigger = trigger_index;
	tilt = GetTiltType(floor, x, lara_item->pos.y_pos, z);
	coll->tilt_x = (char)tilt;
	coll->tilt_z = tilt >> 8;

	switch (coll->quadrant)
	{
	case NORTH:
		xfront = (coll->radius * phd_sin(coll->facing)) >> W2V_SHIFT;
		zfront = coll->radius;
		xright = coll->radius;
		zright = coll->radius;
		zleft = coll->radius;
		xleft = -coll->radius;
		break;

	case EAST:
		xfront = coll->radius;
		zfront = (coll->radius * phd_cos(coll->facing)) >> W2V_SHIFT;
		xright = coll->radius;
		zright = -coll->radius;
		zleft = coll->radius;
		xleft = coll->radius;
		break;

	case SOUTH:
		xfront = (coll->radius * phd_sin(coll->facing)) >> W2V_SHIFT;
		zfront = -coll->radius;
		xright = -coll->radius;
		zright = -coll->radius;
		zleft = -coll->radius;
		xleft = coll->radius;
		break;

	case WEST:
		xfront = -coll->radius;
		zfront = (coll->radius * phd_cos(coll->facing)) >> W2V_SHIFT;
		xright = -coll->radius;
		zright = coll->radius;
		zleft = -coll->radius;
		xleft = -coll->radius;
		break;

	default:
		xfront = 0;
		zfront = 0;
		xright = 0;
		zright = 0;
		zleft = 0;
		xleft = 0;
		break;
	}

	/*front*/
	tx = x + xfront;
	tz = z + zfront;
	floor = GetFloor(tx, yT, tz, &room_num);
	h = GetHeight(floor, tx, yT, tz);

	if (h != NO_HEIGHT)
		h -= y;

	c = GetCeiling(floor, tx, yT, tz);

	if (c != NO_HEIGHT)
		c -= y - hite;

	coll->front_ceiling = c;
	coll->front_floor = h;
	coll->front_type = height_type;

	tx += xfront;
	tz += zfront;
	floor = GetFloor(tx, yT, tz, &room_num);
	h = GetHeight(floor, tx, yT, tz);

	if (h != NO_HEIGHT)
		h -= y;

	if (coll->slopes_are_walls && (coll->front_type == BIG_SLOPE || coll->front_type == DIAGONAL) &&
		coll->front_floor < coll->mid_floor && h < coll->front_floor && coll->front_floor < 0)
		coll->front_floor = -32767;
	else if (coll->slopes_are_pits && (coll->front_type == BIG_SLOPE || coll->front_type == DIAGONAL) && coll->front_floor > coll->mid_floor)
		coll->front_floor = 512;
	else if (coll->lava_is_pit && coll->front_floor > 0 && trigger_index && (trigger_index[0] & 0x1F) == LAVA_TYPE)
		coll->front_floor = 512;

	/*left*/
	room_num2 = room_number;
	tx = x + xleft;
	tz = z + zleft;
	floor = GetFloor(tx, yT, tz, &room_num2);
	h = GetHeight(floor, tx, yT, tz);

	if (h != NO_HEIGHT)
		h -= y;

	c = GetCeiling(floor, tx, yT, tz);

	if (c != NO_HEIGHT)
		c -= y - hite;

	coll->left_ceiling = c;
	coll->left_floor = h;
	coll->left_type = height_type;

	if (coll->slopes_are_walls == 1 && (coll->left_type == BIG_SLOPE || coll->left_type == DIAGONAL) && coll->left_floor < 0)
		coll->left_floor = -32767;
	else if (coll->slopes_are_pits && (coll->left_type == BIG_SLOPE || coll->left_type == DIAGONAL) && coll->left_floor > 0)
		coll->left_floor = 512;
	else if (coll->lava_is_pit && coll->left_floor > 0 && trigger_index && (trigger_index[0] & 0x1F) == LAVA_TYPE)
		coll->left_floor = 512;

	floor = GetFloor(tx, yT, tz, &room_num);
	h = GetHeight(floor, tx, yT, tz);

	if (h != NO_HEIGHT)
		h -= y;

	c = GetCeiling(floor, tx, yT, tz);

	if (c != NO_HEIGHT)
		c -= y - hite;

	coll->left_ceiling2 = c;
	coll->left_floor2 = h;
	coll->left_type2 = height_type;

	if (coll->slopes_are_walls == 1 && (coll->left_type2 == BIG_SLOPE || coll->left_type2 == DIAGONAL) && coll->left_floor2 < 0)
		coll->left_floor2 = -32767;
	else if (coll->slopes_are_pits && (coll->left_type2 == BIG_SLOPE || coll->left_type2 == DIAGONAL) && coll->left_floor2 > 0)
		coll->left_floor2 = 512;
	else if (coll->lava_is_pit && coll->left_floor2 > 0 && trigger_index && (trigger_index[0] & 0x1F) == LAVA_TYPE)
		coll->left_floor2 = 512;

	/*right*/
	room_num2 = room_number;
	tx = x + xright;
	tz = z + zright;
	floor = GetFloor(tx, yT, tz, &room_num2);
	h = GetHeight(floor, tx, yT, tz);

	if (h != NO_HEIGHT)
		h -= y;

	c = GetCeiling(floor, tx, yT, tz);

	if (c != NO_HEIGHT)
		c -= y - hite;

	coll->right_ceiling = c;
	coll->right_floor = h;
	coll->right_type = height_type;

	if (coll->slopes_are_walls == 1 && (coll->right_type == BIG_SLOPE || coll->right_type == DIAGONAL) && coll->right_floor < 0)
		coll->right_floor = -32767;
	else if (coll->slopes_are_pits && (coll->right_type == BIG_SLOPE || coll->right_type == DIAGONAL) && coll->right_floor > 0)
		coll->right_floor = 512;
	else if (coll->lava_is_pit && coll->right_floor > 0 && trigger_index && (trigger_index[0] & 0x1F) == LAVA_TYPE)
		coll->right_floor = 512;

	floor = GetFloor(tx, yT, tz, &room_num);
	h = GetHeight(floor, tx, yT, tz);

	if (h != NO_HEIGHT)
		h -= y;

	c = GetCeiling(floor, tx, yT, tz);

	if (c != NO_HEIGHT)
		c -= y - hite;

	coll->right_ceiling2 = c;
	coll->right_floor2 = h;
	coll->right_type2 = height_type;

	if (coll->slopes_are_walls == 1 && (coll->right_type2 == BIG_SLOPE || coll->right_type2 == DIAGONAL) && coll->right_floor2 < 0)
		coll->right_floor2 = -32767;
	else if (coll->slopes_are_pits && (coll->right_type2 == BIG_SLOPE || coll->right_type2 == DIAGONAL) && coll->right_floor2 > 0)
		coll->right_floor2 = 512;
	else if (coll->lava_is_pit && coll->right_floor2 > 0 && trigger_index && (trigger_index[0] & 0x1F) == LAVA_TYPE)
		coll->right_floor2 = 512;

	/*other left? idk*/
	room_num2 = room_number;
	tx = x + xleft2;
	tz = z + zleft2;
	floor = GetFloor(tx, yT, tz, &room_num2);
	h = GetHeight(floor, tx, yT, tz);

	if (h != NO_HEIGHT)
		h -= y;

	c = GetCeiling(floor, tx, yT, tz);

	if (c != NO_HEIGHT)
		c -= y - hite;

	if (h > coll->bad_pos || h < coll->bad_neg || c > coll->bad_ceiling)
		hit_left = 1;

	/*other right*/
	room_num2 = room_number;
	tx = x + xright2;
	tz = z + zright2;
	floor = GetFloor(tx, yT, tz, &room_num2);
	h = GetHeight(floor, tx, yT, tz);

	if (h != NO_HEIGHT)
		h -= y;

	c = GetCeiling(floor, tx, yT, tz);

	if (c != NO_HEIGHT)
		c -= y - hite;

	if (h > coll->bad_pos || h < coll->bad_neg || c > coll->bad_ceiling)
		hit_right = 1;

	if (CollideStaticObjects(coll, x, y, z, room_number, hite))
	{
		tx = x + coll->shift.x;
		tz = z + coll->shift.z;
		floor = GetFloor(tx, y, tz, &room_num);
		h = GetHeight(floor, tx, y, tz);
		c = GetCeiling(floor, tx, y, tz);

		if (h < y - 512 || c > y - hite)
		{
			coll->shift.x = -coll->shift.x;
			coll->shift.z = -coll->shift.z;
		}
	}

	if (coll->mid_floor == NO_HEIGHT)
	{
		coll->shift.x = coll->old.x - x;
		coll->shift.y = coll->old.y - y;
		coll->shift.z = coll->old.z - z;
		coll->coll_type = CT_FRONT;
		return;
	}

	if (coll->mid_floor - coll->mid_ceiling <= 0)
	{
		coll->shift.x = coll->old.x - x;
		coll->shift.y = coll->old.y - y;
		coll->shift.z = coll->old.z - z;
		coll->coll_type = CT_CLAMP;
		return;
	}

	if (coll->mid_ceiling >= 0)
	{
		coll->hit_ceiling = 1;
		coll->shift.y = coll->mid_ceiling;
		coll->coll_type = CT_TOP;
	}

#ifndef TROYESTUFF	//fix lara getting stuck in corners
	if (hit_left && hit_right)
	{
		coll->shift.x = coll->old.x - x;
		coll->shift.z = coll->old.z - z;
		coll->coll_type = CT_FRONT;
		return;
	}
#endif

	if (coll->front_floor > coll->bad_pos || coll->front_floor < coll->bad_neg || coll->front_ceiling > coll->bad_ceiling)
	{
		if (coll->front_type == DIAGONAL || coll->front_type == SPLIT_TRI)
		{
			coll->shift.x = coll->old.x - x;
			coll->shift.z = coll->old.z - z;
		}
		else
		{
			switch (coll->quadrant)
			{
			case NORTH:
			case SOUTH:
				coll->shift.x = coll->old.x - x;
				coll->shift.z = FindGridShift(z + zfront, z);
				break;

			case EAST:
			case WEST:
				coll->shift.x = FindGridShift(x + xfront, x);
				coll->shift.z = coll->old.z - z;
				break;
			}
		}

		coll->coll_type = CT_FRONT;
		return;
	}

	if (coll->front_ceiling >= coll->bad_ceiling)
	{
		coll->shift.x = coll->old.x - x;
		coll->shift.y = coll->old.y - y;
		coll->shift.z = coll->old.z - z;
		coll->coll_type = CT_TOP_FRONT;
		return;
	}

	if (coll->left_floor > coll->bad_pos || coll->left_floor < coll->bad_neg || coll->left_ceiling > coll->bad_ceiling)
	{
		if (coll->left_type == SPLIT_TRI)
		{
			coll->shift.x = coll->old.x - x;
			coll->shift.z = coll->old.z - z;
		}
		else
		{
			switch (coll->quadrant)
			{
			case NORTH:
			case SOUTH:
				coll->shift.x = FindGridShift(x + xleft, x + xfront);
				break;

			case EAST:
			case WEST:
				coll->shift.z = FindGridShift(z + zleft, z + zfront);
				break;
			}
		}

		coll->coll_type = CT_LEFT;
		return;
	}

	if (coll->bad_pos < coll->right_floor || coll->right_floor < coll->bad_neg || coll->bad_ceiling < coll->right_ceiling)
	{
		if (coll->right_type == SPLIT_TRI)
		{
			coll->shift.x = coll->old.x - x;
			coll->shift.z = coll->old.z - z;
		}
		else
		{
			switch (coll->quadrant)
			{
			case NORTH:
			case SOUTH:
				coll->shift.x = FindGridShift(x + xright, x + xfront);
				break;

			case EAST:
			case WEST:
				coll->shift.z = FindGridShift(z + zright, z + zfront);
				break;
			}
		}

		coll->coll_type = CT_RIGHT;
		return;
	}
}

void UpdateLaraRoom(ITEM_INFO* item, long height)
{
	FLOOR_INFO* floor;
	long x, y, z;
	short room_number;

	x = item->pos.x_pos;
	y = item->pos.y_pos + height;
	z = item->pos.z_pos;
	room_number = item->room_number;
	floor = GetFloor(x, y, z, &room_number);
	item->floor = GetHeight(floor, x, y, z);

	if (item->room_number != room_number)
		ItemNewRoom(lara.item_number, room_number);
}

void DoorCollision(short item_number, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item;

	item = &items[item_number];

	if (TestBoundsCollide(item, l, coll->radius) && TestCollision(item, l))
	{
		if (coll->enable_baddie_push)
		{
			if (item->current_anim_state == item->goal_anim_state)
				ItemPushLara(item, l, coll, 0, 1);
			else
				ItemPushLara(item, l, coll, coll->enable_spaz, 1);
		}
	}
}

void TrapCollision(short item_number, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item;

	item = &items[item_number];

	if (item->status == ITEM_ACTIVE)
	{
		if (!TestBoundsCollide(item, l, coll->radius))
			return;

		TestCollision(item, l);

		if (item->object_number == FAN && item->current_anim_state == 1)
			ObjectCollision(item_number, l, coll);
	}
	else if (item->status != ITEM_INVISIBLE)
		ObjectCollision(item_number, l, coll);
}

long TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, long rad)
{
	short* bounds;
	short* lbounds;
	long s, c, dx, dz, x, z;

	bounds = GetBestFrame(item);
	lbounds = GetBestFrame(l);

	if (item->pos.y_pos + bounds[3] <= l->pos.y_pos + lbounds[2] || item->pos.y_pos + bounds[2] >= l->pos.y_pos + lbounds[3])
		return 0;

	s = phd_sin(item->pos.y_rot);
	c = phd_cos(item->pos.y_rot);
	dx = l->pos.x_pos - item->pos.x_pos;
	dz = l->pos.z_pos - item->pos.z_pos;
	x = (dx * c - dz * s) >> W2V_SHIFT;
	z = (dx * s + dz * c) >> W2V_SHIFT;
	return x >= bounds[0] - rad && x <= rad + bounds[1] && z >= bounds[4] - rad && z <= rad + bounds[5];
}

long TestLaraPosition(short* bounds, ITEM_INFO* item, ITEM_INFO* l)
{
	PHD_VECTOR pos;
	long x, y, z;
	short xrot, yrot, zrot;

	xrot = l->pos.x_rot - item->pos.x_rot;
	yrot = l->pos.y_rot - item->pos.y_rot;
	zrot = l->pos.z_rot - item->pos.z_rot;

	if (xrot < bounds[6] || xrot > bounds[7] ||
		yrot < bounds[8] || yrot > bounds[9] ||
		zrot < bounds[10] || zrot > bounds[11])
		return 0;

	phd_PushUnitMatrix();
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
	pos.x = l->pos.x_pos - item->pos.x_pos;
	pos.y = l->pos.y_pos - item->pos.y_pos;
	pos.z = l->pos.z_pos - item->pos.z_pos;
	x = (pos.x * phd_mxptr[M00] + pos.y * phd_mxptr[M10] + pos.z * phd_mxptr[M20]) >> W2V_SHIFT;
	y = (pos.x * phd_mxptr[M01] + pos.y * phd_mxptr[M11] + pos.z * phd_mxptr[M21]) >> W2V_SHIFT;
	z = (pos.x * phd_mxptr[M02] + pos.y * phd_mxptr[M12] + pos.z * phd_mxptr[M22]) >> W2V_SHIFT;
	phd_PopMatrix();

	return x >= bounds[0] && x <= bounds[1] && y >= bounds[2] && y <= bounds[3] && z >= bounds[4] && z <= bounds[5];
}

void AlignLaraPosition(PHD_VECTOR* pos, ITEM_INFO* item, ITEM_INFO* l)
{
	FLOOR_INFO* floor;
	long x, y, z, h, c;
	short room_number;

	l->pos.x_rot = item->pos.x_rot;
	l->pos.y_rot = item->pos.y_rot;
	l->pos.z_rot = item->pos.z_rot;

	phd_PushUnitMatrix();
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
	x = item->pos.x_pos + ((pos->x * phd_mxptr[M00] + pos->y * phd_mxptr[M01] + pos->z * phd_mxptr[M02]) >> W2V_SHIFT);
	y = item->pos.y_pos + ((pos->x * phd_mxptr[M10] + pos->y * phd_mxptr[M11] + pos->z * phd_mxptr[M12]) >> W2V_SHIFT);
	z = item->pos.z_pos + ((pos->x * phd_mxptr[M20] + pos->y * phd_mxptr[M21] + pos->z * phd_mxptr[M22]) >> W2V_SHIFT);
	phd_PopMatrix();

	room_number = l->room_number;
	floor = GetFloor(x, y, z, &room_number);
	h = GetHeight(floor, x, y, z);
	c = GetCeiling(floor, x, y, z);

	if (abs(h - l->pos.y_pos) <= 256 && abs(c - l->pos.y_pos) >= 762)
	{
		l->pos.x_pos = x;
		l->pos.y_pos = y;
		l->pos.z_pos = z;
	}
}

long Move3DPosTo3DPos(PHD_3DPOS* pos, PHD_3DPOS* dest, long speed, short rotation)
{
	long dx, dy, dz, distance;
	short adiff;

	dx = dest->x_pos - pos->x_pos;
	dy = dest->y_pos - pos->y_pos;
	dz = dest->z_pos - pos->z_pos;
	distance = phd_sqrt(SQUARE(dx) + SQUARE(dy) + SQUARE(dz));

	if (speed < distance)
	{
		pos->x_pos += speed * dx / distance;
		pos->y_pos += speed * dy / distance;
		pos->z_pos += speed * dz / distance;
	}
	else
	{
		pos->x_pos = dest->x_pos;
		pos->y_pos = dest->y_pos;
		pos->z_pos = dest->z_pos;
	}

	adiff = dest->x_rot - pos->x_rot;

	if (adiff > rotation)
		pos->x_rot += rotation;
	else if (adiff < -rotation)
		pos->x_rot -= rotation;
	else
		pos->x_rot = dest->x_rot;

	adiff = dest->y_rot - pos->y_rot;

	if (adiff > rotation)
		pos->y_rot += rotation;
	else if (adiff < -rotation)
		pos->y_rot -= rotation;
	else
		pos->y_rot = dest->y_rot;

	adiff = dest->z_rot - pos->z_rot;

	if (adiff > rotation)
		pos->z_rot += rotation;
	else if (adiff < -rotation)
		pos->z_rot -= rotation;
	else
		pos->z_rot = dest->z_rot;

	return pos->x_pos == dest->x_pos && pos->y_pos == dest->y_pos && pos->z_pos == dest->z_pos &&
		pos->x_rot == dest->x_rot && pos->y_rot == dest->y_rot && pos->z_rot == dest->z_rot;
}

long MoveLaraPosition(PHD_VECTOR* v, ITEM_INFO* item, ITEM_INFO* l)
{
	PHD_3DPOS pos;
	long height;
	short room_number;

	pos.x_rot = item->pos.x_rot;
	pos.y_rot = item->pos.y_rot;
	pos.z_rot = item->pos.z_rot;
	phd_PushUnitMatrix();
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
	pos.x_pos = item->pos.x_pos + ((v->x * phd_mxptr[M00] + v->y * phd_mxptr[M01] + v->z * phd_mxptr[M02]) >> W2V_SHIFT);
	pos.y_pos = item->pos.y_pos + ((v->x * phd_mxptr[M10] + v->y * phd_mxptr[M11] + v->z * phd_mxptr[M12]) >> W2V_SHIFT);
	pos.z_pos = item->pos.z_pos + ((v->x * phd_mxptr[M20] + v->y * phd_mxptr[M21] + v->z * phd_mxptr[M22]) >> W2V_SHIFT);
	phd_PopMatrix();

	if (item->object_number == FLARE_ITEM)
	{
		room_number = l->room_number;
		height = GetHeight(GetFloor(pos.x_pos, pos.y_pos, pos.z_pos, &room_number), pos.x_pos, pos.y_pos, pos.z_pos);

		if (abs(height - l->pos.y_pos) > 512)
			return 0;

		if (phd_sqrt(SQUARE(pos.x_pos - l->pos.x_pos) + SQUARE(pos.y_pos - l->pos.y_pos) + SQUARE(pos.z_pos - l->pos.z_pos)) < 128)
			return 1;
	}

	return Move3DPosTo3DPos(&l->pos, &pos, 16, 364);
}

void inject_collide(bool replace)
{
	INJECT(0x0041E690, ShiftItem, replace);
	INJECT(0x0041EBD0, ObjectCollision, replace);
	INJECT(0x0041EC30, ObjectCollisionSub, replace);
	INJECT(0x0041E630, GetNewRoom, replace);
	INJECT(0x0041E560, GetNearByRooms, replace);
	INJECT(0x0041E730, GetTiltType, replace);
	INJECT(0x0041E170, CollideStaticObjects, replace);
	INJECT(0x0041E140, FindGridShift, replace);
	INJECT(0x0041D500, GetCollisionInfo, replace);
	INJECT(0x0041E6D0, UpdateLaraRoom, replace);
	INJECT(0x0041EC90, DoorCollision, replace);
	INJECT(0x0041ED10, TrapCollision, replace);
	INJECT(0x0041F0E0, TestBoundsCollide, replace);
	INJECT(0x0041F1B0, TestLaraPosition, replace);
	INJECT(0x0041F2F0, AlignLaraPosition, replace);
	INJECT(0x0041F5C0, Move3DPosTo3DPos, replace);
	INJECT(0x0041F430, MoveLaraPosition, replace);
}