/* Copyright (C) 2002 W.P. van Paassen - peter@paassen.tmfweb.nl

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to the Free
   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

/* note that the code has not been fully optimized */

/* From part of The Demo Effect Collection:
   http://demo-effects.sourceforge.net
   Converted to curses & edited by mtklr, May 2014
*/

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#define NUMBER_OF_STARS 100

/* star struct */
typedef struct
{
    float xpos, ypos;
    short zpos, speed;
    attr_t color;
} STAR;

static STAR stars[NUMBER_OF_STARS];

unsigned int centerx, centery;
int bgcolor = -1;
int color_flag = 0;
int delay = 50;

void resize_win(int sig) {
	endwin();
	refresh();
	centerx = COLS >> 1;
	centery = LINES >> 1;
}

void quit(int code)
{
    endwin();
    exit(code);
}

void process_events(void)
{
	int key = getch();

	if (key != EOF && key != KEY_RESIZE) {
		quit(0);
	}
}

void init_star(STAR* star, int i)
{
    /* randomly init stars, generate them around the center of the screen */

    star->xpos = -10.0 + (20.0 * (rand()/(RAND_MAX+1.0)));
    star->ypos = -10.0 + (20.0 * (rand()/(RAND_MAX+1.0)));

    star->xpos *= 1024.0; /* change viewpoint */
    star->ypos *= 1024.0;

    star->zpos = i;
    star->speed = 2 + (int)(2.0 * (rand()/(RAND_MAX+1.0)));

    /* the closer to the viewer the brighter */
    /* TODO: make this work w/ term colors */
    /* star->color = COLOR_PAIR(i >> 2); */

    switch(color_flag) {
        case 1:
            /* standard colors */
            star->color = COLOR_PAIR(i % 7 + 1);
            break;
        case 2:
            /* 256 color */
            star->color = COLOR_PAIR(i % 15 + 1);
            break;
        case 3:
            /* greyscale for 256 color term */
            star->color = COLOR_PAIR(i % 22);
            break;
        default:
            break;
    }
}

void init()
{
    int i;

    for (i = 0; i < NUMBER_OF_STARS; i++) {
        init_star(stars + i, i + 1);
    }

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(delay);
    leaveok(stdscr, TRUE);
    nonl();

    if (color_flag < 1 || !has_colors() || !can_change_color()) {
	    color_flag = 0;
	    return;
    }

            start_color();
	    use_default_colors();

            switch(color_flag) {
                case 1:
                    /* default colors */
                    init_pair(1, COLOR_RED, bgcolor);
                    init_pair(2, COLOR_GREEN, bgcolor);
                    init_pair(3, COLOR_YELLOW, bgcolor);
                    init_pair(4, COLOR_BLUE, bgcolor);
                    init_pair(5, COLOR_MAGENTA, bgcolor);
                    init_pair(6, COLOR_CYAN, bgcolor);
                    init_pair(7, COLOR_WHITE, bgcolor);
                    break;
                case 2:
                    /* 256 colors */
                    for (i = 0; i < 255; ++i)
                      {
                        init_pair(i, i, bgcolor);
                      }
                    break;
                case 3:
                    /* greyscale range 234-255, for 256 color term */
                    for (i = 0; i < 22; ++i)
                      {
                        init_pair(i, 255 - (i % 22), bgcolor);
                      }
                    break;
                default:
                    break;
            }
	    bkgd(COLOR_PAIR(1));
}

int main(int argc, char* argv[])
{
    int tempx, tempy;
    int i;
    int ch;
    bool twotf_flag = 0;
    char *twotf[4] = { "WAY", "OF", "THE", "FUTURE" };
    char *starch = "...";

    while ((ch = getopt(argc, argv, ":Bbcd:fgwz")) != -1) {
        switch (ch) {
		case 'B':
			bgcolor = 16;
			break;
		case 'b':
			bgcolor = 0;
			break;
            case 'c':
                color_flag = 1;
                break;
            case 'd':
                delay = atoi(optarg);
                if (delay < 0 || delay > 100 || !isdigit(*optarg)) {
                    delay = 50;
                }
                break;
            case 'f':
                color_flag = 2;
                break;
            case 'g':
                color_flag = 3;
                break;
            case 'w':
                twotf_flag = 1;
                break;
            case 'z':
                starch = ".+*";
                break;
            case '?':
            default:
                printf("usage: %s [-d 1..100] [-Bbcfg] [-z] [string]\n", argv[0]);
                exit(1);
        }
    }
    argc -= optind;
    argv += optind;

    init();
    signal(SIGWINCH, resize_win);

    centerx = COLS >> 1;
    centery = LINES >> 1;

    /* time based demo loop */
    while (1) {
        process_events();

        /* clear screen */
	clear();

        /* move and draw stars */
        for (i = 0; i < NUMBER_OF_STARS; i++) {
            stars[i].zpos -= stars[i].speed;

            if (stars[i].zpos <= 0) {
                init_star(stars + i, i + 1);
            }

            /* compute 3D position */
            tempx = (stars[i].xpos / stars[i].zpos) + centerx;
            tempy = (stars[i].ypos / stars[i].zpos) + centery;

            /* check if a star leaves the screen */
            if (tempx < 0 || tempx > COLS - 1 || tempy < 0 || tempy > LINES - 1) {
                init_star(stars + i, i + 1);
                continue;
            }

            attrset(stars[i].color);

            /* use command line string, if any */
            if (argv[0]) {
                mvprintw(tempy, tempx, "%s", argv[0]);
            } else {
                if (stars[i].zpos > 60) {
                    mvaddch(tempy, tempx, starch[0]);
                } else if (stars[i].zpos > 40) {
                    mvaddch(tempy, tempx, starch[1]);
                } else {
                    if (twotf_flag) {
                        mvprintw(tempy, tempx, "%s", twotf[i % 4]);
                    } else {
                        mvaddch(tempy, tempx, starch[2]);
                    }
                }
            }
        }
    }

    return 0; /* never reached */
}
