/* patbuffer.c    -- Pattern buffer (backbuffer) utilities
 * $Id: patbuffer.c,v 1.9 2002/02/16 21:12:12 bitman Exp $
 * Copyright (C) 2000 Kev Vance <kev@kvance.com>
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

#include "kevedit.h"
#include "libzzt2/zzt.h"
#include "display.h"
#include "screen.h"

#include <stdlib.h>
#include <string.h>


/* Create an empty patbuffer of given size */
patbuffer* patbuffer_create(int size)
{
	int i;
	patbuffer* pbuf = NULL;

	if (size <= 0)
		return NULL;

	pbuf = (patbuffer *) malloc(sizeof(patbuffer));

	pbuf->patterns = (ZZTtile *) malloc(sizeof(ZZTtile) * size);
	pbuf->size = size;
	pbuf->pos = 0;

	for (i = 0; i < pbuf->size; i++) {
		pbuf->patterns[i].type = ZZT_EMPTY;
		pbuf->patterns[i].color = 0x07;
		pbuf->patterns[i].param = NULL;
	}

	return pbuf;
}

void deletepatternbuffer(patbuffer* pbuf)
{
	int i;

	/* Free all the patterns with param data */
	for (i = 0; i < pbuf->size; i++) {
		if (pbuf->patterns[i].param != NULL) {
			zztParamFree(pbuf->patterns[i].param);
#if 0
			if (pbuf->patterns[i].param->program != NULL)
				free(pbuf->patterns[i].param->program);
			free(pbuf->patterns[i].param);
#endif
		}
	}

	/* Free the patterns themselves */
	free(pbuf->patterns);
	pbuf->patterns = NULL;
}

void patbuffer_resize(patbuffer * pbuf, int delta)
{
	int i;
	int resize = pbuf->size + delta;
	ZZTtile * repat = NULL;

	if (resize <= 0)
		return;

	repat = (ZZTtile *) malloc(sizeof(ZZTtile) * resize);

	/* Copy over all the patterns. To save time, we won't bother the param
	 * data for those patterns which remain in the new buffer. */
	for (i = 0; i < resize && i < pbuf->size; i++)
		repat[i] = pbuf->patterns[i];

	/* pbuf shrunk: clear all param data beyond the new size */
	for (; i < pbuf->size; i++) {
		if (pbuf->patterns[i].param != NULL) {
			if (pbuf->patterns[i].param->program != NULL)
				free(pbuf->patterns[i].param->program);
			free(pbuf->patterns[i].param);
		}
	}

	/* pbuf grew: fill the new slots with empties */
	for (; i < resize; i++) {
		repat[i].type = ZZT_EMPTY;
		repat[i].color = 0x07;
		repat[i].param = NULL;
	}

	/* Out with the old, in with the new */
	free(pbuf->patterns);
	pbuf->patterns = repat;
	pbuf->size = resize;

	if (pbuf->pos >= pbuf->size)
		pbuf->pos = pbuf->size - 1;
}


void pat_applycolordata(patbuffer * pbuf, editorinfo * myinfo)
{
	int i, colour;
	colour = (myinfo->backc << 4) + myinfo->forec;
	if (myinfo->blinkmode == 1)
		colour += 0x80;

	for (i = 0; i < pbuf->size; i++)
		pbuf->patterns[i].color = colour;
}


/* Push the given pattern attributes into a pattern buffer */
void push(patbuffer* pbuf, ZZTtile pattern)
{
	int i;
	ZZTtile lastpat = pbuf->patterns[pbuf->size-1];

	if (lastpat.param != NULL) {
		zztParamFree(lastpat.param);
	}
	
	/* Slide everything forward */
	for (i = pbuf->size - 1; i > 0; i--) {
		pbuf->patterns[i] = pbuf->patterns[i - 1];
	}

	pbuf->patterns[0].type = pattern.type;
	pbuf->patterns[0].color = pattern.color;
	pbuf->patterns[0].param = zztParamDuplicate(pattern.param);
}


void plot(ZZTworld * myworld, editorinfo * myinfo, displaymethod * mydisplay)
{
	patbuffer* pbuf = myinfo->pbuf;
	ZZTtile pattern = pbuf->patterns[pbuf->pos];

	/* Change the color to reflect state of default color mode */
	if (myinfo->defc == 0) {
		pattern.color = (myinfo->backc << 4) + myinfo->forec;
		if (myinfo->blinkmode == 1)
			pattern.color += 0x80;
	}

	if (zztPlot(myworld, myinfo->cursorx, myinfo->cursory, pattern))
		updatepanel(mydisplay, myinfo, myworld);
}

