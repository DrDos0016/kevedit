/* main.c       -- The buck starts here
 * $Id: main.c,v 1.6 2000/08/12 18:17:28 kvance Exp $
 * Copyright (C) 2000 Kev Vance <kvance@tekktonik.net>
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
#include <dirent.h>
#include <stdio.h>

#include "display.h"
#include "kevedit.h"
#include "screen.h"
#include "scroll.h"
#include "zzt.h"

patdef patdefs[16];
param *patparams[10];
unsigned char paramlist[60][25];

void push(int type, int color, param * p)
{
	int i;

	if (patparams[9] != NULL) {
		if (patparams[9]->moredata != NULL)
			free(patparams[9]->moredata);
		free(patparams[9]);
	}
	for (i = 8; i > -1; i--) {
		patdefs[i + 7].type = patdefs[i + 6].type;
		patdefs[i + 7].color = patdefs[i + 6].color;
		patparams[i + 1] = patparams[i];
	}
	patdefs[6].type = type;
	patdefs[6].color = color;
	if (p != NULL) {
		patparams[0] = malloc(sizeof(param));
		memcpy(patparams[0], p, sizeof(param));
		if (patparams[0]->moredata != NULL) {
			/* dup. the data, too */
			patparams[0]->moredata = (char *) malloc(p->length);
			memcpy(patparams[0]->moredata, p->moredata, p->length);
		}
	} else
		patparams[0] = NULL;
}

void plot(world * myworld, editorinfo * myinfo, displaymethod * mydisplay, u_int8_t * bigboard, patdef patdefs[16])
{
	int i, x, t;
	int u = 0;
#define CURRENTPARAM	myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]
#define NEWPARAM	myworld->board[myinfo->curboard]->params[myworld->board[myinfo->curboard]->info->objectcount]

	if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
		return;
	if (paramlist[myinfo->cursorx][myinfo->cursory] != 0) {
		/* We're overwriting a parameter */
		if (CURRENTPARAM->moredata != NULL)
			free(CURRENTPARAM->moredata);
		free(CURRENTPARAM);
		for (t = i = paramlist[myinfo->cursorx][myinfo->cursory]; i < myworld->board[myinfo->curboard]->info->objectcount + 1; i++) {
			myworld->board[myinfo->curboard]->params[i] = myworld->board[myinfo->curboard]->params[i + 1];
		}
		for (x = 0; x < 25; x++) {
			for (i = 0; i < 60; i++) {
				if (paramlist[i][x] > t)
					paramlist[i][x]--;
			}
		}
		myworld->board[myinfo->curboard]->info->objectcount--;
		paramlist[myinfo->cursorx][myinfo->cursory] = 0;
		u = 1;
	}
	/* Plot the type */
	bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] = patdefs[myinfo->pattern].type;
	/* Plot the colour */
	if (patdefs[myinfo->pattern].type == Z_EMPTY)
		bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = 0x07;
	else if (myinfo->pattern < 6 || myinfo->defc == 0) {
		i = (myinfo->backc << 4) + myinfo->forec;
		if (myinfo->blinkmode == 1)
			i += 0x80;
		bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = i;
	} else
		bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = patdefs[myinfo->pattern].color;
	/* Plot the parameter if applicable */
	if (myinfo->pattern > 5 && patparams[myinfo->pattern - 6] != NULL) {
		myworld->board[myinfo->curboard]->info->objectcount++;
		if (myworld->board[myinfo->curboard]->info->objectcount < 151) {
			NEWPARAM = malloc(sizeof(param));
			memcpy(NEWPARAM, patparams[myinfo->pattern - 6], sizeof(param));
			if (patparams[myinfo->pattern - 6]->moredata != NULL) {
				NEWPARAM->moredata = (char *) malloc(patparams[myinfo->pattern - 6]->length);
				memcpy(NEWPARAM->moredata, patparams[myinfo->pattern - 6]->moredata, patparams[myinfo->pattern - 6]->length);
			}
			paramlist[myinfo->cursorx][myinfo->cursory] = myworld->board[myinfo->curboard]->info->objectcount;
			NEWPARAM->x = myinfo->cursorx + 1;
			NEWPARAM->y = myinfo->cursory + 1;
			u = 1;
		}
	}
	/* Oops, too many */
	if (myworld->board[myinfo->curboard]->info->objectcount == 151) {
		bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] = Z_EMPTY;
		bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = 0x07;
		myworld->board[myinfo->curboard]->info->objectcount--;
		u = 1;
	}
	if (u == 1)
		updatepanel(mydisplay, myinfo, myworld);
}

