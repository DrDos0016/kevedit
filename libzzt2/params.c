/* params.c	-- The evil tile params
 * $Id: params.c,v 1.1 2002/02/20 00:01:59 bitman Exp $
 * Copyright (C) 2001 Kev Vance <kev@kvance.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>
#include <string.h>

#include "zzt.h"

/* Param profile look-up table */
/* THIS IS ZZT IN A NUT-SHELL! */
const ZZTprofile _zzt_param_profile_table[] = {
	/* ZZT_EMPTY          */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_EDGE           */ { 0, 0, { 0, 0, 0 } },   /* TODO: are edges paramless? */
	/* Invalid            */ { 0, 0, { 0, 0, 0 } },
	/* Invalid            */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_PLAYER         */ { ZZT_PROPERTY_CYCLE, 1, { 0, 0, 0 } },
	/* ZZT_AMMO           */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_TORCH          */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_GEM            */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_KEY            */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_DOOR           */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_SCROLL         */ { ZZT_PROPERTY_CYCLE | ZZT_PROPERTY_PROGRAM, 1, { 0, 0, 0 } },
	/* ZZT_PASSAGE        */ { ZZT_PROPERTY_NONE, 0, { 0, 0, ZZT_DATAUSE_PASSAGEDEST } },
	/* ZZT_DUPLICATOR     */ { ZZT_PROPERTY_STEP, 0, { 0, ZZT_DATAUSE_DUPRATE, 0 } },
	/* ZZT_BOMB           */ { ZZT_PROPERTY_CYCLE, 6, { 0, 0, 0 } },   /* TODO: there must be more to it than this */
	/* ZZT_ENERGIZER      */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_STAR           */ { ZZT_PROPERTY_STEP | ZZT_PROPERTY_CYCLE, 1, { ZZT_DATAUSE_OWNER, 0, 0 } }, /* TODO: does owner matter? */
	/* ZZT_CWCONV         */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_CCWCONV        */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_BULLET         */ { ZZT_PROPERTY_STEP | ZZT_PROPERTY_CYCLE, 1, { ZZT_DATAUSE_OWNER, 0, 0 } },
	/* ZZT_WATER          */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_FOREST         */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_SOLID          */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_NORMAL         */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_BREAKABLE      */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_BOULDER        */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_NSSLIDER       */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_EWSLIDER       */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_FAKE           */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_INVISIBLE      */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_BLINK          */ { ZZT_PROPERTY_CYCLE | ZZT_PROPERTY_STEP, 1, { ZZT_DATAUSE_STARTTIME, ZZT_DATAUSE_PERIOD, 0 } },
	/* ZZT_TRANSPORTER    */ { ZZT_PROPERTY_CYCLE | ZZT_PROPERTY_STEP, 2, { 0, 0, 0 } },
	/* ZZT_LINE           */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_RICOCHET       */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_BLINKHORIZ     */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_BEAR           */ { ZZT_PROPERTY_CYCLE, 3, { ZZT_DATAUSE_SENSITIVITY, 0, 0 } },
	/* ZZT_RUFFIAN        */ { ZZT_PROPERTY_CYCLE, 1, { ZZT_DATAUSE_INTELLIGENCE, ZZT_DATAUSE_RESTTIME, 0 } },
	/* ZZT_OBJECT         */ { ZZT_PROPERTY_CYCLE | ZZT_PROPERTY_PROGRAM, 3, { ZZT_DATAUSE_CHAR, 0, 0 } },
	/* ZZT_SLIME          */ { ZZT_PROPERTY_CYCLE, 3, { 0, ZZT_DATAUSE_SPEED, 0 } },
	/* ZZT_SHARK          */ { ZZT_PROPERTY_CYCLE, 3, { ZZT_DATAUSE_INTELLIGENCE, 0, 0 } },
	/* ZZT_SPINNINGGUN    */ { ZZT_PROPERTY_CYCLE, 3, { ZZT_DATAUSE_INTELLIGENCE, ZZT_DATAUSE_FIRERATEMODE, 0 } },
	/* ZZT_PUSHER         */ { ZZT_PROPERTY_CYCLE | ZZT_PROPERTY_STEP, 4, { 0, 0, 0 } },
	/* ZZT_LION           */ { ZZT_PROPERTY_CYCLE, 2, { ZZT_DATAUSE_INTELLIGENCE, 0, 0 } },
	/* ZZT_TIGER          */ { ZZT_PROPERTY_CYCLE, 2, { ZZT_DATAUSE_INTELLIGENCE, ZZT_DATAUSE_FIRERATEMODE, 0 } },
	/* ZZT_BLINKVERT      */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_CENTHEAD       */ { ZZT_PROPERTY_CYCLE, 2, { ZZT_DATAUSE_INTELLIGENCE, ZZT_DATAUSE_DEVIANCE, 0 } },
	/* ZZT_CENTBODY       */ { ZZT_PROPERTY_CYCLE, 2, { 0, 0, 0 } }
};

/* Data[] use default value look-up table */
u_int16_t _zzt_datause_default_table[] = {
	/* ZZT_DATAUSE_NONE         */ 0,
	/* ZZT_DATAUSE_PASSAGEDEST  */ 0,
	/* ZZT_DATAUSE_DUPRATE      */ 4,
	/* ZZT_DATAUSE_SENSITIVITY  */ 8,
	/* ZZT_DATAUSE_INTELLIGENCE */ 4,
	/* ZZT_DATAUSE_RESTTIME     */ 4,
	/* ZZT_DATAUSE_CHAR         */ 1,
	/* ZZT_DATAUSE_SPEED        */ 4,
	/* ZZT_DATAUSE_FIRERATEMODE */ 4,
	/* ZZT_DATAUSE_DEVIANCE     */ 4,
	/* ZZT_DATAUSE_STARTTIME    */ 4,
	/* ZZT_DATAUSE_PERIOD       */ 4,
	/* ZZT_DATAUSE_OWNER        */ 1,
};

