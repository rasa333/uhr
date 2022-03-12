#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

#define SECP    '.'
#define MINP    '+'
#define HORP    '*'
#define CIRCLE1 '#'
#define CIRCLE2 '.'
#define DIGITAL '#'

extern int init_tcap();
extern void cursor_hide();
extern void cursor_show();
extern void standout();
extern void standend();
extern char readkey();
extern int kbhit();
extern void gotoxy(int x, int y);
extern int scrsize(int *x, int *y);
extern void clrscr();
extern void signal_action(int sig, void (*handler)(int));
extern void signal_unblock(int sig);
extern void signal_block(int sig);

int digital = 0;
char *rno[] = {"III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII", "I", "II"};

char *no[] = {\
" ----- \
 |     |\
 |     |\
 |     |\
 |     |\
  ----- ",

"    /| \
    / | \
   /  | \
      | \
      | \
      | ",

"  ---  \
  |   | \
     |  \
    |   \
   |    \
  ----- ",

"  ---  \
  |   | \
     -  \
      | \
  |   | \
   ---  ",

" |     \
  |  |  \
  |  |  \
   --+- \
     |  \
     |  ",

"  ---- \
  |     \
  +---  \
      | \
  |   | \
   ---  ",

"  ---  \
  |   | \
  |     \
  +---  \
  |   | \
   ---  ",

" ----- \
      / \
     /  \
  --+-- \
   /    \
  /     ",

"  ---  \
  |   | \
   ---  \
  |   | \
  |   | \
   ---  ",

"  ---  \
  |   | \
   ---+ \
      | \
  |   | \
   ---  ",

"       \
    ##  \
        \
        \
    ##  \
        "};


int *lines_mem[3];
int screen_set = 0;
int line_cnt;
char buf[2];
char soc = 0;
static struct tm *tm, o_tm;
int xso = 0, yso = 0, x, y, xmo = 0, ymo = 0, xho = 0, yho = 0;
long t;
int newmin = 0;
int show_secptr = 1;
int mid_x, mid_y, highest_xy;
struct termios tios, tios_reset;

#define SETSEC 0
#define SETMIN 1
#define SETHOR 2

#define max(a, b)   (a > b ? a : b)

void initmap()
{
    int h, i;

    if (lines_mem[0] == NULL) {
        lines_mem[0] = malloc(sizeof(int) * highest_xy);
        lines_mem[1] = malloc(sizeof(int) * highest_xy);
        lines_mem[2] = malloc(sizeof(int) * highest_xy);
    } else {
        lines_mem[0] = realloc(lines_mem[0], sizeof(int) * highest_xy);
        lines_mem[1] = realloc(lines_mem[1], sizeof(int) * highest_xy);
        lines_mem[2] = realloc(lines_mem[2], sizeof(int) * highest_xy);
    }

    for (h = 0; h < 3; h++)
        for (i = 0; i < highest_xy; i++)
            lines_mem[h][i] = 0;
}

int check_pos(int x, int y)
{
    int i;

    if (screen_set == SETSEC) {
        for (i = 0; lines_mem[SETMIN][i] != 0; i += 2)
            if (lines_mem[SETMIN][i] == x && lines_mem[SETMIN][i + 1] == y)
                return 0;
        for (i = 0; lines_mem[SETHOR][i] != 0; i += 2)
            if (lines_mem[SETHOR][i] == x && lines_mem[SETHOR][i + 1] == y)
                return 0;
    }
    if (screen_set == SETMIN) {
        for (i = 0; lines_mem[SETHOR][i] != 0; i += 2)
            if (lines_mem[SETHOR][i] == x && lines_mem[SETHOR][i + 1] == y)
                return 0;
    }
    return 1;
}


void m_gotoxy(int x, int y, int ch)
{
    if (!check_pos(x, y))
        return;
    gotoxy(x, y);
    putchar(ch);
    lines_mem[screen_set][line_cnt] = x;
    lines_mem[screen_set][line_cnt + 1] = y;
    lines_mem[screen_set][line_cnt + 2] = 0;
    line_cnt += 2;
    fflush(stdout);
}