void floodfill(world * myworld, editorinfo * myinfo, displaymethod * mydisplay, u_int8_t * bigboard, patdef patdefs[16], int xpos, int ypos, char code, u_int8_t colour)
{
	int i, x, t, targetcolour;
	int u = 0;
#define CURRENTFLOODPARAM	myworld->board[myinfo->curboard]->params[paramlist[xpos][ypos]]

	/* Find the target colour */
	if (patdefs[myinfo->pattern].type == Z_EMPTY)
		targetcolour = 0x07;
	else if (myinfo->pattern < 6 || myinfo->defc == 0) {
		i = (myinfo->backc << 4) + myinfo->forec;
		if (myinfo->blinkmode == 1)
			i += 0x80;
		targetcolour = i;
	} else
		targetcolour = patdefs[myinfo->pattern].color;

	/* Is there any parameter space left? */
	if (myinfo->pattern > 5) {
		if (patparams[myinfo->pattern - 6] != NULL && paramlist[xpos][ypos] == 0) {
			if (myworld->board[myinfo->curboard]->info->objectcount == 150)
				return;
		}
	}
	if (xpos == myinfo->playerx && ypos == myinfo->playery)
		return;
	if (paramlist[xpos][ypos] != 0) {
		/* We're overwriting a parameter */
		if (CURRENTFLOODPARAM->moredata != NULL)
			free(CURRENTFLOODPARAM->moredata);
		free(CURRENTFLOODPARAM);
		for (t = i = paramlist[xpos][ypos]; i < myworld->board[myinfo->curboard]->info->objectcount + 1; i++) {
			myworld->board[myinfo->curboard]->params[i] = myworld->board[myinfo->curboard]->params[i + 1];
		}
		for (x = 0; x < 25; x++) {
			for (i = 0; i < 60; i++) {
				if (paramlist[i][x] > t)
					paramlist[i][x]--;
			}
		}
		myworld->board[myinfo->curboard]->info->objectcount--;
		paramlist[xpos][ypos] = 0;
	}
	/* Plot the type */
	bigboard[(xpos + ypos * 60) * 2] = patdefs[myinfo->pattern].type;
	/* Plot the colour */
	bigboard[(xpos + ypos * 60) * 2 + 1] = targetcolour;
	/* Plot the parameter if applicable */
	if (myinfo->pattern > 5 && patparams[myinfo->pattern - 6] != NULL) {
		myworld->board[myinfo->curboard]->info->objectcount++;
		if (myworld->board[myinfo->curboard]->info->objectcount < 151) {
			NEWPARAM = malloc(sizeof(param));
			memcpy(NEWPARAM, patparams[myinfo->pattern - 6], sizeof(param));
			if (patparams[myinfo->pattern - 6]->moredata != NULL) {
				NEWPARAM->moredata = (char *) malloc(patparams[myinfo->pattern - 6]->length);
				memcpy(NEWPARAM->moredata, patparams[myinfo->pattern - 6]->moredata, patparams[myinfo->pattern - 6]->length);
			}
			paramlist[xpos][ypos] = myworld->board[myinfo->curboard]->info->objectcount;
			NEWPARAM->x = xpos + 1;
			NEWPARAM->y = ypos + 1;
		}
	}
	/* Oops, too many */
	if (myworld->board[myinfo->curboard]->info->objectcount == 151) {
		bigboard[(xpos + ypos * 60) * 2] = Z_EMPTY;
		bigboard[(xpos + ypos * 60) * 2 + 1] = 0x07;
		myworld->board[myinfo->curboard]->info->objectcount--;
	}
	if (xpos != 0) {
		if (bigboard[((xpos - 1) + ypos * 60) * 2] == code && bigboard[((xpos - 1) + ypos * 60) * 2 + 1] == colour)
			floodfill(myworld, myinfo, mydisplay, bigboard, patdefs, xpos - 1, ypos, code, colour);
	}
	if (xpos != 59) {
		if (bigboard[((xpos + 1) + ypos * 60) * 2] == code && bigboard[((xpos + 1) + ypos * 60) * 2 + 1] == colour)
			floodfill(myworld, myinfo, mydisplay, bigboard, patdefs, xpos + 1, ypos, code, colour);
	}
	if (ypos != 0) {
		if (bigboard[(xpos + (ypos - 1) * 60) * 2] == code && bigboard[(xpos + (ypos - 1) * 60) * 2 + 1] == colour)
			floodfill(myworld, myinfo, mydisplay, bigboard, patdefs, xpos, ypos - 1, code, colour);
	}
	if (ypos != 24) {
		if (bigboard[(xpos + (ypos + 1) * 60) * 2] == code && bigboard[(xpos + (ypos + 1) * 60) * 2 + 1] == colour)
			floodfill(myworld, myinfo, mydisplay, bigboard, patdefs, xpos, ypos + 1, code, colour);
	}
}