/* Names for each data use */
char * _zzt_datause_name_table[] = {
	/* ZZT_DATAUSE_NONE         */ "",
	/* ZZT_DATAUSE_PASSAGEDEST  */ "Passage Destination",
	/* ZZT_DATAUSE_DUPRATE      */ "Duplication Rate",
	/* ZZT_DATAUSE_SENSITIVITY  */ "Sensitivity",
	/* ZZT_DATAUSE_INTELLIGENCE */ "Intelligence",
	/* ZZT_DATAUSE_RESTTIME     */ "Rest Time",
	/* ZZT_DATAUSE_CHAR         */ "Character",
	/* ZZT_DATAUSE_SPEED        */ "Speed",
	/* ZZT_DATAUSE_FIRERATEMODE */ "Fire Rate",
	/* ZZT_DATAUSE_DEVIANCE     */ "Deviance",
	/* ZZT_DATAUSE_STARTTIME    */ "Start Time",
	/* ZZT_DATAUSE_PERIOD       */ "Period",
	/* ZZT_DATAUSE_OWNER        */ "Fired By",
};

/* Which data[] element does each datause occur in? */
u_int16_t _zzt_datause_location_table[] = {
	/* ZZT_DATAUSE_NONE         */ 0,
	/* ZZT_DATAUSE_PASSAGEDEST  */ 2,
	/* ZZT_DATAUSE_DUPRATE      */ 1,
	/* ZZT_DATAUSE_SENSITIVITY  */ 0,
	/* ZZT_DATAUSE_INTELLIGENCE */ 0,
	/* ZZT_DATAUSE_RESTTIME     */ 1,
	/* ZZT_DATAUSE_CHAR         */ 0,
	/* ZZT_DATAUSE_SPEED        */ 1,
	/* ZZT_DATAUSE_FIRERATEMODE */ 1,
	/* ZZT_DATAUSE_DEVIANCE     */ 1,
	/* ZZT_DATAUSE_STARTTIME    */ 0,
	/* ZZT_DATAUSE_PERIOD       */ 1,
	/* ZZT_DATAUSE_OWNER        */ 0,
};


ZZTparam *zztParamCreate(ZZTtile tile)
{
	ZZTparam *param;
	ZZTprofile profile = _zzt_param_profile_table[tile.type];
	int i;

	/* No params for tiles outside the profile table */
	if (tile.type > ZZT_CENTBODY)
		return NULL;

	/* No params for param-less types */
	if (profile.properties == ZZT_PROPERTY_NOPARAM)
		return NULL;

	/* Allocate the param structure */
	param = (ZZTparam *) malloc(sizeof(ZZTparam));

	/* Zero-ify */
	param->x = 0;
	param->y = 0;
	param->xstep = 0;
	param->ystep = 0;
	param->cycle = 0;
	memset(param->data, 0, sizeof(param->data));
	memset(param->magic1, 0xFF, sizeof(param->magic1));
	param->utype = ZZT_EMPTY;
	param->ucolor = 0x0F;
	memset(param->magic2, 0, sizeof(param->magic2));
	param->instruction = 0;
	param->length = 0;
	param->program = NULL;

	/* Consider profile for given tile type */

	if (profile.properties & ZZT_PROPERTY_STEP)
		param->ystep = -1;  /* North by default */

	param->cycle = profile.cycledefault;

	/* Consider each data[] element */
	for (i = 0; i < 3; i++) {
		int datause = profile.datause[i];
		param->data[i] = _zzt_datause_default_table[datause];
	}

	return param;
}

int zztParamFree(ZZTparam *param)
{
	if (param->program)
		free(param->program);
	free(param);
	return 1;
}

int zztParamCopyPtr(ZZTparam *dest, ZZTparam *src)
{
	if (src == NULL || dest == NULL)
		return 0;

	memcpy(dest, src, sizeof(ZZTparam));
	if (src->program != NULL) {
		/* dup. the data, too */
		dest->program = (char *) malloc(src->length);
		if (dest->program == NULL)
			return 0;
		memcpy(dest->program, src->program, src->length);
	}
	return 1;
}

ZZTparam *zztParamDuplicate(ZZTparam *param)
{
	ZZTparam* dup = NULL;

	/* don't duplicate null params */
	if (param == NULL)
		return NULL;

	dup = (ZZTparam *) malloc(sizeof(ZZTparam));
	zztParamCopyPtr(dup, param);
	return dup;
}

u_int8_t zztParamGetProperties(ZZTtile tile)
{
	if (tile.type > ZZT_CENTBODY)
		return ZZT_PROPERTY_NOPARAM;

	return _zzt_param_profile_table[tile.type].properties;
}

int zztParamDatauseGet(ZZTtile tile, int which)
{
	if (which > 2 || tile.type > ZZT_CENTBODY)
		return ZZT_DATAUSE_NONE;

	return _zzt_param_profile_table[tile.type].datause[which];
}

const char *zztParamDatauseGetName(ZZTtile tile, int which)
{
	return _zzt_datause_name_table[zztParamDatauseGet(tile, which)];
}

int zztParamDatauseLocate(int datause)
{
	if (datause > 12) return 0;
	return _zzt_datause_location_table[datause];
}

