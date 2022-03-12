#include <stdio.h>
#include <stdlib.h>
#include <term.h>
#include <termio.h>
#include <signal.h>
#include <unistd.h>


#define FALSE 0
#define TRUE  1


static char cap_buffer[2048], tcstrings[256];

static char
        *SO,        /* Stand-Out Modus	*/
        *SE,        /* Ende SO Modus	*/
        *CL,        /* Clear Screen		*/
        *CE,        /* Clear to EOLine      */
        *AL,        /* Add New Line         */
        *DL,        /* Del Line             */
        *CD,        /* Clear end of Display */
        *CM,        /* Cursor motion        */
        *VI,        /* Cursor invisible     */
        *VE,        /* Cursor normal        */
        *F1;

static int COLS, LINES;


static int outchar(int c)
{
    putchar(c);
}


int init_tcap()
{
    char *tname, *tcs;

    if ((tname = getenv("TERM")) == NULL)
        return 0;

    if (tgetent(cap_buffer, tname) == -1)
        return 0;

    tcs = tcstrings;
    *tcs++ = 0;
    SO = tgetstr("so", &tcs);
    SE = tgetstr("se", &tcs);
    CL = tgetstr("cl", &tcs);
    AL = tgetstr("al", &tcs);
    DL = tgetstr("dl", &tcs);
    CE = tgetstr("ce", &tcs);
    CM = tgetstr("cm", &tcs);
    CD = tgetstr("cd", &tcs);
    VI = tgetstr("vi", &tcs);
    VE = tgetstr("ve", &tcs);
    F1 = tgetstr("F1", &tcs);
    LINES = tgetnum("li");
    COLS = tgetnum("co");

    if (!CL || !CE || !CM || !CD)
        return 0;

    return 1;
}

void clrscr()
{
    tputs(CL, 1, outchar);
    fflush(stdout);
}

int scrsize(int *x, int *y)
{
    struct winsize size;

    if (ioctl(fileno(stdout), TIOCGWINSZ, &size) == 0) {
        (*y) = LINES = size.ws_row;
        (*x) = COLS = size.ws_col;
        return 0;
    } else
        return -1;
}


void gotoxy(int x, int y)
{
    tputs(tgoto(CM, x, y), 1, outchar);
    fflush(stdout);
}


void standout()
{
    if (SO) {
        tputs(SO, 1, outchar);
        fflush(stdout);
    }
}


void standend()
{
    if (SE) {
        tputs(SE, 1, outchar);
        fflush(stdout);
    }
}

void cursor_hide()
{
    if (VI) {
        tputs(VI, 1, outchar);
        fflush(stdout);
    }
}


void cursor_show()
{
    if (VE) {
        tputs(VE, 1, outchar);
        fflush(stdout);
    }
}

int kbhit()
{
    struct timeval timeout;
    fd_set rd;
    int r;

    FD_ZERO (&rd);
    FD_SET (0, &rd);

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    r = select(1, &rd, (fd_set *) 0, (fd_set *) 0, &timeout);
    if (r < 0)
	return 0;

    return r;
}

char readkey()
{
    char c;

    read(0, &c, 1);

    return c;
}