void line(int x1, int y1, int new_x_cursor, int new_y_cursor, int ch)
{
    int x_dist;
    int y_dist;
    int x_start;
    int y_start;
    int x_final;
    int y_final;
    int x_current;
    int y_current;
    int x_pixels;
    int y_pixels;
    long int slope;
    int offset;
    long int idx;

    line_cnt = 0;
    x_current = x_start = x1;
    y_current = y_start = y1;
    x_final = new_x_cursor;
    y_final = new_y_cursor;
    x_dist = x_final - x_start;
    y_dist = y_final - y_start;
    x_pixels = abs(x_dist);
    y_pixels = abs(y_dist);

    if (x_pixels > y_pixels) {
        if (x_dist != 0)
            slope = (long) y_dist * 0x010000L / (long) x_dist;
        else
            slope = 0x7FFFFFFFL; /* Infinity */
        if (slope > 0)
            offset = 1;
        else if (slope < 0)
            offset = -1;
        else
            offset = 0;
        if (x_final - x_start >= 0) {
            if (slope == 0) {
                while (x_current <= x_final)
                    m_gotoxy(x_current++, y_current, ch);
            } else
                for (idx = 0; idx <= x_pixels; idx++, x_current++) {
                    y_current = y_start + (idx * slope / 0x08000L + offset) / 2;
                    m_gotoxy(x_current, y_current, ch);
                }
        } else {
            if (slope == 0) {
                while (x_current >= x_final)
                    m_gotoxy(x_current--, y_current, ch);
            } else
                for (idx = 0; idx <= x_pixels; idx++, x_current--) {
                    y_current = y_start - (idx * slope / 0x08000L + offset) / 2;
                    m_gotoxy(x_current, y_current, ch);
                }
        }
    } else {
        if (y_dist != 0)
            slope = (long) x_dist * 0x010000L / (long) y_dist;
        else
            slope = 0x7FFFFFFF; /* Infinity */
        if (slope > 0)
            offset = 1;
        else if (slope < 0)
            offset = -1;
        else
            offset = 0;
        if (y_final - y_start >= 0) {
            if (slope == 0) {
                while (y_current <= y_final)
                    m_gotoxy(x_current, y_current++, ch);
            } else
                for (idx = 0; idx <= y_pixels; idx++, y_current++) {
                    x_current = x_start + (idx * slope / 0x08000L + offset) / 2;
                    m_gotoxy(x_current, y_current, ch);
                }
        } else {
            if (slope == 0) {
                while (y_current >= y_final)
                    m_gotoxy(x_current, y_current--, ch);
            } else
                for (idx = 0; idx <= y_pixels; idx++, y_current--) {
                    x_current = x_start - (idx * slope / 0x08000L + offset) / 2;
                    m_gotoxy(x_current, y_current, ch);
                }
        }
    }
    fflush(stdout);
}


void display_no(int x, int y, int number)
{
    int i, stdo = 0;

    for (i = 0 ; i < 47 ; i++) {
        if (!(i % 8))
            gotoxy(x, y + i / 8);
	if (no[number][i] != ' ' && stdo == 0) {
	    standout();
	    stdo = 1;
	} else if (no[number][i] == ' ' && stdo == 1) {
	    standend();
	    stdo = 0;
	}
	putchar(' ');
    }
    fflush(stdout);
}


void dt()
{
    if (tm->tm_hour != o_tm.tm_hour) {
        sprintf(buf, "%2.2d", tm->tm_hour);
        display_no(mid_x - 32, mid_y - 3, buf[0] - 48);
        display_no(mid_x - 24, mid_y - 3, buf[1] - 48);
    }
    if (tm->tm_min != o_tm.tm_min) {
        sprintf(buf, "%2.2d", tm->tm_min);
        display_no(mid_x - 8, mid_y - 3, buf[0] - 48);
        display_no(mid_x + 0, mid_y - 3, buf[1] - 48);
    }
    if (show_secptr) {
        if (tm->tm_sec != o_tm.tm_sec) {
            sprintf(buf, "%2.2d", tm->tm_sec);
            if (soc != buf[0]) {
                display_no(mid_x + 16, mid_y - 3, buf[0] - 48);
                soc = buf[0];
            }
            display_no(mid_x + 24, mid_y - 3, buf[1] - 48);
        }
    }
}


void dt_screen()
{
    scrsize(&mid_x, &mid_y);
    mid_x /= 2;
    mid_y /= 2;

    if (!show_secptr)
        mid_x += 13;

    display_no(mid_x - 16, mid_y - 3, 10);
    if (show_secptr)
        display_no(mid_x + 8, mid_y - 3, 10);
}


void at()
{
    int i;

    tm->tm_hour = tm->tm_hour > 12 ? tm->tm_hour - 12 : tm->tm_hour;

    if (tm->tm_min != o_tm.tm_min) {
        screen_set = SETMIN;
        i = tm->tm_min * 6 - 90;
        x = cos((double) i / 58) * (mid_x / 2.1);
        y = sin((double) i / 58) * (mid_y / 1.9);
        line(mid_x, mid_y, mid_x + xmo, mid_y + ymo, 32);
        line(mid_x, mid_y, mid_x + x, mid_y + y, MINP);
        xmo = x;
        ymo = y;
        newmin = 1;
    }
    if (tm->tm_hour != o_tm.tm_hour || ((tm->tm_min % 10) == 0 && newmin)) {
        screen_set = SETHOR;
        i = ((tm->tm_hour * 30) + tm->tm_min / 2) - 90;
        x = cos((double) i / 58) * (mid_x / 2.6);
        y = sin((double) i / 58) * (mid_y / 2.2);
        line(mid_x, mid_y, mid_x + xho, mid_y + yho, 32);
        line(mid_x, mid_y, mid_x + x, mid_y + y, HORP);
        xho = x;
        yho = y;
        newmin = 0;
    }
    if (tm->tm_sec != o_tm.tm_sec && show_secptr) {
        screen_set = SETSEC;
        i = tm->tm_sec * 6 - 90;
        x = cos((double) i / 58) * (mid_x / 2.1);
        y = sin((double) i / 58) * (mid_y / 1.9);
        line(mid_x, mid_y, mid_x + xso, mid_y + yso, 32);
        line(mid_x, mid_y, mid_x + x, mid_y + y, SECP);
        xso = x;
        yso = y;
    }
}


