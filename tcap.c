#include <stdio.h>
#include <stdlib.h>
#include <term.h>
#include <termio.h>
#include <signal.h>
#include <unistd.h>


#define FALSE 0
#define TRUE  1

extern void signal_action(int sig, void (*handler)(int));
extern void signal_unblock(int sig);
extern void signal_block(int sig);


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


static void adjust()
{
    struct winsize size;

    if (ioctl(fileno(stdout), TIOCGWINSZ, &size) == 0) {
        LINES = size.ws_row;
        COLS = size.ws_col;
    }
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
    signal_action(SIGWINCH, adjust);
    signal_block(SIGWINCH);

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


void clrtobot()
{
    tputs(CD, 1, outchar);
    fflush(stdout);
}


void clrtoeol()
{
    tputs(CE, 1, outchar);
    fflush(stdout);
}


void insertln()
{
    if (AL) {
        tputs(AL, 1, outchar);
        fflush(stdout);
    }
}


void insert_n_lines(int y, int n)
{
    gotoxy(0, LINES - 1 - n);
    clrtobot();
    gotoxy(0, y);
    while (n--)
        insertln();
}


void deleteln()
{
    if (DL) {
        tputs(DL, 1, outchar);
        fflush(stdout);
    }
}


void delete_n_lines(int y, int n)
{
    gotoxy(0, y);
    while (n--)
        deleteln();
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


static int __setty_called = FALSE;
static struct termio otio, ntio;

void setty()
{
    if (!__setty_called) {
        ioctl(0, TCGETA, &otio);
        ntio = otio;
        ntio.c_lflag &= ~ECHO;
        ntio.c_lflag &= ~ICANON;
        ntio.c_oflag |= ONLCR;
        ntio.c_iflag |= ICRNL;
        ntio.c_cc[VMIN] = 1;
        ntio.c_cc[VTIME] = 0;
        ntio.c_cc[VINTR] = 0;
        ioctl(0, TCSETAW, &ntio);
        __setty_called = TRUE;
    }
    ioctl(0, TCSETAW, &ntio);
}


void resetty()
{
    if (__setty_called) {
        ioctl(0, TCSETAW, &otio);
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
