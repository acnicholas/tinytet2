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
/* #define tmno_gfx ACS_CKBOARD */
#define tmno_gfx ACS_BLOCK

/* vim key mapping */
char key_left = 'h', key_right = 'l', key_down = 'j',key_rotate_left = 'k', key_rotate_right = 'i';
/* wasd key mapping */
/* char key_left = 'a', key_right = 'd', key_down = 's', key_rotate_left = 'j', key_rotate_right = 'k'; */

/* component locations */
int gamex = 16, gamey = 0, nextx = 32, nexty = 6;

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

void draw_tetramino(tetramino t, int x, int y, int n)
{
	attron(COLOR_PAIR(t.t + 1));
	for(int i=0; i < 4; i++) {
		int ti = n < 1 ? t.i + tmnolib[t.t][t.r][i] : W + 5 + tmnolib[t.t][t.r][i];
		mvaddch(y + ti/W, x + (ti - (ti/W)*W), tmno_gfx);
	}
}

void draw_board()
{
	for(int i=0; i<W*H; i++) {
		attron(COLOR_PAIR(grid[i]));
		if(grid[i] != 0)
			mvaddch(i/W + gamey, gamex + i - i/W*W,tmno_gfx);
	}
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
		if(full == 0) continue;
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
		attron(COLOR_PAIR(2));
		mvprintw(gamey + H + 1,gamex,"score: %d\thigh: %d",score, high);
		mvprintw(nexty - 2,nextx,"next:");
		draw_board();
		draw_tetramino(next, nextx, nexty, 1);
		draw_tetramino(t, gamex, gamey, 0);
	        //attrset(0);
		tmp = t;
		timeout(20);
		int key_val = getch();

		/* move tmp tetramino if key pressed */
		if(key_val == key_right || key_val == KEY_RIGHT)
			tmp.i++;
		if(key_val == key_left || key_val == KEY_LEFT)
			tmp.i--;
		if(key_val == key_rotate_left || key_val == KEY_UP)
			tmp.r = tmp.r == 3 ? 0 : tmp.r + 1;
		if(key_val == key_rotate_right)
			tmp.r = tmp.r == 0 ? 3 : tmp.r - 1;
		if(key_val == key_down || key_val == KEY_DOWN)
			tmp.i += W;

		/* check if tmp tetramino can be moved to requested location */
		/* and move the current tetramino if space is clean */
		if (grid_free(tmp))
			t = tmp;

		/* check for more key pressed is move time has not elapsed */
		if ((time() - start_time) <=  200/(1 + score/50)) continue;

		start_time=time();

		/* move time has elapsed so try to drop the current tetramino */
		t.i =  t.i + W;
		if(grid_free(t)) continue;

		/* drop could not be done so lock down the tetramino; clear
		 * any full rows and update the score */
		t.i -= W;
		for (int i = 0; i < 4; i++)
			if ((t.i + tmnolib[t.t][t.r][i]) >= 0) grid[t.i + tmnolib[t.t][t.r][i]] = t.t + 1;

		/* Gameover is tetramino location is in the first row */
		/* Otherwise keep going....*/
		if(t.i <= W) {
			if(score > high)
				rw_highscore(&high, score, "w");
			score=0;
			timeout(-1);
			mvprintw(gamey + H + 3,gamex,"game over! press esc to quit");
			esckey = getch();
			initbg();
			continue;
		}

		score += drop_full_rows(t);

		/* assign the next piece to the current tetraomino */
		/* then get a new next. */
		t = next;
		next.t = rand() % 7;
	}

	endwin();
	return 0;
}

/* vim:set noet cindent sts=8 ts=8 sw=8 tw=80: */