void at_screen()
{
    int i, a;

    scrsize(&mid_x, &mid_y);
    mid_x /= 2;
    mid_y /= 2;
    highest_xy = max(mid_x, mid_y) * 1.2;
    a = 12 * highest_xy;
    if (a < 360)
        a = 360;
    initmap();

    for (i = 0; i < a; i++) {
        x = cos((double) i / 58) * (mid_x / 1.5);
        y = sin((double) i / 58) * (mid_y / 1.2);
        gotoxy(mid_x + x, mid_y + y);
        putchar(CIRCLE1);
    }
    for (i = 0; i < a; i++) {
        x = cos((double) i / 58) * (mid_x / 1.7);
        y = sin((double) i / 58) * (mid_y / 1.4);
        gotoxy(mid_x + x, mid_y + y);
        putchar(CIRCLE2);
    }
    for (i = 0; i < 12; i++) {
        x = cos((double) i * 30 / 58) * (mid_x / 1.7);
        y = sin((double) i * 30 / 58) * (mid_y / 1.4);
        gotoxy(mid_x + x, mid_y + y);
        fputs(rno[i], stdout);
    }
}

void adjust()
{
    clrscr();
    initmap();
    if (digital)
        dt_screen();
    else
        at_screen();

    xmo = ymo = xho = yho = xso = yso = 0;
    o_tm.tm_hour = o_tm.tm_min = o_tm.tm_sec = soc = 99;
}

void terminate()
{
    clrscr();
    cursor_show();
    tcsetattr(0, 0, &tios_reset);
    exit(0);
}

void choice()
{
    int kb;
    
    o_tm.tm_hour = o_tm.tm_min = o_tm.tm_sec = 99;

    if (!digital)
        at_screen();
    else
        dt_screen();

    while (1) {
	time(&t);
        tm = localtime(&t);
        switch (digital) {
            case 1:
                dt();
                break;
            case 0:
                at();
                break;
        }
        o_tm.tm_hour = tm->tm_hour;
        o_tm.tm_min = tm->tm_min;
        o_tm.tm_sec = tm->tm_sec;
	
	signal_unblock(SIGWINCH);
	signal_unblock(SIGINT);
	signal_unblock(SIGTERM);
	kb = kbhit();
	signal_block(SIGWINCH);
	signal_block(SIGINT);
	signal_block(SIGTERM);
	if (kb) {
            switch (readkey()) {
                case 12:
                    clrscr();
                    o_tm.tm_hour = o_tm.tm_min = o_tm.tm_sec = soc = 99;
                    if (digital) {
                        dt_screen();
                    } else {
                        at_screen();
                    }
                    break;
                case 'd':
                    if (digital)
                        break;
                    clrscr();
                    dt_screen();
                    digital = 1;
                    o_tm.tm_hour = o_tm.tm_min = o_tm.tm_sec = soc = 99;
                    break;
                case 'a':
                    if (!digital)
                        break;
                    clrscr();
                    at_screen();
                    digital = 0;
                    o_tm.tm_hour = o_tm.tm_min = o_tm.tm_sec = 99;
                    break;
                case 's':
                    show_secptr = !show_secptr;
                    if (!show_secptr && !digital)
                        line(mid_x, mid_y, mid_x + xso, mid_y + yso, 32);
                    if (digital) {
                        clrscr();
                        o_tm.tm_hour = o_tm.tm_min = o_tm.tm_sec = soc = 99;
                        dt_screen();
                    }
                    break;
                case 'q':
                case 3:
                    terminate();
	    }
        }
     }
}


static void usage()
{
    printf("Usage: uhr [option(s)...]\n");
    printf(" -h          help\n");
    printf(" -s          disable second hand\n");
    printf(" -d          enable digital watch\n");
}


int main(int argc, char **argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "dsh")) != -1) {
        switch (opt) {
            case 'd':
                digital = 1;
                break;
            case 's':
                show_secptr = 0;
                break;
            case 'h':
                usage();
                exit(0);
            default:
                usage();
                exit(1);
        }
    }
    lines_mem[0] = NULL;
    init_tcap();
    cursor_hide();

    tcgetattr(0, &tios_reset);
    cfmakeraw(&tios);
    tcsetattr(0, 0, &tios);

    signal_action(SIGWINCH, adjust);
    signal_block(SIGWINCH);
    signal_action(SIGINT, terminate);
    signal_block(SIGINT);
    signal_action(SIGTERM, terminate);
    signal_block(SIGTERM);
    clrscr();
    choice();
}