void drawscrollbox(int yoffset, displaymethod * mydisplay)
{
	int t, x, i;
	i = yoffset * SCROLL_BOX_WIDTH * 2;

	for (t = 3 + yoffset; t < 3 + SCROLL_BOX_DEPTH; t++) {
		for (x = 5; x < 5 + SCROLL_BOX_WIDTH; x++) {
			mydisplay->putch(x, t, SCROLL_BOX[i], SCROLL_BOX[i + 1]);
			i += 2;
		}
	}
}
char filelist[500][13];		/* lalala, wastey wastey */

int main(int argc, char **argv)
{
	int i, x, t;
	int c, e;
	int subc, sube;
	int listpos;
	int quit = 0;
	displaymethod *mydisplay = (displaymethod *) malloc(sizeof(displaymethod));
	editorinfo *myinfo = (editorinfo *) malloc(sizeof(editorinfo));
	char *string = (char *) malloc(256);
	char *bigboard = (char *) malloc(BOARD_MAX * 2);
	char buffer[255];

	world *myworld;
	FILE *fp;

	RegisterDisplays();
	mydisplay = &display;

	/* How many display methods? */
	for (x = 1; x < 9; x++) {
		if (mydisplay->next == NULL) {
			break;
		}
		mydisplay = mydisplay->next;
	}

	if (x > 1) {
		/* More than 1 display method available, user must choose */
		printf("Hi.  This seems to be your first time running KevEdit.  What display method\n" \
		       "works best on your platform?\n\n");

		mydisplay = &display;
		for (i = 0; i < x; i++) {
			printf("[%d] %s\n", i + 1, mydisplay->name);
			if (mydisplay->next != NULL)
				mydisplay = mydisplay->next;
		}
		do {
			printf("\nSelect [1-%i]: ", x);
			fgets(string, 255, stdin);
			i = string[0];
		}
		while (i > 49 && i < 51);
		i -= '1';
		printf("\n%d SELECT\n", i);
		mydisplay = &display;
		for (x = 0; x < i; x++) {
			mydisplay = mydisplay->next;
		}
	} else {
		mydisplay = &display;
	}

	printf("Initializing %s version %s...\n", mydisplay->name, mydisplay->version);
	if (!mydisplay->init()) {
		printf("\nDisplay initialization failed.  Exiting.\n");
		free(mydisplay);
		free(myinfo);
		free(string);
		free(bigboard);
		return 1;
	}
	/* Set up initial info */

	myinfo->cursorx = myinfo->playerx = 0;
	myinfo->cursory = myinfo->playery = 0;
	myinfo->drawmode = 0;
	myinfo->blinkmode = 0;
	myinfo->defc = 1;
	myinfo->forec = 0x0f;
	myinfo->backc = 0x00;
	myinfo->pattern = 0;
	myinfo->currenttitle = (char *) malloc(20);
	strcpy(myinfo->currenttitle, "UNTITLED");
	myinfo->currentfile = (char *) malloc(13);
	myinfo->currentfile = "untitled.zzt";

	myinfo->curboard = 0;

	/* Did we get a world on the command line? */
	myworld = NULL;
	if (argc > 1) {
		myworld = loadworld(argv[1]);
		if (myworld == NULL) {
			/* Maybe it's a .zzt */
			strcpy(buffer, argv[1]);
			strcat(buffer, ".zzt");
			myworld = loadworld(buffer);
		}
		if (myworld != NULL) {
			strncpy(myinfo->currentfile, argv[1], 13);
			strncpy(myinfo->currenttitle, myworld->zhead->title, 20);
			myinfo->currenttitle[myworld->zhead->titlelength] = '\0';
			myinfo->curboard = myworld->zhead->startboard;
			rle_decode(myworld->board[myworld->zhead->startboard]->data, bigboard);
			/* Sweep the board and make all empties 0x07.  This saves space in rle
			   and makes the floodfill work correctly */
			for (i = 0; i < BOARD_MAX * 2; i += 2) {
				if (bigboard[i] == Z_EMPTY)
					bigboard[i + 1] = 0x07;
			}
			for (i = 0; i < 25; i++)
				for (x = 0; x < 60; x++)
					paramlist[x][i] = 0;
			for (i = 0; i < myworld->board[myinfo->curboard]->info->objectcount + 1; i++) {
				if (myworld->board[myinfo->curboard]->params[i]->x > 0 && myworld->board[myinfo->curboard]->params[i]->x < 61 && myworld->board[myinfo->curboard]->params[i]->y > 0 && myworld->board[myinfo->curboard]->params[i]->y < 26)
					paramlist[myworld->board[myinfo->curboard]->params[i]->x - 1][myworld->board[myinfo->curboard]->params[i]->y - 1] = i;
				myinfo->playerx = myworld->board[myinfo->curboard]->params[0]->x - 1;
				myinfo->playery = myworld->board[myinfo->curboard]->params[0]->y - 1;
			}
		}
	}
	/* Create the blank world */
	if (myworld == NULL) {
		myworld = z_newworld();
		myworld->board[0] = z_newboard("KevEdit World");
		rle_decode(myworld->board[0]->data, bigboard);
		for (i = 0; i < 25; i++)
			for (x = 0; x < 60; x++)
				paramlist[x][i] = 0;
	}
	/* Initialize pattern definitions */
	patdefs[0].type = Z_SOLID;
	patdefs[1].type = Z_NORMAL;
	patdefs[2].type = Z_BREAKABLE;
	patdefs[3].type = Z_WATER;
	patdefs[4].type = Z_EMPTY;
	patdefs[5].type = Z_LINE;
	patdefs[6].type = Z_EMPTY;
	patdefs[7].type = Z_EMPTY;
	patdefs[8].type = Z_EMPTY;
	patdefs[9].type = Z_EMPTY;
	patdefs[10].type = Z_EMPTY;
	patdefs[11].type = Z_EMPTY;
	patdefs[12].type = Z_EMPTY;
	patdefs[13].type = Z_EMPTY;
	patdefs[14].type = Z_EMPTY;
	patdefs[15].type = Z_EMPTY;

	for (i = 0; i < 5; i++)
		patparams[i] = NULL;

	/* Draw */

	drawpanel(mydisplay);
	updatepanel(mydisplay, myinfo, myworld);
	drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
	mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
	mydisplay->print(30 - strlen(myworld->board[myinfo->curboard]->title) / 2, 0, 0x70, myworld->board[myinfo->curboard]->title);

	/* Main loop begins */

	while (quit == 0) {
		cursorspace(mydisplay, myworld, myinfo, bigboard, paramlist);
		e = 0;
		c = mydisplay->getch();
		if (!c) {
			e = 1;
			c = mydisplay->getch();
		}
		/* Undo the cursorspace */
		i = (myinfo->cursorx + myinfo->cursory * 60) * 2;
		mydisplay->putch(myinfo->cursorx, myinfo->cursory,
				 z_getchar(bigboard[i], bigboard[i + 1], myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]], bigboard, myinfo->cursorx, myinfo->cursory),
				 z_getcolour(bigboard[i], bigboard[i + 1], myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]));

		switch (c) {
		case 'q':
		case 'Q':
			/* Quit */
			if (e == 0) {
				for (i = 3; i < 25; i++) {
					for (x = 0; x < 20; x++) {
						mydisplay->putch(x + 60, i, ' ', 0x1f);
					}
				}
				mydisplay->print(61, 3, 0x1f, "Quit?");
				mydisplay->print(67, 3, 0x1e, "y/n");
				c = 0;
				while (c == 0) {
					c = mydisplay->getch();
					if (c == 'y' || c == 'Y') {
						quit = 1;
					} else if (c == 'n' || c == 'N') {
						quit = 0;

					} else
						c = 0;
				}
				if (quit == 0) {
					drawpanel(mydisplay);
					updatepanel(mydisplay, myinfo, myworld);
				}
			}
			break;
		case 'U':
			fp = fopen("test.out", "wb");
			for (i = 0; i < 25; i++) {
				for (x = 0; x < 60; x++) {
					fprintf(fp, "%1X", paramlist[x][i]);
				}
				fprintf(fp, "\n");
			}
			fclose(fp);
			break;
		case 'd':
		case 'D':
			/* Toggle DefC mode */
			myinfo->defc ^= 1;
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 9:
			/* Toggle draw mode */
			myinfo->drawmode ^= 1;
			updatepanel(mydisplay, myinfo, myworld);
			if (myinfo->drawmode == 1) {
				plot(myworld, myinfo, mydisplay, bigboard, patdefs);
				drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			}
			break;
		case 'v':
		case 'V':
			/* Toggle blink mode */
			myinfo->blinkmode ^= 1;
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case ' ':
			/* Plot */
			plot(myworld, myinfo, mydisplay, bigboard, patdefs);
			drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			break;
		case 'c':
			/* Change foregeound colour */
			myinfo->forec++;
			if (myinfo->forec == 16)
				myinfo->forec = 0;
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 'C':
			/* Change background colour */
			myinfo->backc++;
			if (myinfo->backc == 8)
				myinfo->backc = 0;
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 'b':
		case 'B':
			/* Switch boards */
			i = boarddialog(myworld, myinfo, mydisplay);
			if (i == -1) {
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				updatepanel(mydisplay, myinfo, myworld);
				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				break;
			}
			free(myworld->board[myinfo->curboard]->data);
			myworld->board[myinfo->curboard]->data = rle_encode(bigboard);
			myinfo->curboard = i;
			if (myinfo->curboard > myworld->zhead->boardcount)
				myinfo->curboard = 0;
			rle_decode(myworld->board[myinfo->curboard]->data, bigboard);
			/* Sweep the board and make all empties 0x07.  This saves space in rle and
			   makes the floodfill work correctly */
			for (i = 0; i < BOARD_MAX * 2; i += 2) {
				if (bigboard[i] == Z_EMPTY)
					bigboard[i + 1] = 0x07;
			}
			for (i = 0; i < 25; i++)
				for (x = 0; x < 60; x++)
					paramlist[x][i] = 0;
			for (i = 0; i < myworld->board[myinfo->curboard]->info->objectcount + 1; i++) {
				if (myworld->board[myinfo->curboard]->params[i]->x > 0 && myworld->board[myinfo->curboard]->params[i]->x < 61 && myworld->board[myinfo->curboard]->params[i]->y > 0 && myworld->board[myinfo->curboard]->params[i]->y < 26)
					paramlist[myworld->board[myinfo->curboard]->params[i]->x - 1][myworld->board[myinfo->curboard]->params[i]->y - 1] = i;
			}
			myinfo->playerx = myworld->board[myinfo->curboard]->params[0]->x - 1;
			myinfo->playery = myworld->board[myinfo->curboard]->params[0]->y - 1;
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			mydisplay->print(30 - strlen(myworld->board[myinfo->curboard]->title) / 2, 0, 0x70, myworld->board[myinfo->curboard]->title);
			break;
		case 's':
		case 'S':
			/* Save world */
			free(myworld->board[myinfo->curboard]->data);
			myworld->board[myinfo->curboard]->data = rle_encode(bigboard);
			for (i = 3; i < 25; i++) {
				for (x = 0; x < 20; x++) {
					mydisplay->putch(x + 60, i, ' ', 0x1f);
				}
			}
			mydisplay->print(61, 3, 0x1f, "Save As");
			for (i = 0; i < 9; i++) {
				if (myinfo->currentfile[i] == '.' || myinfo->currentfile[i] == '\0') {
					buffer[i] = '\0';
					t = i;
					break;
				}
				buffer[i] = myinfo->currentfile[i];
			}
			for (i = 0; i < 9; i++) {
				if (i > strlen(buffer))
					mydisplay->putch(61 + i, 4, ' ', 0x0f);
				else
					mydisplay->putch(61 + i, 4, buffer[i], 0x0f);
			}
			mydisplay->print(70, 4, 0x1f, ".zzt");
			x = 0;
			while (x != 27) {
				mydisplay->cursorgo(61 + t, 4);
				x = mydisplay->getch();
				switch (x) {
				case 8:
					if (t > 0) {
						t--;
						buffer[t] = '\0';
						mydisplay->putch(61 + t, 4, ' ', 0x0f);
					}
					break;
				case 13:
					if (t > 0) {
						if (!strcmp(myworld->zhead->title, "UNTITLED")) {
							strcpy(myworld->zhead->title, buffer);
							myworld->zhead->titlelength = strlen(buffer);
						}
						strcpy(myinfo->currenttitle, buffer);
						strcat(buffer, ".zzt");
						strcpy(myinfo->currentfile, buffer);
						fp = fopen(buffer, "rb");
						if (fp != NULL) {
							fclose(fp);
							mydisplay->print(61, 5, 0x1f, "Overwrite?");
							mydisplay->print(72, 5, 0x1e, "y/n");
							while (c != 0) {
								c = mydisplay->getch();
								if (c == 'y' || c == 'Y') {
									goto __saveconfirm;
								}
								if (c == 'n' || c == 'N') {
									c = 0;
								}
							}
						} else {
						      __saveconfirm:
							myworld->zhead->startboard = myinfo->curboard;
							saveworld(buffer, myworld);
							mydisplay->print(61, 6, 0x1f, "Written.");
							mydisplay->cursorgo(69, 6);
							mydisplay->getch();

						}
						x = 27;
					}
					break;
				default:
					if (t == 8)
						break;
					if (x < 45)
						break;
					if (x > 45 && x < 48)
						break;
					if (x > 57 && x < 65)
						break;
					if (x > 90 && x < 95)
						break;
					if (x == 96)
						break;
					if (x > 122)
						break;
					buffer[t] = x;
					mydisplay->putch(61 + t, 4, x, 0x0f);
					t++;
					buffer[t] = '\0';
					mydisplay->cursorgo(61 + t, 4);
					break;
				}
			}
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case 'f':
		case 'F':
			/* Flood fill */
			if (patdefs[myinfo->pattern].type == Z_EMPTY)
				x = 0x07;
			else if (myinfo->pattern < 6 || myinfo->defc == 0) {
				i = (myinfo->backc << 4) + myinfo->forec;
				if (myinfo->blinkmode == 1)
					i += 0x80;
				x = i;
			} else
				x = patdefs[myinfo->pattern].color;

			if (bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] == patdefs[myinfo->pattern].type && bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] == x)
				break;
			floodfill(myworld, myinfo, mydisplay, bigboard, patdefs, myinfo->cursorx, myinfo->cursory, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2], bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1]);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			break;
		case 'L':
		case 'l':
			/* Load world */
			listpos = filedialog("zzt", mydisplay);
			if (listpos != -1) {
				z_delete(myworld);
				myworld = loadworld(filelist[listpos]);
				strncpy(myinfo->currentfile, filelist[listpos], 13);
				strncpy(myinfo->currenttitle, myworld->zhead->title, 20);
				myinfo->currenttitle[myworld->zhead->titlelength] = '\0';
				myinfo->curboard = myworld->zhead->startboard;
				rle_decode(myworld->board[myworld->zhead->startboard]->data, bigboard);
				/* Sweep the board and make all empties 0x07.  This saves space in
				   rle and makes the floodfill work correctly */
				for (i = 0; i < BOARD_MAX * 2; i += 2) {
					if (bigboard[i] == Z_EMPTY)
						bigboard[i + 1] = 0x07;
				}
				for (i = 0; i < 25; i++)
					for (x = 0; x < 60; x++)
						paramlist[x][i] = 0;
				for (i = 0; i < myworld->board[myinfo->curboard]->info->objectcount + 1; i++) {
					if (myworld->board[myinfo->curboard]->params[i]->x > 0 && myworld->board[myinfo->curboard]->params[i]->x < 61 && myworld->board[myinfo->curboard]->params[i]->y > 0 && myworld->board[myinfo->curboard]->params[i]->y < 26)
						paramlist[myworld->board[myinfo->curboard]->params[i]->x - 1][myworld->board[myinfo->curboard]->params[i]->y - 1] = i;
					myinfo->playerx = myworld->board[myinfo->curboard]->params[0]->x - 1;
					myinfo->playery = myworld->board[myinfo->curboard]->params[0]->y - 1;
				}
			}
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			mydisplay->print(30 - strlen(myworld->board[myinfo->curboard]->title) / 2, 0, 0x70, myworld->board[myinfo->curboard]->title);
			break;
		case 75:
			/* Left arrow */
			if (e == 1) {
				if (myinfo->cursorx > 0) {
					myinfo->cursorx--;
					mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
					updatepanel(mydisplay, myinfo, myworld);
				}
				if (myinfo->drawmode == 1) {
					plot(myworld, myinfo, mydisplay, bigboard, patdefs);
				}
				drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			}
			break;
		case 155:
			/* Alt+Left */
			if (e == 1) {
				myinfo->cursorx -= 10;
				if (myinfo->cursorx < 0)
					myinfo->cursorx = 0;
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				updatepanel(mydisplay, myinfo, myworld);
				drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			}
			break;
		case 77:
			/* Right arrow */
			if (e == 1) {
				if (myinfo->cursorx < 59) {
					myinfo->cursorx++;
					mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
					updatepanel(mydisplay, myinfo, myworld);
				}
				if (myinfo->drawmode == 1) {
					plot(myworld, myinfo, mydisplay, bigboard, patdefs);
				}
				drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			}
			break;
		case 157:
			/* Alt+Right */
			if (e == 1) {
				myinfo->cursorx += 10;
				if (myinfo->cursorx > 59)
					myinfo->cursorx = 59;
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				updatepanel(mydisplay, myinfo, myworld);
				drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			}
			break;
		case 72:
		case 'h':
			/* Up arrow or H */
			if (e == 1) {
				/* Move cursor up */
				if (myinfo->cursory > 0) {
					myinfo->cursory--;
					mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
					updatepanel(mydisplay, myinfo, myworld);
				}
				if (myinfo->drawmode == 1) {
					plot(myworld, myinfo, mydisplay, bigboard, patdefs);
				}
				drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			} else {
				/* Help */
				/* Actually, just an about box now */
				drawscrollbox(0, mydisplay);
				mydisplay->print(24, 4, 0x0a, "About KevEdit");
				mydisplay->print(9, 12, 0x0a, "KevEdit R5, Version");
				mydisplay->print(34, 12, 0x0a, VERSION);
				mydisplay->print(9, 13, 0x0a, "Copyright (C) 2000 Kev Vance");
				mydisplay->print(9, 14, 0x0a, "Distribute under the terms of the GNU GPL");
				mydisplay->cursorgo(9, 13);
				do {
					i = mydisplay->getch();
				} while (i != 13 && i != 27);
				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			}
			break;
		case 152:
			/* Alt+Up */
			if (e == 1) {
				myinfo->cursory -= 5;
				if (myinfo->cursory < 0)
					myinfo->cursory = 0;
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				updatepanel(mydisplay, myinfo, myworld);
				drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			}
			break;
		case 80:
			/* Down arrow or P */
			if (e == 1) {
				if (myinfo->cursory < 24) {
					myinfo->cursory++;
					mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
					updatepanel(mydisplay, myinfo, myworld);
				}
				if (myinfo->drawmode == 1) {
					plot(myworld, myinfo, mydisplay, bigboard, patdefs);
				}
				drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			} else {
				/* Select new pattern backwards */
				myinfo->pattern--;
				if (myinfo->pattern == -1)
					myinfo->pattern = 15;
				updatepanel(mydisplay, myinfo, myworld);
			}
			break;
		case 160:
			/* Alt+Down */
			if (e == 1) {
				myinfo->cursory += 5;
				if (myinfo->cursory > 24)
					myinfo->cursory = 24;
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				updatepanel(mydisplay, myinfo, myworld);
				drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			}
			break;
		case 'p':
			/* Select new pattern forwards */
			myinfo->pattern++;
			if (myinfo->pattern == 16)
				myinfo->pattern = 0;
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 59:
			/* F1 panel */
			if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
				break;
			if (e == 1) {
				i = dothepanel_f1(mydisplay, myinfo);
				switch (i) {
				case -1:
					break;
				case Z_PLAYER:
					bigboard[(myinfo->playerx + myinfo->playery * 60) * 2] = Z_EMPTY;
					bigboard[(myinfo->playerx + myinfo->playery * 60) * 2 + 1] = 0x07;
					bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] = Z_PLAYER;
					bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = 0x1f;
					paramlist[myinfo->cursorx][myinfo->cursory] = 0;
					myworld->board[myinfo->curboard]->params[0]->x = myinfo->playerx = myinfo->cursorx;
					myworld->board[myinfo->curboard]->params[0]->y = myinfo->playery = myinfo->cursory;
					myworld->board[myinfo->curboard]->params[0]->x++;
					myworld->board[myinfo->curboard]->params[0]->y++;
					break;
				case Z_GEM:
				case Z_KEY:
				case Z_CWCONV:
				case Z_CCWCONV:
					bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] = i;
					bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = (myinfo->backc << 4) + myinfo->forec;
					break;
				case Z_AMMO:
				case Z_TORCH:
				case Z_ENERGIZER:
					bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] = i;
					if (myinfo->defc == 0)
						bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = (myinfo->backc << 4) + myinfo->forec;
					else {
						if (i == Z_AMMO)
							bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = 0x03;
						if (i == Z_DUPLICATOR)
							bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = 0x0f;
						if (i == Z_TORCH)
							bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = 0x06;
						if (i == Z_ENERGIZER)
							bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = 0x05;
					}
					break;
				case Z_DOOR:
					bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] = i;
					if (myinfo->defc == 1)
						bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = myinfo->forec > 7 ? ((myinfo->forec - 8) << 4) + 0x0f : (myinfo->forec << 4) + 0x0f;
					else
						bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = (myinfo->backc << 4) + (myinfo->forec);
					break;
				}
				if (i != -1) {
					if (paramlist[myinfo->cursorx][myinfo->cursory] != 0)
						push(bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2], bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]);
					else
						push(bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2], bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], NULL);
				}
				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			}
			break;
		case 60:
			/* F2 panel */
			if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
				break;
			if (e == 1) {
				i = dothepanel_f2(mydisplay, myinfo);
				switch (i) {
				case -1:
					break;
				case Z_OBJECT:
					if (myworld->board[myinfo->curboard]->info->objectcount == 150)
						break;
					if (paramlist[myinfo->cursorx][myinfo->cursory] != 0) {
						/* We're overwriting a parameter */
						if (CURRENTPARAM->moredata != NULL)
							free(CURRENTPARAM->moredata);
						free(CURRENTPARAM);
						for (t = x = paramlist[myinfo->cursorx][myinfo->cursory]; x < myworld->board[myinfo->curboard]->info->objectcount; x++)
							myworld->board[myinfo->curboard]->params[x] = myworld->board[myinfo->curboard]->params[x + 1];
						for (x = 0; x < 25; x++) {
							for (e = 0; e < 60; e++) {
								if (paramlist[e][x] > t)
									paramlist[e][x]--;
							}
						}
						e = 1;
					} else {
						myworld->board[myinfo->curboard]->info->objectcount++;
					}
					x = bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2];
					switch (x) {
					case Z_WATER:
					case Z_FOREST:
					case Z_FAKE:
						break;
					default:
						x = Z_EMPTY;
						break;
					}
					myworld->board[myinfo->curboard]->params[myworld->board[myinfo->curboard]->info->objectcount] = z_newparam_object(myinfo->cursorx + 1, myinfo->cursory + 1, charselect(mydisplay), x, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1]);
					bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] = i;
					bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = (myinfo->backc << 4) + myinfo->forec;
					paramlist[myinfo->cursorx][myinfo->cursory] = myworld->board[myinfo->curboard]->info->objectcount;
					break;
				}
				if (i != -1) {
					if (paramlist[myinfo->cursorx][myinfo->cursory] != 0)
						push(bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2], bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]);
					else
						push(bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2], bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], NULL);
				}
				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			}
			break;
		case 61:
			/* F3 panel */
			if (e == 1) {
				dothepanel_f3(mydisplay, myinfo);
				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
			}
			break;
		case 'i':
		case 'I':
			/* Board Info */
			break;
		case 13:
			/* Modify / Grab */
			if (paramlist[myinfo->cursorx][myinfo->cursory] != 0)
				push(bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2], bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]);
			else
				push(bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2], bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], NULL);
			updatepanel(mydisplay, myinfo, myworld);
			break;
		}

	}

	mydisplay->end();

	free(myinfo);
	free(string);
	free(bigboard);
	z_delete(myworld);
}
