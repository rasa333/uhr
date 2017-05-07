objs   = uhr.o tcap.o signal.o input.o
LFLAGS = -ltermcap -lm
CC     = gcc -O3
prg    = uhr

$(prg)     : $(objs)
	        $(CC) -s $(CFLAGS) -o $(prg) $(objs) $(LFLAGS)

clean	   : 
	        rm -f $(prg) core *.o
 
	   
uhr.o       : uhr.c Makefile
tcap.o      : tcap.c Makefile
signal.o    : signal.c Makefile
input.o     : input.c Makefile
