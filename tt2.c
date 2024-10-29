/*
 *  (C) Copyright 2015 by Andrew Nicholas
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <sys/timeb.h>

/* width and height of game board, including left, right and bottom borders */
#define W 12
#define H 21

/* char to use for each tetramino square */
#define tmno_gfx ACS_CKBOARD

/* key mapping */
char key_left = 'h', key_right = 'l', key_down = 'j',key_rotate = 'k';

/* component locations */
int gamex = 6, gamey = 0, nextx = 22, nexty = 6;

int grid[W*H];

const int tmnolib[7][4][4] = {
	{{0,1,W,W+1},{0,1,W,W+1},{0,1,W,W+1},{0,1,W,W+1}},              /* O. */
	{{-W,0,W,W*2},{-1,0,1,2},{-W,0,W,W*2},{-1,0,1,2}},              /* I. */
	{{-1,0,1,W},{-W,0,W,1},{-1,0,1,-W},{-W,0,W,-1}},                /* T. */
	{{-W,-W+1,0,W},{-1,0,1,-W-1},{-W,0,W,W-1},{-1,0,1,W+1}},        /* P. */
	{{0,-W,-W-1,W},{-1,0,1,W-1},{-W,0,W,W+1},{-1,0,1,-W+1}},        /* L. */
	{{-1,0,-W,-W+1},{-W-1,-1,0,W},{-1,0,-W,-W+1},{-W-1,-1,0,W}},    /* S. */
	{{0,1,-W,-W-1},{0,-W,-1,W-1},{0,1,-W,-W-1},{0,-W,-1,W-1}}       /* Z. */
};

typedef struct tetramino {
	int r; /*rotation value*/
	int t; /*type*/
	int i; /*grid position*/
} tetramino;

void draw(tetramino t, tetramino n)
{
	for(int i=0; i<W*H; i++) {
		attron(COLOR_PAIR(grid[i]));
		if(grid[i] != 0)
			mvaddch(i/W + gamey, gamex + i - i/W*W,tmno_gfx);
	}
	for(int i=0; i < 4; i++) {
		int ti = t.i + tmnolib[t.t][t.r][i];
		int ni = W + 5 + tmnolib[n.t][n.r][i];
		attron(COLOR_PAIR(t.t + 1));
		mvaddch(gamey + ti/W, (gamex + ti - (ti/W)*W), tmno_gfx);
		attron(COLOR_PAIR(n.t + 1));
		mvaddch(ni/W + nexty, nextx + (ni - (ni/W)*W), tmno_gfx);
	}
	attrset(0);
}

/*check for full rows, drop them and return a score */
int drop_full_rows(tetramino t)
{
	int sm = 0;
	for(int i=0; i<4; i++) {
		int full = 1;
		int ti = t.i + tmnolib[t.t][t.r][i];
		for (int c = 0; c < W; c++)
			if(grid[((ti/W)*W) + c] == 0)
				full = 0;
		if(full == 0) 
			continue;
		memmove(&grid[W],&grid[0],((ti)/W) * W * (sizeof(int)));
		sm++;
	}
	return sm * sm;
}

/* check if space is available to place a tetramino */
int grid_free(tetramino t)
{
	int val = 0;
	for (int i = 0; i < 4; i++)
		val = val + grid[t.i + tmnolib[t.t][t.r][i]];
	return val > 0 ? 0 : 1;
}

int time()
{
	struct timeb t;
	ftime(&t);
	return t.time*1000 + t.millitm;
}

void initbg()
{
	for(int i = 0; i < W * H; i++)
		grid[i] = ((i % W == 0) || ((i + 1) % W == 0) ||
				(i > (H * W - W))) ? 1 : 0;
}

void rw_highscore(int *high, int new, const char *rwmode)
{
	FILE *scorefile;
	if ((scorefile = fopen("./hiscore", rwmode)) != NULL) {
		(*rwmode == 'r') ? fscanf(scorefile,"%d", high) : fprintf(scorefile,"%d", new);
		fclose(scorefile);
	}
	*high = *high < new ? new : *high;
}

int main(int argc, const char* argv[])
{
	int esckey=0, score=0, high=0, start_time=time();
	tetramino t = {2,0,5}, tmp = {2,0,5}, next = {2,0,5};

	if (argc == 2 && (strcmp(argv[1],"-h") == 0 || strcmp(argv[1],"-v") == 0)){
		printf("\nTinyTetris2(tt2)\nVersion: %s\nA Nicholas\n",VERSION);
		return 0;
	}

	initscr();
	curs_set(0);
	start_color();
	initbg();
	for(int i=0; i<8; i++)
		init_pair(i,i,0);
	next.t = rand() % 7;
	rw_highscore(&high, -1, "r");
	keypad(stdscr, TRUE);

	while(esckey != 27) {
		clear();
		mvprintw(gamey + H + 1,gamex,"score: %d\thigh: %d",score, high);
		mvprintw(nexty - 2,nextx,"next:");
		draw(t,next);
		tmp = t;
		timeout(20);
		int key_val = getch();

		if(key_val == key_right || key_val == KEY_RIGHT)
			tmp.i++;
		if(key_val == key_left || key_val == KEY_LEFT)
			tmp.i--;
		if(key_val == key_rotate || key_val == KEY_UP)
			tmp.r = tmp.r == 3 ? 0 : tmp.r + 1;
		if(key_val == key_down || key_val == KEY_DOWN)
			tmp.i += W;

		if (grid_free(tmp))
			t = tmp;

		if ((time() - start_time) <=  200/(1 + score/50))
			continue;

		start_time=time();

		t.i =  t.i + W;
		if(grid_free(t))
			continue;

		t.i -= W;
		for (int i = 0; i < 4; i++)
			if ((t.i + tmnolib[t.t][t.r][i]) >= 0) grid[t.i + tmnolib[t.t][t.r][i]] = t.t + 1;

		if(t.i <= W) {
			if(score > high)
				rw_highscore(&high, score, "w");
			score=0;
			timeout(-1);
			mvprintw(23,6,"game over! press esc to quit");
			esckey = getch();
			initbg();
		} else {
			score += drop_full_rows(t);
		}

		t = next;
		next.t = rand() % 7;
	}

	endwin();
	return 0;
}

/* vim:set noet cindent sts=8 ts=8 sw=8 tw=80: */
